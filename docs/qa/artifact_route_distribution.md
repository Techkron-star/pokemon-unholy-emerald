# Recompensas de exploración · Unholy Emerald

Los artefactos se obtienen en la ranura Gear, separada del objeto equipado.
Estas Poké Balls son objetos nuevos: no sustituyen las recompensas originales
y cada una tiene su propia bandera persistente.

| Mapa | Artefacto | Casilla (x,y) | Estado |
| --- | --- | --- | --- |
| Ruta 110 | Lente de Certeza | 34,65 | Poké Ball |
| Ruta 111, zona del desierto | Piedra de Afilar | 16,58 | Poké Ball |
| Ruta 115, sector norte | Guantes Cohete | 22,11 | Poké Ball |
| Ruta 117 | Batería de Reserva | 9,17 | Poké Ball |
| Ruta 119 | Botón de Pánico | 12,121 | Poké Ball anterior |
| Ruta 120 | Casco Hoplita | 20,55 | Poké Ball anterior |
| Ruta 121 | Botas del Umbral | 46,13 | Poké Ball |
| Ruta 123 | Túnica Rúnica | 57,16 | Poké Ball anterior |
| Monte Pírico 4F | Velo Críptico | 8,12 | Poké Ball |
| Cueva Cardumen, sala de hielo | Fragmento de Permafrost | 8,4 | Poké Ball |
| Ruta 127 submarina | Ariete Náutilo | 59,12 | Poké Ball |
| Calle Victoria B1F | Nihil Engine | 35,10 | Poké Ball |

Las nueve nuevas colocaciones usan IDs de objetos y efectos existentes; no
requieren avanzar el desarrollo de los sistemas de jefes o raids. Los scripts,
banderas, colisiones y accesos se verifican con:

```sh
python tools/validate_unholy_pickups.py
```

## Auditoría de todas las Poké Balls visibles

El validador también recorre los mapas de Emerald y FRLG: hay **409 objetos**
con sprite de Poké Ball. De ellos, **171** son recogidas nativas con
`Common_EventScript_FindItem` y **238** utilizan scripts propios (regalos,
encuentros especiales y Battle Pyramid incluidos). Comprueba que ninguno
carezca de script y que las recogidas nativas tengan un ID de objeto y una
bandera definidos y no repetidos, posición en el mapa, elevación correcta y
una casilla adyacente transitable. Las 12 ubicaciones de Gears se comprueban
además por coordenadas, colisión y recompensa.

**Regresión reportada:** la Poción de Ruta 102 (11,15) tenía una Poké Ball
visible que no respondía a A. Ahora queda fijada como caso de prueba explícito.

En `field_control_avatar.c`, la pulsación A intenta resolver primero la Poké
Ball visible **en la casilla frontal** usando el objeto activo y el script de
su plantilla. Esto evita descartarla por una diferencia inesperada de
elevación en tiempo de ejecución o por el detector `Script_HasNoEffect`.
La ruta conserva scripts especiales y sustituciones temporales de scripts;
no permite recoger objetos de otros pisos ni vuelve a ejecutar recompensas
que ya tengan su bandera de recogida. Si no hay Poké Ball, el flujo habitual
de NPC, señales y puertas permanece sin cambios.

La auditoría de archivos no sustituye estas pruebas en mGBA/Pizza Boy:

1. **Ruta 102, Poción (11,15):** acercarse desde el norte, pulsar A, recibir
   la Poción, guardar, recargar y verificar que no reaparece.
2. Una Poké Ball de evento (Voltorb/Foongus o regalo): pulsar A y comprobar que
   inicia la interacción especial y respeta su bandera de finalización.
3. Una Poké Ball de Gear: comprobar artículo recibido, mochila llena y guardado.
4. Interactuar frente a un objeto distinto: confirmar que no se dispara ningún
   evento de Poké Ball ni se altera la interacción de carteles o NPC.

**Estado:** verificaciones estáticas superadas; pruebas de emulador pendientes.

## Próximas recompensas, aún no colocadas

- **Rompeolas:** Jefe Salvaje opcional en la Ruta 126, antes del gimnasio de
  Agua. Primero requiere su identificador, efecto, icono y evento de jefe.
- **Escudo de Aegis:** Jefe Salvaje opcional en la planta inferior de Calle
  Victoria. Se obtiene al derrotarlo, no al capturarlo.
- **MASKARA:** familia para el casino, desbloqueada progresivamente; no se
  dispersa mediante Poké Balls convencionales.

En los Jefes Salvajes, capturar concede únicamente al Pokémon con su mutación
natural, sin sus bonificaciones temporales ni el artefacto. Derrotar concede
el artefacto. Tras completar la Superliga, los jefes reaparecen para permitir
intentos adicionales. No se implanta esta mecánica mediante scripts de
recogida ordinarios.
