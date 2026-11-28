# Genesis 2 — preparación inicial y transiciones

Implementación experimental sobre P19 / PR #139 (`04d3cec`). Esta entrega
resuelve el ciclo de vida de generación y caché del catálogo terrestre seguro.
**No completa el compositor de estructuras ni toda la especificación 2.0.**

## Comportamiento

- Nueva partida: una semilla de 32 bits se obtiene una sola vez, al empezar la
  presentación de Birch/Lotad. La preparación avanza cooperativamente desde
  `CB2_MainMenu`, fuera de IRQ/VBlank, mientras continúan diálogo y animaciones.
  No se entra al mundo antes de terminar. No usa ni consume RNG por celda.
- Continuar: se valida la identidad guardada y se reconstruye la caché una vez,
  con un mensaje de preparación. Entrar en rutas no reconstruye el mundo.
- 14 rutas elegibles: 101, 102, 103, 104, 110, 112, 116–123. Las familias de
  puzzles no revisadas y los demás dominios permanecen nativos. Ruta 101 conserva
  el rescate de Birch. El selector Native sigue funcionando.
- Entrada de mapa: lista dispersa de cambios ya preparada, comparando el original
  y el estado vivo. Sin ruido, CRC, escaneo de eventos, malloc ni SAVX. Las celdas
  modificadas por scripts no se pisan. SavedMapView conserva sus cambios al cargar.
- Identidad: registro de 16 bytes en SaveBlock4 offset 1040, con magic, versión
  uint32, catálogo y semilla. Wallet termina antes de 1040 y Cofradía empieza en
  1088; asserts impiden solapamientos. El CRC del bloque existente lo protege.
  No se amplían SaveBlocks ni se persiste el cache. No hay migración automática.
- Sin GBAX: mapas nativos para partidas G2. Los saves anteriores sin registro
  conservan el piloto bakeado anterior. Registros incompatibles no se reemplazan.

## Contratos y memoria

`include/genesis2/world.h` / `src/genesis2/world.c`: núcleo C99 independiente,
versiones separadas de Genesis 1.1, buffers del llamador y scheduler reanudable
QUEUED → RENDER → VALIDATE → READY/FAILED. Scheduler con coste ponderado: una unidad tiene 16 créditos; una celda
editable cuesta 16, una copia protegida 4 y una validación 8. El runtime
avanza microtareas hasta 55 líneas (~4 ms), con tope de 128 unidades.
Combina VCOUNT con el contador de VBlanks; no reserva timers del juego.

`genesis2_assets.py` extrae mapas y máscaras nativas revisadas, comprueba atributos
General y dimensiones reales con `(w+15)*(h+14) <= 10240`. Admite 40×140 y 140×20.
CRC de assets incluye mapa, metadatos, protección y atributos. Fingerprint canónico
incluye semilla, versiones, ID, georreferencia, dimensiones y hash de assets.

Las coordenadas proceden de conexiones reales incluyendo ciudades. Los ciclos
inconsistentes se registran; actualmente Oldale↔Ruta 103 tiene una inconsistencia
por dimensiones/offsets del mapa modificado. La extracción no inventa una
georreferencia válida para ese ciclo: el dominio afectado se aísla por mapa y
las siete filas/columnas de las conexiones permanecen nativas. Falta revisar
esa conexión antes de afirmar continuidad climática a través de toda Hoenn.

El runtime utiliza una raíz GBAX y una subarena propia fuera de SAVX. Datos y
cursores están en RAM restaurada por estados/rewind. No hay expulsiones ni
asignaciones por visita: el pool de las 14 rutas se reutiliza en el siguiente
inicio/carga. El catálogo ocupa 33.160 celdas: 66.320 bytes de tilemaps y hasta
132.640 bytes de índices dispersos, más descriptores. No toca VRAM ni DMA3.

Cada mapa se valida entero antes de READY. El catálogo actual solo transforma
suelo pasable conocido y conserva exactamente colisión/elevación. Agua, puentes,
saltos, escaleras, gates HM, scripts y eventos no se modifican. Esa invariancia
preserva el movimiento existente; **no demuestra topología procedural nueva**.

## Compilar y comprobar

```
make -C tools/genesis genesis2-test
make -C tools/genesis genesis2-asan
make GBAX=1 -j4 all
```

`GBAX=1` marca título/game code con el contrato exacto del SDK. La versión normal
sigue compilando sin acceso extendido. El SDK MPL-2.0 se copia de la rama
`feat/gbax-native-rewind` del núcleo; no se modifica su ABI.

Pruebas host: 1.000 semillas × 14 mapas, C99 con Wconversion/Werror y ASan/UBSan;
capacidad/alineación; protección exacta; invariancia de movimiento; dos presupuestos
de scheduler y orden inverso de consultas; diversidad; índices dispersos; corrupción
antes de validar y rechazo de publicación parcial; límites de coordenadas.

`gGenesis2Stats`: seed, mapas preparados, ticks, máximos VBlanks por tick/publicación,
cache hits/misses, celdas publicadas y estado. Es diagnóstico sin apropiarse de timers.
Un máximo cero VBlanks no demuestra un presupuesto submilisegundo. El harness ARM mide el
tramo de instrucciones en el núcleo; los tiempos físicos Android siguen pendientes.

## Pendiente de la especificación completa

Catálogo de módulos auténticos multimetatile, nuevos bosques/acantilados/orillas,
plan espacial materializado con nuevas gramáticas/topología y movimiento dirigido,
biomas sin arte preparado (tundra, etc.), cave/ocean pipelines, deltas SAVX compartidos,
100 previews nativos de módulos y QA física Android a 1×/3×. No usar la existencia
de este runtime para declarar terminadas esas etapas.


## Regresión ARM reproducible

`python3 tools/genesis/genesis2_arm_test.py --core ../mgba/build-libretro/mgba_libretro.so`

Ejecuta las funciones de la ROM compilada en el núcleo GBAX real. Usa RAM y
partidas sintéticas: prepara/reconstruye la misma semilla, comprueba los tilemaps
contra C99 host, publicación sin generación en entrada, cambios de scripts y
rechazo de identidad dañada. También prueba una copia sin la marca GBAX.
No sustituye una partida completa ni una prueba de tacto/audio en el móvil.

Medición sintética a 1× con semilla `0x12345678`: aproximadamente 1.280 callbacks de
preparación (no representa el tiempo total de instrucciones), máximo ~4,7 ms
por callback y 2,49 ms de publicación de las 13 rutas probadas tras excluir
el rescate de Birch. La prueba también verifica el fallback sin GBAX.

`python3 tools/genesis/genesis2_preview.py` exporta 100 previews de Ruta 102
a partir del mismo C y de metatiles/paletas reales: 69 resultados distintos
en las semillas 0–99. Son imágenes estáticas sin sprites ni animación de agua;
no son la certificación del compositor de módulos pendiente.
