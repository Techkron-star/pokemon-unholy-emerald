# Unholy Emerald — arquitectura de guardados escalable (128 KiB / objetivo 1 MiB)

**Estado:** infraestructura lógica SaveBlock4/SaveBlock5 implementada en rama experimental; **el backend físico de 1 MiB NO está implementado ni certificado**. El usuario utiliza Pizza Boy Pro y prioriza expansión sin recortar contenido por compatibilidad con cartuchos originales. La afirmación de que la aplicación permite seleccionar «1 MB» requiere una prueba de direccionamiento/escritura real desde la ROM; un archivo de 1 MB, una opción de tamaño o un *save state* no constituyen por sí solos memoria Flash de 1 MB direccionable.

## Contrato del sistema

- Separar **modelo lógico** (datos por Pokémon, progreso de incursiones, inventario, coleccionables, eventos) del **backend de persistencia** (Flash GBA estándar o almacenamiento extendido).
- Nunca definir una nueva habilidad, estadística o mutación mediante «buscar dos bits libres» cuando la arquitectura extendida esté disponible. Los datos deben tener esquemas explícitos, rangos, versión y operaciones de lectura/escritura.
- No almacenar tablas de solo lectura en el guardado: especies, recetas, balance, probabilidades, íconos y equipos predeterminados de jefes viven en ROM. Guardar solamente datos que cambian.
- Mantener **identidad estable de Pokémon** al moverlos entre equipo, cajas, guardería y sistemas alternativos; un mapa por posición de caja no sirve para registrar estadísticas individuales.
- Nunca «actualizar» un guardado a un formato superior sin copia de seguridad y una migración explícita; no reescribir datos de versión desconocida.
- EWRAM = 256 KiB; un save extendido de 1 MiB debe cargarse por bloques/páginas y mantener solo cachés pequeñas.
- Mantener doble copia/journal y comprobación de integridad. Evitar que un cierre inesperado destruya la única partida.

## Etapa A: infraestructura realmente compatible con la ROM actual

La distribución actual de `save.h` contiene dos ranuras de 14 sectores de 4 KiB, más 4 sectores reservados; cada sector dispone de 3968 bytes del bloque principal, **116 bytes sidecar** de SaveBlock3 y 12 bytes de pie.

- `SaveBlock4` **lógico**: 1620 bytes dentro del área lateral de SaveBlock3; desplazamiento 4 después de `dexNavChain`. Guarda cabecera de magic/version/flags/CRC32 y 1608 bytes de payload. No requiere hardware extra. Configuraciones que agreguen otros campos al inicio de SaveBlock3 deben revisar su presupuesto: `STATIC_ASSERT` abortará la compilación ante desborde.
- `SaveBlock5` **lógico**: 1568 bytes anexados después de `struct PokemonStorage` previo (34144 bytes), dentro de su noveno sector. Guarda magic/version/flags/CRC32 y 1556 bytes de payload. La ubicación, el orden y el tamaño de BoxPokemon/PC y sus fusiones no cambian.
- Los accessors `UnholySave4_Get` y `UnholySave5_Get` inicializan áreas completamente vacías (todo `0x00` o `0xFF`). Con datos desconocidos, una versión posterior o un CRC inválido devuelven `NULL` sin sobrescribir nada. `Reset` es explícito. Después de cada modificación de payload/flags hay que llamar a `UnholySave4_Commit` o `UnholySave5_Commit` para actualizar el CRC32.
- Conservar las dimensiones previas de `SaveBlock1` y `SaveBlock2`; la ampliación intencionada de mochila anterior ya aumentó SaveBlock1 a 15784 bytes (vs. 15568 de la prueba original). Los tests de tamaño se actualizan para verificar el contrato de la rama, NO para proclamar retrocompatibilidad con saves más antiguos.
- **Advertencia:** los 116 bytes laterales de cada sector (SaveBlock3/4) no están en el checksum tradicional del sector. Ahora se comprueba su **CRC32 lógico**, que detecta corrupción, pero **todavía falta** reconciliación por generación y recuperación automática de la copia anterior: no es un contenedor certificado para progreso crítico.

**Capacidad disponible en esta etapa:** ~3,180 bytes adicionales de almacenamiento lógico (1620 + 1568), sin cambio de la ROM respecto al .sav de 128 KiB. No confundir esto con 1 MiB.

