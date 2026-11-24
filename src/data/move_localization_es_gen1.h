// Catálogo español: los 165 movimientos originales (GEN 1).
// Formato: [ID] = { nombre (<= MOVE_NAME_LENGTH), descripción GBA (máx. 2 líneas) }.
// En MOVE_NONE conservar el guion y la descripción vacía del original.
[MOVE_NONE] = { COMPOUND_STRING("-"), COMPOUND_STRING("") },
[MOVE_POUND] = { COMPOUND_STRING("Destructor"), COMPOUND_STRING("Golpea con las patas\no la cola.") },
[MOVE_KARATE_CHOP] = { COMPOUND_STRING("Golpe Kárate"), COMPOUND_STRING("Golpe con alta probabilidad\nde crítico.") },
[MOVE_DOUBLE_SLAP] = { COMPOUND_STRING("Doblebofetón"), COMPOUND_STRING("Abofetea de dos\na cinco veces.") },
[MOVE_COMET_PUNCH] = { COMPOUND_STRING("Puño Cometa"), COMPOUND_STRING("Golpea de dos\na cinco veces.") },
[MOVE_MEGA_PUNCH] = { COMPOUND_STRING("Megapuño"), COMPOUND_STRING("Propina un puñetazo\nde gran potencia.") },
[MOVE_PAY_DAY] = { COMPOUND_STRING("Día de Pago"), COMPOUND_STRING("Lanza monedas. Recoge\ndinero tras combatir.") },
[MOVE_FIRE_PUNCH] = { COMPOUND_STRING("Puño Fuego"), COMPOUND_STRING("Puñetazo ardiente que\npuede quemar.") },
[MOVE_ICE_PUNCH] = { COMPOUND_STRING("Puño Hielo"),
#if B_USE_FROSTBITE
    COMPOUND_STRING("Puñetazo helado que puede\ncausar quemadura por frío.") },
#else
    COMPOUND_STRING("Puñetazo helado que\npuede congelar.") },
#endif
[MOVE_THUNDER_PUNCH] = { COMPOUND_STRING("Puño Trueno"), COMPOUND_STRING("Puñetazo eléctrico que\npuede paralizar.") },
[MOVE_SCRATCH] = { COMPOUND_STRING("Arañazo"), COMPOUND_STRING("Araña al rival\ncon garras afiladas.") },
[MOVE_VISE_GRIP] = { COMPOUND_STRING("Agarre"), COMPOUND_STRING("Aprieta al rival\ncon grandes pinzas.") },
[MOVE_GUILLOTINE] = { COMPOUND_STRING("Guillotina"), COMPOUND_STRING("Ataque de pinzas que\npuede causar KO directo.") },
[MOVE_RAZOR_WIND] = { COMPOUND_STRING("Viento Cortante"),
#if B_UPDATED_MOVE_DATA == GEN_3 || B_UPDATED_MOVE_DATA == GEN_1
    COMPOUND_STRING("Carga el primer turno\ny ataca en el segundo.") },
#else
    COMPOUND_STRING("Ataca en dos turnos\ncon alto índice crítico.") },
