# TINY TETRIS VS

### The Ultimate 4-Player Battle Tetris for MSX1

---

```
  ████████ ██ ██    ██ ██    ██
     ██    ██ ███   ██  ██  ██
     ██    ██ ██ ██ ██   ████
     ██    ██ ██  ████    ██
     ██    ██ ██    ██    ██

  ████████ ██████ ████████ ████████  ██  ██████
     ██    ██        ██    ██    ██  ██  ██
     ██    ████      ██    ████████  ██  ██████
     ██    ██        ██    ██  ██    ██      ██
     ██    ██████    ██    ██   ██   ██  ██████

          ██    ██  ██████
          ██    ██  ██
           ██  ██   ██████
            ████        ██
             ██    ██████
```

> *"In the land of 3.58 MHz, where the Z80 reigns supreme and VRAM is measured in precious kilobytes, four warriors enter the arena. Only one will survive."*

---

## WHAT IS THIS

**Tiny Tetris VS** is a 4-player competitive Tetris deathmatch that runs on the original **MSX1** — the humblest of 8-bit home computers. Four players. Four boards. One screen. No mercy.

This is not your grandmother's Tetris. This is **war**.

Clear lines. Send garbage. Bury your opponents alive under mountains of blocks. The last player standing wins. It's simple. It's brutal. It's running on hardware from 1983.

## THE RULES OF ENGAGEMENT

Each player controls their own 8-column Tetris board. The classic rules apply — pieces fall, you rotate and place them, complete lines disappear. But here's where it gets nasty:

### TARGETING SYSTEM

Every player has a **targeting cursor** — a colored arrow sprite hovering above one of the other three players' boards. That's your victim. Your chosen enemy. The poor soul who's about to have a very bad day.

Press **the action button** to cycle your target between opponents. Choose wisely. Or don't. Chaos is also a valid strategy.

### GARBAGE WARFARE

When you clear lines, you don't just score points — you **attack**:

| Lines Cleared | Garbage Sent |
|:---:|:---:|
| 1 | 0 (you get nothing, be better) |
| 2 | 1 row of garbage |
| 3 | 2 rows of garbage |
| 4 | **4 rows of pure devastation** |

Garbage rows slam into the **bottom** of your target's board, pushing everything upward. Each garbage row is filled with gray blocks except for one random gap. When the blocks reach the top — **GAME OVER**.

### LAST ONE STANDING

There is no time limit. There is no final score that matters. There is only survival. When three players are dead, the last one alive is the **champion**. Period.

## REQUIREMENTS

| Component | Specification |
|---|---|
| **Computer** | MSX1 (or higher) |
| **RAM** | 16 KB |
| **ROM** | 16 KB cartridge |
| **Video** | TMS9918A (Screen 2 / GRAPHIC2) |
| **Sound** | AY-3-8910 PSG |
| **Multiplayer** | **Ninja Tap** adapter (required for 2-4 players) |

### ABOUT THE NINJA TAP

The **Ninja Tap** is a joystick multiplexer that connects to the MSX joystick port and allows up to **4 controllers on a single port**. It was originally designed by the Gigamix team for the MSX scene and remains the standard for multiplayer MSX gaming.

**You NEED a Ninja Tap to play with more than one human player.** Without it, Player 1 uses the keyboard and Players 2-4 are controlled by AI.

Compatible Ninja Tap adapters:
- Original Ninja Tap (Gigamix)
- MSXgl-compatible Ninja Tap clones
- openMSX emulator with Ninja Tap emulation (`-command "plug joyporta ninjatap"`)

## CONTROLS

### Player 1 — Keyboard

| Key | Action |
|---|---|
| **←** **→** | Move piece left / right |
| **↑** | Rotate piece |
| **↓** | Soft drop (hold for fast fall) |
| **SPACE** | Cycle target |
| **ESC** | Quit |

### Players 1-4 — Ninja Tap Joystick

| Input | Action |
|---|---|
| **Left / Right** | Move piece |
| **Up** | Rotate piece |
| **Down** | Soft drop (hold for fast fall) |
| **Button A** | Cycle target |

## TECHNICAL INSANITY

This entire game fits in **16 KB of ROM**. That's 16,384 bytes. That's less than a single favicon on most websites. Here's what's packed in there:

- **4 simultaneous Tetris boards** with full game logic
- **7 tetromino pieces** with rotation states, packed as bitmasks in 16-bit words
- **AI opponents** with heuristic board evaluation (column heights, holes, bumpiness, line completion)
- **Dirty rendering engine** — only VRAM cells that actually changed get written. Typical frame: 0-8 writes when a piece moves, zero when nothing changes
- **Line clear flash animation** with board collapse
- **Korobeiniki** (the Tetris theme) playing on PSG channels A and B
- **Sprite-based targeting system** with 4 hardware sprites
- **Garbage warfare** mechanic with board-push animation
- **Soft drop** with auto-repeat on horizontal movement
- **NinjaTap driver** supporting both MSXgl and Gigamix protocols

The Z80 runs at 3.58 MHz. The VDP has 16 KB of VRAM. The entire playfield is 32×24 tiles in Screen 2 mode with identity-mapped name tables for per-cell pattern and color control. Every byte counts. Every cycle matters. And somehow, it all fits.

## BUILD FROM SOURCE

### Prerequisites

- [MSXgl](https://github.com/aoineko-fr/MSXgl) v1.2.17
- SDCC compiler (included with MSXgl)
- Windows (build script is a batch file)

### Building

1. Edit `build.bat` and set the `MSXGL` path to your MSXgl installation
2. Run:

```batch
build.bat
```

The script syncs sources to the MSXgl project tree, compiles, and copies the versioned ROM back. Each successful build auto-increments the version number.

### Running in openMSX

```bash
openmsx -machine C-BIOS_MSX1_EU -cart tinitetris4p.rom -command "plug joyporta ninjatap"
```

## THE STORY

*The year is 1985. Four players sit around a 14-inch TV connected to an MSX computer. A Ninja Tap adapter sprouts four joystick cables like a mechanical octopus. The Korobeiniki melody fills the room.*

*Blocks fall. Lines clear. Garbage rises.*

*Friendships will be tested. Alliances will be broken. Someone will accuse someone else of screen-looking, which is absurd because everyone is on the same screen. That's the whole point.*

*When the dust settles and three boards flash GAME OVER in dark red, one player will stand. One player will know the truth:*

***They are the best Tetris player in the room.***

---

## CREDITS

- **Code & Design** — antxiko
- **AI Assistant** — Claude (Anthropic)
- **Music** — Korobeiniki (Russian folk song, 1861)
- **Library** — [MSXgl](https://github.com/aoineko-fr/MSXgl) by Aoineko
- **Hardware** — Ninja Tap by Gigamix

---

*Tiny Tetris VS — Because 16 KB is all you need to destroy friendships.*
