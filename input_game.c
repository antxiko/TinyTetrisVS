// ____________________________________________________________________________
// Input handling — TiniTetris 4P
// P1: keyboard (arrows) OR NinjaTap joystick 0
// P2-P4: NinjaTap joysticks 1-3
// Falls back to keyboard-only if no NinjaTap detected.
// ____________________________________________________________________________

#include "msxgl.h"
#include "device/ninjatap.h"
#include "game.h"

// Auto-repeat for left/right
#define REPEAT_DELAY  12
#define REPEAT_RATE   3

// Per-player repeat timers
static u8 s_repTimer[NUM_PLAYERS] = { 0, 0, 0, 0 };

// Previous keyboard row states (for P1 keyboard)
static u8 s_prev8 = 0xFF;

// NinjaTap state
static u8 s_ntapPorts = 0;  // 0 = not detected

// ============================================================================
// INIT — call once at startup
// ============================================================================

void Input_Init(void) {
    NTap_Check();
    s_ntapPorts = NTap_GetPortNum();
}

// ============================================================================
// Generic player input handler
// ============================================================================

static void DoPlayer(Player* p, u8 pNum,
                     u8 left, u8 right, u8 rotate, u8 drop,
                     u8 pLeft, u8 pRight, u8 pRotate, u8 pDrop) {
    // Rotate on edge
    if (rotate && !pRotate)
        Player_Rotate(p);

    // Soft drop: hold down = fast, release = normal
    p->softDrop = drop;
    if (drop && !pDrop) {
        Player_Drop(p);
        p->dropTimer = 0;
    }

    // Left/right with auto-repeat
    if (left && !right) {
        if (!pLeft) {
            Player_Move(p, -1);
            s_repTimer[pNum] = 0;
        } else {
            s_repTimer[pNum]++;
            if (s_repTimer[pNum] >= REPEAT_DELAY) {
                if ((s_repTimer[pNum] - REPEAT_DELAY) % REPEAT_RATE == 0)
                    Player_Move(p, -1);
            }
        }
    } else if (right && !left) {
        if (!pRight) {
            Player_Move(p, 1);
            s_repTimer[pNum] = 0;
        } else {
            s_repTimer[pNum]++;
            if (s_repTimer[pNum] >= REPEAT_DELAY) {
                if ((s_repTimer[pNum] - REPEAT_DELAY) % REPEAT_RATE == 0)
                    Player_Move(p, 1);
            }
        }
    } else {
        s_repTimer[pNum] = 0;
    }
}

// ============================================================================
// NinjaTap joystick → DoPlayer
// UP = rotate, Down = soft drop, Button A = cycle target
// ============================================================================

static void DoJoyPlayer(u8 pNum, u8 joyIdx) {
    u8 l  = NTap_IsPressed(joyIdx, NTAP_LEFT);
    u8 r  = NTap_IsPressed(joyIdx, NTAP_RIGHT);
    u8 u  = NTap_IsPressed(joyIdx, NTAP_UP);     // UP = rotate
    u8 d  = NTap_IsPressed(joyIdx, NTAP_DOWN);
    u8 a  = NTap_IsPressed(joyIdx, NTAP_A);      // A = cycle target
    u8 pl = (g_NTap_Prev[joyIdx] & NTAP_LEFT) == 0;
    u8 pr = (g_NTap_Prev[joyIdx] & NTAP_RIGHT) == 0;
    u8 pu = (g_NTap_Prev[joyIdx] & NTAP_UP) == 0;
    u8 pd = (g_NTap_Prev[joyIdx] & NTAP_DOWN) == 0;
    u8 pa = (g_NTap_Prev[joyIdx] & NTAP_A) == 0;
    DoPlayer(&g_Players[pNum], pNum, l, r, u, d, pl, pr, pu, pd);
    // Cycle target on button A edge
    if (a && !pa)
        Player_CycleTarget(&g_Players[pNum], pNum);
}

// ============================================================================
// Keyboard P1 (arrows + UP=rotate, DOWN=drop)
// ============================================================================

static void DoKeyboardP1(void) {
    u8 row8 = Keyboard_Read(8);
    u8 l  = !(row8 & 0x10);
    u8 u  = !(row8 & 0x20);
    u8 d  = !(row8 & 0x40);
    u8 r  = !(row8 & 0x80);
    u8 pl = !(s_prev8 & 0x10);
    u8 pu = !(s_prev8 & 0x20);
    u8 pd = !(s_prev8 & 0x40);
    u8 pr = !(s_prev8 & 0x80);
    DoPlayer(&g_Players[0], 0, l, r, u, d, pl, pr, pu, pd);
    // Space = cycle target (also row 8, bit 0)
    {
        u8 sp  = !(row8 & 0x01);
        u8 psp = !(s_prev8 & 0x01);
        if (sp && !psp)
            Player_CycleTarget(&g_Players[0], 0);
    }
    s_prev8 = row8;
}

// ============================================================================
// PUBLIC — call every frame
// ============================================================================

void Input_GameUpdate(void) {
    if (s_ntapPorts >= 2) {
        // NinjaTap detected — update all joystick states
        NTap_Update();

        // P1: keyboard only (space/arrows leak into joystick in emulator)
        DoKeyboardP1();

        // P2-P4: joysticks 1-3
        if (s_ntapPorts >= 5) {
            DoJoyPlayer(1, 1);
            DoJoyPlayer(2, 2);
            DoJoyPlayer(3, 3);
        } else {
            // Only 2 standard ports: P2 = joystick 1
            DoJoyPlayer(1, 1);
        }
    } else {
        // No NinjaTap — keyboard only for P1
        DoKeyboardP1();
    }
}
