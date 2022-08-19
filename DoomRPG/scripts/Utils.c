#include "Defs.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "Augs.h"
#include "ItemData.h"
#include "Map.h"
#include "Mission.h"
#include "Monsters.h"
#include "Outpost.h"
#include "RPG.h"
#include "Shield.h"
#include "Shop.h"
#include "Skills.h"
#include "Stats.h"
#include "Utils.h"

// --------------------------------------------------
// Variables
//

bool DebugLog;
int const AuraTID = 20000;

// Skill levels stuff
int SkillLevelsMax;

// ----------------------
// Input
//

// Used with bindingStates in order
int const bindingValues[13] =
{
    BT_FORWARD,
    BT_BACK,
    BT_MOVELEFT,
    BT_MOVERIGHT,
    BT_CROUCH,
    BT_SPEED,
    BT_USE,
    BT_ATTACK,
    BT_ALTATTACK,
    BT_ZOOM,
    BT_JUMP,
    BT_USER1,
    BT_USER2
};

// Keep track of KeyDown for multiple keys (requried for key combos, such as switching pages)
bool bindingStates[13];
// Currently pressed key
int keyCurrent;
// Determines whether certain held down keys can repeat themselves
bool keyRepetition = false;

str const ColorNames[26] =
{
    "Brick",
    "Tan",
    "Grey",
    "Green",
    "Brown",
    "Gold",
    "Red",
    "Blue",
    "Orange",
    "White",
    "Yellow",
    "Untranslated",
    "Black",
    "Light Blue",
    "Cream",
    "Olive",
    "Dark Green",
    "Dark Red",
    "Dark Brown",
    "Purple",
    "Dark Grey",
    "Cyan",
    "Ice",
    "Fire",
    "Sapphire",
    "Teal"
};

str const ColorCodes[26] =
{
    "\Ca",
    "\Cb",
    "\Cc",
    "\Cd",
    "\Ce",
    "\Cf",
    "\Cg",
    "\Ch",
    "\Ci",
    "\Cj",
    "\Ck",
    "\Cl",
    "\Cm",
    "\Cn",
    "\Co",
    "\Cp",
    "\Cq",
    "\Cr",
    "\Cs",
    "\Ct",
    "\Cu",
    "\Cv",
    "\Cw",
    "\Cx",
    "\Cy",
    "\Cz"
};

// Skill Level Names
str const SkillLevelsDF[MAX_SKILLLEVELS_DF] =
{
    "\CdEasy",
    "\CjNormal",
    "\CiHard",
    "\CgNightmare",
    "\CaHell",
    "\CmArmageddon"
};

// Skill Level DRLA Names
str const SkillLevelsDRLA[MAX_SKILLLEVELS_DRLA] =
{
    "\CdVery Easy",
    "\CqEasy",
    "\CjModerate",
    "\CiStandard",
    "\CyAdaptive",
    "\CaNightmare",
    "\CnTechnophobia",
    "\CmArmageddon"
};

// TODO: Just number these and use StrParam
str const AuraIcons[AURA_MAX + 1] =
{
    "AuraRed",
    "AuraGree",
    "AuraWhit",
    "AuraPink",
    "AuraCyan",
    "AuraPurp",
    "AuraOran",
    "AuraBlue",
    "AuraYell",
    "AuraBlac"
};


// --------------------------------------------------
// DECORATE
//

NamedScript DECORATE int InOutpost()
{
    return (int)CurrentLevel->UACBase;
}

NamedScript DECORATE int GetUniqueTID()
{
    return UniqueTID();
}

NamedScript DECORATE int GetAmmoMax(int Type)
{
    switch (Type)
    {
    case AMMO_CLIP:
        return GetAmmoCapacity("Clip");
    case AMMO_SHELL:
        return GetAmmoCapacity("Shell");
    case AMMO_ROCKET:
        return GetAmmoCapacity("RocketAmmo");
    case AMMO_CELL:
        return GetAmmoCapacity("Cell");
    }

    return 0;
}

NamedScript DECORATE void SoulEffect(int Type)
{
    Player.Aura.Type[Type].Active = true;

    if (Player.SoulsCount < 40)
        Player.Aura.Time += (35 * 30) * (1 - (Player.SoulsCount / 50));
    else
        Player.Aura.Time += 35 * 6;

    // Cap Aura Timer - 10 min
    if (Player.Aura.Time > 35 * 600)
    {
        Player.Aura.Time = 35 * 600;
    }

    Player.SoulActive[Type] = true;
}

// Used by DECORATE and the Immunity Crystals plus Anti-Demon Field
NamedScript DECORATE void DestroyImmunity()
{
    TakeInventory("DRPGPowerImmunityMelee", 1);
    TakeInventory("DRPGPowerImmunityBullet", 1);
    TakeInventory("DRPGPowerImmunityFire", 1);
    TakeInventory("DRPGPowerImmunityPlasma", 1);
    TakeInventory("DRPGPowerImmunityLightning", 1);
    TakeInventory("DRPGPowerImmunityToxic", 1);
    TakeInventory("DRPGPowerImmunityRadiation", 1);
    TakeInventory("PowerProtection", 1);
}

// Return the Skill Level (for DECORATE)
NamedScript DECORATE int GetSkillLevel()
{
    return GameSkill();
}

// Return the Global shop card rank between all players (for DECORATE)
NamedScript DECORATE int GetShopCard()
{
    return GlobalShopCard;
}

// Set the damage type recieved by the Player this tic (for DECORATE)
NamedScript DECORATE void SetDamageType(int Type)
{
    Player.DamageType = Type;
}

// Return whether your Aug Battery is full or not1
NamedScript DECORATE int CheckAugBatteryMax()
{
    return (Player.Augs.Battery >= Player.Augs.BatteryMax);
}

// Get the max inventory size
NamedScript DECORATE int CheckInventoryMax()
{
    int MaxItems = 5 + Player.CapacityTotal / 2;

    if (MaxItems > 100)
        MaxItems = 100;

    return MaxItems;
}

// Used for handling inventory items using the Capacity stat
NamedScript DECORATE int CheckCapacity()
{
    int Items = 0;
    int MaxItems = CheckInventoryMax();
    bool IsTechnician = (PlayerClass(PlayerNumber()) == 2);

    // Add Capacity XP for total carried items
    if (GetCVar("drpg_levelup_natural") && Timer() % 7 == 0)
    {
        fixed Scale = GetCVarFixed("drpg_capacity_scalexp");
        if (GetCVar("drpg_allow_spec"))
        {
            if (GetActivatorCVar("drpg_character_spec") == 7)
                Scale *= 2;
        }

        if (CompatMode == COMPAT_DRLA)
        {
            // Calculate capacity usage per category in DRLA, excluding weapons and modpacks
            int DRLAItems = CheckInventory("RLArmorInInventory") + CheckInventory("RLSkullLimit") + CheckInventory("RLPhaseDeviceLimit") + (IsTechnician ? CheckInventory("RLScavengerModLimit") : CheckInventory("RLModLimit"));
            int DRLAMaxItems = DRLA_ARMOR_MAX + DRLA_SKULL_MAX + DRLA_DEVICE_MAX + DRLA_MODPACKS_MAX;
            Player.CapacityXP += (int)((DRLAItems + Player.InvItems) * Scale / (DRLAMaxItems + MaxItems) * 20.0);
        }
        else
            Player.CapacityXP += (int)(Player.InvItems * Scale / MaxItems * 20.0);
    }

    // Don't do checks if you have the system disabled
    if (!GetCVar("drpg_inv_capacity")) return true;

    str const ItemList[] =
    {
        // Health
        "DRPGStimpack",
        "DRPGMedikit",
        "DRPGLargeMedikit",
        "DRPGXLMedikit",
        "DRPGMedPack",
        "DRPGSurgeryKit",

        // Armor
        "DRPGGreenArmor",
        "DRPGBlueArmor",
        "DRPGYellowArmor",
        "DRPGRedArmor",
        "DRPGWhiteArmor",
        "DRPGReinforcedGreenArmor",
        "DRPGReinforcedBlueArmor",
        "DRPGReinforcedYellowArmor",
        "DRPGReinforcedRedArmor",
        "DRPGReinforcedWhiteArmor",

        // Powerups
        "DRPGInvulnerabilityCharge",
        "DRPGInvisibilityCharge",
        "DRPGTimeSphere",
        "DRPGRegenSphere",
        "DRPGRadSuit",
        "DRPGInfrared",
        "DRPGAllMap",
        "DRPGBerserk",
        "DRPGWings",

        // Stims
        "DRPGStimDetox",

        // Batteries
        "DRPGBatterySmall",
        "DRPGBatteryLarge",
        NULL
    };


    for (int i = 0; ItemList[i] != NULL; i++)
        if (CheckInventory(ItemList[i]) > 0)
            Items += CheckInventory(ItemList[i]);

    if (CompatMode == COMPAT_DRLA)
    {
        str const ItemListRL[] =
        {
            // DoomRL - Powerups
            "InvulnerabilityCharge2",
            "RadSuit2",
            "InvisibilityCharge2",
            "Infrared2",
            "Berserk2",

            // Craft Parts
            "DRPGCraftPartsExotic",
            "DRPGCraftPartsUnique",

            // End of List
            NULL
        };

        for (int i = 0; ItemListRL[i] != NULL; i++)
            if (CheckInventory(ItemListRL[i]) > 0)
                Items += CheckInventory(ItemListRL[i]);
    };

    if (CompatModeEx == COMPAT_DRLAX)
    {
        str const ItemListRLAX[] =
        {
            // DoomRLAX - Items
            "DRLAX_FamiliarBall",
            "DRLAX_RadarDevice",
            "DRLAX_SoulTrap",

            // DoomRLAX - Trapped Soul Spheres
            "DRLAX_TrappedInvulnerabilitySphere",
            "DRLAX_TrappedInvisibilitySphere",
            "DRLAX_TrappedSoulSphere",
            "DRLAX_TrappedSoulSphere3",
            "DRLAX_TrappedSoulSphere4",
            "DRLAX_TrappedMegaSphere",
            "DRLAX_TrappedAGHSphere",
            "DRLAX_TrappedFirebluSphere",

            // End of List
            NULL
        };

        for (int i = 0; ItemListRLAX[i] != NULL; i++)
            if (CheckInventory(ItemListRLAX[i]) > 0)
                Items += CheckInventory(ItemListRLAX[i]);
    };

    Player.InvItems = Items;

    if (Items >= MaxItems)
        return false;
    else
        return true;
}

// Return Timer() for DECORATE
NamedScript DECORATE int GetTimer()
{
    return Timer();
}

// Returns the Agility for A_SetTics calls in DECORATE
NamedScript DECORATE int GetSpeed(int Tics)
{
    return (Tics + 1) - (Tics * Player.AgilityTotal / 100);
}

// Used by DECORATE to check if both your Health and EP are at max
NamedScript DECORATE int HPEPMax()
{
    return (int)(Player.ActualHealth >= Player.HealthMax && Player.EP >= Player.EPMax);
}

// Really only used to bypass startup error spam due to looking for DRLA actors
NamedScript DECORATE int PowersuitCheck()
{
    bool ValidPlayer = false;

    SetActivatorToTarget(0);

    for (int i = 0; i < MAX_PLAYERS; i++)
        if (ActivatorTID() == Players(i).TID)
        {
            ValidPlayer = true;
            break;
        }

    if (ValidPlayer)
    {
        if (CheckInventory("PowerRLReactiveShieldSystemBullet"))
            return 1; // Bullet
        if (CheckInventory("RLPlasmaShieldArmorToken") || CheckInventory("PowerRLReactiveShieldSystemPlasma"))
            return 2; // Plasma
        if (CheckInventory("PowerRLReactiveShieldSystemFire") || CheckInventory("RLInquisitorsSetBonusActive"))
            return 3; // Fire
        if (CheckInventory("PowerInvulnerable") ||
                CheckInventory("PowerTimeFreezer") ||
                CheckInventory("DRPGPowerStimIndestructible") ||
                CheckInventory("DRPGPowerImmunityBullet") ||
                CheckInventory("DRPGPowerImmunityMelee") ||
                CheckInventory("DRPGPowerImmunityPlasma"))
            return 4; // Powerups/Immunities for Anti Demon Field
        else
            return 0;
    }
    else
        return -1;
}

NamedScript DECORATE void RadiationDamage(int damageamount)
{
    if (CheckInventory("PowerIronFeet") || CheckInventory("DRPGSkillIronFeet"))
        return;

    int mytid = UniqueTID();
    int oldtid = ActivatorTID();

    Thing_ChangeTID(0, mytid);
    SetActivator(0, AAPTR_NULL);
    Thing_Damage2(mytid, damageamount, "Radiation");
    Thing_ChangeTID(mytid, oldtid);
}

NamedScript DECORATE void CauseCorrosion()
{
    if (CheckInventory("PowerIronFeet") || CheckInventory("DRPGSkillIronFeet"))
        return;

    if (Random(0, 3))
        return;

    if (CheckInventory("BasicArmor") && Random(0, 3))
    {
        if (!Player.StatusType[SE_CORROSION] || Player.StatusTimer[SE_CORROSION] < 10500)
            TryStatusEffect(SE_CORROSION, Random(1, 10), Random(1, 3));
    }
    else
    {
        if (!Player.StatusType[SE_CORROSION] || Player.StatusTimer[SE_POISON] < 10500)
            TryStatusEffect(SE_POISON, Random(1, 10), Random(1, 3));
    }
}

NamedScript DECORATE int ShieldRemoveArmor()
{
    return Player.Shield.Active;
}

NamedScript DECORATE int GetAveragePlayerLevel()
{
    return AveragePlayerLevel();
}

NamedScript DECORATE int GetModuleSpawnFactor()
{
    return GetCVarFixed("drpg_module_spawnfactor") * 100;
}

NamedScript DECORATE int StatusEffectSeverity(int Effect)
{
    if (!Player.StatusType[Effect])
        return 0;

    return (int)Player.StatusIntensity[Effect];
}

NamedScript DECORATE int StatusEffectTimer(int Effect)
{
    if (!Player.StatusType[Effect])
        return 0;

    return (int)Player.StatusTimer[Effect];
}

// Super pulsating rainbow translation effect
NamedScript DECORATE void RainbowTranslationPulse(int Time)
{
Start:

    if (ClassifyActor(0) == ACTOR_WORLD) return;

    fixed Red = 1.5 + (Sin(((fixed)Timer()) / (fixed)Time) * 1.5);
    fixed Green = 1.5 + (Sin(((fixed)Timer() + 11.55) / (fixed)Time) * 1.5);
    fixed Blue = 1.5 + (Sin(((fixed)Timer() + 23.45) / (fixed)Time) * 1.5);

    CreateTranslationStart(63);
    CreateTranslationDesat(0, 255, 0, 0, 0, Red, Green, Blue);
    CreateTranslationEnd();

    Thing_SetTranslation(0, 63);

    Delay(1);
    goto Start;
}

