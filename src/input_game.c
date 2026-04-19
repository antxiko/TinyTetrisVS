// ____________________________________________________________________________
// Input handling — TiniTetris 4P
// Two modes selected from the title screen:
//   KB+JOY:    P1=arrows+Space, P2=WASD+Ctrl, P3=Joy1, P4=Joy2
//   NINJATAP:  P1-P4 all on NinjaTap adapter
// ____________________________________________________________________________

#include "msxgl.h"
#include "device/ninjatap.h"
#include "game.h"

// Auto-repeat for left/right
#define REPEAT_DELAY  12
#define REPEAT_RATE   3

// Per-player repeat timers
static u8 s_repTimer[NUM_PLAYERS] = { 0, 0, 0, 0 };

// Previous keyboard row states
static u8 s_prev8  = 0xFF;  // P1: row 8 (arrows + space)
static u8 s_prevP2_r2 = 0xFF;  // P2: row 2 (A, D)
static u8 s_prevP2_r4 = 0xFF;  // P2: row 4 (W, S)
static u8 s_prevP2_r6 = 0xFF;  // P2: row 6 (CTRL)

// NinjaTap state
static u8 s_ntapPorts = 0;

// ============================================================================
// INIT
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
    if (rotate && !pRotate)
        Player_Rotate(p);

    p->softDrop = drop;
    if (drop && !pDrop) {
        Player_Drop(p);
        p->dropTimer = 0;
    }

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
// NinjaTap / standard joystick → DoPlayer
// ============================================================================

// Own button-state tracking — g_NTap_Prev bit 7 is unreliable on some
// real NinjaTap hardware, so we track the physical A button ourselves.
static u8 s_joyBtnPrev[4] = { 0, 0, 0, 0 };

static void DoJoyPlayer(u8 pNum, u8 joyIdx) {
    u8 l  = NTap_IsPressed(joyIdx, NTAP_LEFT);
    u8 r  = NTap_IsPressed(joyIdx, NTAP_RIGHT);
    u8 u  = NTap_IsPressed(joyIdx, NTAP_UP);
    u8 d  = NTap_IsPressed(joyIdx, NTAP_DOWN);
    u8 pl = (g_NTap_Prev[joyIdx] & NTAP_LEFT) == 0;
    u8 pr = (g_NTap_Prev[joyIdx] & NTAP_RIGHT) == 0;
    u8 pu = (g_NTap_Prev[joyIdx] & NTAP_UP) == 0;
    u8 pd = (g_NTap_Prev[joyIdx] & NTAP_DOWN) == 0;
    DoPlayer(&g_Players[pNum], pNum, l, r, u, d, pl, pr, pu, pd);

    // Physical button A = MSXgl NTAP_B (swapped in library).
    // Edge-detect with our own prev to avoid g_NTap_Prev bit 7 issues.
    {
        u8 btn = NTap_IsPressed(joyIdx, NTAP_B);
        if (btn && !s_joyBtnPrev[joyIdx])
            Player_CycleTarget(&g_Players[pNum], pNum);
        s_joyBtnPrev[joyIdx] = btn;
    }
}

// ============================================================================
// Keyboard P1 (arrows + Space = target)
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
    {
        u8 sp  = !(row8 & 0x01);
        u8 psp = !(s_prev8 & 0x01);
        if (sp && !psp)
            Player_CycleTarget(&g_Players[0], 0);
    }
    s_prev8 = row8;
}

// ============================================================================
// Keyboard P2 (WASD + CTRL = target)
// MSX keyboard matrix: A=row2 bit0, D=row2 bit3, W=row4 bit6, S=row4 bit2,
//                      CTRL=row6 bit1
// ============================================================================

static void DoKeyboardP2(void) {
    u8 row2 = Keyboard_Read(2);
    u8 row4 = Keyboard_Read(4);
    u8 row6 = Keyboard_Read(6);

    u8 l  = !(row2 & 0x01);         // A = left
    u8 r  = !(row2 & 0x08);         // D = right
    u8 u  = !(row4 & 0x40);         // W = rotate
    u8 d  = !(row4 & 0x04);         // S = soft drop

    u8 pl = !(s_prevP2_r2 & 0x01);
    u8 pr = !(s_prevP2_r2 & 0x08);
    u8 pu = !(s_prevP2_r4 & 0x40);
    u8 pd = !(s_prevP2_r4 & 0x04);

    DoPlayer(&g_Players[1], 1, l, r, u, d, pl, pr, pu, pd);

    // CTRL = cycle target
    {
        u8 ct  = !(row6 & 0x02);
        u8 pct = !(s_prevP2_r6 & 0x02);
        if (ct && !pct)
            Player_CycleTarget(&g_Players[1], 1);
    }

    s_prevP2_r2 = row2;
    s_prevP2_r4 = row4;
    s_prevP2_r6 = row6;
}

