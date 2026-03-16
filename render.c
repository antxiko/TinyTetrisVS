// ____________________________________________________________________________
// VDP rendering — TiniTetris 4P
// Screen 2 (GRAPHIC2) — ULTRA MINIMAL WRITES
//
// Strategy: NEVER scan the full board. Only touch cells that changed:
//   1. Piece moved: erase old ~4 cells, draw new ~4 cells
//   2. Board lock: only redraw rows where piece landed (~2-4 rows × 8 cols)
//   3. Header: only update changed numbers
//   4. No ghost piece (saves VRAM + CPU)
//
// Typical frame: 0-8 VRAM writes when piece moves, 0 when nothing changes.
// ____________________________________________________________________________

#include "msxgl.h"
#include "tiles.h"
#include "game.h"

#define VRAM_PT    0x0000
#define VRAM_NT    0x1800
#define VRAM_CT    0x2000
#define VRAM_SAT   0x1B00
#define VRAM_SPT   0x3800
#define BANK_SIZE  2048

static u8 g_CBuf[8];

// Arrow sprite pattern (8x8, points down)
static const u8 g_SprArrow[8] = {
    0x00, // ........
    0x7E, // .██████.
    0x3C, // ..████..
    0x3C, // ..████..
    0x18, // ...██...
    0x18, // ...██...
    0x00, // ........
    0x00  // ........
};

// Sprite colors per player (matches block colors)
static const u8 g_SprColor[4] = {
    COLOR_CYAN, COLOR_LIGHT_RED, COLOR_LIGHT_GREEN, COLOR_LIGHT_YELLOW
};

// ============================================================================
// Per-player tracking
// ============================================================================
typedef struct {
    i8  px, py;          // Previous piece position
    u8  rot;             // Previous rotation
    u8  pieceIdx;        // Previous piece index (0xFF = none drawn)
    u16 score;
    u8  lines;
    u8  dead;
    u8  headerDrawn;
    u8  flashState;      // 0=none, 1=white, 2=blocks
    u8  lockH;           // Initialized flag
} RenderState;

static RenderState g_RS[NUM_PLAYERS];

// ============================================================================
// VRAM HELPERS
// ============================================================================

static void PutTile(u8 x, u8 y, const u8* pat, const u8* col) {
    u16 bank = (u16)(y >> 3) * BANK_SIZE;
    u16 tile = (u16)(y & 7) * 32 + x;
    VDP_WriteVRAM_16K(pat, VRAM_PT + bank + (tile << 3), 8);
    VDP_WriteVRAM_16K(col, VRAM_CT + bank + (tile << 3), 8);
}

static void PutEmpty(u8 sx, u8 sy, u8 pIdx) {
    Tiles_ColorEmpty(g_CBuf, pIdx);
    PutTile(sx, sy, g_PatEmpty, g_CBuf);
}

static void PutBlock(u8 sx, u8 sy, u8 ownerIdx) {
    Tiles_ColorBlock(g_CBuf, ownerIdx);
    PutTile(sx, sy, g_PatBlock, g_CBuf);
}

static void PutGarbage(u8 sx, u8 sy) {
    u8 i;
    // Gray block: white highlight, gray body, dark gray shadow
    for (i = 0; i < 7; i++)
        g_CBuf[i] = (u8)((COLOR_WHITE << 4) | COLOR_GRAY);
    g_CBuf[7] = (u8)((COLOR_DARK_YELLOW << 4) | COLOR_GRAY);
    PutTile(sx, sy, g_PatBlock, g_CBuf);
}

static void PutChar(u8 x, u8 y, u8 ch, u8 pIdx, u8 bright) {
    u8 idx = (ch >= 32 && ch < 32 + FONT_COUNT) ? ch - 32 : 0;
    if (bright) Tiles_ColorBright(g_CBuf, pIdx);
    else Tiles_ColorText(g_CBuf, pIdx);
    PutTile(x, y, g_Font[idx], g_CBuf);
}

