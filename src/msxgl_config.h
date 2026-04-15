// ____________________________________________________________________________
// MSXgl Library configuration — TiniTetris 4P
// Target: MSX1 · TMS9918A · Screen 2 (GRAPHIC2) · ROM 48K
// ____________________________________________________________________________
#pragma once

//-----------------------------------------------------------------------------
// BIOS MODULE
//-----------------------------------------------------------------------------
#define BIOS_CALL_MAINROM			BIOS_CALL_DIRECT
#define BIOS_CALL_SUBROM			BIOS_CALL_INTERSLOT
#define BIOS_CALL_DISKROM			BIOS_CALL_INTERSLOT
#define BIOS_USE_MAINROM			TRUE
#define BIOS_USE_VDP				TRUE
#define BIOS_USE_PSG				FALSE
#define BIOS_USE_SUBROM				FALSE
#define BIOS_USE_DISKROM			FALSE

//-----------------------------------------------------------------------------
// VDP MODULE
//-----------------------------------------------------------------------------
#define VDP_VRAM_ADDR				VDP_VRAM_ADDR_14
#define VDP_UNIT					VDP_UNIT_U8

// Screen modes — only GRAPHIC2 (Screen 2)
#define VDP_USE_MODE_T1				FALSE
#define VDP_USE_MODE_G1				FALSE
#define VDP_USE_MODE_G2				TRUE
#define VDP_USE_MODE_MC				FALSE
#define VDP_USE_MODE_T2				FALSE
#define VDP_USE_MODE_G3				FALSE
#define VDP_USE_MODE_G4				FALSE
#define VDP_USE_MODE_G5				FALSE
#define VDP_USE_MODE_G6				FALSE
#define VDP_USE_MODE_G7				FALSE

#define VDP_USE_VRAM16K				TRUE
#define VDP_USE_SPRITE				TRUE
#define VDP_USE_COMMAND				FALSE
#define VDP_USE_CUSTOM_CMD			FALSE
#define VDP_AUTO_INIT				TRUE
#define VDP_USE_UNDOCUMENTED		FALSE
#define VDP_USE_VALIDATOR			FALSE
#define VDP_USE_DEFAULT_PALETTE		FALSE
#define VDP_USE_MSX1_PALETTE		FALSE
#define VDP_USE_DEFAULT_SETTINGS	TRUE
#define VDP_USE_16X16_SPRITE		FALSE
#define VDP_USE_RESTORE_S0			TRUE
#define VDP_USE_PALETTE16			FALSE
#define VDP_ISR_SAFE_MODE			VDP_ISR_SAFE_DEFAULT
#define VDP_INIT_50HZ				VDP_INIT_DEFAULT

//-----------------------------------------------------------------------------
// V9990 MODULE (not used)
//-----------------------------------------------------------------------------
#define V9_USE_MODE_P1				FALSE
#define V9_USE_MODE_P2				FALSE
#define V9_USE_MODE_B0				FALSE
#define V9_USE_MODE_B1				FALSE
#define V9_USE_MODE_B2				FALSE
#define V9_USE_MODE_B3				FALSE
#define V9_USE_MODE_B4				FALSE
#define V9_USE_MODE_B5				FALSE
#define V9_USE_MODE_B6				FALSE
#define V9_USE_MODE_B7				FALSE
#define V9_INT_PROTECT				FALSE
#define V9_PALETTE_MODE				V9_PALETTE_RGB_24

//-----------------------------------------------------------------------------
// INPUT MODULE
//-----------------------------------------------------------------------------
#define INPUT_USE_JOYSTICK			FALSE
#define INPUT_USE_KEYBOARD			TRUE
#define INPUT_USE_MOUSE				FALSE
#define INPUT_USE_DETECT			FALSE
#define INPUT_USE_ISR_PROTECTION	TRUE
#define INPUT_JOY_UPDATE			FALSE
#define INPUT_KB_UPDATE				FALSE
#define INPUT_KB_UPDATE_MIN			0
#define INPUT_KB_UPDATE_MAX			8

//-----------------------------------------------------------------------------
// PADDLE MODULE
//-----------------------------------------------------------------------------
#define PADDLE_USE_CALIB			FALSE