#endif
[MOVE_SWORDS_DANCE] = { COMPOUND_STRING("Danza Espada"), COMPOUND_STRING("Aumenta mucho\nel Ataque del usuario.") },
[MOVE_CUT] = { COMPOUND_STRING("Corte"), COMPOUND_STRING("Corta al rival\ncon hojas o garras.") },
[MOVE_GUST] = { COMPOUND_STRING("Tornado"), COMPOUND_STRING("Azota al rival\ncon una ráfaga de viento.") },
[MOVE_WING_ATTACK] = { COMPOUND_STRING("Ataque Ala"), COMPOUND_STRING("Golpea al rival\ncon las alas extendidas.") },
[MOVE_WHIRLWIND] = { COMPOUND_STRING("Remolino"), COMPOUND_STRING("Expulsa al rival o acaba\ncon el combate salvaje.") },
[MOVE_FLY] = { COMPOUND_STRING("Vuelo"), COMPOUND_STRING("Vuela el primer turno\ny ataca en el siguiente.") },
[MOVE_BIND] = { COMPOUND_STRING("Atadura"), COMPOUND_STRING("Atrapa y aprieta\nal rival varios turnos.") },
[MOVE_SLAM] = { COMPOUND_STRING("Portazo"), COMPOUND_STRING("Golpea con una cola\no una liana larga.") },
[MOVE_VINE_WHIP] = { COMPOUND_STRING("Látigo Cepa"), COMPOUND_STRING("Azota al rival\ncon lianas delgadas.") },
[MOVE_STOMP] = { COMPOUND_STRING("Pisotón"), COMPOUND_STRING("Pisotea al rival.\nPuede hacerlo retroceder.") },
[MOVE_DOUBLE_KICK] = { COMPOUND_STRING("Doble Patada"), COMPOUND_STRING("Propina dos patadas\nseguidas al rival.") },
[MOVE_MEGA_KICK] = { COMPOUND_STRING("Megapatada"), COMPOUND_STRING("Patada muy fuerte\ny de gran potencia.") },
[MOVE_JUMP_KICK] = { COMPOUND_STRING("Patada Salto"), COMPOUND_STRING("Patada en salto. Si falla,\nse hiere el usuario.") },
[MOVE_ROLLING_KICK] = { COMPOUND_STRING("Patada Giro"), COMPOUND_STRING("Patada giratoria que\npuede hacer retroceder.") },
[MOVE_SAND_ATTACK] = { COMPOUND_STRING("Ataque Arena"), COMPOUND_STRING("Arroja arena y reduce\nla precisión del rival.") },
[MOVE_HEADBUTT] = { COMPOUND_STRING("Golpe Cabeza"), COMPOUND_STRING("Cabezazo que puede\nhacer retroceder.") },
[MOVE_HORN_ATTACK] = { COMPOUND_STRING("Cornada"), COMPOUND_STRING("Ataca al rival\ncon cuernos afilados.") },
[MOVE_FURY_ATTACK] = { COMPOUND_STRING("Ataque Furia"), COMPOUND_STRING("Pincha al rival de dos\na cinco veces.") },
[MOVE_HORN_DRILL] = { COMPOUND_STRING("Perforador"), COMPOUND_STRING("Perfora con un cuerno.\nPuede causar KO directo.") },
[MOVE_TACKLE] = { COMPOUND_STRING("Placaje"), COMPOUND_STRING("Arremete contra el rival\ncon todo el cuerpo.") },
[MOVE_BODY_SLAM] = { COMPOUND_STRING("Golpe Cuerpo"), COMPOUND_STRING("Golpea con el cuerpo.\nPuede paralizar.") },
[MOVE_WRAP] = { COMPOUND_STRING("Constricción"), COMPOUND_STRING("Enreda y aprieta\nal rival varios turnos.") },
[MOVE_TAKE_DOWN] = { COMPOUND_STRING("Derribo"), COMPOUND_STRING("Embestida que también\ndaña al usuario.") },
[MOVE_THRASH] = { COMPOUND_STRING("Golpe"), COMPOUND_STRING("Ataca 2 o 3 turnos\ny acaba confundido.") },
[MOVE_DOUBLE_EDGE] = { COMPOUND_STRING("Doble Filo"), COMPOUND_STRING("Placaje feroz que también\ndaña al usuario.") },
[MOVE_TAIL_WHIP] = { COMPOUND_STRING("Látigo"), COMPOUND_STRING("Mueve la cola para bajar\nla Defensa rival.") },
[MOVE_POISON_STING] = { COMPOUND_STRING("Picotazo Veneno"), COMPOUND_STRING("Ataca con púas que\npueden envenenar.") },
[MOVE_TWINEEDLE] = { COMPOUND_STRING("Dobleataque"), COMPOUND_STRING("Pincha dos veces.\nPuede envenenar.") },
[MOVE_PIN_MISSILE] = { COMPOUND_STRING("Pin Misil"), COMPOUND_STRING("Lanza púas de dos\na cinco veces.") },
[MOVE_LEER] = { COMPOUND_STRING("Malicioso"), COMPOUND_STRING("Intimida al rival y baja\nsu Defensa.") },
[MOVE_BITE] = { COMPOUND_STRING("Mordisco"), COMPOUND_STRING("Muerde al rival. Puede\nhacerlo retroceder.") },
[MOVE_GROWL] = { COMPOUND_STRING("Gruñido"), COMPOUND_STRING("Gruñe para reducir\nel Ataque de los rivales.") },
[MOVE_ROAR] = { COMPOUND_STRING("Rugido"), COMPOUND_STRING("Obliga al rival a salir\no acaba el combate salvaje.") },
[MOVE_SING] = { COMPOUND_STRING("Canto"), COMPOUND_STRING("Entona una melodía\nque duerme al rival.") },
[MOVE_SUPERSONIC] = { COMPOUND_STRING("Supersónico"), COMPOUND_STRING("Ondas sonoras que\nconfunden al rival.") },
[MOVE_SONIC_BOOM] = { COMPOUND_STRING("Bomba Sónica"), COMPOUND_STRING("Ondas que siempre\nrestan 20 PS.") },
[MOVE_DISABLE] = { COMPOUND_STRING("Anulación"),
#if B_DISABLE_TURNS >= GEN_5
    COMPOUND_STRING("Impide usar el último\nmovimiento por 4 turnos.") },
