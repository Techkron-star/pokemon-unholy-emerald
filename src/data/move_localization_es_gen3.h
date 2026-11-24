// Traducción de los 103 movimientos añadidos en la tercera generación.
[MOVE_FAKE_OUT] = { COMPOUND_STRING("Sorpresa"), COMPOUND_STRING("Solo al entrar. Golpea\nprimero y hace retroceder.") },
[MOVE_UPROAR] = { COMPOUND_STRING("Alboroto"), COMPOUND_STRING("Hace ruido varios turnos\ny evita que se duerman.") },
[MOVE_STOCKPILE] = { COMPOUND_STRING("Reserva"),
#if B_UPDATED_MOVE_DATA >= GEN_4
    COMPOUND_STRING("Acumula fuerza tres veces\ny refuerza las defensas.") },
#else
    COMPOUND_STRING("Acumula fuerza hasta\ntres veces.") },
#endif
[MOVE_SPIT_UP] = { COMPOUND_STRING("Escupir"), COMPOUND_STRING("Libera la fuerza guardada.\nMás reservas, más daño.") },
[MOVE_SWALLOW] = { COMPOUND_STRING("Tragar"), COMPOUND_STRING("Consume reservas\npara recuperar PS.") },
[MOVE_HEAT_WAVE] = { COMPOUND_STRING("Onda Ígnea"), COMPOUND_STRING("Ola de calor que puede\nquemar a los rivales.") },
#if B_PREFERRED_ICE_WEATHER == B_ICE_WEATHER_SNOW
[MOVE_HAIL] = { COMPOUND_STRING("Paisaje Nevado"), COMPOUND_STRING("Crea nieve cinco turnos.\nSube Defensa de tipo Hielo.") },
#else
[MOVE_HAIL] = { COMPOUND_STRING("Granizo"), COMPOUND_STRING("Crea granizo que daña\ncada turno.") },
#endif
[MOVE_TORMENT] = { COMPOUND_STRING("Tormento"), COMPOUND_STRING("Impide usar el mismo\nmovimiento dos veces seguidas.") },
[MOVE_FLATTER] = { COMPOUND_STRING("Camelo"), COMPOUND_STRING("Confunde al rival, pero\nsube mucho su At. Esp.") },
[MOVE_WILL_O_WISP] = { COMPOUND_STRING("Fuego Fatuo"), COMPOUND_STRING("Lanza fuego fantasma\nque quema al rival.") },
[MOVE_MEMENTO] = { COMPOUND_STRING("Legado"), COMPOUND_STRING("Se debilita y reduce mucho\nambos ataques del rival.") },
[MOVE_FACADE] = { COMPOUND_STRING("Imagen"), COMPOUND_STRING("Gana fuerza si está\nquemado, paralizado o tóxico.") },
[MOVE_FOCUS_PUNCH] = { COMPOUND_STRING("Puño Certero"), COMPOUND_STRING("Se concentra. Falla si\nrecibe un golpe antes.") },
[MOVE_SMELLING_SALTS] = { COMPOUND_STRING("Estímulo"), COMPOUND_STRING("Daña más al paralizado,\npero elimina su parálisis.") },
[MOVE_FOLLOW_ME] = { COMPOUND_STRING("Señuelo"), COMPOUND_STRING("Atrae ataques rivales\nhacia el usuario.") },
[MOVE_NATURE_POWER] = { COMPOUND_STRING("Adaptación"), COMPOUND_STRING("El ataque usado cambia\nsegún el terreno.") },
[MOVE_CHARGE] = { COMPOUND_STRING("Carga"),
#if B_CHARGE_SPDEF_RAISE >= GEN_5
    COMPOUND_STRING("Potencia el próximo ataque\nEléctrico y sube Def. Esp.") },
#else
    COMPOUND_STRING("Potencia el próximo\nataque de tipo Eléctrico.") },
#endif
[MOVE_TAUNT] = { COMPOUND_STRING("Mofa"),
#if B_TAUNT_TURNS >= GEN_5
    COMPOUND_STRING("Solo permite atacar\ndurante tres turnos.") },
