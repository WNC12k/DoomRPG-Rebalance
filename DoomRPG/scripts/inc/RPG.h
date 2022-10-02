#ifndef _RPG_DH_
#define _RPG_DH_

#include "Defs.h"

// Version
extern const str Version;

// Flags
extern bool Transported;
extern bool GlobalsInitialized;
extern int CompatMode;
extern int CompatModeEx;
extern int CompatMonMode;
extern bool MapPacks;

// Arrays
extern str PlayerWeapon[MAX_PLAYERS];

// Structs
extern PlayerData _PlayerData[MAX_PLAYERS];

// Item TIDs
extern bool RPGMap ItemTIDsInitialized;
extern int RPGMap ItemTIDs[MAX_ITEMS];

// --------------------------------------------------
// Multiplayer HUD
//

// Energy Points (EP)
extern int EPArray Address(0) EP[MAX_PLAYERS];

// Shield
extern int ShieldArray Address(0) Shield[MAX_PLAYERS];
extern int ShieldCapacityArray Address(0) ShieldCapacity[MAX_PLAYERS];
extern int ShieldHealthArray Address(0) ShieldHealth[MAX_PLAYERS];

//Stats
extern str const StatColors[STAT_MAX];

NamedScript KeyBind void ToggleFocusMode();
NamedScript void FocusMode();
NamedScript DECORATE void StatusEffect(int, int, int);
NamedScript DECORATE int AddHealth(int, int);
NamedScript DECORATE int AddHealthDirect(int, int);
NamedScript void GiveTip();

NamedScript Type_OPEN void GlobalInit();
NamedScript Type_ENTER void Init();
NamedScript void Loop();
NamedScript void WeaponSpeed();
NamedScript void PlayerHealth();
NamedScript void PlayerSurvive();
NamedScript DECORATE int PlayerDamage(int, int);
NamedScript DECORATE int ShieldDamage(int);
NamedScript void MoneyChecker();
NamedScript void StatRandomizer();
NamedScript void AutosaveHandler();
NamedScript Type_OPEN void ShopSpecialHandler();
NamedScript void ItemHandler();
NamedScript DECORATE void ItemInit();
NamedScript OptionalArgs(1) void DynamicLootGenerator(str, int);
NamedScript Type_DEATH void Dead();
NamedScript Type_RESPAWN void Respawn();
NamedScript void SortStartingItems();
NamedScript void DefaultLoadout();
NamedScript void ReviveHandler();
NamedScript bool SomePlayerAlive();

void CheckHUD();
void CheckKeys();
void CheckCompatibility();
void AssignTIDs();

#endif
