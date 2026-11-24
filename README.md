# PC ↔ Cardputer-Adv MIDI Player

PCからUSBシリアル経由でMIDIイベントを送り、M5Stack Cardputer-Advでリアルタイムに発音するプロジェクトです。

## 現在の実装

- USB CDCシリアルでMIDIバイト列を受信
- MIDI Note On / Note Off
- 8ボイスの同時発音
- サイン波オシレーター
- NoteごとのVelocity
- チャンネルごとのCC7 (Channel Volume)
- MIDIファイル（SMF）をPC側で再生して送信

> 初期実装です。実機での音量・レイテンシ・長時間動作は、ビルドおよび接続後に確認してください。

## 構成

```
PC (Python / mido)
   │ USBケーブル
   │ USB CDC Serial
   ▼
Cardputer-Adv (ESP32-S3)
   │ MIDI parser
   ▼
8-voice sine synth
   │ M5Unified Speaker / I2S audio
   ▼
内蔵スピーカー
```

## PC側

Python 3.10以降を推奨。

```bash
python -m pip install -r pc/requirements.txt
```

接続ポートを確認してMIDIファイルを再生：

```bash
python pc/play_midi.py --port COM5 path/to/song.mid
```

macOS / Linuxではポート名を `/dev/ttyACM0` などに変更してください。ポート名は環境によって異なります。

## Cardputer-Adv側

PlatformIOで `firmware/` を開き、USB CDCを有効にしてビルド・書き込みます。

```bash
cd firmware
pio run -t upload
pio device monitor
```

PlatformIOのボード定義やM5Unifiedの対応状況は、使用する環境に合わせて調整してください。

## MIDI音量

- Velocity: Noteごとの音量
- CC7: MIDIチャンネルごとの音量
- 最終的なボイス音量はVelocity × CC7で計算

## 将来の拡張：PC音声との同時再生

将来的には、PCでWAVなどをデコードし、MIDIイベントとPCM音声を同じUSB接続でCardputer-Advへ送る構成を想定しています。

その段階では、単純なMIDIバイト列の垂れ流しではなく、メッセージ種別・長さ・シーケンス番号を持つフレーム形式へ移行します。Cardputer-Adv側ではMIDIシンセ音とPCMストリームを同じミキサーで合成して出力します。

WAV対応はまだ未実装です。USB転送帯域、バッファ容量、サンプルレート、アンダーラン対策を測定してから実装します。

## License

MIT
