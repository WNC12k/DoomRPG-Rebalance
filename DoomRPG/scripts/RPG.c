#include "Defs.h"

#include "Arena.h"
#include "Augs.h"
#include "HealthBars.h"
#include "HUD.h"
#include "ItemData.h"
#include "Map.h"
#include "Menu.h"
#include "Mission.h"
#include "Monsters.h"
#include "Outpost.h"
#include "Popoffs.h"
#include "RPG.h"
#include "Shield.h"
#include "Shop.h"
#include "Skills.h"
#include "Stats.h"
#include "Stims.h"
#include "Turret.h"
#include "Utils.h"

// Flags
bool Transported;
bool GlobalsInitialized;
int CompatMode;
int CompatModeEx;
int CompatModeLite;
int CompatMonMode;
bool MapPacks;

// Arrays
str PlayerWeapon[MAX_PLAYERS];

// Structs
PlayerData _PlayerData[MAX_PLAYERS];

// Items
bool RPGMap ItemTIDsInitialized;
int RPGMap ItemTIDs[MAX_ITEMS];

// --------------------------------------------------
// Multiplayer HUD
//

Address(8) AddressSpace MapArraySpace    RPGMap;
Address(8) AddressSpace WorldArraySpace  RPGWorld;
Address(8) AddressSpace GlobalArraySpace RPGGlobal;

Address(2)  AddressSpace GlobalArraySpace EPArray;

Address(50) AddressSpace GlobalArraySpace ShieldArray;
Address(51) AddressSpace GlobalArraySpace ShieldCapacityArray;
Address(52) AddressSpace GlobalArraySpace ShieldHealthArray;

// Energy Points (EP)
int EPArray Address(0) EP[MAX_PLAYERS];

// Shield
int ShieldArray Address(0) Shield[MAX_PLAYERS];
int ShieldCapacityArray Address(0) ShieldCapacity[MAX_PLAYERS];
int ShieldHealthArray Address(0) ShieldHealth[MAX_PLAYERS];

str const StatColors[STAT_MAX] =
{
    "Red",         // Strength
    "Green",       // Defense
    "Brick",       // Vitality
    "LightBlue",   // Energy
    "Purple",      // Regeneration
    "Orange",      // Agility
    "Blue",        // Capacity
    "Gold"         // Luck
};

NamedScript Type_OPEN void GlobalInit()
{
    if (!GlobalsInitialized)
    {
        // Version Info
        Log("\CnDoom RPG SE Rebalance (GDCC) (Compiled on %S) loaded!", __DATE__);

        // Compatibility checking
        CheckCompatibility();

        // Get the XP Curve from the CVAR
        XPCurve = GetCVar("drpg_xp_curve");

        // Initial build of ItemData
        BuildItemData();

        // Initialize XP, Stat and Rank Tables
        InitXPTable();
        Delay(1);
        InitStatXPTable();
        Delay(1);
        InitNegativeStatXPTable();
        Delay(1);
        InitRankTable();

        // Arena Wave
        ArenaMaxWave = 1;

        GlobalsInitialized = true;

        Delay(1); // Allow CVARs to update
    }

    // Create Translations
    CreateTranslations();

    // Initial build of SkillData
    BuildSkillData();

    // Initial build of TurretUpgradeData
    BuildTurretData();

    // Initialize the Item handling script
    ItemHandler();
}

// Init Script
NamedScript Type_ENTER void Init()
{
    // Wait until globals are initialized
    while (!GlobalsInitialized) Delay(1);

    // Delay to allow for item data to initialize
    Delay(3);

    // [KS] This needs to be done on its' own, so death exits don't rob you of your TID and break EVERYTHING EVER FFS
    int ATID = ActivatorTID();
    if (ATID == 0 || ATID == 30000) // 30000 is DRLA's PID.
    {
        AssignTIDs();
        if (!InTitle) // Don't give equipment on the title screen
        {
            if (!Player.FirstRun)
                Player.Capacity = GetActivatorCVar("drpg_start_capacity");

            SortStartingItems();
            DefaultLoadout();
        }
    }

    if (!Player.FirstRun)
    {
        // [KS] Portable always-use items, because having too many mandatory use-keys is bad, yo.
        if (GetActivatorCVar("drpg_give_quickinventory"))
        {
            GiveInventory("DRPGPortableStimInjectorItem", 1); // Use Stim
            GiveInventory("DRPGPortableStimDisposalItem", 1); // Toss Stim
            GiveInventory("DRPGPortableShieldItem", 1); // Toggle Shield
            GiveInventory("DRPGPortableMedkitItem", 1); // Quick Heal
            GiveInventory("DRPGPortableFocusItem", 1); // Focus
            GiveInventory("DRPGPortableAugItem", 1); // Reactivate Augs
        }

        // Funds
        GiveInventory("DRPGCredits", GetActivatorCVar("drpg_start_credits"));
        Player.PrevCredits = CheckInventory("DRPGCredits");
        GiveInventory("DRPGModule", GetActivatorCVar("drpg_start_modules"));
        GiveInventory("DRPGTurretPart", GetActivatorCVar("drpg_start_turretparts"));

        // Level/Rank
        if (GetCVar("drpg_start_level") > 0)
        {
            Player.Level = GetActivatorCVar("drpg_start_level");
        }
        if (GetCVar("drpg_start_rank") > 0)
        {
            Player.RankLevel = GetActivatorCVar("drpg_start_rank");
        }

        // Stats
        Player.Strength = GetActivatorCVar("drpg_start_strength");
        Player.Defense = GetActivatorCVar("drpg_start_defense");
        Player.Vitality = GetActivatorCVar("drpg_start_vitality");
        Player.PrevVitality = Player.Vitality;
        Player.Energy = GetActivatorCVar("drpg_start_energy");
        Player.PrevEnergy = Player.Energy;
        Player.Regeneration = GetActivatorCVar("drpg_start_regeneration");
        Player.Agility = GetActivatorCVar("drpg_start_agility");
        Player.Capacity = GetActivatorCVar("drpg_start_capacity");
        Player.Luck = GetActivatorCVar("drpg_start_luck");

        // Total Stat Values
        Player.StrengthTotal = Player.Strength;
        Player.DefenseTotal = Player.Defense;
        Player.VitalityTotal = Player.Vitality;
        Player.EnergyTotal = Player.Energy;
        Player.RegenerationTotal = Player.Regeneration;
        Player.AgilityTotal = Player.Agility;
        Player.CapacityTotal = Player.Capacity;
        Player.LuckTotal = Player.Luck;

        // Natural Bonuses
        Player.StrengthNat = 0;
        Player.DefenseNat = 0;
        Player.VitalityNat = 0;
        Player.EnergyNat = 0;
        Player.RegenerationNat = 0;
        Player.AgilityNat = 0;
        Player.CapacityNat = 0;
        Player.LuckNat = 0;

        // Stat XP
        Player.StrengthXP = 0;
        Player.DefenseXP = 0;
        Player.VitalityXP = 0;
        Player.EnergyXP = 0;
        Player.RegenerationXP = 0;
        Player.AgilityXP = 0;
        Player.CapacityXP = 0;
        Player.LuckXP = 0;

        // Default Health/EP
        Player.EP = 50 + ((Player.Level + 1) / 2) * 5 + Player.EnergyTotal * 5;
        Player.ActualHealth = 50 + ((Player.Level + 1) / 2) * 5 + Player.VitalityTotal * 5;
        Player.PrevHealth = Player.ActualHealth;
        SetActorProperty(0, APROP_Health, Player.ActualHealth);

        // Setup HUD stuff
        Player.StatusTypeHUD = SE_NONE;

        for (int i = 0; i < MAPEVENT_MAX; i++)
            Player.SeenEventTip[i] = false;

        // Setup the Skill Wheel from the persistence CVARs
        for (int i = 0; i < MAX_SKILLKEYS; i++)
        {
            Player.SkillCategory[i] = GetActivatorCVar(StrParam("drpg_skillwheel_category_%d", i + 1));
            Player.SkillIndex[i] = GetActivatorCVar(StrParam("drpg_skillwheel_index_%d", i + 1));
        }

        // Transport should always be known
        Player.SkillLevel[MAX_CATEGORIES - 1][SkillCategoryMax[MAX_CATEGORIES - 1] - 1].Level = 1;
        Player.SkillLevel[MAX_CATEGORIES - 1][SkillCategoryMax[MAX_CATEGORIES - 1] - 1].CurrentLevel = 1;

        // Set default selected skill to nothing
        Player.SkillSelected = -1;

        // Fill Augmentation Battery
        Player.Augs.Battery = 100;

        // Setup the New! shield parts arrays
        for (int i = 0; i < SHIELDPAGE_MAX; i++)
            for (int j = 0; j < MAX_PARTS; j++)
                Player.NewShieldParts[i][j] = true;
        for (int i = 0; i < MAX_ACCESSORIES; i++)
            Player.NewShieldAccessoryParts[i] = true;

        // Compatibility Handling - DoomRL Arsenal
        if (CompatMode == COMPAT_DRLA)
        {
            // Set start chances for Exotic/Superior/Unique/Demonic/Legendary armor and boots
            Player.ArmorAssembledChance = 75.0;
            Player.ArmorExoticChance = 50.0;
            Player.ArmorSuperiorChance = 3.0;
            Player.ArmorUniqueChance = 5.0;
            Player.ArmorDemonicChance = 0.3;
            Player.ArmorLegendaryChance = 0.2;

            // Set start chances for Exotic/Superior/Unique/Demonic/Legendary weapon
            Player.WeaponExoticChance = 50.0;
            Player.WeaponSuperiorChance = 3.0;
            Player.WeaponUniqueChance = 10.0;
            Player.WeaponDemonicChance = 0.3;
            Player.WeaponLegendaryChance = 0.2;
        }

        // Compatibility Handling - DoomRL Monsters
        if (CompatMonMode == COMPAT_DRLA)
        {
            if (GetCVar("drpg_monster_mapweight") > GetCVar("DRLA_startdangerlevel"))
                GiveInventory("RLDangerLevel", GetCVar("drpg_monster_mapweight") - 1);

            if (CurrentLevel->UACBase)
                TakeInventory("RLDangerLevel", 1);
        }

        // Done first run
        Player.FirstRun = true;
    }

    // Death exit handling
    if (Player.ActualHealth <= 0)
        Player.ActualHealth = Player.HealthMax;

    // Apply camera textures and vars
    SetCameraToTexture(Player.TID, StrParam("P%iVIEW", PlayerNumber() + 1), 110);
    SetCameraToTexture(Player.TID, StrParam("P%iSVIEW", PlayerNumber() + 1), 90);
    Player.PlayerView = PlayerNumber();

    // Remove Aura if the keep Aura CVAR is off
    if (!GetCVar("drpg_skill_keepauras"))
        RemoveAura();

    // Transport FX
    if (Transported)
    {
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            // Player is not in-game
            if (!PlayerInGame(i)) continue;

            TransportInFX(Players(i).TID);
        }

        SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN); // In case we were in the menu
        Transported = false;
    }

    // Clear the Player's summons
    for (int i = 0; i < MAX_SUMMONS; i++)
        Player.SummonTID[i] = 0;
    Player.Summons = 0;

    // Initialize DropTID Array
    ArrayCreate(&Player.DropTID, "Drops", 64, sizeof(int));

    // Mission Initialization
    InitMission();

    // Small delay before initializing the scripts for mod compatibility
    Delay(4);

    // Reassignment is necessary for some mods
    if (CompatMode == COMPAT_DRLA)
        AssignTIDs();

    // Execute Game Loops
    Loop();
    PlayerHealth();
    PlayerSurvive();
    MoneyChecker();
    ShieldTimer();
    WeaponSpeed();
    StatRandomizer();
    DamageNumbers();
    InfoPopoffs();
    HealthBars();
    AutosaveHandler();
    ShopItemAutoHandler();
}