## Etapa B: backend experimental de Flash 1 MiB — requiere prueba en Pizza Boy Pro

**Requisito:** validar que el dispositivo de guardado exponga **1,048,576 bytes reales de datos** a instrucciones emitidas desde la ROM, *no* 1 MiB de archivo o de estado del emulador.

1. Generar una ROM **de diagnóstico aislada**, sin usar el save del jugador: guardar patrones distintos en 0x00000, 0x20000, 0x40000, 0x80000 y 0xF0000; apagar, cargar y volver a leer. Comprobar especialmente que direcciones altas no se replican/alias a los 128 KiB bajos. Esas escrituras son destructivas: exclusivamente en copia de la ROM y de .sav.
2. Leer e identificar qué periférico o protocolo extiende el guardado: Flash con bancos, SRAM cartucho personalizado u otra API admitida. No deducirlo de la extensión de archivo. Escribir controlador explícito con detección de tamaños real y protección ante límite.
3. Si el hardware simulado confirma 1 MiB direccionable, crear dos bancos de **512 KiB**, con encabezado, directorio de páginas, generación, versión, flags, CRC32 por sección, tabla de migración y bit de confirmación. Guardar en páginas de 4 KiB o menos; aplicar orden de escritura transaccional: contenido → índices/verificación → marca final de banco. El banco anterior permanece válido hasta finalizar.
4. Conmutar entre backend estándar y extendido **solo tras detección válida**, no cambiando una constante arbitrariamente. Nunca recortar un save extendido al fallback automáticamente: si no hay almacenamiento físico suficiente, avisar y cargar en modo seguro sin sobrescribir.
5. Probar desenchufe/cierre abrupto, sectores dañados, guardados v1/v2, cambios de versión, recuperación del banco anterior, box completo, inventario lleno y vuelta del modo extendido al estándar.

**Mapa lógico propuesto para una ranura extendida (512 KiB):**
| Dominio | Presupuesto inicial | Contenido mutable |
| --- | ---: | --- |
| Cabeceras/índice | 8 KiB | versión, checksums, directorio |
| Núcleo existente | 64 KiB | trainer, mapa, equipo, cajas, bolsa |
| Extensiones Pokémon | 128 KiB | stats nuevas, energía, mutaciones, Gear, slots de habilidad, historial |
| Incursiones y progresión | 64 KiB | raids completadas, desbloqueos, semillas, recompensas |
| Mundo/quests y NPC | 96 KiB | misiones, flags, reputación, bosses, eventos |
| Inventario extendido/crafting | 32 KiB | slots extra, recetas desbloqueadas, recursos |
| Journal y recuperaciones | 32 KiB | transacciones y cancelaciones |
| Reserva futura | 88 KiB | crecimiento sin reorganizar bloques |

En total: **512 KiB por ranura**, dos copias = 1 MiB físico. Estos son **presupuestos de diseño**, NO capacidades habilitadas ni confirmación de compatibilidad de Pizza Boy.

## Diseño de extensiones Pokémon y pantalla de estadísticas

**Primer contrato ya implementado:** `include/unholy_mon_ext.h` y `src/unholy_mon_ext.c` definen/validan un registro **variable** por Pokémon, con la identidad, el Gear, la habilidad oculta seleccionada y una tabla ordenada de campos `(dominio, clave, valor s32)`. Sus pruebas están en `test/unholy_mon_ext.c`.

| Dominio | Ejemplos | Cómo crece |
| --- | --- | --- |
| `UNHOLY_EXT_STAT` | stamina máxima, regeneración, crítico, futuras estadísticas | Añadiendo una nueva clave, sin ampliar `BoxPokemon` |
| `UNHOLY_EXT_MUTATION` | Alfa, Omega, Titan, mutaciones futuras | Una entrada por ID de mutación; no depende de 9/18/32 bits |
| `UNHOLY_EXT_PERMANENT` | entrenamiento, mejoras persistentes, penalizaciones | Entrada tipada con ID y valor de 32 bits |
| `UNHOLY_EXT_ACHIEVEMENT` | historial/logros por Pokémon | Nuevas claves sin rediseñar el resumen |
| Campos fijos de cabecera | `uidLow/uidHigh`, `gearId:u16`, `hiddenAbilitySlot`, `version`, `totalBytes` | Mantener la versión, migrar si se cambia la cabecera |

