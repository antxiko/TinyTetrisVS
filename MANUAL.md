# TINY TETRIS VS — FIELD MANUAL

### Classified Document — For Combatants Only

---

```
    ╔══════════════════════════════════════════╗
    ║  W A R N I N G                           ║
    ║                                          ║
    ║  This cartridge contains a 4-player      ║
    ║  competitive combat simulation.          ║
    ║                                          ║
    ║  Side effects may include:               ║
    ║  - Elevated heart rate                   ║
    ║  - Sudden outbursts of profanity         ║
    ║  - Permanent damage to friendships       ║
    ║  - An uncontrollable urge to play again  ║
    ║                                          ║
    ║  YOU HAVE BEEN WARNED.                   ║
    ╚══════════════════════════════════════════╝
```

---

## I. MISSION BRIEFING

Four players. Four boards. One screen. Blocks fall from the sky. You place them. You complete lines. Lines disappear.

So far, so Tetris.

But this is not regular Tetris. In **Tiny Tetris VS**, every line you clear is a weapon. You choose a target. You pull the trigger. Garbage blocks slam into their board from below, pushing them closer to death.

When the blocks reach the top of your board, you die.

**The last player alive wins. There are no draws. There is no mercy.**

---

## II. EQUIPMENT REQUIRED

You need exactly three things:

```
┌─────────────────────────────────────────────┐
│  1. AN MSX COMPUTER                         │
│     Any MSX1 or higher. 16 KB RAM.          │
│     The humblest machine will do.           │
│     Great wars have been fought with less.  │
│                                             │
│  2. THIS CARTRIDGE                          │
│     32 KB ROM. Slot it in. Power on.        │
│     The battlefield loads instantly.        │
│                                             │
│  3. A NINJA TAP (for 3-4 humans)            │
│     Plug it into joystick port A.           │
│     Connect up to 4 joysticks.              │
│     Press F1 on the title screen to enable  │
│     NinjaTap mode for all four players.     │
│     Without it: 2 humans on keyboard,       │
│     up to 2 more on standard joysticks.     │
└─────────────────────────────────────────────┘
```

---

## III. THE TITLE SCREEN

When the cartridge boots, you are greeted by the **TINY TETRIS VS** logo — built entirely out of Tetris blocks, glowing in four player colors across the darkness.

Below the logo, four player slots await:

```
  ┌────────┬────────┬────────┬────────┐
  │   P1   │   P2   │   P3   │   P4   │
  │PRESS A │PRESS A │PRESS A │PRESS A │
  └────────┴────────┴────────┴────────┘
```

Each player who wants to fight must press their button to claim their slot.

**KB+JOY mode** (default — bottom of screen reads `F1: KEY P1P2 P3P4 JOY`):
- **Player 1**: Press **P** on the keyboard, or **1**
- **Player 2**: Press **V** on the keyboard, or **2**
- **Player 3**: Press **BUTTON A** on the joystick in port 1, or **3**
- **Player 4**: Press **BUTTON A** on the joystick in port 2, or **4**

**NINJATAP mode** (toggle with **F1** — bottom reads `F1: NTAP`):
- **Players 1–4**: Press **BUTTON A** on the corresponding NinjaTap port (1–4), or use the keyboard number key

When a player joins, their slot changes to **READY** in their player color.

### HUMANS VS MACHINES

You don't need four humans. You don't even need two.

The moment the first player joins, an **8-second countdown** begins. When it expires — or when all four slots are claimed — the game starts. **Any unclaimed slot is filled by a CPU opponent**:

```
  ┌────────┬────────┬────────┬────────┐
  │   P1   │   P2   │   P3   │   P4   │
  │ READY  │ READY  │  CPU   │  CPU   │
  └────────┴────────┴────────┴────────┘
```

The CPU is no slouch. It evaluates every possible landing, aims for 2-line clears by default, occasionally sets up for 3-liners, and on rare bloodthirsty pieces hunts for a full **Tetris**. It will target you. It will send you garbage. It will laugh in Z80 assembly.

*A different melody plays on the title screen. Enjoy it. It's the last peaceful moment you'll have.*

---

## IV. CONTROLS

### KEYBOARD WARRIOR — PLAYER 1 (KB+JOY mode)

```
                    ┌───┐
     ROTATE ───────>│ ↑ │
                    └───┘
          ┌───┐     ┌───┐     ┌───┐
  MOVE <──│ ← │     │ ↓ │     │ → │──> MOVE
          └───┘     └───┘     └───┘
                      │
                  SOFT DROP
                (hold to fall fast)

                ┌───┐
                │ P │──> CHOOSE YOUR VICTIM
                └───┘
```

### KEYBOARD WARRIOR — PLAYER 2 (KB+JOY mode)

