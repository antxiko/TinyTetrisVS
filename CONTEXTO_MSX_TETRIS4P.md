# CONTEXTO: MSX TETRIS 4P — Port a MSXgl

## El juego
Tetris para 4 jugadores simultáneos en MSX, pantalla dividida en 4 tiras verticales.
Prototipo funcional hecho en HTML/Canvas como mockup de referencia visual.

## Restricciones de hardware respetadas
- Chip de vídeo: TMS9918A (MSX1)
- Resolución: 256×192 píxeles
- Grid de caracteres: 32×24 chars de 8×8 px
- Paleta: 16 colores fijos TMS9918A (sin inventarse ninguno)
- Gráficos por caracteres (tiles), NO por sprites
- Sprites reservados para efectos puntuales (máx 32, máx 4 por scanline)

## Layout de pantalla (matemática exacta)
- 4 tiras × 64px = 256px exactos (sin divisores extra)
- Cada tira = 8 chars de ancho × 24 chars de alto
- Cabecera: 4 filas × 8px = 32px  (score, level, next, lines)
- Tablero:  8 cols × 20 filas × 8px = 64×160px
- Total: 32 + 160 = 192px exactos ✓

## Paleta usada por jugador
Los 4 colores más oscuros del TMS9918A como fondo — el contraste entre ellos ES el divisor visual, sin añadir píxeles extra.

| Jugador | Fondo     | Bloque   | Texto  |
|---------|-----------|----------|--------|
| P1      | DBLUE     | CYAN     | LBLUE  |
| P2      | DRED      | LRED     | MRED   |
| P3      | DGREEN    | LGREEN   | MGREEN |
| P4      | MAGENTA   | LYELLOW  | DYELLOW|

Paleta TMS9918A completa (RGB exactos):
- BLACK   [0,0,0]
- MGREEN  [62,184,73]
- LGREEN  [116,208,125]
- DBLUE   [89,85,224]
- LBLUE   [128,118,241]
- DRED    [185,94,81]
- CYAN    [101,219,239]
- MRED    [219,101,89]
- LRED    [255,137,125]
- DYELLOW [204,195,94]
- LYELLOW [222,208,135]
- DGREEN  [58,162,65]
- MAGENTA [183,102,181]
- GRAY    [204,204,204]
- WHITE   [255,255,255]

## Stack del port MSX
- Librería: MSXgl (C + SDCC, build con Node.js)
- Target: ROM_ASCII16, 64KB (4 segmentos de 16KB)
- Máquina: MSX1 (TMS9918A)
- Módulos MSXgl: vdp, input, pt3/pt3_player, ayfx/ayfx_player

## Mapa de memoria ROM (ASCII-16, 64KB)
- Segmento #0 (bank#0, 0x4000, FIJO): código principal < 16KB
- Segmento #1 (bank#1, 0x8000): gráficos — tiles, sprites, paleta (raw bin)
- Segmento #2 (bank#1, 0x8000): música — 16 tracks PT3 + SFX ayFX (raw bin)
- Segmento #3 (bank#1, 0x8000): reserva

El código siempre está en seg#0 (bank#0 nunca se conmuta).
bank#1 se conmuta entre seg#1 (solo al init, para cargar VRAM) y seg#2 (cada frame en ISR).

## Modo de vídeo
Screen 2 (VDP_MODE_GRAPHIC2): permite 2 colores por bloque de 8×1px.
Los bloques del tablero son chars definidos en la tabla de patrones de VRAM.
Char de bloque: relleno de color + highlight 1px arriba/izquierda (WHITE) + sombra 1px abajo/derecha (color oscuro del jugador).

## Lógica del juego (del prototipo HTML)
- 7 piezas estándar con rotaciones
- Ghost piece (contorno del destino)
- Flash al completar línea (flashTimer=20 frames, alternando WHITE cada 4 frames)
- Score: [0,100,300,600,1000] × level según líneas simultáneas
- Level sube cada 10 líneas, dropInterval = max(8, 48 - level*4) frames
- Tablero guarda idx_jugador+1 (no idx_pieza) para saber el color del bloque

## Audio
- Música: PT3 player (Vortex Tracker II), 16 tracks
- SFX: ayFX player (compatible con PT3, usa AYFX_BUFFER_PT3)
- Update de música desde ISR VBlank (PT3_Decode() cada frame)
- Datos de música en segmento #2, player en segmento #0 (código)

## Controles (4 jugadores, 1 teclado MSX)
- P1: flechas
- P2: WASD  
- P3: por definir
- P4: por definir
(En MSX real habría que mapear a las filas del teclado con Keyboard_Read())

## Ficheros del proyecto
- msx_tetris4p.html  — prototipo visual de referencia (funcional en navegador)
- project_config.js  — configuración MSXgl (por crear)
- myjuego.c          — código principal (por crear)
- data/gfx/          — tiles.bin, sprites.bin, palette.bin (por crear con MSXimg)
- data/music/        — track00-15.bin, sfx_bank.bin (por crear con Vortex Tracker)
