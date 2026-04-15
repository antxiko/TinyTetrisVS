// ____________________________________________________________________________
// Game logic for TiniTetris 4P
// ____________________________________________________________________________
#pragma once

#include "msxgl.h"

// ============================================================================
// CONSTANTS
// ============================================================================
#define NUM_PLAYERS     4
#define BOARD_W         8
#define BOARD_H         20
#define NUM_PIECES      7
#define HEADER_ROWS     4

#define FLASH_DURATION  20
#define FLASH_TOGGLE    4

#define DROP_INITIAL    50   // ~1 second at 50Hz
#define DROP_MIN        5
#define DROP_ACCEL      4
#define LINES_PER_LVL   10

// ============================================================================
// PIECE DATA — packed as bitmask in 4×4 grid (u16)
// ============================================================================
typedef struct {
    u8  w;
    u8  h;
    u16 bits;  // Row 0 = bits 15..12, Row 1 = 11..8, etc.
} PieceRot;

typedef struct {
    u8       numRots;
    PieceRot rots[4];
} PieceDef;

// ============================================================================
// PLAYER STATE
// ============================================================================
typedef struct {
    u8  board[BOARD_H][BOARD_W]; // 0=empty, 1-4=player who placed
    u16 score;
    u8  level;
    u8  lines;
    u8  dead;           // bool

    u8  pieceIdx;
    u8  rot;
    i8  px, py;
    u8  nextIdx;

    u8  dropTimer;
    u8  dropInterval;
    u8  softDrop;       // bool — held down = fast drop
    u8  flashTimer;
    u8  flashRows[4];
    u8  flashCount;

    // AI
    i8  aiTargetX;
    u8  aiTargetRot;
    u8  aiComputed;     // bool — set when search is finished
    u8  aiGoal;         // lines the AI is trying to clear this piece (1/2/3/4)
    u8  aiDropDelay;    // ticks between AI-forced drops after alignment (per piece)
    u8  aiDropCounter;  // counts up while aligned; triggers drop when == aiDropDelay
    u8  aiEvalRot;      // current rotation being searched
    u8  aiEvalCol;      // current column index being searched (0..BOARD_W)
    u8  aiBaseReady;    // bool — base heights/rowCount have been computed
    i16 aiBestScore;    // best score found so far during the spread search
    u8  aiBaseHeights[BOARD_W];  // precomputed column heights
    u8  aiBaseRowCount[BOARD_H]; // precomputed filled-cells per row

    // Targeting
    u8  targetPlayer;   // index of player we're attacking (0-3)
    u8  boardDirty;     // set by garbage — forces full board redraw
} Player;

// ============================================================================
// API
// ============================================================================
void Game_Init(void);
u8   Game_CheckWinner(void);  // Returns winner index (0-3) or 0xFF if game ongoing
void Player_Spawn(Player* p);
u8   Piece_GetBit(u16 bits, u8 row, u8 col);
const PieceRot* Piece_GetRot(u8 pieceIdx, u8 rot);
u8   Player_Valid(const Player* p, u8 pieceIdx, u8 rot, i8 x, i8 y);
void Player_Move(Player* p, i8 dx);
void Player_Rotate(Player* p);
void Player_Drop(Player* p);
void Player_Update(Player* p);
void Player_AI(Player* p, u8 frame);
void Player_CycleTarget(Player* p, u8 pIdx);
void Player_AddGarbage(Player* p, u8 count);

extern Player g_Players[NUM_PLAYERS];
extern u8     g_Frame;
extern u8     g_HumanMask;  // bit i = 1 → player i is human, else AI
extern const PieceDef g_Pieces[NUM_PIECES];