// ============================================================================
// TITLE SCREEN — check who pressed their button (edge-triggered)
// Returns bitmask: bit 0=P1, bit 1=P2, etc.
// Also checks F1 for mode toggle (returned as bit 7).
// ============================================================================

static u8 s_titlePrev = 0xFF;
static u8 s_titleKeyPrev = 0xFF;
static u8 s_titleF1Prev = 0xFF;
static u8 s_titleJoyPrev[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

u8 Input_TitleCheck(void) {
    u8 result = 0;
    u8 row8 = Keyboard_Read(8);
    u8 row6 = Keyboard_Read(6);

    // P1: Space (row 8 bit 0)
    {
        u8 sp = !(row8 & 0x01);
        u8 psp = !(s_titlePrev & 0x01);
        if (sp && !psp) result |= 0x01;
    }
    s_titlePrev = row8;

    // P2 in KB+JOY mode: CTRL (row 6 bit 1)
    if (g_InputMode == 0) {
        u8 ct = !(row6 & 0x02);
        u8 pct = !(s_titleF1Prev & 0x02);  // reuse for ctrl prev
        if (ct && !pct) result |= 0x02;
    }

    // F1 (row 6 bit 5): mode toggle → bit 7
    {
        u8 f1 = !(row6 & 0x20);
        u8 pf1 = !(s_titleF1Prev & 0x20);
        if (f1 && !pf1) result |= 0x80;
    }
    s_titleF1Prev = row6;

    // Number keys 1-4 (row 0: bits 1-4)
    {
        u8 row0 = Keyboard_Read(0);
        u8 k;
        for (k = 0; k < 4; k++) {
            u8 pressed = !(row0 & (u8)(1 << (k + 1)));
            u8 prev = !(s_titleKeyPrev & (u8)(1 << (k + 1)));
            if (pressed && !prev) result |= (u8)(1 << k);
        }
        s_titleKeyPrev = row0;
    }

    // Joystick/NinjaTap players — use NTAP_B (= physical A, swapped in MSXgl)
    // with our own edge tracking to avoid g_NTap_Prev bit 7 issues on real HW.
    if (s_ntapPorts >= 2) {
        u8 p;
        NTap_Update();
        for (p = 0; p < 4; p++) {
            u8 a = NTap_IsPressed(p, NTAP_B);
            u8 pa = s_titleJoyPrev[p];
            if (a && !pa) {
                // In KB+JOY mode: joy0→P3, joy1→P4
                // In NINJATAP mode: joy0→P1, joy1→P2, joy2→P3, joy3→P4
                if (g_InputMode == 0)
                    result |= (u8)(1 << (p + 2));  // P3=bit2, P4=bit3
                else
                    result |= (u8)(1 << p);
            }
            s_titleJoyPrev[p] = a;
        }
    }

    return result;
}

// ============================================================================
// PUBLIC — call every frame during gameplay
// ============================================================================

void Input_GameUpdate(void) {
    if (g_InputMode == 0) {
        // KB+JOY mode: P1=keyboard, P2=WASD, P3/P4=standard joysticks
        if (g_HumanMask & 0x01) DoKeyboardP1();
        if (g_HumanMask & 0x02) DoKeyboardP2();
        if (s_ntapPorts >= 2) {
            NTap_Update();
            if (g_HumanMask & 0x04) DoJoyPlayer(2, 0);  // P3 = joy port A
            if (g_HumanMask & 0x08) DoJoyPlayer(3, 1);  // P4 = joy port B
        }
    } else {
        // NINJATAP mode: all 4 on NinjaTap
        if (s_ntapPorts >= 2) {
            NTap_Update();
            if (g_HumanMask & 0x01) DoJoyPlayer(0, 0);
            if (g_HumanMask & 0x02) DoJoyPlayer(1, 1);
            if (g_HumanMask & 0x04) DoJoyPlayer(2, 2);
            if (g_HumanMask & 0x08) DoJoyPlayer(3, 3);
        }
    }
}
