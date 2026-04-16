// ____________________________________________________________________________
// VDP rendering — TiniTetris 4P
// Screen 2 (GRAPHIC2) — DOUBLE-BUFFERED via g_NameBuffer[768]
//
// Architecture:
//   - FIXED tiles (0-22) pre-baked in VRAM with pattern+color. Replicated
//     across all 3 banks so any row can reference them.
//   - Gameplay name table layout:
//       Rows 0-3 (header): identity with +128 offset → dynamic text uses
//                          bank 0 tiles 128-255 (one unique slot per cell).
//       Rows 4-23 (board): fixed tile indices (0-22). Only name table
//                          bytes change per cell.
//   - All board drawing writes to g_NameBuffer[768] (RAM), not VRAM.
//   - Once per frame: blast rows 4-23 (640 bytes) to VRAM in one call.
//   - Title/victory screens use full identity mapping + dynamic writes
//     (one-shot, not per-frame).
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

// ============================================================================
// DOUBLE BUFFER
// ============================================================================
static u8  g_NameBuffer[768];   // 32x24 name table mirror
static u8  g_DirtyRows[BOARD_H]; // 1 if row (board row offset) needs flushing
static u8  g_Mode;              // 0=title/victory (identity), 1=gameplay (fixed)
static u8  g_CBuf[8];

// Arrow sprite pattern (8x8, points down)
static const u8 g_SprArrow[8] = {
    0x00, 0x7E, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x00
};

static const u8 g_SprColor[4] = {
    COLOR_CYAN, COLOR_LIGHT_RED, COLOR_LIGHT_GREEN, COLOR_LIGHT_YELLOW
};

// ============================================================================
// Per-player tracking
// ============================================================================
typedef struct {
    i8  px, py;
    u8  rot;
    u8  pieceIdx;
    u16 score;
    u8  lines;
    u8  level;
    u8  nextIdx;
    i8  ghostY;         // last-rendered ghost y (0xFF = not drawn)
    u8  dead;
    u8  headerDrawn;
    u8  flashState;
    u8  lockH;
} RenderState;

static RenderState g_RS[NUM_PLAYERS];

// ============================================================================
// LOW-LEVEL VRAM helpers
// ============================================================================

// Compute VRAM offset for tile data (pattern or color) for a given row/tile
static u16 TileAddr(u8 y, u8 tileIdx) {
    u16 bank = (u16)(y >> 3) * BANK_SIZE;
    return bank + ((u16)tileIdx << 3);
}

// Write pattern + color for a specific tile slot (used by header/text rendering)
static void WriteTileData(u8 y, u8 tileIdx, const u8* pat, const u8* col) {
    u16 addr = TileAddr(y, tileIdx);
    VDP_WriteVRAM_16K(pat, VRAM_PT + addr, 8);
    VDP_WriteVRAM_16K(col, VRAM_CT + addr, 8);
}

// ============================================================================
// BUILD FIXED TILES — writes the 23 fixed tile patterns+colors to VRAM,
// replicated in all 3 banks so they work on any row.
// ============================================================================

static void BuildOneTile(u8 bank, u8 tileIdx, const u8* pat, const u8* col) {
    u16 addr = (u16)bank * BANK_SIZE + ((u16)tileIdx << 3);
    VDP_WriteVRAM_16K(pat, VRAM_PT + addr, 8);
    VDP_WriteVRAM_16K(col, VRAM_CT + addr, 8);
}

