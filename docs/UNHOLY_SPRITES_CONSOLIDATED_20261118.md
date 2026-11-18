# Consolidación de sprites pendientes — 18/11/2026

Objetivo: reunir arte que ya existía en otras ramas de Unholy Emerald y
preservar la Armadura Meteórica de 32×32, sin tocar efectos de combate ni
renumerar los objetos.

## Sprites nativos recuperados

- **Ocho espadas:** Ame no Habakiri, Dragonslayer, Espada Carmesí, Espada
  Plateada, Excalibur, God Slayer, Muramasa y Necro Espada. Cada una conserva
  icono de Gear 16×32 (`graphics/gears/<nombre>/icon.png`), icono de mochila
  24×24 (`graphics/items/icons/<nombre>.png`) y sus paletas correspondientes.
- **Fragmento de Muramasa:** icono de mochila 24×24 y paleta.
- **Armadura Meteórica:** variante 16×32 y paleta recuperadas de la rama de
  arte; original de 32×32 aportado por Camilo más versión indexada 4bpp y
  paleta de prueba. Se preservan las dos propuestas.
- **Escudo de paladín y Báculo Tormentoso:** variantes de prueba 32×32 con
  sus paletas, rescatadas de ramas de arte distintas.

## Procedencia y límites

Los iconos de las espadas proceden de `art/unholy-five-gear-sprites`;
la Armadura Meteórica 16×32, de
`art/unholy-gear-sprite-armadura-meteorica-20261117`;
el Escudo de paladín, de
`art/unholy-gear-sprite-escudo-paladin-20261118`;
y la variante del Báculo, de `work/gear-sprites`.

Los lotes históricos (mutaciones, Gears anteriores, esferas y objetos
conceptuales) se compararon con las rutas existentes y **no se duplicaron**.
Se conservan aparte once referencias nuevas en
`docs/art_sources/unholy_2026-11-17/`.

**Alcance:** incorporación de assets. La mera presencia de PNG/`.pal` no
registra objetos ni sustituye iconos activos en `src/`; esas integraciones
requieren comprobar IDs, asociaciones y QA de ambas interfaces antes de
activar un sprite. Las referencias grandes no forman parte de la ROM.
