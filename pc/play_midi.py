#!/usr/bin/env python3
"""Play a Standard MIDI File by forwarding MIDI events to Cardputer-Adv."""

import argparse
import time

import mido
import serial


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True, help="Serial port, e.g. COM5 or /dev/ttyACM0")
    parser.add_argument("midi_file", help="Path to .mid / .midi file")
    parser.add_argument("--baud", type=int, default=115200)
    args = parser.parse_args()

    midi = mido.MidiFile(args.midi_file)

    with serial.Serial(args.port, args.baud, timeout=0) as device:
        # Opening a USB serial port may reset the ESP32. Give it time to boot.
        time.sleep(2.0)
        device.reset_input_buffer()

        try:
            for message in midi.play():
                if message.is_meta:
                    continue
                payload = message.bytes()
                device.write(bytes(payload))
        except KeyboardInterrupt:
            # Stop any notes that might still be held.
            for channel in range(16):
                device.write(bytes([0xB0 | channel, 123, 0]))
            print("\nPlayback stopped.")

    print("Playback finished.")


if __name__ == "__main__":
    main()