static void BuildFixedTiles(void) {
    u8 bank, i;
    u8 col[8];
    u8 darkRedCol[8], darkRedTxt[8];

    // Prepare "dark red bg" colors for dead tiles
    for (i = 0; i < 8; i++) {
        darkRedCol[i] = (u8)((COLOR_DARK_RED << 4) | COLOR_DARK_RED);
        darkRedTxt[i] = (u8)((COLOR_WHITE    << 4) | COLOR_DARK_RED);
    }

    for (bank = 0; bank < 3; bank++) {
        // TILE_EMPTY_BLACK — black bg, empty pattern
        for (i = 0; i < 8; i++)
            col[i] = (u8)((COLOR_BLACK << 4) | COLOR_BLACK);
        BuildOneTile(bank, TILE_EMPTY_BLACK, g_PatEmpty, col);

        // TILE_EMPTY_P0..P3 — player bg, empty pattern
        for (i = 0; i < NUM_PLAYERS; i++) {
            Tiles_ColorEmpty(col, i);
            BuildOneTile(bank, TILE_EMPTY_P0 + i, g_PatEmpty, col);
        }

        // TILE_BLOCK_P0..P3 — player block
        for (i = 0; i < NUM_PLAYERS; i++) {
            Tiles_ColorBlock(col, i);
            BuildOneTile(bank, TILE_BLOCK_P0 + i, g_PatBlock, col);
        }

        // TILE_GARBAGE — gray block
        for (i = 0; i < 7; i++)
            col[i] = (u8)((COLOR_WHITE << 4) | COLOR_GRAY);
        col[7] = (u8)((COLOR_DARK_YELLOW << 4) | COLOR_GRAY);
        BuildOneTile(bank, TILE_GARBAGE, g_PatBlock, col);

        // TILE_FLASH — solid white
        Tiles_ColorFlash(col);
        BuildOneTile(bank, TILE_FLASH, g_PatSolid, col);

        // TILE_SEP_P0..P3 — separator (solid block color)
        for (i = 0; i < NUM_PLAYERS; i++) {
            Tiles_ColorSeparator(col, i);
            BuildOneTile(bank, TILE_SEP_P0 + i, g_PatSolid, col);
        }

        // TILE_DEAD — dark red empty
        BuildOneTile(bank, TILE_DEAD, g_PatEmpty, darkRedCol);

        // TILE_DEAD_G..R — GAME OVER letters, white on dark red
        BuildOneTile(bank, TILE_DEAD_G, g_Font['G' - 32], darkRedTxt);
        BuildOneTile(bank, TILE_DEAD_A, g_Font['A' - 32], darkRedTxt);
        BuildOneTile(bank, TILE_DEAD_M, g_Font['M' - 32], darkRedTxt);
        BuildOneTile(bank, TILE_DEAD_E, g_Font['E' - 32], darkRedTxt);
        BuildOneTile(bank, TILE_DEAD_O, g_Font['O' - 32], darkRedTxt);
        BuildOneTile(bank, TILE_DEAD_V, g_Font['V' - 32], darkRedTxt);
        BuildOneTile(bank, TILE_DEAD_R, g_Font['R' - 32], darkRedTxt);

        // TILE_GHOST_P0..P3 — outline tiles for landing preview
        for (i = 0; i < NUM_PLAYERS; i++) {
            Tiles_ColorGhost(col, i);
            BuildOneTile(bank, TILE_GHOST_P0 + i, g_PatGhost, col);
        }
    }
}

// ============================================================================
// NAME TABLE INITIALIZERS
// ============================================================================

// Identity 0..255 per bank — for title/victory screens (dynamic per-cell)
static void SetupIdentityNameTable(void) {
    u16 i;
    for (i = 0; i < 768; i++)
        g_NameBuffer[i] = (u8)((i >> 5 & 7) * 32 + (i & 31));
    VDP_WriteVRAM_16K(g_NameBuffer, VRAM_NT, 768);
}

// Gameplay layout: rows 0-3 identity offset 128 (bank 0), rows 4-23 fixed
static void SetupGameNameTable(void) {
    u16 i;
    u8 r, c, pIdx;

    // Rows 0-3: header identity using tiles 128-255 of bank 0
    for (r = 0; r < 4; r++) {
        for (c = 0; c < 32; c++)
            g_NameBuffer[(u16)r * 32 + c] = (u8)(128 + r * 32 + c);
    }

    // Rows 4-23: init to TILE_EMPTY_Px based on player strip
    for (r = HEADER_ROWS; r < 24; r++) {
        for (c = 0; c < 32; c++) {
            pIdx = c >> 3;
            g_NameBuffer[(u16)r * 32 + c] = TILE_EMPTY_P0 + pIdx;
        }
    }

    // Place separators on row 3 (bottom of header) in bank 0 using identity tile
    // (the separator glyph is written by RenderHeader via WriteTileData)

    // Blast full name table to VRAM
    VDP_WriteVRAM_16K(g_NameBuffer, VRAM_NT, 768);

    // Ignore unused var warning
    (void)i;
}

// ============================================================================
// FLUSH — writes only the rows that actually changed to VRAM, as discrete
// 32-byte transfers. This keeps each burst small enough to fit inside VBlank
// (~5ms) and avoids VDP contention that drops writes during active display.
// ============================================================================

static void FlushBoard(void) {
    u8 r;
    for (r = 0; r < BOARD_H; r++) {
        if (g_DirtyRows[r]) {
            u16 off = (u16)(r + HEADER_ROWS) * 32;
            VDP_WriteVRAM_16K(&g_NameBuffer[off], VRAM_NT + off, 32);
            g_DirtyRows[r] = 0;
        }
    }
}

// ============================================================================
// HIGH-LEVEL helpers
// ============================================================================

// Header text cell — writes pattern+color to unique tile slot (rows 0-3 only)
// In gameplay mode, tile index = 128 + y*32 + x (bank 0).
static void PutChar(u8 x, u8 y, u8 ch, u8 pIdx, u8 bright) {
    u8 idx = (ch >= 32 && ch < 32 + FONT_COUNT) ? ch - 32 : 0;
    u8 tileIdx;
    if (bright) Tiles_ColorBright(g_CBuf, pIdx);
    else        Tiles_ColorText(g_CBuf, pIdx);

    if (g_Mode == 1 && y < HEADER_ROWS) {
        // Gameplay header: tile 128+y*32+x in bank 0
        tileIdx = (u8)(128 + y * 32 + x);
        WriteTileData(y, tileIdx, g_Font[idx], g_CBuf);
    } else {
        // Identity (title/victory/game-over full-screen): tile = (y&7)*32+x
        tileIdx = (u8)((y & 7) * 32 + x);
        WriteTileData(y, tileIdx, g_Font[idx], g_CBuf);
    }
}