#elif B_DISABLE_TURNS == GEN_4
    COMPOUND_STRING("Anula el último ataque\ndurante 4 a 7 turnos.") },
#else
    COMPOUND_STRING("Anula el último ataque\ndurante 2 a 5 turnos.") },
#endif
[MOVE_ACID] = { COMPOUND_STRING("Ácido"),
#if B_UPDATED_MOVE_DATA >= GEN_4
    COMPOUND_STRING("Rocía ácido. Puede bajar\nla Defensa Especial.") },
#else
    COMPOUND_STRING("Rocía ácido. Puede bajar\nla Defensa del rival.") },
#endif
[MOVE_EMBER] = { COMPOUND_STRING("Ascuas"), COMPOUND_STRING("Ataque con fuego que\npuede causar quemaduras.") },
[MOVE_FLAMETHROWER] = { COMPOUND_STRING("Lanzallamas"), COMPOUND_STRING("Lanza llamas potentes.\nPuede quemar.") },
[MOVE_MIST] = { COMPOUND_STRING("Neblina"), COMPOUND_STRING("Evita que bajen las\nestadísticas de aliados.") },
[MOVE_WATER_GUN] = { COMPOUND_STRING("Pistola Agua"), COMPOUND_STRING("Dispara un chorro\nde agua al rival.") },
[MOVE_HYDRO_PUMP] = { COMPOUND_STRING("Hidrobomba"), COMPOUND_STRING("Ataca con un chorro\nde agua a gran presión.") },
[MOVE_SURF] = { COMPOUND_STRING("Surf"),
#if B_UPDATED_MOVE_DATA >= GEN_4
    COMPOUND_STRING("Una gran ola golpea\na todos los demás.") },
#else
    COMPOUND_STRING("Una gran ola golpea\na los rivales.") },
#endif
[MOVE_ICE_BEAM] = { COMPOUND_STRING("Rayo Hielo"),
#if B_USE_FROSTBITE
    COMPOUND_STRING("Rayo helado que puede\ncausar quemadura por frío.") },
#else
    COMPOUND_STRING("Rayo helado que puede\ncongelar al rival.") },
#endif
[MOVE_BLIZZARD] = { COMPOUND_STRING("Ventisca"),
#if B_USE_FROSTBITE
    COMPOUND_STRING("Tormenta helada que puede\ncausar quemadura por frío.") },
#else
    COMPOUND_STRING("Tormenta helada que\npuede congelar.") },
#endif
[MOVE_PSYBEAM] = { COMPOUND_STRING("Psico Rayo"), COMPOUND_STRING("Rayo psíquico que\npuede confundir.") },
[MOVE_BUBBLE_BEAM] = { COMPOUND_STRING("Rayo Burbuja"), COMPOUND_STRING("Lanza burbujas que\npueden bajar Velocidad.") },
[MOVE_AURORA_BEAM] = { COMPOUND_STRING("Rayo Aurora"), COMPOUND_STRING("Rayo multicolor que\npuede bajar el Ataque.") },
[MOVE_HYPER_BEAM] = { COMPOUND_STRING("Hiperrayo"),
#if B_SKIP_RECHARGE != GEN_1
    COMPOUND_STRING("Rayo muy potente. El usuario\ndebe descansar después.") },
