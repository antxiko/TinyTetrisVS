// ____________________________________________________________________________
// Game logic — TiniTetris 4P
// ____________________________________________________________________________

#include "game.h"

// ============================================================================
// GLOBALS
// ============================================================================
Player g_Players[NUM_PLAYERS];
u8     g_Frame = 0;
u8     g_HumanMask = 0x01;  // bit i = 1 → player i is human. Default: only P1.

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
    u8 r;
    p->pieceIdx = p->nextIdx;
    p->nextIdx = Math_GetRandom8() % NUM_PIECES;
    p->rot = 0;
    p->px = (BOARD_W / 2) - 1;
    p->py = 0;

    // Reset AI spread-search state
    p->aiComputed = 0;
    p->aiBaseReady = 0;
    p->aiEvalRot = 0;
    p->aiEvalCol = 0;
    p->aiBestScore = -32000;
    p->aiTargetX = p->px;
    p->aiTargetRot = 0;

    // Pick AI goal for this piece: mostly 2 lines, sometimes 3, rarely 4
    // Distribution: ~70% goal=2, ~22% goal=3, ~8% goal=4
    r = Math_GetRandom8();
    if (r < 20)       p->aiGoal = 4;
    else if (r < 76)  p->aiGoal = 3;
    else              p->aiGoal = 2;

    // AI drop cadence: after aligning, force a drop every ~8-14 ticks.
    // Between natural fall (50 ticks) and human soft-drop (3 ticks) — feels
    // deliberate but keeps the match flowing. Slight per-piece variation.
    p->aiDropDelay = (u8)(8 + (Math_GetRandom8() & 7));  // 8..15
    p->aiDropCounter = 0;

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
    p->aiGoal = 2;
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

    // If current target is dead, reassign to a random alive non-self player
    if (g_Players[p->targetPlayer].dead) {
        u8 pIdx = (u8)(p - g_Players);
        u8 start = Math_GetRandom8() & 3;
        u8 j;
        for (j = 0; j < NUM_PLAYERS; j++) {
            u8 cand = (u8)((start + j) & 3);
            if (cand != pIdx && !g_Players[cand].dead) {
                p->targetPlayer = cand;
                break;
            }
        }
    }

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

// Compute where the piece would land at (rot, x) using precomputed heights.
// Returns y >= 0 if valid, -1 if piece spills off the board.
// Constant-time per piece column — replaces a 20-iteration Player_Valid loop.
static i8 FindLandingY(const Player* p, u8 pieceIdx, u8 rot, i8 x) {
    const PieceRot* s = Piece_GetRot(pieceIdx, rot);
    i8 bestY = 100;
    u8 c, r;

    for (c = 0; c < s->w; c++) {
        u8 botPr = 0xFF;
        i8 bc = x + (i8)c;
        for (r = 0; r < s->h; r++)
            if (Piece_GetBit(s->bits, r, c)) botPr = r;
        if (botPr == 0xFF) continue;          // no piece cells in this column
        if (bc < 0 || bc >= BOARD_W) return -1;  // piece extends off-board
        {
            i8 yCol = (i8)(BOARD_H - 1 - p->aiBaseHeights[bc]) - (i8)botPr;
            if (yCol < bestY) bestY = yCol;
        }
    }
    if (bestY < 0 || bestY >= BOARD_H) return -1;
    return bestY;
}

// Precompute column heights and per-row filled counts from the current board.
// Called once per piece (at start of AI search). Without this, every EvalPos
// would need to rescan 160 board cells.
static void ComputeBaseState(Player* p) {
    u8 r, c;
    for (c = 0; c < BOARD_W; c++) {
        u8 found = 0;
        p->aiBaseHeights[c] = 0;
        for (r = 0; r < BOARD_H; r++) {
            if (p->board[r][c] && !found) {
                p->aiBaseHeights[c] = (u8)(BOARD_H - r);
                found = 1;
            }
        }
    }
    for (r = 0; r < BOARD_H; r++) {
        u8 cnt = 0;
        for (c = 0; c < BOARD_W; c++)
            if (p->board[r][c]) cnt++;
        p->aiBaseRowCount[r] = cnt;
    }
    p->aiBaseReady = 1;
}

