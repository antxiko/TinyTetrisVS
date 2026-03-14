// ____________________________________________________________________________
// TiniTetris 4P — MSX1 Tetris for 4 players
// MSX1 · TMS9918A · Screen 2 · MSXgl
// ____________________________________________________________________________

#include "msxgl.h"
#include "game.h"
#include "music.h"

void Render_Init(void);
void Render_Frame(void);
void Input_Init(void);
void Input_GameUpdate(void);

// BIOS JIFFY counter — incremented every VBlank by BIOS ISR
#define JIFFY (*(volatile u16*)0xFC9E)

void main(void) {
    u8 i, j;
    u16 lastJiffy;
    u8 elapsed;

    Bios_SetKeyClick(FALSE);
    Render_Init();
    VDP_EnableVBlank(TRUE);
    Game_Init();
    Input_Init();
    Music_Init();

    // Initial full render
    Render_Frame();

    lastJiffy = JIFFY;

    while (1) {
        Halt();  // Wait for at least 1 VBlank

        // Count real elapsed VBlanks via BIOS JIFFY
        elapsed = (u8)(JIFFY - lastJiffy);
        if (elapsed == 0) elapsed = 1;
        if (elapsed > 8) elapsed = 8; // Cap to avoid death spiral
        lastJiffy = JIFFY;

        // Input once per loop
        Input_GameUpdate();

        // Advance game logic and music by real elapsed frames
        for (j = 0; j < elapsed; j++) {
            for (i = 0; i < NUM_PLAYERS; i++)
                Player_Update(&g_Players[i]);
            Music_Update();
            g_Frame++;
        }

        // Render once per loop (diff-based)
        Render_Frame();

        if (Keyboard_IsKeyPressed(KEY_ESC))
            break;
    }

    Bios_Exit(0);
}