#else
    COMPOUND_STRING("Rayo potente. Si no logra\nKO, pierde el turno siguiente.") },
#endif
[MOVE_PECK] = { COMPOUND_STRING("Picotazo"), COMPOUND_STRING("Ataca al rival\ncon el pico.") },
[MOVE_DRILL_PECK] = { COMPOUND_STRING("Pico Taladro"), COMPOUND_STRING("Taladra al rival\ncon el pico giratorio.") },
[MOVE_SUBMISSION] = { COMPOUND_STRING("Sumisión"), COMPOUND_STRING("Embestida que también\ndaña al usuario.") },
[MOVE_LOW_KICK] = { COMPOUND_STRING("Patada Baja"),
#if B_UPDATED_MOVE_DATA >= GEN_3
    COMPOUND_STRING("Patada que daña más\na enemigos más pesados.") },
#else
    COMPOUND_STRING("Patada baja que puede\nhacer retroceder.") },
#endif
[MOVE_COUNTER] = { COMPOUND_STRING("Contraataque"), COMPOUND_STRING("Devuelve el doble\ndel daño físico recibido.") },
[MOVE_SEISMIC_TOSS] = { COMPOUND_STRING("Mov. Sísmico"), COMPOUND_STRING("Causa daño igual al\nnivel del usuario.") },
[MOVE_STRENGTH] = { COMPOUND_STRING("Fuerza"), COMPOUND_STRING("Ataca al rival con\nun golpe poderoso.") },
[MOVE_ABSORB] = { COMPOUND_STRING("Absorber"), COMPOUND_STRING("Absorbe la mitad del daño\ncausado para curarse.") },
[MOVE_MEGA_DRAIN] = { COMPOUND_STRING("Megaagotar"), COMPOUND_STRING("Absorbe la mitad del daño\ncausado para curarse.") },
[MOVE_LEECH_SEED] = { COMPOUND_STRING("Drenadoras"), COMPOUND_STRING("Planta semillas que roban\nPS cada turno.") },
[MOVE_GROWTH] = { COMPOUND_STRING("Desarrollo"),
#if B_GROWTH_STAT_RAISE >= GEN_5
    COMPOUND_STRING("Aumenta Ataque y\nAtaque Especial.") },
#else
    COMPOUND_STRING("Aumenta el\nAtaque Especial.") },
#endif
[MOVE_RAZOR_LEAF] = { COMPOUND_STRING("Hoja Afilada"), COMPOUND_STRING("Corta con hojas. Alto\níndice de críticos.") },
[MOVE_SOLAR_BEAM] = { COMPOUND_STRING("Rayo Solar"), COMPOUND_STRING("Carga luz el primer turno\ny ataca el siguiente.") },
[MOVE_POISON_POWDER] = { COMPOUND_STRING("Polvo Veneno"), COMPOUND_STRING("Esparce polvo que\nenvenena al rival.") },
[MOVE_STUN_SPORE] = { COMPOUND_STRING("Paralizador"), COMPOUND_STRING("Esparce esporas que\nparalizan al rival.") },
[MOVE_SLEEP_POWDER] = { COMPOUND_STRING("Somnífero"), COMPOUND_STRING("Esparce polvo que\nduerme al rival.") },
[MOVE_PETAL_DANCE] = { COMPOUND_STRING("Danza Pétalo"), COMPOUND_STRING("Ataca 2 o 3 turnos\ny acaba confundido.") },
[MOVE_STRING_SHOT] = { COMPOUND_STRING("Disparo Demora"),
#if B_UPDATED_MOVE_DATA >= GEN_6
    COMPOUND_STRING("Lanza seda y reduce mucho\nla Velocidad enemiga.") },
#else
    COMPOUND_STRING("Lanza seda para bajar\nla Velocidad enemiga.") },