#else
    COMPOUND_STRING("Impide usar movimientos\nde estado varios turnos.") },
#endif
[MOVE_HELPING_HAND] = { COMPOUND_STRING("Refuerzo"), COMPOUND_STRING("Potencia el próximo\nataque de un aliado.") },
[MOVE_TRICK] = { COMPOUND_STRING("Truco"), COMPOUND_STRING("Intercambia los objetos\nequipados con el rival.") },
[MOVE_ROLE_PLAY] = { COMPOUND_STRING("Imitación"), COMPOUND_STRING("Copia la habilidad\ndel objetivo.") },
[MOVE_WISH] = { COMPOUND_STRING("Deseo"), COMPOUND_STRING("Concede curación de PS\nal turno siguiente.") },
[MOVE_ASSIST] = { COMPOUND_STRING("Ayuda"), COMPOUND_STRING("Usa al azar un movimiento\nde otro Pokémon del equipo.") },
[MOVE_INGRAIN] = { COMPOUND_STRING("Arraigo"), COMPOUND_STRING("Se arraiga y cura PS.\nNo puede retirarse.") },
[MOVE_SUPERPOWER] = { COMPOUND_STRING("Fuerza Bruta"), COMPOUND_STRING("Golpe fuerte que reduce\nsu Ataque y Defensa.") },
[MOVE_MAGIC_COAT] = { COMPOUND_STRING("Capa Mágica"), COMPOUND_STRING("Refleja ciertos efectos\nde estado contra el rival.") },
[MOVE_RECYCLE] = { COMPOUND_STRING("Reciclaje"), COMPOUND_STRING("Recupera un objeto\nconsumido anteriormente.") },
[MOVE_REVENGE] = { COMPOUND_STRING("Desquite"), COMPOUND_STRING("Daña más si recibió\nun golpe ese turno.") },
[MOVE_BRICK_BREAK] = { COMPOUND_STRING("Demolición"), COMPOUND_STRING("Golpea y rompe barreras\ncomo Reflejo.") },
[MOVE_YAWN] = { COMPOUND_STRING("Bostezo"), COMPOUND_STRING("Hace que el rival\nse duerma al próximo turno.") },
[MOVE_KNOCK_OFF] = { COMPOUND_STRING("Desarme"),
#if B_KNOCK_OFF_DMG >= GEN_6 && B_KNOCK_OFF_REMOVAL >= GEN_5
    COMPOUND_STRING("Retira el objeto rival.\nDaña más si llevaba uno.") },
#else
    COMPOUND_STRING("Elimina el objeto rival\ny evita que lo use.") },
#endif
[MOVE_ENDEAVOR] = { COMPOUND_STRING("Esfuerzo"), COMPOUND_STRING("Reduce los PS rivales\nal nivel de los propios.") },
[MOVE_ERUPTION] = { COMPOUND_STRING("Estallido"), COMPOUND_STRING("Cuantos más PS conserva,\nmás daño causa.") },
[MOVE_SKILL_SWAP] = { COMPOUND_STRING("Intercambio"), COMPOUND_STRING("Intercambia las habilidades\nde usuario y objetivo.") },
[MOVE_IMPRISON] = { COMPOUND_STRING("Sellar"), COMPOUND_STRING("Impide al rival usar ataques\nque también conozca.") },
[MOVE_REFRESH] = { COMPOUND_STRING("Alivio"), COMPOUND_STRING("Cura su propio veneno,\nparálisis o quemadura.") },
[MOVE_GRUDGE] = { COMPOUND_STRING("Rabia"), COMPOUND_STRING("Si cae, agota todos los PP\ndel último ataque rival.") },
[MOVE_SNATCH] = { COMPOUND_STRING("Robo"), COMPOUND_STRING("Roba el próximo efecto\nde mejora o curación rival.") },
[MOVE_SECRET_POWER] = { COMPOUND_STRING("Daño Secreto"), COMPOUND_STRING("Ataca con efectos que\nvarían según el terreno.") },
[MOVE_DIVE] = { COMPOUND_STRING("Buceo"), COMPOUND_STRING("Se sumerge primero y\ngolpea al turno siguiente.") },
[MOVE_ARM_THRUST] = { COMPOUND_STRING("Empujón"), COMPOUND_STRING("Golpea de dos\na cinco veces.") },
[MOVE_CAMOUFLAGE] = { COMPOUND_STRING("Camuflaje"), COMPOUND_STRING("Cambia de tipo según\nel terreno.") },
[MOVE_TAIL_GLOW] = { COMPOUND_STRING("Luminicola"),
#if B_UPDATED_MOVE_DATA >= GEN_5
    COMPOUND_STRING("Sube muchísimo\nel Ataque Especial.") },