NamedScript DECORATE int CheckActorSky()
{
    return CheckActorCeilingTexture(0, "F_SKY1");
}

// --------------------------------------------------
// Monsters
//

// For calling from DECORATE
NamedScript DECORATE void HealMonster(int HealPercent, int MaxPercent)
{
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];
    long int Health = GetActorProperty(0, APROP_Health);
    long int HealAmount = Stats->HealthMax * HealPercent / 100;
    long int HealMax = Stats->HealthMax * MaxPercent / 100;

    SetActorProperty(0, APROP_Health, Health + HealAmount);

    if (GetActorProperty(0, APROP_Health) >= HealMax)
        SetActorProperty(0, APROP_Health, HealMax);
}

NamedScript DECORATE void TeleportMonster()
{
    str Type = GetActorClass(0);
    int TID = UniqueTID();
    bool Success = false;
    Position *ChosenPosition = NULL;

    // Check the position
    while (!Success)
    {
        ChosenPosition = &((Position *)CurrentLevel->MonsterPositions.Data)[Random(0, CurrentLevel->MonsterPositions.Position)];
        Success = Spawn(Type, ChosenPosition->X, ChosenPosition->Y, ChosenPosition->Z, TID, ChosenPosition->Angle);
        Thing_Remove(TID);
        Delay(1);
    }

    // Teleport to this position
    SetActorPosition(0, ChosenPosition->X, ChosenPosition->Y, ChosenPosition->Z, true);
    SetActorAngle(0, ChosenPosition->Angle);
    SetActorPitch(0, ChosenPosition->Pitch);
}

void DropMoney(int Killer, int TID, int Amount)
{
    int Total = Amount;
    int Drops;

    Drops = Total / 1000;
    while (Drops--) DropMonsterItem(Killer, TID, "DRPGCredits1000", 256);
    Total %= 1000;

    Drops = Total / 500;
    while (Drops--) DropMonsterItem(Killer, TID, "DRPGCredits500", 256);
    Total %= 500;

    Drops = Total / 250;
    while (Drops--) DropMonsterItem(Killer, TID, "DRPGCredits250", 256);
    Total %= 250;

    Drops = Total / 100;
    while (Drops--) DropMonsterItem(Killer, TID, "DRPGCredits100", 256);
    Total %= 100;

    Drops = Total / 50;
    while (Drops--) DropMonsterItem(Killer, TID, "DRPGCredits50", 256);
    Total %= 50;

    Drops = Total / 20;
    while (Drops--) DropMonsterItem(Killer, TID, "DRPGCredits20", 256);
    Total %= 20;

    Drops = Total / 10;
    while (Drops--) DropMonsterItem(Killer, TID, "DRPGCredits10", 256);
    Total %= 10;

    Drops = Total / 5;
    while (Drops--) DropMonsterItem(Killer, TID, "DRPGCredits5", 256);
    Total %= 5;

    Drops = Total;
    while (Drops--) DropMonsterItem(Killer, TID, "DRPGCredits1", 256);
}

int DropMonsterItem(int Killer, int TID, str Item, int Chance, fixed XAdd, fixed YAdd, fixed ZAdd, fixed XVelAdd, fixed YVelAdd, fixed ZVelAdd)
{
    // if this is an invalid killer, return
    if (!(Killer > -1)) return 0;

    // Chance
    if (!(Random(1, 256) <= Chance)) return 0;

    fixed Angle = GetActorAngle(TID);
    fixed X = GetActorX(TID) + XAdd;
    fixed Y = GetActorY(TID) + YAdd;
    fixed Z = GetActorZ(TID) + (GetActorPropertyFixed(0, APROP_Height) / 2.0) + ZAdd;
    fixed XSpeed = GetCVarFixed("drpg_monster_dropdist") + XVelAdd;
    fixed YSpeed = GetCVarFixed("drpg_monster_dropdist") + YVelAdd;
    fixed ZSpeed = 8.0 + ZVelAdd;
    int ItemTID = UniqueTID();

    // Spawn the Item
    bool Success = SpawnForced(Item, X, Y, Z, ItemTID, Angle);

    // Spawn successful
    if (Success)
    {
        // Set Velocity
        if (Players(Killer).Stim.PowerupTimer[STIM_MAGNETIC] <= 0) // Don't toss the item if we're Magnetic, it'll just confuse things
            SetActorVelocity(ItemTID, RandomFixed(-XSpeed, XSpeed), RandomFixed(-YSpeed, YSpeed), ZSpeed, false, false);

        // Array has grown too big, resize it
        if (Players(Killer).DropTID.Position == Players(Killer).DropTID.Size)
            ArrayResize(&Players(Killer).DropTID);

        // Add item's TID to drop array
        ((int *)Players(Killer).DropTID.Data)[Players(Killer).DropTID.Position++] = ItemTID;
    }

    return ItemTID;
}

// Use for while if you want to make a delay when the monster sees the players
bool ActorSeePlayers(int MonsterTID, int Dist)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!PlayerInGame(i)) continue;

        if (!CheckSight(MonsterTID, Players(i).TID, 0) || Distance(MonsterTID, Players(i).TID) > Dist)
            return false;
    }

    return true;
}

// Use for while if you want to make a delay when the monster doesn't see the players
bool ActorNotSeePlayers(int MonsterTID, int Dist)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!PlayerInGame(i)) continue;

        if (CheckSight(MonsterTID, Players(i).TID, 0) || Distance(MonsterTID, Players(i).TID) <= Dist)
            return false;

        if (!Players(i).Summons == 0)
        {
            for (int j = 0; j < Players(i).Summons; j++)
            {
                if (CheckSight(MonsterTID, Players(i).SummonTID[j], 0) || Distance(MonsterTID, Players(i).SummonTID[j]) <= Dist)
                    return false;
            }
        }
    }

    return true;
}

// Corpses Cleanup
NamedScript DECORATE void CorpsesCleanup()
{
    if (Random(1, 2) <= GetCVar("drpg_corpses_cleanup"))
    {
        Delay(35 * (GetCVar("drpg_corpses_cleanup_timer")));

        while (ActorSeePlayers(0, 2048)) Delay(35 * 10);

        Thing_Remove(0);
    }
}

// --------------------------------------------------
// Players
//

// Used by the RegenSphere to temporarily increase regen rates
NamedScript DECORATE void RegenBoost()
{
    Player.RegenBoostTimer += (35 * 15) + ((Player.RegenerationTotal / 2) * 35);
}

// Set Skill Level during the game
NamedScript KeyBind void SetSkill(int NewSkill)
{
    if (NewSkill < 0 || NewSkill > SkillLevelsMax - 1)
    {
        HudMessage("Invalid Skill Level");
        EndHudMessage(HUDMSG_FADEOUT, 0, "Red", 0.5, 0.5, 2.0, 1.0);
        return;
    }

    FadeRange(255, 255, 255, 0.5, 255, 255, 255, 0.0, 0.5);
    ChangeSkill(NewSkill);
    CurrentSkill = NewSkill;
    ActivatorSound("misc/skillchange", 127);
    SetFont("BIGFONT");
    if (CompatMonMode == COMPAT_DRLA)
        HudMessage("\CjSkill Level has been changed to\n\n%S", SkillLevelsDRLA[NewSkill]);
    else
        HudMessage("\CjSkill Level has been changed to\n\n%S", SkillLevelsDF[NewSkill]);
    EndHudMessageBold(HUDMSG_FADEOUT, 0, "White", 1.5, 0.5, 2.0, 1.0);

    // YOU FOOL!
    if (NewSkill == 5)
    {
        Delay(35 / 2);
        AmbientSound("nightmarecyberdemon/sight", 127);
    }
}

// Respec - Respecialize your Player
NamedScript KeyBind void Respec(bool DoStats, bool DoSkills)
{
    if (Player.Level * 750 > CheckInventory("DRPGCredits"))
    {
        PrintError(StrParam("Not enough credits\n\nCost respec for %d lvl: %d C",Player.Level, (Player.Level * 750)));
        ActivatorSound("menu/error", 127);
        return;
    }

    int Modules;
    int OldCredits;

    // Respec Stats
    if (DoStats)
    {
        // Add stats into a pool of tokens to give back to the player
        for (int i = 0; i <= Player.Strength; i++) Modules += (int)(i * (fixed)MODULE_STAT_MULT * GetCVarFixed("drpg_module_statfactor"));
        for (int i = 0; i <= Player.Defense; i++) Modules += (int)(i * (fixed)MODULE_STAT_MULT * GetCVarFixed("drpg_module_statfactor"));
        for (int i = 0; i <= Player.Vitality; i++) Modules += (int)(i * (fixed)MODULE_STAT_MULT * GetCVarFixed("drpg_module_statfactor"));
        for (int i = 0; i <= Player.Energy; i++) Modules += (int)(i * (fixed)MODULE_STAT_MULT * GetCVarFixed("drpg_module_statfactor"));
        for (int i = 0; i <= Player.Regeneration; i++) Modules += (int)(i * (fixed)MODULE_STAT_MULT * GetCVarFixed("drpg_module_statfactor"));
        for (int i = 0; i <= Player.Agility; i++) Modules += (int)(i * (fixed)MODULE_STAT_MULT * GetCVarFixed("drpg_module_statfactor"));
        for (int i = 0; i <= Player.Capacity; i++) Modules += (int)(i * (fixed)MODULE_STAT_MULT * GetCVarFixed("drpg_module_statfactor"));
        for (int i = 0; i <= Player.Luck; i++) Modules += (int)(i * (fixed)MODULE_STAT_MULT * GetCVarFixed("drpg_module_statfactor"));

        // Compensation for standard Stats (Vitality, Energy and Capacity)
        for (int i = 0; i <= 10; i++) Modules -= (int)(i * (fixed)MODULE_STAT_MULT * GetCVarFixed("drpg_module_statfactor") * 3);

        // Reset Stats
        Player.Strength = 0;
        Player.Defense = 0;
        Player.Vitality = 10;
        Player.Energy = 10;
        Player.Regeneration = 0;
        Player.Agility = 0;
        Player.Capacity = 10;
        Player.Luck = 0;

        // Reset Natural Bonuses
        Player.StrengthNat = 0;
        Player.DefenseNat = 0;
        Player.VitalityNat = 0;
        Player.EnergyNat = 0;
        Player.RegenerationNat = 0;
        Player.AgilityNat = 0;
        Player.CapacityNat = 0;
        Player.LuckNat = 0;

        // Reset Stat XP
        Player.StrengthXP = 0;
        Player.DefenseXP = 0;
        Player.VitalityXP = 0;
        Player.EnergyXP = 0;
        Player.RegenerationXP = 0;
        Player.AgilityXP = 0;
        Player.CapacityXP = 0;
        Player.LuckXP = 0;

        // Reset Shield
        Player.Shield.Charge = 0;
        Player.Shield.Timer = 375;
    }

    // Respec Skills
    if (DoSkills)
    {
        // Reset skills, add skill levels together into a pool of tokens to give back to the player
        for (int i = 0; i < MAX_CATEGORIES; i++)
            for (int j = 0; j < SkillCategoryMax[i]; j++)
                if (!(i == 5 && j == 7)) // Skip Transport
                    while (Player.SkillLevel[i][j].Level > 0)
                    {
                        Modules += (int)(((fixed)Player.SkillLevel[i][j].Level * (fixed)MODULE_SKILL_MULT) * GetCVarFixed("drpg_module_skillfactor"));
                        Player.SkillLevel[i][j].CurrentLevel--;
                        Player.SkillLevel[i][j].Level--;
                    }

        // Remove Auras
        RemoveAura();
    }

    // Give Respecced Modules
    GiveInventory("DRPGModule", Modules);

    // Take Credits
    GiveInventory("DRPGCredits", OldCredits);
    TakeInventory("DRPGCredits", Player.Level * 750);

    // FX
    FadeRange(255, 255, 255, 0.75, 0, 0, 0, 0.0, 2.5);
    SetFont("BIGFONT");
    HudMessage("Respec Complete");
    EndHudMessage(HUDMSG_FADEOUT, 0, "White", 0.5, 0.5, 2.5, 2.5);
    ActivatorSound("misc/secret", 127);
}

NamedScript DECORATE int GetAugBattery()
{
    return Player.Augs.Battery;
}

NamedScript DECORATE int GetAugBatteryMax()
{
    return Player.Augs.BatteryMax;
}

NamedScript DECORATE int GetToxicity()
{
    return Player.Toxicity;
}

NamedScript DECORATE int GetStimPowerupActive()
{
    bool Active = false;

    for (int i = 0; i < STIM_MAX - 1; i++)
        if (Player.Stim.PowerupTimer[i] > 0)
        {
            Active = true;
            break;
        }

    return Active;
}

// Add Toxicity to the Player
NamedScript DECORATE void AddToxicity(int Amount)
{
    int PrevToxicity = Player.Toxicity;

    Player.Toxicity += Amount;
    Player.ToxicTimer = 0;
    Player.ToxicOffset = 0;
    Player.ToxicStage = 0;

    if (Player.Toxicity < 0)
        Player.Toxicity = 0;

    if ((PrevToxicity < 25 && Player.Toxicity >= 25) ||
            (PrevToxicity < 50 && Player.Toxicity >= 50) ||
            (PrevToxicity < 75 && Player.Toxicity >= 75))
        ActivatorSound("misc/toxic", 127);
}

// Add Stim Immunity to the Player
NamedScript DECORATE void AddStimImmunity(int Amount)
{
    Player.StimImmunity += Amount;
    if (Player.StimImmunity > 100)
        Player.StimImmunity = 100;
}

NamedScript DECORATE void ClearBurnout()
{
    AddEP(Abs(Player.EP), false);
}

NamedScript KeyBind void PurgeDrops()
{
    int *TID = (int *)Player.DropTID.Data;
    for (int i = 0; i < Player.DropTID.Position; i++)
    {
        if (ClassifyActor(TID[i]) == ACTOR_NONE)
            continue;

        SpawnSpot("TeleportFog", TID[i], 0, 0);
        Thing_Remove(TID[i]);
    }

    CleanDropTIDArray();
    Print("\CdRemoved \Cgall\Cd monster-dropped items");
}

NamedScript Console void CheckArmorStats()
{
    Log("\C[Orange]-- ARMOR STATS --");
    Log("\C[Green]Class name: \C[White]%S", GetArmorInfoString(ARMORINFO_CLASSNAME));
    Log("\C[Green]Durability: \C[LightBlue]%i%%", GetArmorInfo(ARMORINFO_SAVEAMOUNT));
    Log("\C[Green]Protection: \C[Yellow]%.2k%%", GetArmorInfoFixed(ARMORINFO_SAVEPERCENT) * 100.0);
    Log("\C[Green]Max Absorb: \C[Grey]%i%%", GetArmorInfo(ARMORINFO_MAXABSORB));
    Log("\C[Green]Max Full Absorb: \C[Brick]%i%%", GetArmorInfo(ARMORINFO_MAXFULLABSORB));
}