// Evaluate placing piece at (rot, x, y) using precomputed base state.
// No full board scan — only iterates the 4x4 piece footprint.
// Extra holes are approximated as the air gap between piece bottom and the
// existing stack top in each column (ignores intra-piece gaps; accurate enough).
static i16 EvalPos(const Player* p, u8 pieceIdx, u8 rot, i8 x, i8 y) {
    const PieceRot* s = Piece_GetRot(pieceIdx, rot);
    u8 r, c;
    u8 heights[BOARD_W];
    u8 rowAdd[BOARD_H];
    i16 maxH = 0, bump = 0;
    i16 lines = 0;
    i16 extraHoles = 0;

    // Copy base heights (8 bytes) and clear rowAdd (20 bytes)
    for (c = 0; c < BOARD_W; c++) heights[c] = p->aiBaseHeights[c];
    for (r = 0; r < BOARD_H; r++) rowAdd[r] = 0;

    // Per-column pass: find topmost + bottommost piece row in each piece column,
    // update heights, estimate new holes from the air gap.
    for (c = 0; c < s->w; c++) {
        u8 topPr = 0xFF, botPr = 0xFF;
        i8 bc = x + (i8)c;
        if (bc < 0 || bc >= BOARD_W) continue;
        for (r = 0; r < s->h; r++) {
            if (Piece_GetBit(s->bits, r, c)) {
                if (topPr == 0xFF) topPr = r;
                botPr = r;
            }
        }
        if (topPr == 0xFF) continue;
        {
            i8 topRow = y + (i8)topPr;
            i8 botRow = y + (i8)botPr;
            u8 oldH = p->aiBaseHeights[bc];
            u8 oldTopRow = oldH ? (u8)(BOARD_H - oldH) : (u8)BOARD_H;
            u8 newH;
            if (topRow < 0) continue;
            // Air gap below piece bottom creates new holes
            if ((i8)(botRow + 1) < (i8)oldTopRow)
                extraHoles += (i16)((u8)oldTopRow - (u8)(botRow + 1));
            newH = (u8)(BOARD_H - topRow);
            if (newH > heights[bc]) heights[bc] = newH;
        }
    }

    // Count piece cells per board row (for line detection)
    for (r = 0; r < s->h; r++) {
        for (c = 0; c < s->w; c++) {
            if (Piece_GetBit(s->bits, r, c)) {
                i8 br = y + (i8)r;
                if (br >= 0 && br < BOARD_H) rowAdd[br]++;
            }
        }
    }

    // Line count: rows where base + piece additions fill the row
    for (r = 0; r < BOARD_H; r++) {
        if (rowAdd[r] == 0) continue;
        if ((u8)(p->aiBaseRowCount[r] + rowAdd[r]) >= BOARD_W) lines++;
    }

    // maxH
    for (c = 0; c < BOARD_W; c++)
        if ((i16)heights[c] > maxH) maxH = heights[c];

    // Bumpiness
    for (c = 0; c < BOARD_W - 1; c++) {
        i16 d = (i16)heights[c] - (i16)heights[c + 1];
        if (d < 0) d = -d;
        bump += d;
    }

    // Line score — goal-aware
    {
        i16 lineScore;
        if (maxH >= 15)                     lineScore = lines * 300;
        else if (lines == 0)                lineScore = 80;
        else if ((u8)lines < p->aiGoal)     lineScore = 20;
        else                                lineScore = 500 + lines * 200;
        return lineScore - extraHoles * 150 - bump * 20 - maxH * 10;
    }
}

// AI: search is spread across multiple frames to avoid per-piece stalls.
// Each call evaluates up to AI_BUDGET candidate positions, then executes one
// step of the currently-best plan (rotate/move/soft-drop).
#define AI_BUDGET 2

