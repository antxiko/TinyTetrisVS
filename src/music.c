// ____________________________________________________________________________
// PSG music player — Title, Game (Korobeiniki), Victory
// AY-3-8910: Channel A = melody, Channel B = bass
// ____________________________________________________________________________

#include "msxgl.h"
#include "psg.h"
#include "music.h"

// ============================================================================
// Note periods for AY-3-8910 (PAL clock ~1.7734 MHz)
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
#define D5    189
#define E5    168

// Bass octave 2-3
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
#define T16  5    // sixteenth
#define T8   10   // eighth
#define T4   20   // quarter
#define T4D  30   // dotted quarter
#define T2   40   // half
#define T1   80   // whole

typedef struct {
    u16 period;
    u8  dur;
} NoteEntry;

// ============================================================================
// TITLE THEME — Bouncy march in C major (original)
// ============================================================================

static const NoteEntry g_TitleMel[] = {
    // Phrase 1: ascending fanfare
    {C4,T8}, {E4,T8}, {G4,T4}, {G4,T8}, {A4,T8},
    {G4,T4}, {E4,T4},
    {C4,T8}, {D4,T8}, {E4,T4}, {E4,T8}, {F4,T8},
    {E4,T4}, {D4,T4},
    // Phrase 2: climax and resolve
    {C4,T8}, {E4,T8}, {G4,T4}, {A4,T8}, {B4,T8},
    {C5,T4}, {A4,T4},
    {G4,T8}, {F4,T8}, {E4,T8}, {D4,T8},
    {C4,T2}, {REST,T4}, {REST,T4},
};

static const NoteEntry g_TitleBas[] = {
    {C3,T4}, {C3,T4}, {E3,T4}, {E3,T4},
    {C3,T4}, {C3,T4},
    {F3,T4}, {F3,T4}, {G3,T4}, {G3,T4},
    {G3,T4}, {G3,T4},
    {C3,T4}, {C3,T4}, {F3,T4}, {F3,T4},
    {E3,T4}, {F3,T4},
    {G3,T4}, {G3,T4}, {G3,T4}, {G3,T4},
    {C3,T2}, {REST,T4}, {REST,T4},
};

// ============================================================================
// GAME THEME — Korobeiniki
// ============================================================================

static const NoteEntry g_GameMel[] = {
    // Section A
    {E4,T4}, {B3,T8}, {C4,T8}, {D4,T4}, {C4,T8}, {B3,T8},
    {A3,T4}, {A3,T8}, {C4,T8}, {E4,T4}, {D4,T8}, {C4,T8},
    {B3,T4D}, {C4,T8}, {D4,T4}, {E4,T4},
    {C4,T4}, {A3,T4}, {A3,T4}, {REST,T4},
    // Section B
    {REST,T8}, {D4,T4D}, {F4,T8}, {A4,T4}, {G4,T8}, {F4,T8},
    {E4,T4D}, {C4,T8}, {E4,T4}, {D4,T8}, {C4,T8},
    {B3,T4D}, {C4,T8}, {D4,T4}, {E4,T4},
    {C4,T4}, {A3,T4}, {A3,T4}, {REST,T4},
    // Ending
    {E4,T2}, {E4,T2},
    {C4,T2}, {C4,T2},
    {A3,T2}, {A3,T2}, {REST,T4}, {REST,T4},
};

static const NoteEntry g_GameBas[] = {
    // Section A
    {E3,T4}, {E3,T4}, {E3,T4}, {E3,T4},
    {A2,T4}, {A2,T4}, {A2,T4}, {A2,T4},
    {Gs3,T4}, {Gs3,T4}, {E3,T4}, {E3,T4},
    {A2,T4}, {A2,T4}, {A2,T4}, {REST,T4},
    // Section B
    {D3,T4}, {D3,T4}, {D3,T4}, {D3,T4},
    {C3,T4}, {C3,T4}, {C3,T4}, {C3,T4},
    {Gs3,T4}, {Gs3,T4}, {E3,T4}, {E3,T4},
    {A2,T4}, {A2,T4}, {A2,T4}, {REST,T4},
    // Ending
    {A2,T2}, {A2,T2},
    {A2,T2}, {A2,T2},
    {A2,T2}, {A2,T2}, {REST,T4}, {REST,T4},
};

// ============================================================================
// VICTORY FANFARE
// ============================================================================

static const NoteEntry g_VicMel[] = {
    {C4,T8}, {E4,T8}, {G4,T8}, {C5,T4},
    {REST,T8}, {G4,T8}, {C5,T4D},
    {REST,T8}, {E4,T8}, {G4,T8}, {A4,T4}, {G4,T8}, {E4,T8},
    {C4,T4}, {E4,T4}, {C4,T2},
    {REST,T4}, {REST,T4},
    {C4,T8}, {E4,T8}, {G4,T8}, {C5,T2},
    {G4,T4}, {E4,T4}, {C4,T2}, {C4,T2},
    {REST,T2}, {REST,T2},
};