// Returns whether your stats are all currently capped or not
bool StatsCapped()
{
    return (Player.Strength >= Player.StatCap &&
            Player.Defense >= Player.StatCap &&
            Player.Vitality >= Player.StatCap &&
            Player.Energy >= Player.StatCap &&
            Player.Regeneration >= Player.StatCap &&
            Player.Agility >= Player.StatCap &&
            Player.Capacity >= Player.StatCap &&
            Player.Luck >= Player.StatCap);
}

int AveragePlayerLevel()
{
    int NumPlayers;
    int TotalLevel;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        // Skip player if they're not ingame
        if (!PlayerInGame(i)) continue;

        TotalLevel += Players(i).Level;
        NumPlayers++;
    }

    return TotalLevel / NumPlayers;
}

int AveragePlayerRank()
{
    int NumPlayers;
    int TotalRank;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        // Skip player if they're not ingame
        if (!PlayerInGame(i)) continue;

        TotalRank += Players(i).RankLevel;
        NumPlayers++;
    }

    return TotalRank / NumPlayers;
}

int AveragePlayerCredits()
{
    int NumPlayers;
    int TotalCredits;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        // Skip player if they're not ingame
        if (!PlayerInGame(i)) continue;

        TotalCredits += CheckActorInventory(Players(i).TID, "DRPGCredits");
        NumPlayers++;
    }

    return TotalCredits / NumPlayers;
}

int AveragePlayerLuck()
{
    int NumPlayers;
    int TotalLuck;
    int AverageLuck;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        // Skip player if they're not ingame
        if (!PlayerInGame(i)) continue;

        TotalLuck += Players(i).LuckTotal;
        NumPlayers++;
    }

    if (GetCVar("drpg_levelup_natural"))
        TotalLuck /= 2;

    AverageLuck = TotalLuck / NumPlayers;

    if (AverageLuck > 100)
        AverageLuck = 100;

    return AverageLuck;
}

bool NomadInGame()
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!PlayerInGame(i)) continue;

        if (PlayerClass(i) == 7)
            return true;
    }

    return false;
}

bool HaveStatusEffect()
{
    for (int i = 0; i < SE_MAX; i++)
        if (Player.StatusType[i])
            return true;

    return false;
}

bool DropPlayerItem(str Item)
{
    fixed Angle = GetActorAngle(0);
    fixed X = GetActorX(0) + Cos(Angle);
    fixed Y = GetActorY(0) + Sin(Angle);
    fixed Z = GetActorZ(0) + 16.0;
    fixed XSpeed = Cos(Angle) * 4.0;
    fixed YSpeed = Sin(Angle) * 4.0;
    int TID = UniqueTID();
    bool SpawnOK = Spawn(Item, X, Y, Z, TID, Angle);
    bool SightOK = CheckSight(TID, 0, 0);

    // Remove the item if it fails the sight check
    if (!SightOK)
        Thing_Remove(TID);

    // Apply Velocity
    if (SpawnOK && SightOK)
        SetActorVelocity(TID, XSpeed, YSpeed, 0, false, false);

    return (SpawnOK && SightOK);
}

bool IsPlayerMoving()
{
    if (Player.InMenu)
        return false;
    else
        return CheckInput(BT_FORWARD | BT_BACK | BT_MOVELEFT | BT_MOVERIGHT, KEY_HELD, true, PlayerNumber());
}

int FindPlayerID(int TID)
{
    for (int i = 0; i < MAX_PLAYERS; i++)
        if (Players(i).TID == TID)
            return i;

    return -1;
}

OptionalArgs(1) bool SetActivatorToTargetExtended(int TID, int NumCycles)
{
    if (NumCycles == 0) NumCycles = 31;
    fixed OriginX = GetActorX(TID);
    fixed OriginY = GetActorY(TID);
    fixed OriginZ = GetActorZ(TID);
    fixed OriginAngle = GetActorAngle(TID);
    fixed OriginPitch = GetActorPitch(TID);
    bool GotTarget = false;
    int SecondaryCheckerTID = UniqueTID();

    if (SetActivatorToTarget(TID))
        return true;

    for (int i = 1; i < NumCycles; i++)
    {
        fixed CheckDistance = i * 1024.0;
        fixed NewX = OriginX + (CheckDistance * Cos(OriginAngle) * Cos(-OriginPitch));
        fixed NewY = OriginY + (CheckDistance * Sin(OriginAngle) * Cos(-OriginPitch));
        fixed NewZ = OriginZ + (CheckDistance * Sin(-OriginPitch));

        if (!Spawn("MapSpot", NewX, NewY, NewZ, SecondaryCheckerTID, 0))
        {
            break;
        }

        SetActorAngle(SecondaryCheckerTID, OriginAngle);
        SetActorPitch(SecondaryCheckerTID, OriginPitch);

        if (SetActivator(SecondaryCheckerTID, AAPTR_GET_LINETARGET) && CheckSight(TID, 0, 0))
        {
            GotTarget = true;
            Thing_Remove(SecondaryCheckerTID);
            break;
        }
        else
            SetActivator(TID);

        Thing_Remove(SecondaryCheckerTID);
    }

    return GotTarget;
}

bool IsTimeFrozen()
{
    for (int i = 0; i < MAX_PLAYERS; i++)
        if (CheckActorInventory(Players(i).TID, "PowerTimeFreezer") ||
                CheckActorInventory(Players(i).TID, "PowerShieldTimeFreezer") ||
                CheckActorInventory(Players(i).TID, "PowerRLChronotrooperFreeze")) // DoomRL
            return true;

    return false;
}

// Calculate current map level modifier
fixed MapLevelMod()
{
    fixed LevelNum = CurrentLevel->LevelNum;
    fixed LevelMax = GetCVar("drpg_ws_use_wads") * 32.0;
    fixed MapLevelMod = LevelNum / (LevelMax / (2.0 - (LevelNum / LevelMax)));

    if (MapLevelMod > 0.5)
        MapLevelMod = LevelNum / LevelMax + (0.3 - (0.3 * LevelNum / LevelMax));

    if (CurrentLevel->UACBase)
        MapLevelMod = AveragePlayerLevel() / 100.0;

    if (MapLevelMod > 1.0 || LevelNum / LevelMax > 1.0)
        MapLevelMod = 1.0;

    return MapLevelMod;
}

// Calculate natural stat leveling modifier
fixed StatsNatMod()
{
    int NumPlayers;
    int PlayerLevel;
    int StatsNat;
    fixed Modifier;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        // Skip player if they're not ingame
        if (!PlayerInGame(i)) continue;

        PlayerLevel = Players(i).Level;
        if (PlayerLevel <= 0) PlayerLevel = 1;
        StatsNat = Players(i).StrengthNat + Players(i).DefenseNat + Players(i).VitalityNat + Players(i).EnergyNat + Players(i).RegenerationNat + Players(i).AgilityNat + Players(i).CapacityNat + Players(i).LuckNat;
        Modifier += ((fixed)StatsNat / ((fixed)PlayerLevel * 8.0) > 1.0 ? 1.0 : (fixed)StatsNat / ((fixed)PlayerLevel * 8.0));
        NumPlayers++;
    }

    return (1.0 + (Modifier / (fixed)NumPlayers));
}

// --------------------------------------------------
// Inventory
//

int CheckInventoryTID(int TID, str Item)
{
    return (TID == 0 ? CheckInventory(Item) : CheckActorInventory(TID, Item));
}

int SetInventory(str Item, int Count)
{
    int n = Count - CheckInventory(Item);

    if (n > 0)
        GiveInventory(Item, n);
    else if (n < 0)
        TakeInventory(Item, -n);

    return n;
}

int SetActorInventory(int tid, str item, int count)
{
    int n = count - CheckActorInventory (tid, item);

    if (n > 0)
        GiveActorInventory(tid, item, n);
    else if (n < 0)
        TakeActorInventory(tid, item, -1 * n);

    return n;
}

// Drop your entire inventory on the ground
void DropEntireInventory()
{
    DroppedItem const ItemList[] =
    {
        // Health
        { "Stimpack" },
        { "Medikit" },
        { "LargeMedikit" },
        { "XLMedikit" },
        { "MedPack" },
        { "SurgeryKit" },
        { "Continue" },

        // Armor
        { "GreenArmor" },
        { "UsedGreenArmor" },
        { "BlueArmor" },
        { "UsedBlueArmor" },
        { "YellowArmor" },
        { "UsedYellowArmor" },
        { "RedArmor" },
        { "UsedRedArmor" },
        { "WhiteArmor" },
        { "UsedWhiteArmor" },

        // Powerups
        { "InvulnerabilitySphere" },
        { "InvulnerabilityCharge" },
        { "BlurSphere" },
        { "InvisibilityCharge" },
        { "RegenSphere" },
        { "RadSuit" },
        { "Infrared" },
        { "AllMap" },
        { "Berserk" },
        { "Wings" },
        { "StimSmall" },
        { "StimMedium" },
        { "StimLarge" },

        // DoomRL - Powerups
        { "InvulnerabilitySphere2" },
        { "RadSuit2" },
        { "Infrared2" },
        { "Berserk2" },

        // DoomRL - Modpacks
        { "RLPowerModItem" },
        { "RLBulkModItem" },
        { "RLAgilityModItem" },
        { "RLTechnicalModItem" },
        { "RLSniperModItem" },
        { "RLFirestormModItem" },
        { "RLNanoModItem" },
        { "RLOnyxModItem" },
        { "RLArmorModItem" },
        { "RLArtiModItem" },
        { "RLModLimit", true, true, 4 },
        { "RLScavengerModLimit", true, true, 8 },
        { "RLArmorModItemInInventory", true },

        // End of list
        { NULL }
    };

    for (int i = 0; ItemList[i].Actor != NULL; i++)
        for (int j = 0; j < CheckInventory(ItemList[i].Actor); j++)
        {
            // Limit the drops if you have more than 25 to prevent massive amounts of lag
            if (j >= 25) break;

            if (ItemList[i].TakeAll)
                TakeInventory(ItemList[i].Actor, ItemList[i].Max);
            else
                TakeInventory(ItemList[i].Actor, 1);

            if (!ItemList[i].NoDrop)
                DropItem(0, ItemList[i].Actor, 1, 255);
        }
}

// --------------------------------------------------
// Items
//

// Return the amount of ammo corresponding to an ammo pickup
int GetAmmoAmount(str Item)
{
    int Amount = 1;

    if (Item == "Clip")         Amount = 10;
    if (Item == "ClipBox")      Amount = 50;
    if (Item == "Shell")        Amount = 4;
    if (Item == "ShellBox")     Amount = 20;
    if (Item == "RocketAmmo")   Amount = 1;
    if (Item == "RocketBox")    Amount = 5;
    if (Item == "Cell")         Amount = 20;
    if (Item == "CellPack")     Amount = 100;

    return Amount;
}

// --------------------------------------------------
// Health
//

NamedScript DECORATE int GetHealthPercent(int Percent)
{
    return (int)(Player.ActualHealth * Percent / Player.HealthMax);
}

// Returns true if the player's health is below the specified percentage of max health
NamedScript DECORATE int HealthBelowPercent(int Percent)
{
    return (int)(Player.ActualHealth <= Player.HealthMax * Percent / 100);
}

// Returns your max Health (for DECORATE)
NamedScript DECORATE int GetHealthMax()
{
    return Player.HealthMax;
}

// Used by DECORATE in UseShield state to cap healing items to only heal to maximum Health
NamedScript DECORATE void CapHealthItem()
{
    if (Player.ActualHealth > Player.HealthMax)
        Player.ActualHealth = Player.HealthMax;
}

// --------------------------------------------------
// Shield
//

// Returns the active state of your Shield
NamedScript DECORATE bool ShieldActive()
{
    return Player.Shield.Active;
}

// Returns the current charge of your Shield
NamedScript DECORATE int GetShield()
{
    return Player.Shield.Charge;
}

// Returns the capacity of your current Shield (for DECORATE)
NamedScript DECORATE int GetShieldMax()
{
    return Player.Shield.Capacity;
}

// Used by DECORATE to check if your Shield's stored Health is at max
NamedScript DECORATE bool ShieldHealthMax()
{
    return (Player.ActualHealth >= Player.HealthMax);
}

// --------------------------------------------------
// EP
//

NamedScript DECORATE void AddEP(int Amount, bool NoFlash)
{
    if (Player.EP + Amount > Player.EPMax)
        Amount = Player.EPMax - Player.EP;

    if (Amount <= 0)
        return;

    fixed FlashStrength = 0.01 * (Amount * 100 / Player.EPMax);
    fixed FlashDuration = 0.03 * (Amount * 100 / Player.EPMax);

    if (FlashStrength > 0.5)
        FlashStrength = 0.5;

    if (FlashDuration > 3.0)
        FlashDuration = 3.0;

    if (!NoFlash)
        FadeRange(0, 255, 255, FlashStrength, 0, 255, 255, 0, FlashDuration);

    Player.EP += Amount;
}

NamedScript DECORATE OptionalArgs(1) void RemoveEP(int Amount, int InUnits)
{
    if (InUnits)
        Player.EP -= Amount;
    else
        Player.EP -= ((Player.EP * Amount) / 100);
}

// Returns your current EP (for DECORATE)
NamedScript DECORATE int GetEP()
{
    return Player.EP;
}

// Gets your current max EP (for DECORATE)
NamedScript DECORATE int GetEPMax()
{
    return Player.EPMax;
}

// --------------------------------------------------
// Skills
//

NamedScript KeyBind void PlayerTeleport(int PlayerNum)
{
    bool NearPlayers = true;

    if (PlayerNum == PlayerNumber() || !PlayerInGame(PlayerNum))
    {
        PrintError("Not a valid player");
        ActivatorSound("menu/error", 127);
        return;
    }

    int Target = Players(PlayerNum).ActualHealth > 0 ? Players(PlayerNum).TID : Players(PlayerNum).BodyTID > 0 ? Players(PlayerNum).BodyTID : Player.TID;
    // Teleport
    GiveInventory("DRPGPlayerTeleportGhost", 1);
    SetActorPosition(Player.TID, GetActorX(Target), GetActorY(Target), GetActorZ(Target), true);
    SetActorAngle(Player.TID, GetActorAngle(Target));
    SetActorProperty(Player.TID, APROP_RenderStyle, STYLE_Translucent);

    while (NearPlayers)
    {
        // Reset the flag
        NearPlayers = false;

        // Make sure you're not inside or near any other players before we solidify
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (i == PlayerNumber() || !PlayerInGame(i)) continue;
            if (Distance(Player.TID, Players(i).TID) <= GetActorPropertyFixed(Players(i).TID, APROP_Radius) * 4.0)
            {
                NearPlayers = true;
                break;
            }
        }

        // Player Flashyness
        SetActorPropertyFixed(Player.TID, APROP_Alpha, 0.75 + (Sin(Timer() / 32.0) * 0.25));

        // Solidify
        if (!NearPlayers)
        {
            GiveInventory("DRPGPlayerTeleportNormal", 1);
            SetActorProperty(Player.TID, APROP_RenderStyle, STYLE_Normal);
            SetActorPropertyFixed(Player.TID, APROP_Alpha, 1.0);
        }

        Delay(1);
    }
}