static void PutNum(u8 x, u8 y, u16 val, u8 w, u8 pIdx) {
    u8 buf[6];
    u8 n = 0;
    u8 i;
    if (val == 0) { buf[n++] = 0; }
    else { while (val > 0 && n < 6) { buf[n++] = (u8)(val % 10); val /= 10; } }
    for (i = 0; i < w; i++) {
        u8 pos = w - 1 - i;
        u8 d = (i < n) ? buf[i] : 0;
        PutChar(x + pos, y, '0' + d, pIdx, 1);
    }
}

// Generic "write pattern+color" to a cell via identity — for title/victory
static void PutTileIdentity(u8 x, u8 y, const u8* pat, const u8* col) {
    u8 tileIdx = (u8)((y & 7) * 32 + x);
    WriteTileData(y, tileIdx, pat, col);
}

// Write a fixed board tile into NameBuffer (gameplay mode, rows 4-23)
// Skips no-op writes and marks the row as dirty for selective flushing.
static void PutBoard(u8 x, u8 y, u8 tileIdx) {
    u16 idx = (u16)y * 32 + x;
    if (g_NameBuffer[idx] == tileIdx) return;
    g_NameBuffer[idx] = tileIdx;
    if (y >= HEADER_ROWS) g_DirtyRows[y - HEADER_ROWS] = 1;
}

// ============================================================================
// INIT — called once at program start
// ============================================================================

void Render_Init(void) {
    u8 i;

    VDP_SetMode(VDP_MODE_GRAPHIC2);
    VDP_SetColor(0x11);
    VDP_FillVRAM_16K(0x00, 0x0000, 0x4000);

    // Sprites
    VDP_SetSpriteFlag(VDP_SPRITE_SIZE_8);
    VDP_SetSpriteTables(VRAM_SPT, VRAM_SAT);
    VDP_WriteVRAM_16K(g_SprArrow, VRAM_SPT, 8);
    for (i = 0; i < 32; i++)
        VDP_SetSpriteSM1(i, 0, 208, 0, 0);

    for (i = 0; i < NUM_PLAYERS; i++) {
        g_RS[i].px = -99; g_RS[i].py = -99;
        g_RS[i].rot = 0xFF;
        g_RS[i].pieceIdx = 0xFF;
        g_RS[i].score = 0xFFFF;
        g_RS[i].lines = 0xFF;
        g_RS[i].level = 0xFF;
        g_RS[i].nextIdx = 0xFF;
        g_RS[i].ghostY = -1;
        g_RS[i].dead = 0xFF;
        g_RS[i].headerDrawn = 0;
        g_RS[i].flashState = 0;
        g_RS[i].lockH = 0;
    }

    // Default to identity (title screen will call its own setup)
    g_Mode = 0;
    SetupIdentityNameTable();
}

// Called from main.c when entering gameplay (after title)
void Render_GameBegin(void) {
    u8 i;

    // Reset per-player render state so first frame does a full draw
    for (i = 0; i < NUM_PLAYERS; i++) {
        g_RS[i].px = -99; g_RS[i].py = -99;
        g_RS[i].rot = 0xFF;
        g_RS[i].pieceIdx = 0xFF;
        g_RS[i].score = 0xFFFF;
        g_RS[i].lines = 0xFF;
        g_RS[i].level = 0xFF;
        g_RS[i].nextIdx = 0xFF;
        g_RS[i].ghostY = -1;
        g_RS[i].dead = 0;
        g_RS[i].headerDrawn = 0;
        g_RS[i].flashState = 0;
        g_RS[i].lockH = 0;
    }

    // Clear VRAM pattern+color (title may have overwritten fixed tiles)
    VDP_FillVRAM_16K(0x00, VRAM_PT, BANK_SIZE * 3);
    VDP_FillVRAM_16K(0x00, VRAM_CT, BANK_SIZE * 3);

    BuildFixedTiles();
    g_Mode = 1;
    {
        u8 r;
        for (r = 0; r < BOARD_H; r++) g_DirtyRows[r] = 0;
    }
    SetupGameNameTable();
}

// Called from main.c when entering a full-screen dynamic screen (title/victory)
void Render_IdentityMode(void) {
    g_Mode = 0;
    VDP_FillVRAM_16K(0x00, VRAM_PT, BANK_SIZE * 3);
    VDP_FillVRAM_16K(0x00, VRAM_CT, BANK_SIZE * 3);
    SetupIdentityNameTable();
}

