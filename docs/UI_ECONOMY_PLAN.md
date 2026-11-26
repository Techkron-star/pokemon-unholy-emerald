# Plan de mejora de las interfaces de economía

Alcance: cartera (wallet), Horizon, tienda Calagua, Pokélocator, libro de cuentas de la Cofradía y, por coherencia, Fabricación. Es un plan, no una implementación. Se basa en lectura de código y docs (rama `feat/unholy-cofradia-debt`); nada se ha probado en emulador.

## Estado actual

| Pantalla | Dónde vive | Problema principal |
|---|---|---|
| Wallet | `src/trainer_card.c` (`DrawWallet`, `STATE_WALLET`), `src/unholy_wallet.c` | Se dibuja con el aspecto original de la tarjeta, sin `unholy_ui`, sin temas Eclipse/Daylight; filas y coordenadas fijas; no se ve qué campo está activo; sin confirmación; no muestra deuda. |
| Horizon | `src/shop.c` (`HorizonDrawTabs`, `BuyMenuDrawHorizonGoods`), `src/horizon_*.c` | ITEMS y DISCS arrastran la ventana vertical del PokéMart; no hay paginación visible más allá de «n/m»; ayudas de teclas inconsistentes; riesgo de OAM saturado. |
| Calagua | `GetShopPurchasePrice` en `src/shop.c`, mapas Lilycove 2F–5F | No tiene interfaz propia: es el PokéMart vanilla con otros precios. Es el mayor hueco. |
| Pokélocator | `src/pokelocator.c` y archivos hermanos | Coordenadas mágicas, separadores manuales y ciclo de vida de iconos a mano; filtros (Señal, pisos) solo visibles en el menú Tools. |
| Cofradía | `src/unholy_cofradia.c`, `unholy_cofradia_vault.c` | Todo son cuadros de texto y multichoice; el libro de cuentas, el colateral y la tasación no tienen vista gráfica. |
| Fabricación | `src/start_menu.c` | Carga su propia paleta en el slot 14 y usa textos fijos en español, sin `UnholyLocalize`. |

## Bloques compartidos (fase 0)

Ya existen en `src/unholy_ui.c`: marco con cabecera, pie con teclas, tarjeta, fila de opción, barra, iconos de 8×8 y la paleta del slot 14. Se añaden, y se extraen de `shop.c`:

1. **Pestañas** (`UnholyUi_Tabs`): lo que hoy hace `HorizonDrawTabs` e `item_menu.c`, con L/R y la pestaña activa resaltada.
2. **Paginador** (`UnholyUi_Pager`): puntos o «n/m» junto a un indicador de scroll propio, en lugar de depender de los `ScrollArrows` del motor.
3. **Cabecera de saldo**: una franja con el efectivo (y la moneda que aplique a la pantalla), formateada con `UnholyWalletFormat`, reutilizable en tienda, cartera y libro de cuentas.
4. **Confirmación modal** con importe y botones A/B, para compras y transferencias grandes.
5. **Iconos nuevos** de 8×8: cartera, deuda y bóveda (hoy solo hay `COIN` y `LOCK`).
6. **`UnholyMenuSkin`** (propuesto en `UI_HORIZON_NEBULA.md` §5): un único sitio para colores, bordes y acentos, y mover `DrawUnholyUiPanelAccents` fuera de `shop.c`.

Criterio de salida: Horizon se ve igual que antes usando los bloques nuevos.

## Cartera

- Pasar de `trainer_card.c` a una pantalla propia con `UnholyUi_Frame`, con ambos temas.
- Cuatro filas (Efectivo, Reserva, Fichas de casino, Puntos de Batalla) como filas de opción con icono y cifra alineada a la derecha; el foco se ve con el relleno de selección.
- Panel inferior con selector Depositar/Retirar a la vista, campo de importe y chip del paso (1, 100, 10 k, 1 M, 100 M, 1 G).
- Barra de ocupación de la reserva frente a su tope.
- Pie fijo: `A:Confirmar  L/R:Paso  B:Atrás`. Errores en el pie, con el color de aviso.
- Fila «Deuda con la Cofradía», visible solo si hay deuda, alimentada por el libro de cuentas.
- Pendiente de decidir: a partir de qué importe se pide confirmación.