#else
    COMPOUND_STRING("Sube mucho\nel Ataque Especial.") },
#endif
[MOVE_LUSTER_PURGE] = { COMPOUND_STRING("Resplandor"), COMPOUND_STRING("Explosión de luz que puede\nbajar la Defensa Especial.") },
[MOVE_MIST_BALL] = { COMPOUND_STRING("Bola Neblina"), COMPOUND_STRING("Bola de niebla que puede\nbajar el Ataque Especial.") },
[MOVE_FEATHER_DANCE] = { COMPOUND_STRING("Danza Pluma"), COMPOUND_STRING("Lanza plumas y reduce\nmucho el Ataque rival.") },
[MOVE_TEETER_DANCE] = { COMPOUND_STRING("Danza Caos"), COMPOUND_STRING("Confunde a todos\nlos demás Pokémon.") },
[MOVE_BLAZE_KICK] = { COMPOUND_STRING("Patada Ígnea"), COMPOUND_STRING("Patada de alto crítico\nque también puede quemar.") },
[MOVE_MUD_SPORT] = { COMPOUND_STRING("Chapoteolodo"),
#if B_UPDATED_MOVE_DATA >= GEN_6
    COMPOUND_STRING("Reduce daño de Eléctrico\ndurante cinco turnos.") },
#else
    COMPOUND_STRING("Cubre de lodo y debilita\nlos ataques Eléctricos.") },