// ============================================================================
// HEADER — only changed fields (rows 0-3)
// ============================================================================

// Render the next-piece preview (4×2 area at rows 0-1, cols 4-7)
// Best rotation for 4x2 preview area: L and J need horizontal rotation
static const u8 g_PreviewRot[NUM_PIECES] = { 0, 0, 0, 0, 0, 1, 1 };

static void RenderNextPreview(u8 pIdx, u8 pieceIdx) {
    u8 sx = pIdx * 8 + 4;  // preview starts at col 4 of player strip
    const PieceRot* s = Piece_GetRot(pieceIdx, g_PreviewRot[pieceIdx]);
    u8 colBlock[8], colEmpty[8];
    u8 r, c;
    Tiles_ColorBlock(colBlock, pIdx);
    Tiles_ColorEmpty(colEmpty, pIdx);

    for (r = 0; r < 2; r++) {
        for (c = 0; c < 4; c++) {
            u8 tileIdx = (u8)(128 + r * 32 + sx + c);
            u8 bit = (r < s->h && c < s->w) ? Piece_GetBit(s->bits, r, c) : 0;
            if (bit) WriteTileData(r, tileIdx, g_PatBlock, colBlock);
            else     WriteTileData(r, tileIdx, g_PatEmpty, colEmpty);
        }
    }
}

static void RenderHeader(const Player* p, u8 pIdx) {
    u8 sx = pIdx * 8;
    RenderState* rs = &g_RS[pIdx];

    if (!rs->headerDrawn) {
        u8 c;
        // Row 0: "P1".."P4" at cols 0-1, empty at 2-3, preview top 4-7 (filled below)
        PutChar(sx, 0, 'P', pIdx, 1);
        PutChar(sx + 1, 0, '1' + pIdx, pIdx, 1);
        for (c = 2; c < 4; c++) {
            Tiles_ColorEmpty(g_CBuf, pIdx);
            WriteTileData(0, (u8)(128 + sx + c), g_PatEmpty, g_CBuf);
        }
        // Row 1: score at cols 0-3 (4 digits), preview bot at 4-7 (filled below)
        // Clear score area initially
        for (c = 0; c < 4; c++) {
            Tiles_ColorEmpty(g_CBuf, pIdx);
            WriteTileData(1, (u8)(128 + 32 + sx + c), g_PatEmpty, g_CBuf);
        }
        // Row 2: "L" + 3-digit lines at cols 0-3, "V" + single-digit level at 5-7
        PutChar(sx, 2, 'L', pIdx, 0);
        for (c = 1; c < 4; c++) {
            Tiles_ColorEmpty(g_CBuf, pIdx);
            WriteTileData(2, (u8)(128 + 64 + sx + c), g_PatEmpty, g_CBuf);
        }
        Tiles_ColorEmpty(g_CBuf, pIdx);
        WriteTileData(2, (u8)(128 + 64 + sx + 4), g_PatEmpty, g_CBuf);
        PutChar(sx + 5, 2, 'V', pIdx, 0);
        Tiles_ColorEmpty(g_CBuf, pIdx);
        WriteTileData(2, (u8)(128 + 64 + sx + 6), g_PatEmpty, g_CBuf);
        WriteTileData(2, (u8)(128 + 64 + sx + 7), g_PatEmpty, g_CBuf);
        // Row 3: separator (solid block color)
        Tiles_ColorSeparator(g_CBuf, pIdx);
        for (c = 0; c < 8; c++)
            WriteTileData(3, (u8)(128 + 96 + sx + c), g_PatSolid, g_CBuf);
        rs->headerDrawn = 1;
    }
    if (p->score != rs->score) {
        PutNum(sx, 1, p->score, 4, pIdx);
        rs->score = p->score;
    }
    if (p->lines != rs->lines) {
        PutNum(sx + 1, 2, p->lines, 3, pIdx);
        rs->lines = p->lines;
    }
    if (p->level != rs->level) {
        PutNum(sx + 6, 2, p->level, 1, pIdx);
        rs->level = p->level;
    }
    if (p->nextIdx != rs->nextIdx) {
        RenderNextPreview(pIdx, p->nextIdx);
        rs->nextIdx = p->nextIdx;
    }
}

// ============================================================================
// BOARD drawing — writes to g_NameBuffer (rows 4-23)
// ============================================================================

static void DrawBoardCell(u8 pIdx, u8 r, u8 c) {
    u8 sx = pIdx * 8 + c;
    u8 sy = r + HEADER_ROWS;
    u8 val = g_Players[pIdx].board[r][c];
    PutBoard(sx, sy, BOARD_TILE(val, pIdx));
}

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