void Player_AI(Player* p, u8 frame) {
    (void)frame;
    if (p->dead || p->flashTimer > 0) return;

    // --- SEARCH PHASE (spread across frames) ---
    if (!p->aiComputed) {
        u8 numRots = g_Pieces[p->pieceIdx].numRots;
        u8 budget = AI_BUDGET;

        // One-shot: precompute base heights + row counts for this piece
        if (!p->aiBaseReady) {
            ComputeBaseState(p);
            budget--;  // base scan costs roughly one eval
        }

        while (budget-- > 0 && !p->aiComputed) {
            i8 xi = (i8)p->aiEvalCol - 1;   // columns: -1 .. BOARD_W-1
            u8 ri = p->aiEvalRot;
            i8 yi = FindLandingY(p, p->pieceIdx, ri, xi);

            if (yi >= 0) {
                i16 sc = EvalPos(p, p->pieceIdx, ri, xi, yi);
                if (sc > p->aiBestScore) {
                    p->aiBestScore = sc;
                    p->aiTargetX = xi;
                    p->aiTargetRot = ri;
                }
            }

            // Advance to next (rot, col)
            p->aiEvalCol++;
            if (p->aiEvalCol > BOARD_W) {
                p->aiEvalCol = 0;
                p->aiEvalRot++;
                if (p->aiEvalRot >= numRots)
                    p->aiComputed = 1;
            }
        }
        // Still searching — don't execute moves yet
        if (!p->aiComputed) return;
    }

    // --- EXECUTION PHASE: one step per call ---
    // AI never uses human soft-drop (too fast). Instead, once aligned, we
    // gently force one extra drop every aiDropDelay ticks — a steady cadence
    // that sits between natural fall and soft-drop.
    p->softDrop = 0;

    if (p->rot != p->aiTargetRot) {
        p->rot = p->aiTargetRot % g_Pieces[p->pieceIdx].numRots;
        p->aiDropCounter = 0;
    }
    else if (p->px < p->aiTargetX) {
        Player_Move(p, 1);
        p->aiDropCounter = 0;
    }
    else if (p->px > p->aiTargetX) {
        Player_Move(p, -1);
        p->aiDropCounter = 0;
    }
    else {
        // Aligned — accumulate ticks and drop one row when threshold reached
        p->aiDropCounter++;
        if (p->aiDropCounter >= p->aiDropDelay) {
            p->aiDropCounter = 0;
            Player_Drop(p);
        }
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

    // If the target's piece is at landing position (can't drop further), lock
    // it first so it becomes part of the board and rides the shift up with
    // the rest. Otherwise it would appear stuck inside the new garbage.
    // Skip if already mid-flash (piece already locked, waiting to clear).
    if (p->flashTimer == 0 &&
        !Player_Valid(p, p->pieceIdx, p->rot, p->px, p->py + 1)) {
        Player_Lock(p);
        if (p->dead) return;
    }

    p->boardDirty = 1;  // mark dirty BEFORE any death check so partial shifts render

    // If target is mid-flash, shift their flashRows to match the upward shift.
    // Rows pushed off the top are dropped; if all drop, cancel the flash.
    if (p->flashTimer > 0) {
        u8 k;
        u8 newCount = 0;
        for (k = 0; k < p->flashCount; k++) {
            i8 nr = (i8)p->flashRows[k] - (i8)count;
            if (nr >= 0)
                p->flashRows[newCount++] = (u8)nr;
        }
        p->flashCount = newCount;
        if (newCount == 0) p->flashTimer = 0;
    }

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

u8 Game_CheckWinner(void) {
    u8 i, alive = 0, last = 0;
    for (i = 0; i < NUM_PLAYERS; i++) {
        if (!g_Players[i].dead) { alive++; last = i; }
    }
    return (alive == 1) ? last : 0xFF;
}
