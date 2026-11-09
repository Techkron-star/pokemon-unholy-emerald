# Unholy Emerald — Hoja de ruta de la Fase II

**Fecha:** 2026-11-09  
**Origen:** `feat/stamina` en `af672cfe9044a1974180e83398cfc2322eeb2f58`  
**Tipo:** plan técnico; no acredita implementación ni pruebas visuales  
**Rama de documentación:** `docs/unholy-phase2-roadmap`

## 0. Estado confirmado

- CI del commit de origen: compilación satisfactoria (GitHub Actions, run 42712271301).
- Horizon: pestañas Pokémon, Discos, Objetos, Huevos y Movimientos; Discos/Objetos abren compraventa real con stock de prueba; Movimientos abre recordador; Pokémon/Huevos aún informativos.
- Crafting: una receta real para Súper Repartir EXP, interfaz con requisitos y un sprite propio; el script sigue siendo la autoridad para validación y confirmación.
- Eclipse/Daylight: selección mediante `FLAG_UNHOLY_UI_DAYLIGHT` en Opciones; aún requiere prueba guardar/cargar y contraste visual.
- Omega: recurso y animación glint codificados; pendiente comprobar en emulador Omega normal, shiny normal, ambos y salida sin fugas.
- No fusionar `feat/stamina` con `master` por el solo hecho de compilar.

## 1. Regla de trabajo

Primero corregir cualquier regresión descubierta al probar la ROM. Cada bloque se trabaja en una rama corta nacida de `feat/stamina`; integrar solo después de compilar y comprobar casos críticos. Evitar disparar builds de Actions por cambios solo documentales. No inventar precios, tasas, catálogos, balance o efectos de Gears sin decisión de diseño.

## 2. Plan de entregas en orden recomendado

### II-01 — Estabilizar navegación y aspecto de menús

**Código principal:** `src/shop.c`, `src/start_menu.c`, `src/option_menu.c`.

- Recorrer 10 veces cada ruta A/B/L/R, volver de tienda, cancelar compra, abrir/cerrar crafting y gestión de movimientos.
- Auditar ventanas BG, tiles, OAM, sprites, paletas, tareas y callbacks; restaurar estado al salir y al fallar una asignación.
- Unificar `UnholyMenuSkin` para Eclipse/Daylight en lugar de duplicar paletas y estilos en cada pantalla.
- Evitar texto cortado y superposiciones a 240×160. Mostrar información completa mediante panel contextual, scroll o detalle.
- Confirmar que cambiar el tema no afecta RNG y sobrevive guardado/carga.

**Aceptación:** ningún menú negro, cursor bloqueado, sprites fantasmas, fuga de recursos o regreso al menú equivocado en la batería de QA.

### II-02 — Motor de oferta Horizon y persistencia

**Código esperado:** módulos nuevos `horizon_catalog`/`horizon_save` con interfaz usada por tienda, reloj y progreso.

- Tabla de artículos por tipo: especies normales, MT, objetos, huevos; requisitos de medallas y período de juego.
- Leer ciclo día/noche con las utilidades del proyecto (p. ej. `GetTimeOfDay`), independientemente de Eclipse/Daylight. Revisar `RtcGetLocalDayCount` y el reloj simulado antes de elegir la clave temporal.
- Generar una instantánea de oferta por período; guardar su identificador, productos y datos relevantes **en almacenamiento persistente auditado**. No emplear flags temporales ni sobrescribir variables ocupadas.
- Congelar el stock durante la sesión; abrir/cerrar, cancelar, guardar/cargar y reiniciar no regenera productos; respetar la definición de rotación una vez se apruebe.
- Definir de antemano la política cuando la fecha del reloj retrocede o el reloj no es válido: conservar última oferta hasta una transición válida, sin rerolls gratuitos.
- No consumir RNG de batalla por motivos estéticos ni por redibujar paneles.

**Aceptación:** misma oferta al reentrar y cargar; solo cambia cuando corresponde; sin corrupción de partida anterior.

### II-03 — Comprar Pokémon normales

- Crear vista por especie, nivel, precio, cupos, destino y confirmación; permitir inspeccionar antes de gastar.
- Reutilizar rutas de creación y entrega de Pokémon del motor, previa revisión de `CreateMon`, entrega a jugador y espacio PC.
- Filtrar por diseño a **todos los variantes** Alpha/Super/Enraged/Omega. No se muta un ejemplar por comprarlo.
- Operación transaccional: verificar elegibilidad, saldo y destino disponible; generar un ejemplar una sola vez; guardarlo una sola vez; descontar dinero solo si la entrega tuvo éxito.
- Sin hueco en equipo ni PC: mensaje comprensible, sin pérdida de dinero ni duplicación.

**Aceptación:** compras, cancelaciones, capacidad total y guardado/carga verificados; Pokémon vendido aparece exactamente una vez.

### II-04 — Comprar Huevos

- Catálogo separado de huevos misteriosos normales; especie oculta si se anuncia como misteriosa.
- Crear huevo mediante las rutinas existentes, con propiedades válidas de eclosión y origen; no otorgar Omega garantizado ni ventaja por defecto.
- Aplicar el mismo protocolo atómico de capacidad, entrega, cobro y cancelación que en Pokémon.
- Testear equipo lleno, cajas llenas, eclosión y progreso del contador después de guardar/cargar.

**Aceptación:** huevos que eclosionan correctamente, sin compras falsas ni Pokémon duplicados.

### II-05 — Discos, Objetos y Movimientos terminados