//-----------------------------------------------------------------------------
// MEMORY MODULE
//-----------------------------------------------------------------------------
#define MEM_USE_VALIDATOR			FALSE
#define MEM_USE_FASTCOPY			FALSE
#define MEM_USE_FASTSET				FALSE
#define MEM_USE_DYNAMIC				FALSE
#define MEM_USE_BUILTIN				TRUE

//-----------------------------------------------------------------------------
// MSX-DOS MODULE (not used)
//-----------------------------------------------------------------------------
#define DOS_USE_FCB					FALSE
#define DOS_USE_HANDLE				FALSE
#define DOS_USE_UTILITIES			FALSE
#define DOS_USE_VALIDATOR			FALSE

//-----------------------------------------------------------------------------
// CLOCK MODULE (not used)
//-----------------------------------------------------------------------------
#define RTC_USE_CLOCK				FALSE
#define RTC_USE_CLOCK_EXTRA			FALSE
#define RTC_USE_SAVEDATA			FALSE
#define RTC_USE_SAVESIGNED			FALSE

//-----------------------------------------------------------------------------
// PRINT MODULE (not used — we render our own font via VRAM)
//-----------------------------------------------------------------------------
#define PRINT_USE_TEXT				FALSE
#define PRINT_USE_BITMAP			FALSE
#define PRINT_USE_VRAM				FALSE
#define PRINT_USE_SPRITE			FALSE
#define PRINT_USE_FX_SHADOW			FALSE
#define PRINT_USE_FX_OUTLINE		FALSE
#define PRINT_USE_2_PASS_FX			FALSE
#define PRINT_USE_GRAPH				FALSE
#define PRINT_USE_VALIDATOR			FALSE
#define PRINT_USE_UNIT				FALSE
#define PRINT_USE_FORMAT			FALSE
#define PRINT_USE_32B				FALSE
#define PRINT_SKIP_SPACE			FALSE
#define PRINT_COLOR_NUM				12
#define PRINT_WIDTH					PRINT_WIDTH_1
#define PRINT_HEIGHT				PRINT_HEIGHT_1

//-----------------------------------------------------------------------------
// SPRITE FX MODULE (not used)
//-----------------------------------------------------------------------------
#define SPRITEFX_USE_8x8			FALSE
#define SPRITEFX_USE_16x16			FALSE
#define SPRITEFX_USE_CROP			FALSE
#define SPRITEFX_USE_FLIP			FALSE
#define SPRITEFX_USE_MASK			FALSE
#define SPRITEFX_USE_ROTATE			FALSE

//-----------------------------------------------------------------------------
// GAME MODULE (not used — we do our own loop)
//-----------------------------------------------------------------------------
#define GAME_USE_STATE				FALSE
#define GAME_USE_VSYNC				FALSE
#define GAME_USE_LOOP				FALSE

//-----------------------------------------------------------------------------
// GAME PAWN MODULE (not used)
//-----------------------------------------------------------------------------
#define GAMEPAWN_ID_PER_LAYER		FALSE
#define GAMEPAWN_USE_PHYSICS		FALSE
#define GAMEPAWN_BOUND_X			GAMEPAWN_BOUND_CUSTOM
#define GAMEPAWN_BOUND_Y			GAMEPAWN_BOUND_CUSTOM
#define GAMEPAWN_COL_DOWN			GAMEPAWN_COL_50
#define GAMEPAWN_COL_UP				GAMEPAWN_COL_50
#define GAMEPAWN_COL_RIGHT			GAMEPAWN_COL_50
#define GAMEPAWN_COL_LEFT			GAMEPAWN_COL_50
#define GAMEPAWN_BORDER_EVENT		GAMEPAWN_BORDER_NONE
#define GAMEPAWN_BORDER_BLOCK		GAMEPAWN_BORDER_NONE
#define GAMEPAWN_BORDER_MIN_Y		0
#define GAMEPAWN_BORDER_MAX_Y		191
#define GAMEPAWN_TILEMAP_WIDTH		32
#define GAMEPAWN_TILEMAP_HEIGHT		24
#define GAMEPAWN_TILEMAP_SRC		GAMEPAWN_TILEMAP_SRC_VRAM
#define GAMEPAWN_SPT_MODE			GAMEPAWN_SPT_MODE_MSX1

