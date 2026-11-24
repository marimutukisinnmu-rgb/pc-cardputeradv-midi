# Audio roadmap: MIDI + PC audio

## Goal

将来、PCでWAVなどの音声を再生しながら、MIDIノートをCardputer-Advで発音できるようにする。

## 推奨構成

PC側でWAV/MP3などをデコードしてPCM化し、MIDIイベントとPCMチャンクをUSB CDCで送る。Cardputer-Adv側では次の2系統を同じミキサーへ入力する。

- MIDI synth: 現在のサイン波ボイス
- PCM stream: PCから届いた音声サンプル

ミキサーでゲイン調整・クリッピング防止を行い、共通のサンプルレートでI2S出力する。

## なぜ音声ファイルをそのまま送らない？

WAVをそのまま転送するより、PCでデコードしてPCMチャンクとして送るほうが、デバイス側のデコーダ実装を減らし、MIDI音との同期・ミックスを単純化できる。圧縮音声を送る方式は、帯域やデコード負荷を測定してから検討する。

## 次のプロトコル案

MIDIの生バイト列とPCMを混在させるため、次のようなフレームを導入する。

| Field | Purpose |
|---|---|
| Magic | フレーム識別 |
| Type | MIDI event / PCM block / control |
| Sequence | 欠落・順序確認 |
| Timestamp | 再生タイミング |
| Length | ペイロード長 |
| Payload | MIDI bytes または PCM samples |

## 実装順

1. 現行MIDI経路の安定化（Note On/Off、Velocity、CC7、複数チャンネル）
2. USB CDC転送速度と安定性の測定
3. PCMリングバッファとアンダーラン検出
4. MIDI synth + PCM mixer
5. PC側WAVデコードとタイムスタンプ付き送信
6. 必要なら圧縮音声や複数ストリームを検討

PCMは44.1 kHz / 16-bit / monoでも約88.2 kB/s、stereoなら約176.4 kB/sのペイロードになる。USB CDCの実効速度とバッファ設計を確認し、UARTピン経由の低速シリアルと混同しないこと。