Los registros son autodelimitados (`totalBytes`), verifican alineación mediante `memcpy`, rechazan longitudes incorrectas, UIDs cero y claves repetidas o desordenadas; la lectura **no asigna memoria ni modifica el guardado**.

**Importante:** este es **un esquema de datos y un validador**, **no** el almacenamiento persistente por Pokémon terminado. Antes de conectar una nueva estadística a combate o a la pantalla de resumen, hace falta:
1. Asignar un **UID realmente estable** a cada Pokémon, que lo acompañe al moverlo entre equipo, PC, guardería, fusiones y transferencias; el OTID + personalidad por sí solo no garantiza unicidad.
2. Implementar un directorio/índice del backend extendido por UID con transacciones y CRC de página; no usar posición fija de caja como identidad.
3. Migrar en lectura los datos actuales de `MON_DATA_GEAR` y `MON_DATA_SPECIAL_BADGES`, sin descartar los bits antiguos antes de tener pruebas de transferencias, huevos, evolución y rival.
4. Añadir consultas `GetStat(mon, id)` para estadísticas derivadas. **La stamina actual de un turno no debe guardarse** salvo que exista un requisito de juego explícito.

**Pantalla de estadísticas futura**: pestañas General · Combate · Stamina · Mutaciones · Gears · Historial. Cada pestaña lee atributos derivados y sus extensiones por UID; sus páginas son **UI**, no campos nuevos obligatorios en el archivo de guardado. Nuevos atributos no fuerzan modificar el diseño de la pantalla ni reordenar estructuras GBA.

**Capa de datos del resumen ya implementada:** `UnholyMonSummary_Build(mon, out)` en `src/unholy_mon_summary.c` devuelve nivel, especie, estadísticas calculadas, IV, EV, porcentaje de mutación por estadística, Gear y **Stamina máxima**. La Stamina procede de `Stamina_CalcMaxForMon(mon)`, extraída de `src/battle_stamina.c`, de modo que la futura pantalla muestra exactamente la misma fórmula que combate y no genera reglas divergentes. Es **solo lectura**, no depende de `gBattleStruct` y no guarda Stamina actual fuera de combate. `test/unholy_mon_summary.c` cubre Sceptile Omega nivel 100 con 31 IV/252 EV en las seis estadísticas.

**Pendiente para el frontend de la pantalla:** diseño y renderizado real de pestañas en GBA; lectura de los registros adicionales por UID después de conectar el backend persistente. Esta capa es un proveedor de datos, no una pantalla ya dibujada.


## Fronteras de módulos futuras

API planificada (no programada todavía): `UnholySave_Open(profile)`, `UnholySave_Read(domain,uid,out)`, `UnholySave_Write(domain,uid,data)`, `UnholySave_Commit()`, `UnholySave_CheckIntegrity()`, `UnholySave_Migrate(old,new)`.

El ID de dominio desacopla la lógica del juego del tamaño/ubicación física; cada dominio lleva su propia versión. La UI de estadísticas nunca accede a offsets físicos. El guardado ejecuta la validación previa de tamaños y de espacio libre antes de confirmar una transacción.

## QA y bloqueos

- [ ] Compilar Emerald/FireRed/LeafGreen con los dos bloques lógicos anexados.
- [ ] Verificar tamaños exactos de ambos bloques y offsets originales.
- [ ] Validar guardado/carga de datos de prueba en SaveBlock4/5, incluidas versiones desconocidas y datos legados.
- [x] Añadir CRC32 para detectar corrupción en SaveBlock4 y SaveBlock5 (validación y pruebas nuevas).
- [ ] Implementar recuperación automática por ranura de datos laterales SaveBlock3 antes de guardar progreso esencial.
- [ ] Comprobar que la memoria de los bloques no desplaza más allá de 256 KiB EWRAM.
- [ ] Probar ROM de diagnóstico **independiente** en Pizza Boy Pro antes de modificar el backend Flash real.
- [ ] Emulador mGBA como control comparativo para detectar dependencias de un emulador específico.
- [ ] No fusionar la rama experimental hasta QA visual, save/load y fallos por cierre abrupto.

**Fuentes de implementación:** `include/save.h`, `src/save.c`, `src/load_save.c`, `include/global.h`, `include/pokemon_storage_system.h`, `include/pokemon.h` y `test/save.c`.
