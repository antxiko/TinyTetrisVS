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
void Render_Countdown(u8 ch);
void Render_ClearCountdown(void);
void Render_Stats(void);
void Input_Init(void);
void Input_GameUpdate(void);
u8   Input_TitleCheck(void);

// BIOS JIFFY counter — incremented every VBlank by BIOS ISR
#define JIFFY (*(volatile u16*)0xFC9E)

// Auto-start timer after first human joins (~8 seconds at 50Hz)
#define TITLE_COUNTDOWN  400
// Attract mode: 4 CPUs play after ~15 seconds of no input on title
#define ATTRACT_TIMEOUT  750

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
            u16 timer = 0;      // counts down after first human join
            u16 attract = ATTRACT_TIMEOUT;  // counts down to attract mode
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
                    if (newJoins) { timer = TITLE_COUNTDOWN; attract = 0; }
                    if (ready == 0x0F) break;
                }
                // Human join timer
                if (timer > 0) {
                    timer--;
                    if (timer == 0 && ready != 0) break;
                }
                // Attract mode timer: no input → 4 CPUs demo
                if (attract > 0) {
                    attract--;
                    if (attract == 0 && ready == 0) break;
                }
                if (Keyboard_IsKeyPressed(KEY_ESC)) { Bios_Exit(0); }
            }

            // Commit slot configuration (ready=0 means attract: all CPU)
            g_HumanMask = ready;

            // Show CPU labels for unclaimed slots
            for (i = 0; i < NUM_PLAYERS; i++) {
                if (!(ready & (1 << i))) Render_TitleCPU(i);
            }

            // Brief pause (shorter for attract)
            for (i = 0; i < (ready ? 60 : 20); i++) { Halt(); Music_Update(); }
        }

        // --- GAME ---
        Game_Init();
        Music_StartGame();
        Render_GameBegin();

        // 3-2-1 countdown
        Render_Countdown('3');
        for (i = 0; i < 50; i++) { Halt(); Music_Update(); }
        Render_Countdown('2');
        for (i = 0; i < 50; i++) { Halt(); Music_Update(); }
        Render_Countdown('1');
        for (i = 0; i < 50; i++) { Halt(); Music_Update(); }
        Render_ClearCountdown();

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
                // Round-robin: only one AI searches per tick. Others execute
                // their plan every OTHER tick to cut CPU cost in half.
                {
                    u8 aiTurn = g_Frame & 3;
                    u8 execTick = g_Frame & 1;  // AI executes on odd frames
                    for (i = 0; i < NUM_PLAYERS; i++) {
                        if (!(g_HumanMask & (1 << i))) {
                            if (i == aiTurn || (g_Players[i].aiComputed && execTick))
                                Player_AI(&g_Players[i], g_Frame);
                        }
                        Player_Update(&g_Players[i]);
                    }
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
                {
                    u16 waitT;
                    // Victory screen ~4 sec
                    for (waitT = 0; waitT < 200; waitT++) {
                        Halt(); Music_Update();
                        if (Keyboard_IsKeyPressed(KEY_ESC)) break;
                    }
                    // Stats screen ~15 sec or any key
                    Render_Stats();
                    for (waitT = 0; waitT < 750; waitT++) {
                        Halt(); Music_Update();
                        if (Keyboard_Read(8) != 0xFF) break;
                        if (Keyboard_IsKeyPressed(KEY_ESC)) break;
                    }
                }
                break;
            }

            // Attract mode: any key → back to title
            if (g_HumanMask == 0) {
                u8 row8 = Keyboard_Read(8);
                if (row8 != 0xFF) break;  // any key on row 8 pressed
            }

            if (Keyboard_IsKeyPressed(KEY_ESC)) {
                if (g_HumanMask == 0) break;  // attract → title
                Bios_Exit(0);                 // real game → quit
            }
        }
    }
}
