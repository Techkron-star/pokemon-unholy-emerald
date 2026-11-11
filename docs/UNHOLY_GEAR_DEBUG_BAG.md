# Unholy Emerald — L/R Genética, Equipar Gear y Debug Mutaciones

## Ya implementado en esta rama

1. **Pantalla Atributos:** L/R cambia Estadísticas → IV → EV (el botón A conserva la misma alternativa). En la zona de valores que anteriormente mostraba EXP/Next Level ahora aparece **STA MAX**. La fila inferior informa regeneración (modo Stats), IV totales (modo IV) o EV totales (modo EV). *No se cambia el valor de experiencia real en la partida.* Las seis estadísticas en las otras ventanas muestran los valores del modo seleccionado; una mutación Omega/Ascendido refleja sus IV 31/EV 252 reales.
2. **Gears, botón EQUIP:** los seis Gears artesanales muestran **EQUIP** en vez de la acción normal GIVE del bolsillo clave. Se elige el Pokémon en la interfaz habitual de equipo; se escribe **MON_DATA_GEAR**, no MON_DATA_HELD_ITEM. El objeto Gear pasa de mochila a ranura Gear; al reemplazarlo, el Gear anterior vuelve a la mochila. Si no hay hueco para devolverlo, se cancela y el entrante se reintegra sin cambiar el Pokémon. No se equipan en huevos.
3. **Tres vistas en la mochila:** GE (Gears), ML (Correos) y MS (Misceláneos), disponibles en la cinta horizontal y en el selector rápido. Sus ItemSlots se crean en **EWRAM**, copiando los ItemSlots originales **sin desencriptar la cantidad**: la lectura normal de mochila hace la desencriptación exactamente una vez. **No son nuevas particiones físicas del guardado** ni duplican el número de objetos; el inventario real permanece en Key Items / Items. Los bolsillos originales *todavía muestran esos objetos*, por lo que GE/ML/MS son accesos filtrados adicionales, no una migración completa.
4. **Debug Give → Pokémon (Unholy):** ruta corta con especie, nivel, shiny Sí/No y **nueve toggles de mutación independientes** (Alpha, Super, Enraged, Omega, Hyper, Overloaded, Titan, Ancestral, Ascended). También se añadieron los nueve toggles al generador **Pokémon (Complex)** existente. Se guardan los 9 bits completos de `MON_DATA_SPECIAL_BADGES` antes de recalcular estadísticas y regalar el Pokémon. Las bonificaciones múltiples siguen siendo aditivas.

## Validaciones automatizadas

- `test/unholy_gear_equip.c`: objeto independiente del Gear, intercambio y restitución al inventario, conservación del objeto held.
- CI build Emerald, FireRed, LeafGreen, release, docs, y batería de ROM.
- Revisar cambios de `POCKETS_COUNT` en cualquier usuario adicional de los bolsillos (en especial SpriteAnim de mochila; se agregan animaciones seguras), y ausencia de cambios en `SaveBlock1`.

## QA en Pizza Boy Pro

- [ ] Abrir mochila, comprobar 10 pestañas y selector rápido sin textos superpuestos.
- [ ] Gear comprado/creado aparece en la vista GE; equipar en un Pokémon con y sin objeto held. Su objeto held no debe desaparecer.
- [ ] Sustituir Gear por otro; comprobar devolución del anterior y que no se duplique tras guardar/cargar.
- [ ] Abrir ML con un correo y MS con un objeto normal; cantidades correctas. Los bolsillos originales aún muestran los mismos objetos.
- [ ] Asegurarse de que vender/registrar/mover objetos originales no corrompa las vistas GE/ML/MS.
- [ ] En Resumen, L/R cambiar entre Stats/IV/EV y ver Stamina máxima en el espacio de EXP. Confirmar EV 1512 de Omega/Ascendido, sin cambios reales de experiencia.
- [ ] Debug Give → Pokémon (Unholy): elegir shiny y varias mutaciones a la vez; verificar los 9 símbolos, estadísticas y persistencia al guardar/cargar.
- [ ] Probar en dobles y PC, huevos, mochila vacía y alternar rápido 10 pestañas sin bloqueos.

La rama es experimental y apilada sobre el PR #23 y el backend #22. **No fusionar a main** sin ROM compilada, pruebas, QA visual y guardado en Pizza Boy.
