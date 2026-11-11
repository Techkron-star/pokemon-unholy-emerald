# Unholy Emerald — HUD circular de Stamina y evolución del Resumen

## Implementación actual (rama UI apilada sobre SaveBlock4/5)

El personaje aliado usa un medidor **circular de 32×32 píxeles (4bpp)** colocado en el espacio libre a la izquierda/abajo de su tarjeta de PS. No reutiliza un trozo de la barra de vida ni dibuja encima de los campos de IV/EV. El emulador verá un sprite OBJ auténtico con 16 tiles independientes, paleta propia y seguimiento del desplazamiento/visibilidad/orden de capas de la tarjeta.

- `STAMINA_REGEN_PERCENT = 20`: recarga **20 %** de Stamina máxima al final del turno (antes era **12 %**). **Advertencia de balance:** 20 % recupera MÁS que 12 %, aunque la petición verbal mencionara reducirla; se ha priorizado el porcentaje explícito.
- `src/stamina_ring.c`: rasterizador GBA eficiente sin trigonometría ni memoria dinámica; 64 segmentos de anillo, 3 píxeles de grosor, borde oscuro y centro transparente. Colores: verde (capacidad holgada), amarillo (intermedia) y rojo (baja/agotada). Al llegar a cero permanece visible el contorno oscuro.
- `src/battle_interface.c`: la barra de Stamina de jugador es ahora un sprite OAM de 32×32; la del rival continúa horizontal. Animación suave al gastar/recuperar Stamina; el cambio de Pokémon restablece inmediatamente el valor, evitando representar al dueño anterior. Sigue al healthbox al entrar, salir, abrir menús y desvanecer.
- `src/battle_gfx_sfx_util.c`: 512 bytes de tiles reservados para cada sprite circular aliado; los enemigos conservan su asignación de 256 bytes.
- `include/config/stamina.h`: `STAMINA_RING_X_OFFSET=-44`, `Y_OFFSET=12`. Son offsets ajustables tras capturas reales en Pizza Boy. 64 segmentos.
- Pruebas nativas de las fuentes reales (`tools/unholy/host_save_schema.sh`) y de GBA (`test/unholy_stamina_ring.c`) cubren porcentajes 0/20/50/100, saturación, render diferente y hueco central.

### Validaciones pendientes

- [ ] Compilar Emerald, FireRed y LeafGreen y release.
- [ ] Ejecutar el test nativo de rasterizado sin dependencias ARM.
- [ ] Probar visualmente en Pizza Boy Pro: batalla simple, doble, entrada de Pokémon, cambio, KO, caja oculta durante animaciones y en menú.
- [ ] Comprobar que el anillo no cubra el cuadro de diálogo, la Poké Ball, el indicador de Mega/Alfa/Omega, el sprite Pokémon ni la tarjeta de PS durante transiciones.
- [ ] Ajustar offsets por formato simple/doble si la vista real lo necesita.
- [ ] Si la paleta de OBJ está llena, establecer un respaldo probado; no presentar como verificado sin prueba.

## Primera reforma funcional del Resumen (implementada en esta rama)

- En `include/config/summary_screen.h` se habilitan los **valores numéricos de IV y EV** para cualquier Pokémon no huevo: `P_SUMMARY_SCREEN_IV_EV_INFO=TRUE`, `P_SUMMARY_SCREEN_IV_EV_TILESET=TRUE`, `P_SUMMARY_SCREEN_IV_EV_VALUES=TRUE`. El botón **A** de la página de atributos recorre **Estadísticas → IV → EV → Estadísticas**; se reutilizan los controles y ventanas originales, con los títulos del conjunto alternativo de tiles.
- El panel compacto superior de atributos deja de ser un interruptor «mostrar Stamina / ocultarla». **L/R** recorre cuatro contextos: **Stamina máxima**, **nombre del Gear**, **cantidad de mutaciones** y **EV totales**. La fórmula de Stamina coincide con combate, e IV/EV incluyen los valores efectivos de Omega y Ascendido.
- No altera `BoxPokemon`, variables persistidas, datos de combate, índices de guardado ni estadísticas actuales; solo cambia el frontend de Resumen. El Gear se valida contra `GEAR_COUNT` antes de acceder al catálogo y los textos largos usan ajuste de tipografía.
- El contador de mutaciones incluye el noveno bit (Ascendido), evitando el truncamiento a 8 bits ya corregido en la rama base.

**Lo que no se anuncia como terminado:** una página de *atributos ampliados* con navegación visual propia, la lista de modificadores y el historial por UID. Eso requiere construir las ventanas dedicadas y conectar el backend del PR #22, todavía experimental.

## Reforma planificada de la pantalla de estadísticas

No debemos añadir diez estadísticas al panel original sin espacio. La pantalla del GBA es **240×160**; debemos conservar la paginación lateral y separar **datos** de **renderizado**. Ya existe `UnholyMonSummary_Build()` para leer stats, IV/EV, Stamina, bonus de mutaciones y Gear sin alterar al Pokémon.

Arquitectura UI propuesta:
1. **Resumen**: nombre/nivel/especie, naturaleza, habilidad, objeto, iconos de mutación y Gear.
2. **Estadísticas**: PS, Ataque, Defensa, At. Especial, Def. Especial y Velocidad; barra/valor de Stamina máxima; potencialmente regeneración y estadísticas futuras vía catálogo de atributos.
3. **Genética**: IV 0–31, EV 0–252 por estadística y total; el Omega y Ascendido pueden superar 510 EV (1,512 en las seis).
4. **Mutaciones**: símbolo de la mutación principal y `+N` si hay varias; lista desplazable con los modificadores exactos, no solo una abreviación.
5. **Gears**: Gear equipado, fase cíclica (si está en combate), descripción y efecto; objeto equipado permanece independiente.
6. **Historial**: identificador persistente, medallas, raids y logros, **solo** cuando el backend UID esté operativo.

Plan de ejecución: extraer el proveedor de datos de `pokemon_summary_screen.c`, diseñar la navegación por páginas sin romper el Resumen clásico ni PC, crear ventanas de datos optimizadas, probar cambios de equipo/caja/huevo y habilitar metadatos extendidos solo cuando la asociación UID sea segura. Las páginas que dependen de UID no deben mostrar valores inventados.

**Estado real:** el HUD circular y la primera reforma de estadísticas están implementados en código (página de IV/EV y panel de detalles); queda pendiente compilar esta segunda revisión, evaluar el layout en Pizza Boy, y diseñar las páginas ampliadas completas.