static void PutNum(u8 x, u8 y, u16 val, u8 w, u8 pIdx) {
    u8 buf[6];
    u8 n = 0;
    u8 i;
    if (val == 0) { buf[n++] = 0; }
    else { while (val > 0 && n < 6) { buf[n++] = (u8)(val % 10); val /= 10; } }
    // buf[0]=units, buf[1]=tens, etc. Print right-aligned with leading zeros.
    for (i = 0; i < w; i++) {
        u8 pos = w - 1 - i;  // rightmost digit first
        u8 d = (i < n) ? buf[i] : 0;
        PutChar(x + pos, y, '0' + d, pIdx, 1);
    }
}

// Draw board cell at (r,c) for player — reads from game board directly
static void DrawBoardCell(u8 pIdx, u8 r, u8 c) {
    u8 sx = pIdx * 8 + c;
    u8 sy = r + HEADER_ROWS;
    u8 val = g_Players[pIdx].board[r][c];
    if (val) {
        // val 1-4 = player colors, val 5 = garbage (render as gray)
        if (val <= NUM_PLAYERS) PutBlock(sx, sy, val - 1);
        else PutGarbage(sx, sy);
    }
    else PutEmpty(sx, sy, pIdx);
}

// ============================================================================
// INIT
// ============================================================================

void Render_Init(void) {
    u8 row, col, i;
    u8 buf[32];

    VDP_SetMode(VDP_MODE_GRAPHIC2);
    VDP_SetColor(0x11);
    VDP_FillVRAM_16K(0x00, 0x0000, 0x4000);

    // Name Table identity mapping
    for (row = 0; row < 24; row++) {
        u8 bankRow = row & 7;
        for (col = 0; col < 32; col++)
            buf[col] = bankRow * 32 + col;
        VDP_WriteVRAM_16K(buf, VRAM_NT + (u16)row * 32, 32);
    }

    // Set up sprites: 8x8 mode
    VDP_SetSpriteFlag(VDP_SPRITE_SIZE_8);
    VDP_SetSpriteTables(VRAM_SPT, VRAM_SAT);
    // Write arrow pattern as sprite pattern 0
    VDP_WriteVRAM_16K(g_SprArrow, VRAM_SPT, 8);
    // Hide all 32 sprites (Y=208 stops sprite processing on TMS9918A)
    {
        u8 spr;
        for (spr = 0; spr < 32; spr++)
            VDP_SetSpriteSM1(spr, 0, 208, 0, 0);
    }

    for (i = 0; i < NUM_PLAYERS; i++) {
        g_RS[i].px = -99;
        g_RS[i].py = -99;
        g_RS[i].rot = 0xFF;
        g_RS[i].pieceIdx = 0xFF;
        g_RS[i].score = 0xFFFF;
        g_RS[i].lines = 0xFF;
        g_RS[i].dead = 0xFF;
        g_RS[i].headerDrawn = 0;
        g_RS[i].flashState = 0;
        g_RS[i].lockH = 0;
    }
}

// ============================================================================
// HEADER — only changed fields
// ============================================================================

static void RenderHeader(const Player* p, u8 pIdx) {
    u8 sx = pIdx * 8;
    RenderState* rs = &g_RS[pIdx];

    if (!rs->headerDrawn) {
        u8 c;
        // Row 0: "P1" to "P4"
        PutChar(sx, 0, 'P', pIdx, 1);
        PutChar(sx + 1, 0, '1' + pIdx, pIdx, 1);
        for (c = 2; c < 8; c++)
            PutEmpty(sx + c, 0, pIdx);
        // Row 1: score (updated below)
        for (c = 0; c < 8; c++)
            PutEmpty(sx + c, 1, pIdx);
        // Row 2: "L" + lines count
        PutChar(sx, 2, 'L', pIdx, 0);
        for (c = 1; c < 8; c++)
            PutEmpty(sx + c, 2, pIdx);
        // Row 3: separator
        Tiles_ColorSeparator(g_CBuf, pIdx);
        for (c = 0; c < 8; c++)
            PutTile(sx + c, 3, g_PatSolid, g_CBuf);
        rs->headerDrawn = 1;
    }
    if (p->score != rs->score) {
        PutNum(sx + 1, 1, p->score, 5, pIdx);
        rs->score = p->score;
    }
    if (p->lines != rs->lines) {
        PutNum(sx + 1, 2, p->lines, 3, pIdx);
        rs->lines = p->lines;
    }
}