#endif
[MOVE_ICE_BALL] = { COMPOUND_STRING("Bola Hielo"), COMPOUND_STRING("Ataca cinco turnos\ncon potencia creciente.") },
[MOVE_NEEDLE_ARM] = { COMPOUND_STRING("Brazo Pincho"), COMPOUND_STRING("Golpea con púas. Puede\nhacer retroceder.") },
[MOVE_SLACK_OFF] = { COMPOUND_STRING("Relajo"), COMPOUND_STRING("Descansa y recupera\nla mitad de sus PS.") },
[MOVE_HYPER_VOICE] = { COMPOUND_STRING("Vozarrón"), COMPOUND_STRING("Potente onda de sonido\nque daña a los rivales.") },
[MOVE_POISON_FANG] = { COMPOUND_STRING("Colmillo Veneno"), COMPOUND_STRING("Mordisco que puede\nenvenenar gravemente.") },
[MOVE_CRUSH_CLAW] = { COMPOUND_STRING("Garra Brutal"), COMPOUND_STRING("Araña con fuerza. Puede\nbajar la Defensa rival.") },
[MOVE_BLAST_BURN] = { COMPOUND_STRING("Anillo Ígneo"), COMPOUND_STRING("Ataque devastador que\nobliga a descansar después.") },
[MOVE_HYDRO_CANNON] = { COMPOUND_STRING("Hidrocañón"), COMPOUND_STRING("Ataque devastador que\nobliga a descansar después.") },
[MOVE_METEOR_MASH] = { COMPOUND_STRING("Puño Meteoro"), COMPOUND_STRING("Puñetazo como un meteoro.\nPuede subir su Ataque.") },
[MOVE_ASTONISH] = { COMPOUND_STRING("Impresionar"), COMPOUND_STRING("Asusta al rival y puede\nhacerlo retroceder.") },
[MOVE_WEATHER_BALL] = { COMPOUND_STRING("Meteorobola"), COMPOUND_STRING("Cambia de tipo y potencia\nsegún el clima.") },
[MOVE_AROMATHERAPY] = { COMPOUND_STRING("Aromaterapia"), COMPOUND_STRING("Cura estados alterados\nde todo el equipo.") },
[MOVE_FAKE_TEARS] = { COMPOUND_STRING("Llanto Falso"), COMPOUND_STRING("Finge llorar y reduce mucho\nla Defensa Especial rival.") },
[MOVE_AIR_CUTTER] = { COMPOUND_STRING("Aire Afilado"), COMPOUND_STRING("Viento cortante con\nalta probabilidad de crítico.") },
[MOVE_OVERHEAT] = { COMPOUND_STRING("Sofoco"), COMPOUND_STRING("Ataque muy fuerte que\nbaja mucho su At. Esp.") },
[MOVE_ODOR_SLEUTH] = { COMPOUND_STRING("Rastreo"), COMPOUND_STRING("Anula evasión e inmunidad\nFantasma del objetivo.") },
[MOVE_ROCK_TOMB] = { COMPOUND_STRING("Tumba Rocas"), COMPOUND_STRING("Atrapa con rocas y reduce\nla Velocidad rival.") },
[MOVE_SILVER_WIND] = { COMPOUND_STRING("Viento Plata"), COMPOUND_STRING("Onda de polvo que puede\nsubir todas sus estadísticas.") },
[MOVE_METAL_SOUND] = { COMPOUND_STRING("Eco Metálico"), COMPOUND_STRING("Chirrido metálico que baja\nmucho Defensa Especial.") },
[MOVE_GRASS_WHISTLE] = { COMPOUND_STRING("Silbato"), COMPOUND_STRING("Melodía suave que\nduerme al rival.") },
[MOVE_TICKLE] = { COMPOUND_STRING("Cosquillas"), COMPOUND_STRING("Hace reír y baja Ataque\ny Defensa del rival.") },
[MOVE_COSMIC_POWER] = { COMPOUND_STRING("Masa Cósmica"), COMPOUND_STRING("Sube Defensa y\nDefensa Especial propias.") },
[MOVE_WATER_SPOUT] = { COMPOUND_STRING("Salpicar"), COMPOUND_STRING("Causa más daño cuantos\nmás PS conserve.") },
[MOVE_SIGNAL_BEAM] = { COMPOUND_STRING("Rayo Señal"), COMPOUND_STRING("Rayo extraño que puede\nconfundir al rival.") },
[MOVE_SHADOW_PUNCH] = { COMPOUND_STRING("Puño Sombra"), COMPOUND_STRING("Puñetazo sombrío que\nnunca falla.") },
[MOVE_EXTRASENSORY] = { COMPOUND_STRING("Paranormal"), COMPOUND_STRING("Poder extraño que puede\nhacer retroceder.") },
[MOVE_SKY_UPPERCUT] = { COMPOUND_STRING("Gancho Alto"), COMPOUND_STRING("Gancho ascendente\ncontra el rival.") },
[MOVE_SAND_TOMB] = { COMPOUND_STRING("Bucle Arena"), COMPOUND_STRING("Atrapa al rival en arena\ndurante varios turnos.") },
[MOVE_SHEER_COLD] = { COMPOUND_STRING("Frío Polar"),
#if B_SHEER_COLD_ACC >= GEN_7
    COMPOUND_STRING("Puede causar KO directo.\nMenos preciso sin tipo Hielo.") },
#else
    COMPOUND_STRING("Ataque gélido que causa\nKO directo si acierta.") },
#endif
[MOVE_MUDDY_WATER] = { COMPOUND_STRING("Agua Lodosa"), COMPOUND_STRING("Ola de barro que puede\nbajar la precisión rival.") },
[MOVE_BULLET_SEED] = { COMPOUND_STRING("Recurrente"), COMPOUND_STRING("Dispara de dos a cinco\nsemillas seguidas.") },
[MOVE_AERIAL_ACE] = { COMPOUND_STRING("Golpe Aéreo"), COMPOUND_STRING("Ataque veloz que\nnunca falla.") },
[MOVE_ICICLE_SPEAR] = { COMPOUND_STRING("Carámbano"), COMPOUND_STRING("Dispara de dos a cinco\npuntas de hielo.") },
[MOVE_IRON_DEFENSE] = { COMPOUND_STRING("Defensa Férrea"), COMPOUND_STRING("Endurece el cuerpo y\nsube mucho Defensa.") },
[MOVE_BLOCK] = { COMPOUND_STRING("Bloqueo"), COMPOUND_STRING("Cierra el paso e impide\nque el rival escape.") },
[MOVE_HOWL] = { COMPOUND_STRING("Aullido"),
#if B_UPDATED_MOVE_DATA >= GEN_8
    COMPOUND_STRING("Aullido que aumenta\nel Ataque de los aliados.") },