// Fires a projectile from the Player
bool FireProjectile(str Type)
{
    fixed X = GetActorX(0);
    fixed Y = GetActorY(0);
    fixed Z = GetActorZ(0);
    fixed Pitch = GetActorPitch(0);
    fixed Angle = GetActorAngle(0);
    fixed XSpeed = Cos(Angle) * 16.0;
    fixed YSpeed = Sin(Angle) * 16.0;
    fixed ZSpeed = -Sin(Pitch) * 16.0;
    fixed FireHeight = GetActorViewHeight(0) * 0.8;
    int TID = UniqueTID();

    SpawnProjectile(0, Type, 0, 0, 0, 0, TID);
    SetActorVelocity(TID, XSpeed, YSpeed, ZSpeed, 0, 0);
    SetActorAngle(TID, Angle);
    SetActorPosition(TID, X, Y, Z + FireHeight, 0);
    Thing_ChangeTID(TID, 0);

    return true;
}

// Emits a Nova from the Player
bool Nova(str Type, int Projectiles)
{
    fixed X = GetActorX(0);
    fixed Y = GetActorY(0);
    fixed Z = GetActorZ(0);
    fixed Pitch = GetActorPitch(0);
    fixed Angle = GetActorAngle(0);
    fixed XSpeed = Cos(Angle) * 16.0;
    fixed YSpeed = Sin(Angle) * 16.0;
    fixed ZSpeed = -Sin(Pitch) * 16.0;
    fixed FireHeight = GetActorViewHeight(0) * 0.8;
    int TID = UniqueTID();
    fixed AngleAdd;

    AngleAdd = 1.0 / Projectiles;

    for (int i = 0; i < Projectiles; i++)
    {
        XSpeed = Cos(Angle) * 16.0;
        YSpeed = Sin(Angle) * 16.0;
        ZSpeed = -Sin(Pitch) * 16.0;

        SpawnProjectile(0, Type, 0, 0, 0, 0, TID);
        SetActorVelocity(TID, XSpeed, YSpeed, ZSpeed, 0, 0);
        SetActorAngle(TID, Angle);
        SetActorPosition(TID, X, Y, Z + FireHeight, 0);
        Thing_ChangeTID(TID, 0);

        Angle += AngleAdd;
    }

    return true;
}

// --------------------------------------------------
// Auras
//

NamedScript DECORATE void GetAuraTokens()
{
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];
    AuraInfo RPGMap *Aura = &Stats->Aura;

    if (Aura->Type[AURA_RED].Active)
        SetInventory("DRPGRedAuraToken", 1);
    if (Aura->Type[AURA_GREEN].Active)
        SetInventory("DRPGGreenAuraToken", 1);
    if (Aura->Type[AURA_WHITE].Active)
        SetInventory("DRPGWhiteAuraToken", 1);
    if (Aura->Type[AURA_PINK].Active)
        SetInventory("DRPGPinkAuraToken", 1);
    if (Aura->Type[AURA_BLUE].Active)
        SetInventory("DRPGBlueAuraToken", 1);
    if (Aura->Type[AURA_PURPLE].Active)
        SetInventory("DRPGPurpleAuraToken", 1);
    if (Aura->Type[AURA_ORANGE].Active)
        SetInventory("DRPGOrangeAuraToken", 1);
    if (Aura->Type[AURA_DARKBLUE].Active)
        SetInventory("DRPGDarkBlueAuraToken", 1);
    if (Aura->Type[AURA_YELLOW].Active)
        SetInventory("DRPGYellowAuraToken", 1);
    if (MonsterHasShadowAura(Stats))
        SetInventory("DRPGShadowAuraToken", 1);
}

// Spawn the respective Aura effects on the given Player/Monster's TID
void SpawnAuras(int TID, bool ForceFancy)
{
    str const AuraActors[AURA_MAX] =
    {
        "DRPGRedAura",
        "DRPGGreenAura",
        "DRPGWhiteAura",
        "DRPGPinkAura",
        "DRPGCyanAura",
        "DRPGPurpleAura",
        "DRPGOrangeAura",
        "DRPGBlueAura",
        "DRPGYellowAura"
    };

    fixed X = GetActorX(TID);
    fixed Y = GetActorY(TID);
    fixed Z = GetActorZ(TID);
    fixed Radius = GetActorPropertyFixed(TID, APROP_Radius);
    fixed Height = GetActorPropertyFixed(TID, APROP_Height);
    int Angle = GetActorAngle(TID) * 256;
    bool SpawnOK = false;
    bool IsPlayer = (ClassifyActor(TID) & ACTOR_PLAYER);
    int SimpleType = GetCVar("drpg_simple_auras");
    bool Simple = (IsPlayer ? (SimpleType == 1 || SimpleType == 3) : (SimpleType == 2 || SimpleType == 3));
    AuraInfo *PlayerAura = &Players(FindPlayerID(TID)).Aura;
    AuraInfo RPGMap *MonsterAura = &Monsters[GetMonsterID(TID)].Aura;
    int AuraCount = 0;
    int AuraAdd = 0;

    // WHYYYYYYYYYYYYYYYYYYYYYYY.YPEG
    if (IsPlayer)
    {
        // Calculate number of active Auras
        for (int i = 0; i < AURA_MAX; i++)
            if (PlayerAura->Type[i].Active)
                AuraCount++;

        // Spawn the Aura
        if (AuraCount >= AURA_MAX) // Shadow
        {
            if (!Simple)
                SpawnForced("DRPGBlackAura", X, Y, Z + Height / 2.0, AuraTID, Angle);
            else
                SpawnForced("DRPGBlackAuraIndicator", X, Y, Z + Height + 12.0 + Sin(Timer() / 64.0) * 4.0, 0, Angle);
        }
        else // Normal
        {
            for (int i = 0; i < AURA_MAX; i++)
            {
                if (PlayerAura->Type[i].Active)
                {
                    // Create rotation for simple Aura icons
                    if (Simple && AuraCount > 1)
                    {
                        fixed AngleOffset = (1.0 / (fixed)AuraCount) * ((fixed)AuraAdd++ * 128.0);
                        X = GetActorX(TID) + Sin(((fixed)Timer() + AngleOffset) / 128.0) * (Radius * 2.0);
                        Y = GetActorY(TID) + Cos(((fixed)Timer() + AngleOffset) / 128.0) * (Radius * 2.0);
                        Z = GetActorZ(TID) + 4.0 + Sin(Timer() / 64.0) * 4.0;
                    };

                    // Spawn
                    if (!Simple)
                        SpawnForced(AuraActors[i], X, Y, Z + Height / 2.0, AuraTID, Angle);
                    else
                    {
                        if (GetCVar("drpg_players_aura_icons_disable"))
                            return;
                        SpawnForced(StrParam("%SIndicator", AuraActors[i]), X, Y, Z + Height + 8.0, 0, Angle);
                    }
                }
            }
        }
    }
    else // Monster
    {
        // Calculate number of active Auras
        for (int i = 0; i < AURA_MAX; i++)
            if (MonsterAura->Type[i].Active)
                AuraCount++;

        // Return if the monster is stealthy and NOT shadow
        if (IsStealth(TID) && AuraCount < AURA_MAX)
            return;

        // LOS Checks
        if (GameType() == GAME_SINGLE_PLAYER)
        {
            if (CheckSight(Players(0).TID, TID, CSF_NOBLOCKALL))
                SpawnOK = true;
        }
        else
        {
            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (!PlayerInGame(i))
                    continue;

                if (CheckSight(Players(i).TID, TID, CSF_NOBLOCKALL))
                {
                    SpawnOK = true;
                    break;
                }
            }
        }

        // Return if LOS checks failed
        if (!SpawnOK && !InTitle)
            return;

        // Spawn the Aura
        if (GetActorProperty(TID, APROP_Friendly)) // Friendly
        {
            if (!Simple)
                SpawnForced("DRPGFriendlyAura", X, Y, Z + 32.0, 0, Angle);
            else
                SpawnForced("DRPGFriendlyIndicator", X, Y, Z + Height + 12.0 + Sin(Timer() / 64.0) * 4.0, 0, Angle);
        }
        else if (AuraCount >= AURA_MAX) // Shadow
        {
            if (!Simple)
                SpawnForced("DRPGBlackAura", X, Y, Z + Height / 2.0, AuraTID, Angle);
            else
                SpawnForced("DRPGBlackAuraIndicator", X, Y, Z + Height + 12.0 + Sin(Timer() / 64.0) * 4.0, 0, Angle);
        }
        else // Normal
        {
            for (int i = 0; i < AURA_MAX; i++)
                if (MonsterAura->Type[i].Active)
                {
                    // Create rotation for simple Aura icons
                    if (Simple && AuraCount > 1)
                    {
                        fixed AngleOffset = (1.0 / (fixed)AuraCount) * ((fixed)AuraAdd++ * 128.0);
                        X = GetActorX(TID) + Sin(((fixed)Timer() + AngleOffset) / 128.0) * (Radius * 2.0);
                        Y = GetActorY(TID) + Cos(((fixed)Timer() + AngleOffset) / 128.0) * (Radius * 2.0);
                        Z = GetActorZ(TID) + 4.0 + Sin(Timer() / 64.0) * 4.0;
                    }

                    // Spawn
                    if (!Simple)
                        SpawnForced(AuraActors[i], X, Y, Z + Height / 2.0, AuraTID, Angle);
                    else
                        SpawnForced(StrParam("%SIndicator", AuraActors[i]), X, Y, Z + Height + 8.0, 0, Angle);
                }
        }
    }

    // Pass Radius and Height to the Auras for DECORATE usage
    if (!Simple)
    {
        SetUserVariable(AuraTID, "user_radius", (int)Radius);
        SetUserVariable(AuraTID, "user_height", (int)Height);
    }

    // Remove the TID
    Thing_ChangeTID(AuraTID, 0);
}

// Returns whether a monster is in some form of stealth or not
bool IsStealth(int tid)
{
    return
        (
            // Fuzzy
            GetActorProperty(tid, APROP_RenderStyle) == STYLE_Fuzzy ||
            GetActorProperty(tid, APROP_RenderStyle) == STYLE_OptFuzzy ||
            // Translucent
            (GetActorProperty(tid, APROP_RenderStyle) == STYLE_Translucent &&
             GetActorPropertyFixed(tid, APROP_Alpha) < 0.5) ||
            (GetActorProperty(tid, APROP_RenderStyle) == STYLE_TranslucentStencil &&
             GetActorPropertyFixed(tid, APROP_Alpha) < 0.5) ||
            (GetActorProperty(tid, APROP_RenderStyle) == STYLE_Add &&
             GetActorPropertyFixed(tid, APROP_Alpha) < 0.5) ||
            // Totally invisible
            GetActorProperty(tid, APROP_RenderStyle) == STYLE_None ||
            // Actor flags
            // These two cases are already handled by the RenderStyle checks above
            // CheckFlag(tid, "STEALTH") ||
            // CheckFlag(tid, "SHADOW") ||
            CheckFlag(tid, "INVISIBLE")
        );
}

bool PlayerHasAura(int PlayerNum)
{
    bool HaveAura = false;

    for (int i = 0; i < AURA_MAX; i++)
        if (Players(PlayerNum).Aura.Type[i].Active)
        {
            HaveAura = true;
            break;
        };

    return HaveAura;
}

bool MonsterHasAura(MonsterStatsPtr Stats)
{
    bool HaveAura = false;

    for (int i = 0; i < AURA_MAX; i++)
        if (Stats->Aura.Type[i].Active)
        {
            HaveAura = true;
            break;
        }

    return HaveAura;
}

bool PlayerHasShadowAura(int PlayerNum)
{
    int AuraCount;

    for (int i = 0; i < AURA_MAX; i++)
        if (Player.Aura.Type[i].Active)
            AuraCount++;

    if (AuraCount >= AURA_MAX)
        return true;

    return false;
}

bool MonsterHasShadowAura(MonsterStatsPtr Stats)
{
    int AuraCount;

    for (int i = 0; i < AURA_MAX; i++)
        if (Stats->Aura.Type[i].Active)
            AuraCount++;

    if (AuraCount >= AURA_MAX)
        return true;

    return false;
}

// --------------------------------------------------
// Drawing
//

int HudMessage(str Format, ...)
{
    va_list Args;
    int NumChars;
    va_start (Args, Format);

    BeginPrint ();
    NumChars = __vnprintf_str (Format, Args);

    va_end (Args);
    return NumChars;
}

OptionalArgs(3) void EndHudMessageSelect(bool bold, int flags, int id, str color, fixed x, fixed y, fixed holdtime, fixed opt1, fixed opt2, fixed opt3)
{
    MoreHudMessage ();
    OptHudMessageS (flags | HUDMSG_COLORSTRING, id, color, x, y, holdtime);
    if (!bold)
        EndHudMessage4 (opt1, opt2, opt3);
    else
        EndHudMessageBold4 (opt1, opt2, opt3);
}

str StrParam(str Format, ...)
{
    va_list Args;
    va_start (Args, Format);

    BeginStrParam();
    __vnprintf_str(Format, Args);

    va_end (Args);
    return EndStrParam();
}

str PlayerName(int n)
{
    BeginPrint();
    PrintName(n);
    return EndStrParam();
}

NamedScript void PrintTextWiggle(char *Text, int ID, int Color, int X, int Y, fixed HoldTime, fixed Speed, fixed Spacing, fixed Radius)
{
    int Time = (int)(HoldTime * 35.0);
    int TimeMax = Time;

    SetHudSize(640, 480, false);

    while (Time > 0)
    {
        for (int i = 0; Text[i] != NULL; i++)
        {
            fixed XOff = (fixed)X + (Spacing * i);
            fixed YOff = (fixed)Y + Sin((Timer() + i) / Speed) * Radius;
            HudMessage("%c", Text[i]);
            EndHudMessage(HUDMSG_PLAIN | HUDMSG_ALPHA, ID + i, ColorNames[Color], (int)XOff, (int)YOff, 0.05, ((fixed)Time / (fixed)TimeMax));
        };

        Time--;
        Delay(1);
    }
}