// Loop Script
NamedScript void Loop()
{
    // Small delay before initializing the script
    Delay(4);

Start:

    // If we're on the title map, terminate
    if (InTitle)
    {
        SetActorProperty(0, APROP_Invulnerable, true);
        TeleportOther(Player.TID, 1003, false);
        return;
    }

    // If the player's dead, terminate
    if (GetActorProperty(0, APROP_Health) <= 0) return;

    // Update Functions
    CheckCombo();
    CheckStats();
    CheckHardStatCaps();
    CheckRegen();
    CheckLuck();
    CheckHealth();
    CheckAugSlots();
    CheckAugs();
    CheckLevel();
    CheckRank();
    CheckSkills();
    CheckAuras();
    CheckBurnout();
    CheckStim();
    CheckPerks();
    CheckShields();
    CheckShieldAccessory();
    CheckStatBounds();
    CheckToxicity();
    CheckStimImmunity();
    CheckStatusEffects();
    CheckCapacity();
    CheckShopCard();
    CheckMission();
    CheckHUD();
    CheckKeys();

    // Handle the pulsating menu colors
    MenuCursorColor = CursorColors[(Timer() / 3) % 6];

    // Calculate Shop Discount
    Player.ShopDiscount = (int)((Player.RankLevel * 1.05) + (CurrentLevel->UACBase ? (Player.ShopCard * 5) : 0));

    // Main Menu
    if (Player.InMenu)
        MenuLoop();

    // Shop Menu
    if (Player.InShop)
        ShopLoop();

    // Menu dimming
    if (Player.InMenu || Player.InShop || Player.OutpostMenu > 0)
    {
        if (GetCVar("drpg_menudim") && !Player.StatusType[SE_BLIND])
            FadeRange(0, 0, 0, 0.65, 0, 0, 0, 0.0, 0.25);
    }

    // Menu-specific Help
    if (Player.InMenu || Player.InShop || Player.OutpostMenu > 0 || Player.CrateOpen || Player.CrateHacking)
        MenuHelp();

    // Menu icon in multiplayer
    if (InMultiplayer && (Player.InMenu || Player.InShop || Player.OutpostMenu > 0))
    {
        fixed X = GetActorX(Player.TID);
        fixed Y = GetActorY(Player.TID);
        fixed Z = GetActorZ(Player.TID);
        fixed Height = GetActorPropertyFixed(Player.TID, APROP_Height);

        SpawnForced("DRPGMenuIcon", X, Y, Z + Height + 8.0, 0, 0);
    }

    // Regeneration
    DoRegen();

    // Always Quick Heal if CVAR is set
    if (GetActivatorCVar("drpg_auto_heal"))
        if (Player.ActualHealth < Player.HealthMax / GetActivatorCVar("drpg_auto_heal_percent"))
            UseMedkit();

    // Set mass stupid high when Invulnerable or transporting to prevent knockback
    if (CheckInventory("PowerInvulnerable") || Transported)
        Player.Mass *= 128;

    // Apply Stats
    SetActorPropertyFixed(Player.TID, APROP_DamageMultiplier, (1.0 + ((fixed)Player.TotalDamage / 100.0)) * Player.DamageMult);
    SetActorPropertyFixed(Player.TID, APROP_DamageFactor, Player.DamageFactor);
    SetActorProperty(Player.TID, APROP_Mass, Player.Mass);
    SetActorProperty(Player.TID, APROP_SpawnHealth, Player.HealthMax);
    SetActorPropertyFixed(Player.TID, APROP_Speed, Player.Speed);
    SetActorPropertyFixed(Player.TID, APROP_JumpZ, Player.JumpHeight);

    // Store your current weapon in the Player Weapon array
    PlayerWeapon[PlayerNumber()] = GetWeapon();

    // DRLA Debug Mode
    if (GetCVar("drpg_debug_drla") & DDM_NOLIMITS)
    {
        SetInventory("RLWeaponLimit", 0);
        SetInventory("RLArmorInInventory", 0);
        SetInventory("RLModLimit", 0);
        SetInventory("RLScavengerModLimit", 0);
        SetInventory("RLArmorModItemInInventory", 0);
        SetInventory("RLSkullLimit", 0);
        SetInventory("RLPhaseDeviceLimit", 0);
    }

    if (InMultiplayer && GetCVar("drpg_multi_revives"))
        ReviveHandler();

    if (Player.FocusingCooldown > 0)
        Player.FocusingCooldown--;

    // Loop
    Delay(1);

    goto Start;
}

// Health Handler Entry Point
NamedScript void PlayerHealth()
{
    int BeforeHealth;
    int AfterHealth;

    while (true)
    {
        BeforeHealth = GetActorProperty(0, APROP_Health);

        // If the player's dead, terminate
        if (BeforeHealth <= 0) break;

        Delay(1);

        AfterHealth = GetActorProperty(0, APROP_Health);

        if (AfterHealth > BeforeHealth)
            Player.ActualHealth += AfterHealth - BeforeHealth;

        // Update health
        SetActorProperty(0, APROP_Health, Player.ActualHealth);
    }
}

NamedScript void PlayerSurvive()
{
    while (true)
    {
        Player.CanSurvive = (Random(0, 100) <= Player.SurvivalBonus);
        if (Player.CanSurvive || CheckInventory("DRPGLife"))
            SetPlayerProperty(0, 1, PROP_BUDDHA);
        else
            SetPlayerProperty(0, 0, PROP_BUDDHA);
        Delay(35);
    }
}

// Damage Handler Entry Point
NamedScript DECORATE int PlayerDamage(int Inflictor, int DamageTaken)
{
    bool Critical;
    int MonsterID;
    fixed LuckChance;
    fixed EnergyLevel;

    // Assign attacker's ID
    Player.DamageTID = Inflictor;

    Player.AutosaveTimerReset = true;

    // Find Monster ID
    MonsterID = FindMonster(Player.DamageTID);

    // Calculate monster crit/status chance
    LuckChance = (fixed)Monsters[MonsterID].Luck / 20.0;
    EnergyLevel = (fixed)Monsters[MonsterID].Energy / 12.5;

    // Calculate a critical hit
    if (Player.DamageTID > 0 && MonsterID > 0)
        if (RandomFixed(0.0, 100.0) <= EnergyLevel)
        {
            DamageTaken *= 2.0;
            Critical = true;
        }

    AugDamage(DamageTaken);
    ToxicityDamage();

    if (MonsterID > 0)
        StatusDamage(DamageTaken, LuckChance, Critical);
    else
        StatusDamage(DamageTaken, RandomFixed(0.0, 100.0 * MapLevelModifier), Critical);

    ResetRegen();
    DamageHUD(DamageTaken, Critical);

    Player.DamageType = DT_NONE;

    Player.ActualHealth -= DamageTaken;
    // Receiving damage to health interrupts focusing
    Player.Focusing = false;

    if (Player.ActualHealth <= 1) // Near-Death stuff
    {
        // Extra Life check
        if (!Player.CanSurvive && CheckInventory("DRPGLife"))
        {
            Player.ActualHealth = Player.HealthMax;
            ActivatorSound("health/resurrect", 127);
            if (!CurrentLevel->UACBase)
            {
                SetInventory("ArtiTeleport", 1);
                UseInventory("ArtiTeleport");
            }
            TakeInventory("DRPGLife", 1);

            SetHudSize(320, 200, false);
            SetFont("BIGFONT");
            HudMessage("Used an Extra Life!");
            EndHudMessage(HUDMSG_FADEOUT, 0, "Gold", 160.0, 140.0, 0.5, 1.5);
            PrintSpriteFade("P1UPA0", 0, 172.0, 210.0, 0.5, 1.5);

            return 0;
        }

        // Survival Bonus
        if (Player.CanSurvive)
        {
            Player.ActualHealth = Player.HealthMax / 5;

            SetInventory("DRPGInvulnerabilitySurvive", 1);

            if (Player.Shield.Accessory && Player.Shield.Accessory->PassiveEffect == SHIELD_PASS_SURVIVECHARGE)
                Player.Shield.Charge = Player.Shield.Capacity;

            ActivatorSound("health/survive", 127);
            SetHudSize(320, 200, false);
            SetFont("BIGFONT");
            HudMessage("Agility Save!");
            EndHudMessage(HUDMSG_FADEOUT, 0, "Orange", 160.0, 140.0, 0.5, 0.5);
            PrintSpriteFade("AGISAVE", 0, 160.0, 140.0, 0.5, 0.5);

            return 0;
        }
    }

    SetActorProperty(0, APROP_Health, Player.ActualHealth);

    return 0;
}

// Shield Damage Handler Entry Point
NamedScript DECORATE int ShieldDamage(int DamageTaken)
{
    int ShieldDamageAmount;

    ShieldTimerReset();

    if (Player.Shield.Charge > 0)
    {
        Player.AutosaveTimerReset = true;
        AugDamage(DamageTaken);
        ToxicityDamage();
        StatusDamage(DamageTaken, RandomFixed(0.0, 10.0 * MapLevelModifier), false);
        DamageHUD(DamageTaken, false);

        ShieldDamageAmount = DamageTaken; // For callback
        if (ShieldDamageAmount > Player.Shield.Charge)
            ShieldDamageAmount = Player.Shield.Charge;

        Player.Shield.Charge -= DamageTaken;
        Player.Shield.Full = false;

        if (GetActivatorCVar("drpg_shield_effect_hit_enable"))
        {
            FadeRange(0, 100, 255, 0.25, 0, 100, 255, 0, 0.25);
            PlaySound(0, "shield/hit", 5, 1.0, false, 1.0);
        }
        if (Player.Shield.Accessory && Player.Shield.Accessory->Damage)
            Player.Shield.Accessory->Damage(ShieldDamageAmount);

        if (Player.Shield.Charge <= 0)
        {
            if (Player.Shield.Charge < 0)
            {
                DamageTaken = -Player.Shield.Charge;
                Player.Shield.Charge = 0;
            }
            else
                DamageTaken = 0;

            if (GetActivatorCVar("drpg_shield_sound_empty_enable"))
                PlaySound(0, "shield/empty", 5, 1.0, false, 1.0);

            if (Player.Shield.Accessory && Player.Shield.Accessory->Break)
                Player.Shield.Accessory->Break();
        }
        else
            DamageTaken = 0;
    }

    return DamageTaken;
}

NamedScript void MoneyChecker()
{
    int BeforeCredits;
    int AfterCredits;

Start:

    BeforeCredits = CheckInventory("DRPGCredits");

    Delay(1);

    AfterCredits = CheckInventory("DRPGCredits");

    goto Start;
}

NamedScript DECORATE int AddHealth(int HealthPercent, int MaxPercent)
{
    int RealMax = Player.HealthMax * MaxPercent / 100;
    int HealthAmount = Player.HealthMax * HealthPercent / 100;

    if (HealthPercent > 0) //to make this consistent with how AddHealthDirect handles non-positive values
    {
        if (Player.OverHeal)
            if (MaxPercent >= 200 && Player.ActualHealth + HealthAmount >= RealMax)
                Player.OverHeal = false;

        if (HealthAmount > 0 && Player.ActualHealth >= RealMax)
            return 0;

        if (Player.ActualHealth + HealthAmount > RealMax)
            HealthAmount = RealMax - Player.ActualHealth;

        // Add Vitality XP for using healing items
        if (GetCVar("drpg_levelup_natural"))
        {
            fixed Scale = GetCVarFixed("drpg_vitality_scalexp") / GetCVar("drpg_ws_use_wads");
            if (GetCVar("drpg_allow_spec"))
            {
                if (GetActivatorCVar("drpg_character_spec") == 3)
                    Scale *= 2;
            }

            int Factor = CalcPercent(HealthAmount, Player.HealthMax);
            Player.VitalityXP += (RoundInt)(Factor * Scale * 10);
        }
    }

    Player.ActualHealth += HealthAmount;
    HealthPercent = HealthAmount * 100 / Player.HealthMax;
    return HealthPercent;
}

NamedScript DECORATE int AddHealthDirect(int HealthAmount, int MaxPercent)
{
    int RealMax = Player.HealthMax * MaxPercent / 100;

    if (HealthAmount > 0) //basically none of this applies if you're being directly hurt, or if HealthAmount is zero somehow
    {
        if (Player.OverHeal)
            if (MaxPercent >= 200 && Player.ActualHealth + HealthAmount >= RealMax)
                Player.OverHeal = false;

        if (Player.ActualHealth >= RealMax)
            return 0;

        if (Player.ActualHealth + HealthAmount > RealMax)
            HealthAmount = RealMax - Player.ActualHealth;

        // Add Vitality XP for using healing items
        if (GetCVar("drpg_levelup_natural"))
        {
            fixed Scale = GetCVarFixed("drpg_vitality_scalexp") / GetCVar("drpg_ws_use_wads");
            if (GetCVar("drpg_allow_spec"))
            {
                if (GetActivatorCVar("drpg_character_spec") == 3)
                    Scale *= 2;
            }

            int Factor = CalcPercent(HealthAmount, Player.HealthMax);
            Player.VitalityXP += (RoundInt)(Factor * Scale * 10);
        }
    }

    Player.ActualHealth += HealthAmount;
    return HealthAmount;
}

typedef struct
{
    str Title;
    str Tip;
} TipInfo;

