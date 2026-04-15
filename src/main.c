// ____________________________________________________________________________
// TiniTetris 4P — MSX1 Tetris for 4 players
// MSX1 · TMS9918A · Screen 2 · MSXgl
// ____________________________________________________________________________

#include "msxgl.h"
#include "game.h"
#include "music.h"

void Render_Init(void);
void Render_GameBegin(void);
void Render_Frame(void);
void Render_Victory(u8 winnerIdx);
void Render_TitleScreen(void);
void Render_TitleReady(u8 pIdx);
void Render_TitleCPU(u8 pIdx);
void Input_Init(void);
void Input_GameUpdate(void);
u8   Input_TitleCheck(void);

// BIOS JIFFY counter — incremented every VBlank by BIOS ISR
#define JIFFY (*(volatile u16*)0xFC9E)

// Auto-start timer after first human joins (~8 seconds at 50Hz)
#define TITLE_COUNTDOWN  400

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
            u16 timer = 0;  // 0 = not started; counts down after first join
            Render_TitleScreen();
            while (1) {
                Halt();
                Music_Update();
                {
                    u8 pressed = Input_TitleCheck();
                    u8 p;
                    u8 newJoins = 0;
                    for (p = 0; p < NUM_PLAYERS; p++) {
                        if ((pressed & (1 << p)) && !(ready & (1 << p))) {
                            ready |= (1 << p);
                            Render_TitleReady(p);
                            newJoins = 1;
                        }
                    }
                    // Reset timer on any new join
                    if (newJoins) timer = TITLE_COUNTDOWN;
                    // All humans joined → start immediately
                    if (ready == 0x0F) break;
                }
                // Count down if timer active; start when it hits 0
                if (timer > 0) {
                    timer--;
                    if (timer == 0 && ready != 0) break;
                }
                if (Keyboard_IsKeyPressed(KEY_ESC)) { Bios_Exit(0); }
            }

            // Commit slot configuration
            g_HumanMask = ready;

            // Show CPU labels for unclaimed slots
            for (i = 0; i < NUM_PLAYERS; i++) {
                if (!(ready & (1 << i))) Render_TitleCPU(i);
            }

            // Brief pause before starting
            for (i = 0; i < 60; i++) { Halt(); Music_Update(); }
        }

        // --- GAME ---
        Game_Init();
        Music_StartGame();
        Render_GameBegin();
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
                // Round-robin: only one AI gets a turn to think each tick.
                // Others just execute their already-computed plan (cheap).
                u8 aiTurn = g_Frame & 3;
                for (i = 0; i < NUM_PLAYERS; i++) {
                    if (!(g_HumanMask & (1 << i))) {
                        // Only the "turn" AI may do the heavy search this tick;
                        // others run their execution step only if already planned.
                        if (i == aiTurn || g_Players[i].aiComputed)
                            Player_AI(&g_Players[i], g_Frame);
                    }
                    Player_Update(&g_Players[i]);
                }
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
                while (vicTimer < 200) {
                    Halt();
                    Music_Update();
                    vicTimer++;
                    if (Keyboard_IsKeyPressed(KEY_ESC)) break;
                }
                break;
            }

            if (Keyboard_IsKeyPressed(KEY_ESC)) { Bios_Exit(0); }
        }
    }
}
