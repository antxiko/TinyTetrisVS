// ____________________________________________________________________________
// Tile definitions for TiniTetris 4P
// Screen 2 (GRAPHIC2): 8x8 tiles, 2 colors per row per tile
// ____________________________________________________________________________
#pragma once

#include "msxgl.h"

// ============================================================================
// TILE PATTERNS (8 bytes each — 1 bit per pixel, MSB left)
// ============================================================================
// In GRAPHIC2: bit=1 → foreground color, bit=0 → background color
// Color byte per row: (fg << 4) | bg

// Empty tile (all background)
static const u8 g_PatEmpty[8] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Game block with highlight (top/left) and shadow (bottom)
// Row 0: full highlight line (fg = WHITE, bg = block color)
// Row 1-6: left pixel highlight (fg = WHITE), rest background (bg = block)
// Row 7: full shadow line (fg = shadow, bg = block color)
static const u8 g_PatBlock[8] = {
    0xFF, // ████████  row 0: highlight
    0x80, // █·······  row 1: left col
    0x80, // █·······  row 2
    0x80, // █·······  row 3
    0x80, // █·······  row 4
    0x80, // █·······  row 5
    0x80, // █·······  row 6
    0x00  // ········  row 7: shadow
};

// Ghost piece (outline only)
static const u8 g_PatGhost[8] = {
    0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF
};

// Solid fill (separator, flash)
static const u8 g_PatSolid[8] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

// ============================================================================
// PLAYER COLORS
// ============================================================================
typedef struct {
    u8 bg;
    u8 block;
    u8 shadow;
    u8 text;
} PlayerColors;

static const PlayerColors g_PlayerColors[4] = {
    { COLOR_DARK_BLUE,  COLOR_CYAN,         COLOR_DARK_BLUE,  COLOR_LIGHT_BLUE   }, // P1
    { COLOR_DARK_RED,   COLOR_LIGHT_RED,    COLOR_DARK_RED,   COLOR_MEDIUM_RED   }, // P2
    { COLOR_DARK_GREEN, COLOR_LIGHT_GREEN,  COLOR_DARK_GREEN, COLOR_MEDIUM_GREEN }, // P3
    { COLOR_MAGENTA,    COLOR_LIGHT_YELLOW, COLOR_MAGENTA,    COLOR_DARK_YELLOW  }, // P4
};

// ============================================================================
// COLOR TABLE BUILDERS (8 bytes output)
// ============================================================================

// Block: row 0 = white/block, rows 1-6 = white/block, row 7 = shadow/block
static void Tiles_ColorBlock(u8* out, u8 playerIdx) {
    const PlayerColors* pc = &g_PlayerColors[playerIdx];
    u8 i;
    u8 hiColor = (COLOR_WHITE << 4) | pc->block;
    u8 shColor = (pc->shadow << 4) | pc->block;
    for (i = 0; i < 7; i++)
        out[i] = hiColor;
    out[7] = shColor;
}

// Ghost: outline in block color on bg
static void Tiles_ColorGhost(u8* out, u8 playerIdx) {
    const PlayerColors* pc = &g_PlayerColors[playerIdx];
    u8 i;
    u8 c = (pc->block << 4) | pc->bg;
    for (i = 0; i < 8; i++)
        out[i] = c;
}

// Empty: solid bg
static void Tiles_ColorEmpty(u8* out, u8 playerIdx) {
    const PlayerColors* pc = &g_PlayerColors[playerIdx];
    u8 i;
    u8 c = (pc->bg << 4) | pc->bg;
    for (i = 0; i < 8; i++)
        out[i] = c;
}

// Separator: solid block color
static void Tiles_ColorSeparator(u8* out, u8 playerIdx) {
    const PlayerColors* pc = &g_PlayerColors[playerIdx];
    u8 i;
    u8 c = (pc->block << 4) | pc->block;
    for (i = 0; i < 8; i++)
        out[i] = c;
}

// Flash: solid white
static void Tiles_ColorFlash(u8* out) {
    u8 i;
    u8 c = (u8)((COLOR_WHITE << 4) | COLOR_WHITE);
    for (i = 0; i < 8; i++)
        out[i] = c;
}

// Text: player text color on bg
static void Tiles_ColorText(u8* out, u8 playerIdx) {
    const PlayerColors* pc = &g_PlayerColors[playerIdx];
    u8 i;
    u8 c = (pc->text << 4) | pc->bg;
    for (i = 0; i < 8; i++)
        out[i] = c;
}

