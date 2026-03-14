// ____________________________________________________________________________
// Game logic — TiniTetris 4P
// ____________________________________________________________________________

#include "game.h"

// ============================================================================
// GLOBALS
// ============================================================================
Player g_Players[NUM_PLAYERS];
u8     g_Frame = 0;

// ============================================================================
// PIECES — packed bitmask
// ============================================================================
#define P4(r0,r1,r2,r3) (u16)(((u16)(r0)<<12)|((u16)(r1)<<8)|((u16)(r2)<<4)|(u16)(r3))

const PieceDef g_Pieces[NUM_PIECES] = {
    // I
    { 2, { { 4, 1, P4(0xF,0,0,0) }, { 1, 4, P4(0x8,0x8,0x8,0x8) }, {0,0,0}, {0,0,0} } },
    // O
    { 1, { { 2, 2, P4(0xC,0xC,0,0) }, {0,0,0}, {0,0,0}, {0,0,0} } },
    // T
    { 4, { { 3,2, P4(0x4,0xE,0,0) }, { 2,3, P4(0x8,0xC,0x8,0) }, { 3,2, P4(0xE,0x4,0,0) }, { 2,3, P4(0x4,0xC,0x4,0) } } },
    // S
    { 2, { { 3,2, P4(0x6,0xC,0,0) }, { 2,3, P4(0x8,0xC,0x4,0) }, {0,0,0}, {0,0,0} } },
    // Z
    { 2, { { 3,2, P4(0xC,0x6,0,0) }, { 2,3, P4(0x4,0xC,0x8,0) }, {0,0,0}, {0,0,0} } },
    // L
    { 4, { { 2,3, P4(0x8,0x8,0xC,0) }, { 3,2, P4(0xE,0x8,0,0) }, { 2,3, P4(0xC,0x4,0x4,0) }, { 3,2, P4(0x2,0xE,0,0) } } },
    // J
    { 4, { { 2,3, P4(0x4,0x4,0xC,0) }, { 3,2, P4(0x8,0xE,0,0) }, { 2,3, P4(0xC,0x8,0x8,0) }, { 3,2, P4(0xE,0x2,0,0) } } },
};

// ============================================================================
// HELPERS
// ============================================================================

// Bitmask lookup: bit positions 15..0 → mask high byte or low byte
// bits stored as: row0=bits[15:12], row1=bits[11:8], row2=bits[7:4], row3=bits[3:0]
static const u16 g_BitMask[4][4] = {
    { 0x8000, 0x4000, 0x2000, 0x1000 },  // row 0
    { 0x0800, 0x0400, 0x0200, 0x0100 },  // row 1
    { 0x0080, 0x0040, 0x0020, 0x0010 },  // row 2
    { 0x0008, 0x0004, 0x0002, 0x0001 },  // row 3
};

u8 Piece_GetBit(u16 bits, u8 row, u8 col) {
    return (bits & g_BitMask[row][col]) ? 1 : 0;
}

const PieceRot* Piece_GetRot(u8 pieceIdx, u8 rot) {
    return &g_Pieces[pieceIdx].rots[rot % g_Pieces[pieceIdx].numRots];
}

u8 Player_Valid(const Player* p, u8 pieceIdx, u8 rot, i8 x, i8 y) {
    const PieceRot* s = Piece_GetRot(pieceIdx, rot);
    u8 r, c;
    for (r = 0; r < s->h; r++) {
        for (c = 0; c < s->w; c++) {
            if (Piece_GetBit(s->bits, r, c)) {
                i8 nx = x + (i8)c;
                i8 ny = y + (i8)r;
                if (nx < 0 || nx >= BOARD_W || ny >= BOARD_H)
                    return 0;
                if (ny >= 0 && p->board[ny][nx] != 0)
                    return 0;
            }
        }
    }
    return 1;
}

// ============================================================================
// INIT / SPAWN
// ============================================================================

void Player_Spawn(Player* p) {
    p->pieceIdx = p->nextIdx;
    p->nextIdx = Math_GetRandom8() % NUM_PIECES;
    p->rot = 0;
    p->px = (BOARD_W / 2) - 1;
    p->py = 0;
    p->aiComputed = 0;
    if (!Player_Valid(p, p->pieceIdx, p->rot, p->px, p->py))
        p->dead = 1;
}

static void Player_Init(Player* p, u8 idx) {
    u8 r, c;
    for (r = 0; r < BOARD_H; r++)
        for (c = 0; c < BOARD_W; c++)
            p->board[r][c] = 0;

    p->score = 0;
    p->level = 1;
    p->lines = 0;
    p->dead = 0;
    p->dropTimer = 0;
    p->dropInterval = DROP_INITIAL;
    p->softDrop = 0;
    p->flashTimer = 0;
    p->flashCount = 0;
    p->aiComputed = 0;
    p->targetPlayer = (idx + 1) % NUM_PLAYERS;
    p->boardDirty = 0;

    p->pieceIdx = Math_GetRandom8() % NUM_PIECES;
    p->nextIdx = Math_GetRandom8() % NUM_PIECES;
    p->rot = 0;
    p->px = (BOARD_W / 2) - 1;
    p->py = 0;

    if (!Player_Valid(p, p->pieceIdx, p->rot, p->px, p->py))
        p->dead = 1;
}

