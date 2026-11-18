# Unholy Wild Bosses V1 · Aegis Guardian

The first boss is a level-62 Registeel at Victory Road B2F (14, 8), using the existing Registeel overworld graphic. Press A to challenge.

- Special `UnholyWildBoss_Arm` runs after `setwildbattle`. It guarantees Alpha while retaining any naturally generated mutation(s).
- The opponent receives +2 Attack, +3 Defense, +3 Special Defense and +1 Speed **as temporary in-battle stat stages**, after initial BattleMon data is loaded.
- Defeat: award `ITEM_GEAR_ESCUDO_AEGIS` and hide the overworld boss.
- Capture: keep Registeel with its real mutation(s) but without stat-stage boosts; forfeit the artifact.
- Flee or lose: no reward, no completion flag.
- Full bag: do not set the completion flag; boss remains available for a new attempt.
- After the Super League, `MAP_SCRIPT_ON_TRANSITION` clears the completed flag each time the player enters Victory Road B2F, permitting another attempt with a fresh capture/reward. This intentionally allows multiple copies over time.

## Testing checklist

1. Confirm Registeel is reachable and visible at 14,8 without blocking the normal route.
2. Challenge: verify its first-turn stage bonuses; lowering those stages should still work normally.
3. Capture: check the resulting PC/party Pokémon has Alpha, may have other rolled mutations, and has **no boss-only boosts**.
4. Defeat: check exactly one Aegis is received and the boss disappears.
5. Run/whiteout: it must remain available with no Aegis awarded.
6. With full bag: reward is not lost permanently.
7. Complete Super League, re-enter B2F, and verify replay is possible repeatedly.
8. Verify ordinary Registeel encounters, raids and trainers never gain the boss boosts.

### Next phase

Route 126's Breakwater boss requires the actual Rompeolas item ID/effect before the reward script can be wired. Extend the same encounter-ID dispatch rather than copying the combat hook.
