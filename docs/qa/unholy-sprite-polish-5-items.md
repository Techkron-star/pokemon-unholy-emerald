# Tanda de corrección de sprites · Unholy Emerald

**Alcance:** Corazón de Titán, Ancla de Realidad, **Impulso Cósmico** (nombre oficial, motor warp en miniatura), Cetro Siniestro y Super Exp. Share.

El lote anterior de cuatro sprites fue rechazado visualmente. Se sustituyeron por cinco diseños pixel art **redibujados directamente en cuadrículas de 24×24**, sin reducir una ilustración grande a la fuerza. Se cuidaron la silueta, los reflejos, las sombras y la legibilidad a escala nativa.

### Corrección del Super Exp. Share
- El icono antiguo en la ROM representaba tres nodos pequeños; el nuevo es un medallón metálico con **tres nodos cian conectados**, conservando la idea original aprobada.
- El test `test/text.c:240` registró una descripción de 115 px donde el máximo permitido era 102 px (workflow run 42753979246). La descripción ahora dice:
  ```
  100% EXP for
  each eligible
  Pokémon.
  ```
- En Crafting, la imagen se coloca usando `sprite.x`/`sprite.y` directamente, con los offsets de animación en cero. No cambia los materiales ni el efecto.
- Los punteros a `gItemIcon_SuperExpShare` y su paleta ya existían en `src/data/graphics/items.h`; se conservan.

### Local QA aplicado
- **5/5** iconos en PNG indexado **4bpp** de 24×24, con **16 entradas de paleta** e índice 0 transparente.
- Paletas de los PNG comparadas byte a byte con sus archivos `.pal` JASC correspondientes.
- Decodificación de scanlines PNG y verificación de píxeles no transparentes.
- Comprobación visual de los cinco recursos ampliados a escala de píxel y a tamaño nativo sobre los fondos Eclipse y Daylight.
- Comprobación de rutas y nombres, sin agregar IDs ni mecánicas ficticias.
- Validador reproducible: `python3 dev_scripts/validate_unholy_icons.py`.

### Bloqueo antes de fusionar
La compilación de la ROM y QA real en emulador siguen pendientes. Además, el workflow de pruebas antiguo presentaba otras fallas distintas de esta descripción (compatibilidad de SaveBlock, captura y algunos tests). No confundir aprobar estos recursos con aprobar toda la ROM. El código de Gears en otras ramas todavía puede requerir declarar iconos en `src/data/graphics/items.h` antes de compilar.

**Ubicación:** los cuatro Gears todavía son recursos preparados para integrar en `graphics/unholy_concepts`; **Super Exp. Share sí está registrado como ítem** en el motor y su icono se reemplaza directamente.