NamedScript void DrawStatUp(int Stat)
{
    str const StatNames[STAT_MAX] =
    {
        "\CgStrength",
        "\CdDefense",
        "\CaVitality",
        "\CnEnergy",
        "\CtRegen",
        "\CiAgility",
        "\ChCapacity",
        "\CfLuck"
    };

    int StatAmount[STAT_MAX] =
    {
        Player.StrengthNat,
        Player.DefenseNat,
        Player.VitalityNat,
        Player.EnergyNat,
        Player.RegenerationNat,
        Player.AgilityNat,
        Player.CapacityNat,
        Player.LuckNat
    };

    // Log
    if (InMultiplayer)
        Log("%tS\C-'s %S\C- bonus has increased to \C%c%d", PlayerNumber() + 1, StatNames[Stat], StatNames[Stat][1], StatAmount[Stat]);
    else
        Log("Your %S\C- bonus has increased to \C%c%d", StatNames[Stat], StatNames[Stat][1], StatAmount[Stat]);

    // Fade
    switch (Stat)
    {
    case STAT_STRENGTH:
        FadeRange(255, 0, 0, 0.25, 255, 0, 0, 0.0, 0.5);
        break;
    case STAT_DEFENSE:
        FadeRange(0, 255, 0, 0.25, 0, 255, 0, 0.0, 0.5);
        break;
    case STAT_VITALITY:
        FadeRange(255, 0, 255, 0.25, 255, 0, 255, 0.0, 0.5);
        break;
    case STAT_ENERGY:
        FadeRange(0, 255, 255, 0.25, 0, 255, 255, 0.0, 0.5);
        break;
    case STAT_REGENERATION:
        FadeRange(128, 0, 128, 0.25, 128, 0, 128, 0.0, 0.5);
        break;
    case STAT_AGILITY:
        FadeRange(255, 128, 0, 0.25, 255, 128, 0, 0.0, 0.5);
        break;
    case STAT_CAPACITY:
        FadeRange(0, 0, 255, 0.25, 0, 0, 255, 0.0, 0.5);
        break;
    case STAT_LUCK:
        FadeRange(255, 255, 0, 0.25, 255, 255, 0, 0.0, 0.5);
        break;
    }

    fixed TextX = 0.1 + ((Stat % 4) * 0.25);
    fixed TextY = 0.75 + (Stat >= 4 ? 0.05 : 0);

    SetHudSize(0, 0, false);
    SetFont("BIGFONT");

    HudMessage("%S +", StatNames[Stat]);
    EndHudMessage(HUDMSG_FADEOUT, 0, "White", TextX, TextY, 3.0, 2.0);
}

// PrintSprite Utility Functions
void PrintSpriteFade(str Sprite, int ID, fixed X, fixed Y, fixed DelayTime, fixed HoldTime)
{
    SetFont(Sprite);
    HudMessage("A");
    EndHudMessage(HUDMSG_FADEOUT, ID, "Untranslated", X, Y, DelayTime, HoldTime);
}

void PrintSpriteAlpha(str Sprite, int ID, fixed X, fixed Y, fixed DelayTime, fixed Alpha)
{
    SetFont(Sprite);
    HudMessage("A");
    EndHudMessage(HUDMSG_PLAIN | HUDMSG_ALPHA, ID, "Untranslated", X, Y, DelayTime, Alpha);
}

void PrintSpritePulse(str Sprite, int ID, fixed X, fixed Y, fixed Alpha, fixed Speed, fixed Radius, bool AddBlend)
{
    SetFont(Sprite);

    if (AddBlend)
    {
        HudMessage("A");
        EndHudMessage(HUDMSG_PLAIN | HUDMSG_ALPHA | HUDMSG_ADDBLEND, ID, "Untranslated", X, Y, 0.05, Alpha + (Sin((fixed)Timer() / Speed) * Radius));
    }
    else
    {
        HudMessage("A");
        EndHudMessage(HUDMSG_PLAIN | HUDMSG_ALPHA, ID, "Untranslated", X, Y, 0.05, Alpha + (Sin((fixed)Timer() / Speed) * Radius));
    }
}

void PrintMessage(str Message, int ID, fixed Offset)
{
    int Width = GetActivatorCVar("drpg_hud_width");
    int Height = GetActivatorCVar("drpg_hud_height");

    SetHudSize(Width, Height, false);
    SetFont("BIGFONT");
    HudMessage("%S", Message);
    EndHudMessage(HUDMSG_FADEOUT | HUDMSG_LOG, ID, "White", Width / 2 + 0.4, Height / 2 + Offset, 3.0, 2.0);
}

// Should use ERROR_ID as it's ID, but it doesn't work and just layers it wherever the fuck it wants, specifically UNDERNEATH EVERYTHING WHERE IT SHOULDN'T BE FHGJDKHJLKDAFHLKJGH
void PrintError(str Message)
{
    int Width = GetActivatorCVar("drpg_hud_width");
    int Height = GetActivatorCVar("drpg_hud_height");

    SetHudSize(Width, Height, false);
    SetFont("BIGFONT");
    HudMessage("%S", Message);
    EndHudMessage(HUDMSG_FADEOUT | HUDMSG_LOG, 0, "Red", Width / 2 + 0.4, Height / 2 + 0.4, 2.0, 1.0);
}

void DrawBattery()
{
    fixed Width = GetActivatorCVar("drpg_hud_width");
    fixed Height = GetActivatorCVar("drpg_hud_height");
    fixed X = GetActivatorCVar("drpg_aug_x");
    fixed Y = GetActivatorCVar("drpg_aug_y");
    int HoldTime = (GetActivatorCVar("drpg_aug_showbattery") ? 0.05 : 3.0);
    int FadeTime = (GetActivatorCVar("drpg_aug_showbattery") ? 0.05 : 2.0);

    SetHudSize(Width, Height, false);
    PrintSpriteFade("AUGBATT", BATTERY_ID, X + 0.4, Y + 0.4, HoldTime, FadeTime);
    SetFont("BIGFONT");
    if (Player.Augs.Battery < 0)
        HudMessage("%d%%", 0);
    else
        HudMessage("%d%%", (int)Player.Augs.Battery);
    EndHudMessage(HUDMSG_FADEOUT, BATTERY_ID + 1, "Yellow", X + 24.0, Y - 10.0, HoldTime, FadeTime);
}

void DrawShieldInfo(int ID, fixed X, fixed Y, int DrawID)
{
    PlayerData *CurrentPlayer = (ID == -1 ? &Player : &Players(ID));

    if (CurrentPlayer->Shield.Body != 0 || CurrentPlayer->Shield.Battery != 0 || CurrentPlayer->Shield.Capacitor != 0)
    {
        // Epic Shield Name
        str ShieldName = "";
        if (CurrentPlayer->Shield.Capacitor) ShieldName = StrParam("%S %S", ShieldName, CurrentPlayer->Shield.Capacitor->Name);
        if (CurrentPlayer->Shield.Battery)   ShieldName = StrParam("%S \C-%S", ShieldName, CurrentPlayer->Shield.Battery->Name);
        if (CurrentPlayer->Shield.Body)      ShieldName = StrParam("%S \C-%S", ShieldName, CurrentPlayer->Shield.Body->Name);
        if (CurrentPlayer->Shield.Accessory) ShieldName = StrParam("%S \C-%S", ShieldName, CurrentPlayer->Shield.Accessory->Name);
        ShieldName = StrParam("%S\C- Shield", ShieldName);

        SetFont("SMALLFONT");

        // Shield Name
        HudMessage("%S", ShieldName);
        EndHudMessage(HUDMSG_PLAIN, (DrawID == 0 ? 0 : DrawID++), "White", X, Y, 0.05);

        // Shield Stats
        HudMessage(" \CvCapacity: %d / %d", CurrentPlayer->Shield.Charge, CurrentPlayer->Shield.Capacity);
        EndHudMessage(HUDMSG_PLAIN, (DrawID == 0 ? 0 : DrawID++), "White", X, Y + 15.0, 0.05);
        if (CurrentPlayer->Shield.Accessory && CurrentPlayer->Shield.Accessory->PassiveEffect == SHIELD_PASS_KILLSCHARGE)
        {
            HudMessage(" \CgDoes not recharge automatically");
            EndHudMessage(HUDMSG_PLAIN, (DrawID == 0 ? 0 : DrawID++), "White", X, Y + 15.0, 0.05);
        }
        else
        {
            HudMessage(" \CdCharge: %d", CurrentPlayer->Shield.ChargeRate);
            EndHudMessage(HUDMSG_PLAIN, (DrawID == 0 ? 0 : DrawID++), "White", X, Y + 30.0, 0.05);
            HudMessage(" \CaDelay: %.2k", CurrentPlayer->Shield.DelayRate);
            EndHudMessage(HUDMSG_PLAIN, (DrawID == 0 ? 0 : DrawID++), "White", X, Y + 45.0, 0.05);
        }

        // Draw Shield Model
        DrawShieldModel(ID, X + 15.0, Y + 25.0, DrawID);
    }
}

void DrawShieldModel(int ID, fixed X, fixed Y, int DrawID)
{
    PlayerData *CurrentPlayer = (ID == -1 ? &Player : &Players(ID));

    // [KS] These need to be reversed because 0 draws behind the last graphic printed

    // Accessory
    if (CurrentPlayer->Shield.Accessory)
        PrintSprite(CurrentPlayer->Shield.Accessory->Icon, (DrawID == 0 ? 0 : DrawID++), X + 0.1, Y + 0.1, 0.05);

    // Battery
    if (CurrentPlayer->Shield.Battery)
        PrintSprite(CurrentPlayer->Shield.Battery->Icon, (DrawID == 0 ? 0 : DrawID++), X + 0.1, Y + 0.1, 0.05);

    // Capacitor
    if (CurrentPlayer->Shield.Capacitor)
        PrintSprite(CurrentPlayer->Shield.Capacitor->Icon, (DrawID == 0 ? 0 : DrawID++), X + 0.1, Y + 0.1, 0.05);

    // Body
    if (CurrentPlayer->Shield.Body)
        PrintSprite(CurrentPlayer->Shield.Body->Icon, (DrawID == 0 ? 0 : DrawID++), X + 0.1, Y + 0.1, 0.05);
}

void DrawMissionInfo(MissionInfo *Mission, fixed X, fixed Y, bool Active)
{
    // Header
    SetFont("BIGFONT");
    HudMessage("- MISSION INFO -");
    EndHudMessage(HUDMSG_PLAIN, 0, "Grey", X + 0.1, Y, 0.05);
    HudMessage("- REWARD -");
    EndHudMessage(HUDMSG_PLAIN, 0, "Grey", X + 250.1, Y, 0.05);

    // Mission Info
    SetFont("SMALLFONT");
    HudMessage("Type: %S", MissionTypes[Mission->Type]);
    EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 16.0, 0.05);
    HudMessage("Difficulty: %S", MissionDifficulties[Mission->Difficulty]);
    EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 32.0, 0.05);
    // Reward
    HudMessage("XP: %lu", Mission->RewardXP);
    EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 250.1, Y + 16.0, 0.05);
    HudMessage("Rank: %lu", Mission->RewardRank);
    EndHudMessage(HUDMSG_PLAIN, 0, "Yellow", X + 250.1, Y + 32.0, 0.05);
    HudMessage("Credits: %d", Mission->RewardCredits);
    EndHudMessage(HUDMSG_PLAIN, 0, "Gold", X + 250.1, Y + 48.0, 0.05);
    HudMessage("Modules: %d", Mission->RewardModules);
    EndHudMessage(HUDMSG_PLAIN, 0, "Green", X + 250.1, Y + 64.0, 0.05);
    HudMessage("\CiItem: \C-%S", Mission->RewardItem->Name);
    EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 250.1, Y + 80.0, 0.05);

    // Mission Objective
    switch (Mission->Type)
    {
    case MT_COLLECT:
        HudMessage("Item: \Ci%S", Mission->Item->Name);
        EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 48.0, 0.05);
        if (Active)
        {
            HudMessage("Amount: \Cd%d / %d", Mission->Current, Mission->Amount);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 64.0, 0.05);
        }
        else
        {
            HudMessage("Amount: \Cd%d", Mission->Amount);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 64.0, 0.05);
            HudMessage("Carrying: \Cd%d", CheckInventory(Mission->Item->Actor));
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 80.0, 0.05);
        }
        break;
    case MT_KILL:
        HudMessage("Target: \Cg%S", Mission->Monster->Name);
        EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 48.0, 0.05);
        if (Active)
        {
            HudMessage("Amount: \Ca%d / %d", Mission->Current, Mission->Amount);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 64.0, 0.05);
        }
        else
        {
            HudMessage("Amount: \Ca%d", Mission->Amount);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 64.0, 0.05);
        }
        break;
    case MT_KILLAURAS:
    case MT_REINFORCEMENTS:
        if (Active)
        {
            HudMessage("Amount: \Ca%d / %d", Mission->Current, Mission->Amount);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 48.0, 0.05);
        }
        else
        {
            HudMessage("Amount: \Ca%d", Mission->Amount);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 48.0, 0.05);
        }
        break;
    case MT_ASSASSINATION:
        HudMessage("Target: \Cg%S", Mission->Monster->Name);
        EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 48.0, 0.05);
        break;
    case MT_SECRETS:
        if (Active)
        {
            HudMessage("Amount: \Ck%d / %d", Mission->Current, Mission->Amount);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 48.0, 0.05);
        }
        else
        {
            HudMessage("Amount: \Ck%d", Mission->Amount);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 48.0, 0.05);
        }
        break;
    case MT_ITEMS:
        if (Active)
        {
            HudMessage("Amount: \Cn%d / %d", Mission->Current, Mission->Amount);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 48.0, 0.05);
        }
        else
        {
            HudMessage("Amount: \Cn%d", Mission->Amount);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 48.0, 0.05);
        }
        break;
    case MT_COMBO:
        if (Active)
        {
            HudMessage("Amount: \Ct%d / %d", Mission->Current, Mission->Amount);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 48.0, 0.05);
        }
        else
        {
            HudMessage("Amount: \Ct%d", Mission->Amount);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 48.0, 0.05);
        }
        break;
    }
}

OptionalArgs(1) void DrawBar(str Fill, int X, int Y, int Amount, bool Pulse)
{
    if (GetCVar("drpg_toaster"))
        Pulse = false;

    for (int i = 0; i < Amount; i++)
        if (Pulse)
            PrintSpriteAlpha(Fill, 0, X + 0.1 + (i * 1.0), Y, 0.05, 0.75 + Sin((Timer() + i) / 32.0) * 0.25);
        else
            PrintSprite(Fill, 0, X + 0.1 + (i * 1.0), Y, 0.05);
}

void DrawProgressBar(str Message, int Percent)
{
    SetHudSize(320, 240, false);
    SetFont("SMALLFONT");
    HudMessage("%S: %d%%", Message, Percent);
    EndHudMessage(HUDMSG_PLAIN, 0, "Green", 160.4, 200.4, 0.05);
    SetHudClipRect(60, 200 - 6, (int)((fixed)Percent * 2.02), 200 + 6);
    PrintSpritePulse("FillBar", 0, 160.4, 200.4, 0.75, 32.0, 0.25, true);
    SetHudClipRect(0, 0, 0, 0);
}