#endif
[MOVE_DRAGON_RAGE] = { COMPOUND_STRING("Furia Dragón"), COMPOUND_STRING("Ondas que siempre\nrestan 40 PS.") },
[MOVE_FIRE_SPIN] = { COMPOUND_STRING("Giro Fuego"), COMPOUND_STRING("Atrapa al rival en fuego\ndurante varios turnos.") },
[MOVE_THUNDER_SHOCK] = { COMPOUND_STRING("Impactrueno"), COMPOUND_STRING("Descarga eléctrica que\npuede paralizar.") },
[MOVE_THUNDERBOLT] = { COMPOUND_STRING("Rayo"), COMPOUND_STRING("Potente descarga eléctrica\nque puede paralizar.") },
[MOVE_THUNDER_WAVE] = { COMPOUND_STRING("Onda Trueno"), COMPOUND_STRING("Descarga eléctrica que\nparaliza al rival.") },
[MOVE_THUNDER] = { COMPOUND_STRING("Trueno"), COMPOUND_STRING("Lanza un rayo que\npuede paralizar.") },
[MOVE_ROCK_THROW] = { COMPOUND_STRING("Lanzarrocas"), COMPOUND_STRING("Lanza piedras pequeñas\ncontra el rival.") },
[MOVE_EARTHQUAKE] = { COMPOUND_STRING("Terremoto"), COMPOUND_STRING("Sacude la tierra y golpea\na todos los demás.") },
[MOVE_FISSURE] = { COMPOUND_STRING("Fisura"), COMPOUND_STRING("Abre una grieta que\npuede causar KO directo.") },
[MOVE_DIG] = { COMPOUND_STRING("Excavar"), COMPOUND_STRING("Se entierra el primer turno\ny ataca en el segundo.") },
[MOVE_TOXIC] = { COMPOUND_STRING("Tóxico"), COMPOUND_STRING("Envenena al rival y aumenta\nel daño cada turno.") },
[MOVE_CONFUSION] = { COMPOUND_STRING("Confusión"), COMPOUND_STRING("Ataque psíquico que\npuede confundir.") },
[MOVE_PSYCHIC] = { COMPOUND_STRING("Psíquico"), COMPOUND_STRING("Ataque mental que puede\nbajar la Defensa Especial.") },
[MOVE_HYPNOSIS] = { COMPOUND_STRING("Hipnosis"), COMPOUND_STRING("Hipnotiza al rival\npara dormirlo.") },
[MOVE_MEDITATE] = { COMPOUND_STRING("Meditación"), COMPOUND_STRING("Concentración que\naumenta el Ataque.") },
[MOVE_AGILITY] = { COMPOUND_STRING("Agilidad"), COMPOUND_STRING("Aumenta mucho la\nVelocidad del usuario.") },
[MOVE_QUICK_ATTACK] = { COMPOUND_STRING("Ataque Rápido"), COMPOUND_STRING("Golpe veloz con\nprioridad alta.") },
[MOVE_RAGE] = { COMPOUND_STRING("Furia"), COMPOUND_STRING("Sube el Ataque cuando\nrecibe daño.") },
[MOVE_TELEPORT] = { COMPOUND_STRING("Teletransporte"),
    COMPOUND_STRING("Pasa cambios de stats\nal Pokémon entrante.") },
[MOVE_NIGHT_SHADE] = { COMPOUND_STRING("Tinieblas"), COMPOUND_STRING("Causa daño igual al\nnivel del usuario.") },
[MOVE_MIMIC] = { COMPOUND_STRING("Mimético"), COMPOUND_STRING("Copia el último ataque\nrival durante el combate.") },
[MOVE_SCREECH] = { COMPOUND_STRING("Chirrido"), COMPOUND_STRING("Ruido agudo que reduce\nmucho la Defensa rival.") },
[MOVE_DOUBLE_TEAM] = { COMPOUND_STRING("Doble Equipo"), COMPOUND_STRING("Crea copias ilusorias\ny sube la evasión.") },
[MOVE_RECOVER] = { COMPOUND_STRING("Recuperación"), COMPOUND_STRING("Recupera hasta la mitad\nde sus PS máximos.") },
[MOVE_HARDEN] = { COMPOUND_STRING("Fortaleza"), COMPOUND_STRING("Endurece el cuerpo\ny aumenta la Defensa.") },
[MOVE_MINIMIZE] = { COMPOUND_STRING("Reducción"),
#if B_MINIMIZE_EVASION >= GEN_5
    COMPOUND_STRING("Se encoge y aumenta mucho\nla evasión.") },