// ============================================================================
// ACTIONS
// ============================================================================

void Player_Move(Player* p, i8 dx) {
    if (p->dead || p->flashTimer > 0) return;
    if (Player_Valid(p, p->pieceIdx, p->rot, p->px + dx, p->py))
        p->px += dx;
}

void Player_Rotate(Player* p) {
    u8 newRot;
    if (p->dead || p->flashTimer > 0) return;
    newRot = (p->rot + 1) % g_Pieces[p->pieceIdx].numRots;
    if (Player_Valid(p, p->pieceIdx, newRot, p->px, p->py))
        p->rot = newRot;
    else if (Player_Valid(p, p->pieceIdx, newRot, p->px - 1, p->py))
        { p->px--; p->rot = newRot; }
    else if (Player_Valid(p, p->pieceIdx, newRot, p->px + 1, p->py))
        { p->px++; p->rot = newRot; }
}

static void Player_Lock(Player* p) {
    const PieceRot* s = Piece_GetRot(p->pieceIdx, p->rot);
    u8 r, c;
    u8 pIdx;
    u8 fullCount = 0;
    static const u16 scoreTable[5] = { 0, 1, 3, 6, 10 };

    // Player index (0-based)
    pIdx = (u8)(p - g_Players);

    // Place on board
    for (r = 0; r < s->h; r++) {
        for (c = 0; c < s->w; c++) {
            if (Piece_GetBit(s->bits, r, c)) {
                i8 by = p->py + (i8)r;
                i8 bx = p->px + (i8)c;
                if (by >= 0 && by < BOARD_H && bx >= 0 && bx < BOARD_W)
                    p->board[by][bx] = pIdx + 1;
            }
        }
    }

    // Find full lines
    p->flashCount = 0;
    for (r = 0; r < BOARD_H; r++) {
        u8 full = 1;
        for (c = 0; c < BOARD_W; c++) {
            if (p->board[r][c] == 0) { full = 0; break; }
        }
        if (full && p->flashCount < 4)
            p->flashRows[p->flashCount++] = r;
    }

    if (p->flashCount > 0) {
        // Send garbage to targeted player
        {
            static const u8 garbageTable[5] = { 0, 0, 1, 2, 4 };
            u8 gCount = garbageTable[p->flashCount];
            if (gCount > 0)
                Player_AddGarbage(&g_Players[p->targetPlayer], gCount);
        }
        p->flashTimer = FLASH_DURATION;
        p->score += scoreTable[p->flashCount];
        p->lines += p->flashCount;
        p->level = (p->lines / LINES_PER_LVL) + 1;
        {
            u8 di = DROP_INITIAL - p->level * DROP_ACCEL;
            p->dropInterval = (di < DROP_MIN) ? DROP_MIN : di;
        }
    } else {
        Player_Spawn(p);
    }
}

void Player_Drop(Player* p) {
    if (p->dead || p->flashTimer > 0) return;
    if (Player_Valid(p, p->pieceIdx, p->rot, p->px, p->py + 1))
        p->py++;
    else
        Player_Lock(p);
}

static void Player_ClearLines(Player* p) {
    u8 i, r, c;
    // Process top-to-bottom: removing a top row shifts it down,
    // rows below stay in place so their indices remain valid.
    for (i = 0; i < p->flashCount; i++) {
        u8 row = p->flashRows[i];
        for (r = row; r > 0; r--)
            for (c = 0; c < BOARD_W; c++)
                p->board[r][c] = p->board[r - 1][c];
        for (c = 0; c < BOARD_W; c++)
            p->board[0][c] = 0;
    }
    p->flashCount = 0;
    Player_Spawn(p);
}

// ============================================================================
// UPDATE
// ============================================================================

void Player_Update(Player* p) {
    if (p->dead) return;
    if (p->flashTimer > 0) {
        p->flashTimer--;
        if (p->flashTimer == 0)
            Player_ClearLines(p);
        return;
    }
    p->dropTimer++;
    {
        u8 interval = p->softDrop ? 3 : p->dropInterval;
        if (p->dropTimer >= interval) {
            p->dropTimer = 0;
            Player_Drop(p);
        }
    }
}

// ============================================================================
// AI
// ============================================================================