NamedScript void GiveTip()
{
    if ((CurrentLevel->Event == MAPEVENT_SKILL_TECHNOPHOBIA && GameSkill() != 6) || (CurrentLevel->Event == MAPEVENT_SKILL_ARMAGEDDON && GameSkill() != 7))
        return;

    TipInfo Tips[MAX_TIPS] =
    {
        // Level/Rank
        { "\CjLevel",                           "Your level determines your general competency and progress through the game. Your level is increased by gathering XP by killing monsters, completing missions, and achieving combos.", },
        { "\CkRank",                            "Your rank determines your standing within the UAC. Rank will determine how much money you are paid by the UAC as well as the discount you will receive when executing financial operations at the Outpost. Your rank is increased by killing monsters, completing missions, achieving combos, and engaging in certain secret activities.", },
        { "\CtCombo",                           "The combo system allows you to kill enemies in rapid succession in order to gain bonus XP and rank. When an enemy is killed, you will gain +1 to your combo counter, and your combo timer will reset. When your timer reaches its cooldown point, indicated by the split between the red and green in the bar, it will add the indicated XP and rank to your totals and calculate a bonus (the green number). If you keep incrementing your combo during the cooldown period, you can keep stacking XP and rank into the bonus. When the combo timer ends, your combo will be completely reset, and your bonus will be added to your totals.", },

        // Currencies
        { "\CfCredits",                         "Credits are the universal currency of the UAC and are used for almost all actions at the Outpost that include obtaining goods and services.", },
        { "\CdModules",                         "Modules are used to upgrade your stats and skills. They are typically given by level ups, missions, and completing level events, as well as occasionally being dropped by monsters and found in the world.", },
        { "\CcTurret Parts",                    "Turret parts are used to build and upgrade your turret. They can be found around the world by disassembling broken turrets and lying freely.", },

        // Stats
        { "\CgStrength",                        "Your strength stat determines the extra damage you deal with all types of attacks.", },
        { "\CdDefense",                         "Your defense stat determines your resistance to all forms of damage.", },
        { "\CaVitality",                        "Your vitality stat determines your max HP, HP regeneration amount, and resistance to status effects.", },
        { "\CnEnergy",                          "Your energy stat determines your max EP, EP regeneration amount, and how many and for how long your auras last.", },
        { "\CtRegeneration",                    "Your regeneration stat determines your EP and HP regeneration rates, toxicity dissipation, and the time that regeneration spheres last.", },
        { "\CiAgility",                         "Your agility stat governs your movement, jumping, and fire speeds as well as your survival bonus.", },
        { "\ChCapacity",                        "Your capacity stat determines your ammo carrying capacity, how many compounds of each type you can carry for stims, as well as how many various inventory items you can hold.", },
        { "\CfLuck",                            "Your luck stat determines which items can drop as well as the chance that they will drop. Luck also affects several other hidden factors as well.", },

        // Stats Extended
        { "\CiSurvival Bonus",                  "Survival bonus is a special attribute that works as a last stand mechanic. When you take a hit that would kill you and your survival bonus is triggered, you will not be killed by the attack and instead be instantly revived with part of your HP and EP restored.", },
        { "\CdPerks",                           "Perks are special enhancements that you receive when you upgrade a stat to 100 points or more.", },
        { "\CgStat Cap",                        "You can only increase a stat past 100 up to the maximum of your current level + 100.", },

        // Skills
        { "\CnEP",                              "Energy points (EP) are needed in order to use skills. They can also be used to charge your shield and to deposit and withdraw items from your personal locker while not in the Outpost.", },
        { "\CnOverdrive",                       StrParam("You can overdrive a skill by holding the \Cd%jS\C- button. Doing this will always use the skill, regardless of its EP cost. However, this will bring your current EP into the negatives. When negative, you will suffer stat penalties and not be able to use any skills until your EP is restored past 0.", "+speed"), },
        { "\CnAuras",                           "Auras are special passive abilities you can activate to give you temporary boosts to your ability. Auras will also affect teammates within their respective radii, as determined by your energy stat.", },

        // Augmentations
        { "\CkAugmentations",                   "Augmentations give you permanent bonuses to your ability while they are active.", },
        { "\CkAugmentation Battery",            "Augmentations can only work while your augmentation battery has a charge. Your battery can be recharged by using augmentation batteries from your inventory or by receiving certain types of damage. Battery drain is determined by how many augmentations you have active and how high their levels are. The more augmentations that are active and the higher their levels, the quicker the battery will drain.", },
        { "\CkAugmentation Slots",              "You can only activate as many augmentations as you have available slots for. To increase the number of slots you have, you must obtain an augmentation slot upgrade. Augmentation slot upgrades can be bought from the shop, found as rare drops from enemies, or found in the world.", },

        // Shield
        { "\CvShields",                         "Shields will protect you from direct damage by absorbing the damage into their own charge pool. When a shield takes damage, its charge is directly affected and will be reduced by the damage's amount. When a shield's charge reaches 0, you will begin taking normal damage again.", },
        { "\CvShield Components",               "Shields are built from 4 basic components, which you can collect and customize during the game. The body is named after the manufacturer and is generally responsible for modifying the capacity of the shield, but it also evenly modifies the other stats as well. The battery is generally responsible for modifying the charge rate of the shield. The capacitor is generally responsible for modifying the charge rate and delay rate of the shield. Accessories give the shield unique effects and abilities, which can be triggered in various ways.", },
        { "\CvShield Stats",                    "Each shield has 4 stats that govern its behavior and abilities. The capacity determines the total amount of charge the shield can hold. The charge rate determines the amount the charge pool will regenerate when a charge cycle is completed. The delay rate determines the wait time that occurs when a charge cycle is interrupted before charging will resume. The charge interval determines the length between charge cycles.", },
        { "\CvShield Charge Cycle",             "A charging cycle occurs based on the charge interval (by default, 1 second) and increases the shield's charge pool, which is determined by the charge rate. When the shield is struck, the charging cycle is interrupted, and a waiting period, determined by the delay rate, must complete before charging cycles will resume again.", },
        { "\CvQuick Shield Recharge",           StrParam("You can quickly recharge your shield at the cost of EP by holding \Cd%jS\C- + \Cd%jS\C-.", "+speed", "+use"), },

        // Stims
        { "\CcStims",                           "Stims are portable, injected temporary increases in ability. There are three types of stim vials that can be encountered: stat vials, booster vials, and powerup vials.", },
        { "\CcStim Injectors",                  "Injectors are what are used to actually store the compounds for use. Injectors can either be bought in the shop or found around the world. There are 4 different injector sizes, which hold varying amounts of compounds.", },
        { "\CcStim Vials",                      "Vials are the items that contain the different kinds of compounds. Vials can be bought in the shop or found by killing enemies that have auras or elsewhere in the world.", },
        { "\CcStim Stat Vials",                 "Stat vials increase their various corresponding stats when used.", },
        { "\CcStim Booster Vials",              "Booster vials are special vials that have unique stat related effects. Purifier vials increase the duration that stat vials will last. Potency vials increase all stats at once.", },
        { "\CcStim Powerup Vials",              "Powerup vials grant various special powers for a limited duration. Unlike the stat stims, they are unaffected by purifier vials, and their time is affected solely by the amount of vials put into the stim.", },
        { "\CdToxicity",                        "Toxicity is a measure of the radiation and toxins in your body. Toxicity is increased by exposure to radiation or toxin related damage, such as standing on damage floors. The higher your toxicity levels get, the more penalties you will suffer. If your toxicity levels reach 100%, you will die. Toxicity will naturally dissipate over time, the speed of which is determined by your regeneration stat.", },

        // Turrets
        { "\CdTurrets",                         "Turrets are portable sentry drones that can be built and equipped with multiple offensive and defensive upgrades. Turrets must initially be built from turret parts, at which point the turret may be used and upgraded further as more parts become available.", },
        { "\CdDestroyed Turrets",               "In the world, you will find destroyed turrets. These turrets can be scavenged for spare turret parts. Some turrets will be in better condition than others, allowing for more parts to be scavenged.", },
        { "\CdTurret Maintenance",              StrParam("Your turret will need routine maintenance performed on it in order to keep it in working condition. There are three components to maintenance: charging, repairing, and refitting. Charging will charge the turret's internal battery, allowing it to stay active on the field. Repairing the turret will patch it up, restoring its durability. Refitting occurs when upgrades are performed on the turret and must be allowed to finish before the turret can be used again. In order for maintenance to begin and continue, a steady supply of credits will be deducted from your account. To send the turret in for maintenance, in either the turret menu or with the turret command wheel open, press \Cd%jS\C-.", "+speed"), },
        { "\CdTurret Deployment",               StrParam("To quickly deploy or deactivate your turret, use \Cd%jS\C- + \Cd%jS\C-.", "+speed", "+user2"), },

        // Monsters
        { "\CgMonster Stats",                   "Like you, monsters also have a level and stats, which will affect their abilities in various ways.", },
        { "\CgMonster Auras",                   "Like you, monsters can also have one or more auras, which will give them special abilities. Each aura a monster has will also double its respective stat.", },
        { "\CgMonster Shadow Auras",            "Monsters that have all possible auras active at once will become shadow enemies with extremely high stats and abilities for their level. These enemies will receive names and should be priority targets to take down, as their combined abilities can quickly overwhelm you if you are not prepared.", },
        { "\CgMonster Regeneration Cycle",      "Like you, monsters will regenerate a portion of their HP every so often. The amount of HP they will regenerate is determined by their regeneration stat.", },
        { "\CgMonster Threat Level",            "Each monster has a threat level. Their threat level is determined by their level, stats, and active auras, and ranges from 0 to 10. Threat level can be used as a general way to gauge a monster's abilities and which ones should be prioritized when attacking. The higher a monster's threat level, the more XP and rank you will receive for defeating it. The threat level is represented on a monster's health bar by the number of emblems present.", },
        { "\CgMegabosses",                      "Under certain circumstances, you will encounter monsters known as megabosses. These bosses are extremely difficult, having special and unique behaviors that can quickly and effectively destroy you if you are not prepared. When killed, megabosses will provide a tremendous amount of loot as well as XP and rank.", },

        // Status Effects
        { "\CaStatus Effects",                  "Status effects are debuffs that will affect you in different ways. You can get them from taking different types of damage or by exposing yourself to specific types of hazards. When inflicted with a status effect, it will last for a certain amount of time. Each status effect has 5 levels of intensity, which are increasingly detrimental to you. If you are hit with a status effect you already have, it will add to your current timer and potentially increase its intensity.", },
        { "\CcBlindness",                       "Blindness is a status effect that will darken your view and make it harder to see.", },
        { "\CfConfusion",                       "Confusion is a status effect that will make your vision blurry and make it harder for you to aim and move.", },
        { "\CqPoison",                          "Poison is a status effect that deals direct damage to you every second. It will not actually kill you if your HP reaches a critical level.", },
        { "\CdCorrosion",                       "Corrosion is a status effect that will deal damage to your armor every second.", },
        { "\CiFatigue",                         "Fatigue is a status effect that will decrease your movement speed and jumping height.", },
        { "\CtVirus",                           "Virus is a status effect that will stop you from regenerating altogether.", },
        { "\CnSilence",                         "Silence is a status effect that prevents you from using skills.", },
        { "\CgCurse",                           "Curse is a status effect that increases the amount of damage you take.", },
        { "\CvEMP",                             "EMP is a status effect that will completely disable your shields and augmentations and not allow you to reenable them. It will also drain your augmentation battery.", },
        { "\CdRadiation",                       "Radiation is a status effect that slowly increases your toxicity levels. It will not increase your toxicity past 85%, however.", },

        // Outpost/Arena
        { "\CqOutpost",                         "The UAC Outpost is the main hub of operations where you can perform various activities, access various facilities, and stock up between levels. You can transport to the Outpost at any time using the Transport skill.", },
        { "\CgArena",                           "In the Arena, you can fight waves of enemies for XP, rank, and loot.", },
        { "\CdLevel Transporter",               "In the Outpost, you can use the level transporter to view stats on all the levels you have visited and to transport back to them at will.", },
        { "\CdSkill Computer",                  "In the Outpost, you can use the skill computer to change the current skill level without the need to restart the game.", },
        { "\CdMinigames",                       "In the Outpost, there are minigames that you can play in order to win items using your gold and platinum chips.", },
        { "\CgPractice Target",                 "In the Outpost, you can shoot the target in order to gauge the amount of damage your weapons are doing.", },

        // Missions
        { "\CdMissions",                        "You can accept different missions in the Outpost in order to gain XP, rank, credits, modules, and items. There are several different types of missions at varying difficulty levels you can undertake. The list of available missions will refresh each time you visit the Outpost.", },
        { "\CdCollection Missions",             "You must collect a specific number of a specified item to complete the mission. These items are scattered around the world.", },
        { "\CaKill Missions",                   "You must kill a given number of enemies of the same type to complete the mission.", },
        { "\CaKill Auras Missions",             "You must kill a given amount of enemies who have at least one aura to complete the mission.", },
        { "\CaKill Reinforcements Missions",    "While in the field, monsters will spawn around you. You must kill a given number of them to complete the mission.", },
        { "\CgAssassination Missions",          "You must eliminate the specified shadow enemy, located somewhere in the level, to complete the mission.", },
        { "\CkFind Secrets Missions",           "You must find the given number of unique secrets to complete the mission.", },
        { "\CnFind Items Missions",             "You must find the given number of items to complete the mission.", },
        { "\CtAchieve Combo Missions",          "You must get your combo to the given amount to complete the mission.", },

        // Shop/Locker
        { "\CfShop",                            "The shop is where you can go to purchase and sell the various equipment and items you can find during the game. The shop also contains a locker system, where you may withdraw and deposit items for later usage.", },
        { "\ChLocker",                          StrParam("You can access the locker system via the shop and deposit or withdraw your items there. Withdrawing and depositing items has a 1%% cost to your EP per use outside the Outpost. You can switch between the shop and locker using the \Cd%jS\C- key.", "+jump"), },
        { "\CfShop Cards",                      "You may find UAC cards that provide you with a discount as well as other benefits, like access to certain equipment. Finding a new card will replace your old card with the upgraded version. The discount benefits from a card will only apply when shopping at the Outpost.", },

        // Level Events
        { "\CdLevel Events",                    "When entering a level, sometimes unique, levelwide events will happen. Each event has its own unique characteristics and conditions for ending the event.", },
        { "\CdRolling Level Events",            "Level events will not only occur when entering a level, but previous levels that you have visited will also sometimes contain events and require revisiting in order to participate in the event.", },
    };

    TipInfo const EventTips[MAPEVENT_MAX] =
    {
        { "", "", }, // Never used

        { "\CgMegaboss Event",                  "All of the hostiles have vacated the area, and a very powerful creature is lurking about. Return to the Outpost if needed, and make sure you have the power and munitions to take down the death machine.", },
        { "\CqEnvironmental Hazard Event",      "The area is covered in lethal radiation, but there is also a radiation neutralizer to fix the problem. Gather fuel and refill the neutralizer whenever it runs low, until the radiation is brought down to safe levels.", },
        { "\CiThermonuclear Bomb Event",        "Find as many keys as you can to disable the countdown. Worse comes to worst, you can hack through the key slots by using the bomb without keys, but it's going to take time to do so for each key missing, as indicated by the HUD. And make sure the bomb doesn't get shot!", },
        { "\CuLow Power Event",                 "The UAC had a major battle in this area, and the discards were left behind when the area was damaged. Pluck some extra items out of the wreckage before they come back to clean up. You can even get some extra credits and modules if you restore the backup power before you leave.", },
        { "\CdAll Auras Event",                 "All of the monsters have rather deadly magic auras. Understand their weaknesses and attempt to stay away from some of them, as they are dangerous.", },
        { "\CnOne Monster Event",               "There is one species of demon currently occupying the area. Clearing the area of hostiles will net you a bonus.", },
        { "\CgHell Unleashed Event",            "Open the box and partake in the rare contents within, but be warned: you will anger the demonic army, and they will pursue you endlessly until you are killed or leave the area. The longer you stay, the more likely they are to drop items, but the deadlier they will become.", },
        { "\CdHarmonized Destruction Event",    "All of the enemies in the area have the same type of aura. Get accustomed to the aura's strengths and weaknesses and use them to your advantage. Don't get caught off guard, as some monsters can and will have other auras as well.", },
        { "\CdCracks in the Veil Event",        "The area is spatially unstable. Avoid stepping into the portals, as they will teleport you to other points in the area at random. Enemies will be teleported in as well.", },
        { "\Cg12 Hours 'til Doomsday Event",    "Hell has claimed the area! Flee while you still can.", },
        { "\CqVicious Downpour",                "It's raining acidic substances. It's best to wear a radiation suit if you plan to head outdoors.", },
        { "\CtThe Dark Zone",                   "This is no ordinary mist. The evil fog will slowly sap away the light from its surroundings. Once that happens, enemies will begin to resurrect and become infused with shadow auras. Sticking around here for too long is not recommended without preparation.", },

        { "\CaC       O     N   S    U    M    E         \CgY   O     U", "Tip: Run.", },
        { "\CmWhispers of Darkness",            "\Cg\"He who fights with monsters should see to it that he himself does not become one. \CaAnd when you gaze long into an abyss, \Cuthe abyss also gazes into you.\"", },


        { "\CgR\CiA\CkI\CdN\ChB\CtO\CaW\CjS",   "SHE BROKE THE REALITY GENERATOR AGAIN! WE'RE DOOMED!", },

        { "\CaHell Event",                      "The difficulty level has increased to \CaHell\C- for this area.", },
        { "\CmArmageddon Event",                "The difficulty level has increased to \CmArmageddon\C- for this area.", },

        { "\CgSinstorm",                        "You're getting close to the source of the invasion. The demonic legion is pulling out all of its stops. Expect frequent battles with shadow enemies and reinforcements to pour in relentlessly. Be careful when traversing outdoors, as the fire rain will harm you as well.", },
    };

    int TipID = Random(0, MAX_TIPS - 1);
    fixed HoldTime = StrLen(Tips[TipID].Tip) * 0.1;

    str TipTitle = Tips[TipID].Title;
    str TipText = Tips[TipID].Tip;

    if (CurrentLevel->Event != MAPEVENT_NONE && !Player.SeenEventTip[CurrentLevel->Event])
    {
        TipTitle = EventTips[CurrentLevel->Event].Title;
        TipText = EventTips[CurrentLevel->Event].Tip;
        HoldTime = StrLen(Tips[TipID].Tip) * 0.1;

        Player.SeenEventTip[CurrentLevel->Event] = true;
    }

    SetHudSize(0, 0, false);
    // Shove it in the console as one line
    HudMessage("%S\n\n\C-%S", TipTitle, TipText);
    EndHudMessage(HUDMSG_PLAIN | HUDMSG_LOG, 9998, "White", 0, 0, 1);
    HudMessage("");
    EndHudMessage(HUDMSG_PLAIN, 9998, "White", 0, 0, 1);
    SetFont("BIGFONT");
    HudMessage("%S", TipTitle);
    EndHudMessage(HUDMSG_FADEOUT, MAKE_ID('1','T','I','P'), "White", 0.5, 0.7, HoldTime, 3.0);
    SetFont("SMALLFONT");
    HudMessage("%S", TipText);
    EndHudMessage(HUDMSG_FADEOUT, MAKE_ID('2','T','I','P'), "White", 1.5, -0.725, HoldTime, 3.0);
}

