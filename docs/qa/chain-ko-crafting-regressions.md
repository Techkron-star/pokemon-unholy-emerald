# Chain KO / crafting / summary QA (feat/stamina)

## Reproducible battle checks

1. Player chains two different moves; first move KOs foe. The second move must NOT hit the replacement.
2. Rival chains; its first attack KOs the player's mon. The remaining queued attack must NOT hit the replacement.
3. Player's first move KOs the foe while the rival had two moves queued. Cancel every pending action in that expanded turn.
4. Rival's first move KOs player while player had two moves queued. Cancel every pending action in that expanded turn.
5. KO on the very last queued move: no out-of-bounds access of turn-action index 4.
6. No KO: both sides may chain, maintaining roles and their intended order. Verify PP/stamina costs.
7. No chain: standard one-action-per-side behavior remains unchanged.
8. Trainer KO and forced switch: exactly one replacement prompt; no duplicate or invalid switch index assertion.
9. Fainted attacker: no queued move or Z-Move setup starts for the replacement.
10. Lethal Tempo exception: NOT implemented; keep default post-KO cancellation until the ability has its own tests.

## UI checks

- Open Crafting in the Start menu after unlocking it; choose the Super Exp. Share recipe or Exit/B.
- With no Exp. Share: show missing-ingredient message without corrupting the display.
- With the Exp. Share, with and without Stone Badge: validate prerequisites and completion.
- Inspect Pokémon Info with Alpha, Super, Enraged and Omega flags: badges stay on the ability name line; ability description remains readable.
- Omega glint: verify shiny-only, Omega-only, and shiny+Omega entry effects in both wild/trainer and single/double battles; no blocked turns, resource leak or color corruption.
- Full graphical crafting and advanced Horizon stock still need emulator QA and full implementation.

## Verification status

Code reviewed and source changes prepared. A ROM build, emulator test and CI result are required before declaring these fixes fully validated.