## Horizon

- Mismo esquema para las cinco pestañas: cuadrícula de tarjetas con cabecera de saldo, detalle a la derecha y pie único `A:Comprar  B:Salir  L/R:Pestaña`.
- Sustituir la ventana vertical del PokéMart en ITEMS y DISCS por la cuadrícula de 6 tarjetas, con paginador visible para las 12 ofertas V2.
- Precio con tres estados (asequible, caro, agotado) y marca cromática con icono, no solo con texto.
- Medir el uso de OAM por pestaña antes de añadir iconos; si se satura, los iconos de las tarjetas pasan a tiles de fondo.
- Unificar las ayudas de teclas que hoy mezclan `L/R:SHOP SEL:TABS` y variantes.

## Calagua

1. **Re-skin sin tocar la lógica:** marco, cabecera de saldo y pie con los bloques compartidos sobre las ventanas del PokéMart, con ambos temas.
2. **Directorio de plantas:** panel de ascensor con una fila por planta (1F recepción y lotería … 5F Mega y Z, según `BLACK_ECONOMY.md` P1) y nombre de planta en la cabecera de cada tienda.
3. **Categorías por planta** con las pestañas compartidas, si una planta tiene más de una.

## Pokélocator

- Sustituir las coordenadas mágicas por una tabla de constantes de diseño y mover los separadores a un helper de `unholy_ui`.
- Envolver adquisición y liberación de iconos y paletas en un solo par de funciones, para no repetir `DestroyIcons` a mano.
- Pestañas y paginador compartidos en el catálogo.
- Mostrar en la cabecera los filtros activos (Señal, piso) y la marca de objetivo registrado.
- Estados vacíos claros (sin tablas de Miel o Headbutt, sin resultados).
- Respetar el límite `STATIC_ASSERT` de tiles y probar en mGBA.

## Libro de cuentas de la Cofradía

Pantalla gráfica que sustituye al texto plano y a los multichoice, en tres tarjetas:

- **Cuenta:** saldo, deuda y techo de crédito (hoy provisional, 10× el mínimo) con una barra de uso.
- **Colateral:** cuadrícula de 8 Pokémon con icono y 24 pilas, con `LOCK` en lo comprometido y valor de tasación por tarjeta.
- **Apuesta:** importe con paso ×10, mínimo y máximo, y la financiación (efectivo, mixto, crédito) como tres chips.

## Fabricación

Migrar a `UnholyUi_AcquirePalette` y a los widgets compartidos, y pasar los textos por `UnholyLocalize`.

## Orden propuesto

| Fase | Contenido | Depende de |
|---|---|---|
| 0 | Bloques compartidos y `UnholyMenuSkin` | — |
| 1 | Cartera | 0 |
| 2 | Horizon unificado | 0 |
| 3 | Calagua (re-skin y directorio) | 0, 2 |
| 4 | Pokélocator | 0 |
| 5 | Libro de cuentas y colateral | 0, 1 |
| 6 | Fabricación | 0 |

## Comprobaciones por fase

- Capturas en mGBA a 240×160, con Eclipse y Daylight, en español e inglés.
- Presupuesto de tiles y de OAM por pantalla.
- Compilar Emerald y comprobar FRLG si se tocan archivos compartidos.
- Pruebas automáticas para el formateo de cifras y la navegación, sin tocar los tests ya fallidos de otras ramas.

## Decisiones abiertas

1. ¿Umbral de confirmación para transferencias y compras?
2. ¿Calagua con directorio de plantas desde el principio o primero solo el re-skin?
3. ¿Se acepta mover el aspecto de la cartera fuera de la tarjeta de entrenador?
4. Valores del crédito de la Cofradía: hoy son provisionales.
