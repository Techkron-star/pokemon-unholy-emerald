# Genesis 2 — compositor y topología: piloto Ruta 102

Esta etapa introduce geometría nueva dentro de un dominio terrestre revisado.
La ROM materializa un plan espacial en C99 y coloca estructuras auténticas.
El piloto está habilitado en **Ruta 102**. Las otras 13 rutas conservan la
preparación inicial y el compositor de suelos de la etapa anterior.

No representa el generador completo de Hoenn. Los obstáculos originales,
ciudades, entradas, coordenadas de eventos, dimensiones y puzzles permanecen
nativos. Ahora pueden añadirse árboles completos que desvíen el recorrido en
las áreas permitidas; todavía no se sustituyen bosques, acantilados ni océanos.

## Assets comprobados

`genesis2_modules.py` extrae dos módulos primary-only de General:

| Módulo | Huella | Procedencia | Entradas nativas |
| --- | --- | --- | --- |
| Árbol completo | 2×2 | LavaridgeTown, (12,5) | 35D6,35D7 / 35E6,35E7 |
| Claro de hierba | 4×4 | Route102, (36,7) | 3001 ×16 |

El árbol tiene colisión1 y elevación3 en las cuatro celdas. Se verifica que su
arte usa únicamente tiles primarios <512 y paletas <6. La extracción fija SHA
para gráficos, metatiles, atributos, paleta y halo de origen. No toma edificios,
sprites, agua ni partes de copas que necesitan continuaciones. Las rocas aún
no cuentan con un módulo aprobado. Modificar un asset requiere nueva revisión.

El catálogo solo se coloca sobre suelo de hierba compatible; las zonas secas
con suelo de arena necesitan su propio arte. Los corredores conservan la arena
cuando corresponde. Un ancla de hierba alta evita eliminar todos los encuentros
de un mapa que originalmente los tenga.

## Dominio y plan

`topology_mask` está vinculado a los scripts revisados de Ruta102 y PetalburgCity.
El tutorial de Wally usa textos de Ruta102 pero transcurre en PetalburgCity.
Un cambio de script invalida la habilitación geométrica hasta revisar de nuevo.

Solo admite hierba normal/alta original, sin colisión y con elevación3, dentro
de la máscara editable. Conserva márgenes de conexiones, eventos, recorrido de
NPC y posibles finales de aproximación de entrenadores. Excluye un halo
Manhattan2 de todo comportamiento especial: saltos, agua y otras transiciones
no se convierten en aristas ordinarias.

El plan retenido contiene:

- Componentes de suelo, etiquetas de región y puertos hacia el exterior.
- Árboles de padres calculados por Dijkstra con heap indexado y costes positivos.
- Corredores entre todos los puertos y sus raíces, además de claros reservados.
- Materialización de terreno, posiciones de módulos y propietario de cada celda.
- Digest determinista sin direcciones de punteros ni padding.

Tres perfiles de costes dan preferencia a trayectos más directos, sinuosos o
por bandas. Son sesgos de este planificador local; no equivalen todavía a todas
las gramáticas de biomas de la especificación completa.

## Contrato de navegación

Los puertos se mantienen pasables y conectados dentro de cada componente.
También se comprueba que ningún vértice pasable restante quede aislado.
Un módulo nunca ocupa un puerto, corredor o ancla de encuentro.

La prueba no convierte agua, puentes, ledges, HM ni scripts en una máscara de
movimiento binaria. Sustituye solamente componentes interiores bidireccionales
de suelo ordinario, manteniendo su interfaz exterior. Bajo la protección
revisada, toda conexión dirigida o condicionada del exterior conserva la misma
alcanzabilidad: no se crean vértices pasables nuevos ni se pierden conexiones
entre puertos. Los fixtures prueban un exterior dirigido con cuatro estados de
capacidades; esto no es un solver general de todas las mecánicas de Emerald.

Si un módulo desconecta el dominio, se retiran módulos completos en orden
inverso y se valida de nuevo. Los reintentos están acotados por la cantidad de
módulos. Ante fallo del planificador se restaura el compositor base por celdas.
Una huella incompleta o corrupta termina en FAILED y nunca es publicable.

## Publicación y guardado

Cada módulo pasa una comprobación de huella completa. En entrada de mapa se
comprueban **todas** sus celdas vivas antes de publicarlo. Un delta de script
impide publicar el módulo entero. Tampoco se publica un árbol sobre la posición
del jugador ni sobre actores activos/guardados de ese mapa.

SavedMapView se preinspecciona antes de restaurar celdas. Si contiene un delta
sobre una parte de un árbol, se retira la estructura completa y se preserva
ese delta sin reponer los otros fragmentos. Hay reconciliación adicional tras
los scripts de carga. El modo Native filtra restos procedurales del saved view.

La geometría se prepara durante la introducción con Lotad y se reconstruye una
vez al continuar. No hay Dijkstra, BFS ni generación de módulos en cada cambio
de zona. El scheduler conserva cursores/colas/heap entre microtareas y mantiene
el presupuesto de unas 4 ms más una microtarea. Todo el pool vive en GBAX RAM;
SAVX y los timers del juego no se usan como scratch ni contador privado.

Identidad: generator `0x020100`, catálogo 2; el formato del catálogo de módulos
es 1. El registro de 16 bytes sigue en SaveBlock4, offset 1040. No hay migración
automática de los mundos `0x020000`: esta build se prueba con partida nueva.
No requiere modificar el núcleo ni volver a instalar el APK de la etapa anterior.

## Verificación reproducible

```
python3 tools/genesis/genesis2_modules_test.py
make -C tools/genesis genesis2-test genesis2-asan
make -C tools/genesis genesis2-topology-test genesis2-topology-asan
make GBAX=1 -j4 all
python3 tools/genesis/genesis2_arm_test.py --core ../mgba/build-libretro/mgba_libretro.so
python3 tools/genesis/genesis2_preview.py --output build/genesis2-topology-previews
```

Pruebas: 1.000 semillas × 14 mapas para determinismo/buffers; 1.000 semillas del
piloto con verificador independiente de componentes, puertos, padres, huellas
y cuatro estados de aristas dirigidas; fixtures sintéticos de heap, reparación,
fallo de memoria, huella incompleta y dimensiones desbordadas; 12 pruebas del
extractor. Los tests C corren con C99 estricto, ASan y UBSan.

El harness ejecuta la ROM real en CPU ARM emulada con dos semillas, recarga de
identidad, publicación, deltas de scripts, posición del jugador, SavedMapView,
modo Native y fallback sin GBAX. No sustituye una partida completa ni prueba
física de audio/tacto/1×/3× en Android. Las previews usan assets estáticos, sin
sprites ni animación del agua.

Pendiente: ampliar dominios revisados y variedad de módulos, sustituir geometría
original con equivalencia de progresión, más gramáticas/biomas y composición de
cuevas, costas y océanos. La generación de fauna por bioma es otra etapa.

Resultado del piloto: 100 previews con 82 mapas distintos en las semillas 0–99.
Las 1.000 semillas de navegación incluyen 142 árboles completos generados y validados en
el dominio permitido; otras semillas conservan el claro y los caminos si no se coloca un módulo compatible. No se fuerza un obstáculo para aumentar
una métrica de diversidad.

En el harness a 1×, el tramo de preparación se mantiene por debajo de 6 ms
(puerta de prueba), con muestras cercanas a 4,7 ms. La publicación se mantiene
también bajo 6 ms en las dos semillas medidas. Estos son ciclos de CPU emulada,
no una promesa de latencia física Android.
