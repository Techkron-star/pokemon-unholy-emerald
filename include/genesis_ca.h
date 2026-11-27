#ifndef GUARD_GENESIS_CA_H
#define GUARD_GENESIS_CA_H
#include "genesis_core.h"
#define GENESIS_CA_MAX_ITERATIONS 16
void GenesisCA_Init(u8*,u16,u16,u32,u8);
void GenesisCA_Step(u8*,u8*,u16,u16,u8);
void GenesisCA_Run(u8*,u8*,u16,u16,u32,u8,u8,u8);
#endif
