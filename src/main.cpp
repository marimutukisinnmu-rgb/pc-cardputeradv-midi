#include <Arduino.h>
#include <M5Unified.h>
#include <math.h>

// USB CDC receives ordinary MIDI 1.0 byte stream.
static constexpr uint32_t SERIAL_BAUD = 115200;
static constexpr uint32_t SAMPLE_RATE = 22050;
static constexpr size_t AUDIO_BLOCK = 256;
static constexpr int VOICE_COUNT = 8;
static constexpr float TWO_PI_F = 6.28318530718f;

struct Voice {
  bool active = false;
  uint8_t channel = 0;
  uint8_t note = 0;
  uint8_t velocity = 0;
  float phase = 0.0f;
  float phaseStep = 0.0f;
  uint32_t age = 0;
};

static Voice voices[VOICE_COUNT];
static uint8_t channelVolume[16];
static uint32_t voiceAge = 0;
static int16_t audioBlock[AUDIO_BLOCK];

static uint8_t runningStatus = 0;
static uint8_t dataBytes[2];
static uint8_t dataCount = 0;
static uint8_t expectedData = 0;

static float noteFrequency(uint8_t note) {
  return 440.0f * powf(2.0f, (static_cast<int>(note) - 69) / 12.0f);
}

static void noteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (velocity == 0) {
    for (auto &v : voices) {
      if (v.active && v.channel == channel && v.note == note) v.active = false;
    }
    return;
  }

  Voice *slot = nullptr;
  for (auto &v : voices) {
    if (!v.active) {
      slot = &v;
      break;
    }
  }

  // Steal the oldest voice if all 8 slots are occupied.
  if (!slot) {
    slot = &voices[0];
    for (auto &v : voices) {
      if (v.age < slot->age) slot = &v;
    }
  }

  slot->active = true;
  slot->channel = channel;
  slot->note = note;
  slot->velocity = velocity;
  slot->phase = 0.0f;
  slot->phaseStep = TWO_PI_F * noteFrequency(note) / SAMPLE_RATE;
  slot->age = ++voiceAge;
}

static void noteOff(uint8_t channel, uint8_t note) {
  for (auto &v : voices) {
    if (v.active && v.channel == channel && v.note == note) {
      v.active = false;
    }
  }
}

static void handleMessage(uint8_t status, uint8_t d1, uint8_t d2) {
  const uint8_t kind = status & 0xF0;
  const uint8_t channel = status & 0x0F;

  if (kind == 0x90) {
    noteOn(channel, d1 & 0x7F, d2 & 0x7F);
  } else if (kind == 0x80) {
    noteOff(channel, d1 & 0x7F);
  } else if (kind == 0xB0) {
    const uint8_t cc = d1 & 0x7F;
    const uint8_t value = d2 & 0x7F;
    if (cc == 7) channelVolume[channel] = value;
  }
}

static void parseMidiByte(uint8_t b) {
  if (b & 0x80) {
    // System messages are ignored by this first version.
    if (b >= 0xF0) {
      runningStatus = 0;
      dataCount = 0;
      return;
    }

    runningStatus = b;
    const uint8_t kind = b & 0xF0;
    expectedData = (kind == 0xC0 || kind == 0xD0) ? 1 : 2;
    dataCount = 0;
    return;
  }

  if (!runningStatus || expectedData == 0) return;
  dataBytes[dataCount++] = b & 0x7F;

  if (dataCount >= expectedData) {
    const uint8_t d1 = dataBytes[0];
    const uint8_t d2 = expectedData == 2 ? dataBytes[1] : 0;
    handleMessage(runningStatus, d1, d2);
    dataCount = 0;
  }
}

static void renderAudioBlock() {
  for (size_t i = 0; i < AUDIO_BLOCK; ++i) {
    float mixed = 0.0f;
    int activeCount = 0;

    for (auto &v : voices) {
      if (!v.active) continue;
      const float velocityGain = v.velocity / 127.0f;
      const float channelGain = channelVolume[v.channel] / 127.0f;
      mixed += sinf(v.phase) * velocityGain * channelGain;
      v.phase += v.phaseStep;
      if (v.phase >= TWO_PI_F) v.phase -= TWO_PI_F;
      ++activeCount;
    }

    // Conservative normalization prevents clipping as voices accumulate.
    if (activeCount > 1) mixed /= sqrtf(static_cast<float>(activeCount));
    if (mixed > 1.0f) mixed = 1.0f;
    if (mixed < -1.0f) mixed = -1.0f;
    audioBlock[i] = static_cast<int16_t>(mixed * 10000.0f);
  }

  // playRaw queues this block to M5Unified's speaker output.
  M5.Speaker.playRaw(audioBlock, AUDIO_BLOCK, SAMPLE_RATE, false, 1);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  Serial.begin(SERIAL_BAUD);
  M5.Speaker.setVolume(96);

  for (auto &volume : channelVolume) volume = 127;

  Serial.println("PC-CardputerAdv MIDI ready");
}

void loop() {
  M5.update();

  // Drain incoming MIDI quickly so Note Off messages are not delayed.
  while (Serial.available() > 0) {
    parseMidiByte(static_cast<uint8_t>(Serial.read()));
  }

  // Keep the speaker queue supplied without blocking MIDI reception
  // for an entire audio block.
  if (!M5.Speaker.isPlaying()) {
    renderAudioBlock();
  }
}