// ============================================================================
// ERASE piece shape at given position (restore board cells)
// ============================================================================

static void ErasePieceAt(u8 pIdx, u8 pieceIdx, u8 rot, i8 px, i8 py) {
    const PieceRot* s = Piece_GetRot(pieceIdx, rot);
    u8 r, c;
    for (r = 0; r < s->h; r++) {
        for (c = 0; c < s->w; c++) {
            if (Piece_GetBit(s->bits, r, c)) {
                i8 br = py + (i8)r;
                i8 bc = px + (i8)c;
                if (br >= 0 && br < BOARD_H && bc >= 0 && bc < BOARD_W)
                    DrawBoardCell(pIdx, (u8)br, (u8)bc);
            }
        }
    }
}

// ============================================================================
// DRAW piece shape at position
// ============================================================================

static void DrawPieceAt(u8 pIdx, u8 pieceIdx, u8 rot, i8 px, i8 py) {
    const PieceRot* s = Piece_GetRot(pieceIdx, rot);
    u8 sx = pIdx * 8;
    u8 r, c;
    for (r = 0; r < s->h; r++) {
        for (c = 0; c < s->w; c++) {
            if (Piece_GetBit(s->bits, r, c)) {
                i8 br = py + (i8)r;
                i8 bc = px + (i8)c;
                if (br >= 0 && br < BOARD_H && bc >= 0 && bc < BOARD_W)
                    PutBlock(sx + (u8)bc, (u8)br + HEADER_ROWS, pIdx);
            }
        }
    }
}

// ============================================================================
// FLASH — only update when flash state changes
// ============================================================================

static void RenderFlash(const Player* p, u8 pIdx) {
    RenderState* rs = &g_RS[pIdx];
    u8 newState;
    u8 fi, c;
    u8 sx = pIdx * 8;

    newState = ((p->flashTimer % FLASH_TOGGLE) < (FLASH_TOGGLE / 2)) ? 1 : 2;
    if (newState == rs->flashState) return; // No visual change

    rs->flashState = newState;
    for (fi = 0; fi < p->flashCount; fi++) {
        u8 row = p->flashRows[fi];
        u8 sy = row + HEADER_ROWS;
        if (newState == 1) {
            Tiles_ColorFlash(g_CBuf);
            for (c = 0; c < BOARD_W; c++)
                PutTile(sx + c, sy, g_PatSolid, g_CBuf);
        } else {
            for (c = 0; c < BOARD_W; c++) {
                u8 val = p->board[row][c];
                if (val) {
                    if (val <= NUM_PLAYERS) PutBlock(sx + c, sy, val - 1);
                    else PutGarbage(sx + c, sy);
                }
                else PutEmpty(sx + c, sy, pIdx);
            }
        }
    }
}

// ============================================================================
// RENDER PLAYER — minimal per-frame work
// ============================================================================