```
                    ┌───┐
     ROTATE ───────>│ W │
                    └───┘
          ┌───┐     ┌───┐     ┌───┐
  MOVE <──│ A │     │ S │     │ D │──> MOVE
          └───┘     └───┘     └───┘
                      │
                  SOFT DROP

                ┌───┐
                │ V │──> CHOOSE YOUR VICTIM
                └───┘
```

### JOYSTICK SOLDIER

In **KB+JOY** mode, players 3 and 4 use the standard joysticks plugged into ports 1 and 2.

In **NINJATAP** mode, all four players use joysticks attached to the NinjaTap on port A.

```
                    ┌───┐
     ROTATE ───────>│ ▲ │
                    └───┘
          ┌───┐               ┌───┐
  MOVE <──│ ◄ │               │ ► │──> MOVE
          └───┘               └───┘
                    ┌───┐
  SOFT DROP ───────>│ ▼ │
                    └───┘
                  (hold to fall fast)

               ┌──────────┐
               │ BUTTON A │──> CHOOSE YOUR VICTIM
               └──────────┘
```

### ESC — TACTICAL RETREAT

Press **ESC** during a battle to abandon the match and return to the title screen. On the title itself, ESC does nothing — there is nowhere left to retreat to from a cartridge.

---

## IV. THE BATTLEFIELD

The screen is divided into four vertical strips. Each strip is one player's domain:

```
  ┌────────┬────────┬────────┬────────┐
  │   P1   │   P2   │   P3   │   P4   │
  │  CYAN  │  RED   │ GREEN  │ YELLOW │
  ├────────┼────────┼────────┼────────┤
  │ Score  │ Score  │ Score  │ Score  │
  │ Lines  │ Lines  │ Lines  │ Lines  │
  ├════════┼════════┼════════┼════════┤
  │        │        │        │        │
  │        │        │        │        │
  │  YOUR  │        │        │        │
  │ BOARD  │ THEIR  │ THEIR  │ THEIR  │
  │        │ BOARD  │ BOARD  │ BOARD  │
  │ 8 cols │        │        │        │
  │ 20 rows│        │        │        │
  │        │        │        │        │
  └────────┴────────┴────────┴────────┘
```

The **colored arrows** floating above the separator line are the targeting cursors. Your arrow shows which opponent you are currently aiming at. Other players' arrows show who *they* are targeting. If you see three arrows pointing at your board — pray.

### THE HUD — FOUR ROWS OF INTEL

Each player's top four rows display critical battlefield data:

```
  ┌────────────────────┐
  │ P1 · · · ■■■■      │  <- Row 0: player label + next-piece preview (top)
  │ 0042       ■ ■     │  <- Row 1: score + next-piece preview (bottom)
  │ L012    V 3        │  <- Row 2: lines cleared + current level
  │ ════════════════   │  <- Row 3: separator bar in player color
  └────────────────────┘
```

- **Next-piece preview**: the 4×2 block in the top-right shows what piece is coming after the current one. Plan your stack accordingly.
- **Score**: 4 digits. Accumulates per line cleared.
- **Lines**: running count of total lines you've cleared. Every 10 lines = level up.
- **Level**: a single digit. Higher level = faster natural drop = less thinking time.

### THE GHOST

When you rotate or move your piece, an **outlined ghost** appears at the position where the piece would land if you dropped right now. This is your landing forecast. Use it. The ghost shows only for human players — the CPUs fly blind and they still beat you, so don't get cocky.

---

## V. HOW TO KILL

### Step 1: Play Tetris
Place blocks. Complete horizontal lines. This is the easy part.

### Step 2: Aim
Press **SPACE** (keyboard) or **BUTTON A** (joystick) to cycle your targeting arrow between the other three players. Pick someone. Preferably someone who's doing well. Nobody likes a winner.

### Step 3: Clear Multiple Lines
Single lines are for survival. If you want to **attack**, you need combos:

```
  ╔═══════════════╦══════════════════════════════╗
  ║  LINES        ║  DAMAGE                      ║
  ╠═══════════════╬══════════════════════════════╣
  ║  1 line       ║  Nothing. Pathetic.          ║
  ║  2 lines      ║  1 garbage row               ║
  ║  3 lines      ║  2 garbage rows              ║
  ║  4 lines      ║  4 GARBAGE ROWS OF DOOM      ║
  ╚═══════════════╩══════════════════════════════╝
```

A **4-line clear** (the legendary Tetris) sends FOUR rows of garbage. That's 20% of the entire board. In one move. It's devastating. It's beautiful. It's why you play this game.

### Step 3.5: THE T-SPIN

If you rotate a **T-piece** into a tight slot where 3 of its 4 diagonal corners are occupied — walls, blocks, floor — that's a **T-spin**. The game knows. The game rewards you.

T-spin garbage is **doubled**: a T-spin single sends 2 rows. A T-spin double sends 4. A T-spin triple sends 6. This is the deadliest move in the game. Learn it. Master it. Fear the players who already have.