static void DrawShapeAt(u8 pIdx, u8 pieceIdx, u8 rot, i8 px, i8 py, u8 tileIdx) {
    const PieceRot* s = Piece_GetRot(pieceIdx, rot);
    u8 sx = pIdx * 8;
    u8 r, c;
    for (r = 0; r < s->h; r++) {
        for (c = 0; c < s->w; c++) {
            if (Piece_GetBit(s->bits, r, c)) {
                i8 br = py + (i8)r;
                i8 bc = px + (i8)c;
                if (br >= 0 && br < BOARD_H && bc >= 0 && bc < BOARD_W)
                    PutBoard(sx + (u8)bc, (u8)br + HEADER_ROWS, tileIdx);
            }
        }
    }
}

static void DrawPieceAt(u8 pIdx, u8 pieceIdx, u8 rot, i8 px, i8 py) {
    DrawShapeAt(pIdx, pieceIdx, rot, px, py, (u8)(TILE_BLOCK_P0 + pIdx));
}

static void DrawGhostAt(u8 pIdx, u8 pieceIdx, u8 rot, i8 px, i8 py) {
    DrawShapeAt(pIdx, pieceIdx, rot, px, py, (u8)(TILE_GHOST_P0 + pIdx));
}

// Compute where the current piece would land (for ghost rendering)
static i8 ComputeGhostY(const Player* p) {
    i8 y = p->py;
    while (Player_Valid(p, p->pieceIdx, p->rot, p->px, y + 1)) y++;
    return y;
}

// ============================================================================
// FLASH — write TILE_FLASH into the flashing rows
// ============================================================================

static void RenderFlash(const Player* p, u8 pIdx) {
    RenderState* rs = &g_RS[pIdx];
    u8 newState;
    u8 fi, c;
    u8 sx = pIdx * 8;

    newState = ((p->flashTimer % FLASH_TOGGLE) < (FLASH_TOGGLE / 2)) ? 1 : 2;
    if (newState == rs->flashState) return;

    rs->flashState = newState;
    for (fi = 0; fi < p->flashCount; fi++) {
        u8 row = p->flashRows[fi];
        u8 sy = row + HEADER_ROWS;
        if (newState == 1) {
            for (c = 0; c < BOARD_W; c++)
                PutBoard(sx + c, sy, TILE_FLASH);
        } else {
            for (c = 0; c < BOARD_W; c++) {
                u8 val = p->board[row][c];
                PutBoard(sx + c, sy, BOARD_TILE(val, pIdx));
            }
        }
    }
}

// ============================================================================
// DEAD PLAYER — fill strip with TILE_DEAD + GAME OVER text (fixed tiles)
// ============================================================================

static void RenderDeadStrip(u8 pIdx) {
    u8 sx = pIdx * 8;
    u8 r, c;

    // Fill strip (rows 0-23) with dark red
    // Header rows 0-3: use PutChar-style writes (dark red bg)
    // Board rows 4-23: set NameBuffer to TILE_DEAD
    for (r = HEADER_ROWS; r < 24; r++)
        for (c = 0; c < 8; c++)
            PutBoard(sx + c, r, TILE_DEAD);

    // For header rows 0-3, overwrite with dark red (dynamic tiles)
    {
        u8 darkRedCol[8];
        u8 i;
        for (i = 0; i < 8; i++)
            darkRedCol[i] = (u8)((COLOR_DARK_RED << 4) | COLOR_DARK_RED);
        for (r = 0; r < 4; r++) {
            for (c = 0; c < 8; c++) {
                u8 tileIdx = (u8)(128 + r * 32 + sx + c);
                WriteTileData(r, tileIdx, g_PatEmpty, darkRedCol);
            }
        }
    }

    // "GAME" at row 11, cols sx+2..sx+5
    PutBoard(sx + 2, 11, TILE_DEAD_G);
    PutBoard(sx + 3, 11, TILE_DEAD_A);
    PutBoard(sx + 4, 11, TILE_DEAD_M);
    PutBoard(sx + 5, 11, TILE_DEAD_E);
    // "OVER" at row 12
    PutBoard(sx + 2, 12, TILE_DEAD_O);
    PutBoard(sx + 3, 12, TILE_DEAD_V);
    PutBoard(sx + 4, 12, TILE_DEAD_E);
    PutBoard(sx + 5, 12, TILE_DEAD_R);
}

// ============================================================================
// RENDER PLAYER — minimal per-frame work
// ============================================================================