// Handles Weapon Firing Speed
NamedScript void WeaponSpeed()
{
    int Time;

Start:

    // Compatibility Handling - DoomRL Arsenal
    if (CompatMode == COMPAT_DRLA)
        while (CheckInventory("RLWeaponDrop"))
            Delay(35);

    if (Player.Stim.PowerupTimer[STIM_RAGE] > 0 || Player.WeaponSpeed >= 100 || Player.WeaponSpeed <= 0)
        Time = 4;
    else
        Time = (int)(4 * (100.0 / Player.WeaponSpeed - 0.05)) + 1;

    if (Player.Stim.PowerupTimer[STIM_RAGE] > 0 || (Player.WeaponSpeed > 0 && GetActivatorCVar("drpg_stat_weaponspeed")))
        SetInventory("DRPGSpeed", 1);

    Delay(Time);
    goto Start;
}

// Stat Randomizer Script
NamedScript void StatRandomizer()
{
Start:

    if (GetActivatorCVar("drpg_auto_spend"))
    {
        while (CheckInventory("DRPGModule") > 0 && !StatsCapped())
        {
            // Select Preferred Stat
            if (Random(1, 2) == 1 && GetActivatorCVar("drpg_auto_spend_pref") >= 0)
                IncreaseStat(GetActivatorCVar("drpg_auto_spend_pref"));
            else // Select Random Stat
                IncreaseStat(Random(0, STAT_MAX - 1));

            Delay(1);
        }
    }

    Delay(1);
    goto Start;
}

// Handles autosaving
NamedScript void AutosaveHandler()
{
    // Terminate if the autosave CVar is disabled
    if (GetCVar("drpg_autosave") == 0 || (PlayerCount() > 1 && !Arbitrator)) return;

    int SaveTimer = GetCVar("drpg_autosave") * (35 * 60);
    bool Safe;
    int DamageTimer;

    while (true)
    {
        if (Player.AutosaveTimerReset)
            DamageTimer = 0;
        else
            ++DamageTimer;
        Player.AutosaveTimerReset = false;

        if (Player.InMenu || Player.InShop || Player.OutpostMenu > 0)
            DamageTimer = ASAVE_SAFETIME;

        if (!SaveTimer)
        {
            Safe = true;

            if (DamageTimer < ASAVE_SAFETIME ||
                    GetActorProperty(Player.TID, APROP_Health) <= GetActorProperty(Player.TID, APROP_SpawnHealth) / 10)
                Safe = false;

            if (Safe)
            {
                Autosave();
                SaveTimer = GetCVar("drpg_autosave") * (35 * 60);
            }
            else
                SaveTimer = ASAVE_RETRYTIME;
        }
        else
            --SaveTimer;

        Delay(1);
    }
}

NamedScript Type_OPEN void ShopSpecialHandler()
{
    bool ValidItem;
    int Tries, MinValue, MaxValue, Category, Index;
Start:

    // Reset the item
    if (ShopSpecialTimer <= 0)
    {
        ValidItem = false;
        Tries = 0;
        MinValue = 0;
        MaxValue = 0;
        Category = 0;
        Index = 0;

        // Calculate min and max based on settings
        switch (GetCVar("drpg_shopspecial_type"))
        {
        case SHOPSPECIAL_MINMAX:
            MinValue = GetCVar("drpg_shopspecial_min");
            MaxValue = GetCVar("drpg_shopspecial_max");
            break;
        case SHOPSPECIAL_LEVEL:
            MinValue = AveragePlayerLevel() * MAX_LEVEL;
            MaxValue = 1000 + AveragePlayerLevel() * (20 * MAX_LEVEL);
            break;
        case SHOPSPECIAL_RANK:
            MinValue = AveragePlayerRank() * (20 * MAX_RANK);
            MaxValue = 1000 + AveragePlayerRank() * (350 * MAX_RANK);
            break;
        case SHOPSPECIAL_CREDITS:
            MinValue = AveragePlayerCredits() / 20;
            MaxValue = 1000 + AveragePlayerCredits() * 2;
            break;
        case SHOPSPECIAL_LUCK:
            MinValue = AveragePlayerLuck() * 100;
            MaxValue = 1000 + AveragePlayerLuck() * 2000;
            break;
        }
        if (DebugLog)
            Log("\CdDEBUG: Shop Special \C-Min/Max Calculated: \Ca%d \C-/ \Cd%d", MinValue, MaxValue);

        // Blank out the item until a new one is found
        ShopSpecialItem = GetBlankItem();

        while (!ValidItem)
        {
            ValidItem = true;

            Delay(1);

            // Pick an item
            Category = (GetCVar("drpg_shopspecial_category") >= 0 ? GetCVar("drpg_shopspecial_category") : Random(0, (ItemCategories - 1)));
            Index = Random(0, ItemMax[Category] - 1);

            // We're not having any luck meeting the criteria, or the min/max value are impossible values to meet (0 / 0)
            // Break and give up on generating a shop special this time
            if (Tries > 100 || (MinValue == 0 && MaxValue == 0))
            {
                ValidItem = false;
                break;
            }

            // Retry if you land on an item with no cost
            if (ItemData[Category][Index].Price == 0)
            {
                ValidItem = false;
                Tries++;
                continue;
            }

            // Skip Loot category entirely
            if (Category == 7)
            {
                ValidItem = false;
                Tries++;
                continue;
            }

            // Skip Vial category entirely
            if (Category == 6 && Index < 24)
            {
                ValidItem = false;
                Tries++;
                continue;
            }

            // Skip Ammo category entirely
            if (Category == 1 && Index < 8)
            {
                ValidItem = false;
                Tries++;
                continue;
            }

            // Skip Armor Bonus
            if (Category == 3 && Index == 0)
            {
                ValidItem = false;
                Tries++;
                continue;
            }

            // Skip Thermonuclear Bomb and Extra Life
            if (Category == 4 && (Index == 17 || Index == 18))
            {
                ValidItem = false;
                Tries++;
                continue;
            }

            // Skip over high rarity items
            if (ItemData[Category][Index].Rarity > 4 + RoundInt(8.0 * MapLevelModifier))
            {
                ValidItem = false;
                Tries++;
                continue;
            }

            // Make sure the item is within the valid price range
            if (ItemData[Category][Index].Price < MinValue || ItemData[Category][Index].Price > MaxValue)
            {
                ValidItem = false;
                Tries++;
                continue;
            }

            // Item is valid, continue
            if (ValidItem)
                break;
        }

        if (ValidItem)
        {
            // Assign the item to the Shop Special item
            ShopSpecialItem = &ItemData[Category][Index];

            // Spawn the item if you're in the Outpost
            SpawnShopSpecialItem();
        }

        // Reset the timer and bought status
        ShopSpecialTimer = 35 * 60 * GetCVar("drpg_shopspecial_time");
        ShopSpecialBought = false;

        if (DebugLog)
        {
            if (ShopSpecialItem == GetBlankItem())
                Log("\CdDEBUG: Shop Special expired! \CaNo new item generated");
            else
                Log("\CdDEBUG: Shop Special expired! Now \Cj%S", ShopSpecialItem->Name);
        }
    }

    // Remove the item if it was bought
    if (CurrentLevel->UACBase && ShopSpecialBought)
        Thing_Remove(ShopSpecialTID + 1);

    // Decrease the timer
    ShopSpecialTimer--;

    Delay(1);
    goto Start;
}

// Handles the Items dynamic array and associated behavior
NamedScript void ItemHandler()
{
    fixed Dist, Height, Divisor, Angle, Pitch, Amount, X, Y, Z;
    int ItemTID;
    bool NoClip;

    // Create the Items Array
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        ItemTIDs[i] = -1;

        if ((i % 15000) == 0)
            Delay(1);
    }

    // Items array initialization complete
    ItemTIDsInitialized = true;

    while (ItemTIDsInitialized && !CurrentLevel->UACBase)
    {
        for (int i = 0; i < MAX_ITEMS; i++)
        {
            // We've hit the end of the array
            if (ItemTIDs[i] == -1) break;

            // Invalid item, continue
            if (ItemTIDs[i] == 0) continue;

            // This item was picked up or otherwise removed
            if (ClassifyActor(ItemTIDs[i]) == ACTOR_NONE || ClassifyActor(ItemTIDs[i]) == ACTOR_WORLD) continue;

            NoClip = false;
            // Iterate players and check distances
            for (int j = 0; j < MAX_PLAYERS; j++)
            {
                // Skip this player if they aren't in the game
                if (!PlayerInGame(j)) continue;

                // Skip this player if they aren't Magnetic
                if (Players(j).Stim.PowerupTimer[STIM_MAGNETIC] <= 0) continue;

                // Skip this player if they are dead
                if (GetActorProperty(Players(j).TID, APROP_Health) <= 0) continue;

                Dist = Distance(ItemTIDs[i], Players(j).TID);
                Height = GetActorPropertyFixed(Players(j).TID, APROP_Height);
                Divisor = (Dist - 16.0) + Dist;
                Angle = VectorAngle (GetActorX(Players(j).TID) - GetActorX(ItemTIDs[i]), GetActorY(Players(j).TID) - GetActorY(ItemTIDs[i]));
                Pitch = VectorAngle (Dist, GetActorZ(Players(j).TID) - GetActorZ(ItemTIDs[i]));

                // Calculate the amount based on close versus far distance
                if (Dist < 16.0)
                    Amount = 1.0;
                else
                    Amount = 16.0 / Divisor;

                // Calculate the lerped positions
                X = (Amount * 16.0) * Cos(Angle) * Cos(Pitch);
                Y = (Amount * 16.0) * Sin(Angle) * Cos(Pitch);
                Z = (Amount * 16.0) * Sin(Pitch);

                SetActorVelocity(ItemTIDs[i], X, Y, Z, true, false);
                NoClip = true;
            }

            // Enable/Disable clipping on this item
            if (NoClip)
            {
                SetActorFlag(ItemTIDs[i], "NOCLIP", true);
                SetActorFlag(ItemTIDs[i], "NOGRAVITY", true);
                SetActorFlag(ItemTIDs[i], "NOTELEPORT", true);
                SetActorFlag(ItemTIDs[i], "VISIBILITYPULSE", true);
                SetActorFlag(ItemTIDs[i], "BRIGHT", true);
            }
            else
            {
                SetActorFlag(ItemTIDs[i], "NOCLIP", false);
                SetActorFlag(ItemTIDs[i], "NOGRAVITY", false);
                SetActorFlag(ItemTIDs[i], "NOTELEPORT", false);
                SetActorFlag(ItemTIDs[i], "VISIBILITYPULSE", false);
                SetActorFlag(ItemTIDs[i], "BRIGHT", false);
            }
        }

        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            // Skip this player if they aren't in the game
            if (!PlayerInGame(i)) continue;

            // Skip this player if they aren't Magnetic
            if (Players(i).Stim.PowerupTimer[STIM_MAGNETIC] <= 0) continue;

            // Skip this player if they are dead
            if (GetActorProperty(Players(i).TID, APROP_Health) <= 0) continue;


            int *ItemTID = (int *)Player.DropTID.Data;

            for (int j = 0; j < Players(i).DropTID.Position; j++)
            {
                NoClip = false;
                if (ClassifyActor(ItemTID[j]) == ACTOR_NONE || ClassifyActor(ItemTID[j]) == ACTOR_WORLD) continue;

                Dist = Distance(ItemTID[j], Players(i).TID);
                Height = GetActorPropertyFixed(Players(i).TID, APROP_Height);
                Divisor = (Dist - 16.0) + Dist;
                Angle = VectorAngle (GetActorX(Players(i).TID) - GetActorX(ItemTID[j]), GetActorY(Players(i).TID) - GetActorY(ItemTID[j]));
                Pitch = VectorAngle (Dist, GetActorZ(Players(i).TID) - GetActorZ(ItemTID[j]));

                // Calculate the amount based on close versus far distance
                if (Dist < 16.0)
                    Amount = 1.0;
                else
                    Amount = 16.0 / Divisor;

                // Calculate the lerped positions
                X = (Amount * 16.0) * Cos(Angle) * Cos(Pitch);
                Y = (Amount * 16.0) * Sin(Angle) * Cos(Pitch);
                Z = (Amount * 16.0) * Sin(Pitch);

                SetActorVelocity(ItemTID[j], X, Y, Z, true, false);
                NoClip = true;

                if (NoClip)
                {
                    SetActorFlag(ItemTID[j], "NOCLIP", true);
                    SetActorFlag(ItemTID[j], "NOGRAVITY", true);
                    SetActorFlag(ItemTID[j], "NOTELEPORT", true);
                    SetActorFlag(ItemTID[j], "VISIBILITYPULSE", true);
                    SetActorFlag(ItemTID[j], "BRIGHT", true);
                }
                else
                {
                    SetActorFlag(ItemTID[j], "NOCLIP", false);
                    SetActorFlag(ItemTID[j], "NOGRAVITY", false);
                    SetActorFlag(ItemTID[j], "NOTELEPORT", false);
                    SetActorFlag(ItemTID[j], "VISIBILITYPULSE", false);
                    SetActorFlag(ItemTID[j], "BRIGHT", false);
                }
            }
        }

        Delay(4);
    }
}

