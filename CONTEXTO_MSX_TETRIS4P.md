# CONTEXTO: Tiny Tetris VS — MSX1 4-Player Battle Tetris

## El juego
Tetris competitivo para 4 jugadores simultáneos en MSX1. Pantalla dividida en 4 tiras verticales de 8 columnas cada una. Prototipo HTML/Canvas como referencia visual, portado a C con MSXgl.

## Hardware
- CPU: Z80 @ 3.58 MHz
- Vídeo: TMS9918A — Screen 2 (GRAPHIC2), 256x192, 32x24 tiles, 16KB VRAM
- Audio: AY-3-8910 PSG — 3 canales (A+B música, C libre)
- RAM: 16 KB
- ROM: 32 KB cartridge (ROM_32K, pages 1-2: 0x4000-0xBFFF)

## Stack técnico
- Librería: MSXgl v1.4.1 (C + SDCC)
- Build: Node.js build system + build.bat auto-sync
- Emulador: openMSX con NinjaTap emulado
- Multiplayer: Ninja Tap adapter (driver Gigamix DM-System2 vía MSXgl v1.4.1)

## Arquitectura de rendering
- **Double-buffer**: g_NameBuffer[768] (RAM shadow de la name table)
- **27 fixed tiles** pre-baked en los 3 bancos de Screen 2 (blocks, empty, garbage, ghost, flash, separators, dead/gameover letters)
- **Per-row dirty tracking**: solo se flushean las filas que cambiaron
- **Header dinámico**: tiles 128-255 de banco 0 para texto (score, lines, level, next-piece preview)
- **Ghost piece**: outline en color del jugador, solo para humanos

## IA
- Búsqueda distribuida: 2 posiciones evaluadas por tick, round-robin entre IAs
- Base state precomputado (heights + rowCount) una vez por pieza
- FindLandingY: cálculo O(4) de y de aterrizaje sin loops de Player_Valid
- Piece_GetBit como macro inline (sin overhead de llamada)
- Goal-aware: 70% apunta a 2 líneas, 22% a 3, 8% a Tetris
- Drop cadence moderada y variable por pieza cuando alineada
- Ejecuta best-so-far durante búsqueda (no espera a terminar)

## Mecánicas de combate
- **Targeting**: sprites 8x8 con bob, 1 por jugador, selección con Space/Button A
- **Garbage**: tabla base {0,0,1,2,4} + T-spin (x2) + combo (+1 por chain)
- **Garbage gradual**: sube 1 fila cada 3 ticks (animación de subida)
- **Lock-on-landing**: si la pieza está en posición de aterrizar cuando llega garbage, se lockea y sube con el board
- **Pieza sube con garbage**: py-- por cada fila de basura
- **Auto-retarget**: cuando el target muere, reasigna a un vivo aleatorio

## Flujo de pantallas
1. **Título**: logo en bloques de Tetris + "PRESS A" por jugador + timer 8s
2. **Attract mode**: 15s sin input → 4 CPUs juegan demo, cualquier tecla → título
3. **Countdown**: 3-2-1 en dígitos grandes de bloques
4. **Partida**: 4 tableros simultáneos + HUD (score, lines, level, next, ghost)
5. **Victoria**: pantalla color del ganador + "PLAYER X WINS" + fanfarria
6. **Estadísticas**: SCO/LIN/GRB/TSP/CMB/LVL por jugador, 15s o tecla

## Ficheros del proyecto
```
src/
  main.c           — loop principal, transiciones de pantalla
  game.c / game.h  — lógica (piezas, colisión, lock, IA, garbage, T-spin, combos)
  render.c         — VDP rendering (double-buffer, ghost, HUD, stats, countdown)
  input_game.c     — input keyboard + NinjaTap, filtrado human/CPU
  music.c / music.h — PSG player (título, Korobeiniki, victoria)
  tiles.h          — patrones, colores, font 3x5, tile indices fijos
  msxgl_config.h   — configuración MSXgl
  project_config.js — configuración de build
assets/
  korobeiniki.mid  — referencia MIDI del tema
  msx_tetris4p.html — prototipo visual original
builds/
  tinytetris4p.rom — última build
  tinytetrisNN.rom — builds versionadas (auto-prune a 5)
```

## Paleta por jugador
| Jugador | Fondo    | Bloque   | Texto    |
|---------|----------|----------|----------|
| P1      | DBLUE    | CYAN     | LBLUE    |
| P2      | DRED     | LRED     | MRED     |
| P3      | DGREEN   | LGREEN   | MGREEN   |
| P4      | MAGENTA  | LYELLOW  | DYELLOW  |