static void RenderPlayer(const Player* p, u8 pIdx) {
    RenderState* rs = &g_RS[pIdx];
    u8 pieceMoved;
    u8 pieceChanged;

    // Game over — one shot
    if (p->dead) {
        if (rs->dead != 1) {
            u8 sx = pIdx * 8;
            u8 r, c, i;
            u8 colDR[8], colTxt[8];
            for (i = 0; i < 8; i++) {
                colDR[i] = (u8)((COLOR_DARK_RED << 4) | COLOR_DARK_RED);
                colTxt[i] = (u8)((COLOR_WHITE << 4) | COLOR_DARK_RED);
            }
            for (r = 0; r < 24; r++)
                for (c = 0; c < 8; c++)
                    PutTile(sx + c, r, g_PatEmpty, colDR);
            PutTile(sx+2, 11, g_Font['G'-32], colTxt);
            PutTile(sx+3, 11, g_Font['A'-32], colTxt);
            PutTile(sx+4, 11, g_Font['M'-32], colTxt);
            PutTile(sx+5, 11, g_Font['E'-32], colTxt);
            PutTile(sx+2, 12, g_Font['O'-32], colTxt);
            PutTile(sx+3, 12, g_Font['V'-32], colTxt);
            PutTile(sx+4, 12, g_Font['E'-32], colTxt);
            PutTile(sx+5, 12, g_Font['R'-32], colTxt);
            rs->dead = 1;
        }
        return;
    }

    // Header
    RenderHeader(p, pIdx);

    // Garbage pushed board — full redraw
    if (p->boardDirty) {
        u8 r, c;
        // Erase piece overlay first
        if (rs->pieceIdx != 0xFF) {
            ErasePieceAt(pIdx, rs->pieceIdx, rs->rot, rs->px, rs->py);
            rs->pieceIdx = 0xFF;
        }
        for (r = 0; r < BOARD_H; r++)
            for (c = 0; c < BOARD_W; c++)
                DrawBoardCell(pIdx, r, c);
        ((Player*)p)->boardDirty = 0;
    }

    // Flash ended? (flashState was set but flashTimer is now 0)
    if (rs->flashState != 0 && p->flashTimer == 0) {
        u8 r, c;
        rs->flashState = 0;
        // ClearLines shifted the board — redraw everything
        for (r = 0; r < BOARD_H; r++)
            for (c = 0; c < BOARD_W; c++)
                DrawBoardCell(pIdx, r, c);
        rs->pieceIdx = 0xFF; // Force piece redraw
    }

    // Flash in progress
    if (p->flashTimer > 0) {
        // During flash: erase piece overlay if it was drawn
        if (rs->pieceIdx != 0xFF) {
            ErasePieceAt(pIdx, rs->pieceIdx, rs->rot, rs->px, rs->py);
            rs->pieceIdx = 0xFF; // Mark as not drawn
        }
        RenderFlash(p, pIdx);
        return;
    }

    // Detect if piece changed (piece was locked and new one spawned)
    pieceChanged = (p->pieceIdx != rs->pieceIdx && rs->pieceIdx != 0xFF);

    if (pieceChanged) {
        // Piece was locked — only redraw the rows where old piece was
        // (board cells already updated by game logic)
        const PieceRot* oldS = Piece_GetRot(rs->pieceIdx, rs->rot);
        u8 r, c;
        i8 minR = rs->py;
        i8 maxR = rs->py + (i8)oldS->h - 1;
        if (minR < 0) minR = 0;
        if (maxR >= BOARD_H) maxR = BOARD_H - 1;
        for (r = (u8)minR; r <= (u8)maxR; r++)
            for (c = 0; c < BOARD_W; c++)
                DrawBoardCell(pIdx, r, c);
        rs->pieceIdx = 0xFF; // Force piece redraw below
    }

    // First render ever — draw full board
    if (rs->pieceIdx == 0xFF && !pieceChanged && rs->lockH == 0) {
        // Initial state — full board render
        u8 r, c;
        for (r = 0; r < BOARD_H; r++)
            for (c = 0; c < BOARD_W; c++)
                DrawBoardCell(pIdx, r, c);
    }

    // Check if piece actually moved
    pieceMoved = (p->px != rs->px || p->py != rs->py ||
                  p->rot != rs->rot || p->pieceIdx != rs->pieceIdx);

    if (!pieceMoved) return; // Nothing changed — 0 VRAM writes!

    // Erase old piece overlay (restore board cells)
    if (rs->pieceIdx != 0xFF) {
        ErasePieceAt(pIdx, rs->pieceIdx, rs->rot, rs->px, rs->py);
    }

    // Draw new piece
    DrawPieceAt(pIdx, p->pieceIdx, p->rot, p->px, p->py);

    // Save state
    rs->px = p->px;
    rs->py = p->py;
    rs->rot = p->rot;
    rs->pieceIdx = p->pieceIdx;
    rs->lockH = 1; // Mark as initialized
}

