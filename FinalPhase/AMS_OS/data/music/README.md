# 🎵 AMS OS Music Library

Place your audio files (`.wav`, `.mp3`, `.ogg`) in this folder.

The Music Player will automatically scan this directory and add any audio files
to the playlist alongside the built-in demo tracks.

## Supported Formats
- `.wav` — Uncompressed audio (best compatibility)
- `.mp3` — Compressed audio (requires mpg123 or ffplay)
- `.ogg` — Ogg Vorbis (requires ogg123 or ffplay)

## Playback
Audio is played using system audio tools (`aplay` for WAV, `ffplay`/`mpg123` for MP3/OGG).
Install one of these if audio playback doesn't work:

```bash
sudo apt install alsa-utils    # for aplay (WAV)
sudo apt install ffmpeg         # for ffplay (all formats)
sudo apt install mpg123         # for mpg123 (MP3)
```

## File Naming
The filename (without extension) becomes the song title in the playlist.
For example: `My Cool Song.wav` → displays as "My Cool Song".