static i16 EvalPos(const Player* p, u8 pieceIdx, u8 rot, i8 x, i8 y) {
    u8 tmpBoard[BOARD_H][BOARD_W];
    const PieceRot* s = Piece_GetRot(pieceIdx, rot);
    u8 r, c;
    i16 maxH = 0, holes = 0, bump = 0, lines = 0;
    i16 heights[BOARD_W];

    for (r = 0; r < BOARD_H; r++)
        for (c = 0; c < BOARD_W; c++)
            tmpBoard[r][c] = p->board[r][c];

    for (r = 0; r < s->h; r++)
        for (c = 0; c < s->w; c++)
            if (Piece_GetBit(s->bits, r, c)) {
                i8 br = y + (i8)r;
                i8 bc = x + (i8)c;
                if (br >= 0 && br < BOARD_H && bc >= 0 && bc < BOARD_W)
                    tmpBoard[br][bc] = 1;
            }

    for (c = 0; c < BOARD_W; c++) {
        u8 found = 0;
        heights[c] = 0;
        for (r = 0; r < BOARD_H; r++) {
            if (tmpBoard[r][c]) {
                if (!found) { heights[c] = BOARD_H - r; found = 1; }
                if ((i16)(BOARD_H - r) > maxH) maxH = BOARD_H - r;
            } else if (found) {
                holes++;
            }
        }
    }

    for (r = 0; r < BOARD_H; r++) {
        u8 full = 1;
        for (c = 0; c < BOARD_W; c++)
            if (tmpBoard[r][c] == 0) { full = 0; break; }
        if (full) lines++;
    }

    for (c = 0; c < BOARD_W - 1; c++) {
        i16 d = heights[c] - heights[c + 1];
        if (d < 0) d = -d;
        bump += d;
    }

    return lines * 200 - holes * 150 - bump * 20 - maxH * 10;
}

void Player_AI(Player* p, u8 frame) {
    (void)frame;
    if (p->dead || p->flashTimer > 0) return;

    if (!p->aiComputed) {
        i16 bestScore = -32000;
        i8 bestX = p->px;
        u8 bestRot = p->rot;
        u8 numRots = g_Pieces[p->pieceIdx].numRots;
        u8 ri;
        i8 xi, yi;

        for (ri = 0; ri < numRots; ri++) {
            for (xi = -1; xi < (i8)BOARD_W; xi++) {
                if (!Player_Valid(p, p->pieceIdx, ri, xi, p->py))
                    continue;
                yi = p->py;
                while (Player_Valid(p, p->pieceIdx, ri, xi, yi + 1)) yi++;
                {
                    i16 sc = EvalPos(p, p->pieceIdx, ri, xi, yi);
                    if (sc > bestScore) {
                        bestScore = sc;
                        bestX = xi;
                        bestRot = ri;
                    }
                }
            }
        }
        p->aiTargetX = bestX;
        p->aiTargetRot = bestRot;
        p->aiComputed = 1;
    }

    if (p->rot != p->aiTargetRot)
        p->rot = p->aiTargetRot % g_Pieces[p->pieceIdx].numRots;
    else if (p->px < p->aiTargetX)
        Player_Move(p, 1);
    else if (p->px > p->aiTargetX)
        Player_Move(p, -1);
    else {
        // Hard drop: slam piece all the way down instantly
        while (Player_Valid(p, p->pieceIdx, p->rot, p->px, p->py + 1))
            p->py++;
        Player_Drop(p); // This will call Player_Lock
    }
}

// ============================================================================
// TARGETING / GARBAGE
// ============================================================================

void Player_CycleTarget(Player* p, u8 pIdx) {
    u8 next = p->targetPlayer;
    u8 i;
    for (i = 0; i < NUM_PLAYERS - 1; i++) {
        next = (next + 1) % NUM_PLAYERS;
        if (next != pIdx && !g_Players[next].dead) {
            p->targetPlayer = next;
            return;
        }
    }
}

void Player_AddGarbage(Player* p, u8 count) {
    u8 i, c;
    u8 gap;
    if (p->dead) return;
    for (i = 0; i < count; i++) {
        u8 r;
        // Check if top row has blocks — if so, player dies
        for (c = 0; c < BOARD_W; c++) {
            if (p->board[0][c]) { p->dead = 1; return; }
        }
        // Shift entire board up by 1
        for (r = 0; r < BOARD_H - 1; r++)
            for (c = 0; c < BOARD_W; c++)
                p->board[r][c] = p->board[r + 1][c];
        // Fill bottom row with garbage (all filled except one random gap)
        gap = Math_GetRandom8() % BOARD_W;
        for (c = 0; c < BOARD_W; c++)
            p->board[BOARD_H - 1][c] = (c == gap) ? 0 : 5;  // 5 = garbage color
    }
    p->boardDirty = 1;
}

// ============================================================================
// GAME INIT
// ============================================================================

void Game_Init(void) {
    u8 i;
    for (i = 0; i < NUM_PLAYERS; i++)
        Player_Init(&g_Players[i], i);
    g_Frame = 0;
}