// ============================================================================
// PUBLIC
// ============================================================================

void Render_Frame(void) {
    u8 i;
    for (i = 0; i < NUM_PLAYERS; i++)
        RenderPlayer(&g_Players[i], i);

    // Update targeting sprites (one per player)
    // Each player's arrow goes on top of the targeted player's strip.
    // Stagger X within the 64px strip so multiple arrows don't overlap:
    // P0 at col 1, P1 at col 3, P2 at col 5, P3 at col 7 (×8px each)
    {
        static const u8 sprXOff[4] = { 4, 20, 36, 52 }; // px offset within strip
        for (i = 0; i < NUM_PLAYERS; i++) {
            if (g_Players[i].dead) {
                VDP_SetSpriteSM1(i, 0, 208, 0, 0);
            } else {
                u8 tgt = g_Players[i].targetPlayer;
                u8 sy = (u8)(HEADER_ROWS * 8 - 9);
                u8 sx = (u8)(tgt * 64 + sprXOff[i]);
                VDP_SetSpriteSM1(i, sx, sy, 0, g_SprColor[i]);
            }
        }
    }
}

// ============================================================================
// VICTORY SCREEN — one-shot, called once when winner is determined
// ============================================================================

// ============================================================================
// TITLE SCREEN
// ============================================================================

// Draw a character from g_Font as big blocks (each pixel = 1 block tile)
// The 3x5 font data sits in bits 5,4,3 of rows 1-5
static void DrawBigChar(u8 x, u8 y, u8 ch, u8 colorIdx) {
    u8 idx = (ch >= 32 && ch < 32 + FONT_COUNT) ? ch - 32 : 0;
    const u8* pat = g_Font[idx];
    u8 r, c;
    Tiles_ColorBlock(g_CBuf, colorIdx);
    for (r = 0; r < 5; r++) {
        u8 row = pat[r + 1]; // rows 1-5 have the data
        for (c = 0; c < 3; c++) {
            if (row & (0x20 >> c))
                PutTile(x + c, y + r, g_PatBlock, g_CBuf);
        }
    }
}

void Render_TitleScreen(void) {
    u8 r, c, i, x;
    u8 colBg[8], colTxt[8];

    // Black background
    for (i = 0; i < 8; i++)
        colBg[i] = (u8)((COLOR_BLACK << 4) | COLOR_BLACK);
    for (r = 0; r < 24; r++)
        for (c = 0; c < 32; c++)
            PutTile(c, r, g_PatEmpty, colBg);

    // "TINY" in big blocks — row 1, each letter 3 wide + 1 gap = 15 tiles
    // Colored like Tetris pieces: T=cyan(P1), I=red(P2), N=green(P3), Y=yellow(P4)
    x = 8;
    DrawBigChar(x,    1, 'T', 0);  // cyan
    DrawBigChar(x+4,  1, 'I', 1);  // red
    DrawBigChar(x+8,  1, 'N', 2);  // green
    DrawBigChar(x+12, 1, 'Y', 3);  // yellow

    // "TETRIS" in big blocks — row 7, 6 letters × 4 = 23 tiles
    x = 4;
    DrawBigChar(x,    7, 'T', 1);  // red
    DrawBigChar(x+4,  7, 'E', 0);  // cyan
    DrawBigChar(x+8,  7, 'T', 3);  // yellow
    DrawBigChar(x+12, 7, 'R', 2);  // green
    DrawBigChar(x+16, 7, 'I', 1);  // red
    DrawBigChar(x+20, 7, 'S', 0);  // cyan

    // "VS" in big blocks — row 13, centered
    x = 12;
    DrawBigChar(x,   13, 'V', 3);  // yellow
    DrawBigChar(x+4, 13, 'S', 1);  // red

    // Player slots in their quadrants (each 8 tiles wide)
    // Row 20: "Px", Row 21: "PRESS A"
    for (i = 0; i < NUM_PLAYERS; i++) {
        u8 colP[8];
        const PlayerColors* pc = &g_PlayerColors[i];
        u8 sx = i * 8;

        for (c = 0; c < 8; c++)
            colP[c] = (u8)((pc->block << 4) | COLOR_BLACK);
        for (c = 0; c < 8; c++)
            colTxt[c] = (u8)((pc->text << 4) | COLOR_BLACK);

        // "Px" centered in strip
        PutTile(sx + 3, 19, g_Font['P'-32], colP);
        PutTile(sx + 4, 19, g_Font['1'+i-32], colP);

        // "PRESS" on row 21
        PutTile(sx + 1, 21, g_Font['P'-32], colTxt);
        PutTile(sx + 2, 21, g_Font['R'-32], colTxt);
        PutTile(sx + 3, 21, g_Font['E'-32], colTxt);
        PutTile(sx + 4, 21, g_Font['S'-32], colTxt);
        PutTile(sx + 5, 21, g_Font['S'-32], colTxt);

        // "A" on row 22
        PutTile(sx + 3, 22, g_Font['A'-32], colP);
    }
}