// Bright text: white on bg
static void Tiles_ColorBright(u8* out, u8 playerIdx) {
    const PlayerColors* pc = &g_PlayerColors[playerIdx];
    u8 i;
    u8 c = (COLOR_WHITE << 4) | pc->bg;
    for (i = 0; i < 8; i++)
        out[i] = c;
}

// ============================================================================
// MINI FONT 3×5 in 8×8 tiles
// ============================================================================
// Characters are 3px wide, 5px tall, in bits 5-3 of each byte (centered)
// Rows: [0]=blank, [1-5]=char data, [6-7]=blank

#define _F(a,b,c,d,e) { \
    0x00, \
    (u8)(((a)&7)<<3), \
    (u8)(((b)&7)<<3), \
    (u8)(((c)&7)<<3), \
    (u8)(((d)&7)<<3), \
    (u8)(((e)&7)<<3), \
    0x00, \
    0x00  \
}

// Index: ch - 32. We define 32(' ') through 88('X')
static const u8 g_Font[][8] = {
    _F(0,0,0,0,0),  // 32 ' '
    _F(0,0,0,0,0),  // 33 (unused)
    _F(0,0,0,0,0),  // 34
    _F(0,0,0,0,0),  // 35
    _F(0,0,0,0,0),  // 36
    _F(0,0,0,0,0),  // 37
    _F(0,0,0,0,0),  // 38
    _F(0,0,0,0,0),  // 39
    _F(0,0,0,0,0),  // 40
    _F(0,0,0,0,0),  // 41
    _F(0,0,0,0,0),  // 42
    _F(0,0,0,0,0),  // 43
    _F(0,0,0,0,0),  // 44
    _F(0,0,0,0,0),  // 45
    _F(0,0,0,0,0),  // 46
    _F(0,0,0,0,0),  // 47
    _F(7,5,5,5,7),  // 48 '0'
    _F(2,6,2,2,7),  // 49 '1'
    _F(7,1,2,4,7),  // 50 '2'
    _F(7,1,3,1,7),  // 51 '3'
    _F(5,5,7,1,1),  // 52 '4'
    _F(7,4,7,1,7),  // 53 '5'
    _F(7,4,7,5,7),  // 54 '6'
    _F(7,1,2,2,2),  // 55 '7'
    _F(7,5,7,5,7),  // 56 '8'
    _F(7,5,7,1,7),  // 57 '9'
    _F(0,0,0,0,0),  // 58 ':'
    _F(0,0,0,0,0),  // 59
    _F(0,0,0,0,0),  // 60
    _F(0,0,0,0,0),  // 61
    _F(0,0,0,0,0),  // 62
    _F(0,0,0,0,0),  // 63
    _F(0,0,0,0,0),  // 64 '@'
    _F(2,5,7,5,5),  // 65 'A'
    _F(0,0,0,0,0),  // 66 'B'
    _F(3,4,4,4,3),  // 67 'C'
    _F(0,0,0,0,0),  // 68 'D'
    _F(7,4,6,4,7),  // 69 'E'
    _F(0,0,0,0,0),  // 70 'F'
    _F(3,4,5,5,3),  // 71 'G'
    _F(0,0,0,0,0),  // 72 'H'
    _F(7,2,2,2,7),  // 73 'I'
    _F(0,0,0,0,0),  // 74 'J'
    _F(5,6,6,5,5),  // 75 'K'
    _F(4,4,4,4,7),  // 76 'L'
    _F(5,7,5,5,5),  // 77 'M'
    _F(5,7,7,5,5),  // 78 'N'
    _F(2,5,5,5,2),  // 79 'O'
    _F(6,5,6,4,4),  // 80 'P'
    _F(0,0,0,0,0),  // 81 'Q'
    _F(6,5,6,5,5),  // 82 'R'
    _F(3,4,2,1,6),  // 83 'S'
    _F(7,2,2,2,2),  // 84 'T'
    _F(0,0,0,0,0),  // 85 'U'
    _F(5,5,5,5,2),  // 86 'V'
    _F(5,5,7,7,5),  // 87 'W'
    _F(5,2,2,2,5),  // 88 'X'
};

#undef _F

#define FONT_COUNT (sizeof(g_Font) / 8)