// Initializes an item and adds it to the Items array
NamedScript DECORATE void ItemInit()
{
    int TID;

    // Delay while the Items array is being initialized
    while (!ItemTIDsInitialized) Delay(1);

    // [KS] Some items cease existing once they're added to inventory
    // If that's the case, return early so we don't add empty item info
    if (ClassifyActor(0) == ACTOR_WORLD)
        return;

    // Add to Items array
    for (int i = 0; i < MAX_ITEMS; i++)
        if (ItemTIDs[i] == -1)
        {
            if (DebugLog)
                Log("\CdDEBUG: \C-Item \Cd%S\C- added (Index \Cd%d\C-)", GetActorClass(0), i);

            // Doesn't have a TID, so assign it one
            if (ActivatorTID() == 0)
            {
                TID = UniqueTID();
                ItemTIDs[i] = TID;
                Thing_ChangeTID(0, TID);
            }
            else
                ItemTIDs[i] = ActivatorTID();

            break;
        }
}

NamedScript Console void ItemDump()
{
    Log("\Cd  ===== ITEM ARRAY DATA =====");

    for (int i = 0; i < MAX_ITEMS; i++)
    {
        if (ItemTIDs[i] == -1) break;
        Log("\Cd%d\C-: \Cd%S", i, GetActorClass(ItemTIDs[i]));
    }

    Log("\Cd  ===== ITEM ARRAY DATA END =====");
}

// Dynamic Loot Generation System
NamedScript bool DynamicLootGeneratorCheckRemoval(int TID, fixed Z, bool monster)
{
    bool Remove = true;

    // [KS] Don't spawn stuff on special-effects flats like skyfloor or blackness, or at extreme height differences, because more often than not those are used as void space filler or instakill floors.
    // Examples: Epic2 MAP14, SF2012 MAP02, some CC4 maps.
    bool BadFloor = (CheckActorFloorTexture(TID, "F_SKY1") || CheckActorFloorTexture(TID, "F_SKY2") || CheckActorFloorTexture(TID, "BLACK") || CheckActorFloorTexture(TID, "FBLACK") || CheckActorFloorTexture(TID, "ALLBLAKF"));
    bool LegitSector = monster && GetCVar("drpg_spawnercheck_sector") ? ScriptCall("DRPGZUtilities", "CheckActorInLegitSector", TID, monster) : true;
    bool check = monster ? GetCVar("drpg_spawnercheck_sight") : true;

    for (int i = 0; ItemTIDs[i] != -1; i++)
    {
        // If this spot is blank, skip it
        if (ClassifyActor(ItemTIDs[i]) == ACTOR_WORLD) continue;

        // Randomly continue for variance
        if (Random(1, 4) == 1) continue;

        bool CanSee = check ? CheckSight(ItemTIDs[i], TID, 0) : true;

        if (CanSee && LegitSector && !BadFloor && Abs(Z - GetActorZ(ItemTIDs[i])) <= 24)
        {
            Remove = false;
            break;
        }
    }

    return Remove;
}

NamedScript OptionalArgs(1) void DynamicLootGenerator(str Actor, int MaxItems)
{
    // Delay while the map is being initialized
    while (!CurrentLevel->Init) Delay(1);

    LogMessage(StrParam("Running DynamicLootGenerator to create %d items of %S", MaxItems, Actor), LOG_DEBUG);
    fixed LowerX = GetActorX(0);
    fixed UpperX = GetActorX(0);
    fixed LowerY = GetActorY(0);
    fixed UpperY = GetActorY(0);
    int Items = 0;
    int Iterations = 0;
    int NumItems = 0;

    // Initial delay to make sure all items have been added/initialized in the level
    while (!ItemTIDsInitialized) Delay(1);
    Delay(4);

    // Determine the max amount of items to create if it's not specifically specified
    if (MaxItems == 0)
    {
        MaxItems = RoundInt(RandomFixed(24.0, 56.0) * ((fixed)AveragePlayerLuck() / 100.0));
        MaxItems *= GetCVarFixed("drpg_lootgen_factor");
    }

    if (MaxItems == 0)
    {
        if (DebugLog)
            Log("\CdDebug: \C-Skipped item generation.");
        return;
    }
    else if (MaxItems < 0)
    {
        // [KS] Negative luck? Na na nana na! *raspberry*
        if (CurrentLevel->Event == MAPEVENT_MEGABOSS)
            return;

        MaxItems = -MaxItems;
        Actor = "DRPGGenericMonsterDropper";
    }

    fixed ItemX, ItemY;

    // Find the furthest item from the player to determine some approximate boundaries to work with
    for (int i = 0; ItemTIDs[i] != -1; i++)
    {
        if (ClassifyActor(ItemTIDs[i]) == ACTOR_NONE) continue;

        NumItems = i + 1;

        ItemX = GetActorX(ItemTIDs[i]);
        ItemY = GetActorY(ItemTIDs[i]);

        if (LowerX > ItemX) LowerX = ItemX;
        if (UpperX < ItemX) UpperX = ItemX;
        if (LowerY > ItemY) LowerY = ItemY;
        if (UpperY < ItemY) UpperY = ItemY;

        if (i % 1000 == 0) Delay(1);
    }

    if (NumItems < 1) // [KS] If we can't find any possible positions to spawn anything, fuck it.
        return;

    int TID, A;
    fixed X, Y, Z;

    while (Items < MaxItems)
    {
        TID = UniqueTID();
        X = RandomFixed(LowerX, UpperX);
        Y = RandomFixed(LowerY, UpperY);
        Z = 0;
        A = 32 * Random(0, 7);

        // Try to keep Z near the floor
        SpawnForced("MapSpot", X, Y, 0, TID, 0);
        Z = GetActorFloorZ(TID);
        Thing_Remove(TID);

        bool Spawned = Spawn("MapSpotTall", X, Y, Z, TID, A);

        Iterations++;

        if (Iterations < 512 && (CheckSight(TID, MAP_START_TID, 0) || Distance(TID, MAP_START_TID) <= 1024 - Iterations))
            continue;

        if (Spawned)
        {
            bool Remove = ScriptCall("DRPGZUtilities", "CheckActorInMap", TID) ? DynamicLootGeneratorCheckRemoval(TID, Z, MaxItems < 0) : true;
            bool Visible = false;

            if (!Remove)
            {
                SetActorFlag(TID, "LOOKALLAROUND", true);
                for (int i = 0; i < MAX_PLAYERS; i++)
                {
                    if (CheckSight(TID, Players(i).TID, CSF_NOFAKEFLOORS))
                    {
                        Visible = true;
                        break;
                    }
                    else if (CheckSight(Players(i).TID, TID, CSF_NOFAKEFLOORS))
                    {
                        Visible = true;
                        break;
                    }
                }
            }
            Thing_Remove(TID);
            if (!Remove)
            {
                if (Actor == "DRPGGenericMonsterDropper")
                {
                    str SpawnMonster;
                    if (CurrentLevel != NULL && CurrentLevel->Event == MAPEVENT_ONEMONSTER)
                        SpawnMonster = GetMissionMonsterActor(CurrentLevel->SelectedMonster->Actor);
                    else if (GetCVar("drpg_monster_adaptive_spawns"))
                        SpawnMonster = Monsters[Random(1, CurrentLevel->MaxTotalMonsters)].Actor;
                    else
                        SpawnMonster = Actor;
                    if (!Visible && Spawn(SpawnMonster, X, Y, Z, TID, A))
                        Items++;

                    // Compatibility Handling - LegenDoom
                    if (CompatMode == COMPAT_LEGENDOOM || CompatModeLite == COMPAT_LEGENDOOMLITE)
                    {
                        if (CheckClass("LDLegendaryMonsterPickupEasy"))
                        {
                            switch (GameSkill())
                            {
                            case 0:
                                GiveActorInventory(TID, "LDLegendaryMonsterPickupEasy", 1);
                                break;
                            case 1:
                                GiveActorInventory(TID, "LDLegendaryMonsterPickupNormal", 1);
                                break;
                            case 2:
                                GiveActorInventory(TID, "LDLegendaryMonsterPickupHard", 1);
                                break;
                            case 3:
                                GiveActorInventory(TID, "LDLegendaryMonsterPickupUV", 1);
                                break;
                            default:
                                GiveActorInventory(TID, "LDLegendaryMonsterPickupNightmare", 1);
                                break;
                            }
                        }
                    }
                }
                else if (Spawn(Actor, X, Y, Z, TID, A))
                    Items++;
            }
        }

        if (DebugLog)
        {
            HudMessage("\CfGenerating Loot\n\Cd%d \Cj/ \Cd%d\n\n\CdActor: \C-%S\n\CdIteration: %d\n\CiBoundaries: %.2k-%.2k, %.2k-%.2k\n\nX: %.2k\nY: %.2k\nZ: %.2k", Items, MaxItems, Actor, Iterations, LowerX, UpperX, LowerY, UpperY, X, Y, Z);
            EndHudMessage(HUDMSG_FADEOUT, MAKE_ID('L', 'O', 'O', 'T'), "White", 1.5, 0.8, 1.5, 0.5);
        }

        if (Iterations % 50 == 0) Delay(1);

        /*if (Iterations == 2000) // Trick to restart ourselves if we ran out of iterations
        {
            if (DebugLog)
                Log("\CdDebug: \C-Dynamic Loot Generation created \Cd%d\C- items of type \Cd%S\C- and is restarting to place more", Items, Actor);
            DynamicLootGenerator(Actor, MaxItems - Items);
            return;
        }*/
    }

    if (DebugLog)
        Log("\CdDebug: \C-Dynamic Loot Generation created \Cd%d\C- items of type \Cd%S", Items, Actor);
}

// Activate Focus Mode
NamedScript KeyBind void ToggleFocusMode()
{
    if (Player.Augs.Battery < (Player.EPMax / 50) * 5)
    {
        PrintError(StrParam("There is not enough power for focusing device\n\nYou need %d AUG Battery", (Player.EPMax / 50) * 5));
        ActivatorSound("menu/error", 127);
        return;
    }

    if (Player.FocusingCooldown > 0)
    {
        PrintError(StrParam("Focusing device is overloaded\n\nYou must wait %S before reattemtping the focusing", FormatTime(Player.FocusingCooldown)));
        ActivatorSound("menu/error", 127);
        return;
    }

    Player.Focusing = !Player.Focusing;

    if (Player.Focusing)
        FocusMode();
}

// Focus Mode Script
NamedScript void FocusMode()
{
    int RegenWindupSpeed = ((35 * 40) - ((35 * 35) * Player.RegenerationTotal / 200));
    if (RegenWindupSpeed < 35) // Enforce a 1-second wind-up at least
        RegenWindupSpeed = 35;
    int StartWindupSpeed = RegenWindupSpeed;
    int RegenDelay = (RegenWindupSpeed * (Player.EPTime / 4)) / StartWindupSpeed;
    int Percent;

    // [KS] Someone did this!
    if (GetActorProperty(0, APROP_Health) <= 0) return;

    // Return if you're already at max EP
    if (Player.EP >= Player.EPMax) return;

    SetPlayerProperty(0, 1, PROP_FROZEN);

    Player.Augs.Battery -= (Player.EPMax / 50) * 5;
    DrawBattery();

    while (Player.Focusing)
    {
        Percent = 100 - (RegenWindupSpeed * 100 / StartWindupSpeed);
        fixed X = GetActorX(0) + Cos((fixed)Timer() / 64.0) * 32.0;
        fixed Y = GetActorY(0) + Sin((fixed)Timer() / 64.0) * 32.0;
        fixed Z = GetActorZ(0);

        if (GetActorProperty(0, APROP_Health) <= 0)
        {
            Player.Focusing = false;
            return;
        }

        Player.EPRate = 0;

        PlaySound(0, "misc/epfocus", CHAN_BODY, 0.5, true, ATTN_NORM);
        SpawnForced("DRPGEPFocusAura", X, Y, Z, 0, 0);
        DrawProgressBar("\CnFocusing", Percent);

        if (RegenWindupSpeed > 1)
            RegenWindupSpeed--;

        if (RegenDelay > 0)
            RegenDelay--;
        else
        {
            Player.EP += Player.EPAmount;
            if (Player.EP > Player.EPMax)
                Player.EP = Player.EPMax;
            RegenDelay = (RegenWindupSpeed * (Player.EPTime / 4)) / StartWindupSpeed;
        }

        // Moving and attacking interrupts focusing
        if (IsPlayerMoving() || Player.EP >= Player.EPMax || CheckInput(BT_ATTACK, KEY_HELD, true, -1) || CheckInput(BT_ALTATTACK, KEY_HELD, true, -1))
            Player.Focusing = false;

        Delay(1);
    }

    Player.FocusingCooldown += 35 * 300;
    Player.Focusing = false; // So we can't gain Regen XP out of thin air
    PlaySound(0, "misc/epfocusdone", CHAN_BODY, 0.5, false, ATTN_NORM);
    SetPlayerProperty(0, 0, PROP_FROZEN);
}