### Step 3.75: COMBOS

Clear lines on **consecutive locks** and the combo counter climbs. Each combo level beyond the first adds **+1 garbage row** on top of the base damage. Chain three clears in a row and you're sending devastating waves. Miss one lock without a clear and the combo resets to zero.

### Step 4: Watch Them Suffer
Garbage rows rise from the **bottom** of your target's board — **one row at a time**, climbing like a slow, inevitable tide. Everything they've built gets shoved upward. Their falling piece rides the wave up. Each garbage row is solid gray blocks with one random gap — just enough hope to keep them playing, not enough to save them.

### Step 5: Be the Last One Standing
When a player's blocks reach the top, their board turns **dark red** and displays:

```
        ┌────────┐
        │  GAME  │
        │  OVER  │
        └────────┘
```

They're done. Gone. Eliminated. Three must fall for one to rise.

### Step 6: Victory

The moment the third player falls, the game stops. The screen floods with the winner's color. Two words appear:

```
        ╔════════════════════╗
        ║                    ║
        ║    PLAYER X WINS   ║
        ║                    ║
        ╚════════════════════╝
```

A **victory fanfare** erupts from the PSG. Trumpets of triumph. Horns of glory. Well, square waves of glory — but on a Z80, that's the same thing.

After the celebration, the **STATS** screen appears:

```
           STATS

  P1     P2     P3     P4
  SCO    SCO    SCO    SCO
  LIN    LIN    LIN    LIN
  GRB    GRB    GRB    GRB
  TSP    TSP    TSP    TSP
  CMB    CMB    CMB    CMB
  LVL    LVL    LVL    LVL
```

Every player's final score, lines cleared, garbage sent, T-spins landed, longest combo, and level reached — all displayed in their player color. The winner gets a shining **WIN** label. Study these numbers. Learn from them. Use them to trash-talk your friends.

Press any key or wait 15 seconds. The game returns to the title screen. **Go again.**

### ATTRACT MODE

Nobody home? No problem. Leave the title screen alone for 15 seconds and the machine takes over — **4 CPUs play a demo match**. Watch, learn, be humbled. Press any key to return to the title and join the fight yourself.

---

## VI. ADVANCED TACTICS

**THE SNIPER** — Target the player closest to death. Efficient. Cold. Effective.

**THE DIPLOMAT** — Target whoever is winning. Keep the balance. Let others do your dirty work. Strike at the end.

**THE MANIAC** — Switch targets constantly. Keep everyone guessing. Spread the chaos evenly. Nobody is safe.

**THE FARMER** — Ignore targeting entirely. Stack perfectly. Build four-line setups. When you strike, strike to kill.

**THE SOFT DROP** — Hold DOWN to make your piece fall faster. Essential for speed. Essential for survival. If you're not soft-dropping, you're losing.

---

## VII. KNOW YOUR PIECES

```
  ████      ██       █        ██        █       █       █
            ██      ███       ██      ███     ███     ███
                                █       █     █
    I        O       T        S        Z       L       J
```

Seven pieces. Four rotations each (except O and I). You don't choose them. They choose you. Adapt or die.

---

## VIII. THE MUSIC

The **Korobeiniki** — a Russian folk song from 1861 — plays endlessly through the AY-3-8910 PSG chip. It is the soundtrack of your triumph or your destruction. After enough games, you will hear it in your sleep. This is normal. This is the price of greatness.

---

## IX. FINAL WORDS

```
╔═══════════════════════════════════════════════╗
║                                               ║
║  Tiny Tetris VS fits in 32 KB.                ║
║                                               ║
║  That's 32,768 bytes.                         ║
║  Less than a single web font.                 ║
║  Less than a modern app's splash screen.      ║
║  Less than the JavaScript to render a button. ║
║                                               ║
║  And yet it contains:                         ║
║  - 4 complete Tetris games running at once    ║
║  - AI that will humble you                    ║
║  - Music that will haunt you                  ║
║  - A combat system that will ruin friendships ║
║                                               ║
║  All on a CPU from 1976                       ║
║  running at 3.58 MHz                          ║
║  with 16 KB of VRAM                           ║
║  on a computer from 1983 that runs on         ║
║  less power than your phone's screen dimmer.  ║
║                                               ║
║  Some things don't need gigabytes.            ║
║  Some things don't need gigahertz.            ║
║  Some things just need four players,          ║
║  a Ninja Tap, and the will to win.            ║
║                                               ║
║  Now plug in. Power on. Drop blocks.          ║
║                                               ║
║  AND SHOW NO MERCY.                           ║
║                                               ║
╚═══════════════════════════════════════════════╝
```

---

*Tiny Tetris VS v1.1 — 32 KB of friendship destruction since 2026.*