#else
    COMPOUND_STRING("Aullido que aumenta\nel Ataque del usuario.") },
#endif
[MOVE_DRAGON_CLAW] = { COMPOUND_STRING("Garra Dragón"), COMPOUND_STRING("Ataque con garras\nde dragón afiladas.") },
[MOVE_FRENZY_PLANT] = { COMPOUND_STRING("Planta Feroz"), COMPOUND_STRING("Ataque devastador que\nobliga a descansar después.") },
[MOVE_BULK_UP] = { COMPOUND_STRING("Corpulencia"), COMPOUND_STRING("Sube su Ataque\ny Defensa.") },
[MOVE_BOUNCE] = { COMPOUND_STRING("Bote"), COMPOUND_STRING("Salta un turno y cae\nal siguiente. Puede paralizar.") },
[MOVE_MUD_SHOT] = { COMPOUND_STRING("Disparo Lodo"), COMPOUND_STRING("Lanza barro y reduce\nla Velocidad rival.") },
[MOVE_POISON_TAIL] = { COMPOUND_STRING("Cola Veneno"), COMPOUND_STRING("Alto índice crítico.\nTambién puede envenenar.") },
[MOVE_COVET] = { COMPOUND_STRING("Antojo"), COMPOUND_STRING("Ataca y toma el objeto\nque llevaba el rival.") },
[MOVE_VOLT_TACKLE] = { COMPOUND_STRING("Placaje Eléc."), COMPOUND_STRING("Placaje que daña al usuario\ny puede paralizar.") },
[MOVE_MAGICAL_LEAF] = { COMPOUND_STRING("Hoja Mágica"), COMPOUND_STRING("Hoja especial que\nnunca falla.") },
[MOVE_WATER_SPORT] = { COMPOUND_STRING("Hidrochorro"),
#if B_UPDATED_MOVE_DATA >= GEN_6
    COMPOUND_STRING("Reduce daño de Fuego\ndurante cinco turnos.") },
#else
    COMPOUND_STRING("Cubre de agua y debilita\nlos ataques de Fuego.") },
#endif
[MOVE_CALM_MIND] = { COMPOUND_STRING("Paz Mental"), COMPOUND_STRING("Sube el Ataque Especial\ny la Defensa Especial.") },
[MOVE_LEAF_BLADE] = { COMPOUND_STRING("Hoja Aguda"), COMPOUND_STRING("Cuchillada con alto\níndice de golpes críticos.") },
[MOVE_DRAGON_DANCE] = { COMPOUND_STRING("Danza Dragón"), COMPOUND_STRING("Sube su Ataque\ny su Velocidad.") },
[MOVE_ROCK_BLAST] = { COMPOUND_STRING("Pedrada"), COMPOUND_STRING("Dispara de dos a cinco\nrocas seguidas.") },
[MOVE_SHOCK_WAVE] = { COMPOUND_STRING("Onda Voltio"), COMPOUND_STRING("Descarga eléctrica que\nnunca falla.") },
[MOVE_WATER_PULSE] = { COMPOUND_STRING("Hidropulso"), COMPOUND_STRING("Ondas de agua que\npueden confundir.") },
[MOVE_DOOM_DESIRE] = { COMPOUND_STRING("Deseo Oculto"), COMPOUND_STRING("Luz intensa que golpea\ntras dos turnos.") },
[MOVE_PSYCHO_BOOST] = { COMPOUND_STRING("Psicoataque"), COMPOUND_STRING("Ataque poderoso que reduce\nmucho el Ataque Especial.") },