#else
    COMPOUND_STRING("Se encoge y aumenta\nla evasión.") },
#endif
[MOVE_SMOKESCREEN] = { COMPOUND_STRING("Pantallahumo"), COMPOUND_STRING("Cubre de humo al rival\ny reduce su precisión.") },
[MOVE_CONFUSE_RAY] = { COMPOUND_STRING("Rayo Confuso"), COMPOUND_STRING("Rayo siniestro que\nconfunde al rival.") },
[MOVE_WITHDRAW] = { COMPOUND_STRING("Refugio"), COMPOUND_STRING("Se oculta en el caparazón\npara aumentar su Defensa.") },
[MOVE_DEFENSE_CURL] = { COMPOUND_STRING("Rizo Defensa"), COMPOUND_STRING("Se encoge y aumenta\nsu Defensa.") },
[MOVE_BARRIER] = { COMPOUND_STRING("Barrera"), COMPOUND_STRING("Crea una barrera que\naumenta mucho la Defensa.") },
[MOVE_LIGHT_SCREEN] = { COMPOUND_STRING("Pantalla Luz"), COMPOUND_STRING("Reduce daño especial\ndurante cinco turnos.") },
[MOVE_HAZE] = { COMPOUND_STRING("Niebla"), COMPOUND_STRING("Elimina los cambios\nde estadísticas de todos.") },
[MOVE_REFLECT] = { COMPOUND_STRING("Reflejo"), COMPOUND_STRING("Reduce daño físico\ndurante cinco turnos.") },
[MOVE_FOCUS_ENERGY] = { COMPOUND_STRING("Foco Energía"), COMPOUND_STRING("Aumenta la probabilidad\nde golpes críticos.") },
[MOVE_BIDE] = { COMPOUND_STRING("Venganza"), COMPOUND_STRING("Recibe golpes dos turnos\ny devuelve doble daño.") },
[MOVE_METRONOME] = { COMPOUND_STRING("Metrónomo"), COMPOUND_STRING("Usa al azar cualquier\nmovimiento Pokémon.") },
[MOVE_MIRROR_MOVE] = { COMPOUND_STRING("Mov. Espejo"), COMPOUND_STRING("Repite el último ataque\nusado por el rival.") },
[MOVE_SELF_DESTRUCT] = { COMPOUND_STRING("Autodestrucción"), COMPOUND_STRING("Causa gran daño, pero\ndebilita al usuario.") },
[MOVE_EGG_BOMB] = { COMPOUND_STRING("Bomba Huevo"), COMPOUND_STRING("Arroja un huevo\ncontra el rival.") },
[MOVE_LICK] = { COMPOUND_STRING("Lengüetazo"), COMPOUND_STRING("Lame al rival.\nPuede paralizarlo.") },
[MOVE_SMOG] = { COMPOUND_STRING("Polución"), COMPOUND_STRING("Ataca con gases que\npueden envenenar.") },
[MOVE_SLUDGE] = { COMPOUND_STRING("Residuos"), COMPOUND_STRING("Arroja lodo tóxico.\nPuede envenenar.") },
[MOVE_BONE_CLUB] = { COMPOUND_STRING("Hueso Palo"), COMPOUND_STRING("Golpea con un hueso.\nPuede hacer retroceder.") },
[MOVE_FIRE_BLAST] = { COMPOUND_STRING("Llamarada"), COMPOUND_STRING("Lanza una llamarada\nque puede quemar.") },
[MOVE_WATERFALL] = { COMPOUND_STRING("Cascada"),
#if B_UPDATED_MOVE_DATA >= GEN_4
    COMPOUND_STRING("Embestida acuática que\npuede hacer retroceder.") },