//-----------------------------------------------------------------------------
// GAME MENU MODULE (not used)
//-----------------------------------------------------------------------------
#define MENU_USE_DEFAULT_CALLBACK	FALSE
#define MENU_SCREEN_WIDTH			MENU_VARIABLE
#define MENU_FRAME_X				0
#define MENU_FRAME_Y				0
#define MENU_FRAME_WIDTH			32
#define MENU_FRAME_HEIGHT			8
#define MENU_CHAR_CLEAR				'\0'
#define MENU_CHAR_CURSOR			'@'
#define MENU_CHAR_TRUE				'O'
#define MENU_CHAR_FALSE				'X'
#define MENU_CHAR_LEFT				'<'
#define MENU_CHAR_RIGHT				'>'
#define MENU_TITLE_X				4
#define MENU_TITLE_Y				6
#define MENU_ITEM_X					6
#define MENU_ITEM_Y					8
#define MENU_ITEM_X_GOTO			6
#define MENU_ITEM_ALIGN				MENU_ITEM_ALIGN_LEFT
#define MENU_ITEM_ALIGN_GOTO		MENU_ITEM_ALIGN_LEFT
#define MENU_VALUE_X				14
#define MENU_CURSOR_MODE			MENU_CURSOR_MODE_NONE
#define MENU_CURSOR_OFFSET			(-2)

//-----------------------------------------------------------------------------
// STRING MODULE
//-----------------------------------------------------------------------------
#define STRING_USE_FROM_INT8		FALSE
#define STRING_USE_FROM_UINT8		TRUE
#define STRING_USE_FROM_INT16		TRUE
#define STRING_USE_FROM_UINT16		TRUE
#define STRING_USE_FORMAT			FALSE
#define STRING_USE_INT32			FALSE

//-----------------------------------------------------------------------------
// SCROLL MODULE (not used)
//-----------------------------------------------------------------------------
#define SCROLL_HORIZONTAL			FALSE
#define SCROLL_VERTICAL				FALSE
#define SCROLL_SRC_X				0
#define SCROLL_SRC_Y				0
#define SCROLL_SRC_W				32
#define SCROLL_SRC_H				24
#define SCROLL_DST_X				0
#define SCROLL_DST_Y				0
#define SCROLL_DST_W				32
#define SCROLL_DST_H				24
#define SCROLL_SCREEN_W				32
#define SCROLL_WRAP					FALSE
#define SCROLL_ADJUST				FALSE
#define SCROLL_ADJUST_SPLIT			FALSE
#define SCROLL_MASK					FALSE
#define SCROLL_MASK_ID				0
#define SCROLL_MASK_COLOR			COLOR_BLACK
#define SCROLL_MASK_PATTERN			0

//-----------------------------------------------------------------------------
// TILE
//-----------------------------------------------------------------------------
#define TILE_WIDTH					8
#define TILE_HEIGHT					8
#define TILE_BPP					1
#define TILE_SCREEN_WIDTH			256
#define TILE_SCREEN_HEIGHT			192
#define TILE_USE_SKIP				FALSE
#define TILE_SKIP_INDEX				0

//-----------------------------------------------------------------------------
// AUDIO (not used in Phase 1)
//-----------------------------------------------------------------------------
#define PSG_CHIP					PSG_INTERNAL
#define PSG_ACCESS					PSG_DIRECT
#define PSG_USE_NOTES				FALSE
#define PSG_USE_EXTRA				FALSE
#define PSG_USE_RESUME				TRUE

#define MSXAUDIO_USE_RESUME			FALSE
#define MSXMUSIC_USE_RESUME			FALSE

#define SCC_USE_EXTA				FALSE
#define SCC_USE_RESUME				FALSE
#define SCC_SLOT_MODE				SCC_SLOT_AUTO

#define VGM_USE_PSG					FALSE
#define VGM_USE_MSXMUSIC			FALSE
#define VGM_USE_MSXAUDIO			FALSE
#define VGM_USE_SCC					FALSE

#define PCMENC_FREQ					PCMENC_NONE
#define PCMPLAY_FREQ				PCMPLAY_8K
#define PCMPLAY_USE_RESTORE			FALSE