static void RenderPlayer(const Player* p, u8 pIdx) {
    RenderState* rs = &g_RS[pIdx];
    u8 pieceMoved;
    u8 pieceChanged;

    if (p->dead) {
        if (rs->dead != 1) {
            RenderDeadStrip(pIdx);
            rs->dead = 1;
        }
        return;
    }

    RenderHeader(p, pIdx);

    if (p->boardDirty) {
        u8 r, c;
        if (rs->pieceIdx != 0xFF) {
            ErasePieceAt(pIdx, rs->pieceIdx, rs->rot, rs->px, rs->py);
            rs->pieceIdx = 0xFF;
        }
        for (r = 0; r < BOARD_H; r++)
            for (c = 0; c < BOARD_W; c++)
                DrawBoardCell(pIdx, r, c);
        ((Player*)p)->boardDirty = 0;
    }

    if (rs->flashState != 0 && p->flashTimer == 0) {
        u8 r, c;
        rs->flashState = 0;
        for (r = 0; r < BOARD_H; r++)
            for (c = 0; c < BOARD_W; c++)
                DrawBoardCell(pIdx, r, c);
        rs->pieceIdx = 0xFF;
    }

    if (p->flashTimer > 0) {
        if (rs->pieceIdx != 0xFF) {
            if (rs->ghostY >= 0 && rs->ghostY != rs->py)
                ErasePieceAt(pIdx, rs->pieceIdx, rs->rot, rs->px, rs->ghostY);
            ErasePieceAt(pIdx, rs->pieceIdx, rs->rot, rs->px, rs->py);
            rs->pieceIdx = 0xFF;
            rs->ghostY = -1;
        }
        RenderFlash(p, pIdx);
        return;
    }

    pieceChanged = (p->pieceIdx != rs->pieceIdx && rs->pieceIdx != 0xFF);

    if (pieceChanged) {
        const PieceRot* oldS = Piece_GetRot(rs->pieceIdx, rs->rot);
        u8 r, c;
        i8 minR = rs->py;
        i8 maxR = rs->py + (i8)oldS->h - 1;
        if (minR < 0) minR = 0;
        if (maxR >= BOARD_H) maxR = BOARD_H - 1;
        for (r = (u8)minR; r <= (u8)maxR; r++)
            for (c = 0; c < BOARD_W; c++)
                DrawBoardCell(pIdx, r, c);
        // Also erase the stale ghost — it lives on different rows than the
        // piece and would otherwise linger after the new piece spawns.
        if (rs->ghostY >= 0 && rs->ghostY != rs->py) {
            i8 gmin = rs->ghostY;
            i8 gmax = rs->ghostY + (i8)oldS->h - 1;
            if (gmin < 0) gmin = 0;
            if (gmax >= BOARD_H) gmax = BOARD_H - 1;
            for (r = (u8)gmin; r <= (u8)gmax; r++)
                for (c = 0; c < BOARD_W; c++)
                    DrawBoardCell(pIdx, r, c);
        }
        rs->pieceIdx = 0xFF;
        rs->ghostY = -1;
    }

    if (rs->pieceIdx == 0xFF && !pieceChanged && rs->lockH == 0) {
        u8 r, c;
        for (r = 0; r < BOARD_H; r++)
            for (c = 0; c < BOARD_W; c++)
                DrawBoardCell(pIdx, r, c);
    }

    pieceMoved = (p->px != rs->px || p->py != rs->py ||
                  p->rot != rs->rot || p->pieceIdx != rs->pieceIdx);

    if (!pieceMoved) return;

    {
        // Ghost is expensive (up to 20 Player_Valid calls); only render for
        // human players. AIs don't benefit from it.
        u8 showGhost = (g_HumanMask & (1 << pIdx)) ? 1 : 0;
        // Ghost y only changes on horizontal move, rotation, or new piece.
        // Pure vertical fall keeps the same landing y.
        u8 needGhostRecompute = (p->px != rs->px || p->rot != rs->rot ||
                                 p->pieceIdx != rs->pieceIdx);

        // Erase old ghost (only if we had one drawn and position differs)
        if (rs->pieceIdx != 0xFF && rs->ghostY >= 0 && rs->ghostY != rs->py)
            ErasePieceAt(pIdx, rs->pieceIdx, rs->rot, rs->px, rs->ghostY);

        // Erase old piece overlay
        if (rs->pieceIdx != 0xFF)
            ErasePieceAt(pIdx, rs->pieceIdx, rs->rot, rs->px, rs->py);

        // Update ghost y only when needed
        if (showGhost && needGhostRecompute)
            rs->ghostY = ComputeGhostY(p);
        else if (!showGhost)
            rs->ghostY = -1;

        // Draw ghost if visible gap
        if (showGhost && rs->ghostY > p->py)
            DrawGhostAt(pIdx, p->pieceIdx, p->rot, p->px, rs->ghostY);

        // Draw piece on top
        DrawPieceAt(pIdx, p->pieceIdx, p->rot, p->px, p->py);
    }

    rs->px = p->px;
    rs->py = p->py;
    rs->rot = p->rot;
    rs->pieceIdx = p->pieceIdx;
    rs->lockH = 1;
}

// ============================================================================
// PUBLIC
// ============================================================================

