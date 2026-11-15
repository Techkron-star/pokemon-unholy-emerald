# DISEÑO — Unholy Emerald

> Especificación de diseño del proyecto **Unholy Emerald**, una base de ROM hack de Pokémon Emerald construida sobre `pokeemerald-expansion` (RHH) y la decompilación de `pret/pokeemerald`.
>
> Este documento consolida las decisiones de diseño tomadas en los PR abiertos (#9 a #18) y establece las reglas que deben respetar nuevas contribuciones.

---

## 1. Alcance y principios fundamentales

### 1.1. Qué es Unholy Emerald

Es una **base de desarrollo** para crear ROM hacks de Pokémon Emerald, no un juego jugable por sí mismo. Proporciona:

- Mecánicas modernas (nuevas generaciones de Pokémon, objetos, movimientos, habilidades).
- Sistema de **mutaciones** con tiers y badges visuales.
- Mercado **Horizon** con objetos, MTs, Pokémon y huevos con stock persistente.
- Sistema de **Gears** fabricables y equipables.
- UX mejorada de bolsos (siete bolsillos, selector rápido L+R).

### 1.2. Principios de diseño

| Principio | Descripción |
|-----------|-------------|
| **SaveBlocks ampliables y versionados** | Se permite usar los SaveBlocks ampliados o añadir espacio persistente para sistemas nuevos cuando sea necesario. Cada modificación debe documentar estructura, tamaño, cifrado, versión y compatibilidad de guardados; no reutilizar offsets ocupados. Las variables históricas de Horizon conservan su asignación salvo migración explícita. |
| **Compatibilidad Emerald + FRLG** | El código debe compilar en Pokémon Emerald, FireRed y LeafGreen. Las banderas específicas de FRLG deben declararse para evitar errores de compilación (ej. EXP Share). |
| **No romper mecánicas heredadas** | Precios base, IDs de objetos existentes, clasificaciones, bonos porcentuales y denominadores de RNG no se alteran sin justificación. Los cambios al esquema de guardado requieren versionado y pruebas específicas. |
| **Determinismo y persistencia** | Los sistemas con RNG (mercado, huevos, mutaciones) deben usar semillas persistentes para reproducir exactamente el mismo estado al abrir/cerrar, guardar/cargar y cambiar de pestaña. |
| **UI segura** | Las pantallas nuevas no deben dejar callbacks pendientes, ventanas residuales ni congelar al jugador. El cierre debe liberar recursos y reconstruir el campo correctamente (`CB2_ReturnToFieldWithOpenMenu`). |
| **Sin dualidad de callbacks verticales** | Mientras se navega en menús horizontales (Horizon), no usar `Menu_ProcessInputNoWrap` ni `Menu_MoveCursor` (pensados para menú vertical). |

---

## 2. Sistema de mutaciones

### 2.1. Taxonomía de tiers

Las nueve mutaciones se clasifican por potencia en cinco tiers:

| Tier | Sigla | Mutaciones |
|------|-------|------------|
| C | Alpha / Hyper | Alpha, Hyper |
| B | Super / Overloaded | Super, Overloaded |
| A | Enraged / Ancestral | Enraged, Ancestral |
| S | Titan / Ascendido | Titan, Ascendido |
| Ω | Omega | Omega |

### 2.2. Funciones centrales

Se proporciona una API central para trabajar con tiers:

- `MutationTierFromBadge(badge)` — devuelve el tier a partir del badge.
- `MutationTierFromMask(mask)` — devuelve el tier a partir de la máscara de mutación.
- `MutationTierCanAppearInMarket(tier)` — indica si un tier puede aparecer en el mercado Horizon.
- `MutationTierGetName(tier)` — devuelve el nombre legible del tier.

### 2.3. Badges visuales (16×16)

Los tres badges compactos aprobados para acompañar nombres de Pokémon en Resumen/Combate:

| Badge | Descripción | Archivos |
|-------|-------------|----------|
| **Hyper** | Doble flecha verde ascendente, centrada. | `graphics/mutations/badges/hyper.png`, `.pal` |
| **Titán** | Fortaleza de piedra con núcleo dorado, centrada. | `graphics/mutations/badges/titan.png`, `.pal` |
| **Overloaded** | Solo un rayo celeste con borde morado, fondo transparente. Sin círculo/medalla/placa adicional. | `graphics/mutations/badges/overloaded.png`, `.pal` |

**Formato:** PNG indexado 4bpp nativo 16×16, 16 entradas de paleta JASC, transparencia en índice 0. Fuentes editables en `graphics/mutations/badges/source_grid.json` y `SOURCE.md`.

> **Estado:** recursos de arte subidos, **sin integración runtime todavía**. Falta conectar la carga/dibujo con las mutaciones en UI de Resumen/Combate y verificar visualmente en emulador. (PR #18)

### 2.4. Reglas de aparición

- Huevos incluyen **todas** las mutaciones, independientemente de calidad o rango visual.
- Mercado Horizon de 6 Pokémon: límite **tier A** aplicado por función central. Los tiers **S y Ω** no pueden filtrarse desde stock antiguo.
- Máximo una mutación por oferta en el formato inicial del mercado.
- Omega solo desde 8 insignias.

---

## 3. Mercado Horizon

### 3.1. Estructura de pestañas

Horizon usa **cinco pestañas horizontales**: PKMN / DISCS / ITEMS / EGGS / MOVES.

- Navegación izquierda/derecha o L/R; A abre, B regresa.
- Root propia de 27×3 tiles, panel inferior de 27×14.
- Textos de stock, calidad de huevo, confirmación y avisos mutantes distribuidos en dos columnas.

### 3.2. Pestaña DISCS (MTs)

- Ya no vende cinco MT fijas: muestra **seis MT distintas** seleccionadas por medallas y período día/noche.
- Stock persistente mediante dos semillas en variables sin uso `0x40B8` y `0x40BB`.
- Listas sin duplicados, terminadas en `ITEM_NONE`, compatibles con compras repetidas del PokéMart.
- No vende Infinity Ball ni Gears.

### 3.3. Pestaña ITEMS

- Ofrece **tres básicos permanentes** (Poké Ball, Poción, Cuerda Huida) más **cinco objetos de RNG** con límites por progreso y horario.
- No vende Infinity Ball ni Gears.
- Semillas persistentes para reproducir inventario exacto al abrir/cerrar, guardar/cargar y cambiar de pestaña.

### 3.4. Pestaña PKMN (Pokémon)

- **Dos ofertas persistentes** en variables de guardado `0x40F7–0x40FE` (sin ampliar SaveBlock1).
- Sujeto a medallas y horario diurno/nocturno.
- RNG de rareza según denominadores de mutaciones existentes.
- Alpha y variantes progresivas por medallas; Omega solo desde 8 insignias.
- Máximo una mutación por oferta en este formato inicial.
- Compra funcional con A+A, costo variable, validación de saldo, almacenamiento en equipo/PC. Cada plaza se agota tras su compra.

### 3.5. Pestaña EGGS (huevos)

- **Cinco rangos de huevos:** Básico, Medio, Alto, Mítico y Legendario.
- Cada huevo visible recibe su propio roll de calidad (Estándar / Radiante / Prismática), persistente por período.
- Huevos con **icono real** en Horizon, sin revelar la especie.
- Al comprar, una tirada de contenido puede dar normales, míticos, legendarios y hasta **tres mutaciones**, Omega incluido.
- Huevo básico también tiene jackpot legendario muy excepcional.
- Calidad alta mejora chances pero **no garantiza** un resultado.

### 3.6. Pestaña MOVES

- Catálogo de movimientos (lista interna, por definir detalle de stock).

### 3.7. Alertas de stock raro (mutantes)

Jerarquía de alertas verídicas:

1. Punto rojo junto a HORIZON en el menú START.
2. Punto rojo en la pestaña Pokémon de Horizon.
3. Texto `!!! RARE STOCK` dentro de Horizon.
4. Detalle de mutación en la ficha del Pokémon.

El aviso se reconoce al entrar en el catálogo Pokémon, no solo al abrir Horizon. Abrir/cerrar el menú no regenera ofertas.

> **Excepción a "solo Pokémon normales":** El mercado Horizon permite la venta de Pokémon con mutaciones. Esta es una excepción intencional al principio de "solo Pokémon normales" y debe documentarse explícitamente en `diseño.md`. (PR #11)

### 3.8. Variables reservadas (sin ampliar SaveBlock)

| Variable | Uso |
|----------|-----|
| `0x40B8`, `0x40BB` | Semillas persistentes de stock DISCS / ITEMS |
| `0x40F7–0x40FE` | Ofertas de mercado PKMN |
| `0x40DB`, `0x40DC`, `0x40E5`, `0x40A1`, `0x40A8` | Variables de mercado/huevos (Horizon) |

---

## 4. Sistema de Gears

### 4.1. Descripción

Seis Gears fabricables con combinaciones de materiales y cantidades diferentes. Solo el **Cetro Siniestro** exige Medalla Mente.

### 4.2. Flujo de uso

- Panel Eclipse/Daylight: L/R recetas, A+A confirmación, SELECT equipamiento, arriba/abajo Pokémon.
- El Gear consume una copia del inventario y usa su slot independiente; el antiguo regresa a la mochila.
- Seis IDs nuevos de ítems (877–882) sin renumerar antiguos.
- Seis iconos pixel art preparados (24×24, indexed 4bpp, 16 posiciones de paleta, índice 0 transparente).

### 4.3. Gears (lote 1-3)

| Gear | ID | Nota |
|------|-----|------|
| (Gear 1) | 877 | — |
| (Gear 2) | 878 | — |
| (Gear 3) | 879 | — |
| (Gear 4) | 880 | — |
| (Gear 5) | 881 | — |
| Cetro Siniestro | 882 | Exige Medalla Mente |

Recetas detalladas en `docs/GEARS.md`. Fuentes editables en `docs/art_sources/gear_batch3_source_grid.json`.

---

## 5. UX del bolso (mochila)

### 5.1. Siete bolsillos claros

- Cinta persistente de **siete pestañas identificadas** arriba de la lista.
- Resaltado del bolsillo actual y puntos discretos para bolsillos con contenido.
- No altera clasificación de objetos ni SaveBlock.
- Conserva cursores, desplazamiento, animación L/R, ordenación START y cambio de posición SELECT.

### 5.2. Selector rápido L+R

- Abre un selector que muestra los siete bolsillos con nombres completos y recuentos de pilas.
- A salta directamente al bolsillo seleccionado; B cancela.
- Navegación arriba/abajo o izquierda/derecha.

### 5.3. Restricciones

- Contextos de batalla, tutorial y link no afectados por los cambios de navegación entre bolsillos.
- QA documentada en `docs/BAG_UX.md`. (PR #15)

### 5.4. Evolución de mochilas (diseño aprobado; implementación pendiente)

La mochila evoluciona de manera **permanente**. Cada mejora aumenta **dos capacidades distintas**: (1) los slots disponibles para tipos de objetos y (2) la cantidad máxima que se puede almacenar de **cada objeto**.

| Tier de mochila | Máximo por objeto | Slots por bolsillo |
|-----------------|------------------:|--------------------|
| **Mochila normal** | **999** | Capacidad base; cifras por definir |
| **Gran Mochila** | **9,999** | Más slots que la normal; cifras por definir |
| **Mochila Dimensional** | **99,999** | Más slots que la Gran Mochila; cifras por definir |

**Reglas funcionales:**

1. Los límites `999 → 9,999 → 99,999` son **cantidades por ID de objeto**, no una suma global de la mochila ni un límite por transacción.
2. Los slots y las cantidades máximas aumentan de forma acumulativa. El número exacto de slots por bolsillo se decidirá después; no asumir valores ni porcentajes inventados.
3. Un mismo tipo de objeto se presenta como **una sola entrada** en la UI (por ejemplo, `Poción ×99,999`), sin exigir al jugador gestionar pilas duplicadas para alcanzar el máximo. La implementación debe preservar la clasificación automática por bolsillo.
4. Al mejorar la mochila, **ningún objeto se borra ni se reinicia**; se conservan IDs, cantidades, orden y bolsillos. No se contempla una degradación voluntaria de tier.
5. El límite dinámico depende del tier desbloqueado, **no** de una constante única que permita 99,999 unidades desde el comienzo.
6. Objetos únicos, claves, equipamiento y otras categorías con reglas especiales conservan sus restricciones propias; aumentar el límite general no habilita duplicaciones indebidas.
7. Los requisitos, ubicaciones y momentos de desbloqueo de Gran Mochila y Mochila Dimensional **no están definidos todavía**. Las mejoras deben seguir el progreso legítimo de la aventura, no saltarlo.

### 5.5. Arquitectura de almacenamiento

**Estado del código base observado:** `struct ItemSlot` en `include/global.h` contiene `u16 quantity`, que solo representa valores de 0 a 65,535 y **no permite 99,999**. La ampliación del SaveBlock por sí sola no cambia ese tipo de dato.

**Implementación requerida para la Mochila Dimensional:**

- Usar `u32` para las cantidades persistentes de inventario (al menos en `ItemSlot` y en las representaciones equivalentes afectadas). `u32` permite guardar 99,999 sin formatos de bits especiales.
- Auditar y actualizar operaciones, parámetros, variables temporales y retornos que limiten cantidades a `u16` o `s16`: añadir, quitar, comprobar, transferir, contar, vender, ordenar, craftear, consumir y rutas de debug. En particular, revisar `include/item.h`, `src/item.c`, `src/item_menu.c` e integraciones de Horizon/Gears.
- Reemplazar las comprobaciones estáticas de `MAX_BAG_ITEM_CAPACITY` en las rutas relevantes por una **capacidad efectiva basada en el tier** de mochila. Las operaciones deben validar desbordamiento y respetar el máximo por tipo de objeto.
- Adaptar el **cifrado de cantidades**: la lectura/escritura actual aplica XOR con `gSaveBlock2Ptr->encryptionKey` sobre cantidades de 16 bits; al ampliar a 32 bits se debe mantener una transformación reversible y compatible con los datos versionados.
- Reservar espacio suficiente en los SaveBlocks nuevos o ampliados; recalcular offsets, estructuras, sectores de guardado y consumo de RAM. Ampliar `u16 → u32` añade **2 bytes por entrada** afectada, antes de cualquier padding o estructura auxiliar.
- Guardar el **tier de mochila** como dato persistente y coherente con el inventario. Versionar el formato del guardado y definir migración o invalidación explícita de saves previos durante el desarrollo; nunca interpretar silenciosamente el layout antiguo como nuevo.
- Actualizar la UI y sus buffers para **cinco dígitos**, incluido el ancho de ventanas y menús de cantidad. Corregir selectores cuya lógica interna emplee `s16`, y asegurar que operaciones parciales con cantidades grandes no trunquen valores.
- Comprobar capacidad y límites del formato de guardado en GBA y en el emulador objetivo. **SaveBlocks adicionales no equivalen a memoria ilimitada.**

### 5.6. Criterios de aceptación y QA

- Probar cada tier con cantidades `0`, `1`, `998`, `999`, `1,000`, `9,998`, `9,999`, `10,000`, `65,535` y `99,999` según corresponda.
- Verificar que compras, recompensas, debug y crafting **no superen el máximo activo**, no dupliquen ni destruyan objetos por errores de truncamiento.
- Comprobar que al mejorar de mochila se conservan objetos, cantidades, orden y distribución de bolsillos; probar inventarios llenos en los tres tiers.
- Validar guardar/cargar, reinicio, cifrado, venta de grandes cantidades, consumo en batalla y equipamiento de Gears.
- Revisar en emulador GBA que la UI muestra cinco dígitos sin texto cortado, desplazado ni superpuesto.
- Mantener estas especificaciones como **diseño pendiente** hasta que exista implementación y QA verificadas.

---

## 6. Navegación y cierre de menús

### 6.1. Horizon horizontal (PR #16)

- Cierre de Horizon libera ventanas, icono de huevo y paleta temporal, luego ejecuta `CB2_ReturnToFieldWithOpenMenu` para reconstruir el campo y el menú START.
- Antes se dejaba un callback pendiente que no reconstruía correctamente el menú.

### 6.2. Pestañas Coming Soon (PR #10)

- Pestañas POKÉMON y EGGS visibles como **Coming Soon**.
- Al confirmar con A no destruyen el menú ni reconstruyen ventanas mediante callbacks de mensajes.
- Se reproduce un sonido de acción no disponible, sin salir de Horizon.
- No habilita aún compras de Pokémon/Huevos.

---

## 7. Reglas de compatibilidad y restricciones

1. **Emerald, FireRed y LeafGreen** deben compilar y pasar los tests sin regresiones heredadas.
2. **SaveBlocks versionados:** sus tamaños y estructuras pueden ampliarse justificadamente (incluida la mochila evolutiva), con auditoría de memoria, serialización, cifrado y guardado/carga. No reinterpretar saves antiguos sin una migración definida.
3. **Denominadores de RNG** no cambian; se bumps la versión de stock para evitar reinterpretación de catálogos antiguos.
4. **IDs de objetos antiguos** no se renumeran.
5. **Gráficos de juego:** PNG 4bpp indexado, paletas JASC, índice 0 transparente.
6. **PRs en borrador:** no fusionar a `master` hasta validación de CI y QA en emulador GBA.

---

## 8. Estado de PRs abiertos (referencia)

| PR | Título | Estado | Pendiente clave |
|----|--------|--------|-----------------|
| #18 | Badges de mutación 16×16 (Hyper, Titán, Overloaded) | Draft | Integración runtime en UI de Resumen/Combate |
| #17 | Horizon: objetos y discos con rotación real | Draft | QA en emulador, auditoría IDs UNUSED |
| #16 | Horizon horizontal y cierre seguro a START | Draft | Compilación Emerald/FRLG, QA visual |
| #15 | Mochila UX: 7 bolsillos + selector L+R | Draft | Compilación, QA visual 240×160 |
| #14 | Tiers unificados para 9 mutaciones | Draft | CI y QA de GBA |
| #13 | Horizon: 6 Pokémon + 5 huevos calidad RNG | Draft | Compilación GBA, test de guardados |
| #12 | 3 sprites de Gears 24×24 | Draft | Compilación CI, verificación visual |
| #11 | Horizon market real + alertas rojas | Draft | QA, revisión de DISENO.md (este archivo) |
| #10 | Evitar bloqueos en pestañas Coming Soon | Draft | Compilación Emerald/FRLG |
| #9 | Crafteo y equipamiento de 6 Gears + FRLG | Draft | Compilación, QA en emulador, 118 errores CI previa |

---

## 9. Convenciones para contribuciones

- Los nombres de funciones y structs usan `PascalCase`.
- Las variables y campos usan `camelCase` (prefijo `g` para globales, `s` para estáticos).
- Macros y constantes usan `CAPS_WITH_UNDERSCORES`.
- Archivos `.c` y `.h`: 4 espacios, sin tabs.
- Comentarios explican el _porqué_, no el _qué_.
- Todo PR nuevo debe actualizar este `diseño.md` si introduce cambios de arquitectura, nuevas variables de guardado, nuevas pestañas de UI o excepciones a los principios listados.

---

## 10. La Cofradía y la Torre Celestial (postgame; diseño pendiente)

> **Estado:** concepto narrativo y mecánico aprobado para documentación. **No implica que esté implementado.** La reforma de la Torre de Batalla, la isla y sus combates queda para una fase posterior.

### 10.1. Identidad, territorio y función narrativa

- **La Cofradía** es una organización **neutral-negativa**: no actúa por odio al protagonista ni tiene por qué aspirar a destruir Hoenn, pero protege sus intereses mediante corrupción, ventajas ilegítimas y manipulación institucional.
- Controla **toda la isla del postgame donde está la actual Torre de Batalla**, además de administrar la propia instalación. La Torre de Batalla será rebautizada como **Torre Celestial**.
- Ejerce un **gobierno de facto** sobre la isla: decide reglas, acceso competitivo, clasificación, recompensas e intervenciones. Su control es previo a la llegada del protagonista y no desaparece porque este gane combates.
- La Cofradía **no puede ser derrocada por el jugador**. No se plantea una misión de liberación de la isla ni un final que destituya al gobierno. Se puede vencer a sus representantes, no quitarles el poder.
- La isla y la torre deben sentirse como una institución elegante, autosuficiente y abiertamente oportunista. Los empleados pueden hablar con corrección burocrática incluso cuando cometen abusos evidentes.
- El daño al jugador es **indirecto y sistémico**: el protagonista quiere competir y obtener recursos; la Cofradía quiere conservar el control de la clasificación y evitar que un competidor acapare las recompensas.

### 10.2. Economía: el mejor lugar para farmear

La **Torre Celestial** debe ser una de las mejores actividades repetibles del postgame para conseguir:

- **Dinero** mediante victorias, rachas, puestos y desafíos.
- **Gears** raros y materiales relacionados con ellos.
- **Recompensas de alto valor** del postgame, cuya tabla exacta queda por definir.

**Regla de diseño:** pese a la corrupción, participar debe seguir siendo rentable. La Cofradía crea obstáculos deliberados, pero no debe hacer inútil el farmeo ni bloquear permanentemente el acceso a las recompensas. No se fijan todavía precios, porcentajes de impuestos, cantidades, monedas adicionales ni límites por sesión.

### 10.3. Intervenciones institucionales y dificultad dinámica

La Cofradía puede alterar un combate futuro para **romper una racha, bajar al jugador en el ranking o frenar sus ganancias**. Estas alteraciones son actos de sabotaje dentro del universo del juego, no un modo de dificultad global elegido por el jugador.

Posibles factores para decidir una intervención (sin umbrales numéricos definitivos):

- Racha actual y frecuencia de victorias.
- Posición y ascenso del jugador en la clasificación.
- Ganancias/recompensas acumuladas de la Torre Celestial.
- Resultado de intervenciones anteriores y nivel de amenaza percibido.

Intervenciones permitidas como diseño:

| Tipo | Ejemplo | Intención |
|------|---------|-----------|
| Refuerzo | Mejorar sets, IA, objetos equipados y Gears del rival | Dificultar victorias previsibles |
| Sabotaje | Introducir debuffs o restricciones temporales de combate contra el jugador | Alterar las condiciones a favor de la casa |
| Patrocinio | Proporcionar al rival Pokémon con mutaciones raras o combinadas | Romper rachas excepcionales |
| Intervención ejecutiva | Preparar un equipo extraordinario con legendarios, mutaciones acumuladas y equipamiento de élite | Impedir que el jugador domine el ranking |

- Una intervención puede ser **repentina**; no tiene que estar limitada a jefes de pisos predefinidos. Su aparición debe tener reglas y RNG verificables, no ser un resultado imposible de anticipar o reproducir técnicamente.
- Una intervención puede entregar al rival **tres Pokémon Omega con mutaciones adicionales**, si las reglas del encuentro lo autorizan.
- El apilamiento de bonificaciones de mutación debe respetar la regla general del proyecto: **se suman las bonificaciones porcentuales aplicables**, no se multiplican entre sí.
- La Cofradía puede infringir **sus propias normas competitivas**, pero no debe romper los invariantes del motor, corromper partidas ni crear estados irrecuperables.
- La intervención debe **anunciarse** mediante un comunicado breve; no necesariamente revelar toda la composición del equipo. El jugador debe reconocer que la dificultad extraordinaria es intencional.
- Los rivales pueden ser competidores normales que reciben apoyo institucional: no es obligatorio que todos pertenezcan a La Cofradía.
- Las recompensas por superar encuentros intervenidos deben compensar el riesgo; los multiplicadores y premios concretos **siguen pendientes de balanceo**.
- No expulsar permanentemente al jugador ni cerrar la torre por superar demasiadas intervenciones: la Cofradía sigue beneficiándose de su participación.

**Ejemplo de comunicado:**

> «Su rendimiento excede los parámetros previstos. El Consejo ha autorizado medidas extraordinarias para garantizar el equilibrio competitivo».

**Ejemplo de respuesta posterior:**

> «La Cofradía garantiza la igualdad de oportunidades, no la igualdad de resultados».

### 10.4. Encuentro extremo ilustrativo: Ultra Necrozma y Floette Flor Eterna

Caso memorable **posible**, no obligatorio ni fijo en cada racha:

1. La Cofradía subvenciona un **Ultra Necrozma Omega**, incluso con otras mutaciones como **Hyper y Ascendido**, según las reglas de apilamiento habilitadas.
2. Puede llevar un **objeto que potencie Ataque Especial** (por ejemplo, Gafas Elegidas, con sus restricciones habituales, o una variante propia de Unholy si se define), además de **un Gear independiente**.
3. Gears candidatos para variantes de este combate: **Psicosangre**, **Tesseracto** o **Báculo Tormentoso**. **No son tres Gears simultáneos**: cada configuración elige uno, salvo cambio futuro explícito del sistema general de equipamiento.
4. Si el jugador consigue derribar a Ultra Necrozma, el siguiente Pokémon del rival **puede ser Floette Flor Eterna**. Su mutación, objeto y Gear específicos quedan a decisión del balance y de las variantes del encuentro.
5. Otros miembros del equipo pueden ser Pokémon inusuales con sets deliberadamente diseñados para frenar la racha.

Este ejemplo busca transmitir la magnitud de una intervención de la Cofradía, **no establece una pelea imposible como requisito para farmear**. La composición, la frecuencia, los niveles y los multiplicadores finales requieren pruebas.

### 10.5. Buzón de quejas en la pared

**Ubicación:** una pared accesible del vestíbulo de la Torre Celestial, con el rótulo **«Buzón de quejas»**.

**Interacción:**

1. El jugador pulsa **A** frente al buzón.
2. Se muestra una interfaz breve para **escribir una reclamación** usando el sistema de entrada de texto que admita el motor.
3. Tras elegir **«Enviar»**, suena **inmediatamente un efecto de trituradora de papel**, procedente de detrás de la pared.
4. Después aparece el mensaje: **«Su reclamación ha sido procesada exitosamente»**.
5. El empleado cercano puede declarar: «Todas las quejas son atendidas personalmente por nuestro departamento». La trituradora vuelve a escucharse si se presenta otra.

**Tono:** humor ambiental y burocrático. Nadie explica el ruido ni admite abiertamente que la máquina destruye las quejas. **No desbloquea una misión para derrocar a La Cofradía.**

**UX y alcance:** permitir cancelar la escritura antes de enviar; no bloquear el movimiento al cerrar; evitar guardar cadenas arbitrarias en el SaveBlock si no son necesarias. El sonido requiere un recurso compatible con el motor y una comprobación en emulador. Los textos deben caber en las ventanas de GBA sin recortes.

### 10.6. Contadores, mensajes y persistencia

- Puede mostrarse un contador de **intervenciones institucionales superadas** como estadística de prestigio de la Torre Celestial. Es propuesta de interfaz, no obligación de la primera versión.
- Guardar, cuando se implemente, los datos mínimos para preservar rachas, ranking, premios e intervenciones pendientes. Evitar exploits de reiniciar o recargar para convertir cada intervención en una tirada gratuita diferente.
- No introducir números definitivos (porcentajes, umbrales de racha, impuestos o recompensas) antes de las pruebas de dificultad y economía.
- Textos institucionales breves, naturales, irónicos y consistentes; evitar explicaciones técnicas o etiquetas de desarrollo en diálogos visibles para el jugador.

---

## 11. Gear: Báculo Tormentoso (diseño pendiente)

**Categoría:** Gear equipable para Pokémon, independiente del objeto equipado ordinario.  
**Función:** potenciar ataques especiales de daño con una segunda descarga, a cambio de suprimir efectos secundarios y restringir el encadenamiento especial.

### 11.1. Efecto principal

1. Cuando el portador usa un **movimiento especial que inflige daño** y este conecta, el movimiento impacta **una segunda vez con el 25 % de su potencia original**.
2. La segunda descarga es **un impacto separado**, no un aumento plano del 25 % a la estadística de Ataque Especial ni una promesa de infligir exactamente 1,25 veces el daño final. La fórmula debe resolver cada impacto bajo las reglas del motor y definir su redondeo.
3. El movimiento **pierde sus efectos secundarios** cuando se usa bajo el Báculo. Esto incluye efectos accesorios como posibles estados alterados, retrocesos o cambios de estadísticas causados como efectos adicionales. La supresión debe afectar a ambas descargas.
4. El portador **no puede encadenar dos movimientos especiales juntos en una misma cadena**. Se pueden estudiar cadenas con un físico y un especial, en cualquier orden, siempre que cumplan las restricciones generales de encadenamiento.
5. La prohibición anterior **no impide** usar movimientos especiales en turnos distintos ni convierte todos los movimientos del usuario en físicos.
6. El Gear no autoriza más ataques de los permitidos por el sistema de encadenamiento ni anula la regla existente por la cual un KO cancela la cadena.

### 11.2. Interpretación pendiente de casos límite

Para evitar comportamientos contradictorios, la implementación debe fijar y probar explícitamente:

- **Costes o efectos obligatorios del movimiento** (retroceso de daño, recarga, reducción propia garantizada, etc.): no asumir que son «efectos secundarios» eliminables. Propuesta conservadora: conservar esos costes y eliminar únicamente efectos accesorios.
- **Luz Aniquiladora de Floette Flor Eterna:** su retroceso obligatorio debe tratarse como coste del movimiento, no eliminarse inadvertidamente.
- **Movimientos de estado, daño fijo, golpes múltiples y movimientos que fallan:** definir excepciones; no crear una segunda descarga si no existe impacto especial válido.
- **Sustituto, Banda Focus, Robustez, resistencias, inmunidades, escudos, KO y movimientos de protección:** evaluar los dos impactos conforme a las reglas del combate; no forzar una segunda descarga tras una interrupción de combate que la invalide.
- **Stamina y PP:** la segunda descarga es parte de la misma ejecución, no un segundo movimiento elegido; propuesta: no cobrar otra vez PP/stamina por el impacto adicional.
- **Gears y habilidades activadas al impactar:** evitar recursión o un tercer golpe no previsto. Precisar interacciones con otras mecánicas antes de programar.
- **Interfaz:** mostrar el Gear activo en Resumen y HUD de batalla y dar una indicación visual/sonora de la segunda descarga. La restricción de encadenamiento debe verse antes de confirmar una cadena inválida.
- **Objeto + Gear:** puede coexistir con un objeto que suba Ataque Especial porque son slots distintos. Esto exige balancear el daño total, sin fingir que Psicosangre, Tesseracto y Báculo funcionan simultáneamente.

### 11.3. Disponibilidad y balance

- El **Báculo Tormentoso** es un nuevo Gear propuesto para Unholy Emerald, no una función ya programada.
- Puede ser una recompensa rara de la **Torre Celestial** o formar parte de su equipamiento de élite; receta, tasa de obtención, ID, descripción final, sprite y sonido **pendientes**.
- La Cofradía puede usarlo en sus intervenciones con equipos ofensivos de alto nivel, incluido Ultra Necrozma.
- Debe ser poderoso sin invalidar por completo las demás opciones de Gear. Comparar su rendimiento frente a **Psicosangre** y **Tesseracto** en combates comunes, jefes y cadenas.

### 11.4. Pruebas de aceptación futuras

- El segundo golpe aparece solo en movimientos especiales de daño válidos, y su potencia equivale al 25 % de la original según el cálculo acordado.
- Ninguno de los dos impactos activa los efectos secundarios eliminados; los costes obligatorios siguen la regla documentada.
- La interfaz rechaza **especial → especial** en la misma cadena, sin bloquear por error **físico → especial** y **especial → físico** cuando esas cadenas sean legales.
- No se descuentan indebidamente PP/stamina adicionales, ni se duplica una activación de Gear hasta producir bucles.
- El HUD representa el Gear y el segundo impacto sin solapamientos; validar en GBA real/emulador.
- Las intervenciones de La Cofradía no destruyen la persistencia de la racha ni las recompensas de la Torre Celestial.
- El buzón deja cancelar, reproduce la trituradora al enviar y devuelve el control sin bloquear menús.

**Prioridad de implementación:** documentar ahora; implementar Torre Celestial, Cofradía y Báculo Tormentoso en una fase futura del postgame, sin desviar el trabajo actual de UI, estabilidad y mecánicas base.

---

## 12. Encadenamiento 2.0 y regeneración de stamina (balance propuesto)

> **Estado:** ajustes de diseño aceptados para la próxima fase de balance; no afirmar que están implementados hasta verificar el motor y la ROM en emulador.

### 12.1. Objetivo

Que encadenar dos ataques continúe siendo una herramienta táctica potente, pero no equivalga a usar dos movimientos de daño completo por el coste de un turno. Conservar la posibilidad de mezclar tipos, ataques físicos y especiales. Las mismas reglas se aplican al jugador y a los entrenadores controlados por la IA, incluida La Cofradía.

### 12.2. Reglas propuestas

| Elemento | Regla |
|---|---|
| Primer ataque de una cadena | 100 % de su daño normal; coste normal de stamina |
| Segundo ataque de la cadena | **70 % del daño normal**, calculado para ese ataque |
| Stamina del segundo ataque | **150 % de su coste normal** (incremento del 50 %), con redondeo coherente y comprobación de recursos antes de confirmar |
| Regeneración pasiva | **18 % de la stamina máxima por turno**, antes 20 %; solo una vez al final del turno |
| Movimiento individual | 100 % de daño y coste normal; no recibe la penalización de cadena |
| KO en el primer ataque | La cadena se cancela; no atacar al sustituto recién enviado |
| Movimientos de estado | **No encadenables** en la versión inicial de este balance; no quedan prohibidos como acciones normales |
| Simetría | Las mismas reglas de daños, costes, cancelaciones y regeneración rigen para la IA |
| Báculo Tormentoso | Sigue prohibido encadenar **especial → especial**; las combinaciones mixtas se permiten si cumplen todas las demás condiciones |

El **70 %** corresponde al modificador de daño del **segundo movimiento**, no a una modificación permanente de su potencia base. Las resistencias, el STAB, los críticos, las habilidades, las mutaciones y los Gears deben calcularse con normalidad, aplicando el modificador una sola vez de forma consistente.

**Regeneración:** al finalizar cada turno completo, recuperar aproximadamente `floor(staminaMaxima × 0.18)` puntos, con mínimo de 1 si corresponde recuperar y todavía falta stamina, sin superar el máximo. La cifra del 18 % es inicial y deberá ajustarse con pruebas; **no regenerar entre las dos acciones de una misma cadena**. Si el motor tiene otras fuentes explícitas de recuperación, estas se resuelven aparte y sin duplicar la regeneración pasiva.

### 12.3. Orden de ejecución de las acciones

Evitar que un Pokémon rápido encadene dos movimientos consecutivos antes de que el rival tenga oportunidad de actuar solo por tener mayor Velocidad:

1. Resolver **la primera acción** de cada combatiente según las reglas existentes de prioridad y Velocidad.
2. Si la cadena sigue siendo válida, resolver **las segundas acciones** según prioridad y Velocidad entre quienes tengan segundo movimiento pendiente.
3. En caso de KO, cambio de Pokémon, incapacidad para actuar, pérdida de objetivo válido o interrupción prevista en las reglas del motor, cancelar o revalidar la segunda acción; **nunca dirigir automáticamente el segundo ataque contra el Pokémon que acaba de entrar por un KO**.
4. Resolver el cierre normal del turno y **una sola regeneración pasiva** por Pokémon según el sistema de stamina.

Esto es un objetivo de comportamiento, no una orden de sustituir a ciegas la cola de acciones: las prioridades, ataques dobles, acciones de cambio, interrupciones y casos de multibatalla requieren integración cuidadosa con la lógica existente.

### 12.4. Integración con Gears y efectos adicionales

- **Báculo Tormentoso:** su segundo impacto del 25 % es parte **del mismo movimiento especial**, no cuenta como un segundo movimiento encadenado ni incurre en el recargo del 50 % de stamina de cadena.
- Si el movimiento con Báculo ocupa la **segunda posición de una cadena mixta válida**, la reducción de daño del **70 %** debe afectar de manera coherente el daño de ambos impactos de ese movimiento, sin aplicarse dos veces ni crear una tercera descarga.
- Evitar activaciones recursivas de Gears y habilidades que produzcan ataques extra no permitidos.
- Conservar las reglas acordadas para que un KO interrumpa el encadenamiento, incluidos combates con mutaciones y jefes especiales.
- La interfaz debe comunicar claramente **coste de stamina**, orden de ataques y restricciones antes de confirmar; no permitir que la selección de movimientos cree una cadena inválida o gaste recursos sin ejecutar una acción válida.

### 12.5. Balance y pruebas antes de activar

- Comparar ataques individuales, cadenas con dos ataques, cadenas mixtas y alternativas con movimientos de estado.
- Verificar la regeneración del 18 % con máximos de stamina grandes/pequeños, redondeo, stamina casi llena y stamina insuficiente para el segundo ataque.
- Confirmar que una cadena no regenera stamina entre ataques y que el segundo gasto del 150 % se aplica al movimiento correcto, sin desbordamiento.
- Validar que la IA cumple las mismas limitaciones y que nunca vuelve a actuar tras cancelar la cadena por KO.
- Probar interacciones con **Báculo Tormentoso, Psicosangre, Tesseracto**, mutaciones acumuladas y las intervenciones de **La Cofradía**, sin ventajas ocultas del motor para NPC.
- Hacer QA de la UI de selección y ejecución en emulador: mensajes naturales, datos alineados, animaciones correctas y ausencia de acciones repetidas involuntariamente.

**No cambiar todavía otros sistemas de stamina ni tasas propias de habilidades/Gears** sin medir primero el efecto de estas tres modificaciones principales: 70 % de daño en el segundo ataque, +50 % a su coste de stamina y 18 % de regeneración pasiva.

---

## 13. Supervivencia al daño: protección de turno y ataques multigolpe (rework propuesto)

> **Decisión de diseño:** reequilibrar **Resistencia (Endure)**, **Banda Focus (Focus Sash)** y efectos de supervivencia similares para que el **encadenamiento de movimientos separados no anule automáticamente la protección**, mientras que los **movimientos multigolpe sí pueden superar esa protección con impactos sucesivos**. Pendiente de implementación y pruebas.

### 13.1. Distinguir movimientos encadenados e impactos del mismo movimiento

- **Cadena:** dos movimientos **distintos** ejecutados durante el mismo turno (p. ej., Rayo → Psíquico). Cada uno es una acción/movimiento independiente aunque compartan turno.
- **Multigolpe:** **un solo movimiento** con varios impactos internos (p. ej., Pedrada o Ataque Furia); no es una cadena y no se le aplica el coste extra de encadenamiento.
- **Impacto adicional del Báculo Tormentoso:** es otro impacto **dentro del mismo movimiento especial**, no una segunda acción de cadena. Por tanto, se trata como multigolpe a efectos de romper esta protección.

### 13.2. Escudo de supervivencia durante el turno

Cuando una protección de tipo «sobrevivir con 1 PS» está vigente o se activa legítimamente:

1. **Contra movimientos separados de una misma cadena**, conserva una protección de **mínimo 1 PS hasta que termine ese turno**. Si la primera acción la activa, la segunda acción encadenada **no puede eliminarla solo por ser el segundo movimiento**.
2. **Contra un único movimiento multigolpe**, la protección puede salvar del **primer impacto que de otro modo sería letal**, pero **no cubre indefinidamente los impactos posteriores de ese mismo movimiento**. Si aún hay impactos y causan suficiente daño, el Pokémon puede caer **en ese mismo turno**.
3. **La protección expira al terminar el turno**; no se traslada al siguiente. El estado «ya activado en este turno» no permite volver a activar una Banda Focus consumida.
4. El umbral se aplica sobre **daño de movimientos**. Daño residual (veneno, quemadura, clima, etc.), autodaño y otros costes de final de turno mantienen sus reglas propias; no quedan cubiertos automáticamente.
5. Los ataques y habilidades deben seguir resolviendo inmunidades, habilidades, efectos de objeto y excepciones existentes; esta regla no convierte al objetivo en invulnerable.

**Ejemplo A — cadena que NO remata:** un Pokémon a PS completos recibe un primer ataque letal; Banda Focus se activa y lo mantiene a 1 PS. La siguiente acción **de esa misma cadena**, aunque haga daño, no lo remata durante ese turno. Al terminar, el escudo desaparece y la Banda Focus ya está consumida.

**Ejemplo B — multigolpe que SÍ puede rematar:** un movimiento golpea cuatro veces. Un impacto activa la protección y deja al objetivo a 1 PS; **otro impacto del mismo movimiento** lo alcanza y puede debilitarlo. Si no quedan impactos tras activarse el escudo, sobrevive.

**Ejemplo C — Báculo Tormentoso:** el primer impacto de un movimiento especial activa Banda Focus o Resistencia; la descarga del **25 %** es un impacto posterior **del mismo movimiento** y puede rematar si supera la protección ya gastada para ese movimiento.

### 13.3. Aplicación por mecánica

| Efecto | Regla especial de Unholy Emerald |
|---|---|
| **Resistencia / Endure** | Mientras esté activa, mantiene el mínimo de 1 PS ante **movimientos separados** del turno, incluidas cadenas; los impactos posteriores **dentro del mismo movimiento multigolpe** pueden romper ese mínimo. |
| **Banda Focus / Focus Sash** | Mantiene su condición inicial de estar a **PS completos** para activarse frente a un golpe letal; se consume una sola vez y el **escudo resultante dura hasta fin de turno** frente a movimientos separados. Un multigolpe puede rematar tras la activación. |
| **Robustez / Sturdy** (si aplica) | Conservar el requisito de PS completos para la activación; aplicar la misma protección temporal contra acciones separadas y la misma vulnerabilidad a impactos posteriores de un único multigolpe. |
| **Cinta Focus / Focus Band** (si aplica) | Mantener la probabilidad existente de activación ante daño letal; **no repetir tiradas por cada golpe de una cadena** una vez activado el escudo. Si se activa, dura el resto del turno ante movimientos separados y puede romperse por impactos del mismo multigolpe. |

- Estas reglas son intencionalmente distintas de la mecánica estándar y requieren un tratamiento explícito en el motor.
- **Protección/Detección y movimientos que bloquean por completo ataques no son equivalentes a «sobrevivir con 1 PS»**. Mantienen sus propias reglas y no se declaran vulnerables a multigolpes por esta decisión.
- Si coinciden varios efectos de supervivencia, definir **prioridad y consumo** para evitar que se creen escudos ilimitados o que un objeto se gaste si otro efecto ya resolvió el golpe.
- **Simetría completa entre jugador e IA**, incluso para los equipos de La Cofradía.

### 13.4. Requisito técnico y persistencia

La rutina de daño debe conocer **turno**, **identificador de ejecución de movimiento**, **índice de impacto**, **origen y condición de activación del escudo** y si la protección **ya salvó un impacto letal dentro de esa ejecución**. No basar todo en un flag genérico «protegido durante el turno» que haga invulnerable también a todos los impactos de multigolpe.

Modelo conceptual:

- \`survivalGuardActiveThisTurn\`: protección temporal activa tras un disparador válido.
- \`survivalGuardConsumedForMoveExecution\`: protección ya usada contra un impacto letal dentro de la **ejecución actual**; si llegan más impactos del mismo movimiento, **no se vuelve a poner el objetivo a 1 PS automáticamente**.
- Al iniciar **un nuevo movimiento distinto dentro del mismo turno**, la protección temporal **sí** puede conservar el mínimo de 1 PS.
- Al cerrar el turno, limpiar los estados transitorios. No persistir flags temporales en SaveBlock; los consumos reales de objeto y la partida guardada siguen sus reglas normales.

Los nombres anteriores son **conceptuales**, no API ni estructuras existentes verificadas.

### 13.5. Casos de QA obligatorios

- Banda Focus: primero letal y segundo ataque encadenado letal → termina el turno con **1 PS**, Banda Focus consumida.
- Resistencia: dos movimientos separados en un turno, ambos letales → termina con **1 PS** si no hay daño residual.
- Banda Focus/Resistencia: movimiento multigolpe cuyo golpe letal ocurre antes del último impacto → otro impacto interno **puede causar KO**.
- Multigolpe que solo se vuelve letal **en el último impacto** → la protección puede salvar; **no** afirmar que todo multigolpe mata automáticamente.
- Báculo Tormentoso: impacto inicial letal con protección y segundo impacto de 25 % → puede rematar, respetando mitigaciones y daño final real.
- Banda Focus bajo PS completos perdidos antes de activarse → **no activación gratuita**; Robustez mantiene su condición equivalente.
- Cinta Focus: sin tiradas repetidas abusivas, sin escudos permanentes, interacciones documentadas con otros efectos.
- Cadenas interrumpidas por KO, dobles, habilidades, Gears y ataques con varios objetivos → sin estados residuales ni consumo doble.
- Verificar que las mismas condiciones afectan a los entrenadores enemigos y que los mensajes/HUD representen las activaciones correctamente.

---

## 14. Estados alterados y stamina (propuesta de balance)

> **Estado:** idea de diseño incorporada para pruebas; porcentajes **provisionales**, sujetos a balance y aprobación final. Esta sección no implica código implementado. Conserva el valor base de regeneración de **18 % de stamina máxima al final de cada turno** (sección 12).

### 14.1. Objetivo

Los estados alterados influyen en la **economía de stamina** además de sus efectos normales. Cada estado debe tener identidad mecánica; evitar convertir todos en un simple «-X de stamina» o hacer que un Pokémon sin stamina quede atrapado en un bucle irrecuperable.

### 14.2. Modificadores propuestos

| Estado | Regeneración pasiva de stamina / turno | Modificador adicional propuesto |
|---|---:|---|
| **Sin estado principal** | **18 %** | Sin penalización |
| **Quemado** | **12 %** | Conserva el efecto normal de la quemadura sobre el combate; no duplicar su penalización de daño |
| **Paralizado** | **15 %** | El coste del **segundo movimiento encadenado** sube de 150 % a **160 % de su coste base**; conserva las restricciones normales de parálisis |
| **Congelado** | **6 %** | La recuperación vuelve al valor correspondiente al estado vigente al descongelarse; no consumir stamina por acciones que nunca se ejecutaron |
| **Envenenado** | **14 %** | Conserva el daño residual habitual |
| **Gravemente envenenado** | **12 %** | Penalización de stamina **constante**; no escalar también con el contador de daño tóxico |
| **Dormido** | **22 %** | Dormir puede favorecer la recuperación de stamina, pero no permite atacar mientras el estado impida actuar; vigilar sinergia con Descanso |
| **Confuso (volátil)** | **18 %** de base | Los movimientos **ejecutados** cuestan **10 % más de stamina base**; no cobrar un movimiento no ejecutado por autogolpe de confusión |

**Interpretación de costes:** el 160 % de la parálisis se refiere al **coste base del segundo ataque encadenado** (frente al 150 % habitual), **no** a multiplicar 1.50 × 1.10 ni a cobrar el coste dos veces. La confusión usa un recargo de **+10 puntos porcentuales del coste base** sobre el coste de esa acción, tras decidir si realmente se ejecuta. Si ambas reglas coinciden, limitar y documentar la suma; no multiplicar recargos en cascada.

**Ejemplo con 100 de stamina máxima:** recuperación básica 18; quemadura 12; parálisis 15; congelación 6; veneno 14; veneno grave 12; sueño 22. Son valores de diseño propuestos, no los efectos ya existentes en la ROM.

### 14.3. Reglas globales e integración

1. Los estados principales son excluyentes entre sí conforme al sistema Pokémon. No sumar penalizaciones de quemadura + parálisis + veneno sobre un mismo Pokémon.
2. Los estados **volátiles** (por ejemplo confusión) pueden coexistir con un estado principal, pero sus recargos deben seguir un orden de cálculo claro y tener límites razonables.
3. Aplicar el modificador de regeneración **una sola vez al cerrar cada turno**, con la fórmula y redondeo definidos en la sección 12. El estado considerado es el vigente en el momento de la regeneración.
4. **No regenerar entre los ataques de una cadena**, ni regenerar stamina extra por cada impacto de un movimiento multigolpe o del **Báculo Tormentoso**.
5. Cuando un Pokémon no llega a ejecutar la acción por parálisis, sueño, congelación o confusión, **no cobrar stamina del movimiento cancelado**. Mantener únicamente costes que correspondan a efectos realmente ejecutados según el motor.
6. Las mismas reglas rigen para **jugador, rivales y jefes**; La Cofradía puede infligir estados alterados, pero no obtiene inmunidad encubierta a estas penalizaciones.
7. Los efectos personalizados existentes (como Rooted, Confinado, Aterrado, etc.) quedan **sin nuevos modificadores específicos** hasta estudiarlos por separado; no asignar números arbitrarios sin definir antes su función y posibles solapamientos.
8. Las habilidades, objetos y Gears que previenen o curan estados deben actualizar inmediatamente el cálculo posterior; los que modifican la stamina requieren un orden de operaciones documentado.

### 14.4. Balance y QA pendiente

- Simular al menos 10 turnos con stamina máxima baja/media/alta y con cada estado; validar redondeo, coste de ataque, recuperación y ausencia de desbordamientos.
- Evaluar si quemadura y parálisis resultan demasiado fuertes al combinar penalización de stamina con sus efectos normales; ajustar números **antes de considerarlos definitivos**.
- Comprobar Descanso (sueño inducido), despertar, descongelación, curación entre turnos, cambios de estado durante una cadena y combates dobles.
- Para confusión, verificar que el recargo se aplica solo a movimientos efectivamente ejecutados y que las animaciones/sonidos no repiten el consumo.
- Probar con cadenas 2.0 (segundo ataque al 70 % del daño), Gears, mutaciones y jefes de Torre Celestial. Ningún estado debe permitir obtener stamina infinita o impedir toda recuperación sin posibilidad de respuesta.
- Mostrar el cambio de regeneración o coste en la interfaz de stamina si hay espacio, sin superponer el ring, barras, badges o textos existentes.

**Prioridad:** especificar y balancear primero; no programar este módulo ni modificar los efectos normales de los estados sin QA del encadenamiento 2.0 y la regeneración pasiva del 18 %.

---

## 15. Encadenado: control del encadenamiento mediante Gear o movimiento (diseño pendiente)

> **Propuesta:** incorporar una forma de impedir que otro Pokémon ejecute dos movimientos encadenados durante un período breve. Dos formas de aplicación a evaluar: Gear **Grillete** o movimiento de estado. Es una restricción al **encadenamiento**, no un estado de inmovilización total ni un aumento automático de daño. **Sin implementación confirmada.**

### 15.1. Estado volátil «Encadenado»

- **Efecto:** el Pokémon afectado **solo puede ejecutar un movimiento elegido por turno**; no puede seleccionar o completar una cadena de dos movimientos mientras dure el efecto.
- **Sigue pudiendo** atacar normalmente una vez, usar movimientos de estado, cambiarse (salvo otro efecto que lo impida), utilizar movimientos multigolpe y recibir la regeneración de stamina que corresponda.
- **No altera directamente** el daño, velocidad, coste ni recuperación de stamina: su ventaja es bloquear la opción de dos acciones, no «apagar» al objetivo.
- **No es un estado principal** como quemadura/parálisis; es un **estado volátil** y puede coexistir con estados principales, siempre según la lógica habitual de estados simultáneos.
- Si el objetivo había seleccionado dos ataques y queda Encadenado **antes de ejecutar el segundo**, ese segundo movimiento se **cancela sin atacar ni consumir sus PP/stamina**. Conservar únicamente los costes del movimiento que sí se ejecutó; no se recalcula o reinicia el turno.
- No confundir **Encadenado** con el «encadenamiento» ofensivo del usuario ni con **Confinado/Rooted** u otros efectos que restringen cambios, movilidad o acciones.
- Si el objetivo se retira del campo, **Encadenado se limpia**, salvo que más adelante se defina expresamente una mecánica de persistencia distinta.
- Si expira en mitad de un turno, **no se añade retroactivamente** una segunda acción que no se había seleccionado. La duración debe tener reglas visibles y consistentes.

### 15.2. Gear candidato: Grillete

**Función:** un Gear ofensivo de control que pueda aplicar Encadenado al enemigo mediante golpes, sin convertir cada impacto en una garantía de bloqueo.

**Parámetros iniciales a probar (no definitivos):**

- Probabilidad orientativa: **20 %** de aplicar **Encadenado** cuando el portador acierta un **movimiento que cause daño** sobre el objetivo.
- **Una tirada por movimiento**, no por cada impacto, ni por cada objetivo repetido del mismo movimiento; evitar explotación mediante movimientos multigolpe y el segundo impacto del **Báculo Tormentoso**.
- Duración sugerida para el Gear: **1 turno completo de restricción** tras la aplicación efectiva, con la segunda acción actual cancelada si todavía no se ejecutó. Definir exactamente en qué cierre de turno vence antes de programar.
- **No acumular cargas** ni prolongar indefinidamente al reaplicar cuando el estado ya está activo; limitar frecuencia y probar si hace falta un intervalo de resistencia a reaplicaciones.
- No confiere un bloqueo pasivo a todo el equipo rival ni anula automáticamente cadenas al entrar al campo.
- Respeta la regla de **un Gear equipado** por Pokémon; no se combina con Báculo Tormentoso o Psicosangre simultáneamente salvo que una mecánica futura habilite explícitamente múltiples slots.

### 15.3. Movimiento de estado candidato

Crear un **movimiento de estado** que aplique el mismo estado **Encadenado** como alternativa o complemento al Gear.

- **Efecto:** al acertar, impide encadenar movimientos durante un breve período.
- Valores provisionales de ensayo: **90 % de precisión** y **2 turnos de restricción**; PP, tipo, nombre definitivo, disponibilidad y animación pendientes.
- **Protección, inmunidades, Sustituto** y reglas especiales deben respetar la lógica de ataques de estado del motor; no permitir que atraviese barreras de manera implícita.
- No produce daño por sí mismo y **no puede ser el segundo movimiento de una cadena**, de acuerdo con la regla de la sección 12 que inicialmente excluye movimientos de estado de las cadenas.
- El jugador y la IA deben disponer de reglas equivalentes. La Cofradía puede usarlo, pero no obtiene versiones ocultamente superiores.

**Decisión abierta:** usar ambas vías de aplicación, solo una o reservar una para jefes; resolver tras balancear el impacto de impedir la segunda acción.

### 15.4. Compatibilidades y límites

- **Movimientos multigolpe:** continúan golpeando varias veces porque constituyen **una sola ejecución**, no dos movimientos encadenados.
- **Báculo Tormentoso:** la descarga especial extra del **25 %** sigue ocurriendo dentro del mismo movimiento. Encadenado **no** suprime esa descarga. Esto mantiene la distinción establecida en las secciones 11–13.
- **Resistencia, Banda Focus, Robustez y Cinta Focus:** sus nuevas reglas por turno no cambian; impedir la cadena no modifica la protección frente a impactos de multigolpe.
- **Stamina:** Encadenado no provoca pérdidas adicionales ni reduce automáticamente la regeneración del 18 %. Al impedir la segunda acción se evita su coste; no se debe consumir stamina por un ataque cancelado.
- **Mutaciones/habilidades/objetos:** revisar aquellos que otorgan movimientos extra o inmunidades, para no generar bucles ni evasiones de la prohibición. La prohibición afecta a **segundas acciones de cadena**, no a cualquier impacto extra de un mismo movimiento.
- **Interfaz:** un icono/indicador reconocible de **Encadenado** en el resumen de estado de combate; el selector de movimientos debe desactivar la elección de segundo ataque y mostrar un mensaje corto («No puede encadenar»).

### 15.5. Pruebas de aceptación

1. Pokémon Encadenado puede usar **un movimiento normal** y sigue regenerando stamina.
2. No puede confirmar dos movimientos de cadena mientras dure el estado.
3. Si se aplica el estado durante la primera ronda de acciones y existe una segunda acción ya pendiente, esta se cancela **sin gasto de PP ni stamina**.
4. Un movimiento de múltiples golpes y un ataque especial con segundo impacto del Báculo Tormentoso siguen resolviéndose como **un solo movimiento**.
5. Cambiar de Pokémon limpia el estado; finalizar duración lo elimina sin restaurar acciones ya perdidas.
6. Grillete tira su probabilidad **una sola vez por ejecución** de movimiento y no una vez por impacto.
7. Aplicación, duración, reintentos, curación y expiración funcionan con igual criterio para jugador, NPC y jefes de Torre Celestial.
8. La HUD y el selector de cadenas comunican claramente la restricción y no crean duplicados de turnos o ataques.

**Prioridad:** registrar el concepto, diseñar sprite/animación y ensayar el balance tras estabilizar Encadenamiento 2.0. Los porcentajes, duración y la coexistencia de las dos vías quedan abiertos a validación.

---

*Última actualización: 14 de noviembre de 2026. Incorporado diseño del estado Encadenado y candidatos Gear Grillete/movimiento de estado (sección 15), además de estados y stamina, encadenamiento 2.0, Cofradía, Torre Celestial y Báculo Tormentoso; apartados futuros pendientes de implementación y balance.*