// Death Script
NamedScript Type_DEATH void Dead()
{
    // Reset menu vars
    Player.InMenu = false;
    Player.InShop = false;
    Player.OutpostMenu = 0;

    // Remove Aura
    RemoveAura();

    // Remove Shield
    SetInventory("DRPGShield", 0);

    // Remove Status Effect
    for (int i = 0; i < SE_MAX; i++)
        Player.StatusTimer[i] = 0;

    // Drop Credits
    if (GetCVar("drpg_multi_dropcredits") && CheckInventory("DRPGCredits") > 0)
    {
        int DropAmount = CheckInventory("DRPGCredits") / 100 * GetCVar("drpg_multi_dropcredits_percent");

        // Cap out at a million so if you have stupid amounts of Credits you don't freeze/nuke the game
        if (DropAmount > 1000000) DropAmount = 1000000;

        TakeInventory("DRPGCredits", DropAmount);
        DropMoney(PlayerNumber(), 0, DropAmount);
    }

    // Drop Inventory
    if (GetCVar("drpg_multi_dropinv"))
        DropEntireInventory();

    // VENG-R Accessory
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        // Skip yourself
        if (Player.TID == Players(i).TID) continue;

        if (Players(i).Shield.Accessory && Players(i).Shield.Accessory->PassiveEffect == SHIELD_PASS_AVENGER)
        {
            GiveActorInventory(Players(i).TID, "DRPGShieldAvengerDamage", 1);
            GiveActorInventory(Players(i).TID, "DRPGShieldAvengerDefense", 1);
        }
    }

    if (GetCVar("drpg_multi_revives"))
    {
        // Incapacitation announcement
        SetHudSize(320, 200, false);
        SetFont("SMALLFONT");
        if (SomePlayerAlive())
            HudMessage("%tS was incapacitated", PlayerNumber() + 1);
        else
            HudMessage("%tS has died", PlayerNumber() + 1);
        EndHudMessageBold(HUDMSG_FADEOUT, 0, "Brick", 160.0, 140.0, 1.5, 1.0);

        // Remember body location
        Player.BodyTID = GetUniqueTID();

        // Assign TID to player's body
        Thing_ChangeTID(Player.TID, Player.BodyTID);
        Delay(1);

        // Actualize the actual health
        Player.ActualHealth = GetActorProperty(0, APROP_Health);
    }
    else
    {
        // Remove TID
        Thing_ChangeTID(Player.TID, 0);
    }
}

// Respawn
NamedScript Type_RESPAWN void Respawn()
{
    // Reassign TID
    if (CompatMode == COMPAT_DRLA)
        Delay(4);
    AssignTIDs();

    // Heal to max health if revives are disabled or revive at the body's location
    if (Player.ReviverTID > 0 && Player.BodyTID > 0)
    {
        SetActorFlag(Player.BodyTID, "INVISIBLE", true);
        SetActorPosition(Player.TID, GetActorX(Player.BodyTID), GetActorY(Player.BodyTID), GetActorZ(Player.BodyTID), false);
        SetActorAngle(Player.TID, GetActorAngle(Player.BodyTID));
        Thing_Remove(Player.BodyTID);
    }
    else
        Player.ActualHealth = Player.HealthMax;

    SetActorProperty(0, APROP_Health, Player.ActualHealth);

    // XP/Rank Penalty
    if (GetCVar("drpg_multi_takexp") && Player.ReviverTID == 0)
    {
        long int XPPenalty = (long int)(XPTable[Player.Level] * GetCVar("drpg_multi_takexp_percent") / 100);
        long int RankPenalty = (long int)(RankTable[Player.RankLevel] * GetCVar("drpg_multi_takexp_percent") / 100);

        if (XPPenalty > Player.XP) XPPenalty = Player.XP;
        if (RankPenalty > Player.Rank) RankPenalty = Player.Rank;

        if (XPPenalty > 0 || RankPenalty > 0)
        {
            Player.XP -= XPPenalty;
            Player.Rank -= RankPenalty;
            SetFont("BIGFONT");
            HudMessage("\CjXP -%ld\n\CkRank -%ld", XPPenalty, RankPenalty);
            EndHudMessage(HUDMSG_FADEOUT | HUDMSG_LOG, 0, "White", 1.5, 0.75, 2.0, 2.0);
        }
    }

    // Restore EP if CVAR is set
    if (GetCVar("drpg_multi_restoreep") && Player.ReviverTID == 0)
        Player.EP = Player.EPMax;

    // Give a box of ammo if a specific ammo type is empty if the CVAR is set
    if (GetCVar("drpg_multi_restoreammo") && Player.ReviverTID == 0)
    {
        if (CheckInventory("Clip") < GetAmmoAmount("Clip") * (Player.CapacityTotal / 10))
            SetInventory("Clip", GetAmmoAmount("Clip") * (Player.CapacityTotal / 10));
        if (CheckInventory("Shell") < GetAmmoAmount("Shell") * (Player.CapacityTotal / 10))
            SetInventory("Shell", GetAmmoAmount("Shell") * (Player.CapacityTotal / 10));
        if (CheckInventory("RocketAmmo") < GetAmmoAmount("RocketAmmo") * (Player.CapacityTotal / 10))
            SetInventory("RocketAmmo", GetAmmoAmount("RocketAmmo") * (Player.CapacityTotal / 10));
        if (CheckInventory("Cell") < GetAmmoAmount("Cell") * (Player.CapacityTotal / 10))
            SetInventory("Cell", GetAmmoAmount("Cell") * (Player.CapacityTotal / 10));
    }

    // Reset revive stuff
    if (GetCVar("drpg_multi_revives"))
    {
        Player.ReviveeNum = -1;
        Player.ReviveKeyTimer = 0;
        Player.ReviverTID = 0;
        Player.BodyTID = 0;
    }

    // Apply camera textures and vars
    SetCameraToTexture(Player.TID, StrParam("P%iVIEW", PlayerNumber() + 1), 110);
    SetCameraToTexture(Player.TID, StrParam("P%iSVIEW", PlayerNumber() + 1), 90);
    Player.PlayerView = PlayerNumber();

    // Run Scripts
    Loop();
    PlayerHealth();
    PlayerSurvive();
    MoneyChecker();
    DamageNumbers();
    InfoPopoffs();
    HealthBars();
    ShieldTimer();

    // NU-YU Accessory
    if (Player.Shield.Accessory && Player.Shield.Accessory->PassiveEffect == SHIELD_PASS_HYPERION)
    {
        ActivatorSound("shield/newu", 127);
        Player.Shield.Active = true;
        Player.Shield.Charge = Player.Shield.Capacity;
        GiveInventory("DRPGShieldNewUProtection", 1);
    }

    // Quick Teleport
    if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
    {
        int PlayerNum = -1;
        while (PlayerNum == -1 || PlayerNum == PlayerNumber() || !PlayerInGame(PlayerNum))
            PlayerNum = Random(0, PlayerCount());
        PlayerTeleport(PlayerNum);
        Player.EP -= ScaleEPCost(50);
    }
}

str const Loadout_WeaponInfo[LOADOUT_WEAPONS][3] =
{
    // CVAR, Vanilla Actor, Extras Actor

    { "drpg_start_weapon_chainsaw",         "DRPGChainsaw",         "DRPGChainsawExtras",       },
    { "drpg_start_weapon_pistol",           "DRPGPistol",           "DRPGPistolExtras",         },
    { "drpg_start_weapon_shotgun",          "DRPGShotgun",          "DRPGShotgunExtras",        },
    { "drpg_start_weapon_ssg",              "DRPGSuperShotgun",     "DRPGSuperShotgunExtras",   },
    { "drpg_start_weapon_chaingun",         "DRPGChaingun",         "DRPGChaingunExtras",       },
    { "drpg_start_weapon_rocketlauncher",   "DRPGRocketLauncher",   "DRPGRocketLauncherExtras", },
    { "drpg_start_weapon_plasmarifle",      "DRPGPlasmaRifle",      "DRPGPlasmaRifleExtras",    },
    { "drpg_start_weapon_bfg",              "DRPGBFG9000",          "DRPGBFG9000Extras",        },
};
str const Loadout_ArmorInfo[LOADOUT_ARMORS] =
{
    "DRPGGreenArmorEffect",
    "DRPGBlueArmorEffect",
    "DRPGYellowArmorEffect",
    "DRPGRedArmorEffect",
    "DRPGWhiteArmorEffect",
    "DRPGReinforcedGreenArmorEffect",
    "DRPGReinforcedBlueArmorEffect",
    "DRPGReinforcedYellowArmorEffect",
    "DRPGReinforcedRedArmorEffect",
    "DRPGReinforcedWhiteArmorEffect",
};
str const Loadout_ShieldPartColorCodes[LOADOUT_SHIELDPARTS] =
{
    "\Cv",  // Capacity +
    "\Ch",  // Capacity -
    "\Cd",  // Charge Rate +
    "\Cq",  // Charge Rate -
    "\Cr",  // Delay Rate +
    "\Cg",  // Delay Rate -
};
str const Loadout_ShieldAccessoryColorCodes[LOADOUT_SHIELDACCS] =
{
    "\Cv",  // Shield Capacity
    "\Cg",  // Strength / Damage
    "\Ca",  // Vitality / Health
    "\Ci",  // Agility / Speed
    "\Cd",  // Defense / Armor / Shield Charge
    "\Cf",  // Luck / Money
    "\Cn",  // Energy
    "\Ch",  // Capacity / Ammo
    "\Ck",  // Rank
    "\Cc",  // Augmentation
    "\Cq",  // Powerup
    "\Cj",  // Misc / Multi-Category
};
str const Loadout_DRLAWeaponColorCodes[LOADOUT_DRLAWEAPONS] =
{
    "",     // Common
    "\Cv",  // Assembled
    "\Ct",  // Exotic
    "\Ci",  // Superior
    "\Cd",  // Unique
    "\Cg",  // Demonic
    "\Cf",  // Legendary
};
str const Loadout_DRLAArmorColorCodes[LOADOUT_DRLAARMORS] =
{
    "",     // Common
    "\Cv",  // Assembled
    "\Ct",  // Exotic
    "\Cc",  // Onyx
    "\Cd",  // Unique
    "\Cf",  // Legendary
};
str const Loadout_DRLAModPacks[LOADOUT_DRLAMODPACKS] =
{
    "RLPowerModItem",
    "RLBulkModItem",
    "RLAgilityModItem",
    "RLTechnicalModItem",
    "RLSniperModItem",
    "RLFirestormModItem",
    "RLNanoModItem",
    "RLOnyxModItem",
    // RLArtiModItem is unnecessary since it provides random modpacks from above
};

// Shield Databases
static ItemInfoPtr Loadout_ShieldBodyTypes[LOADOUT_SHIELDPARTS][MAX_BODIES];
static ItemInfoPtr Loadout_ShieldBatteryTypes[LOADOUT_SHIELDPARTS][MAX_BATTERIES];
static ItemInfoPtr Loadout_ShieldCapacitorTypes[LOADOUT_SHIELDPARTS][MAX_CAPACITORS];
static ItemInfoPtr Loadout_ShieldAccessoryTypes[LOADOUT_SHIELDACCS][MAX_ACCESSORIES];
static int Loadout_BodyCount[LOADOUT_SHIELDPARTS];
static int Loadout_BatteryCount[LOADOUT_SHIELDPARTS];
static int Loadout_CapacitorCount[LOADOUT_SHIELDPARTS];
static int Loadout_AccessoryCount[LOADOUT_SHIELDACCS];

// DRLA Databases
static ItemInfoPtr Loadout_DRLAWeaponTypes[LOADOUT_DRLAWEAPONS][ITEM_MAX];
static ItemInfoPtr Loadout_DRLAArmorTypes[LOADOUT_DRLAARMORS][ITEM_MAX];
static ItemInfoPtr Loadout_DRLABootsTypes[LOADOUT_DRLAARMORS][ITEM_MAX];
static int Loadout_DRLAWeaponCount[LOADOUT_DRLAWEAPONS];
static int Loadout_DRLAArmorCount[LOADOUT_DRLAARMORS];
static int Loadout_DRLABootsCount[LOADOUT_DRLAARMORS];