void Render_Frame(void) {
    u8 i;
    for (i = 0; i < NUM_PLAYERS; i++)
        RenderPlayer(&g_Players[i], i);

    // Flush board portion (rows 4-23, 640 bytes) in ONE VRAM transfer
    if (g_Mode == 1)
        FlushBoard();

    // Sprites (targeting arrows) — bob ±2px vertically for life
    {
        static const u8 sprXOff[4] = { 4, 20, 36, 52 };
        u8 bob = ((g_Frame >> 3) & 1) ? 2 : 0;  // toggles every 8 frames
        for (i = 0; i < NUM_PLAYERS; i++) {
            if (g_Players[i].dead) {
                VDP_SetSpriteSM1(i, 0, 208, 0, 0);
            } else {
                u8 tgt = g_Players[i].targetPlayer;
                u8 sy = (u8)(HEADER_ROWS * 8 - 10 + bob);
                u8 sx = (u8)(tgt * 64 + sprXOff[i]);
                VDP_SetSpriteSM1(i, sx, sy, 0, g_SprColor[i]);
            }
        }
    }
}

// ============================================================================
// COUNTDOWN — big digit on each player's board before game starts
// ============================================================================

void Render_Countdown(u8 ch) {
    u8 idx = (ch >= 32 && ch < 32 + FONT_COUNT) ? ch - 32 : 0;
    const u8* pat = g_Font[idx];
    u8 p, r, c;

    for (p = 0; p < NUM_PLAYERS; p++) {
        u8 sx = p * 8 + 2;
        u8 sy = 11;    // rows 11-15 (centered on 20-row board)
        // Clear a 4×7 area
        for (r = 0; r < 7; r++)
            for (c = 0; c < 4; c++)
                PutBoard(sx + c, sy + r, TILE_EMPTY_P0 + p);
        // Draw 3×5 glyph as block tiles
        for (r = 0; r < 5; r++) {
            u8 row = pat[r + 1];
            for (c = 0; c < 3; c++) {
                if (row & (0x20 >> c))
                    PutBoard(sx + c, sy + 1 + r, (u8)(TILE_BLOCK_P0 + p));
            }
        }
    }
    FlushBoard();
}

void Render_ClearCountdown(void) {
    u8 p, r, c;
    for (p = 0; p < NUM_PLAYERS; p++) {
        u8 sx = p * 8;
        for (r = HEADER_ROWS; r < 24; r++)
            for (c = 0; c < 8; c++)
                PutBoard(sx + c, r, TILE_EMPTY_P0 + p);
    }
    FlushBoard();
}

// ============================================================================
// TITLE SCREEN (identity mode, dynamic writes)
// ============================================================================

static void DrawBigChar(u8 x, u8 y, u8 ch, u8 colorIdx) {
    u8 idx = (ch >= 32 && ch < 32 + FONT_COUNT) ? ch - 32 : 0;
    const u8* pat = g_Font[idx];
    u8 r, c;
    Tiles_ColorBlock(g_CBuf, colorIdx);
    for (r = 0; r < 5; r++) {
        u8 row = pat[r + 1];
        for (c = 0; c < 3; c++) {
            if (row & (0x20 >> c))
                PutTileIdentity(x + c, y + r, g_PatBlock, g_CBuf);
        }
    }
}

void Render_TitleScreen(void) {
    u8 r, c, i, x;
    u8 colBg[8];

    // Ensure we're in identity mode
    Render_IdentityMode();

    // Hide all sprites (targeting arrows from previous game/attract)
    for (i = 0; i < 32; i++)
        VDP_SetSpriteSM1(i, 0, 208, 0, 0);

    for (i = 0; i < 8; i++)
        colBg[i] = (u8)((COLOR_BLACK << 4) | COLOR_BLACK);
    for (r = 0; r < 24; r++)
        for (c = 0; c < 32; c++)
            PutTileIdentity(c, r, g_PatEmpty, colBg);

    // "TINY"
    x = 8;
    DrawBigChar(x,    1, 'T', 0);
    DrawBigChar(x+4,  1, 'I', 1);
    DrawBigChar(x+8,  1, 'N', 2);
    DrawBigChar(x+12, 1, 'Y', 3);

    // "TETRIS"
    x = 4;
    DrawBigChar(x,    7, 'T', 1);
    DrawBigChar(x+4,  7, 'E', 0);
    DrawBigChar(x+8,  7, 'T', 3);
    DrawBigChar(x+12, 7, 'R', 2);
    DrawBigChar(x+16, 7, 'I', 1);
    DrawBigChar(x+20, 7, 'S', 0);

    // "VS"
    x = 12;
    DrawBigChar(x,   13, 'V', 3);
    DrawBigChar(x+4, 13, 'S', 1);

    // Player slots
    for (i = 0; i < NUM_PLAYERS; i++) {
        u8 colP[8], colTxt[8];
        const PlayerColors* pc = &g_PlayerColors[i];
        u8 sx = i * 8;

        for (c = 0; c < 8; c++) {
            colP[c]   = (u8)((pc->block << 4) | COLOR_BLACK);
            colTxt[c] = (u8)((pc->text  << 4) | COLOR_BLACK);
        }

        PutTileIdentity(sx + 3, 19, g_Font['P'-32], colP);
        PutTileIdentity(sx + 4, 19, g_Font['1'+i-32], colP);

        PutTileIdentity(sx + 1, 21, g_Font['P'-32], colTxt);
        PutTileIdentity(sx + 2, 21, g_Font['R'-32], colTxt);
        PutTileIdentity(sx + 3, 21, g_Font['E'-32], colTxt);
        PutTileIdentity(sx + 4, 21, g_Font['S'-32], colTxt);
        PutTileIdentity(sx + 5, 21, g_Font['S'-32], colTxt);

        PutTileIdentity(sx + 3, 22, g_Font['A'-32], colP);
    }
}