- Sustituir las listas fijas actuales por la instantánea de oferta aprobada; respetar medallas, día/noche y límites.
- Conservar el motor real de PokéMart para precios, bolsa y confirmación; no incluir Infinity Ball entre mercancías.
- En Movimientos, separar **Aprender** de **Olvidar**. Reusar recordador/borrador reales, validando restricciones HM, 4 movimientos y compatibilidad.
- Conservar pestaña/cursor al volver de un servicio; no descontar dinero por acciones canceladas.

**Aceptación:** cada botón hace una acción real o informa explícitamente de una restricción; no simular servicios.

### II-06 — Crafting escalable

- Convertir receta única en tabla de recetas (materiales consumibles, medallas, resultado, cantidades, permisos).
- Mantener como primera receta `EXP. SHARE ×1 + STONE BADGE` (la medalla no se consume) → `SUPER EXP. SHARE`.
- Mostrar ficha, sprite, materiales poseídos/requeridos y explicación; sin filas inventadas.
- Verificar bolsa, requisitos y confirmar antes de retirar materiales. Recuperar o impedir cambios parciales si no cabe el resultado.
- Al completar, actualizar inventario/estado en una misma sesión sin duplicar efectos.

**Aceptación:** cancelación no consume; éxito consume exactamente una unidad; errores no alteran bolsa.

### II-07 — Pulido artístico y glint Omega

- Mejorar jerarquía, cursor, contraste, marcos y sprites sin introducir imágenes fijas que suplanten estado real.
- Conservar el mecanismo shiny; usar para Omega únicamente estrellas rojas de seis puntas con borde negro.
- Validar cuatro casos: sin variantes, shiny, Omega no shiny, Omega shiny; revisar timing, capas, etiquetas de sprites, destrucción y OAM/paletas.
- Escalar arte solo si sigue legible a 240×160; usar pixel art nativo, no efectos WebGL.

**Aceptación:** ambas animaciones independientes funcionan sin bloquear batalla y las dos paletas son legibles.

## 3. Mecánicas posteriores a la tienda (no bloquear fase II)

### III — Sistemas de progresión

1. Repartir EXP Sobrecargado tras cuarta medalla; precisar antes la fórmula de EXP extra, EV/IV y contribución a eclosión.
2. Poké Simulator de cuarta medalla: interfaz por estadística, validación de límites, dinero y guardado, sin convertir Pokémon normal en Omega.
3. Pokélocator por ruta: especies, condiciones por medallas y mecanismo de aumentar probabilidad de variantes; definir el recurso/límite.
4. Mutator tras octava medalla: Alpha/Super/Enraged únicamente, nunca Omega; costo/probabilidad y pérdida máxima de 15 niveles, bloqueos de reroll hasta superar Super Liga.
5. Gears: un slot propio, potencia inicial grande, buff residual y recarga por **turnos**, no por cantidad de movimientos; dos ataques encadenados cuentan como un turno.

### IV — Reglas avanzadas y posjuego

- Corregir límites de stamina/encadenados (restricción de Protect, coste, IA simétrica, efectos por turno) con tests reproducibles.
- Añadir estados confirmados cuando sus efectos estén aprobados; no suponer efectos de nombres reservados.
- Super Liga solo tras campeón de Hoenn; encuentros difíciles con Omega y obtención de Infinity Ball como recompensa, no mercancía.
- Legendarios Omega posjuego y encuentro de Rayquaza con transición Mega al umbral diseñado.

## 4. Matriz mínima de QA

| Caso | Esperado |
| --- | --- |
| Entrar/salir Horizon 10 veces | Sin bloqueo, pantalla negra ni fugas |
| L/R + B en cada pestaña | Cursor y retorno correctos |
| Cambiar Eclipse/Daylight, guardar, reiniciar | Mismo tema |
| Cancelar compra | Dinero y stock intactos |
| Comprar con equipo y PC llenos | No cobrar ni entregar |
| Entrar/salir, guardar/cargar durante mismo período | Oferta idéntica |
| Cambiar de período aprobado | Oferta actualizada solo cuando corresponda |
| Fabricar sin material/medalla/espacio | No consumir |
| Fabricar y confirmar | Consumir solo EXP. SHARE y entregar una unidad |
| Shiny / Omega / combinación | Efectos correctos e independientes |
| Jugar combate con dos movimientos encadenados | Solo un avance de turnos, coste por movimiento |
| Comprar huevos y eclosionar | Datos correctos y no mutaciones prometidas |

### Fuentes internas de verdad

- `DISENO.md` — reglas y decisiones de jugabilidad.
- `docs/UI_HORIZON_NEBULA.md` — especificación estética y navegación.
- `src/shop.c`, `src/start_menu.c`, `src/option_menu.c`, `data/scripts/crafting.inc` — implementación real.
- `.github/workflows/rom.yml` — validación de compilación.

**Criterio de cierre:** compilar + QA en emulador + inspección del código + sin regresiones. CI verde por sí solo no significa interfaz validada.

## 5. Decisiones todavía abiertas (no inventar en implementación)

- Tabla de especies por medallas, nivel y precios de Pokémon.
- Rotación exacta por período, tamaño de oferta, pesos RNG y stock.
- Precio, rango y política de generación de huevos.
- Costos de aprender/borrar movimientos.
- Multiplicadores y cadencia de cada Gear, balance del Mutator y del Super Repartir EXP Sobrecargado.
- Mensajería final ES/EN y presencia de traducciones en la ROM (no asumir i18n sin sistema real).

Mientras no se aprueben estos valores, usar fixtures de prueba etiquetadas como tales y no anunciarlas como balance definitivo.