#define PT3_SKIP_HEADER				TRUE
#define PT3_AUTOPLAY				FALSE
#define PT3_EXTRA					FALSE

#define TRILO_USE_SFXPLAY			FALSE
#define TRILO_USE_TREMOLO			FALSE
#define TRILO_USE_TAIL				FALSE

#define LVGM_USE_PSG				FALSE
#define LVGM_USE_MSXMUSIC			FALSE
#define LVGM_USE_MSXAUDIO			FALSE
#define LVGM_USE_SCC				FALSE
#define LVGM_USE_SCCI				FALSE
#define LVGM_USE_PSG2				FALSE
#define LVGM_USE_OPL4				FALSE
#define LVGM_USE_NOTIFY				FALSE

#define WYZ_CHANNELS				WYZ_3CH
#define WYZ_USE_DIRECT_ACCESS		FALSE
#define WYZ_CHAN_BUFFER_SIZE		0x20

//-----------------------------------------------------------------------------
// MATH MODULE
//-----------------------------------------------------------------------------
#define RANDOM_8_METHOD				RANDOM_8_ION
#define RANDOM_16_METHOD			RANDOM_16_XORSHIFT

//-----------------------------------------------------------------------------
// COMPRESS (not used)
//-----------------------------------------------------------------------------
#define COMPRESS_USE_RLEP			FALSE
#define COMPRESS_USE_RLEP_DEFAULT	FALSE
#define COMPRESS_USE_RLEP_FIXSIZE	FALSE
#define PLETTER_LENGTHINDATA		FALSE
#define PLETTER_DI_MODE				PLETTER_DI_NONE
#define PLETTER_WRITE_MODE			PLETTER_WRITE_SAFE
#define BITBUSTER_WRITE_MODE		BITBUSTER_WRITE_SAFE
#define ZX0_MODE					ZX0_MODE_STANDARD
#define LZ48_MODE					LZ48_MODE_STANDARD
#define MSXi_USE_COMP_NONE			TRUE
#define MSXi_USE_COMP_CROP16		FALSE
#define MSXi_USE_COMP_CROP32		FALSE
#define MSXi_USE_COMP_CROP256		FALSE
#define MSXi_USE_COMP_CROPLINE16	FALSE
#define MSXi_USE_COMP_CROPLINE32	FALSE
#define MSXi_USE_COMP_CROPLINE256	FALSE
#define MSXi_USE_COMP_RLE0			FALSE
#define MSXi_USE_COMP_RLE4			FALSE
#define MSXi_USE_COMP_RLE8			FALSE

//-----------------------------------------------------------------------------
// NINJATAP MODULE
//-----------------------------------------------------------------------------
#define NTAP_DRIVER					(NTAP_DRIVER_MSXGL | NTAP_DRIVER_GIGAMIX)
#define NTAP_USE_PREVIOUS			TRUE

//-----------------------------------------------------------------------------
// PAC MODULE (not used)
//-----------------------------------------------------------------------------
#define PAC_USE_SIGNATURE			FALSE
#define PAC_USE_VALIDATOR			FALSE
#define PAC_DEVICE_MAX				1
#define PAC_ACCESS					PAC_ACCESS_BIOS

//-----------------------------------------------------------------------------
// QR CODE MODULE (not used)
//-----------------------------------------------------------------------------
#define QRCODE_VERSION_MIN			1
#define QRCODE_VERSION_MAX			1
#define QRCODE_VERSION_CUSTOM		FALSE
#define QRCODE_USE_BYTE_ONLY		TRUE
#define QRCODE_USE_EXTRA			FALSE
#define QRCODE_BOOST_ECL			FALSE
#define QRCODE_TINY_VERSION			1
#define QRCODE_TINY_ECC				QRCODE_ECC_LOW
#define QRCODE_TINY_MASK			QRCODE_MASK_0

//-----------------------------------------------------------------------------
// DEBUG & PROFILE
//-----------------------------------------------------------------------------
#define DEBUG_TOOL					DEBUG_DISABLE
#define PROFILE_TOOL				PROFILE_DISABLE
#define PROFILE_LEVEL				10
