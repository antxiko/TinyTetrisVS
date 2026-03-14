// ____________________________________________________________________________
// Simple PSG music player — Korobeiniki (Tetris theme)
// AY-3-8910: Channel A = melody, Channel B = bass
// ____________________________________________________________________________

#include "msxgl.h"
#include "psg.h"
#include "music.h"

// ============================================================================
// Note periods for AY-3-8910 (PAL clock ~1.7734 MHz)
// Period = 1773400 / (16 * frequency)
// ============================================================================
#define REST  0

// Octave 3
#define A3    504
#define Bb3   476
#define B3    449

// Octave 4
#define C4    424
#define Cs4   400
#define D4    378
#define Ds4   357
#define E4    337
#define F4    318
#define Fs4   300
#define G4    283
#define Gs4   267
#define A4    252
#define Bb4   238
#define B4    224

// Octave 5
#define C5    212

// Bass octave 2
#define A2    1008
#define B2    898
#define C3    848
#define D3    756
#define E3    674
#define F3    636
#define G3    566
#define Gs3   534

// ============================================================================
// Duration in frames (50Hz). Tempo ~150 BPM → quarter = 20 frames
// ============================================================================
#define T8   10   // eighth
#define T4   20   // quarter
#define T4D  30   // dotted quarter
#define T2   40   // half

// ============================================================================
// Song data: {period, duration} pairs. 0xFFFF marks loop point.
// ============================================================================
typedef struct {
    u16 period;
    u8  dur;
} NoteEntry;

// Melody (Channel A)
static const NoteEntry g_Melody[] = {
    // Section A (8 measures)
    // |E4   B3 C4 |D4   C4 B3 |A3   A3 C4 |E4   D4 C4 |
    {E4,T4}, {B3,T8}, {C4,T8}, {D4,T4}, {C4,T8}, {B3,T8},
    {A3,T4}, {A3,T8}, {C4,T8}, {E4,T4}, {D4,T8}, {C4,T8},
    // |B3.    C4 |D4   E4   |C4   A3   |A3   --   |
    {B3,T4D}, {C4,T8}, {D4,T4}, {E4,T4},
    {C4,T4}, {A3,T4}, {A3,T4}, {REST,T4},

    // Section B (8 measures)
    // |-- D4.   F4 |A4   G4 F4 |E4.    C4 |E4   D4 C4 |
    {REST,T8}, {D4,T4D}, {F4,T8}, {A4,T4}, {G4,T8}, {F4,T8},
    {E4,T4D}, {C4,T8}, {E4,T4}, {D4,T8}, {C4,T8},
    // |B3.    C4 |D4   E4   |C4   A3   |A3   --   |
    {B3,T4D}, {C4,T8}, {D4,T4}, {E4,T4},
    {C4,T4}, {A3,T4}, {A3,T4}, {REST,T4},

    // Ending chord held
    {E4,T2}, {E4,T2},
    {C4,T2}, {C4,T2},
    {A3,T2}, {A3,T2}, {REST,T4}, {REST,T4},
};

// Bass (Channel B) — root notes, half notes mostly
static const NoteEntry g_Bass[] = {
    // Section A
    {E3,T4}, {E3,T4}, {E3,T4}, {E3,T4},    // E minor
    {A2,T4}, {A2,T4}, {A2,T4}, {A2,T4},    // A minor
    {Gs3,T4}, {Gs3,T4}, {E3,T4}, {E3,T4},  // G#/E
    {A2,T4}, {A2,T4}, {A2,T4}, {REST,T4},

    // Section B
    {D3,T4}, {D3,T4}, {D3,T4}, {D3,T4},    // D minor
    {C3,T4}, {C3,T4}, {C3,T4}, {C3,T4},    // C
    {Gs3,T4}, {Gs3,T4}, {E3,T4}, {E3,T4},  // G#/E
    {A2,T4}, {A2,T4}, {A2,T4}, {REST,T4},

    // Ending
    {A2,T2}, {A2,T2},
    {A2,T2}, {A2,T2},
    {A2,T2}, {A2,T2}, {REST,T4}, {REST,T4},
};

#define MELODY_LEN  (sizeof(g_Melody) / sizeof(NoteEntry))
#define BASS_LEN    (sizeof(g_Bass) / sizeof(NoteEntry))

static u8 s_melIdx;
static u8 s_basIdx;
static u8 s_melTimer;
static u8 s_basTimer;

// ============================================================================
// INIT
// ============================================================================

void Music_Init(void) {
    // Mixer: tone A+B on, noise off on all channels
    // Bits: noise_C noise_B noise_A tone_C tone_B tone_A (active LOW)
    PSG_SetRegister(7, 0x3C);  // 00111100 = tone A+B enabled

    // Volumes
    PSG_SetRegister(8, 12);    // Channel A (melody) volume
    PSG_SetRegister(9, 9);     // Channel B (bass) volume
    PSG_SetRegister(10, 0);    // Channel C silent

    s_melIdx = 0;
    s_basIdx = 0;
    s_melTimer = 1;  // Trigger immediately
    s_basTimer = 1;
}

// ============================================================================
// UPDATE — call every frame
// ============================================================================

void Music_Update(void) {
    // Melody channel A
    s_melTimer--;
    if (s_melTimer == 0) {
        u16 p = g_Melody[s_melIdx].period;
        s_melTimer = g_Melody[s_melIdx].dur;
        s_melIdx++;
        if (s_melIdx >= MELODY_LEN) s_melIdx = 0;

        if (p > 0) {
            PSG_SetRegister(0, (u8)(p & 0xFF));
            PSG_SetRegister(1, (u8)((p >> 8) & 0x0F));
            PSG_SetRegister(8, 12);
        } else {
            PSG_SetRegister(8, 0);
        }
    }

    // Bass channel B
    s_basTimer--;
    if (s_basTimer == 0) {
        u16 p = g_Bass[s_basIdx].period;
        s_basTimer = g_Bass[s_basIdx].dur;
        s_basIdx++;
        if (s_basIdx >= BASS_LEN) s_basIdx = 0;

        if (p > 0) {
            PSG_SetRegister(2, (u8)(p & 0xFF));
            PSG_SetRegister(3, (u8)((p >> 8) & 0x0F));
            PSG_SetRegister(9, 9);
        } else {
            PSG_SetRegister(9, 0);
        }
    }
}
