# Raids repetibles y contratos de cazarrecompensas

Implementación del 21 de noviembre de 2026. Esta revisión sustituye el límite diario global de las raids.

## Raids (P0)

- El HF Poké Watch mantiene su acceso a partir de cinco medallas. Las cuatro dificultades se eligen manualmente desde ese momento. Los niveles, equipos, mutaciones y probabilidades de captura pertenecen a la dificultad elegida; ganar medallas no aumenta automáticamente la dificultad.
- Cada combinación de jefe y dificultad tiene **cinco minutos de cooldown**. Se aplica al entrar y se renueva tras cada combate, de modo que abandonar o perder no permite saltárselo. Otros jefes siguen disponibles.
- El panel muestra `DISPONIBLE` o `Espera mm:ss`, actualizado cada segundo. Arriba/abajo cambia de jefe; L/R cambia de dificultad. Izquierda/derecha sigue siendo un atajo de dificultad.
- Cada incursión conserva dos combates dobles y un jefe. El avance y la elección de premio siguen siendo los existentes.

| Progreso | Jefes por dificultad | Bonificación de recursos |
| --- | ---: | ---: |
| Cinco medallas | 12 | +20% |
| Seis medallas | 14 | +20% |
| Siete medallas | 16 | +20% |
| Ocho medallas | 18 | +28% |
| Liga completada | 20 | +35% |

Los bonos afectan a los recursos consumibles del botín, no multiplican artefactos ni alteran la captura. Las fracciones se conservan por dificultad y ranura de premio: cien recompensas de un recurso base con +20% entregan 120 unidades en total. El premio mostrado coincide con el cobrado. Si falta espacio, se revierten las entregas parciales y no se consume el saldo fraccionario.

Los cooldowns y los saldos se guardan en el bloque lógico 4 ya existente, dentro del `.sav` estándar de 128 KiB. Un ajuste del reloj hacia atrás conserva como máximo los cinco minutos pendientes. Las versiones desconocidas del registro se rechazan sin borrar datos. El antiguo `FLAG_DAILY_HFC_RAID` ya no bloquea el acceso.

## Bounty hunting

Trece vendedores junto a las entradas de los doce Poké Mart activos de Hoenn y los grandes almacenes de Calagua. Incluye el Mart del Frente Batalla; excluye el Mart sin uso de Calagua. Ninguno ocupa la puerta ni su casilla de salida.

Se compra **un contrato de entrenador a la vez**. La confirmación enseña ruta, nivel, precio y pago. Un sello ámbar marca la ruta en el mapa regional. Las ubicaciones iniciales son las rutas 102, 104, 110, 117 y 113; las rutas posteriores requieren visitar su ciudad vecina. La ruta 102 es el destino accesible de respaldo.

| Rango | Medallas | Nivel | Precio | Pago adicional | Botín |
| --- | ---: | ---: | ---: | ---: | --- |
| Bronce | 0 | 12 | 500 | 1,200–1,800 | 2 Pociones + 1 Parte Roja |
| Plata | 2 | 24 | 2,000 | 4,000–6,000 | 2 Superpociones + 2 Partes Azules |
| Oro | 4 | 36 | 6,000 | 12,000–18,000 | 2 Hiperpociones + 3 Partes Amarillas |
| Platino | 6 | 48 | 15,000 | 30,000–45,000 | 2 Restaurar Todo + 4 Partes Verdes |
| Legendario | 8 | 60 | 40,000 | 80,000–120,000 | 3 Caramelos Raros + 5 Partes Rojas |

- Combates normales contra entrenadores: no se capturan sus Pokémon. El pago del contrato se suma al premio habitual del combate.
- Perder conserva el contrato y permite reintentar sin volver a comprar. No se marca al entrenador como derrotado permanentemente.
- Ganar habilita el cobro una sola vez. Con la mochila llena, el objetivo y cualquier vendedor permiten recuperar el premio más tarde. No se pierde dinero ni botín por una entrega parcial.
- Hablar con un vendedor permite consultar la ruta o abandonar el contrato con confirmación, sin reembolso. Se puede comprar el siguiente inmediatamente después de cobrar.
- Contratos, importe y ruta sobreviven al guardado. No hay investigaciones, arrestos ni facciones.
- Interfaz nueva en español e inglés. En partidas previas, cambiar de mapa o usar `L → Actualizar mundo` incorpora los nuevos NPC.

## Verificación y alcance

- Pruebas de actividades: variedad, niveles independientes de medallas, bonos exactos, cooldowns independientes, retroceso del reloj, CRC/versiones de guardado, compra única, derrota/reintento, cobro único y reversión con mochila llena.
- Regresión del Watch y de los equipos de raid.
- `python3 tools/unholy/validate_bounties.py`: comprueba los trece vendedores y cinco puntos de objetivo contra las colisiones y eventos reales.
- Buffers grandes de las carpetas del PC trasladados a EWRAM. Dejaban IWRAM casi llena y reiniciaban los menús/pruebas. Los enlazadores de juego y pruebas ahora exigen margen para la pila.
- Compilación estricta de Emerald. Pendiente recorrer visualmente los trece vendedores y jugar el ciclo de contrato/raids en Pizza Boy.
