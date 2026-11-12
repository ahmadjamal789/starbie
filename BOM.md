# Starbie ⭐

A tiny motion-controlled desktop pet, like a mini Tamagotchi. Starbie lives on a custom PCB with a small OLED screen for its face, two RGB LEDs that show its mood, and a motion sensor that wakes it up when you shake or move it.

Made for the **Hack Club Half-Life Challenge**.

<img width="1154" height="647" alt="Screenshot from 2026-10-11 10-07-18" src="https://github.com/user-attachments/assets/ef6f2d65-2654-4bc9-9203-1638e4dfaa12" />


## What it does

- **Shake to wake:** the MPU6050 motion sensor detects movement and wakes Starbie up.
- **Sleeps when idle:** after a while with no motion, Starbie goes to sleep and shows a sleeping face.
- **Shows emotions:** the OLED shows faces with eyes and a mouth (happy, sad, sleepy and more).
- **Mood lights:** two RGB LEDs change colour with its emotion, for example red when angry and green when something good happens.
- **Two buttons:** used to interact with Starbie.
- **Planned:** a sleepy yawn before it falls asleep, extra faces, and a hunger/boredom meter.

## Hardware

### Bill of materials

| Part | Reference | Purpose | Qty | Unit price (USD) | Buy link |
|---|---|---|---|---|---|
| Seeed Studio XIAO ESP32-C3 | U1 | Microcontroller | 1 | $4.97 | [link](https://example.com) |
| MPU6050 | U2 | Motion sensing (shake to wake) | 1 | $3.69 | [link](https://example.com) |
| 0.96" 128x64 I2C OLED | U3 | Shows Starbie's face | 1 | $8.59 | [link](https://example.com) |
| Push button | SW1, SW2 | User input | 2 | $0.94 | [link](https://example.com) |
| RGB LED (ARGB) | D1, D2 | Mood lights | 2 | $0.72 | [link](https://example.com) |
| **Total** | | | | **$20.44** | |


Full parts list: see [`bom.csv`](bom.csv).

## Design files

Everything was designed in **KiCad**.

| File | What it is |
|---|---|
| `starbie.kicad_sch` | Schematic |
| `starbie.kicad_pcb` | PCB layout |
| `starbie.kicad_pro` | KiCad project file |

### Schematic

<img width="985" height="704" alt="Screenshot from 2026-10-09 10-24-23" src="https://github.com/user-attachments/assets/08ab8bf5-91a5-4249-83c6-57c445670290" />


### PCB layout

<img width="1155" height="654" alt="Screenshot from 2026-10-11 10-14-03" src="https://github.com/user-attachments/assets/195716f1-0866-4228-b4e6-4e02243737e9" />


### 3D view

<img width="1154" height="647" alt="Screenshot from 2026-10-11 10-07-18" src="https://github.com/user-attachments/assets/ef6f2d65-2654-4bc9-9203-1638e4dfaa12" />


## How it works

1. The XIAO ESP32-C3 reads motion data from the MPU6050 over I2C.
2. A shake wakes Starbie. If there is no motion for a set time, it goes to sleep.
3. The OLED (also on I2C) draws a face for the current mood.
4. The two LEDs change colour to match the mood.


## Build it yourself

1. Open `starbie.kicad_pro` in KiCad.
2. Order the PCB using the fabrication files (Gerbers) if included, and buy the parts in the BOM.
3. Solder the components onto the board.
4. Flash the firmware to the XIAO ESP32-C3.

## Project log

See [`JOURNAL.md`](JOURNAL.md) for my build journal and time tracking.

## What I learned

- Designing a schematic and PCB layout in KiCad
- Using AI models as a build partner
- Using Git and GitHub to version my hardware project

## Status

- [x] Schematic
- [ ] PCB routing (finishing the last connections)
- [x] BOM
- [x] Firmware (see the `firmware/` folder)
- [ ] Built and tested

## License

[Add a license, for example MIT.]