void DrawBorder(str Prefix, int StartID, int BorderSize, int X, int Y, int Width, int Height)
{
    // Border corners
    PrintSprite(StrParam("%STL", Prefix), (StartID == 0 ? 0 : StartID++), X + 0.1, Y + 0.1, 0.03);
    PrintSprite(StrParam("%STR", Prefix), (StartID == 0 ? 0 : StartID++), X + Width + 0.2, Y + 0.1, 0.03);
    PrintSprite(StrParam("%SBL", Prefix), (StartID == 0 ? 0 : StartID++), X + 0.1, Y + Height + 0.2, 0.03);
    PrintSprite(StrParam("%SBR", Prefix), (StartID == 0 ? 0 : StartID++), X + Width + 0.2, Y + Height + 0.2, 0.03);

    // Border sides
    SetHudClipRect(X, Y + BorderSize, BorderSize, Height - (BorderSize * 2));
    PrintSprite(StrParam("%SL", Prefix), (StartID == 0 ? 0 : StartID++), X + 0.1, Y + 0.1, 0.03);
    SetHudClipRect(X + Width - BorderSize, Y + BorderSize, BorderSize, Height - (BorderSize * 2));
    PrintSprite(StrParam("%SR", Prefix), (StartID == 0 ? 0 : StartID++), X + Width + 0.2, Y + 0.1, 0.03);
    SetHudClipRect(X + BorderSize, Y, Width - (BorderSize * 2), BorderSize);
    PrintSprite(StrParam("%ST", Prefix), (StartID == 0 ? 0 : StartID++), X + BorderSize + 0.1, Y + 0.1, 0.03);
    SetHudClipRect(X + BorderSize, Y + Height - BorderSize, Width - (BorderSize * 2), BorderSize);
    PrintSprite(StrParam("%SB", Prefix), (StartID == 0 ? 0 : StartID), X + BorderSize + 0.1, Y + Height + 0.2, 0.03);
    SetHudClipRect(0, 0, 0, 0);
}

// --------------------------------------------------
// Compatibility/Extensions
//

void RemoveDRLAItem(int Category, int Index)
{
    str ItemActor = ItemData[Category][Index].Actor;

    if (Category == 0) // Weapons
    {
        unsigned char CompatMods = ItemData[Category][Index].CompatMods;
        // Wipe the modpacks off the weapon
        SetInventory(StrParam("%SModLimit", ItemActor), 0);
        if (CompatMods & RL_POWER_MOD)
            SetInventory(StrParam("%SPowerMod", ItemActor), 0);
        if (CompatMods & RL_BULK_MOD)
            SetInventory(StrParam("%SBulkMod", ItemActor), 0);
        if (CompatMods & RL_AGILITY_MOD)
            SetInventory(StrParam("%SAgilityMod", ItemActor), 0);
        if (CompatMods & RL_TECH_MOD)
            SetInventory(StrParam("%STechnicalMod", ItemActor), 0);
        if (CompatMods & RL_SNIPER_MOD)
            SetInventory(StrParam("%SSniperMod", ItemActor), 0);
        if (CompatMods & RL_FIREST_MOD)
            SetInventory(StrParam("%SFirestormMod", ItemActor), 0);
        if (CompatMods & RL_NANO_MOD)
            SetInventory(StrParam("%SNanoMod", ItemActor), 0);
        if (CompatMods & RL_DEMON_MOD)
            SetInventory(StrParam("%SDemonArtifacts", ItemActor), 0);

        TakeInventory("RLWeaponLimit", 1);
        CheckDRLASetWeapons();
    }
    else if (Category == 3 || Category == 9) // Armor/Boots
        TakeInventory("RLArmorInInventory", 1);
    else if (ItemActor == "RLArmorModItem") // Armor Modpack
        TakeInventory("RLArmorModItemInInventory", 1);
    else if (ItemActor == "RLDemonArtifactItem") // Demon Artifacts [KS] THANKS YHOLL
        TakeInventory("RLDemonArtifactLimit", 1);
    else if (Category == 8) // Mod Packs
    {
        if (PlayerClass(PlayerNumber()) == 2) // Special handling case for Technician
            TakeInventory("RLScavengerModLimit", 1);
        else
            TakeInventory("RLModLimit", 1);
    }
    else if (ItemActor == "RLHatredSkull" || // Skulls
             ItemActor == "RLBloodSkull" ||
             ItemActor == "RLFireSkull")
        TakeInventory("RLSkullLimit", 1);
    else if (ItemActor == "RLPhaseDevice" || // Phase Devices
             ItemActor == "RLHomingPhaseDevice" ||
             ItemActor == "RLRecallPhaseDevice" ||
             ItemActor == "RLExperimentalPhaseDevice")
        TakeInventory("RLPhaseDeviceLimit", 1);
}

void GiveDRLAArmorToken(str ArmorType)
{
    if (PlayerClass(PlayerNumber()) == 3) // Renegade
        GiveInventory(StrParam("%SToken", StrLeft(ArmorType, StrLen(ArmorType) - 8)), 1);
    else
        GiveInventory(StrParam("%SToken", ArmorType), 1);
}

void RemoveDRLAArmorToken(str ArmorType)
{
    if (PlayerClass(PlayerNumber()) == 3) // Renegade
        TakeInventory(StrParam("%SToken", StrLeft(ArmorType, StrLen(ArmorType) - 8)), 1);
    else
        TakeInventory(StrParam("%SToken", ArmorType), 1);
}

void CheckDRLASetWeapons()
{
    bool hasNuclearWeapon = false;
    str const NuclearWeapons[15] =
    {
        // Nuclear Plasma Pistol
        "RLNuclearPlasmaPistol",
        "RLHighPowerNuclearPlasmaPistol",
        "RLStormNuclearPlasmaPistol",
        "RLSuperchargedNuclearPlasmaPistol",

        // Nuclear Plasma Shotgun
        "RLNuclearPlasmaShotgun",

        // Nuclear Plasma Rifle
        "RLNuclearPlasmaRifle",
        "RLHighPowerNuclearPlasmaRifle",
        "RLAssaultRifleNuclearPlasmaRifle",
        "RLBurstCannonNuclearPlasmaRifle",

        // Nuclear Plasma RIfle Mk II
        "RLNuclearPlasmaRifleMkII",

        // Nuclear BFG 9000
        "RLNuclearBFG9000",
        "RLHighPowerNuclearBFG9000",

        // Nuclear VBFG 9000
        "RLNuclearVBFG9000",

        // Nuclear Biggest Fucking Gun
        "RLNuclearBiggestFuckingGun",

        // Onslought
        "RLNuclearOnslaught"
    };

    if (CheckInventory("RLNuclearWeaponSetBonusWeapon")) {
        // Weapon portion of Nuclear Set Bonus Checking
        for (int i = 0; i < 15; i++)
            if (CheckInventory(NuclearWeapons[i]))
            {
                hasNuclearWeapon = true;
                break;
            }
        if (!hasNuclearWeapon)
        {
            TakeInventory("RLNuclearWeaponSetBonusWeapon", 1);
            TakeInventory("RLNuclearWeaponSetBonusActive", 1);
        }
    }

    // Tristar blaster Set Bonus Checking
    if (!CheckInventory("RLTristarBlaster") && !CheckInventory("RLHighPowerTristarBlaster") && !CheckInventory("RLNanomanufactureAmmoTristarBlaster"))
    {
        TakeInventory("RLCerberusSetBonusTristarBlaster", 1);
        TakeInventory("RLCerberusSetBonusActive", 1);
    }
}

void NomadModPacksSave()
{
    int Number;
    int Category;
    ItemInfoPtr ItemPtr;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!PlayerInGame(i)) continue;

        if (PlayerClass(i) != 7) continue;

        SetActivator(Players(i).TID);

        for (int i = 0; i < ItemMax[Category]; i++)
            if (CheckInventory(ItemData[Category][i].Actor))
            {
                ItemPtr = &ItemData[Category][i];

                if ((ItemPtr->CompatMods & RL_MOD_LIMIT) && CheckInventory(StrParam("%SModLimit", ItemPtr->Actor)))
                {
                    Player.NomadBasicItems[Number] = ItemPtr->Actor;
                    Player.NomadModPacks[Number] = StrParam("%SModLimit", ItemPtr->Actor);
                    Player.NomadAmountModPacks[Number] = CheckInventory(StrParam("%SModLimit", ItemPtr->Actor));
                    Number++;
                }
                if ((ItemPtr->CompatMods & RL_POWER_MOD) && CheckInventory(StrParam("%SPowerMod", ItemPtr->Actor)))
                {
                    Player.NomadBasicItems[Number] = ItemPtr->Actor;
                    Player.NomadModPacks[Number] = StrParam("%SPowerMod", ItemPtr->Actor);
                    Player.NomadAmountModPacks[Number] = CheckInventory(StrParam("%SPowerMod", ItemPtr->Actor));
                    Number++;
                }
                if ((ItemPtr->CompatMods & RL_BULK_MOD) && CheckInventory(StrParam("%SBulkMod", ItemPtr->Actor)))
                {
                    Player.NomadBasicItems[Number] = ItemPtr->Actor;
                    Player.NomadModPacks[Number] = StrParam("%SBulkMod", ItemPtr->Actor);
                    Player.NomadAmountModPacks[Number] = CheckInventory(StrParam("%SBulkMod", ItemPtr->Actor));
                    Number++;
                }
                if ((ItemPtr->CompatMods & RL_AGILITY_MOD) && CheckInventory(StrParam("%SAgilityMod", ItemPtr->Actor)))
                {
                    Player.NomadBasicItems[Number] = ItemPtr->Actor;
                    Player.NomadModPacks[Number] = StrParam("%SAgilityMod", ItemPtr->Actor);
                    Player.NomadAmountModPacks[Number] = CheckInventory(StrParam("%SAgilityMod", ItemPtr->Actor));
                    Number++;
                }
                if ((ItemPtr->CompatMods & RL_TECH_MOD) && CheckInventory(StrParam("%STechnicalMod", ItemPtr->Actor)))
                {
                    Player.NomadBasicItems[Number] = ItemPtr->Actor;
                    Player.NomadModPacks[Number] = StrParam("%STechnicalMod", ItemPtr->Actor);
                    Player.NomadAmountModPacks[Number] = CheckInventory(StrParam("%STechnicalMod", ItemPtr->Actor));
                    Number++;
                }
                if ((ItemPtr->CompatMods & RL_SNIPER_MOD) && CheckInventory(StrParam("%SSniperMod", ItemPtr->Actor)))
                {
                    Player.NomadBasicItems[Number] = ItemPtr->Actor;
                    Player.NomadModPacks[Number] = StrParam("%SSniperMod", ItemPtr->Actor);
                    Player.NomadAmountModPacks[Number] = CheckInventory(StrParam("%SSniperMod", ItemPtr->Actor));
                    Number++;
                }
                if ((ItemPtr->CompatMods & RL_FIREST_MOD) && CheckInventory(StrParam("%SFirestormMod", ItemPtr->Actor)))
                {
                    Player.NomadBasicItems[Number] = ItemPtr->Actor;
                    Player.NomadModPacks[Number] = StrParam("%SFirestormMod", ItemPtr->Actor);
                    Player.NomadAmountModPacks[Number] = CheckInventory(StrParam("%SFirestormMod", ItemPtr->Actor));
                    Number++;
                }
                if ((ItemPtr->CompatMods & RL_NANO_MOD) && CheckInventory(StrParam("%SNanoMod", ItemPtr->Actor)))
                {
                    Player.NomadBasicItems[Number] = ItemPtr->Actor;
                    Player.NomadModPacks[Number] = StrParam("%SNanoMod", ItemPtr->Actor);
                    Player.NomadAmountModPacks[Number] = CheckInventory(StrParam("%SNanoMod", ItemPtr->Actor));
                    Number++;
                }
                if ((ItemPtr->CompatMods & RL_DEMON_MOD) && CheckInventory(StrParam("%SDemonArtifacts", ItemPtr->Actor)))
                {
                    Player.NomadBasicItems[Number] = ItemPtr->Actor;
                    Player.NomadModPacks[Number] = StrParam("%SDemonArtifacts", ItemPtr->Actor);
                    Player.NomadAmountModPacks[Number] = CheckInventory(StrParam("%SDemonArtifacts", ItemPtr->Actor));
                    Number++;
                }
            }

        SetActivator(0, AAPTR_NULL);
    }
}

void NomadModPacksLoad()
{
    int Category;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!PlayerInGame(i)) continue;

        if (PlayerClass(i) != 7) continue;

        SetActivator(Players(i).TID);

        for (int i = 0; i < ItemMax[Category]; i++)
            if (CheckInventory(ItemData[Category][i].Actor))
            {
                for (int j = 0; j < 30; j++)
                {
                    if (ItemData[Category][i].Actor == Player.NomadBasicItems[j])
                        SetInventory(Player.NomadModPacks[j], Player.NomadAmountModPacks[j]);
                }
            }

        // Clear Data
        for (int i = 0; i < 30; i++)
        {
            Player.NomadBasicItems[i] = 0;
            Player.NomadModPacks[i] = 0;
            Player.NomadAmountModPacks[i] = 0;
        }

        SetActivator(0, AAPTR_NULL);
    }
}

void NanomaniacTransport()
{
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!PlayerInGame(i)) continue;
        if (PlayerClass(i) != 8) continue;

        SetActivator(Players(i).TID);
        SetInventory("RLNanoManiacTransportToken", 1);
        SetActivator(0, AAPTR_NULL);
    }
}

// --------------------------------------------------
// Math
//

// This function is specifically to avoid dividing by zero in the event that that can happen.
int CalcPercent(int Current, int Maximum)
{
    if (Maximum == 0)
        return 100;
    return Current * 100 / Maximum;
}

// Computes n-th power of x
int Pow(int x, int n)
{
    int y = 1;
    while (n-- > 0) y *= x;
    return y;
}

fixed PowFixed(fixed x, int n)
{
    fixed y = 1.0;
    while (n-- > 0) y *= x;
    return y;
}

// Return the absolute value of a negative integer
int Abs(int x)
{
    if (x < 0)
        return -x;

    return x;
}

// Rounds a fixed to the nearest integer
int RoundInt(fixed x)
{
    return (int)(x + 0.5);
}

// Return the absolute value of a fixed-point value
fixed AbsFixed(fixed x)
{
    if (x < 0.0)
        return -x;

    return x;
}

fixed Min(fixed x, fixed y)
{
    if (x < y)
        return x;

    return y;
}

fixed Max(fixed x, fixed y)
{
    if (x > y)
        return x;

    return y;
}

// Gets the distance between two TIDs
fixed Distance(int TID1, int TID2)
{
    fixed X = GetActorX(TID1) - GetActorX(TID2);
    fixed Y = GetActorY(TID1) - GetActorY(TID2);
    fixed Z = GetActorZ(TID1) - GetActorZ(TID2);

    return VectorLength(VectorLength(X, Y), Z);
}