void Render_TitleReady(u8 pIdx) {
    u8 colR[8], colBg[8], i, x;
    const PlayerColors* pc = &g_PlayerColors[pIdx];
    u8 sx = pIdx * 8;

    for (i = 0; i < 8; i++) {
        colR[i]  = (u8)((pc->block << 4) | COLOR_BLACK);
        colBg[i] = (u8)((COLOR_BLACK << 4) | COLOR_BLACK);
    }

    for (x = sx; x < sx + 8; x++) {
        PutTileIdentity(x, 21, g_PatEmpty, colBg);
        PutTileIdentity(x, 22, g_PatEmpty, colBg);
    }

    PutTileIdentity(sx + 1, 21, g_Font['R'-32], colR);
    PutTileIdentity(sx + 2, 21, g_Font['E'-32], colR);
    PutTileIdentity(sx + 3, 21, g_Font['A'-32], colR);
    PutTileIdentity(sx + 4, 21, g_Font['D'-32], colR);
    PutTileIdentity(sx + 5, 21, g_Font['Y'-32], colR);
}

void Render_TitleCPU(u8 pIdx) {
    u8 colC[8], colBg[8], i, x;
    const PlayerColors* pc = &g_PlayerColors[pIdx];
    u8 sx = pIdx * 8;

    for (i = 0; i < 8; i++) {
        colC[i]  = (u8)((pc->text << 4) | COLOR_BLACK);
        colBg[i] = (u8)((COLOR_BLACK << 4) | COLOR_BLACK);
    }

    for (x = sx; x < sx + 8; x++) {
        PutTileIdentity(x, 21, g_PatEmpty, colBg);
        PutTileIdentity(x, 22, g_PatEmpty, colBg);
    }

    PutTileIdentity(sx + 2, 21, g_Font['C'-32], colC);
    PutTileIdentity(sx + 3, 21, g_Font['P'-32], colC);
    PutTileIdentity(sx + 4, 21, g_Font['U'-32], colC);
}

// ============================================================================
// VICTORY SCREEN (identity mode)
// ============================================================================

void Render_Victory(u8 winnerIdx) {
    u8 r, c, i, sx;
    u8 colWin[8], colTxt[8], colBlock[8];
    const PlayerColors* pc = &g_PlayerColors[winnerIdx];

    // Switch to identity mode for full-screen dynamic rendering
    Render_IdentityMode();

    for (i = 0; i < 32; i++)
        VDP_SetSpriteSM1(i, 0, 208, 0, 0);

    for (i = 0; i < 8; i++)
        colWin[i] = (u8)((pc->bg << 4) | pc->bg);
    for (r = 0; r < 24; r++)
        for (c = 0; c < 32; c++)
            PutTileIdentity(c, r, g_PatEmpty, colWin);

    for (i = 0; i < 8; i++) {
        colTxt[i]   = (u8)((COLOR_WHITE << 4) | pc->bg);
        colBlock[i] = (u8)((pc->block << 4) | pc->bg);
    }

    sx = 12;
    PutTileIdentity(sx+0, 9, g_Font['P'-32], colTxt);
    PutTileIdentity(sx+1, 9, g_Font['L'-32], colTxt);
    PutTileIdentity(sx+2, 9, g_Font['A'-32], colTxt);
    PutTileIdentity(sx+3, 9, g_Font['Y'-32], colTxt);
    PutTileIdentity(sx+4, 9, g_Font['E'-32], colTxt);
    PutTileIdentity(sx+5, 9, g_Font['R'-32], colTxt);
    PutTileIdentity(sx+7, 9, g_Font['1' + winnerIdx - 32], colBlock);

    sx = 14;
    PutTileIdentity(sx+0, 11, g_Font['W'-32], colBlock);
    PutTileIdentity(sx+1, 11, g_Font['I'-32], colBlock);
    PutTileIdentity(sx+2, 11, g_Font['N'-32], colBlock);
    PutTileIdentity(sx+3, 11, g_Font['S'-32], colBlock);
}
