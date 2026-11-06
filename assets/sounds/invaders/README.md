# Space Invaders sound samples

The emulator plays each sound from a WAV file in this folder, named as below.
A missing file means that sound is silent.

| File | Sound | Port bit |
|---|---|---|
| `0.wav` | UFO (loops while flying) | OUT 3 bit 0 |
| `1.wav` | Shot | OUT 3 bit 1 |
| `2.wav` | Player death | OUT 3 bit 2 |
| `3.wav` | Invader death | OUT 3 bit 3 |
| `4.wav` | Fleet movement 1 | OUT 5 bit 0 |
| `5.wav` | Fleet movement 2 | OUT 5 bit 1 |
| `6.wav` | Fleet movement 3 | OUT 5 bit 2 |
| `7.wav` | Fleet movement 4 | OUT 5 bit 3 |
| `8.wav` | UFO hit | OUT 5 bit 4 |
| `9.wav` | Extra life | OUT 3 bit 4 |

Any WAV format works. The folder is read relative to the directory the
emulator runs from. Git ignores everything here except this README.