#else
    COMPOUND_STRING("Embestida acuática\ncontra el rival.") },
#endif
[MOVE_CLAMP] = { COMPOUND_STRING("Tenaza"), COMPOUND_STRING("Aprieta al rival con\npinzas varios turnos.") },
[MOVE_SWIFT] = { COMPOUND_STRING("Rapidez"), COMPOUND_STRING("Lanza estrellas que\nnunca fallan.") },
[MOVE_SKULL_BASH] = { COMPOUND_STRING("Cabezazo"),
#if B_UPDATED_MOVE_DATA >= GEN_2
    COMPOUND_STRING("Sube Defensa al cargar\ny golpea al turno siguiente.") },
#else
    COMPOUND_STRING("Carga un turno y\ngolpea al siguiente.") },
#endif
[MOVE_SPIKE_CANNON] = { COMPOUND_STRING("Clavo Cañón"), COMPOUND_STRING("Dispara púas entre dos\ny cinco veces.") },
[MOVE_CONSTRICT] = { COMPOUND_STRING("Restricción"), COMPOUND_STRING("Aprieta al rival. Puede\nreducir su Velocidad.") },
[MOVE_AMNESIA] = { COMPOUND_STRING("Amnesia"), COMPOUND_STRING("Aumenta mucho\nsu Defensa Especial.") },
[MOVE_KINESIS] = { COMPOUND_STRING("Kinético"), COMPOUND_STRING("Distrae al rival\ny baja su precisión.") },
[MOVE_SOFT_BOILED] = { COMPOUND_STRING("Amortiguador"), COMPOUND_STRING("Recupera hasta la mitad\nde sus PS máximos.") },
[MOVE_HIGH_JUMP_KICK] = { COMPOUND_STRING("Pat. Salto Alta"), COMPOUND_STRING("Patada en salto. Si falla,\nse hiere el usuario.") },
[MOVE_GLARE] = { COMPOUND_STRING("Deslumbrar"), COMPOUND_STRING("Intimida al rival\ny lo paraliza.") },
[MOVE_DREAM_EATER] = { COMPOUND_STRING("Comesueños"), COMPOUND_STRING("Roba PS a un rival dormido:\nla mitad del daño causado.") },
[MOVE_POISON_GAS] = { COMPOUND_STRING("Gas Venenoso"),
#if B_UPDATED_MOVE_DATA >= GEN_5
    COMPOUND_STRING("Gas que envenena\na los rivales.") },
#else
    COMPOUND_STRING("Gas que envenena\nal rival.") },
#endif
[MOVE_BARRAGE] = { COMPOUND_STRING("Bombardeo"), COMPOUND_STRING("Lanza objetos redondos\nde dos a cinco veces.") },
[MOVE_LEECH_LIFE] = { COMPOUND_STRING("Chupavidas"), COMPOUND_STRING("Recupera PS equivalentes\na la mitad del daño.") },
[MOVE_LOVELY_KISS] = { COMPOUND_STRING("Beso Amoroso"), COMPOUND_STRING("Besa al rival\ny lo duerme.") },
[MOVE_SKY_ATTACK] = { COMPOUND_STRING("Ataque Aéreo"),
#if B_UPDATED_MOVE_DATA >= GEN_3
    COMPOUND_STRING("Carga 1 turno. Puede dar\ncrítico y hacer retroceder.") },
#else
    COMPOUND_STRING("Busca puntos débiles y\nataca al turno siguiente.") },
#endif
[MOVE_TRANSFORM] = { COMPOUND_STRING("Transformación"), COMPOUND_STRING("Copia la apariencia y\naptitudes del rival.") },
[MOVE_BUBBLE] = { COMPOUND_STRING("Burbuja"), COMPOUND_STRING("Lanza burbujas que pueden\nreducir la Velocidad.") },
[MOVE_DIZZY_PUNCH] = { COMPOUND_STRING("Puño Mareo"),
#if B_UPDATED_MOVE_DATA >= GEN_2
    COMPOUND_STRING("Puñetazo rítmico que\npuede confundir al rival.") },
