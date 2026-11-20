#ifndef GUARD_UNHOLY_WORLD_UPDATE_H
#define GUARD_UNHOLY_WORLD_UPDATE_H

// Reconcile versioned save state and dynamic events before map objects spawn.
void UnholyWorldUpdate_Reconcile(void);

// Invoked from the in-game QoL settings. Refresh only versioned additions;
// never clear a reward's permanent CLAIMED flag or native pickup flags.
struct ScriptContext;
void UnholyWorldUpdate_RefreshCurrentMap(struct ScriptContext *ctx);

#endif // GUARD_UNHOLY_WORLD_UPDATE_H