static const NoteEntry g_VicBas[] = {
    {C3,T4}, {C3,T4}, {E3,T4}, {C3,T4},
    {G3,T4}, {G3,T4}, {C3,T4D},
    {REST,T8}, {C3,T4}, {C3,T4}, {F3,T4}, {G3,T4},
    {C3,T4}, {G3,T4}, {C3,T2},
    {REST,T4}, {REST,T4},
    {C3,T4}, {C3,T4}, {E3,T4}, {C3,T2},
    {G3,T4}, {C3,T4}, {C3,T2}, {C3,T2},
    {REST,T2}, {REST,T2},
};

// ============================================================================
// Lengths
// ============================================================================
#define TITLE_MEL_LEN   (sizeof(g_TitleMel) / sizeof(NoteEntry))
#define TITLE_BAS_LEN   (sizeof(g_TitleBas) / sizeof(NoteEntry))
#define GAME_MEL_LEN    (sizeof(g_GameMel) / sizeof(NoteEntry))
#define GAME_BAS_LEN    (sizeof(g_GameBas) / sizeof(NoteEntry))
#define VIC_MEL_LEN     (sizeof(g_VicMel) / sizeof(NoteEntry))
#define VIC_BAS_LEN     (sizeof(g_VicBas) / sizeof(NoteEntry))

// ============================================================================
// State
// ============================================================================
#define MODE_TITLE   0
#define MODE_GAME    1
#define MODE_VICTORY 2

static u8 s_mode;
static u8 s_melIdx;
static u8 s_basIdx;
static u8 s_melTimer;
static u8 s_basTimer;

static const NoteEntry* GetMel(void) {
    if (s_mode == MODE_VICTORY) return g_VicMel;
    if (s_mode == MODE_GAME) return g_GameMel;
    return g_TitleMel;
}
static const NoteEntry* GetBas(void) {
    if (s_mode == MODE_VICTORY) return g_VicBas;
    if (s_mode == MODE_GAME) return g_GameBas;
    return g_TitleBas;
}
static u8 GetMelLen(void) {
    if (s_mode == MODE_VICTORY) return VIC_MEL_LEN;
    if (s_mode == MODE_GAME) return GAME_MEL_LEN;
    return TITLE_MEL_LEN;
}
static u8 GetBasLen(void) {
    if (s_mode == MODE_VICTORY) return VIC_BAS_LEN;
    if (s_mode == MODE_GAME) return GAME_BAS_LEN;
    return TITLE_BAS_LEN;
}

// ============================================================================
// INIT — starts with title music
// ============================================================================

static void ResetPlayback(void) {
    s_melIdx = 0;
    s_basIdx = 0;
    s_melTimer = 1;
    s_basTimer = 1;
}

void Music_Init(void) {
    // R#7 mixer: tone A+B enabled, all noise disabled, port A input,
    // port B OUTPUT (bit 7 = 1) — required so the joystick output
    // pulses written to R#15 by NTap_Update actually reach the pins.
    // Using 0x3C here would zero bit 7 and break NinjaTap on real HW.
    PSG_SetRegister(7, 0xBC);
    PSG_SetRegister(8, 12);
    PSG_SetRegister(9, 9);
    PSG_SetRegister(10, 0);
    s_mode = MODE_TITLE;
    ResetPlayback();
}

void Music_StartGame(void) {
    s_mode = MODE_GAME;
    ResetPlayback();
    PSG_SetRegister(8, 12);
    PSG_SetRegister(9, 9);
}

void Music_Victory(void) {
    s_mode = MODE_VICTORY;
    ResetPlayback();
    PSG_SetRegister(8, 14);
    PSG_SetRegister(9, 11);
}

// ============================================================================
// UPDATE — call every frame
// ============================================================================

void Music_Update(void) {
    const NoteEntry* mel = GetMel();
    const NoteEntry* bas = GetBas();
    u8 melLen = GetMelLen();
    u8 basLen = GetBasLen();
    u8 melVol = (s_mode == MODE_VICTORY) ? 14 : 12;
    u8 basVol = (s_mode == MODE_VICTORY) ? 11 : 9;

    // Melody channel A
    s_melTimer--;
    if (s_melTimer == 0) {
        u16 p = mel[s_melIdx].period;
        s_melTimer = mel[s_melIdx].dur;
        s_melIdx++;
        if (s_melIdx >= melLen) {
            if (s_mode == MODE_VICTORY) s_melIdx = melLen - 2;
            else s_melIdx = 0;
        }
        if (p > 0) {
            PSG_SetRegister(0, (u8)(p & 0xFF));
            PSG_SetRegister(1, (u8)((p >> 8) & 0x0F));
            PSG_SetRegister(8, melVol);
        } else {
            PSG_SetRegister(8, 0);
        }
    }

    // Bass channel B
    s_basTimer--;
    if (s_basTimer == 0) {
        u16 p = bas[s_basIdx].period;
        s_basTimer = bas[s_basIdx].dur;
        s_basIdx++;
        if (s_basIdx >= basLen) {
            if (s_mode == MODE_VICTORY) s_basIdx = basLen - 2;
            else s_basIdx = 0;
        }
        if (p > 0) {
            PSG_SetRegister(2, (u8)(p & 0xFF));
            PSG_SetRegister(3, (u8)((p >> 8) & 0x0F));
            PSG_SetRegister(9, basVol);
        } else {
            PSG_SetRegister(9, 0);
        }
    }
}
