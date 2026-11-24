# PC ↔ Cardputer-Adv MIDI Player

PCからUSB CDCシリアル経由でMIDIイベントを送り、M5Stack Cardputer-Advでリアルタイムに発音するプロジェクトです。

## Firmware (PlatformIO)

このリポジトリのルートがPlatformIOプロジェクトです。

```powershell
pio run
pio run -t upload
pio device monitor
```

- 設定: `platformio.ini`
- ソース: `src/main.cpp`

## 現在のファームウェア機能

- USB CDCシリアルでMIDIバイト列を受信
- MIDI Note On / Note Off
- 8ボイス同時発音
- サイン波オシレーター
- NoteごとのVelocity
- チャンネルごとのCC7 (Channel Volume)

> 初期実装です。実機でのビルド・音量・レイテンシ・長時間動作は未確認です。

## PC側

PC用のMIDI送信スクリプトや依存ライブラリは `pc` ブランチで管理します。

## 将来の拡張：PC音声との同時再生

将来的には、PCでWAVなどをデコードし、MIDIイベントとPCM音声を同じUSB接続でCardputer-Advへ送る構成を想定しています。

Cardputer-Adv側ではMIDIシンセ音とPCMストリームを同じミキサーで合成して出力します。WAV対応はまだ未実装です。USB転送帯域、バッファ容量、サンプルレート、アンダーラン対策を測定してから実装します。

設計メモ: [docs/audio-roadmap.md](docs/audio-roadmap.md)

## License

MIT