// Gets the distance between two points
fixed Distance2D(int X1, int Y1, int X2, int Y2)
{
    fixed X = X1 - X2;
    fixed Y = Y1 - Y2;

    return VectorLength(X, Y);
}

fixed Lerp(fixed a, fixed b, fixed t)
{
    return ((1.0 - t) * a) + (t * b);
}

void Interpolate(InterpData *Data)
{
    if (Data->Value != Data->OldValue)
    {
        Data->StartValue = Data->DisplayValue;
        Data->TimerMax = Abs(Data->Value - Data->DisplayValue) * Data->TimerMaxCap;
        if (Data->TimerMax > 35 * Data->TimerMaxCap)
            Data->TimerMax = 35 * Data->TimerMaxCap;
        Data->Timer = Data->TimerMax - 1;
    }

    if (Data->Timer > 0)
    {
        long int Percent = (Data->TimerMax * Data->TimerMax) - (Data->Timer * Data->Timer);
        Data->DisplayValue = Data->StartValue + (int)((long int)(Data->Value - Data->StartValue) * Percent / (Data->TimerMax * Data->TimerMax));
        Data->Timer--;
    }
    else
        Data->DisplayValue = Data->Value;
}

/* Super SidDoyle Math Functions(TM)

   I'm not quite sure how I came up with this. I have to use this myself, now.
   Thank you for asking me to do this. :D

   Example: Say you have a stat value ranging from 1-200 that you want to
   produce a gradual curve from. Say we want that curve to start at 1 and
   end at 75. Now let's say we need a value from this curve -- maybe we
   have a defense value of 67 and need to know what percent damage that
   absorbs. You would call the following:

   fixed result = curve(67, 1, 200, 1, 75);

   Coincidentally, that value is close to 42, if you round up. Have fun.
*/

fixed Map(fixed Value, fixed Low1, fixed High1, fixed Low2, fixed High2)
{
    fixed Ratio = (Value - Low1) / (High1 - Low1);
    return (High2 - Low2) * Ratio + Low2;
}

fixed Curve(fixed Value, fixed Low1, fixed High1, fixed Low2, fixed High2)
{
    fixed Constant = High2 / High1;
    fixed CurveLow = Map(Value * Constant, Low1 * Constant, High2, Low2, High2);
    return Map(Value, Low1, High1, CurveLow, High2);
}

// Here's an alternative curve that starts gradually and accelerates. --SidDoyle
fixed AltCurve(fixed Value, fixed Low1, fixed High1, fixed Low2, fixed High2)
{
    fixed Constant = High2 / High1;
    fixed CurveHigh = Map(Value * Constant, Low1 * Constant, High1 * Constant, Low1 * Constant, High2);
    return Map(Value, Low1, High1, Low2, CurveHigh);
}

// --------------------------------------------------
// Strings
//

OptionalArgs(1) bool StartsWith(str InString, str Prefix, bool NoCase)
{
    if (NoCase)
        return !StrICmp(InString, Prefix, StrLen(Prefix));
    return !StrCmp(InString, Prefix, StrLen(Prefix));
}

OptionalArgs(1) bool EndsWith(str InString, str Suffix, bool NoCase)
{
    if (StrLen(InString) < StrLen(Suffix))
        return false;

    if (NoCase)
        return !StrICmp(StrRight(InString, StrLen(Suffix)), Suffix);
    return !StrCmp(StrRight(InString, StrLen(Suffix)), Suffix);
}

OptionalArgs(1) bool Contains(str InString, str SubString, bool NoCase)
{
    int TargLen = StrLen(InString);
    int SubLen = StrLen(SubString);

    if (TargLen < SubLen)
        return false;

    for (int i = 0; i <= TargLen - SubLen; i++)
    {
        str Part = StrMid(InString, i, SubLen);
        if ((!NoCase && !StrCmp(Part, SubString)) || (NoCase && !StrICmp(Part, SubString)))
            return true;
    }

    return false;
}

// Formats time into MM:SS for display with Print or HudMessage
str FormatTime(int t)
{
    int Hours = t / 35 / 60 / 60;
    int Minutes = t / 35 / 60 % 60;
    int Seconds = t / 35 % 60;
    str Time = "";

    // Hours
    if (Hours > 0)
        Time = StrParam("%d:", Hours);

    // Minutes
    if (Hours > 0 && Minutes < 10)
        Time = StrParam("%S0%d:", Time, Minutes);
    else
        Time = StrParam("%S%d:", Time, Minutes);

    // Seconds
    if (Seconds < 10)
        Time = StrParam("%S0%d", Time, Seconds);
    else
        Time = StrParam("%S%d", Time, Seconds);

    return Time;
}

str ColorCodeFromName(str s)
{
    for (int i = 0; i < sizeof(ColorNames); i++)
    {
        if (StrCmp(s, ColorNames[i]) == 0)
            return ColorCodes[i];
    }
    return "";
}

// --------------------------------------------------
// Debugging/Cheats
//

// HAAAAAAAAAAAAAAAX
NamedScript Console void Cheat(int StatBoost)
{
    // gg no re
    if (StatBoost == 0)
    {
        Player.ActualHealth = 0;
        ActivatorSound("mission/gottarget2", 127);
        return;
    }

    // Max Level/Rank
    Player.Level = MAX_LEVEL;
    Player.RankLevel = MAX_RANK;

    // Stats
    Player.Strength = StatBoost;
    Player.Defense = StatBoost;
    Player.Vitality = StatBoost;
    Player.Energy = StatBoost;
    Player.Regeneration = StatBoost;
    Player.Agility = StatBoost;
    Player.Capacity = StatBoost;
    Player.Luck = StatBoost;

    // Max out all Skills
    AllSkills();

    // Items - Modules
    SetInventory("DRPGModule", 1000000000);

    // Items - Powerups
    SetInventory("PowerInvulnerable", 1);
    SetInventory("PowerShadow", 1);
    SetInventory("PowerIronFeet", 1);
    SetInventory("PowerLightAmp", 1);
    SetInventory("PowerStrength", 1);

    // Items - Ammo
    SetInventory("Clip", GetAmmoCapacity("Clip") * 2);
    SetInventory("Shell", GetAmmoCapacity("Shell") * 2);
    SetInventory("RocketAmmo", GetAmmoCapacity("RocketAmmo") * 2);
    SetInventory("Cell", GetAmmoCapacity("Cell") * 2);

    // Items - Keys
    SetInventory("DRPGRedCard", 1);
    SetInventory("DRPGYellowCard", 1);
    SetInventory("DRPGBlueCard", 1);
    SetInventory("DRPGRedSkull", 1);
    SetInventory("DRPGYellowSkull", 1);
    SetInventory("DRPGBlueSkull", 1);

    // Items - Stims
    SetInventory("DRPGStimSmall", 1000);
    SetInventory("DRPGStimMedium", 1000);
    SetInventory("DRPGStimLarge", 1000);
    SetInventory("DRPGStimXL", 1000);
    SetInventory("DRPGStimDetox", 1000);

    // Items - Misc
    SetInventory("DRPGLife", 1000);
    SetInventory("DRPGChipGold", 1000);
    SetInventory("DRPGChipPlatinum", 1000);
    GiveAugs(100, 100, 100);
    GiveCredits(0);
    FullShield();
    GiveCompounds(40000);

    // Restore HP/EP/Shields
    Player.ActualHealth = Player.HealthMax * 2;
    Player.EP = Player.EPMax;
    Player.Shield.Charge = Player.Shield.Capacity;
    Player.Shield.Timer = 0;
}

NamedScript Console void AllSkills()
{
    for (int i = 0; i < MAX_CATEGORIES; i++)
        for (int j = 0; j < MAX_SKILLS; j++)
        {
            Player.SkillLevel[i][j].Level = Skills[i][j].MaxLevel;
            Player.SkillLevel[i][j].CurrentLevel = Skills[i][j].MaxLevel;
        }
}

// For testing different stats and their effects
NamedScript Console void ModStat(int Stat, int Value)
{
    switch (Stat - 1)
    {
    case STAT_STRENGTH:
        Player.Strength = Value;
        break;
    case STAT_DEFENSE:
        Player.Defense = Value;
        break;
    case STAT_VITALITY:
        Player.Vitality = Value;
        break;
    case STAT_ENERGY:
        Player.Energy = Value;
        break;
    case STAT_REGENERATION:
        Player.Regeneration = Value;
        break;
    case STAT_AGILITY:
        Player.Agility = Value;
        break;
    case STAT_CAPACITY:
        Player.Capacity = Value;
        break;
    case STAT_LUCK:
        Player.Luck = Value;
        break;
    }
}

NamedScript Console void ModStatXP(int Stat, long int Value)
{
    switch (Stat - 1)
    {
    case STAT_STRENGTH:
        Player.StrengthXP = Value;
        break;
    case STAT_DEFENSE:
        Player.DefenseXP = Value;
        break;
    case STAT_VITALITY:
        Player.VitalityXP = Value;
        break;
    case STAT_ENERGY:
        Player.EnergyXP = Value;
        break;
    case STAT_REGENERATION:
        Player.RegenerationXP = Value;
        break;
    case STAT_AGILITY:
        Player.AgilityXP = Value;
        break;
    case STAT_CAPACITY:
        Player.CapacityXP = Value;
        break;
    case STAT_LUCK:
        Player.LuckXP = Value;
        break;
    }
}

// Look up a value on the XP Table
NamedScript Console void LookupXPTable(int index)
{
    Log("%ld", XPTable[index]);
}

// Give Credits
NamedScript Console void GiveCredits(int Amount)
{
    if (Amount == 0) Amount = 1000000000;
    GiveInventory("DRPGCredits", Amount);
    ActivatorSound("credits/payout", 127);
}

// Shuffle the Shop Special
NamedScript Console void ShopSpecialShuffle()
{
    ShopSpecialTimer = 0;
}

// Give all Augs
NamedScript Console void GiveAugs(int Canisters, int Upgrades, int Slots)
{
    GiveInventory("DRPGAugCanister", Canisters);
    GiveInventory("DRPGAugUpgradeCanister", Upgrades);
    GiveInventory("DRPGAugSlotUpgrade", Slots);

    for (int i = 0; i < AUG_MAX; i++)
    {
        AugInfoPtr AugPtr = &AugData[i];
        Player.Augs.Level[i] = AugPtr->MaxLevel;
        Player.Augs.CurrentLevel[i] = Player.Augs.Level[i];
    }
}

// Give all Shield Components
NamedScript Console void FullShield()
{
    // Bodies
    for (int i = 0; i < ShieldPartsMax[0]; i++)
    {
        ShieldPartPtr Part = &ShieldParts[0][i];
        GiveInventory(Part->Actor, 1);
    }

    // Batteries
    for (int i = 0; i < ShieldPartsMax[1]; i++)
    {
        ShieldPartPtr Part = &ShieldParts[1][i];
        GiveInventory(Part->Actor, 1);
    }

    // Capacitors
    for (int i = 0; i < ShieldPartsMax[2]; i++)
    {
        ShieldPartPtr Part = &ShieldParts[2][i];
        GiveInventory(Part->Actor, 1);
    }

    // Accessories
    for (int i = 0; i < MAX_ACCESSORIES; i++)
    {
        ShieldAccsPtr Accessory = &ShieldAccessories[i];
        GiveInventory(Accessory->Actor, 1);
    }
}

// Full Locker
NamedScript Console void FullLocker(int Amount)
{
    for (int i = 0; i < ItemCategories; i++)
        for (int j = 0; j < ItemMax[i]; j++)
        {
            Player.Locker[i][j] = Amount;
        }
}

// Give all Stim Compounds
NamedScript Console void GiveCompounds(int Amount)
{
    GiveInventory("DRPGStimSmall", Player.CapacityTotal);
    GiveInventory("DRPGStimMedium", Player.CapacityTotal);
    GiveInventory("DRPGStimLarge", Player.CapacityTotal);
    GiveInventory("DRPGStimXL", Player.CapacityTotal);

    for (int i = 0; i < STIM_MAX; i++)
        if (Amount == 0)
            Player.Stim.Vials[i] = Player.CapacityTotal * 10;
        else
            Player.Stim.Vials[i] += Amount;
}

// --------------------------------------------------
// System
//

void CreateTranslations()
{
    // Damage Numbers - Critical
    CreateTranslationStart(DNUM_CRITICAL);
    CreateTranslationDesat(0, 255, 0, 0, 0, 1.5, 0, 0);
    CreateTranslationEnd();

    // Damage Numbers - Healed
    CreateTranslationStart(DNUM_HEAL);
    CreateTranslationDesat(0, 255, 0, 0, 0, 0, 1.5, 0);
    CreateTranslationEnd();

    // Damage Numbers - Scratch
    CreateTranslationStart(DNUM_SCRATCH);
    CreateTranslationDesat(0, 255, 0, 0, 0, 0.75, 0.75, 0.75);
    CreateTranslationEnd();

    // Credit Gain
    CreateTranslationStart(DNUM_CREDGAIN);
    CreateTranslationDesat(0, 255, 0, 0, 0, 1.5, 1.5, 0);
    CreateTranslationEnd();

    // Credit Loss
    CreateTranslationStart(DNUM_CREDLOSS);
    CreateTranslationDesat(0, 255, 0, 0, 0, 1.5, 0.75, 0);
    CreateTranslationEnd();

    // EP Gain
    CreateTranslationStart(DNUM_EPGAIN);
    CreateTranslationDesat(0, 255, 0, 0, 0, 0, 1.5, 1.5);
    CreateTranslationEnd();

    // EP Loss
    CreateTranslationStart(DNUM_EPLOSS);
    CreateTranslationDesat(0, 255, 0, 0, 0, 0, 0, 1.5);
    CreateTranslationEnd();

    // Shield Gain
    CreateTranslationStart(DNUM_SHIELDGAIN);
    CreateTranslationDesat(0, 255, 0, 0, 0, 1.5, 0, 1.5);
    CreateTranslationEnd();

    // Shield Loss
    CreateTranslationStart(DNUM_SHIELDLOSS);
    CreateTranslationDesat(0, 255, 0, 0, 0, 0.75, 0, 0.75);
    CreateTranslationEnd();
}

bool CheckInput(int Key, int State, bool ModInput, int PlayerNumber)
{
    if (InMultiplayer || GetUserCVar(PlayerNumber, "drpg_deltatouch"))
        return CheckInputACS(Key, State, ModInput, PlayerNumber);
    else
    {
        int i = State;

        if (Key == (BT_FORWARD | BT_BACK | BT_MOVELEFT | BT_MOVERIGHT))
            i = KEY_ANYMOVEMENT;

        return CheckInputZS(Key, i);
    }

    static int CheckInputRepeatTimer; // Delete in the future
}