NamedScript void SortStartingItems()
{
    // Build Shield and DRLA item databases based on their respective items' color codes
    for (int i = 0; i < MAX_BODIES; i++)
        for (int j = 0; j < LOADOUT_SHIELDPARTS; j++)
            if (Contains(ShieldParts[0][i].Name, Loadout_ShieldPartColorCodes[j]))
            {
                Loadout_ShieldBodyTypes[j][Loadout_BodyCount[j]++] = FindItemInCategory(ShieldParts[0][i].Actor, 5);
                break;
            }
    for (int i = 0; i < MAX_BATTERIES; i++)
        for (int j = 0; j < LOADOUT_SHIELDPARTS; j++)
            if (Contains(ShieldParts[1][i].Name, Loadout_ShieldPartColorCodes[j]))
            {
                Loadout_ShieldBatteryTypes[j][Loadout_BatteryCount[j]++] = FindItemInCategory(ShieldParts[1][i].Actor, 5);
                break;
            }
    for (int i = 0; i < MAX_CAPACITORS; i++)
        for (int j = 0; j < LOADOUT_SHIELDPARTS; j++)
            if (Contains(ShieldParts[2][i].Name, Loadout_ShieldPartColorCodes[j]))
            {
                Loadout_ShieldCapacitorTypes[j][Loadout_CapacitorCount[j]++] = FindItemInCategory(ShieldParts[2][i].Actor, 5);
                break;
            }
    for (int i = 0; i < MAX_ACCESSORIES; i++)
        for (int j = 0; j < LOADOUT_SHIELDACCS; j++)
            if (Contains(ShieldAccessories[i].Name, Loadout_ShieldAccessoryColorCodes[j]))
            {
                Loadout_ShieldAccessoryTypes[j][Loadout_AccessoryCount[j]++] = FindItemInCategory(ShieldAccessories[i].Actor, 5);
                break;
            }
    for (int i = 0; i < ItemMax[0]; i++)
        for (int j = 1; j < LOADOUT_DRLAWEAPONS; j++)
        {
            if (i < 10) // Special catch for Common
            {
                Loadout_DRLAWeaponTypes[0][Loadout_DRLAWeaponCount[0]++] = &ItemData[0][i];
                break;
            }
            else if (Contains(ItemData[0][i].Name, Loadout_DRLAWeaponColorCodes[j]))
            {
                Loadout_DRLAWeaponTypes[j][Loadout_DRLAWeaponCount[j]++] = &ItemData[0][i];
                break;
            }
        };
    for (int i = 1; i < ItemMax[3]; i++)
        for (int j = 1; j < LOADOUT_DRLAARMORS; j++)
        {
            // Onyx is handled below
            if (j == 3) continue;

            if (i < 4) // Special catch for Common
            {
                Loadout_DRLAArmorTypes[0][Loadout_DRLAArmorCount[0]++] = &ItemData[3][i];
                break;
            }
            else if (Contains(ItemData[3][i].Name, "Onyx")) // Special catch for Onyx
            {
                Loadout_DRLAArmorTypes[3][Loadout_DRLAArmorCount[3]++] = &ItemData[3][i];
                break;
            }
            else if (Contains(ItemData[3][i].Name, Loadout_DRLAArmorColorCodes[j]))
            {
                Loadout_DRLAArmorTypes[j][Loadout_DRLAArmorCount[j]++] = &ItemData[3][i];
                break;
            }
        }
    for (int i = 0; i < ItemMax[9]; i++)
        for (int j = 1; j < LOADOUT_DRLAARMORS; j++)
        {
            if (i < 3) // Special catch for Common
            {
                Loadout_DRLABootsTypes[0][Loadout_DRLABootsCount[0]++] = &ItemData[9][i];
                break;
            }
            else if (Contains(ItemData[9][i].Name, Loadout_DRLAArmorColorCodes[j]))
            {
                Loadout_DRLABootsTypes[j][Loadout_DRLABootsCount[j]++] = &ItemData[9][i];
                break;
            }
        }
}

// Gives the player their default loadout based on their starting options
NamedScript void Loadout_GiveAugs()
{
    // Active Augs
    int ActiveAugs = 0;
    int Type;

    while (ActiveAugs < GetActivatorCVar("drpg_start_aug_amount"))
    {
        Type = Random(0, AUG_MAX - 1);

        // Skip this aug if it's already active
        if (Player.Augs.Level[Type] > 0) continue;

        Player.Augs.Level[Type]++;

        if (Player.Augs.Level[Type] > 0)
            Player.Augs.CurrentLevel[Type] = Player.Augs.Level[Type];

        ActiveAugs++;
    }

    // Aug Canisters/Upgrades
    GiveInventory("DRPGAugCanister", GetActivatorCVar("drpg_start_aug_canisters"));
    GiveInventory("DRPGAugUpgradeCanister", GetActivatorCVar("drpg_start_aug_upgrades"));

    // Aug Slots
    Player.Augs.BaseSlots = GetActivatorCVar("drpg_start_aug_slots");
}

NamedScript void Loadout_GiveShieldParts()
{
    // Shield Parts
    for (int i = 0; i < 3; i++)
    {
        int *Count[3] =
        {
            &Loadout_BodyCount[0],
            &Loadout_BatteryCount[0],
            &Loadout_CapacitorCount[0]
        };
        int Total[3] =
        {
            GetActivatorCVar("drpg_start_shield_amount_body"),
            GetActivatorCVar("drpg_start_shield_amount_battery"),
            GetActivatorCVar("drpg_start_shield_amount_capacitor")
        };
        int Current[3];

        // Calculate total if we specify a type so that overflows or explosions don't happen
        if (GetActivatorCVar("drpg_start_shield_type") >= 0)
            if (Total[i] > Count[i][GetActivatorCVar("drpg_start_shield_type")])
                Total[i] = Count[i][GetActivatorCVar("drpg_start_shield_type")];

        while (Current[i] < Total[i])
        {
            int Category = Random(0, LOADOUT_SHIELDPARTS - 1);

            // Force the category type if the CVAR is set
            if (GetActivatorCVar("drpg_start_shield_type") >= 0)
                Category = GetActivatorCVar("drpg_start_shield_type");

            for (int j = 0; j < Count[i][Category]; j++)
            {
                ItemInfoPtr *Type[3];

                Type[0] = &Loadout_ShieldBodyTypes[Category][j];
                Type[1] = &Loadout_ShieldBatteryTypes[Category][j];
                Type[2] = &Loadout_ShieldCapacitorTypes[Category][j];

                if (!CheckInventory((*(Type[i]))->Actor))
                {
                    GiveInventory((*(Type[i]))->Actor, 1);
                    Current[i]++;
                    break;
                }
            }
        }
    }
}

NamedScript void Loadout_GiveShieldAccessories()
{
    // Shield Accessories
    int TotalAccessories = GetActivatorCVar("drpg_start_shield_amount_acc");
    int Accessories = 0;
    int Category;
    if (GetActivatorCVar("drpg_start_shield_type_acc") >= 0)
        if (TotalAccessories > Loadout_AccessoryCount[GetActivatorCVar("drpg_start_shield_type_acc")])
            TotalAccessories = Loadout_AccessoryCount[GetActivatorCVar("drpg_start_shield_type_acc")];

    while (Accessories < TotalAccessories)
    {
        Category = Random(0, LOADOUT_SHIELDACCS - 1);

        // Force the category type if the CVAR is set
        if (GetActivatorCVar("drpg_start_shield_type_acc") >= 0)
            Category = GetActivatorCVar("drpg_start_shield_type_acc");

        for (int i = 0; i < Loadout_AccessoryCount[Category]; i++)
        {
            ItemInfoPtr Type = Loadout_ShieldAccessoryTypes[Category][i];

            if (!CheckInventory(Type->Actor))
            {
                GiveInventory(Type->Actor, 1);
                Accessories++;
                break;
            }
        }
    }
}

NamedScript void Loadout_GiveStims()
{
    // Stims
    int Injectors = 0;

    while (Injectors < GetActivatorCVar("drpg_start_stim_injectors"))
    {
        switch (Random(1, 4))
        {
        case 1: // Small
            if (CheckInventory("DRPGStimSmall") >= 1000) continue;
            GiveInventory("DRPGStimSmall", 1);
            break;
        case 2: // Medium
            if (CheckInventory("DRPGStimMedium") >= 1000) continue;
            GiveInventory("DRPGStimMedium", 1);
            break;
        case 3: // Large
            if (CheckInventory("DRPGStimLarge") >= 1000) continue;
            GiveInventory("DRPGStimLarge", 1);
            break;
        case 4: // XL
            if (CheckInventory("DRPGStimXL") >= 1000) continue;
            GiveInventory("DRPGStimXL", 1);
            break;
        }

        Injectors++;
    }
}

NamedScript void Loadout_GiveVials()
{
    int Vials = 0;
    int MaxVials = GetActivatorCVar("drpg_start_stim_vials");
    if (MaxVials > 3000) MaxVials = 3000;
    int Capacity = Player.Capacity * 10;
    int Types = 8;
    if (GetActivatorCVar("drpg_start_stim_boosters")) Types += 2;
    if (GetActivatorCVar("drpg_start_stim_powerups")) Types += 9;
    if (MaxVials > Types * Capacity) MaxVials = Types * Capacity;
    int Type;

    while (Vials < MaxVials)
    {
        Type = Random(0, STIM_MAX - 1);

        // Don't include Boosters
        if (!GetActivatorCVar("drpg_start_stim_boosters") && Type >= StimStatsEnd && Type <= StimPowerupStart) continue;

        // Don't include Powerups
        if (!GetActivatorCVar("drpg_start_stim_powerups") && Type >= StimPowerupStart && Type <= StimPowerupEnd) continue;

        // Skip this one if this vial type is full
        if (Player.Stim.Vials[Type] >= Capacity) continue;

        Player.Stim.Vials[Type]++;
        Vials++;
    }
}

// DRLA
NamedScript void Loadout_GiveDRLAEquipment()
{
    bool FoundWeapon = false;
    bool FoundArmor = false;
    bool FoundBoots = false;
    int WeaponCount = 0;
    int MaxWeaponCount = GetActivatorCVar("drpg_start_drla_weapon_amount");
    int ModPackCount = 0;
    int ModPackMax = GetActivatorCVar("drpg_start_drla_modpacks");

    // Weapons
    if (GetActivatorCVar("drpg_start_drla_weapon_type") > 0)
        if (MaxWeaponCount > Loadout_DRLAWeaponCount[GetActivatorCVar("drpg_start_drla_weapon_type")])
            MaxWeaponCount = Loadout_DRLAWeaponCount[GetActivatorCVar("drpg_start_drla_weapon_type")];
    while (WeaponCount < MaxWeaponCount)
    {
        for (int i = 0; i < LOADOUT_DRLAWEAPONS; i++)
        {
            FoundWeapon = false;

            // Force the weapon type if the CVAR is set
            if (GetActivatorCVar("drpg_start_drla_weapon_type") >= 0)
                i = GetActivatorCVar("drpg_start_drla_weapon_type");

            for (int j = 0; j < Loadout_DRLAWeaponCount[i]; j++)
                if (Random(1, 10) == 1 && !CheckInventory(Loadout_DRLAWeaponTypes[i][j]->Actor))
                {
                    GiveInventory(Loadout_DRLAWeaponTypes[i][j]->Actor, 1);
                    WeaponCount++;
                    FoundWeapon = true;
                    break;
                }

            if (FoundWeapon)
                break;
        }
    }
    GiveInventory("RLWeaponLimit", WeaponCount);

    // Armor
    if (GetActivatorCVar("drpg_start_drla_armor") >= -1)
        while (!FoundArmor)
        {
            for (int i = 0; i < LOADOUT_DRLAARMORS; i++)
            {
                // If there is nothing in this category, skip it
                if (GetActivatorCVar("drpg_start_drla_armor") >= 0 && Loadout_DRLAArmorCount[GetActivatorCVar("drpg_start_drla_armor")] == 0)
                {
                    FoundArmor = true;
                    continue;
                }
                else if (GetActivatorCVar("drpg_start_drla_armor") >= 0) // Force the armor type if the CVAR is setup
                    i = GetActivatorCVar("drpg_start_drla_armor");

                for (int j = 0; j < Loadout_DRLAArmorCount[i]; j++)
                    if (Random(1, 10) == 1)
                    {
                        GiveInventory(Loadout_DRLAArmorTypes[i][j]->Actor, 1);
                        FoundArmor = true;
                        break;
                    }

                if (FoundArmor)
                    break;
            }
        }

    // Boots
    if (GetActivatorCVar("drpg_start_drla_boots") >= -1)
        while (!FoundBoots)
        {
            for (int i = 0; i < LOADOUT_DRLAARMORS; i++)
            {
                // If there is nothing in this category, skip it
                if (GetActivatorCVar("drpg_start_drla_boots") >= 0 && Loadout_DRLABootsCount[GetActivatorCVar("drpg_start_drla_boots")] == 0)
                {
                    FoundBoots = true;
                    continue;
                }
                else if (GetActivatorCVar("drpg_start_drla_boots") >= 0) // Force the boots type if the CVAR is setup
                    i = GetActivatorCVar("drpg_start_drla_boots");

                for (int j = 0; j < Loadout_DRLABootsCount[i]; j++)
                    if (Random(1, 10) == 1)
                    {
                        GiveInventory(Loadout_DRLABootsTypes[i][j]->Actor, 1);
                        FoundBoots = true;
                        break;
                    }

                if (FoundBoots)
                    break;
            }
        }

    // Mod Packs
    int Type;
    if (PlayerClass(0) == 2)
        ModPackMax--;
    while (ModPackCount < ModPackMax)
    {
        Type = Random(0, LOADOUT_DRLAMODPACKS - 1);

        // Stop if we're not the technician and we try and give over 4 mods
        if (PlayerClass(0) != 2 && ModPackCount >= 4) break;

        // Skip exotic modpacks if the selection is disabled
        if (Type > 3 && !GetActivatorCVar("drpg_start_drla_modpacks_amount")) continue;

        GiveInventory(Loadout_DRLAModPacks[Type], 1);
        ModPackCount++;
    }
}