#else
    COMPOUND_STRING("Golpea con un\npuñetazo rítmico.") },
#endif
[MOVE_SPORE] = { COMPOUND_STRING("Espora"), COMPOUND_STRING("Esporas que siempre\nduermen al rival.") },
[MOVE_FLASH] = { COMPOUND_STRING("Destello"), COMPOUND_STRING("Emite un fogonazo que\nreduce la precisión.") },
[MOVE_PSYWAVE] = { COMPOUND_STRING("Psicoonda"), COMPOUND_STRING("Ataca con una onda\npsíquica de daño variable.") },
[MOVE_SPLASH] = { COMPOUND_STRING("Salpicadura"), COMPOUND_STRING("Solo salpica.\nNo tiene ningún efecto.") },
[MOVE_ACID_ARMOR] = { COMPOUND_STRING("Armadura Ácida"), COMPOUND_STRING("Licúa su cuerpo y\naumenta mucho su Defensa.") },
[MOVE_CRABHAMMER] = { COMPOUND_STRING("Martillazo"), COMPOUND_STRING("Golpe de pinza con alta\nprobabilidad de crítico.") },
[MOVE_EXPLOSION] = { COMPOUND_STRING("Explosión"), COMPOUND_STRING("Causa gran daño, pero\ndebilita al usuario.") },
[MOVE_FURY_SWIPES] = { COMPOUND_STRING("Golpes Furia"), COMPOUND_STRING("Araña de dos a cinco\nveces con garras.") },
[MOVE_BONEMERANG] = { COMPOUND_STRING("Huesomerang"), COMPOUND_STRING("Lanza un hueso que\ngolpea dos veces.") },
[MOVE_REST] = { COMPOUND_STRING("Descanso"), COMPOUND_STRING("Duerme dos turnos y cura\nsus PS y estados alterados.") },
[MOVE_ROCK_SLIDE] = { COMPOUND_STRING("Avalancha"),
#if B_UPDATED_MOVE_DATA >= GEN_2
    COMPOUND_STRING("Lanza grandes rocas.\nPuede hacer retroceder.") },
#else
    COMPOUND_STRING("Una avalancha de rocas\ngolpea a los rivales.") },
#endif
[MOVE_HYPER_FANG] = { COMPOUND_STRING("Hipercolmillo"), COMPOUND_STRING("Muerde con fuerza.\nPuede hacer retroceder.") },
[MOVE_SHARPEN] = { COMPOUND_STRING("Afilar"), COMPOUND_STRING("Reduce sus polígonos\ny aumenta su Ataque.") },
[MOVE_CONVERSION] = { COMPOUND_STRING("Conversión"),
#if B_UPDATED_CONVERSION >= GEN_6
    COMPOUND_STRING("Adopta el tipo del primer\nmovimiento que conoce.") },
#else
    COMPOUND_STRING("Adopta el tipo de uno\nde sus movimientos.") },
#endif
[MOVE_TRI_ATTACK] = { COMPOUND_STRING("Triataque"),
#if B_UPDATED_MOVE_DATA >= GEN_2
#if B_USE_FROSTBITE
    COMPOUND_STRING("Puede quemar, paralizar\no causar quemadura por frío.") },
#else
    COMPOUND_STRING("Triple rayo. Puede quemar,\nparalizar o congelar.") },
#endif
#else
    COMPOUND_STRING("Dispara tres rayos\nde energía a la vez.") },
#endif
[MOVE_SUPER_FANG] = { COMPOUND_STRING("Superdiente"), COMPOUND_STRING("Mordisco que reduce\na la mitad los PS rivales.") },
[MOVE_SLASH] = { COMPOUND_STRING("Cuchillada"), COMPOUND_STRING("Corta al rival. Alta\nprobabilidad de crítico.") },
[MOVE_SUBSTITUTE] = { COMPOUND_STRING("Sustituto"), COMPOUND_STRING("Gasta 1/4 de los PS máximos\npara crear un señuelo.") },
[MOVE_STRUGGLE] = { COMPOUND_STRING("Combate"), COMPOUND_STRING("Sin PP, ataca a la fuerza\ny también sufre daño.") },
