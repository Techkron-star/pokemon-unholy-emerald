# Unholy Emerald · World Update V1

## Qué hace

Un guardado normal de Pokémon puede continuar tras instalar una ROM nueva.
Cuando se carga un mapa, `UnholyWorldUpdate_Reconcile()` ajusta los eventos
visibles según la versión persistida del mundo y el progreso de la historia.
Los nuevos mapas y NPC se leen de los datos de la ROM actual; **los flags de
objetos anteriores nunca se limpian**. No se altera la estructura de SaveBlocks.

- `VAR_UNHOLY_WORLD_VERSION` ocupa una variable de Hoenn sin uso (0x4068).
- Cada evento nuevo conserva su posición y su bandera; se añade después de
  los eventos existentes para no renumerar sus identificadores locales.
- Las nuevas Poké Balls tienen dos banderas distintas: `HIDE` (visibilidad)
  y `CLAIMED` (recogida irreversible). La segunda nunca se borra al actualizar.
- Dos rutas de carga (warp y transición lateral) reconcilian las banderas
  **antes** de cargar las plantillas de objetos del mapa.
- Las actualizaciones de progreso aparecen al volver a entrar al mapa; no es
  necesario iniciar una partida nueva.

## Contenido demostrativo de V1

| Desbloqueo | Mapa | Contenido nuevo |
| --- | --- | --- |
| Primera medalla | Ruta 102 (22,9) | Great Ball, recogida una sola vez |
| Segunda medalla | Ruta 102 (30,12) | Investigadora: intercambio de los 18 Inyectores por gema, Polvo Estelar y 3.000 pokédólares |
| Tercera medalla | Ruta 104 (27,7) | Ultra Ball, recogida una sola vez |
| Liga superada | Ruta 110 (15,8) | Veterano y diálogo posterior a la Liga |

Las Poké Balls tradicionales, incluidos los 12 Gears de rutas, mantienen sus
propias banderas y recompensas. Si la mochila está llena, las dos recompensas
nuevas permanecen disponibles para un intento posterior.

## Cómo extenderlo en futuras builds

1. Agregar objetos al final de `object_events` y asignarles banderas nuevas.
2. Implementar scripts que guarden `CLAIMED` **solo tras entregar** el objeto.
3. Registrar sus condiciones en `src/unholy_world_update.c`.
4. Incrementar `UNHOLY_WORLD_VERSION_LATEST` cuando haya migraciones nuevas,
   añadiéndolas en orden y sin reciclar banderas de otros eventos.
5. Comprobar coordenadas, elevación y accesos con el validador.

Los rediseños de la geometría de rutas usan normalmente los datos nuevos de la
ROM al recargar el mapa, pero **requieren revisión de posiciones guardadas** si
una actualización coloca árboles o paredes sobre la posición del jugador.
World Update V1 todavía no reubica automáticamente esos casos excepcionales.

## QA

```sh
python3 tools/unholy/validate_world_updates.py
python3 tools/validate_unholy_pickups.py
```

Prueba manual recomendada: guardar dentro del juego en la ROM anterior,
abrir la ROM nueva sin cargar *save states* antiguos, comprobar que los objetos
recogidos continúan ocultos, ganar cada medalla, visitar las tres rutas y
verificar que las nuevas recompensas no reaparecen después de guardado/carga.
La migración no descarga contenido por internet ni convierte estados rápidos
del emulador en guardados compatibles.