NamedScript void DefaultLoadout()
{
    // Weapons
    if (CompatMode != COMPAT_DRLA && CompatMode != COMPAT_LEGENDOOM)
    {
        for (int i = 0; i < 8; i++)
        {
            if (GetActivatorCVar(Loadout_WeaponInfo[i][0]))
            {
                if (CompatMode == COMPAT_EXTRAS)
                {
                    GiveInventory(Loadout_WeaponInfo[i][2], 1);
                    SetWeapon(Loadout_WeaponInfo[i][2]);
                }
                else
                {
                    GiveInventory(Loadout_WeaponInfo[i][1], 1);
                    SetWeapon(Loadout_WeaponInfo[i][1]);
                }
            }
            else
            {
                if (CompatMode == COMPAT_EXTRAS)
                    TakeInventory(Loadout_WeaponInfo[i][2], 1);
                else
                    TakeInventory(Loadout_WeaponInfo[i][1], 1);
            }
        }
    }

    // Ammo
    SetInventory("Clip", GetActivatorCVar("drpg_start_ammo_bullet"));
    SetInventory("Shell", GetActivatorCVar("drpg_start_ammo_shell"));
    SetInventory("RocketAmmo", GetActivatorCVar("drpg_start_ammo_rocket"));
    SetInventory("Cell", GetActivatorCVar("drpg_start_ammo_cell"));

    // Armor
    if (CompatMode != COMPAT_DRLA && GetActivatorCVar("drpg_start_armor") >= 0)
        GiveInventory(Loadout_ArmorInfo[GetActivatorCVar("drpg_start_armor")], 1);

    // Medkit
    Player.Medkit = GetActivatorCVar("drpg_start_medkit");

    Loadout_GiveAugs();
    Loadout_GiveShieldParts();
    Loadout_GiveShieldAccessories();
    Loadout_GiveStims();
    Loadout_GiveVials();

    // Minigame Chips
    GiveInventory("DRPGChipGold", GetActivatorCVar("drpg_start_bonus_goldchips"));
    GiveInventory("DRPGChipPlatinum", GetActivatorCVar("drpg_start_bonus_platchips"));

    // UAC Shop Card
    if (GetActivatorCVar("drpg_start_bonus_shopcard") > 0)
    {
        // Diamond Card
        if (GetActivatorCVar("drpg_start_bonus_shopcard") >= 5)
            GiveInventory("DRPGDiamondUACCard", 1);
        else
            GiveInventory("DRPGUACCard", GetActivatorCVar("drpg_start_bonus_shopcard"));
    }

    if (CompatMode == COMPAT_DRLA)
        Loadout_GiveDRLAEquipment();
}

// Apply values to global vars visible on the HUD
void CheckHUD()
{
    int PlayerNum = PlayerNumber();

    // EP
    EP[PlayerNum] = Player.EP;

    // EP Bar on HUD
    if (Player.EPMax <= 0)
        SetInventory("DRPGEP", 0);
    else
        SetInventory("DRPGEP", Player.EP * 100 / Player.EPMax);

    // Shield
    if (Player.Shield.Capacity > 0)
    {
        Shield[PlayerNum] = Player.Shield.Charge;
        ShieldCapacity[PlayerNum] = Player.Shield.Capacity;
        ShieldHealth[PlayerNum] = Player.ActualHealth;
        SetInventory("DRPGShieldCapacity", (int)(((fixed)Player.Shield.Charge / (fixed)Player.Shield.Capacity) * 100.0));
        SetInventory("DRPGShield", Player.Shield.Active);
    }
}

void CheckKeys()
{
    str const Keycards[6] =
    {
        "DRPGRedCard",
        "DRPGYellowCard",
        "DRPGBlueCard",
        "DRPGRedSkull",
        "DRPGYellowSkull",
        "DRPGBlueSkull"
    };

    // Don't bother if we're not in multiplayer
    if (!InMultiplayer) return;

    for (int i = 0; i < 6; i++)
    {
        if (CheckInventory(Keycards[i]))
        {
            for (int j = 0; j < MAX_PLAYERS; j++)
            {
                if (!PlayerInGame(j)) continue;
                if (CheckActorInventory(Players(j).TID, Keycards[i])) continue;

                SetActorInventory(Players(j).TID, Keycards[i], 1);
            }
        }
    }
}

void CheckCompatibility()
{
    bool Success = false;
    int TID = UniqueTID();

    if (DebugLog)
        Log("\CdDEBUG: \C-Checking Compatibility...");

    SkillLevelsMax = MAX_SKILLLEVELS_DF;

    CompatMode = COMPAT_NONE;
    CompatModeEx = COMPAT_NONE;
    CompatModeLite = COMPAT_NONE;
    CompatMonMode = COMPAT_NONE;

    MonsterData = MonsterDataDF;
    MonsterDataAmount = MAX_DEF_MONSTERS_DF;

    MegaBosses = MegaBossesDF;
    MegaBossesAmount = MAX_MEGABOSSES_DF;

    MapPacks = false;

    // WadSmoosh
    Success = SpawnForced("DRPGWadSmooshActive", 0, 0, 0, TID, 0);
    if (Success)
    {
        if (DebugLog)
            Log("\CdDEBUG: \CaWadSmoosh\C- detected");
        MapPacks = true;
        Thing_Remove(TID);
    }

    // Lexicon
    Success = SpawnForced("DRPGLexiconActive", 0, 0, 0, TID, 0);
    if (Success)
    {
        if (DebugLog)
            Log("\CdDEBUG: \CaLexicon\C- detected");
        MapPacks = true;
        Thing_Remove(TID);
    }

    // Compendium
    Success = SpawnForced("DRPGCompendiumActive", 0, 0, 0, TID, 0);
    if (Success)
    {
        if (DebugLog)
            Log("\CdDEBUG: \CaCompendium\C- detected");
        MapPacks = true;
        Thing_Remove(TID);
    }

    // Extras
    Success = SpawnForced("DRPGExtrasIsLoaded", 0, 0, 0, TID, 0);
    if (Success)
    {
        if (DebugLog)
            Log("\CdDEBUG: \CaExtras\C- detected");
        CompatMode = COMPAT_EXTRAS;
        Thing_Remove(TID);
    }

    // LegenDoom
    Success = SpawnForced("LDLegendaryZombie", 0, 0, 0, TID, 0);
    if (Success)
    {
        if (DebugLog)
            Log("\CdDEBUG: \CdLegenDoom\C- detected");

        CompatMode = COMPAT_LEGENDOOM;
        CompatMonMode = COMPAT_LEGENDOOM;
        MonsterData = MonsterDataLD;
        Thing_Remove(TID);
    }

    // LegenDoomLite
    Success = SpawnForced("DRPGLegenDoomLiteActive", 0, 0, 0, TID, 0);
    if (Success)
    {
        if (DebugLog)
            Log("\CdDEBUG: \CdLegenDoomLite\C- detected");

        CompatModeLite = COMPAT_LEGENDOOMLITE;
        Thing_Remove(TID);
    }

    // DoomRL Arsenal
    Success = SpawnForced("RLPistolPickup", 0, 0, 0, TID, 0);
    if (Success)
    {
        if (DebugLog)
            Log("\CdDEBUG: \CdDoomRL \C-detected");

        CompatMode = COMPAT_DRLA;
        SetInventory("DRPGDRLAActive", 1);
        Thing_Remove(TID);
    }

    // DoomRL Arsenal Extended
    Success = SpawnForced("DRLAX_RadarDevice", 0, 0, 0, TID, 0);
    if (Success)
    {
        if (DebugLog)
            Log("\CdDEBUG: \CdDoomRL Extended \C-detected");

        CompatModeEx = COMPAT_DRLAX;
        Thing_Remove(TID);
    }

    // DoomRL Monsters
    Success = SpawnForced("RLBaronOfHell", 0, 0, 0, TID, 0);
    if (Success)
    {
        if (DebugLog)
            Log("\CdDEBUG: \CdDoomRL Monsters \C-detected");

        SkillLevelsMax = MAX_SKILLLEVELS_DRLA;
        CompatMonMode = COMPAT_DRLA;
        MonsterData = MonsterDataDRLA;
        MonsterDataAmount = MAX_DEF_MONSTERS_DRLA;
        Thing_Remove(TID);
    }

    // Colourful Hell
    Success = SpawnForced("RedZombie", 0, 0, 0, TID, 0);
    if (Success)
    {
        if (DebugLog)
            Log("\CdDEBUG: \CdColourful Hell \C-detected");

        CompatMonMode = COMPAT_CH;
        MonsterData = MonsterDataCH;
        MonsterDataAmount = MAX_DEF_MONSTERS_CH;
        MegaBosses = MegaBossesCH;
        MegaBossesAmount = MAX_MEGABOSSES_CH;
        Thing_Remove(TID);
    }

    // Rampancy
    Success = SpawnForced("Robot_SentryBot", 0, 0, 0, TID, 0);
    if (Success)
    {
        if (DebugLog)
            Log("\CdDEBUG: \CdRampancy\C- detected");

        CompatMonMode = COMPAT_RAMPANCY;
        MonsterData = MonsterDataRAMPANCY;
        Thing_Remove(TID);
    }

    // Dehacked Attack
    Success = SpawnForced("DEHZombieman", 0, 0, 0, TID, 0);
    if (Success)
    {
        if (DebugLog)
            Log("\CdDEBUG: \CdDeHacked Attack\C- detected");

        CompatMonMode = COMPAT_DEHACKED;
        MonsterData = MonsterDataDEHACKED;
        Thing_Remove(TID);
    }

    // Pandemonia Monsters
    Success = SpawnForced("PistolZombie", 0, 0, 0, TID, 0);
    if (Success)
    {
        if (DebugLog)
            Log("\CdDEBUG: \CdPandemonia Monsters\C- detected");

        CompatMonMode = COMPAT_PANDEMONIA;
        MonsterData = MonsterDataPANDM;
        MonsterDataAmount = MAX_DEF_MONSTERS_PANDM;
        Thing_Remove(TID);
    }

    if (DebugLog && CompatMode == COMPAT_NONE && CompatMonMode == COMPAT_NONE && !MapPacks)
        Log("\CdDEBUG: \C-No compatible mods found");
}

void AssignTIDs()
{
    Player.TID = PLAYER_TID + PlayerNumber();
    Thing_ChangeTID(0, Player.TID);

    if (DebugLog)
        Log("\CdDEBUG: Player TID: %d", Player.TID);
}

NamedScript void ReviveHandler()
{
    if ((!Player.InMenu && !Player.InShop && !Player.OutpostMenu && !Player.CrateOpen) && !Player.MenuBlock)
    {
        for (int i = 0; i < PlayerCount(); i++)
        {
            if (i != PlayerNumber() && Players(i).ActualHealth <= 0 && Players(i).BodyTID > 0 && Distance(Player.TID, Players(i).BodyTID) < 48)
            {
                SetHudSize(0, 0, false);
                SetFont("BIGFONT");
                if (Players(i).ReviverTID == 0 || Players(i).ReviverTID == Player.TID)
                {
                    if (Player.Medkit > 0)
                    {
                        int HealAmount = Players(i).HealthMax - Players(i).ActualHealth;
                        int Expense = Player.Medkit > HealAmount ? HealAmount : Player.Medkit;
                        bool Stabilize = Players(i).ActualHealth + Expense < 0;
                        if (CheckInput(BT_USE, KEY_HELD, false, PlayerNumber()))
                        {
                            SetPlayerProperty(PlayerNumber(), true, PROP_TOTALLYFROZEN);
                            Player.ReviveeNum = i;
                            Players(i).ReviverTID = Player.TID;
                            Players(i).ReviveKeyTimer++;
                            if (Players(i).ReviveKeyTimer >= 105)
                            {
                                if (Stabilize)
                                {
                                    Players(i).ReviverTID = 0;
                                    Players(i).ReviveKeyTimer = 0;
                                    HudMessage("%tS was stabilized", i + 1);
                                    EndHudMessage(HUDMSG_FADEOUT, 0, "Green", 1.5, 0.7, 1.0, 1.0);
                                }
                                else
                                {
                                    ScriptCall("DRPGZUtilities", "ForceRespawn", i);
                                    HudMessage("%tS was revived", i + 1);
                                    EndHudMessageBold(HUDMSG_FADEOUT, 0, "Green", 1.5, 0.8, 1.0, 1.0);
                                }
                                SetPlayerProperty(PlayerNumber(), false, PROP_TOTALLYFROZEN);
                                Players(i).ActualHealth += Expense;
                                Player.Medkit -= Expense;
                            }
                            else if (Players(i).ReviveKeyTimer > 0)
                            {
                                int Percent = CalcPercent(Players(i).ReviveKeyTimer, 105);
                                if (!Stabilize)
                                    DrawProgressBar("Reviving", Percent);
                                else
                                    DrawProgressBar("Stabilizing", Percent);
                            }

                            return;
                        }
                        else
                        {
                            if (!Stabilize)
                                HudMessage("Hold \Cd%jS\C- to revive", "+use");
                            else
                                HudMessage("Hold \Cd%jS\C- to stabilize", "+use");
                            EndHudMessage(HUDMSG_PLAIN, 0, "Green", 1.5, 0.75, 0.05);
                        }
                    }
                    else
                    {
                        HudMessage("Find some medkit to revive");
                        EndHudMessage(HUDMSG_PLAIN, 0, "Green", 1.5, 0.75, 0.05);
                    }
                }
                else
                {
                    HudMessage("%tS is receiving treatment from %tS", i + 1, PlayerNumber());
                    EndHudMessage(HUDMSG_PLAIN, 0, "Green", 1.5, 0.75, 0.05);
                }
            }
        }
    }

    if (Player.ReviveeNum >= 0)
    {
        SetPlayerProperty(PlayerNumber(), false, PROP_TOTALLYFROZEN);
        Players(Player.ReviveeNum).ReviveKeyTimer = 0;
        if (Players(Player.ReviveeNum).ActualHealth <= 0)
            Players(Player.ReviveeNum).ReviverTID = 0;
        Player.ReviveeNum = -1;
    }
}

NamedScript bool SomePlayerAlive()
{
    for (int i = 0; i < PlayerCount(); i++)
        if (Players(i).ActualHealth > 0)
            return true;

    return false;
}