void Render_TitleReady(u8 pIdx) {
    u8 colR[8], colBg[8], i, x;
    const PlayerColors* pc = &g_PlayerColors[pIdx];
    u8 sx = pIdx * 8;

    for (i = 0; i < 8; i++) {
        colR[i] = (u8)((pc->block << 4) | COLOR_BLACK);
        colBg[i] = (u8)((COLOR_BLACK << 4) | COLOR_BLACK);
    }

    // Clear "PRESS" and "A" rows
    for (x = sx; x < sx + 8; x++) {
        PutTile(x, 21, g_PatEmpty, colBg);
        PutTile(x, 22, g_PatEmpty, colBg);
    }

    // Write "READY" in player block color on row 21
    PutTile(sx + 1, 21, g_Font['R'-32], colR);
    PutTile(sx + 2, 21, g_Font['E'-32], colR);
    PutTile(sx + 3, 21, g_Font['A'-32], colR);
    PutTile(sx + 4, 21, g_Font['D'-32], colR);
    PutTile(sx + 5, 21, g_Font['Y'-32], colR);
}

void Render_Victory(u8 winnerIdx) {
    u8 r, c, i, sx;
    u8 colWin[8], colTxt[8], colBlock[8];
    const PlayerColors* pc = &g_PlayerColors[winnerIdx];

    // Hide all sprites
    for (i = 0; i < 32; i++)
        VDP_SetSpriteSM1(i, 0, 208, 0, 0);

    // Fill entire screen with winner's bg color
    for (i = 0; i < 8; i++)
        colWin[i] = (u8)((pc->bg << 4) | pc->bg);
    for (r = 0; r < 24; r++)
        for (c = 0; c < 32; c++)
            PutTile(c, r, g_PatEmpty, colWin);

    // Text colors
    for (i = 0; i < 8; i++) {
        colTxt[i] = (u8)((COLOR_WHITE << 4) | pc->bg);
        colBlock[i] = (u8)((pc->block << 4) | pc->bg);
    }

    // "PLAYER" centered at row 9
    sx = 12;
    PutTile(sx+0, 9, g_Font['P'-32], colTxt);
    PutTile(sx+1, 9, g_Font['L'-32], colTxt);
    PutTile(sx+2, 9, g_Font['A'-32], colTxt);
    PutTile(sx+3, 9, g_Font['Y'-32], colTxt);
    PutTile(sx+4, 9, g_Font['E'-32], colTxt);
    PutTile(sx+5, 9, g_Font['R'-32], colTxt);
    // Player number
    PutTile(sx+7, 9, g_Font['1' + winnerIdx - 32], colBlock);

    // "WINS" at row 11 in block color
    sx = 14;
    PutTile(sx+0, 11, g_Font['W'-32], colBlock);
    PutTile(sx+1, 11, g_Font['I'-32], colBlock);
    PutTile(sx+2, 11, g_Font['N'-32], colBlock);
    PutTile(sx+3, 11, g_Font['S'-32], colBlock);
}