// Singleplayer Input is managed by ZScript and updated via this function
NamedScript DECORATE void UpdateInput(int Key, bool KeyDown, bool KeyRepeat)
{
    // ACS lags behind a tick, so it must wait
    Delay(1);

    // Misc
    keyRepetition = KeyRepeat;

    // Check binding states
    for (int i = 0; i < sizeof (bindingValues); i++)
        if (Key == bindingValues[i])
            bindingStates[i] = KeyDown;

    // Single Key Processing
    if (KeyDown)
    {
        keyCurrent = Key;
        Delay(1);
        keyCurrent = 0;
    }
}

// New
bool CheckInputZS(int Key, int State)
{
    switch (State)
    {
    case KEY_PRESSED:
    {
        if (Key == keyCurrent)
            return true;
    }
    break;
    case KEY_ONLYPRESSED:
    {
        if (Key == keyCurrent)
        {
            // Check if any other keys are being used
            for (int i = 0; i < sizeof (bindingValues); i++)
                if (Key != bindingValues[i] && bindingStates[i])
                    return false;

            // Yay
            return true;
        }
    }
    break;
    case KEY_HELD:
    {
        // Check binding states
        for (int i = 0; i < sizeof (bindingValues); i++)
            if (Key == bindingValues[i] && bindingStates[i])
                return true;
    }
    break;
    case KEY_ONLYHELD:
    {
        // Check binding states
        for (int i = 0; i < sizeof (bindingValues); i++)
            if (Key != bindingValues[i] && bindingStates[i])
                return false;

        // Check binding states
        for (int i = 0; i < sizeof (bindingValues); i++)
            if (Key == bindingValues[i] && bindingStates[i])
                return true;
    }
    break;
    case KEY_ANYIDLE:
    {
        // Check binding states
        for (int i = 0; i < sizeof (bindingValues); i++)
            if (bindingStates[i])
                return false;

        return true;
    }
    break;
    case KEY_ANYNOTIDLE:
    {
        // Check binding states
        for (int i = 0; i < sizeof (bindingValues); i++)
            if (bindingStates[i])
                return true;

        return false;
    }
    break;
    case KEY_ANYMOVEMENT:
    {
        // Check binding states
        // bindingStates values 0 - 3 are movement keys
        for (int i = 0; i < 4; i++)
            if (bindingStates[i])
                return true;

        return false;
    }
    break;
    case KEY_REPEAT:
    {
        if (keyRepetition)
        {
            // Limit repetition
            if ((Timer() % 5) == 0)
                // Check binding states
                for (int i = 0; i < sizeof (bindingValues); i++)
                    if (Key == bindingValues[i] && bindingStates[i])
                        return true;
        }
        else
        {
            // No repetition
            if (Key == keyCurrent)
                return true;
        }
    }
    break;
    }

    return false;
}

// Original
bool CheckInputACS(int Key, int State, bool ModInput, int PlayerNumber)
{
    int Input;
    int InputOld;
    int Buttons;
    int OldButtons;
    static bool CheckInputRepeat;
    static int CheckInputRepeatTimer;
    double AxisY;
    double AxisX;
    // These two are meant to mimic OldButtons
    static double OldAxisY;
    static double OldAxisX;
    bool UseAxis = false;
    bool UseDeltaTouch = GetUserCVar(PlayerNumber, "drpg_deltatouch");

    if (!ModInput)
    {
        Input = INPUT_BUTTONS;
        InputOld = INPUT_OLDBUTTONS;
    }
    else
    {
        Input = MODINPUT_BUTTONS;
        InputOld = MODINPUT_OLDBUTTONS;
    }

    Buttons = GetPlayerInput(PlayerNumber, Input);
    OldButtons = GetPlayerInput(PlayerNumber, InputOld);

    // Proper navigation support for joystick, binding movement keys no longer necessary
    if (UseDeltaTouch && Key & BT_FORWARD || UseDeltaTouch && Key & BT_BACK || UseDeltaTouch && Key & BT_MOVELEFT || UseDeltaTouch && Key & BT_MOVERIGHT)
    {
        AxisY = GetPlayerInput(PlayerNumber, INPUT_FORWARDMOVE);
        AxisX = GetPlayerInput(PlayerNumber, INPUT_SIDEMOVE);

        // Simplify
        if (AxisY > 1.0) AxisY = 1.0;
        if (AxisY < -1.0) AxisY = -1.0;
        if (AxisX > 1.0) AxisX = 1.0;
        if (AxisX < -1.0) AxisX = -1.0;

        // Decide if the axises are old and moldy
        if (AxisY < OldAxisY || AxisY > OldAxisY)
            OldAxisY = 0;
        if (AxisX > OldAxisX || AxisX < OldAxisX)
            OldAxisX = 0;

        if (OldAxisY == 0 && OldAxisX == 0)
        {
            if (Key & BT_FORWARD && AxisY == 1.0)
                UseAxis = true;
            else if (Key & BT_BACK && AxisY == -1.0)
                UseAxis = true;
            else if (Key & BT_MOVELEFT && AxisX == -1.0)
                UseAxis = true;
            else if (Key & BT_MOVERIGHT && AxisX == 1.0)
                UseAxis = true;
        }
    }

    switch (State)
    {
    case KEY_PRESSED:
    {
        if (UseAxis)
        {
            OldAxisY = AxisY;
            OldAxisX = AxisX;
            return true;
        }
        else if (Buttons & Key && !(OldButtons & Key))
            return true;
    }
    break;
    case KEY_ONLYPRESSED:
    {
        if (UseAxis)
        {
            OldAxisY = AxisY;
            OldAxisX = AxisX;
            return true;
        }
        else if (Buttons & Key && !(OldButtons & Key))
            return true;
    }
    break;
    case KEY_HELD:
    {
        if (UseAxis)
            return true;
        else if (Buttons & Key)
            return true;
    }
    break;
    case KEY_ONLYHELD:
    {
        if (Buttons & Key)
            return true;
    }
    break;
    case KEY_ANYIDLE:
    {
        if (!UseAxis && Buttons & Key && !(OldButtons & Key))
            return true;
    }
    break;
    case KEY_ANYNOTIDLE:
    {
        if (Buttons > 0)
            return true;
    }
    break;
    // Originally not compatible with multiplayer, so it is now KEY_PRESSED
    case KEY_REPEAT:
    {
        if (UseAxis)
        {
            int CurrentTime = Timer();
            if (CurrentTime + 5 + 1 < CheckInputRepeatTimer) CheckInputRepeat = false;
            if (!CheckInputRepeat)
            {
                CheckInputRepeatTimer = CurrentTime;
                CheckInputRepeatTimer += 5;
                CheckInputRepeat = true;
                return true;
            }
            else
            {
                if (CurrentTime >= CheckInputRepeatTimer)
                    CheckInputRepeat = false;
                else
                    return false;
            }
        }
        else if (Buttons & Key && !(OldButtons & Key))
            return true;
    }
    break;
    }

    return false;
}

NamedScript MenuEntry void SetHUDPreset(int Preset)
{
    ScriptCall("DRPGZUtilities", "SetHUDPreset", Preset);
}

NamedScript MenuEntry void ResetToDefaults()
{
    if (InMultiplayer)
    {
        ActivatorSound("menu/error", 127);
        HudMessage("'Reset to Defaults' can only be done in singleplayer due to potential desync.");
        EndHudMessage(HUDMSG_FADEOUT, 0, "Orange", 0.5, 0.5, 8.0, 1.0);
        return;
    }

    ScriptCall("DRPGZUtilities", "ResetToDefaults");
}

OptionalArgs(1) void LogMessage(str Message, int Level)
{
    bool DebugMode = (ActivatorTID() == Player.TID ? DebugLog : GetActivatorCVar("drpg_debug"));

    if (Level == LOG_DEBUG)
        Message = StrParam("\CdDEBUG: \C-%S", Message);
    if (Level == LOG_WARNING)
        Message = StrParam("\CiWARNING: \C-%S", Message);
    if (Level == LOG_ERROR)
        Message = StrParam("\CgERROR: \C-%S", Message);

    // Don't log the message if the caller isn't in Debug Mode
    if (Level == LOG_DEBUG && !DebugMode)
        return;

    Log("%S", Message);
}

void ClearInfo(CharSaveInfo *Info)
{
    int i, j;

    // Version / Compatibility Flag
    Info->Version = 0;
    Info->CompatMode = 0;

    // Level / Rank Level
    Info->Level = 0;
    Info->RankLevel = 0;

    // Stats
    for (i=0; i < STAT_MAX; i++)
        Info->Stats[i] = 0;

    // Skills
    for (i=0; i < MAX_CATEGORIES; i++)
        for (j=0; j < MAX_SKILLS; j++)
            Info->Skills[i][j] = 0;

    // Skill Wheel
    for (i=0; i < MAX_SKILLKEYS; i++)
        for (j=0; j < 2; j++)
            Info->SkillWheel[i][j] = 0;

    // Augmentations
    for (i=0; i < AUG_MAX; i++)
        Info->Augs[i] = 0;

    // Stims
    for (i=0; i < STIM_MAX; i++)
        Info->Stims[i] = 0;

    // Turret Upgrades
    for (i=0; i < TU_MAX; i++)
        Info->TurretUpgrades[i] = 0;

    // Misc
    Info->Credits = 0;
    Info->Modules = 0;
    Info->Medkit = 0;
    Info->GoldChips = 0;
    Info->PlatinumChips = 0;
    Info->ShopCard = 0;
    Info->Battery = 0;
    Info->Toxicity = 0;
    Info->ArenaWave = 0;

    // Locker
    for (i=0; i < ITEM_CATEGORIES; i++)
        for (j=0; j < ITEM_MAX; j++)
            Info->Locker[i][j] = 0;
    // Auto-Sell
    Info->ItemAutoMode[i][j] = 0;
    for (i=0; i < ITEM_MAX; i++)
        for (j=0; j < DRLA_MODPACK_SIZE; j++)
            Info->WeaponMods[i][j] = 0;


    // ----- COMPATIBILITY EXTENSIONS -----

    // DRLA Tokens
    for (i=0; i < DRLA_MAX_TOKENS; i++)
        Info->DRLATokens[i] = false;

    // ------------------------------------

    // Checksum
    Info->Checksum = 0;
}

// --------------------------------------------------
// Dynamic Arrays
//

void ArrayCreate(DynamicArray *Array, str Name, int InitSize, int ItemSize)
{
    if (DebugLog)
    {
        Log("acArgName: %S", Name);
        Log("acArgInitSize: %i", InitSize);
        Log("acArgItemSize: %i", ItemSize);

        Log("Array: %i", Array);
        Log("Array->Name: %S", Array->Name);
        Log("Array->Position: %i", Array->Position);
        Log("Array->Size: %i", Array->Size);
        Log("Array->ItemSize: %i", Array->ItemSize);
        Log("Array->Data: %i", Array->Data);
    }

    bool Recreate = false;
    if (Array && Array->Data != NULL)
        Recreate = true;

    Array->Name = Name;
    Array->Position = 0;

    if (DebugLog)
        Log("\CdDynamicArray: Allocating \Cj%S", Array->Name);

    if (Recreate)
    {
        if(Array->Size != InitSize || Array->ItemSize != ItemSize)
        {
            LogMessage("Reallocating Array",LOG_DEBUG);
            LogMessage(StrParam("Previously: @ %p Size: %i", Array->Data, Array->Size),LOG_DEBUG);
            LogMessage(StrParam("To size: %i",Array->Size * Array->ItemSize),LOG_DEBUG);
            Array->Size = InitSize;
            Array->ItemSize = ItemSize;
            Array->Data = realloc(Array->Data, Array->Size * Array->ItemSize);
        }

        LogMessage("Erasing Leftover data",LOG_DEBUG);
        memset(Array->Data, NULL, Array->Size * Array->ItemSize);
    }
    else
    {
        Array->Size = InitSize;
        Array->ItemSize = ItemSize;
        LogMessage("Creating Array",LOG_DEBUG);
        Array->Data = calloc(Array->Size, Array->ItemSize);
    }

    if (Array->Data == NULL)
    {
        Log("\CgERROR: \C-Could not allocate space for array \Cj%S", Array->Name);
        return;
    }

    if (DebugLog)
        Log("\CdDynamicArray: \Cj%S\Cd @ %p", Array->Name, Array->Data);

    //memset(Array->Data, 0xAAAAAAAA, Array->Size * Array->ItemSize);
}

void ArrayResize(DynamicArray *Array)
{
    if (Array->Data == NULL)
    {
        Log("\CgERROR: \C-Tried to resize destroyed array \Cj%S", Array->Name);
        return;
    }

    int OldSize = Array->Size;
    Array->Size *= 2;

    if (DebugLog)
        Log("\CdAttempting to resize DynamicArray: \Cj%S\Cd @ %p", Array->Name, Array->Data);

    void *tmp = realloc(Array->Data, Array->ItemSize * Array->Size);

    if (tmp == NULL)
    {
        free(Array->Data);
        Log("\CgERROR: \C-Cannot resize dynamic array \Cj%S", Array->Name);
        return;
    }

    if (DebugLog)
        Log("\CdDynamicArray: Resizing array \Cj%S\Cd @ %p to \Cj%d\Cd elements", Array->Name, Array->Data, Array->Size);

    Array->Data = tmp;

    memset((char *)Array->Data + (Array->ItemSize * OldSize), 0x00000000, (Array->Size * Array->ItemSize) - (Array->ItemSize * OldSize));
}

void ArrayDestroy(DynamicArray *Array)
{
    if (DebugLog)
        Log("\CdDynamicArray: Destroying array \Cj%S\Cd @ %p", Array->Name, Array->Data);

    free(Array->Data);

    Array->Name = "";
    Array->Position = 0;
    Array->Size = 0;
    Array->ItemSize = 0;
    Array->Data = NULL;
}

/*void ArrayDump(DynamicArray *Array)
{
    Log("\CiDynamicArray \Cj%S\C- @ %p", Array->Name, Array->Data);
    Log("\Cd* Array size: \Cj%d", Array->Size);
    Log("\Cd* Item bytesize: \Cj%d", Array->ItemSize);
    Log("\Cd* End Position: \Cj%d", Array->Position);
    Log("");
    Log("\CiItem data:");
    for (int i = 0; i < Array->Size; i++)
    {
        str DataString = StrParam("  %X: ", i);
        for (int b = 0; b < Array->ItemSize; b++)
            DataString = StrParam("%S%X ", DataString, (char)((char *)Array->Data)[Array->ItemSize * i + b]);
        if (i >= Array->Position)
            DataString = StrParam("%s\Cj(\CgUnused\Cj)", DataString);
        Log("%s", DataString);
    }
}*/

NamedScript DECORATE void SetDebugMode()
{
    DebugLog = GetCVar("drpg_debug");
}

NamedScript void Silly()
{
    SetMusic("Credits2");
}

NamedScript Console void Test()
{
    Log("Test is work");
}
