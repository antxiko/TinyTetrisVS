// ____________________________________________________________________________
// TiniTetris 4P — MSX1 Tetris for 4 players
// MSX1 · TMS9918A · Screen 2 · MSXgl
// ____________________________________________________________________________

#include "msxgl.h"
#include "game.h"
#include "music.h"

void Render_Init(void);
void Render_Frame(void);
void Render_Victory(u8 winnerIdx);
void Render_TitleScreen(void);
void Render_TitleReady(u8 pIdx);
void Input_Init(void);
void Input_GameUpdate(void);
u8   Input_TitleCheck(void);

// BIOS JIFFY counter — incremented every VBlank by BIOS ISR
#define JIFFY (*(volatile u16*)0xFC9E)

void main(void) {
    u8 i, j;
    u16 lastJiffy;
    u8 elapsed;
    u8 winner;

    Bios_SetKeyClick(FALSE);
    Render_Init();
    VDP_EnableVBlank(TRUE);
    Input_Init();

    // === MAIN LOOP: Title → Game → Victory → repeat ===
    while (1) {

        // --- TITLE SCREEN ---
        Music_Init();  // Start title music
        {
            u8 ready = 0;
            Render_TitleScreen();
            while (ready != 0x0F) {
                Halt();
                Music_Update();
                {
                    u8 pressed = Input_TitleCheck();
                    u8 p;
                    for (p = 0; p < NUM_PLAYERS; p++) {
                        if ((pressed & (1 << p)) && !(ready & (1 << p))) {
                            ready |= (1 << p);
                            Render_TitleReady(p);
                        }
                    }
                }
                if (Keyboard_IsKeyPressed(KEY_ESC)) { Bios_Exit(0); }
            }
            // Brief pause before starting
            for (i = 0; i < 30; i++) { Halt(); Music_Update(); }
        }

        // --- GAME ---
        Game_Init();
        Music_StartGame();
        Render_Init();
        Render_Frame();
        lastJiffy = JIFFY;

        while (1) {
            Halt();

            elapsed = (u8)(JIFFY - lastJiffy);
            if (elapsed == 0) elapsed = 1;
            if (elapsed > 8) elapsed = 8;
            lastJiffy = JIFFY;

            Input_GameUpdate();

            for (j = 0; j < elapsed; j++) {
                for (i = 0; i < NUM_PLAYERS; i++)
                    Player_Update(&g_Players[i]);
                Music_Update();
                g_Frame++;
            }

            Render_Frame();

            // Check for winner
            winner = Game_CheckWinner();
            if (winner != 0xFF) {
                u8 vicTimer = 0;
                Music_Victory();
                Render_Victory(winner);
                // Play victory music, then return to title
                while (vicTimer < 200) {  // ~4 seconds at 50Hz
                    Halt();
                    Music_Update();
                    vicTimer++;
                    if (Keyboard_IsKeyPressed(KEY_ESC)) break;
                }
                break;  // Back to title
            }

            if (Keyboard_IsKeyPressed(KEY_ESC)) { Bios_Exit(0); }
        }
    }
}
