#include "Defs.h"

#include "Augs.h"
#include "Crate.h"
#include "ItemData.h"
#include "Map.h"
#include "Menu.h"
#include "Minigame.h"
#include "Mission.h"
#include "RPG.h"
#include "Shield.h"
#include "Shop.h"
#include "Skills.h"
#include "Stats.h"
#include "Stims.h"
#include "Turret.h"
#include "Utils.h"

str RPGMap MainMenu[MAX_MENU] =
{
    "Stats",
    "Augs",
    "Skills",
    "Shield",
    "Stims",
    "Turret",
    "Shop"
};

str RPGMap MainMenuColor[MAX_MENU] =
{
    "Orange",
    "Cyan",
    "Fire",
    "Sapphire",
    "Red",
    "Green",
    "Teal"
};

str RPGMap CursorColors[6] =
{
    "Black",
    "Dark Grey",
    "Grey",
    "White",
    "Grey",
    "Dark Grey"
};

str RPGMap MenuCursorColor;

/* --------------------------------------------------

   GUI MENU

   Extremely super WIP, not even close to working
   Don't fuck with it, it'll eat your face and probably
   steal your lunch money too

   -------------------------------------------------- */

NamedScript void UpdateMenu()
{
    while (true)
    {
        Delay(1);
    }
}

// --------------------------------------------------

NamedScript KeyBind void OpenMenu()
{
    // If you're dead, terminate
    if (GetActorProperty(0, APROP_Health) <= 0) return;

    // Exit handling for Outpost menus.
    if (Player.OutpostMenu > 0)
    {
        ActivatorSound("menu/leave", 127);
        SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
        Player.OutpostMenu = 0;
        return;
    }
    // Exit handling for Crates.
    else if (Player.CrateOpen)
    {
        ActivatorSound("crate/close", 127);
        Player.CrateOpen = false;

        // Set the crate to it's inactive state if it's empty
        if (CrateEmpty(Player.CrateID))
        {
            SetActorState(Crates[Player.CrateID].TID, "Empty");
            Crates[Player.CrateID].Empty = true;
        }

        return;
    }
    else if (Player.CrateHacking)
    {
        ActivatorSound("hacking/select", 127);
        Player.CrateHacking = false;
        return;
    }
    // Exit handling for Minigames.
    else if (Player.InMinigame)
    {
        Player.InMinigame = false;
        StopSound(0, CHAN_BODY);
        return;
    }

    if (Player.InShop && CurrentLevel->UACBase)
    {
        ActivatorSound("menu/leave", 127);
        SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
        Player.InMenu = false;
        Player.InShop = false;
        return;
    }
    else if (Player.InShop)
        Player.InShop = false;

    if (Player.InMenu)
    {
        if (Player.Menu > 0)
        {
            Player.Menu = MENUPAGE_MAIN;
            Player.MenuIndex = 0;
            ClearToxicityMeter();
            return;
        }
        else
        {
            ActivatorSound("menu/leave", 127);
            SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
            Player.InMenu = false;
            Player.MenuIndex = 0;
            Player.Menu = MENUPAGE_MAIN;
            ClearToxicityMeter();
        }
    }
    else
    {
        if (DebugLog)
            Log("\CdDEBUG: \CfOpening Menu");
        ActivatorSound("menu/enter", 127);
        Player.InMenu = true;
        Player.Menu = MENUPAGE_MAIN;
        Player.MenuIndex = 0;
    }
}

void MenuLoop()
{
    // Freeze the Player
    SetPlayerProperty(0, 1, PROP_TOTALLYFROZEN);

    // Set the HUD Size
    SetHudSize(GetActivatorCVar("drpg_menu_width"), GetActivatorCVar("drpg_menu_height"), true);

    // Draw Border
    // These are pushed back a bit so the border doesn't overlap anything
    if (GetActivatorCVar("drpg_menu_background_border"))
        DrawBorder("Bor", -1, 8, -5.0, 0.0, 470, 470);

    // Draw current menu
    if (Player.Menu == MENUPAGE_MAIN)   DrawMainMenu();
    if (Player.Menu == MENUPAGE_STATS)  DrawStatsMenu();
    if (Player.Menu == MENUPAGE_AUGS)   DrawAugsMenu();
    if (Player.Menu == MENUPAGE_SKILLS) DrawSkillMenu();
    if (Player.Menu == MENUPAGE_SHIELD) DrawShieldMenu();
    if (Player.Menu == MENUPAGE_STIMS)  DrawStimsMenu();
    if (Player.Menu == MENUPAGE_TURRET) DrawTurretMenu();

    // Handle menu input
    MenuInput();
}

void DrawMainMenu()
{
    fixed X = 0.0;
    fixed Y = 10.0; // So text doesn't stick to the top of the screen
    int CurrentKills = GetLevelInfo(LEVELINFO_KILLED_MONSTERS);
    int CurrentItems = GetLevelInfo(LEVELINFO_FOUND_ITEMS);
    int CurrentSecretsFound = GetLevelInfo(LEVELINFO_FOUND_SECRETS);
    int TotalKills = GetLevelInfo(LEVELINFO_TOTAL_MONSTERS);
    int TotalItems = GetLevelInfo(LEVELINFO_TOTAL_ITEMS);
    int TotalSecretsFound = GetLevelInfo(LEVELINFO_TOTAL_SECRETS);
    int Modules = CheckInventory("DRPGModule");
    int TurretParts = CheckInventory("DRPGTurretPart");
    int AugCanisters = CheckInventory("DRPGAugCanister");
    int AugUpgradeCanisters = CheckInventory("DRPGAugUpgradeCanister");
    int SmallStims = CheckInventory("DRPGStimSmall");
    int MediumStims = CheckInventory("DRPGStimMedium");
    int LargeStims = CheckInventory("DRPGStimLarge");
    int XLStims = CheckInventory("DRPGStimXL");
    int GoldChips = CheckInventory("DRPGChipGold");
    int PlatinumChips = CheckInventory("DRPGChipPlatinum");
    bool AllKills = (CurrentLevel && CurrentLevel->KillBonus);
    bool AllItems = (CurrentLevel && CurrentLevel->ItemsBonus);
    bool AllSecrets = (CurrentLevel && CurrentLevel->SecretsBonus);
    int ShopType = GetCVar("drpg_shoptype");

    if (!CurrentLevel->UACBase && ShopType < 2)
    {
        if (!ShopType)
            MainMenu[6] = "Closed";
        else
            MainMenu[6] = "Locker";
    }
    else
    {
        MainMenu[6] = "Shop";
    }

    SetFont("BIGFONT");

    // Clear the tips
    HudMessage("");
    EndHudMessage(HUDMSG_PLAIN, MAKE_ID('1','T','I','P'), "Untranslated", 0, 0, 0.05);
    HudMessage("");
    EndHudMessage(HUDMSG_PLAIN, MAKE_ID('2','T','I','P'), "Untranslated", 0, 0, 0.05);

    // Show mission info when holding specific input keys
    if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()) && Player.Mission.Active)
    {
        DrawMissionInfo(&Player.Mission, 16, 188, true);
        return;
    }

    // Draw headers
    // Use + 20 (40 for bigfont) on Y between the header and first entry for consistency.
    // Use + 15 on Y between entries to get good spacing.
    HudMessage("- MENU -");
    EndHudMessage(HUDMSG_PLAIN, 0, "Grey",          0.1, 10.0, 0.05);
    HudMessage("CLASS");
    EndHudMessage(HUDMSG_PLAIN, 0, "Grey",          95.1, 10.0, 0.05);
    HudMessage("- CHAR. STATS -");
    EndHudMessage(HUDMSG_PLAIN, 0, "Grey",          180.1, 10.0, 0.05);
    HudMessage("- LEVEL STATS -");
    EndHudMessage(HUDMSG_PLAIN, 0, "Grey",          180.1, 100.0, 0.05);
    HudMessage("- AUG. ITEMS -");
    EndHudMessage(HUDMSG_PLAIN, 0, "Grey",          0.1, 175.0, 0.05);
    HudMessage("- SHIELD -");
    EndHudMessage(HUDMSG_PLAIN, 0, "Grey",          180.1, 175.0, 0.05);
    HudMessage("- STIM ITEMS -");
    EndHudMessage(HUDMSG_PLAIN, 0, "Grey",          0.1, 290.0, 0.05);
    HudMessage("- TOXICITY -");
    EndHudMessage(HUDMSG_PLAIN, 0, "Grey",          180.1, 290.0, 0.05);

    // Draw Menu
    for (int i = 0; i < MAX_MENU; i++)
    {
        HudMessage("%S", MainMenu[i]);
        EndHudMessage(HUDMSG_PLAIN, 0, (i == Player.MenuIndex ? MenuCursorColor : MainMenuColor[i]), X + 10.1, Y + 20.0, 0.05);

        // Separate menu buttons.
        Y += 20.0;

        /*if (i > 0 && i % 5 == 0)
        {
            X += 192.0;
            Y = 10.0;
        }*/
    }

    // Reset X
    X = 0;

    // Player Sprite (represents player class)
    DrawPlayerSprite(PlayerNumber(), 130.1, 80.1);

    // Player Stats
    SetFont("SMALLFONT");
    HudMessage("Level: %d", Player.Level);
    EndHudMessage(HUDMSG_PLAIN, 0, "White",          180.1, 30.0, 0.05);
    HudMessage("XP: %ld / %ld", Player.XP, Player.XPNext);
    EndHudMessage(HUDMSG_PLAIN, 0, "White",          180.1, 45.0, 0.05);
    HudMessage("Title: %S (%d/%d)", Ranks[Player.RankLevel], Player.RankLevel, MAX_RANK);
    EndHudMessage(HUDMSG_PLAIN, 0, "Yellow",         180.1, 60.0, 0.05);
    HudMessage("Rank: %ld / %ld", Player.Rank, Player.RankNext);
    EndHudMessage(HUDMSG_PLAIN, 0, "Gold",         180.1, 75.0, 0.05);

    SetFont("BIGFONT");
    // Modules amount
    HudMessage("%d", Modules);
    EndHudMessage(HUDMSG_PLAIN, 0, "Green",          120.1, 100.0, 0.05);
    // Augs
    HudMessage("%d", AugCanisters);
    EndHudMessage(HUDMSG_PLAIN, 0, "Green",          40.1, 200.0, 0.05);
    HudMessage("%d", AugUpgradeCanisters);
    EndHudMessage(HUDMSG_PLAIN, 0, "Green",          40.1, 220.0, 0.05);
    HudMessage("%d / %d", Player.Augs.SlotsUsed, Player.Augs.Slots);
    EndHudMessage(HUDMSG_PLAIN, 0, "Green",          40.1, 240.0, 0.05);
    HudMessage("%d%%", (int)Player.Augs.Battery);
    EndHudMessage(HUDMSG_PLAIN, 0, "Yellow",         40.1, 260.0, 0.05);
    // Stim
    HudMessage("S: %d\nM: %d\nL: %d\nXL: %d", SmallStims, MediumStims, LargeStims, XLStims);
    EndHudMessage(HUDMSG_PLAIN, 0, "White",          40.1, 330.0, 0.05);
    // Turret parts
    HudMessage("%d", TurretParts);
    EndHudMessage(HUDMSG_PLAIN, 0, "White",          105.1, 127.0, 0.05);
    // Modules
    PrintSprite("UMODA0", 0, 105.1, 110.1, 0.05);
    // Augs
    PrintSprite("AUGCA0", 0, 16.1, 210.1, 0.05);
    PrintSprite("AUGUA0", 0, 16.1, 225.1, 0.05);
    PrintSprite("AUGUB0", 0, 16.1, 250.1, 0.05);
    // Aug battery
    PrintSprite("AUGBATT", 0, 16.1, 265.1, 0.05);
    // Stim
    PrintSprite("STIMB0", 0, 16.1, 340.1, 0.05);
    // Turret parts
    PrintSprite("TPRTA0", 0, 130.1, 135.1, 0.05);

    // Level Stats
    if (!CurrentLevel->UACBase)
    {
        fixed Y1;

        // Compatibility Handling - DoomRL Monsters
        if (CompatMonMode == COMPAT_DRLA && GameSkill() == 5)
        {
            str NameDangerLevel;
            fixed DangerLevelMod = (CheckInventory("RLDangerLevel") / (18.0 + (GetCVar("drpg_ws_use_wads") * 32.0)));
            Y1 = 8.0;

            if (DangerLevelMod >= 0.0) NameDangerLevel = "\CqVery Low\C-";
            if (DangerLevelMod > 0.10) NameDangerLevel = "\CdLow\C-";
            if (DangerLevelMod > 0.25) NameDangerLevel = "\CiMedium\C-";
            if (DangerLevelMod > 0.5) NameDangerLevel = "\CaHigh\C-";
            if (DangerLevelMod > 0.75) NameDangerLevel = "\CgVery High\C-";

            SetFont("SMALLFONT");
            HudMessage("Danger Level: %d (%S)", CheckInventory("RLDangerLevel"), NameDangerLevel);
            EndHudMessage(HUDMSG_PLAIN, 0, "Red", 180.1, 115.0, 0.05);
        }

        // Compatibility Handling - Pandemonia Monsters
        if (CompatMonMode == COMPAT_PANDEMONIA)
        {
            str NameGameLevel;
            fixed GameLevelMod = (CheckInventory("PMGameLevel") / (GetCVar("drpg_ws_use_wads") * 32.0 * 4.0));
            Y1 = 8.0;

            if (GameLevelMod >= 0.0) NameGameLevel = "\CqVery Low\C-";
            if (GameLevelMod > 0.10) NameGameLevel = "\CdLow\C-";
            if (GameLevelMod > 0.25) NameGameLevel = "\CiMedium\C-";
            if (GameLevelMod > 0.5) NameGameLevel = "\CaHigh\C-";
            if (GameLevelMod > 0.75) NameGameLevel = "\CgVery High\C-";

            SetFont("SMALLFONT");
            HudMessage("Game Level: %d - %S", CheckInventory("PMGameLevel"), NameGameLevel);
            EndHudMessage(HUDMSG_PLAIN, 0, "Red", 180.1, 115.0, 0.05);
        }

        SetFont("SMALLFONT");
        HudMessage("Monsters: %d / %d", CurrentKills, TotalKills);
        EndHudMessage(HUDMSG_PLAIN, 0, (AllKills ? "Green" : "Brick"),       180.1, 121.0 + Y1, 0.05);
        HudMessage("Items: %d / %d", CurrentItems, TotalItems);
        EndHudMessage(HUDMSG_PLAIN, 0, (AllItems ? "Green" : "LightBlue"),   180.1, 135.0 + Y1, 0.05);
        HudMessage("Secrets: %d / %d", CurrentSecretsFound, TotalSecretsFound);
        EndHudMessage(HUDMSG_PLAIN, 0, (AllSecrets ? "Green" : "Yellow"),    180.1, 149.0 + Y1, 0.05);
    }

    // Chips
    SetFont("BIGFONT");
    HudMessage("%d", GoldChips);
    EndHudMessage(HUDMSG_PLAIN, 0, "Gold", 110.0, 150.1, 0.05);
    HudMessage("%d", PlatinumChips);
    EndHudMessage(HUDMSG_PLAIN, 0, "White", 150.0, 150.1, 0.05);
    PrintSprite("CHIPGOLD", 0, 95.1, 140.1, 0.05);
    PrintSprite("CHIPPLAT", 0, 135.1, 140.1, 0.05);

    // Shield
    DrawShieldInfo(-1, 180.1, 195.0, 0);

    // Current Stim
    if (Player.Stim.Size > 0)
    {
        SetFont("SMALLFONT");

        // Draw Stim selection
        switch(Player.Stim.Size)
        {
        case 1:
        {
            HudMessage("Small Stim: %d/%d", Player.Stim.Amount, Player.Stim.Capacity);
            EndHudMessage(HUDMSG_PLAIN, 0, "Green", 180.1, 310.0, 0.05);
        }
        break;
        case 2:
        {
            HudMessage("Medium Stim: %d/%d", Player.Stim.Amount, Player.Stim.Capacity);
            EndHudMessage(HUDMSG_PLAIN, 0, "Green", 180.1, 310.0, 0.05);
        }
        break;
        case 3:
        {
            HudMessage("Large Stim: %d/%d", Player.Stim.Amount, Player.Stim.Capacity);
            EndHudMessage(HUDMSG_PLAIN, 0, "Green", 180.1, 310.0, 0.05);
        }
        break;
        case 4:
        {
            HudMessage("Extra Large Stim: %d/%d", Player.Stim.Amount, Player.Stim.Capacity);
            EndHudMessage(HUDMSG_PLAIN, 0, "Green", 180.1, 310.0, 0.05);
        }
        break;
        }

        // Stim Compound Bar
        if (Player.Stim.Size > 0)
            for (int i = 0; i < STIM_MAX; i++)
                if (Player.Stim.Current[i] > 0)
                {
                    DrawBar(StrParam("Stim%d", i + 1), 180.1 + X, 325.0, Player.Stim.Current[i], true);
                    X += Player.Stim.Current[i];
                }
    }

    // Toxicity
    if (Player.Toxicity > 0 || Player.Stim.Size > 0)
    {
        SetFont("SMALLFONT");
        HudMessage("Toxicity: %d%%", Player.Toxicity);
        EndHudMessage(HUDMSG_PLAIN, 0, "Green", 180.1, 335.0, 0.05);
        DrawToxicityBar(180.1, 335.0, false);
    }
}

void DrawStatsMenu()
{
    int *Stats[STAT_MAX] =
    {
        &Player.Strength,
        &Player.Defense,
        &Player.Vitality,
        &Player.Energy,
        &Player.Regeneration,
        &Player.Agility,
        &Player.Capacity,
        &Player.Luck
    };

    // Title
    SetFont("BIGFONT");
    switch(Player.StatPage)
    {
    case STATPAGE_STATS:
    {
        HudMessage("Stats");
        EndHudMessage(HUDMSG_PLAIN, 0, "Green", 0.1, 10.0, 0.05);
    }
    break;
    case STATPAGE_STATXP:
    {
        HudMessage("Stats XP");
        EndHudMessage(HUDMSG_PLAIN, 0, "Green", 0.1, 10.0, 0.05);
    }
    break;
    case STATPAGE_PERKS:
    {
        HudMessage("Perks");
        EndHudMessage(HUDMSG_PLAIN, 0, "Green", 0.1, 10.0, 0.05);
    }
    break;
    case STATPAGE_TEAM:
    {
        HudMessage("Team");
        EndHudMessage(HUDMSG_PLAIN, 0, "Green", 0.1, 10.0, 0.05);
    }
    break;
    }

    // Stats Page
    switch(Player.StatPage)
    {
    case STATPAGE_STATS:
    {
        int Cost = (int)((((fixed)*Stats[Player.MenuIndex] + 1) * (fixed)MODULE_STAT_MULT) * GetCVarFixed("drpg_module_statfactor"));
        if (Cost < 0)
            Cost = -Cost;
        else if (Cost == 0)
            Cost = (int)((1 * (fixed)MODULE_STAT_MULT) * GetCVarFixed("drpg_module_statfactor"));

        // Upgrade Modules
        if (!GetActivatorCVar("drpg_auto_spend"))
        {
            PrintSprite("UMODA0", 0, 16.1, 304.1, 0.05);
            SetFont("BIGFONT");
            HudMessage("%d", CheckInventory("DRPGModule"));
            EndHudMessage(HUDMSG_PLAIN, 0, "Green", 40.1, 288.1, 0.05);
            HudMessage("-%d", Cost);
            EndHudMessage(HUDMSG_PLAIN, 0, "Red", 40.1, 302.1, 0.05);
        }

        SetFont("BIGFONT");
        switch(Player.MenuIndex)
        {
        case 0:
        {
            if (Player.StrengthTotal > Player.Strength)
                HudMessage("Strength: %d (+%d)", Player.Strength, (Player.StrengthNat + Player.SoulRedCount + Player.StrengthBonus));
            else
                HudMessage("Strength: %d", Player.Strength);
            EndHudMessage(HUDMSG_PLAIN, 0, MenuCursorColor,   0.1,    25.0,   0.05);
        }
        break;
        case 1:
        {
            if (Player.DefenseTotal > Player.Defense)
                HudMessage("Defense: %d (+%d)", Player.Defense, (Player.DefenseNat + Player.SoulGreenCount + Player.DefenseBonus));
            else
                HudMessage("Defense: %d", Player.Defense);
            EndHudMessage(HUDMSG_PLAIN, 0, MenuCursorColor,   220.1,  25.0,   0.05);
        }
        break;
        case 2:
        {
            if (Player.VitalityTotal > Player.Vitality)
                HudMessage("Vitality: %d (+%d)", Player.Vitality, (Player.VitalityNat + Player.SoulPinkCount + Player.VitalityBonus));
            else
                HudMessage("Vitality: %d", Player.Vitality);
            EndHudMessage(HUDMSG_PLAIN, 0, MenuCursorColor,   0.1,    75.0,   0.05);
        }
        break;
        case 3:
        {
            if (Player.EnergyTotal > Player.Energy)
                HudMessage("Energy: %d (+%d)", Player.Energy, (Player.EnergyNat + Player.SoulBlueCount + Player.EnergyBonus));
            else
                HudMessage("Energy: %d", Player.Energy);
            EndHudMessage(HUDMSG_PLAIN, 0, MenuCursorColor,   220.1,  75.0,   0.05);
        }
        break;
        case 4:
        {
            if (Player.RegenerationTotal > Player.Regeneration)
                HudMessage("Regeneration: %d (+%d)", Player.Regeneration, (Player.RegenerationNat + Player.SoulPurpleCount + Player.RegenerationBonus));
            else
                HudMessage("Regeneration: %d", Player.Regeneration);
            EndHudMessage(HUDMSG_PLAIN, 0, MenuCursorColor,   0.1,    125.0,  0.05);
        }
        break;
        case 5:
        {
            if (Player.AgilityTotal > Player.Agility)
                HudMessage("Agility: %d (+%d)", Player.Agility, (Player.AgilityNat + Player.SoulOrangeCount + Player.AgilityBonus));
            else
                HudMessage("Agility: %d", Player.Agility);
            EndHudMessage(HUDMSG_PLAIN, 0, MenuCursorColor,   220.1,  125.0,  0.05);
        }
        break;
        case 6:
        {
            if (Player.CapacityTotal > Player.Capacity)
                HudMessage("Capacity: %d (+%d)", Player.Capacity, (Player.CapacityNat + Player.SoulDarkBlueCount + Player.CapacityBonus));
            else
                HudMessage("Capacity: %d", Player.Capacity);
            EndHudMessage(HUDMSG_PLAIN, 0, MenuCursorColor,   0.1,    175.0,  0.05);
        }
        break;
        case 7:
        {
            if (Player.LuckTotal > Player.Luck)
            {
                if ((Player.LuckNat + Player.SoulYellowCount + Player.LuckBonus) > 0 && Player.NomadLuckBonus > 0)
                    HudMessage("Luck: %d (+%d; +%d)", Player.Luck, (Player.LuckNat + Player.SoulYellowCount + Player.LuckBonus), Player.NomadLuckBonus);
                else if ((Player.LuckNat + Player.SoulYellowCount + Player.LuckBonus) == 0 && Player.NomadLuckBonus > 0)
                    HudMessage("Luck: %d (+%d)", Player.Luck, Player.NomadLuckBonus);
                else
                    HudMessage("Luck: %d (+%d)", Player.Luck, (Player.LuckNat + Player.SoulYellowCount + Player.LuckBonus));
            }
            else
                HudMessage("Luck: %d", Player.Luck);
            EndHudMessage(HUDMSG_PLAIN, 0, MenuCursorColor,   220.1,  175.0,  0.05);
        }
        break;
        }

        // Primary Stats
        SetFont("BIGFONT");
        if (Player.StrengthTotal > Player.Strength)
            HudMessage("Strength: %d (+%d)", Player.Strength, (Player.StrengthNat + Player.SoulRedCount + Player.StrengthBonus));
        else
            HudMessage("Strength: %d", Player.Strength);
        EndHudMessage(HUDMSG_PLAIN, 0, "Red",        0.1,    25.0,   0.05);
        if (Player.DefenseTotal > Player.Defense)
            HudMessage("Defense: %d (+%d)", Player.Defense, (Player.DefenseNat + Player.SoulGreenCount + Player.DefenseBonus));
        else
            HudMessage("Defense: %d", Player.Defense);
        EndHudMessage(HUDMSG_PLAIN, 0, "Green",      220.1,  25.0,   0.05);
        if (Player.VitalityTotal > Player.Vitality)
            HudMessage("Vitality: %d (+%d)", Player.Vitality, (Player.VitalityNat + Player.SoulPinkCount + Player.VitalityBonus));
        else
            HudMessage("Vitality: %d", Player.Vitality);
        EndHudMessage(HUDMSG_PLAIN, 0, "Brick",      0.1,    75.0,   0.05);
        if (Player.EnergyTotal > Player.Energy)
            HudMessage("Energy: %d (+%d)", Player.Energy, (Player.EnergyNat + Player.SoulBlueCount + Player.EnergyBonus));
        else
            HudMessage("Energy: %d", Player.Energy);
        EndHudMessage(HUDMSG_PLAIN, 0, "LightBlue",  220.1,  75.0,   0.05);
        if (Player.RegenerationTotal > Player.Regeneration)
            HudMessage("Regeneration: %d (+%d)", Player.Regeneration, (Player.RegenerationNat + Player.SoulPurpleCount + Player.RegenerationBonus));
        else
            HudMessage("Regeneration: %d", Player.Regeneration);
        EndHudMessage(HUDMSG_PLAIN, 0, "Purple",     0.1,    125.0,  0.05);
        if (Player.AgilityTotal > Player.Agility)
            HudMessage("Agility: %d (+%d)", Player.Agility, (Player.AgilityNat + Player.SoulOrangeCount + Player.AgilityBonus));
        else
            HudMessage("Agility: %d", Player.Agility);
        EndHudMessage(HUDMSG_PLAIN, 0, "Orange",     220.1,  125.0,  0.05);
        if (Player.CapacityTotal > Player.Capacity)
            HudMessage("Capacity: %d (+%d)", Player.Capacity, (Player.CapacityNat + Player.SoulDarkBlueCount + Player.CapacityBonus));
        else
            HudMessage("Capacity: %d", Player.Capacity);
        EndHudMessage(HUDMSG_PLAIN, 0, "Blue",       0.1,    175.0,  0.05);
        if (Player.LuckTotal > Player.Luck)
        {
            if ((Player.LuckNat + Player.SoulYellowCount + Player.LuckBonus) > 0 && Player.NomadLuckBonus > 0)
                HudMessage("Luck: %d (+%d; +%d)", Player.Luck, (Player.LuckNat + Player.SoulYellowCount + Player.LuckBonus), Player.NomadLuckBonus);
            else if ((Player.LuckNat + Player.SoulYellowCount + Player.LuckBonus) == 0 && Player.NomadLuckBonus > 0)
                HudMessage("Luck: %d (+%d)", Player.Luck, Player.NomadLuckBonus);
            else
                HudMessage("Luck: %d (+%d)", Player.Luck, (Player.LuckNat + Player.SoulYellowCount + Player.LuckBonus));
        }
        else
            HudMessage("Luck: %d", Player.Luck);
        EndHudMessage(HUDMSG_PLAIN, 0, "Gold",       220.1,  175.0,  0.05);

        // Ammo Amounts
        int Bullets = CheckInventory("Clip");
        int MaxBullets = GetAmmoCapacity("Clip");
        int Shells = CheckInventory("Shell");
        int MaxShells = GetAmmoCapacity("Shell");
        int Rockets = CheckInventory("RocketAmmo");
        int MaxRockets = GetAmmoCapacity("RocketAmmo");
        int Cells = CheckInventory("Cell");
        int MaxCells = GetAmmoCapacity("Cell");

        // Secondary Stats
        SetFont("SMALLFONT");
        if (Player.StrengthTotal > 0)
        {
            HudMessage("+%d%% Base Damage", Player.LevelDamage);
            EndHudMessage(HUDMSG_PLAIN, 0, "Red",                30.1,   36.0,   0.05);
        }
        else
        {
            HudMessage("%d%% Base Damage", Player.LevelDamage);
            EndHudMessage(HUDMSG_PLAIN, 0, "Red",                30.1,   36.0,   0.05);
        }
        if (Player.StrengthTotal > 0)
        {
            HudMessage("+%d%% Bonus Damage", Player.BonusDamage);
            EndHudMessage(HUDMSG_PLAIN, 0, "Red",                30.1,   44.0,   0.05);
        }
        else
        {
            HudMessage("%d%% Bonus Damage", Player.BonusDamage);
            EndHudMessage(HUDMSG_PLAIN, 0, "Red",                30.1,   44.0,   0.05);
        }
        HudMessage("%.2kX Multiplier", Player.DamageMult);
        EndHudMessage(HUDMSG_PLAIN, 0, "Red",                30.1,   52.0,   0.05);
        if (Player.StrengthTotal > 0)
        {
            HudMessage("+%d%% Total Damage", (int)(Player.TotalDamage * Player.DamageMult));
            EndHudMessage(HUDMSG_PLAIN, 0, "Red",                30.1,   60.0,   0.05);
        }
        else
        {
            HudMessage("%d%% Total Damage", (int)(Player.TotalDamage * Player.DamageMult));
            EndHudMessage(HUDMSG_PLAIN, 0, "Red",                30.1,   60.0,   0.05);
        }
        if (Player.DefenseTotal > 0)
        {
            HudMessage("%.2k%% Damage Reduction", (Player.DefenseTotal > 0 ? (1.0 - Player.DamageFactor) : 0) * 100.0);
            EndHudMessage(HUDMSG_PLAIN, 0, "Green",              250.1,  36.0,   0.05);
        }
        else
        {
            HudMessage("%.2k%% Damage Reduction", ((1.0 - Player.DamageFactor) * 100.0));
            EndHudMessage(HUDMSG_PLAIN, 0, "Green",              250.1,  36.0,   0.05);
        }
        HudMessage("%d Mass", Player.Mass);
        EndHudMessage(HUDMSG_PLAIN, 0, "Green",              250.1,  44.0,   0.05);
        HudMessage("%d Max HP", Player.HealthMax);
        EndHudMessage(HUDMSG_PLAIN, 0, "Brick",              30.1,   86.0,   0.05);
        HudMessage("%d HP Regen.", Player.HPAmount);
        EndHudMessage(HUDMSG_PLAIN, 0, "Brick",              30.1,   94.0,   0.05);
        HudMessage("%.2k%% Status Resist", Player.StatusEffectResist);
        EndHudMessage(HUDMSG_PLAIN, 0, "Brick",              30.1,   102.0,  0.05);
        HudMessage("%d Max EP", Player.EPMax);
        EndHudMessage(HUDMSG_PLAIN, 0, "LightBlue",          250.1,  86.0,   0.05);
        HudMessage("%d EP Regen.", Player.EPAmount);
        EndHudMessage(HUDMSG_PLAIN, 0, "LightBlue",          250.1,  94.0,   0.05);
        HudMessage("%S Sec Aura Time", FormatTime(AURA_CALCTIME));
        EndHudMessage(HUDMSG_PLAIN, 0, "LightBlue",          250.1,  102.0,  0.05);
        HudMessage("%d Aura Range", (int)Player.Aura.Range);
        EndHudMessage(HUDMSG_PLAIN, 0, "LightBlue",          250.1,  110.0,  0.05);
        HudMessage("HP Timer: %.2k Sec", (fixed)(Player.HPTime / 35.0K));
        EndHudMessage(HUDMSG_PLAIN, 0, "Brick",              30.1,   136.0,  0.05);
        HudMessage("EP Timer: %.2k Sec", (fixed)(Player.EPTime / 35.0K));
        EndHudMessage(HUDMSG_PLAIN, 0, "LightBlue",          30.1,   144.0,  0.05);
        if (GetCVar("drpg_levelup_natural"))
        {
            HudMessage("Regen. Sphere: %d Sec", (int)(40 + Player.RegenerationTotal));
            EndHudMessage(HUDMSG_PLAIN, 0, "Purple",             30.1,   152.0,  0.05);
        }
        else
        {
            HudMessage("Regen. Sphere: %d Sec", (int)(40 + Player.RegenerationTotal * 2));
            EndHudMessage(HUDMSG_PLAIN, 0, "Purple",             30.1,   152.0,  0.05);
        }
        HudMessage("Toxicity Timer: %d Sec", 30 - Player.ToxicityRegenBonus);
        EndHudMessage(HUDMSG_PLAIN, 0, "Green",              30.1,   160.0,  0.05);
        HudMessage("Movement Speed: %.2k", Player.Speed);
        EndHudMessage(HUDMSG_PLAIN, 0, "Orange",             250.1,  136.0,  0.05);
        HudMessage("Jump Height: %.2k", Player.JumpHeight);
        EndHudMessage(HUDMSG_PLAIN, 0, "Orange",             250.1,  144.0,  0.05);
        HudMessage("+%d%% Weapon Speed", Player.WeaponSpeed);
        EndHudMessage(HUDMSG_PLAIN, 0, "Orange",             250.1,  152.0,  0.05);
        HudMessage("%d%% Survival Bonus", Player.SurvivalBonus);
        EndHudMessage(HUDMSG_PLAIN, 0, "Orange",             250.1,  160.0,  0.05);
        HudMessage("Bullets: %d/%d", Bullets, MaxBullets);
        EndHudMessage(HUDMSG_PLAIN, 0, "Brick",              30.1,   186.0,  0.05);
        HudMessage("Shells: %d/%d", Shells, MaxShells);
        EndHudMessage(HUDMSG_PLAIN, 0, "Orange",             30.1,   194.0,  0.05);
        HudMessage("Rockets: %d/%d", Rockets, MaxRockets);
        EndHudMessage(HUDMSG_PLAIN, 0, "DarkGray",           30.1,   202.0,  0.05);
        HudMessage("Cells: %d/%d", Cells, MaxCells);
        EndHudMessage(HUDMSG_PLAIN, 0, "Green",              30.1,   210.0,  0.05);
        HudMessage("Aug. Battery Max: %d%%", (int)Player.Augs.BatteryMax);
        EndHudMessage(HUDMSG_PLAIN, 0, "Yellow",             30.1,   218.0,  0.05);
        HudMessage("Stim Vial Max: %d", Player.Stim.VialMax);
        EndHudMessage(HUDMSG_PLAIN, 0, "Cyan",               30.1,   226.0,  0.05);
        HudMessage("Medkit: %d/%d", Player.Medkit, Player.MedkitMax);
        EndHudMessage(HUDMSG_PLAIN, 0, "Brick",              30.1,   234.0,  0.05);
        if (GetCVar("drpg_inv_capacity"))
        {
            HudMessage("Inv. Items: %d/%d", Player.InvItems, CheckInventoryMax());
            EndHudMessage(HUDMSG_PLAIN, 0, "White",          30.1,   242.0,  0.05);
        }
        if (CompatMode == COMPAT_DRLA)
        {
            // Compatibility Handling - DoomRL Arsenal Extended
            // Phase Sisters - Change the Weapons calculation
            if (CompatModeEx == COMPAT_DRLAX && PlayerClass(PlayerNumber()) == 9)
                HudMessage("Weapons: %d/%d", CheckInventory("RLWeaponLimit") - 2, DRLA_WEAPON_MAX);
            else
                HudMessage("Weapons: %d/%d", CheckInventory("RLWeaponLimit"), DRLA_WEAPON_MAX);
            EndHudMessage(HUDMSG_PLAIN, 0, "Brick",              30.1,   250.0,  0.05);
            HudMessage("Armors: %d/%d", CheckInventory("RLArmorInInventory"), DRLA_ARMOR_MAX);
            EndHudMessage(HUDMSG_PLAIN, 0, "Green",              30.1,   258.0,  0.05);
            HudMessage("Skulls: %d/%d", CheckInventory("RLSkullLimit"), DRLA_SKULL_MAX);
            EndHudMessage(HUDMSG_PLAIN, 0, "Orange",             30.1,   266.0,  0.05);
            HudMessage("Phase Devices: %d/%d", CheckInventory("RLPhaseDeviceLimit"), DRLA_DEVICE_MAX);
            EndHudMessage(HUDMSG_PLAIN, 0, "Gray",               30.1,   274.0,  0.05);
        };

        // Luck
        SetFont("BIGFONT");
        HudMessage("%d", LUCK_HEALTHDROP);
        EndHudMessage(HUDMSG_PLAIN, 0, (Player.HealthDrop ? "Gold" : "Gray"), 266.0, 204.0, 0.05);
        HudMessage("%d", LUCK_EPDROP);
        EndHudMessage(HUDMSG_PLAIN, 0, (Player.EPDrop ? "Gold" : "Gray"), 304.0, 204.0, 0.05);
        HudMessage("%d", LUCK_AMMODROP);
        EndHudMessage(HUDMSG_PLAIN, 0, (Player.AmmoDrop ? "Gold" : "Gray"), 342.0, 204.0, 0.05);
        HudMessage("%d", LUCK_TURRETDROP);
        EndHudMessage(HUDMSG_PLAIN, 0, (Player.TurretDrop ? "Gold" : "Gray"), 266.0, 240.0, 0.05);
        HudMessage("%d", LUCK_MODULEDROP);
        EndHudMessage(HUDMSG_PLAIN, 0, (Player.ModuleDrop ? "Gold" : "Gray"), 304.0, 240.0, 0.05);
        HudMessage("%d", LUCK_ARMORDROP);
        EndHudMessage(HUDMSG_PLAIN, 0, (Player.ArmorDrop ? "Gold" : "Gray"), 342.0, 240.0, 0.05);
        HudMessage("%d", LUCK_WEAPONDROP);
        EndHudMessage(HUDMSG_PLAIN, 0, (Player.WeaponDrop ? "Gold" : "Gray"), 266.0, 275.0, 0.05);
        HudMessage("%d", LUCK_SHIELDDROP);
        EndHudMessage(HUDMSG_PLAIN, 0, (Player.ShieldDrop ? "Gold" : "Gray"), 304.0, 276.0, 0.05);
        HudMessage("%d", LUCK_AUGDROP);
        EndHudMessage(HUDMSG_PLAIN, 0, (Player.AugDrop ? "Gold" : "Gray"), 342.0, 276.0, 0.05);
        PrintSprite("MEDIA0", 0, 280.0, 212.0, 0.05);
        PrintSprite("EPUPA0", 0, 320.0, 213.0, 0.05);
        PrintSprite("AMMOA0", 0, 354.0, 208.0, 0.05);
        PrintSprite("PTURA2A8", 0, 278.0, 234.0, 0.05);
        PrintSprite("UMODA0", 0, 314.0, 245.0, 0.05);
        PrintSprite("ARM1A0", 0, 356.0, 238.0, 0.05);
        PrintSprite("PISTA0", 0, 277.0, 272.0, 0.05);
        PrintSprite("SHPAA0", 0, 322.0, 293.0, 0.05);
        PrintSprite("AUGCA0", 0, 354.0, 282.0, 0.05);
        SetFont("SMALLFONT");
        if (Player.LuckTotal > 0)
        {
            HudMessage("\CfCredit\C- Drop Rate: \Cf+%d%%", (GetCVar("drpg_levelup_natural") ? Player.LuckTotal / 2 : Player.LuckTotal));
            EndHudMessage(HUDMSG_PLAIN, 0, "White", 250.1, 292.0, 0.05);
        }
        if (Player.HealthDrop)
        {
            HudMessage("\CaHP\C- Drop Rate: \Cf%.3k%%", Player.HealthChance);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", 250.1, 300.0, 0.05);
        }
        if (Player.EPDrop)
        {
            HudMessage("\CnEP\C- Drop Rate: \Cf%.3k%%", Player.EPChance);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", 250.1, 308.0, 0.05);
        }
        if (Player.AmmoDrop)
        {
            HudMessage("\CdAmmo\C- Drop Rate: \Cf%.3k%%", Player.AmmoChance);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", 250.1, 316.0, 0.05);
        }
        if (Player.TurretDrop)
        {
            HudMessage("\CgTurret\C- Drop Rate: \Cf%.3k%%", Player.TurretChance);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", 250.1, 324.0, 0.05);
        }
        if (Player.ModuleDrop)
        {
            HudMessage("\CdModule\C- Drop Rate: \Cf%.3k%%", Player.ModuleChance);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", 250.1, 332.0, 0.05);
        }
        if (Player.ArmorDrop)
        {
            HudMessage("\CrArmor\C- Drop Rate: \Cf%.3k%%", Player.ArmorChance);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", 250.1, 340.0, 0.05);
        }
        if (Player.WeaponDrop)
        {
            HudMessage("\CqWeapon\C- Drop Rate: \Cf%.3k%%", Player.WeaponChance);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", 250.1, 348.0, 0.05);
        }
        if (Player.ShieldDrop)
        {
            HudMessage("\CvShield\C- Drop Rate: \Cf%.3k%%", Player.ShieldChance);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", 250.1, 356.0, 0.05);
        }
        if (Player.AugDrop)
        {
            HudMessage("\CkAug.\C- Drop Rate: \Cf%.3k%%", Player.AugChance);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", 250.1, 364.0, 0.05);
        }

        // Perk Icons
        if (Player.Perks[STAT_STRENGTH]) PrintSprite("STATP", 0, 16.1 + 8.0, 60.1, 0.05);
        if (Player.Perks[STAT_DEFENSE]) PrintSprite("STATP", 0, 214.1 + 8.0, 56.1, 0.05);
        if (Player.Perks[STAT_VITALITY]) PrintSprite("STATP", 0, 16.1 + 8.0, 110.1, 0.05);
        if (Player.Perks[STAT_ENERGY]) PrintSprite("STATP", 0, 214.1 + 8.0, 112.1, 0.05);
        if (Player.Perks[STAT_REGENERATION]) PrintSprite("STATP", 0, 16.1 + 8.0, 162.1, 0.05);
        if (Player.Perks[STAT_AGILITY]) PrintSprite("STATP", 0, 214.1 + 8.0, 160.1, 0.05);
        if (Player.Perks[STAT_CAPACITY]) PrintSprite("STATP", 0, 16.1 + 8.0, 210.1, 0.05);
        if (Player.Perks[STAT_LUCK]) PrintSprite("STATP", 0, 216.1 + 8.0, 210.1, 0.05);

        // Icons
        PrintSprite("STAT1", 0, 16.1, 60.1, 0.05);
        PrintSprite("STAT2", 0, 234.1, 56.1, 0.05);
        PrintSprite("STAT3", 0, 16.1, 110.1, 0.05);
        PrintSprite("STAT4", 0, 234.1, 112.1, 0.05);
        PrintSprite("STAT5", 0, 16.1, 162.1, 0.05);
        PrintSprite("STAT6", 0, 234.1, 160.1, 0.05);
        PrintSprite("STAT7", 0, 16.1, 210.1, 0.05);
        PrintSprite("STAT8", 0, 236.1, 210.1, 0.05);
    };
    break;
    // Stats XP Page
    case STATPAGE_STATXP:
    {
        str const StatNames[STAT_MAX] =
        {
            "Strength",
            "Defense",
            "Vitality",
            "Energy",
            "Regen",
            "Agility",
            "Capacity",
            "Luck"
        };

        int *Stats[STAT_MAX] =
        {
            &Player.StrengthNat,
            &Player.DefenseNat,
            &Player.VitalityNat,
            &Player.EnergyNat,
            &Player.RegenerationNat,
            &Player.AgilityNat,
            &Player.CapacityNat,
            &Player.LuckNat
        };

        long int *StatXP[STAT_MAX] =
        {
            &Player.StrengthXP,
            &Player.DefenseXP,
            &Player.VitalityXP,
            &Player.EnergyXP,
            &Player.RegenerationXP,
            &Player.AgilityXP,
            &Player.CapacityXP,
            &Player.LuckXP
        };

        for (int i = 0; i < STAT_MAX; i++)
        {
            // Icons
            if (*Stats[i] < NATURALCAP)
                PrintSprite(StrParam("STAT%d", i + 1), 0, 16.1, 56.1 + (i * 44.0), 0.05);
            else
                PrintSpritePulse(StrParam("STAT%d", i + 1), 0, 16.1, 56.1 + (i * 44.0), 0.75, 64.0, 0.25);

            // XP
            SetFont("BIGFONT");
            if (*Stats[i] < NATURALCAP)
            {
                HudMessage("%S: %ld/%ld (+%d)", StatNames[i], *StatXP[i], StatTable[*Stats[i]], *Stats[i]);
                EndHudMessage(HUDMSG_PLAIN, 0, StatColors[i], 40.1, 32.0 + (i * 44.0), 0.05);
            }
            else
            {
                HudMessage("%S: %ld (+%d)", StatNames[i], *StatXP[i], *Stats[i]);
                EndHudMessage(HUDMSG_PLAIN, 0, StatColors[i], 40.1, 32.0 + (i * 44.0), 0.05);
            }

            // Bar
            long int CurrentXP = *StatXP[i];
            long int NextXP = StatTable[*Stats[i]];
            long int StatPercent = (CurrentXP * 100) / NextXP;
            if (*Stats[i] >= NATURALCAP)
                StatPercent = 100;
            if (StatPercent > 100)
                StatPercent = 100;
            SetHudClipRect(40.1, 40.0 + (i * 44.0), StatPercent * 3, 56.0 + (i * 44.0));
            PrintSprite(StrParam("StatBar%d", i + 1), 0, 40.1, 48.0 + (i * 44.0), 0.05);
            SetHudClipRect(0, 0, 0, 0);
            PrintSprite("StatBarB", 0, 40.1, 48.0 + (i * 44.0), 0.05);
        }
    }
    break;
    // Perks Page
    case STATPAGE_PERKS:
    {
        bool naturalStats = GetCVar("drpg_levelup_natural");

        // Holds the perk information
        str const PerkInfo[STAT_MAX][4] =
        {
            // Strength
            {
                StrParam("Strength level - %d:", (naturalStats ? 150 : 75)),
                "Total damage dealt growingly increases as HP gets lower",
                NULL
            },

            // Defense
            {
                StrParam("Defense level - %d:", (naturalStats ? 150 : 75)),
                "Total damage taken growingly decreases as HP gets lower",
                NULL
            },

            // Vitality
            {
                StrParam("Vitality level - %d:", (naturalStats ? 150 : 75)),
                "No movement penalties at a HP critical level",
                "200% HP regeneration rate when below 20% max HP",
                NULL
            },

            // Energy
            {
                StrParam("Energy level - %d:", (naturalStats ? 100 : 50)),
                "200% EP regeneration rate when burned out",
                (naturalStats ? "Can stack an extra Aura every 50 Energy invested" : "Can stack an extra Aura every 25 Energy invested"),
                NULL
            },

            // Regeneration
            {
                StrParam("Regeneration level - %d:", (naturalStats ? 150 : 75)),
                "Regeneration rate increases as HP/EP gets lower",
                NULL
            },

            // Agility
            {
                StrParam("Agility level - %d:", (naturalStats ? 150 : 75)),
                "+15% Survival Bonus",
                "Movement increases HP/EP regeneration rate",
                NULL
            },

            // Capacity
            {
                StrParam("Capacity level - %d:", (naturalStats ? 200 : 100)),
                "Ammo replenishment",
                NULL
            },

            // Luck
            {
                StrParam("Luck level - %d:", (naturalStats ? 200 : 100)),
                "Always have full auto map",
                NULL
            }
        };

        for (int i = 0; i < STAT_MAX; i++)
        {
            str Description = "";
            str Color = "Gray";

            // Icon
            if (Player.Perks[i])
                PrintSpritePulse(StrParam("STAT%d", i + 1), 0, 16.1, 56.1 + (i * 44.0), 0.75, 64.0, 0.25);
            else
                PrintSpriteAlpha(StrParam("STAT%d", i + 1), 0, 16.1, 56.1 + (i * 44.0), 0.05, 0.5);

            // Build description string
            for (int j = (Player.Perks[i] ? 1 : 0); PerkInfo[i][j] != NULL; j++)
                Description = StrParam("%S%S\n", Description, PerkInfo[i][j]);

            // Determine Color
            if (Player.Perks[i])
                Color = StatColors[i];

            // Descriptions
            SetFont("SMALLFONT");
            HudMessage("%S", Description);
            EndHudMessage(HUDMSG_PLAIN, 0, Color, 40.1, 44.0 + (i * 44.0), 0.05);
        }
    }
    break;
    // Team Page
    case STATPAGE_TEAM:
    {
        PlayerData *PlayerPtr = &Players(Player.MenuIndex);
        int PlayerNum = Player.MenuIndex;
        fixed X = 32.1;
        fixed Y = 52.0;

        // Player List
        SetFont("SMALLFONT");
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (!PlayerInGame(i)) continue;

            // Currently selected
            if (Player.MenuIndex == i)
            {
                HudMessage("-->");
                EndHudMessage(HUDMSG_PLAIN, 0, MenuCursorColor, X - 24.0, Y, 0.05);
            }

            if (i == PlayerNumber())
            {
                HudMessage("%tS", i + 1);
                EndHudMessage(HUDMSG_PLAIN, 0, "Green", X, Y, 0.05);
            }
            else
            {
                HudMessage("%tS (\Ci%d\C-)", i + 1, (int)Distance(Player.TID, Players(i).TID));
                EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            }

            Y += 8.0;
        }

        // Player Icon
        DrawPlayerSprite(PlayerNum, 184.1, 100.1);

        // XP/Rank
        SetFont("BIGFONT");
        HudMessage("Level: %d", PlayerPtr->Level);
        EndHudMessage(HUDMSG_PLAIN, 0, "White", 208.1, 56.0, 0.05);
        HudMessage("XP: %ld / %ld", PlayerPtr->XP, PlayerPtr->XPNext);
        EndHudMessage(HUDMSG_PLAIN, 0, "White", 208.1, 68.0, 0.05);
        HudMessage("Title: %S (%d/%d)", Ranks[PlayerPtr->RankLevel], PlayerPtr->RankLevel, MAX_RANK);
        EndHudMessage(HUDMSG_PLAIN, 0, "Yellow", 208.1, 80.0, 0.05);
        HudMessage("Rank: %ld / %ld", PlayerPtr->Rank, PlayerPtr->RankNext);
        EndHudMessage(HUDMSG_PLAIN, 0, "Yellow", 208.1, 92.0, 0.05);

        // Stats
        HudMessage("%d", PlayerPtr->StrengthTotal);
        EndHudMessage(HUDMSG_PLAIN, 0, "Red", 244.0, 120.0, 0.05);
        HudMessage("%d", PlayerPtr->DefenseTotal);
        EndHudMessage(HUDMSG_PLAIN, 0, "Green", 244.0, 140.0, 0.05);
        HudMessage("%d", PlayerPtr->VitalityTotal);
        EndHudMessage(HUDMSG_PLAIN, 0, "Brick", 244.0, 160.0, 0.05);
        HudMessage("%d", PlayerPtr->EnergyTotal);
        EndHudMessage(HUDMSG_PLAIN, 0, "LightBlue", 244.0, 180.0, 0.05);
        HudMessage("%d", PlayerPtr->RegenerationTotal);
        EndHudMessage(HUDMSG_PLAIN, 0, "Purple", 244.0, 200.0, 0.05);
        HudMessage("%d", PlayerPtr->AgilityTotal);
        EndHudMessage(HUDMSG_PLAIN, 0, "Orange", 244.0, 220.0, 0.05);
        HudMessage("%d", PlayerPtr->CapacityTotal);
        EndHudMessage(HUDMSG_PLAIN, 0, "Blue", 244.0, 240.0, 0.05);
        HudMessage("%d", PlayerPtr->LuckTotal);
        EndHudMessage(HUDMSG_PLAIN, 0, "Yellow", 244.0, 260.0, 0.05);
        PrintSprite("STAT1S", 0, 228.0, 139.0, 0.05);
        PrintSprite("STAT2S", 0, 228.0, 157.0, 0.05);
        PrintSprite("STAT3S", 0, 228.0, 179.0, 0.05);
        PrintSprite("STAT4S", 0, 228.0, 199.0, 0.05);
        PrintSprite("STAT5S", 0, 228.0, 221.0, 0.05);
        PrintSprite("STAT6S", 0, 228.0, 240.0, 0.05);
        PrintSprite("STAT7S", 0, 228.0, 260.0, 0.05);
        PrintSprite("STAT8S", 0, 228.0, 280.0, 0.05);

        // Augs
        for (int i = 0; i < AUG_MAX; i++)
        {
            AugInfoPtr AugPtr = &AugData[i];

            SetFont("SMALLFONT");

            if (PlayerPtr->Augs.Active[i])
            {
                HudMessage("E");
                EndHudMessage(HUDMSG_PLAIN, 0, "Green", 271.1 + ((i % 3) * 34.0), 140.0 + ((i / 3) * 34.0), 0.05);
            }

            // Draw the level and max level
            HudMessage("%d/%d", PlayerPtr->Augs.CurrentLevel[i], AugPtr->MaxLevel);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", 303.2 + ((i % 3) * 34.0), 140.0 + ((i / 3) * 34.0), 0.05);

            // Icon
            if (PlayerPtr->Augs.Active[i])
                PrintSprite(StrParam("Aug%dE", i + 1), 0, 280.1 + ((i % 3) * 34.0), 120.1 + ((i / 3) * 34.0), 0.05);
            else if (PlayerPtr->Augs.Level[i] > 0)
                PrintSprite(StrParam("Aug%dB", i + 1), 0, 280.1 + ((i % 3) * 34.0), 120.1 + ((i / 3) * 34.0), 0.05);
            else
                PrintSprite(StrParam("Aug%d", i + 1), 0, 280.1 + ((i % 3) * 34.0), 120.1 + ((i / 3) * 34.0), 0.05);
        }

        // Aug Battery
        PrintSprite("AUGBATT", 0, 286.1, 228.1, 0.05);
        SetFont("BIGFONT");
        HudMessage("%d%%", (int)PlayerPtr->Augs.Battery);
        EndHudMessage(HUDMSG_PLAIN, 0, "Yellow", 306.1, 228.0, 0.05);

        // Toxicity
        if (PlayerPtr->Toxicity > 0 || PlayerPtr->Stim.Size > 0)
        {
            SetFont("BIGFONT");
            HudMessage("Toxicity: %d%%", Player.Toxicity);
            EndHudMessage(HUDMSG_PLAIN, 0, "Green", 276.1, 250.0, 0.05);
            DrawToxicityBar(276.1, 250.0, true);
        }

        // Shield
        DrawShieldInfo(Player.MenuIndex, 232, 281, 0);

        // Health/Armor
        int Health = PlayerPtr->ActualHealth;
        str Armor = (CompatMode == COMPAT_DRLA && CheckActorInventory(PlayerPtr->TID, "RLIndestructibleArmorWorn") ? "\CaIndestructible\Cd" : StrParam("%d", CheckActorInventory(PlayerPtr->TID, "Armor")));
        SetFont("SMALLFONT");
        if (CheckActorInventory(PlayerPtr->TID, "Armor") > 0)
        {
            HudMessage("\Ca%d Health\C- / \Cd%S Armor", Health, Armor);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", 48.1, 348.0, 0.05);
        }
        else
        {
            HudMessage("\Ca%d Health", Health);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", 48.1, 348.0, 0.05);
        }

        // Weapon
        for (int i = 0; i < ItemMax[0]; i++)
        {
            ItemInfoPtr ItemPtr = &ItemData[0][i];

            // Horrible, horrible hacks
            if (PlayerWeapon[PlayerNum] == ItemPtr->Actor)
            {
                SetFont("SMALLFONT");
                HudMessage("\CgWeapon: \C-%S", ItemPtr->Name);
                EndHudMessage(HUDMSG_PLAIN, 0, "White", 48.1, 356.0, 0.05);
                PrintSprite(ItemPtr->Sprite.Name, 0, 100.0 + ItemPtr->Sprite.XOff, 312 + ItemPtr->Sprite.YOff, 0.05);
                break;
            }
        }

        // Armor
        for (int i = 1; i < ItemMax[3]; i++)
        {
            ItemInfoPtr ItemPtr = &ItemData[3][i];
            str Actor = ItemPtr->Actor;

            // DoomRL Compatibility
            if (CompatMode == COMPAT_DRLA)
            {
                Actor = StrLeft(Actor, StrLen(Actor) - 6); // Strip "Pickup" off the end
                Actor = StrParam("%SToken", Actor);
            }

            if ((CompatMode == COMPAT_DRLA ? CheckActorInventory(PlayerPtr->TID, Actor) : GetArmorType(StrParam("%SEffect", Actor), PlayerNum)))
            {
                SetFont("SMALLFONT");
                HudMessage("\CdArmor: \C-%S", ItemPtr->Name);
                EndHudMessage(HUDMSG_PLAIN, 0, "White", 48.1, 364.0, 0.05);
                PrintSprite(ItemPtr->Sprite.Name, 0, 160.0 + ItemPtr->Sprite.XOff, 312 + ItemPtr->Sprite.YOff, 0.05);
                break;
            }
        }
    }
    break;
    }
}

ItemInfoPtr GetIncreaseAugAddItem(int AugIndex, int AugLevel)
{
    // Get additional item for Enhanced Battery Augmentation
    if (AugIndex == 8)
    {
        if (AugLevel <= 3) return FindItem("DRPGBatterySmall");
        if (AugLevel > 3) return FindItem("DRPGBatteryLarge");
    }
}

int GetIncreaseAugAddCost(int AugIndex, int AugLevel)
{
    // Get ammount additional items for Enhanced Battery Augmentation
    if (AugIndex == 8)
    {
        if (AugLevel <= 3) return 1 + AugLevel;
        if (AugLevel > 3) return AugLevel - 3;
    }
}

void DrawAugsMenu()
{
    // Titles
    SetFont("BIGFONT");
    HudMessage("Augmentations");
    EndHudMessage(HUDMSG_PLAIN, 0, "Green", 0.1, 10.0, 0.05);

    // Upgrade Requirements (if applicable)
    int SelectedIndex = Player.MenuIndex;
    AugInfoPtr SelectedAug = &AugData[SelectedIndex];
    str UpgradeCanisterAmount = "";
    str UpgradeComponentAmount = "";
    ItemInfoPtr  AdditionalItem;
    int AdditionalItemAmount;
    if (Player.Augs.Level[SelectedIndex] == 0)
    {
        UpgradeCanisterAmount = " \Cg(-1)";
        AdditionalItem = GetIncreaseAugAddItem(SelectedIndex, Player.Augs.Level[SelectedIndex]);
        AdditionalItemAmount = GetIncreaseAugAddCost(SelectedIndex, Player.Augs.Level[SelectedIndex]);
    }
    else if (Player.Augs.Level[SelectedIndex] < SelectedAug->MaxLevel)
    {
        UpgradeCanisterAmount = " \Cg(-1)";
        UpgradeComponentAmount = StrParam(" \Cg(%d)", -(Player.Augs.Level[SelectedIndex] + 1));
        AdditionalItem = GetIncreaseAugAddItem(SelectedIndex, Player.Augs.Level[SelectedIndex]);
        AdditionalItemAmount = GetIncreaseAugAddCost(SelectedIndex, Player.Augs.Level[SelectedIndex]);
    };

    // Aug Canisters
    PrintSprite("AUGCA0", 0, 200.0, 54.0, 0.05);
    SetFont("BIGFONT");
    HudMessage("%d%S", CheckInventory("DRPGAugCanister"), UpgradeCanisterAmount);
    EndHudMessage(HUDMSG_PLAIN, 0, "Green", 208.1, 30.0, 0.05);

    // Aug Upgrade Canisters
    PrintSprite("AUGUA0", 0, 200.0, 72.0, 0.05);
    SetFont("BIGFONT");
    HudMessage("%d%S", CheckInventory("DRPGAugUpgradeCanister"), UpgradeComponentAmount);
    EndHudMessage(HUDMSG_PLAIN, 0, "Green", 208.1, 53.0, 0.05);

    // Aug Slots
    PrintSprite("AUGUB0", 0, 204.0, 102.0, 0.05);
    SetFont("BIGFONT");
    HudMessage("%d / %d", Player.Augs.SlotsUsed, Player.Augs.Slots);
    EndHudMessage(HUDMSG_PLAIN, 0, "Green", 208.1, 76.0, 0.05);

    // Aug Battery Power
    PrintSprite("AUGBATT", 0, 200.0, 110.0, 0.05);
    SetFont("BIGFONT");
    HudMessage("%d%%", (int)Player.Augs.Battery);
    EndHudMessage(HUDMSG_PLAIN, 0, "Yellow", 208.1, 99.0, 0.05);

    // Additional items
    if (AdditionalItemAmount > 0)
    {
        fixed SpriteYOffset;
        str TextColor;

        // Set sprite offsets and colors
        if (AdditionalItem->Actor == "DRPGBatterySmall")
        {
            SpriteYOffset = 40.0;
            TextColor = "Yellow";
        }
        else if (AdditionalItem->Actor == "DRPGBatteryLarge")
        {
            SpriteYOffset = 49.0;
            TextColor = "Yellow";
        }

        PrintSprite(AdditionalItem->Sprite.Name, 0, 300.0, SpriteYOffset, 0.05);
        SetFont("BIGFONT");
        HudMessage("%d \Cg(-%d)", CheckInventory(AdditionalItem->Actor), AdditionalItemAmount);
        EndHudMessage(HUDMSG_PLAIN, 0, TextColor, 308.1, 30.0, 0.05);
    }

    // Draw Aug slots
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 5; j++)
        {
            int Index = j + (i * 5);
            AugInfoPtr AugPtr = &AugData[Index];

            // Catch to make sure unimplemented stuff isn't drawn
            if (Index > AUG_MAX - 1) continue;

            // Draw the augs that we have
            SetFont("SMALLFONT");

            // Draw the E if it's equipped
            if (Player.Augs.Active[Index])
            {
                HudMessage("E");
                EndHudMessage(HUDMSG_PLAIN, 0, "Green", 0.1 + (j * 34.0), 48.0 + (i * 34.0), 0.05);
            }

            // Draw the level and max level
            HudMessage("%d/%d", Player.Augs.CurrentLevel[Index], AugPtr->MaxLevel);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", 32.2 + (j * 34.0), 48.0 + (i * 34.0), 0.05);

            // Icon
            if (Player.MenuIndex == Index)
            {
                if (Player.Augs.Active[Index])
                    PrintSpritePulse(StrParam("Aug%dE", Index + 1), 0, 9.1 + (j * 34.0), 28.1 + (i * 34.0), 0.75, 32.0, 0.25);
                else if (Player.Augs.Level[Index] > 0)
                    PrintSpritePulse(StrParam("Aug%dB", Index + 1), 0, 9.1 + (j * 34.0), 28.1 + (i * 34.0), 0.75, 32.0, 0.25);
                else
                    PrintSpritePulse(StrParam("Aug%d", Index + 1), 0, 9.1 + (j * 34.0), 28.1 + (i * 34.0), 0.75, 32.0, 0.25);
            }
            else
            {
                if (Player.Augs.Active[Index])
                    PrintSprite(StrParam("Aug%dE", Index + 1), 0, 9.1 + (j * 34.0), 28.1 + (i * 34.0), 0.05);
                else if (Player.Augs.Level[Index] > 0)
                    PrintSprite(StrParam("Aug%dB", Index + 1), 0, 9.1 + (j * 34.0), 28.1 + (i * 34.0), 0.05);
                else
                    PrintSprite(StrParam("Aug%d", Index + 1), 0, 9.1 + (j * 34.0), 28.1 + (i * 34.0), 0.05);
            }

            // Currently highlighted Aug's name/description
            SetFont("SMALLFONT");
            if (Player.MenuIndex == Index)
            {
                HudMessage("\Ca%S", AugPtr->Name);
                EndHudMessage(HUDMSG_PLAIN, 0, "Green", 0.1, 128.1, 0.05);

                for (int k = 0; k < AugPtr->MaxLevel; k++)
                {
                    AugInfoPtr AugIterPtr = &AugData[Player.MenuIndex];

                    if (Player.Augs.Level[Index] <= k)
                    {
                        HudMessage("\Cu%S", AugIterPtr->Description[k]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "White", 0.1, 136.1 + (k * 8.0), 0.05);
                    }
                    else if (Player.Augs.CurrentLevel[Index]  <= k)
                    {
                        HudMessage("\Ck%S", AugIterPtr->Description[k]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "Yellow", 0.1, 136.1 + (k * 8.0), 0.05);
                    }
                    else
                    {
                        HudMessage("\Cd%S", AugIterPtr->Description[k]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "Green", 0.1, 136.1 + (k * 8.0), 0.05);
                    }
                }
            }

            // Draw the cursor
            if (Player.MenuIndex == Index)
                PrintSprite("SelectBo", 0, 2.1 + (j * 34.0), 36.0 + (i * 34.0), 0.05);

            // Boxes
            PrintSprite("ItemBox", 0, 0.1 + (j * 34.0), 36.0 + (i * 34.0), 0.05);
        }
}

ItemInfoPtr GetIncreaseSkillAddItem(int SkillCategory, int SkillIndex, int SkillLevel)
{
    // Get additional item for Summoning category
    if (SkillCategory == 4)
    {
        if (SkillIndex > 0) return FindItem("DRPGLootDemonArtifact"); // For Demons
    }
}

int GetIncreaseSkillAddCost(int SkillCategory, int SkillIndex, int SkillLevel)
{
    int Amount;

    // Get ammount additional items for Summoning category
    if (SkillCategory == 4)
    {
        if (SkillIndex == 1) Amount = 1;   // For Former Human
        if (SkillIndex == 2) Amount = 1;   // For Sergeant
        if (SkillIndex == 3) Amount = 2;   // For Commando
        if (SkillIndex == 4) Amount = 2;   // For Imp
        if (SkillIndex == 5) Amount = 2;   // For Demon
        if (SkillIndex == 6) Amount = 3;   // For Cacodemon
        if (SkillIndex == 7) Amount = 3;   // For Hell Knight
        if (SkillIndex == 8) Amount = 5;   // For Baron of Hell
        if (SkillIndex == 9) Amount = 1;   // For Loast Soul
        if (SkillIndex == 10) Amount = 5;  // For Pain Elemental
        if (SkillIndex == 11) Amount = 5;  // For Revenant
        if (SkillIndex == 12) Amount = 7;  // For Mancubus
        if (SkillIndex == 13) Amount = 7;  // For Arachnotron
        if (SkillIndex == 14) Amount = 10; // For Archvile
        if (SkillIndex == 15) Amount = 15; // For Cyberdemon
        if (SkillIndex == 16) Amount = 20; // For Spider Mastermind
    }

    return Amount * (SkillLevel + 1);
}

void DrawSkillMenu()
{
    // Skill Catagories
    auto str SkillCategories[MAX_CATEGORIES] =
    {
        "\CaHealing/Support",
        "\CqPowerups",
        "\CnAuras",
        "\CgAttacks",
        "\CiSummoning",
        "\CcUtility"
    };

    fixed YOffset;
    SkillPtr CurrentSkill = &Skills[Player.SkillPage][Player.MenuIndex];
    SkillLevelInfo *SkillLevel = &Player.SkillLevel[Player.SkillPage][Player.MenuIndex];
    int SkillCost = ScaleEPCost(CurrentSkill->Cost * SkillLevel->CurrentLevel);
    int SkillCostNext = ScaleEPCost(CurrentSkill->Cost * (SkillLevel->CurrentLevel + 1));

    // Set variables for an additional item
    ItemInfoPtr  AdditionalItem = GetIncreaseSkillAddItem(Player.SkillPage, Player.MenuIndex, SkillLevel->Level);
    int AdditionalItemAmount = GetIncreaseSkillAddCost(Player.SkillPage, Player.MenuIndex, SkillLevel->Level);

    // Hold EP cost of skill "Magnetize"
    if (CurrentSkill->Name == "Magnetize")
        SkillCostNext = 25;

    // Title
    SetFont("BIGFONT");
    HudMessage("Skills");
    EndHudMessage(HUDMSG_PLAIN, 0, "Green", 0.1, 10.0, 0.05);

    // Skill Category
    HudMessage("%S\C- \Cd(%d/%d)", SkillCategories[Player.SkillPage], Player.SkillPage + 1, MAX_CATEGORIES);
    EndHudMessage(HUDMSG_PLAIN, 0, "White", 0.1, 25.0, 0.05);

    // Upgrade Modules
    if (SkillLevel->Level < CurrentSkill->MaxLevel)
    {
        YOffset += 32.0;
        PrintSprite("UMODA0", 0, 232.1, 32.1 + YOffset, 0.05);
        SetFont("BIGFONT");
        HudMessage("%d \Cg(-%d)", CheckInventory("DRPGModule"), (int)((((fixed)SkillLevel->Level + 1) * (fixed)MODULE_SKILL_MULT) * GetCVarFixed("drpg_module_skillfactor")));
        EndHudMessage(HUDMSG_PLAIN, 0, "Green", 256.1, 22.0 + YOffset, 0.05);
    }

    // Additional items
    if (SkillLevel->Level < CurrentSkill->MaxLevel && AdditionalItemAmount > 0)
    {
        YOffset += 32.0;
        str TextColor = (Player.SkillPage == 4 && Player.MenuIndex > 0 ? "Brick" : "White");
        PrintSprite(AdditionalItem->Sprite.Name, 0, 232.1, 32.1 + YOffset, 0.05);
        SetFont("BIGFONT");
        HudMessage("%d \Cg(-%d)", CheckInventory(AdditionalItem->Actor), AdditionalItemAmount);
        EndHudMessage(HUDMSG_PLAIN, 0, TextColor, 256.1, 22.0 + YOffset, 0.05);
    }

    // Skill Cost/Next Level Cost
    PrintSprite("EPUPA0", 0, 248.0, 56.1 + YOffset, 0.05);
    SetFont("BIGFONT");
    if (SkillLevel->Level == 0)
    {
        HudMessage("Skill Cost: %d", SkillCostNext);
        EndHudMessage(HUDMSG_PLAIN, 0, "LightBlue", 256.1, 42.1 + YOffset, 0.05);
    }
    else if (SkillLevel->Level < CurrentSkill->MaxLevel)
    {
        HudMessage("Skill Cost: %d\nNext Level: %d", SkillCost, SkillCostNext);
        EndHudMessage(HUDMSG_PLAIN, 0, "LightBlue", 256.1, 42.1 + YOffset, 0.05);
    }
    else
    {
        HudMessage("Skill Cost: %d", SkillCost);
        EndHudMessage(HUDMSG_PLAIN, 0, "LightBlue", 256.1, 42.1 + YOffset, 0.05);
    }

    // Skill Cost Multiplier/Refund
    if (Player.SkillCostMult > 0 || Player.SkillRefundMult > 0)
    {
        PrintSpritePulse("Aura", 0, 232.0, 111.0 + YOffset, 0.75, 32.0, 0.25);
        SetFont("BIGFONT");
        if (Player.SkillRefundMult > 0)
        {
            HudMessage("+%d%% Skill Cost\n%d%% Cost Refund", Player.SkillCostMult, RoundInt(Player.SkillRefundMult * 100));
            EndHudMessage(HUDMSG_PLAIN, 0, "Cyan", 256.1, 90.0 + YOffset, 0.05);
        }
        else
        {
            HudMessage("+%d%% Skill Cost", Player.SkillCostMult);
            EndHudMessage(HUDMSG_PLAIN, 0, "Cyan", 256.1, 82.0 + YOffset, 0.05);
        }
    }

    // Active Aura Levels
    if (Player.Aura.Time > 0 || Player.Aura.Team)
    {
        str const AuraColors[AURA_MAX + 1] =
        {
            "Red",
            "Green",
            "White",
            "Brick",
            "Cyan",
            "Purple",
            "Orange",
            "Blue",
            "Yellow",
            "Black"
        };

        for (int i = 0; i < AURA_MAX; i++)
            if (Player.Aura.Type[i].Active)
            {
                PrintSpritePulse(AuraIcons[i], 0, 232.0 + (i * 24.0), 144.0 + YOffset, 0.75, 64.0, 0.25);
                SetFont("BIGFONT");
                HudMessage("%d", Player.Aura.Type[i].Level);
                EndHudMessage(HUDMSG_PLAIN, 0, AuraColors[i], 232.0 + (i * 24.0), 128.0 + YOffset, 0.05);
            }
    }

    // Skill Grid
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 6; j++)
        {
            int Index = j + (i * 6);
            SkillPtr CurrentSkillIter = &Skills[Player.SkillPage][Index];
            SkillLevelInfo *SkillLevelIter = &Player.SkillLevel[Player.SkillPage][Index];
            str Color = "Red";

            // Break when we reach the last skill
            if (Index >= SkillCategoryMax[Player.SkillPage]) break;

            SetFont("SMALLFONT");

            // This skill is on a skill key
            for (int k = 0; k < MAX_SKILLKEYS; k++)
                if (Player.SkillCategory[k] == Player.SkillPage && Player.SkillIndex[k] == Index)
                {
                    HudMessage("%d", k + 1);
                    EndHudMessage(HUDMSG_PLAIN, 0, "White", 24.1 + (j * 34.0), 40.0 + (i * 34.0), 0.05);
                }

            // Determine Skill Levels Color
            if (SkillLevelIter->Level >= CurrentSkillIter->MaxLevel)
                Color = "Green";
            else if (SkillLevelIter->Level > 0)
                Color = "White";

            // Skill Levels
            HudMessage("%d/%d", SkillLevelIter->CurrentLevel, CurrentSkillIter->MaxLevel);
            EndHudMessage(HUDMSG_PLAIN, 0, Color, 0.1 + (j * 34.0), 64.0 + (i * 34.0), 0.05);

            // Skill Icon
            if (Player.MenuIndex == Index)
                PrintSpritePulse(Skills[Player.SkillPage][Index].Icon, 0, 0.1 + (j * 34.0), 52.0 + (i * 34.0), 0.75, 32.0, 0.25);
            else
                PrintSprite(Skills[Player.SkillPage][Index].Icon, 0, 0.1 + (j * 34.0), 52.0 + (i * 34.0), 0.05);

            // Highlight Box
            if (Player.MenuIndex == Index)
                PrintSprite("SelectBo", 0, 0.1 + (j * 34.0), 52.0 + (i * 34.0), 0.05);

            // Box
            PrintSprite("ItemBox", 0, 0.1 + (j * 34.0), 52.0 + (i * 34.0), 0.05);
        }

    // Skill Name
    SetFont("BIGFONT");
    HudMessage("%S", CurrentSkill->Name);
    EndHudMessage(HUDMSG_PLAIN, 0, (SkillLevel->Level > 0 ? "White" : "Red"), 0.1, 154.1, 0.05);

    // Skill Description
    SetFont("SMALLFONT");
    if (SkillLevel->Level > 0 && SkillLevel->Level < CurrentSkill->MaxLevel)
    {
        HudMessage("%S\n\n\CdNext Level\n%S",
                   CurrentSkill->Description[SkillLevel->CurrentLevel - 1],
                   CurrentSkill->Description[SkillLevel->Level]);
        EndHudMessage(HUDMSG_PLAIN, 0, "Yellow", 0.1, 168.1, 0.05);
    }
    else if (SkillLevel->Level == CurrentSkill->MaxLevel)
    {
        HudMessage("%S", CurrentSkill->Description[SkillLevel->CurrentLevel - 1]);
        EndHudMessage(HUDMSG_PLAIN, 0, "Yellow", 0.1, 168.1, 0.05);
    }
    else
    {
        HudMessage("%S", CurrentSkill->Description[0]);
        EndHudMessage(HUDMSG_PLAIN, 0, "DarkGray", 0.1, 168.1, 0.05);
    }
}

void DrawShieldMenu()
{
    str const PageTitles[4] =
    {
        "\CjBodies",
        "\CfBatteries",
        "\CnCapacitors",
        "\CdAccessories"
    };

    str Description = "";
    int Amount;

    // Title
    SetFont("BIGFONT");
    HudMessage("Shield");
    EndHudMessage(HUDMSG_PLAIN, 0, "Green", 0.1, 10.0, 0.05);

    // Page
    HudMessage("%S", PageTitles[Player.ShieldPage]);
    EndHudMessage(HUDMSG_PLAIN, 0, "Green", 208.1, 10.0, 0.05);

    // Reset New! status on highlighted parts
    if (Player.ShieldPage == SHIELDPAGE_ACCESSORY)
    {
        if (Player.NewShieldAccessoryParts[Player.MenuIndex])
            Player.NewShieldAccessoryParts[Player.MenuIndex] = false;
    }
    else
    {
        if (Player.NewShieldParts[Player.ShieldPage][Player.MenuIndex])
            Player.NewShieldParts[Player.ShieldPage][Player.MenuIndex] = false;
    }

    // Draw Components
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            ShieldPartPtr CurrentPart;
            ShieldAccsPtr CurrentAccessory;
            int MaxItems;
            int Index = j + (i * 10);

            if (Player.ShieldPage == SHIELDPAGE_ACCESSORY)
            {
                CurrentAccessory = &ShieldAccessories[Index];
                MaxItems = MAX_ACCESSORIES;
            }
            else
            {
                CurrentPart = &ShieldParts[Player.ShieldPage][Index];
                MaxItems = ShieldPartsMax[Player.ShieldPage];
            }

            // Break if we're at the end of the list
            if (Index >= MaxItems) break;

            // Draw Available Components
            if (Index < MaxItems)
            {
                SetFont("SMALLFONT");

                // NEW!
                if (Player.ShieldPage == SHIELDPAGE_BODY && Player.NewShieldParts[SHIELDPAGE_BODY][Index] && CheckInventory(CurrentPart->Actor) > 0)
                {
                    HudMessage("NEW!");
                    EndHudMessage(HUDMSG_PLAIN, 0, "White",     1.1 + (j * 34.0), 20.1 + (i * 34.0), 0.05);
                }
                if (Player.ShieldPage == SHIELDPAGE_BATTERY && Player.NewShieldParts[SHIELDPAGE_BATTERY][Index] && CheckInventory(CurrentPart->Actor) > 0)
                {
                    HudMessage("NEW!");
                    EndHudMessage(HUDMSG_PLAIN, 0, "Gold",      1.1 + (j * 34.0), 20.1 + (i * 34.0), 0.05);
                }
                if (Player.ShieldPage == SHIELDPAGE_CAPACITOR && Player.NewShieldParts[SHIELDPAGE_CAPACITOR][Index] && CheckInventory(CurrentPart->Actor) > 0)
                {
                    HudMessage("NEW!");
                    EndHudMessage(HUDMSG_PLAIN, 0, "LightBlue", 1.1 + (j * 34.0), 20.1 + (i * 34.0), 0.05);
                }
                if (Player.ShieldPage == SHIELDPAGE_ACCESSORY && Player.NewShieldAccessoryParts[Index] && CheckInventory(CurrentAccessory->Actor) > 0)
                {
                    HudMessage("NEW!");
                    EndHudMessage(HUDMSG_PLAIN, 0, "Green",     1.1 + (j * 34.0), 20.1 + (i * 34.0), 0.05);
                }

                // Draw Equipped Status
                if (Player.ShieldPage == SHIELDPAGE_BODY && Player.Shield.Body && !StrCmp(Player.Shield.Body->Actor, CurrentPart->Actor))
                {
                    HudMessage("E");
                    EndHudMessage(HUDMSG_PLAIN, 0, "White",        1.1 + (j * 34.0), 48.0 + (i * 34.0), 0.05);
                }
                if (Player.ShieldPage == SHIELDPAGE_BATTERY && Player.Shield.Battery && !StrCmp(Player.Shield.Battery->Actor, CurrentPart->Actor))
                {
                    HudMessage("E");
                    EndHudMessage(HUDMSG_PLAIN, 0, "Gold",         1.1 + (j * 34.0), 48.0 + (i * 34.0), 0.05);
                }
                if (Player.ShieldPage == SHIELDPAGE_CAPACITOR && Player.Shield.Capacitor && !StrCmp(Player.Shield.Capacitor->Actor, CurrentPart->Actor))
                {
                    HudMessage("E");
                    EndHudMessage(HUDMSG_PLAIN, 0, "LightBlue",    1.1 + (j * 34.0), 48.0 + (i * 34.0), 0.05);
                }
                if (Player.ShieldPage == SHIELDPAGE_ACCESSORY && Player.Shield.Accessory && !StrCmp(Player.Shield.Accessory->Actor, CurrentAccessory->Actor))
                {
                    HudMessage("E");
                    EndHudMessage(HUDMSG_PLAIN, 0, "Green",        1.1 + (j * 34.0), 48.0 + (i * 34.0), 0.05);
                }

                // Draw Quantity if you have more than one
                if (Player.ShieldPage < SHIELDPAGE_ACCESSORY && CheckInventory(CurrentPart->Actor) > 1)
                {
                    HudMessage("%d", CheckInventory(CurrentPart->Actor));
                    EndHudMessage(HUDMSG_PLAIN, 0, "White", 33.2 + (j * 34.0), 48.0 + (i * 34.0), 0.05);
                }
                if (Player.ShieldPage == SHIELDPAGE_ACCESSORY && CheckInventory(CurrentAccessory->Actor) > 1)
                {
                    HudMessage("%d", CheckInventory(CurrentAccessory->Actor));
                    EndHudMessage(HUDMSG_PLAIN, 0, "White", 33.2 + (j * 34.0), 48.0 + (i * 34.0), 0.05);
                }
            }

            // Draw Components and set Description
            if (Player.ShieldPage < SHIELDPAGE_ACCESSORY)
            {
                if (Index < MaxItems && CheckInventory(CurrentPart->Actor))
                {
                    // Draw Component
                    if (Player.MenuIndex == Index)
                        PrintSpritePulse(CurrentPart->Icon, 0, 17.1 + (j * 34.0), 52.1 + (i * 34.0), 0.75, 32.0, 0.25);
                    else if (Player.NewShieldParts[Player.ShieldPage][Index])
                        PrintSpritePulse(CurrentPart->Icon, 0, 17.1 + (j * 34.0), 52.1 + (i * 34.0), 0.75, 64.0, 0.25);
                    else
                        PrintSprite(CurrentPart->Icon, 0, 17.1 + (j * 34.0), 52.1 + (i * 34.0), 0.05);

                    // Set Description
                    if (Index == Player.MenuIndex)
                    {
                        // Name
                        Description = StrParam("%S", CurrentPart->Name);

                        str Prepend;

                        // Capacity
                        if (CurrentPart->Capacity < 0)
                            Prepend = "\Cc";
                        else
                            Prepend = "\Cj+";
                        if (CurrentPart->Capacity != 0)
                            Description = StrParam("%S\n%S%d Capacity", Description, Prepend, CurrentPart->Capacity);

                        // Charge Rate
                        if (CurrentPart->ChargeRate < 0)
                            Prepend = "\Cc";
                        else
                            Prepend = "\Cj+";
                        if (CurrentPart->ChargeRate != 0)
                            Description = StrParam("%S\n%S%d Charge Rate/Sec", Description, Prepend, CurrentPart->ChargeRate);

                        // Delay Rate
                        if (CurrentPart->DelayRate > 0)
                            Prepend = "\Cc+";
                        else
                            Prepend = "\Cj";
                        if (CurrentPart->DelayRate != 0)
                            Description = StrParam("%S\n%S%.2k Delay", Description, Prepend, CurrentPart->DelayRate);
                    }
                }
            }
            else if (Player.ShieldPage == SHIELDPAGE_ACCESSORY)
            {
                if (Index < MaxItems && CheckInventory(CurrentAccessory->Actor))
                {
                    // Draw Component
                    if (Player.MenuIndex == Index)
                        PrintSpritePulse(CurrentAccessory->Icon, 0, 17.1 + (j * 34.0), 52.1 + (i * 34.0), 0.75, 32.0, 0.25);
                    else if (Player.NewShieldAccessoryParts[Index])
                        PrintSpritePulse(CurrentAccessory->Icon, 0, 17.1 + (j * 34.0), 52.1 + (i * 34.0), 0.75, 64.0, 0.25);
                    else
                        PrintSprite(CurrentAccessory->Icon, 0, 17.1 + (j * 34.0), 52.1 + (i * 34.0), 0.05);

                    // Set Description
                    if (Index == Player.MenuIndex)
                    {
                        // Name
                        Description = StrParam("%S", CurrentAccessory->Name);

                        // Extra Description
                        if (CurrentAccessory->Description != "")
                            Description = StrParam("%S\n%S", Description, CurrentAccessory->Description);
                    }
                }
            }

            // Draw the cursor
            if (Player.MenuIndex == Index)
                PrintSprite("SelectBo", 0, 2.1 + (j * 34.0), 36.0 + (i * 34.0), 0.05);

            // Boxes
            PrintSprite("ItemBox", 0, 0.1 + (j * 34.0), 36.0 + (i * 34.0), 0.05);
        }
    }

    // Shield Stats/Model
    DrawShieldInfo(-1, 30.1, 270.0, 0);

    // Component Description
    SetFont("SMALLFONT");
    // Header
    HudMessage("- COMPONENT -");
    EndHudMessage(HUDMSG_PLAIN, 0, "Grey", 0.1, 330.1, 0.05);
    // Description
    HudMessage("%S", Description);
    EndHudMessage(HUDMSG_PLAIN, 0, "White", 0.1, 345.1, 0.05);
}

void DrawStimsMenu()
{
    fixed X = 32.1;
    fixed Y = 50.0;
    str StimString;
    str Color;

    // Title
    SetFont("BIGFONT");
    HudMessage("Stims");
    EndHudMessage(HUDMSG_PLAIN, 0, "Green", 0.1, 10.0, 0.05);

    // Stim String
    switch(Player.Stim.Size)
    {
    case 1:
        StimString = StrParam("Small Stim: %d/%d (\Cq+%d%% Toxicity\C-)", Player.Stim.Amount, Player.Stim.Capacity, Player.Stim.Toxicity);
        break;
    case 2:
        StimString = StrParam("Medium Stim: %d/%d (\Cq+%d%% Toxicity\C-)", Player.Stim.Amount, Player.Stim.Capacity, Player.Stim.Toxicity);
        break;
    case 3:
        StimString = StrParam("Large Stim: %d/%d (\Cq+%d%% Toxicity\C-)", Player.Stim.Amount, Player.Stim.Capacity, Player.Stim.Toxicity);
        break;
    case 4:
        StimString = StrParam("Extra Large Stim: %d/%d (\Cq+%d%% Toxicity\C-)", Player.Stim.Amount, Player.Stim.Capacity, Player.Stim.Toxicity);
        break;
    default:
    {
        switch(Player.StimSelected)
        {
        case 0:
            StimString = StrParam("Small Stim: %d", CheckInventory("DRPGStimSmall"));
            break;
        case 1:
            StimString = StrParam("Medium Stim: %d", CheckInventory("DRPGStimMedium"));
            break;
        case 2:
            StimString = StrParam("Large Stim: %d", CheckInventory("DRPGStimLarge"));
            break;
        case 3:
            StimString = StrParam("Extra Large Stim: %d", CheckInventory("DRPGStimXL"));
            break;
        }
    }
    break;
    }

    // Set the Color
    if (Player.MenuIndex == 0)
        Color = MenuCursorColor;
    else if (Player.Stim.Size > 0)
        Color = "Green";
    else
        Color = "White";

    // Draw Current Stim
    HudMessage("%S", StimString);
    EndHudMessage(HUDMSG_PLAIN, 0, Color, 30.1, 24.0, 0.05);

    // Draw Current Stim Bar
    if (Player.Stim.Size > 0)
        for (int i = 0; i < STIM_MAX; i++)
            if (Player.Stim.Current[i] > 0)
            {
                DrawBar(StrParam("Stim%d", i + 1), X, 34, Player.Stim.Current[i], true);
                X += Player.Stim.Current[i];
            }

    for (int i = 1; i <= STIM_MAX; i++)
    {
        // Reset X
        X = 32.1;

        // Set the Color
        if (i == Player.MenuIndex)
            Color = MenuCursorColor;
        else
            Color = CompoundColors[i - 1];

        // Vial Bar
        int Amount = (int)(((fixed)Player.Stim.Vials[i - 1] / (fixed)Player.Stim.VialMax) * 100.0 + 0.5);
        if (Amount > 100)
            Amount = 100;

        //Due to the lags that occurred when displaying sprites in GZDoom 4.5.0
        //I was forced to change the principle of displaying strips reflecting the number of vials collected.
        //Below is a frozen old code with subsequent replacement with a new one. I left it until better times.

        // DrawBar(StrParam("Stim%d", i), X, Y, Amount);
        for (int k = 0; k < (int)((fixed)Amount / 3.0 + 0.7); k++)
        {
            if (GetCVar("drpg_toaster"))
                continue;
            SetFont("SMALLFONT");
            HudMessage("/");
            EndHudMessage(HUDMSG_PLAIN, 0, CompoundColors[i - 1], X + 0.1 + (k * 3.0), Y, 0.05);
        }

        // Vial
        if (!GetCVar("drpg_toaster"))
            X += 112.0;
        SetFont("SMALLFONT");
        HudMessage("%S: %d/%d", CompoundNames[i - 1], Player.Stim.Vials[i - 1], Player.Stim.VialMax);
        EndHudMessage(HUDMSG_PLAIN, 0, Color, X, Y, 0.05);

        if (i == StimStatsEnd || i == StimPowerupStart)
            Y += 16.0;
        else
            Y += 8.0;
    }

    // Toxicity
    DrawToxicityBar(40.1, 240.0, false);
    SetFont("BIGFONT");
    HudMessage("Toxicity: %d%%", Player.Toxicity);
    EndHudMessage(HUDMSG_PLAIN, 0, "Green", 40.1, 240.0, 0.05);

    // Immunity
    HudMessage("Immunity: %d%%", Player.StimImmunity);
    EndHudMessage(HUDMSG_PLAIN, 0, "DarkRed", 256.0, 240.0, 0.05);
}

// This was written while I am sick and messed up on medication
// Good luck figuring it out
// [KS] Challenge accepted
void DrawTurretMenu()
{
    int Count = 0;
    int Parts = CheckInventory("DRPGTurretPart");
    str Color = "Red";
    int PageMax = MAX_UPGRADES / TURRET_PAGE_MAX;
    int Page = Player.MenuIndex / TURRET_PAGE_MAX;
    TurretUpgradePtr Upgrade = &TurretUpgradeData[Player.MenuIndex];

    // Title
    SetFont("BIGFONT");
    if (Player.TurretPage == TURRETPAGE_COMMAND)
    {
        HudMessage("Turret (\CkCommand\C-)");
        EndHudMessage(HUDMSG_PLAIN, 0, "Green", 0.1, 10.0, 0.05);
    }
    if (Player.TurretPage == TURRETPAGE_UPGRADE)
    {
        HudMessage("Turret (\CvUpgrade\C-)");
        EndHudMessage(HUDMSG_PLAIN, 0, "Green", 0.1, 10.0, 0.05);
    }

    // Icon
    if (Player.Turret.Init)
        PrintSprite("PTURA3A7", 0, 32.1, 46.0 + (int)(Sin((fixed)Timer() / 64.0) * 8.0), 0.05);

    // Parts
    SetFont("BIGFONT");
    if (Player.Turret.Upgrade[Player.MenuIndex] < Upgrade->MaxLevel)
    {
        HudMessage("%d \Cg(-%d)", Parts, TurretUpgradeCost(Player.MenuIndex));
        EndHudMessage(HUDMSG_PLAIN, 0, "White", 96.0, 38.0, 0.05);
    }
    else
    {
        HudMessage("%d", Parts);
        EndHudMessage(HUDMSG_PLAIN, 0, "White", 96.0, 38.0, 0.05);
    }
    PrintSprite("TPRTA0", 0, 96.1, 56.0, 0.05);

    // Timers
    DrawTurretTimers(160.0, 38.0);

    // Modules
    while (Count < TURRET_PAGE_MAX)
    {
        int Index = (Page * TURRET_PAGE_MAX) + Count;
        fixed X = 24.0 + ((Count % (TURRET_PAGE_MAX / 4)) * 48.0);
        fixed Y = 80.0 + ((Count / (TURRET_PAGE_MAX / 4)) * 48.0);
        TurretUpgradePtr UpgradeIter = &TurretUpgradeData[Index];

        // Stop if we've hit the page or upgrade max limit
        if (Index >= MAX_UPGRADES) break;

        // Highlight
        if (Index == Player.MenuIndex)
            PrintSpritePulse("TurrBoxH", 0, X, Y, 0.75, 32.0, 0.25);

        // Upgrade Level
        if (Player.TurretPage == TURRETPAGE_UPGRADE)
        {
            if (Player.Turret.Upgrade[Index] >= UpgradeIter->MaxLevel)
                Color = "Green";
            else if (Player.Turret.Upgrade[Index] > 0)
                Color = "White";
            else
                Color = "Red";
            SetFont("SMALLFONT");
            HudMessage("%d/%d", Player.Turret.Upgrade[Index], UpgradeIter->MaxLevel);
            EndHudMessage(HUDMSG_PLAIN, 0, Color, X, Y + 16.0, 0.05);
        }
        else if (Player.Turret.Upgrade[Index] > 0)
            DrawTurretInfo(X, Y, Index);

        // Icon
        if (Index == Player.MenuIndex)
            PrintSpritePulse(StrParam("T_UPG%d", Index + 1), 0, X, Y, 0.75, 32.0, 0.25);
        else
            PrintSprite(StrParam("T_UPG%d", Index + 1), 0, X, Y, 0.05);

        // Box
        PrintSprite("TurrBoxB", 0, X, Y, 0.05);

        Count++;
    }

    // Upgrade Name
    SetFont("BIGFONT");
    HudMessage("%S", Upgrade->Name);
    EndHudMessage(HUDMSG_PLAIN, 0, "White", 0.1, 256.1, 0.05);

    // Upgrade Description
    SetFont("SMALLFONT");
    HudMessage("\Cd%S\n\Cv%S\n\Ck%S", Upgrade->Description, Upgrade->UpgradeInfo, Upgrade->CommandInfo);
    EndHudMessage(HUDMSG_PLAIN, 0, "White", 0.1, 284.1, 0.05);
}

void DrawTurretInfo(fixed X, fixed Y, int Index)
{
    str Info = "";

    switch (Index)
    {
    case TU_BUILD:
        if (Player.Turret.Active)
            Info = "\CdON";
        else
            Info = "\CaOFF";
        break;
    case TU_WEAPON_BULLET:
        if (Player.Turret.Weapon == TW_BULLET)
            Info = "\CaON";
        break;
    case TU_WEAPON_BULLET_CAPACITY:
        Y -= 4.0;
        Info = StrParam("\Ca%d\n\Ca%d", Player.Turret.BulletAmmo, Player.Turret.BulletAmmoMax);
        break;
    case TU_WEAPON_PELLET:
        if (Player.Turret.Weapon == TW_PELLET)
            Info = "\CiON";
        break;
    case TU_WEAPON_PELLET_CAPACITY:
        Y -= 4.0;
        Info = StrParam("\Ci%d\n\Ci%d", Player.Turret.ShellAmmo, Player.Turret.ShellAmmoMax);
        break;
    case TU_WEAPON_ROCKET:
        if (Player.Turret.Weapon == TW_ROCKET)
            Info = "\CcON";
        break;
    case TU_WEAPON_ROCKET_CAPACITY:
        Y -= 4.0;
        Info = StrParam("\Cc%d\n\Cc%d", Player.Turret.RocketAmmo, Player.Turret.RocketAmmoMax);
        break;
    case TU_WEAPON_PLASMA:
        if (Player.Turret.Weapon == TW_PLASMA)
            Info = "\CnON";
        break;
    case TU_WEAPON_PLASMA_CAPACITY:
        Y -= 4.0;
        Info = StrParam("\Cn%d\n\Cn%d", Player.Turret.PlasmaAmmo, Player.Turret.PlasmaAmmoMax);
        break;
    case TU_WEAPON_RAILGUN:
        if (Player.Turret.Weapon == TW_RAILGUN)
            Info = "\CdON";
        break;
    case TU_WEAPON_RAILGUN_CAPACITY:
        Y -= 4.0;
        Info = StrParam("\Cd%d\n\Cd%d", Player.Turret.RailAmmo, Player.Turret.RailAmmoMax);
        break;
    case TU_AMMO_AUTOLOADER:
        if (Player.Turret.Autoload)
            Info = "\CdON";
        else
            Info = "\CaOFF";
        break;
    case TU_BATTERY_AUGBATTERY:
        if (Player.Turret.AugBattery)
            Info = "\CdON";
        else
            Info = "\CaOFF";
        break;
    case TU_AMMO_NANOGEN:
        Y -= 4.0;
        Info = StrParam("\Cj%d\n\CjSec", 15 - Player.Turret.Upgrade[TU_AMMO_NANOGEN]);
        break;
    case TU_ARMOR_PLATING:
        Info = StrParam("\Ca%d", Player.Turret.HealthMax);
        break;
    case TU_ARMOR_PLATING_MELEE:
        Info = StrParam("\Cc%.1k%%", Player.Turret.Upgrade[TU_ARMOR_PLATING_MELEE] * 5.0);
        if (Player.Turret.Upgrade[TU_ARMOR_PLATING_MELEE] > 10)
            Info = StrParam("\Cc%.1k%%", 50.0 + (Player.Turret.Upgrade[TU_ARMOR_PLATING_MELEE] - 10) * 2.5);
        break;
    case TU_ARMOR_PLATING_BULLET:
        Info = StrParam("\Cj%.1k%%", Player.Turret.Upgrade[TU_ARMOR_PLATING_BULLET] * 5.0);
        if (Player.Turret.Upgrade[TU_ARMOR_PLATING_BULLET] > 10)
            Info = StrParam("\Cj%.1k%%", 50.0 + (Player.Turret.Upgrade[TU_ARMOR_PLATING_BULLET] - 10) * 2.5);
        break;
    case TU_ARMOR_PLATING_FIRE:
        Info = StrParam("\Cg%.1k%%", Player.Turret.Upgrade[TU_ARMOR_PLATING_FIRE] * 5.0);
        if (Player.Turret.Upgrade[TU_ARMOR_PLATING_FIRE] > 10)
            Info = StrParam("\Cg%.1k%%", 50.0 + (Player.Turret.Upgrade[TU_ARMOR_PLATING_FIRE] - 10) * 2.5);
        break;
    case TU_ARMOR_PLATING_PLASMA:
        Info = StrParam("\Cn%.1k%%", Player.Turret.Upgrade[TU_ARMOR_PLATING_PLASMA] * 5.0);
        if (Player.Turret.Upgrade[TU_ARMOR_PLATING_PLASMA] > 10)
            Info = StrParam("\Cn%.1k%%", 50.0 + (Player.Turret.Upgrade[TU_ARMOR_PLATING_PLASMA] - 10) * 2.5);
        break;
    case TU_ARMOR_MODULE_PHASE:
        Y -= 4.0;
        Info = StrParam("\Cn%d\n\CnSec", Player.Turret.Upgrade[TU_ARMOR_MODULE_PHASE]);
        break;
    case TU_ARMOR_MODULE_REPAIR:
        Y -= 4.0;
        Info = StrParam("\Ca%.2k\n\CaSec", 30 - (Player.Turret.Upgrade[TU_ARMOR_MODULE_REPAIR] * 2.5));
        break;
    case TU_BATTERY_CAPACITY:
        Info = StrParam("\Ck%S", FormatTime(Player.Turret.BatteryMax * 35));
        break;
    case TU_BATTERY_GENERATOR_KINETIC:
        Y -= 4.0;
        Info = StrParam("\Ck%d Sec\n\CkDelay", 11 - Player.Turret.Upgrade[TU_BATTERY_GENERATOR_KINETIC]);
        break;
    case TU_BATTERY_GENERATOR_ILLUMINATION:
        Y -= 4.0;
        Info = StrParam("\Ck%d Sec\n\CkDelay", 11 - Player.Turret.Upgrade[TU_BATTERY_GENERATOR_ILLUMINATION]);
        break;
    case TU_BATTERY_GENERATOR_FORCE:
        Y -= 4.0;
        Info = StrParam("\Cj+%d\n\CjDMG", Player.Turret.Upgrade[TU_BATTERY_GENERATOR_FORCE]);
        break;
    case TU_BATTERY_GENERATOR_THERMAL:
        Y -= 4.0;
        Info = StrParam("\Cg+%d\n\CgDMG", Player.Turret.Upgrade[TU_BATTERY_GENERATOR_THERMAL]);
        break;
    case TU_BATTERY_GENERATOR_PLASMA:
        Y -= 4.0;
        Info = StrParam("\Cn+%d\n\CnDMG", Player.Turret.Upgrade[TU_BATTERY_GENERATOR_PLASMA]);
        break;
    case TU_BATTERY_GENERATOR_NUCLEAR:
        Y -= 4.0;
        Info = StrParam("\Cd+%d\n\CdDMG", Player.Turret.Upgrade[TU_BATTERY_GENERATOR_NUCLEAR]);
        break;
    case TU_COMMAND_DRAW_FIRE:
        if (Player.Turret.Command == TC_DRAW_FIRE)
            Info = "\CdON";
        break;
    case TU_COMMAND_HOLD_POSITION:
        if (Player.Turret.Command == TC_HOLD_POSITION)
            Info = "\CdON";
        break;
    }

    SetFont("SMALLFONT");
    HudMessage("%S", Info);
    EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.4, Y + 16.0, 0.05);
}

void DrawTurretTimers(fixed X, fixed Y)
{
    // Timers
    if (Player.Turret.Init)
    {
        // Charge Timer
        SetFont("BIGFONT");
        HudMessage("%S", FormatTime(Player.Turret.ChargeTimer * 35));
        EndHudMessage(HUDMSG_PLAIN, 0, "Yellow", X + 0.1, Y, 0.05);
        if (Player.Turret.Maintenance && Player.Turret.ChargeTimer > 0)
            PrintSpritePulse("TCharge", 0, X + 0.1, Y - 3.0, 0.5, 32.0, 0.5);
        else
            PrintSprite("TCharge", 0, X + 0.1, Y - 3.0, 0.05);

        // Repair Timer
        SetFont("BIGFONT");
        HudMessage("%S", FormatTime(Player.Turret.RepairTimer * 35));
        EndHudMessage(HUDMSG_PLAIN, 0, (Player.Turret.PaidForRepair ? "Brick" : "Red"), X + 64.1, Y, 0.05);
        if (Player.Turret.Maintenance && Player.Turret.RepairTimer > 0)
            PrintSpritePulse("TRepair", 0, X + 64.1, Y - 3.0, 0.5, 32.0, 0.5);
        else
            PrintSprite("TRepair", 0, X + 64.1, Y - 3.0, 0.05);

        // Refit Timer
        SetFont("BIGFONT");
        HudMessage("%S", FormatTime(Player.Turret.RefitTimer * 35));
        EndHudMessage(HUDMSG_PLAIN, 0, "LightBlue", X + 128.1, Y, 0.05);
        if (Player.Turret.Maintenance && Player.Turret.RefitTimer > 0)
            PrintSpritePulse("TRefit", 0, X + 128.1, Y - 3.0, 0.5, 32.0, 0.5);
        else
            PrintSprite("TRefit", 0, X + 128.1, Y - 3.0, 0.05);
    }
}

void MenuInput()
{
    // Reset the menu block
    Player.MenuBlock = false;

    // Main Menu
    if (Player.Menu == MENUPAGE_MAIN)
    {
        if (CheckInput(BT_FORWARD, KEY_PRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            Player.MenuIndex--;
            if (Player.MenuIndex < 0) Player.MenuIndex = MAX_MENU - 1;
        }
        if (CheckInput(BT_BACK, KEY_PRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            Player.MenuIndex++;
            if (Player.MenuIndex > MAX_MENU - 1) Player.MenuIndex = 0;
        }
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
        {
            Player.MenuBlock = true;

            if (Player.MenuIndex == MAX_MENU - 1)
                OpenShop(false);
            else
            {
                ActivatorSound("menu/move", 127);
                Player.Menu = Player.MenuIndex + 1;
                Player.MenuIndex = 0;
                ClearToxicityMeter();
            }
        }
    }

    // Stats menu
    if (Player.Menu == MENUPAGE_STATS && !Player.MenuBlock)
    {
        if (CheckInput(BT_FORWARD, KEY_REPEAT, false, PlayerNumber()))
        {
            if (Player.StatPage == STATPAGE_STATS)
            {
                ActivatorSound("menu/move", 127);
                Player.MenuIndex -= 2;
                if (Player.MenuIndex < 0) Player.MenuIndex = STAT_MAX - 1;
            }
            else if (Player.StatPage == STATPAGE_TEAM)
            {
                ActivatorSound("menu/move", 127);
                Player.MenuIndex--;
                ClearToxicityMeter();
                if (Player.MenuIndex < 0) Player.MenuIndex = PlayerCount() - 1;
            }
        }
        if (CheckInput(BT_BACK, KEY_REPEAT, false, PlayerNumber()))
        {
            if (Player.StatPage == STATPAGE_STATS)
            {
                ActivatorSound("menu/move", 127);
                Player.MenuIndex += 2;
                if (Player.MenuIndex > STAT_MAX - 1) Player.MenuIndex = 0;
            }
            else if (Player.StatPage == STATPAGE_TEAM)
            {
                ActivatorSound("menu/move", 127);
                Player.MenuIndex++;
                ClearToxicityMeter();
                if (Player.MenuIndex > PlayerCount() - 1) Player.MenuIndex = 0;
            }
        }
        if (CheckInput(BT_MOVELEFT, KEY_REPEAT, false, PlayerNumber()))
        {
            if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
            {
                if (Player.StatPage > 0)
                {
                    ActivatorSound("menu/move", 127);
                    Player.StatPage--;
                    Player.MenuIndex = 0;
                    if (Player.StatPage == STATPAGE_STATXP && !GetCVar("drpg_levelup_natural"))
                        Player.StatPage--;
                }
            }
            else if (Player.StatPage == STATPAGE_STATS)
            {
                ActivatorSound("menu/move", 127);
                Player.MenuIndex--;
                if (Player.MenuIndex < 0) Player.MenuIndex = STAT_MAX - 1;
            }
        }
        if (CheckInput(BT_MOVERIGHT, KEY_REPEAT, false, PlayerNumber()))
        {
            if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
            {
                if (Player.StatPage < STATPAGE_MAX - (InMultiplayer ? 1 : 2))
                {
                    ActivatorSound("menu/move", 127);
                    Player.StatPage++;
                    Player.MenuIndex = 0;
                    if (Player.StatPage == STATPAGE_STATXP && !GetCVar("drpg_levelup_natural"))
                        Player.StatPage++;
                }
            }
            else if (Player.StatPage == STATPAGE_STATS)
            {
                ActivatorSound("menu/move", 127);
                Player.MenuIndex++;
                if (Player.MenuIndex > STAT_MAX - 1) Player.MenuIndex = 0;
            }
        }
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
            if (Player.StatPage == STATPAGE_STATS && !GetActivatorCVar("drpg_auto_spend"))
                IncreaseStat(Player.MenuIndex);
            else if (Player.StatPage == STATPAGE_TEAM && Player.MenuIndex != PlayerNumber())
            {
                PlayerTeleport(Player.MenuIndex);
                Player.EP -= ScaleEPCost(50);
                Player.Menu = MENUPAGE_MAIN;
                Player.MenuIndex = 0;
                Player.InMenu = false;
                SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
            }
        if (CheckInput(BT_ATTACK, KEY_ONLYPRESSED, false, PlayerNumber()) && Player.StatPage == STATPAGE_TEAM)
        {
            Player.PlayerView = Player.MenuIndex;
            ActivatorSound("menu/move", 127);
        }
    }

    // Augmentations menu
    if (Player.Menu == MENUPAGE_AUGS && !Player.MenuBlock)
    {
        if (CheckInput(BT_FORWARD, KEY_REPEAT, false, PlayerNumber()))
        {
            if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
            {
                if (Player.Augs.CurrentLevel[Player.MenuIndex] > 1)
                {
                    Player.Augs.CurrentLevel[Player.MenuIndex]--;
                    ActivatorSound("menu/move", 127);
                }
            }
            else if (Player.MenuIndex > 0)
            {
                ActivatorSound("menu/move", 127);
                Player.MenuIndex -= 5;
                if (Player.MenuIndex < 0) Player.MenuIndex = 0;
            }
        }
        if (CheckInput(BT_BACK, KEY_REPEAT, false, PlayerNumber()))
        {
            if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
            {
                if (Player.Augs.CurrentLevel[Player.MenuIndex] < Player.Augs.Level[Player.MenuIndex])
                {
                    Player.Augs.CurrentLevel[Player.MenuIndex]++;
                    ActivatorSound("menu/move", 127);
                }
            }
            else if (Player.MenuIndex < AUG_MAX - 1)
            {
                ActivatorSound("menu/move", 127);
                Player.MenuIndex += 5;
                if (Player.MenuIndex > AUG_MAX - 1) Player.MenuIndex = AUG_MAX - 1;
            }
        }
        if (CheckInput(BT_MOVELEFT, KEY_REPEAT, false, PlayerNumber()) && Player.MenuIndex > 0)
        {
            ActivatorSound("menu/move", 127);
            Player.MenuIndex--;
        }
        if (CheckInput(BT_MOVERIGHT, KEY_REPEAT, false, PlayerNumber()) && Player.MenuIndex < AUG_MAX - 1)
        {
            ActivatorSound("menu/move", 127);
            Player.MenuIndex++;
        }
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
        {
            if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
                IncreaseAug(Player.MenuIndex);
            else
            {
                EquipAug(Player.MenuIndex);

                // If the player starts manually toggling augs, don't try to automatically activate any later.
                ClearDisabledAugs();
            }
        }
    }

    // Skills Menu
    if (Player.Menu == MENUPAGE_SKILLS && !Player.MenuBlock && !Player.SkillWheelOpen)
    {
        if (CheckInput(BT_FORWARD, KEY_REPEAT, false, PlayerNumber()))
        {
            if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
            {
                if (Player.SkillLevel[Player.SkillPage][Player.MenuIndex].CurrentLevel > 1)
                {
                    Player.SkillLevel[Player.SkillPage][Player.MenuIndex].CurrentLevel--;
                    ActivatorSound("menu/move", 127);
                }
            }
            else if (Player.MenuIndex > 0)
            {
                ActivatorSound("menu/move", 127);
                Player.MenuIndex -= 6;
                if (Player.MenuIndex < 0) Player.MenuIndex = 0;
            }
        }
        if (CheckInput(BT_BACK, KEY_REPEAT, false, PlayerNumber()))
        {
            if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
            {
                if (Player.SkillLevel[Player.SkillPage][Player.MenuIndex].CurrentLevel < Player.SkillLevel[Player.SkillPage][Player.MenuIndex].Level)
                {
                    Player.SkillLevel[Player.SkillPage][Player.MenuIndex].CurrentLevel++;
                    ActivatorSound("menu/move", 127);
                }
            }
            else if (Player.MenuIndex < SkillCategoryMax[Player.SkillPage] - 1)
            {
                ActivatorSound("menu/move", 127);
                Player.MenuIndex += 6;
                if (Player.MenuIndex > SkillCategoryMax[Player.SkillPage] - 1) Player.MenuIndex = SkillCategoryMax[Player.SkillPage] - 1;
            }
        }
        if (CheckInput(BT_MOVELEFT, KEY_REPEAT, false, PlayerNumber()))
        {
            if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
            {
                ActivatorSound("menu/move", 127);
                Player.MenuIndex = 0;
                Player.SkillPage--;
                if (Player.SkillPage < 0) Player.SkillPage = MAX_CATEGORIES - 1;
            }
            else if (Player.MenuIndex > 0)
            {
                ActivatorSound("menu/move", 127);
                Player.MenuIndex--;
            }
        }
        if (CheckInput(BT_MOVERIGHT, KEY_REPEAT, false, PlayerNumber()))
        {
            if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
            {
                ActivatorSound("menu/move", 127);
                Player.MenuIndex = 0;
                Player.SkillPage++;
                if (Player.SkillPage >= MAX_CATEGORIES) Player.SkillPage = 0;
            }
            else if (Player.MenuIndex < SkillCategoryMax[Player.SkillPage] - 1)
            {
                ActivatorSound("menu/move", 127);
                Player.MenuIndex++;
            }
        }
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
            IncreaseSkill(Player.SkillPage, Player.MenuIndex, false);
        if (CheckInput(BT_ATTACK, KEY_ONLYPRESSED, false, PlayerNumber()))
            UseSkill(0);
    }

    // Shield Menu
    if (Player.Menu == MENUPAGE_SHIELD && !Player.MenuBlock)
    {
        int PartsMax;
        ShieldPartPtr CurrentPart;
        ShieldAccsPtr CurrentAccessory;

        if (Player.ShieldPage == 3)
        {
            CurrentAccessory = &ShieldAccessories[Player.MenuIndex];
            PartsMax = MAX_ACCESSORIES;
        }
        else
        {
            CurrentPart = &ShieldParts[Player.ShieldPage][Player.MenuIndex];
            PartsMax = ShieldPartsMax[Player.ShieldPage];
        }

        if (CheckInput(BT_FORWARD, KEY_REPEAT, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            Player.MenuIndex -= 10;
            if (Player.MenuIndex < 0) Player.MenuIndex = 0;
        }
        if (CheckInput(BT_BACK, KEY_REPEAT, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            Player.MenuIndex += 10;
            if (Player.MenuIndex > PartsMax - 1) Player.MenuIndex = PartsMax - 1;
        }
        if (CheckInput(BT_MOVELEFT, KEY_REPEAT, false, PlayerNumber()))
        {
            if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
            {
                if (Player.ShieldPage > 0)
                {
                    ActivatorSound("menu/move", 127);
                    Player.ShieldPage--;
                    Player.MenuIndex = 0;
                }
            }
            else
            {
                ActivatorSound("menu/move", 127);
                Player.MenuIndex--;
                if (Player.MenuIndex < 0) Player.MenuIndex = PartsMax - 1;
            }
        }
        if (CheckInput(BT_MOVERIGHT, KEY_REPEAT, false, PlayerNumber()))
        {
            if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
            {
                if (Player.ShieldPage < 3)
                {
                    ActivatorSound("menu/move", 127);
                    Player.ShieldPage++;
                    Player.MenuIndex = 0;
                }
            }
            else
            {
                ActivatorSound("menu/move", 127);
                Player.MenuIndex++;
                if (Player.MenuIndex > PartsMax - 1) Player.MenuIndex = 0;
            }
        }
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
        {
            if (Player.MenuIndex < PartsMax)
            {
                for (int i = 0; i < SHIELDPAGE_ACCESSORY; i++)
                    if (i == Player.ShieldPage)
                        if (CheckInventory(CurrentPart->Actor))
                        {
                            if (i == SHIELDPAGE_BODY) Player.Shield.Body = CurrentPart;
                            if (i == SHIELDPAGE_BATTERY) Player.Shield.Battery = CurrentPart;
                            if (i == SHIELDPAGE_CAPACITOR) Player.Shield.Capacitor = CurrentPart;

                            ActivatorSound("shield/equip", 127);
                        }
                if (Player.ShieldPage == SHIELDPAGE_ACCESSORY)
                    if (CheckInventory(CurrentAccessory->Actor))
                    {
                        SetShieldAccessory(CurrentAccessory);

                        ActivatorSound("shield/equip", 127);
                    }
            }
        }
        if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
        {
            if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
            {
                if (Player.MenuIndex < PartsMax)
                    for (int i = 0; i < SHIELDPAGE_MAX; i++)
                        if (i == Player.ShieldPage)
                        {
                            if (i == SHIELDPAGE_BODY) Player.Shield.Body = NULL;
                            if (i == SHIELDPAGE_BATTERY) Player.Shield.Battery = NULL;
                            if (i == SHIELDPAGE_CAPACITOR) Player.Shield.Capacitor = NULL;
                            if (i == SHIELDPAGE_ACCESSORY) RemoveShieldAccessory();

                            ActivatorSound("shield/unequip", 127);
                        }
            }
        }
    }

    // Stims Menu
    if (Player.Menu == MENUPAGE_STIMS && !Player.MenuBlock)
    {
        if (CheckInput(BT_FORWARD, KEY_REPEAT, false, PlayerNumber()))
        {
            Player.MenuIndex--;
            ActivatorSound("menu/move", 127);
            if (Player.MenuIndex < 0) Player.MenuIndex = STIM_MAX;
        }
        if (CheckInput(BT_BACK, KEY_REPEAT, false, PlayerNumber()))
        {
            Player.MenuIndex++;
            ActivatorSound("menu/move", 127);
            if (Player.MenuIndex > STIM_MAX) Player.MenuIndex = 0;
        }
        if (CheckInput(BT_MOVELEFT, KEY_ONLYPRESSED, false, PlayerNumber()) && Player.StimSelected > 0)
        {
            if (Player.Stim.Size > 0) return;

            Player.StimSelected--;
            ActivatorSound("menu/move", 127);
        }
        if (CheckInput(BT_MOVERIGHT, KEY_ONLYPRESSED, false, PlayerNumber()) && Player.StimSelected < 4 - 1)
        {
            if (Player.Stim.Size > 0) return;

            Player.StimSelected++;
            ActivatorSound("menu/move", 127);
        }
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
        {
            if (Player.MenuIndex == 0)
                SetStim(Player.StimSelected);
            else
                MixStim(Player.MenuIndex - 1);
        }
    }

    // Turret Menu
    if (Player.Menu == MENUPAGE_TURRET && !Player.MenuBlock)
    {
        if (CheckInput(BT_FORWARD, KEY_REPEAT, false, PlayerNumber()) && Player.MenuIndex > 0)
        {
            Player.MenuIndex -= TURRET_PAGE_MAX / 4;
            if (Player.MenuIndex < 0) Player.MenuIndex = 0;
            ActivatorSound("menu/move", 127);
        }
        if (CheckInput(BT_BACK, KEY_REPEAT, false, PlayerNumber()) && Player.MenuIndex < MAX_UPGRADES - 1)
        {
            Player.MenuIndex += TURRET_PAGE_MAX / 4;
            if (Player.MenuIndex >= MAX_UPGRADES - 1) Player.MenuIndex = MAX_UPGRADES - 1;
            ActivatorSound("menu/move", 127);
        }
        if (CheckInput(BT_MOVELEFT, KEY_REPEAT, false, PlayerNumber()) && Player.MenuIndex > 0)
        {
            Player.MenuIndex--;
            ActivatorSound("menu/move", 127);
        }
        if (CheckInput(BT_MOVERIGHT, KEY_REPEAT, false, PlayerNumber()) && Player.MenuIndex < MAX_UPGRADES - 1)
        {
            Player.MenuIndex++;
            ActivatorSound("menu/move", 127);
        }
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
        {
            if (Player.TurretPage == TURRETPAGE_COMMAND)
                TurretCommand(Player.MenuIndex);
            else if (Player.TurretPage == TURRETPAGE_UPGRADE)
                UpgradeTurret(Player.MenuIndex);
        }
        if (CheckInput(BT_SPEED, KEY_ONLYPRESSED, false, PlayerNumber()))
            TurretMaintenance();
        if (CheckInput(BT_JUMP, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            Player.TurretPage++;
            if (Player.TurretPage >= TURRETPAGE_MAX)
                Player.TurretPage = TURRETPAGE_COMMAND;
            ActivatorSound("menu/move", 127);
        }
    }
}

void IncreaseStat(int Stat)
{
    int *Stats[STAT_MAX] =
    {
        &Player.Strength,
        &Player.Defense,
        &Player.Vitality,
        &Player.Energy,
        &Player.Regeneration,
        &Player.Agility,
        &Player.Capacity,
        &Player.Luck
    };

    // Determine the cost of the stat upgrade
    int Cost = (int)((((fixed)*Stats[Stat] + 1) * (fixed)MODULE_STAT_MULT) * GetCVarFixed("drpg_module_statfactor"));
    if (Cost < 0)
        Cost = -Cost;
    else if (Cost == 0)
        Cost = (int)((1 * (fixed)MODULE_STAT_MULT) * GetCVarFixed("drpg_module_statfactor"));

    // Check the minimum number of modules if Stat Randomizer is enabled
    if (GetActivatorCVar("drpg_auto_spend") && (CheckInventory("DRPGModule") - Cost < ((250 + Player.Level * 25) / 250 * 250))) return;

    // Make sure you have enough Modules
    if (CheckInventory("DRPGModule") < Cost && (Player.InMenu || Player.GUI.Open))
    {
        if (GetActivatorCVar("drpg_auto_spend")) return;
        PrintError("You don't have enough Modules to upgrade this stat");
        ActivatorSound("menu/error", 127);
        return;
    }

    // Check Stat Caps
    CheckStatCaps();

    // Upgrade the Stat
    if (*Stats[Stat] < Player.StatCap)
        (*Stats[Stat])++;
    else
    {
        if (!GetActivatorCVar("drpg_auto_spend")) // From the menu
            PrintStatError();
        return;
    }

    if (Player.InMenu || Player.GUI.Open) // Spent the point in the menu, make a sound
        ActivatorSound("menu/move", 127);

    TakeInventory("DRPGModule", Cost);
}

void IncreaseAug(int AugIndex)
{
    if (Player.Augs.Level[AugIndex] < AugData[AugIndex].MaxLevel)
    {
        bool CanLevel = true;

        // Get information about additional items
        ItemInfoPtr  AdditionalItem = GetIncreaseAugAddItem(AugIndex, Player.Augs.Level[AugIndex]);
        int AdditionalItemAmount = GetIncreaseAugAddCost(AugIndex, Player.Augs.Level[AugIndex]);

        if (AdditionalItemAmount > 0 && CheckInventory(AdditionalItem->Actor) < AdditionalItemAmount)
        {
            CanLevel = false;
        }
        else if (Player.Augs.Level[AugIndex] == 0)
        {
            if (CheckInventory("DRPGAugCanister"))
            {
                ActivatorSound("aug/levelup", 127);
                TakeInventory("DRPGAugCanister", 1);
                TakeInventory(AdditionalItem->Actor, AdditionalItemAmount);
            }
            else
                CanLevel = false;
        }
        else
        {
            if (CheckInventory("DRPGAugCanister") && CheckInventory("DRPGAugUpgradeCanister") >= Player.Augs.Level[AugIndex] + 1)
            {
                ActivatorSound("aug/levelup", 127);
                TakeInventory("DRPGAugCanister", 1);
                TakeInventory("DRPGAugUpgradeCanister", Player.Augs.Level[AugIndex] + 1);
                TakeInventory(AdditionalItem->Actor, AdditionalItemAmount);
            }
            else
                CanLevel = false;
        }

        if (!CanLevel)
        {
            PrintError("You cannot upgrade\nthis augmentation");
            ActivatorSound("menu/error", 127);
            return;
        }

        Player.Augs.Level[AugIndex]++;
        Player.Augs.CurrentLevel[AugIndex]++;
    }
}

void IncreaseSkill(int Category, int Index, bool UsingOverdrive)
{
    SkillPtr CurrentSkill = &Skills[Category][Index];
    SkillLevelInfo *SkillLevel = &Player.SkillLevel[Category][Index];
    int Cost = (int)((((fixed)SkillLevel->Level + 1) * (fixed)MODULE_SKILL_MULT) * GetCVarFixed("drpg_module_skillfactor"));

    // Get information about additional items
    ItemInfoPtr  AdditionalItem = GetIncreaseSkillAddItem(Category, Index, SkillLevel->Level);
    int AdditionalItemAmount = GetIncreaseSkillAddCost(Category, Index, SkillLevel->Level);

    if (SkillLevel->Level < CurrentSkill->MaxLevel)
    {
        if (CheckInventory("DRPGModule") < Cost)
        {
            PrintError("You don't have enough Modules");
            ActivatorSound("menu/error", 127);
        }
        else if (AdditionalItemAmount > 0 && CheckInventory(AdditionalItem->Actor) < AdditionalItemAmount)
        {
            SetHudSize(0, 0, false);
            SetFont("BIGFONT");
            HudMessage("You don't have enough %S", AdditionalItem->Name);
            EndHudMessage(HUDMSG_FADEOUT, 0, "Red", 0.5, 0.5, 2.0, 1.0);
            ActivatorSound("menu/error", 127);
        }
        else
        {
            SkillLevel->Level++;
            SkillLevel->CurrentLevel++;
            TakeInventory("DRPGModule", Cost);
            TakeInventory(AdditionalItem->Actor, AdditionalItemAmount);
            if (!UsingOverdrive)
                ActivatorSound("menu/move", 127);
            else
            {
                FadeRange(0, 255, 255, 0.5, 0, 255, 255, 0.0, 0.5);
                ActivatorSound("health/epcapsule", 127);
            }
        }
    }
}

void UpgradeTurret(int Index)
{
    if (Index > 0 && !Player.Turret.Upgrade[TU_BUILD])
    {
        PrintError("You haven't built the turret yet");
        ActivatorSound("menu/error", 127);
        return;
    }

    if (Player.Turret.Upgrade[Index] < TurretUpgradeData[Index].MaxLevel)
    {
        if (CheckInventory("DRPGTurretPart") >= TurretUpgradeCost(Index))
        {
            TakeInventory("DRPGTurretPart", TurretUpgradeCost(Index));
            ActivatorSound("turret/upgrade", 127);

            Player.Turret.Upgrade[Index]++;

            // Refit
            if (Index > 0)
            {
                TurretDespawn();
                Player.Turret.RefitTimer += Player.Turret.Upgrade[Index] * (10 - Player.Turret.Upgrade[TU_HARDWARE_SPECS]);
            }
            else // Build
            {
                Player.Turret.Health = 100;
                Player.Turret.RefitTimer += 60;

                // Automatically start maintenance to begin building the turret
                if (CheckInventory("DRPGCredits") > 0)
                    Player.Turret.Maintenance = true;
            }
        }
        else
        {
            PrintError("You don't have enough Turret Parts to perform this upgrade");
            ActivatorSound("menu/error", 127);
        }
    }
}

void PrintStatError()
{
    SetHudSize(0, 0, false);
    SetFont("BIGFONT");
    HudMessage("You cannot increase stats past %d", Player.StatCap);
    EndHudMessage(HUDMSG_FADEOUT, 0, "Red", 0.5, 0.5, 2.0, 1.0);
    ActivatorSound("menu/error", 127);
}

void MenuHelp()
{
    fixed X = 0.1;
    fixed Y = 380.1;

    // Return if the help CVAR is off
    if (!GetActivatorCVar("drpg_menuhelp")) return;

    // Set the HUD Size and Font
    SetHudSize(GetActivatorCVar("drpg_menu_width"), GetActivatorCVar("drpg_menu_height"), true);
    SetFont("SMALLFONT");

    // Main Menu Help
    if (Player.InMenu && !Player.InShop)
        switch (Player.Menu)
        {
        case MENUPAGE_MAIN:
            // Mission Help
            if (Player.Mission.Active)
            {
                if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                    HudMessage("Navigate: \Cd%S/%S\C-\nSelect: \Cd%S\C-\nMission Info: \CiHOLD \Cd%S\C-",
                               "Up", "Down", "Use", "Run");
                else
                    HudMessage("Navigate: \Cd%jS/%jS\C-\nSelect: \Cd%jS\C-\nMission Info: \CiHOLD \Cd%jS\C-",
                               "+forward", "+back", "+use", "+speed");
                EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            }
            else
            {
                if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                    HudMessage("Navigate: \Cd%S/%S\C-\nSelect: \Cd%S\C-\nNo mission currently active.",
                               "Up", "Down", "Use");
                else
                    HudMessage("Navigate: \Cd%jS/%jS\C-\nSelect: \Cd%jS\C-\nNo mission currently active.",
                               "+forward", "+back", "+use");
                EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            }
            break;
        case MENUPAGE_STATS:
            if (Player.StatPage == STATPAGE_STATS)
            {
                if (GetActivatorCVar("drpg_auto_spend"))
                {
                    if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                        HudMessage("Navigate: \Cd%S/%S/%S/%S\C-\nSwitch pages: \Cd%S + %S/%S\C-",
                                   "Up", "Down", "Left", "Right", "Run", "Left", "Right");
                    else
                        HudMessage("Navigate: \Cd%jS/%jS/%jS/%jS\C-\nSwitch pages: \Cd%jS + %jS/%jS\C-",
                                   "+forward", "+back", "+moveleft", "+moveright", "+speed", "+moveleft", "+moveright");
                }
                else
                {
                    if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                        HudMessage("Navigate: \Cd%S/%S/%S/%S\C-\nSwitch pages: \Cd%S + %S/%S\C-\nIncrease Stat: \Cd%S\C-",
                                   "Up", "Down", "Left", "Right", "Run", "Left", "Right", "Use");
                    else
                        HudMessage("Navigate: \Cd%jS/%jS/%jS/%jS\C-\nSwitch pages: \Cd%jS + %jS/%jS\C-\nIncrease Stat: \Cd%jS\C-",
                                   "+forward", "+back", "+moveleft", "+moveright", "+speed", "+moveleft", "+moveright", "+use");
                }
                EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            }
            else if (Player.StatPage == STATPAGE_STATXP || Player.StatPage == STATPAGE_PERKS)
            {
                if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                    HudMessage("Switch pages: \Cd%S + %S/%S\C-",
                               "Run", "Left", "Right");
                else
                    HudMessage("Switch pages: \Cd%jS + %jS/%jS\C-",
                               "+speed", "+moveleft", "+moveright");
                EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            }
            else if (Player.StatPage == STATPAGE_TEAM)
            {
                if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                    HudMessage("Navigate: \Cd%S/%S\C-\nSwitch pages: \Cd%S + %S/%S\C-\nPlayer's view on HUD: \Cd%S\C-\nTeleport to selected Player (\Cn%d EP\C-): \Cd%S\C-",
                               "Up", "Down", "Run", "Left", "Right", "Attack", ScaleEPCost(50), "Use");
                else
                    HudMessage("Navigate: \Cd%jS/%jS\C-\nSwitch pages: \Cd%jS + %jS/%jS\C-\nPlayer's view on HUD: \Cd%jS\C-\nTeleport to selected Player (\Cn%d EP\C-): \Cd%jS\C-",
                               "+forward", "+back", "+speed", "+moveleft", "+moveright", "+attack", ScaleEPCost(50), "+use");
                EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            }
            break;
        case MENUPAGE_AUGS:
            if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                HudMessage("Navigate: \Cd%S/%S/%S/%S\C-\nSwitch Current Level: \Cd%S + %S/%S\C-\nToggle On/Off: \Cd%S\C-\nUpgrade: \Cd%S + %S\C-",
                           "Up", "Down", "Left", "Right", "Run", "Up", "Down", "Use", "Run", "Use");
            else
                HudMessage("Navigate: \Cd%jS/%jS/%jS/%jS\C-\nSwitch Current Level: \Cd%jS + %jS/%jS\C-\nToggle On/Off: \Cd%jS\C-\nUpgrade: \Cd%jS + %jS\C-",
                           "+forward", "+back", "+moveleft", "+moveright", "+speed", "+forward", "+back", "+use", "+speed", "+use");
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            break;
        case MENUPAGE_SKILLS:
            if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                HudMessage("Navigate: \Cd%S/%S/%S/%S\C-\nSwitch Pages: \Cd%S + %S/%S\C-\nLearn/Upgrade Skill: \Cd%S\C-\nAssign Skill to Wheel Slot: \CiHOLD \Cd%S\C-\nClear Skill from Wheel Slot: \CiHOLD \Cd%S + %S + %S\C-\nSwitch Current Skill Level: \Cd%S + %S/%S\C-\nQuickuse selected skill: \Cd%S\C-",
                           "Up", "Down", "Left", "Right", "Run", "Left", "Right", "Use", "Skill Wheel", "Skill Wheel", "Run", "Use", "Run", "Up", "Down", "Attack");
            else
                HudMessage("Navigate: \Cd%jS/%jS/%jS/%jS\C-\nSwitch Pages: \Cd%jS + %jS/%jS\C-\nLearn/Upgrade Skill: \Cd%jS\C-\nAssign Skill to Wheel Slot: \CiHOLD \Cd%jS\C-\nClear Skill from Wheel Slot: \CiHOLD \Cd%jS + %jS + %jS\C-\nSwitch Current Skill Level: \Cd%jS + %jS/%jS\C-\nQuickuse selected skill: \Cd%jS\C-",
                           "+forward", "+back", "+moveleft", "+moveright", "+speed", "+moveleft", "+moveright", "+use", "+user1", "+user1", "+speed", "+use", "+speed", "+forward", "+back", "+attack");
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            break;
        case MENUPAGE_SHIELD:
            if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                HudMessage("Navigate: \Cd%S/%S/%S/%S\C-\nSwitch Pages: \Cd%S + %S/%S\C-\nEquip Part: \Cd%S\C-\nUnequip Part: \Cd%S + %S\C-",
                           "Up", "Down", "Left", "Right", "Run", "Left", "Right", "Use", "Run", "Use");
            else
                HudMessage("Navigate: \Cd%jS/%jS/%jS/%jS\C-\nSwitch Pages: \Cd%jS + %jS/%jS\C-\nEquip Part: \Cd%jS\C-\nUnequip Part: \Cd%jS + %jS\C-",
                           "+forward", "+back", "+moveleft", "+moveright", "+speed", "+moveleft", "+moveright", "+use", "+speed", "+use");
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y + 45.0, 0.05); // This help is pushed down so shield component description doesn't overlap it.
            break;
        case MENUPAGE_STIMS:
            if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                HudMessage("Navigate: \Cd%S/%S\C-\nSelect Stim Size: \Cd%S/%S\C-\nChoose Stim/Add Compound to Stim: \Cd%S\C-\nUse Stim: \Cd%S\C-\nDispose Stim: \Cd%S\C-",
                           "Up", "Down", "Left", "Right", "Use", "Use Stim Command", "Throwaway Stim Command");
            else
                HudMessage("Navigate: \Cd%jS/%jS\C-\nSelect Stim Size: \Cd%jS/%jS\C-\nChoose Stim/Add Compound to Stim: \Cd%jS\C-\nUse Stim: \Cd%jS\C-\nDispose Stim: \Cd%jS\C-",
                           "+forward", "+back", "+moveleft", "+moveright", "+use", "drpg_usestim", "drpg_throwawaystim");
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            break;
        case MENUPAGE_TURRET:
            if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                HudMessage("Navigate: \Cd%S/%S/%S/%S\C-\nSwitch Pages: \Cd%S\C-\n%S: \Cd%S\C-\n%S: \Cd%S\C-\n",
                           "Up", "Down", "Left", "Right", "Jump", Player.Turret.Maintenance? "Reclaim Turret" : "Maintenance Turret", "Run", (Player.TurretPage == TURRETPAGE_COMMAND)? "Issue Command" : "Upgrade Turret", "Use");
            else
                HudMessage("Navigate: \Cd%jS/%jS/%jS/%jS\C-\nSwitch Pages: \Cd%jS\C-\n%S: \Cd%jS\C-\n%S: \Cd%jS\C-\n",
                           "+forward", "+back", "+moveleft", "+moveright", "+jump", Player.Turret.Maintenance? "Reclaim Turret" : "Maintenance Turret", "+speed", (Player.TurretPage == TURRETPAGE_COMMAND)? "Issue Command" : "Upgrade Turret", "+use");
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            break;
        }

    // Shop Menu Help
    if (!Player.InMenu && Player.InShop)
    {
        // For Shield Parts Category
        if (Player.ShopPage == 5 && Player.ShopIndex > 0)
        {
            if (Player.LockerMode)
            {
                if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                    HudMessage("Switch to the Shop: \Cd%S\C-\nSwitch Pages: \Cd%S + %S/%S\C-\nDeposit Item: \Cd%S\C-\nWithdraw Item: \Cd%S + %S\C-\nToggle Auto-Storing of item: \Cd%S\C-\nWithdraw all items on the page: \Cd%S\C-\nExit: \Cd%S\C-",
                               "Jump", "Run", "Left", "Right", "Use", "Run", "Use", "Attack", "Alt Attack", "Menu");
                else
                    HudMessage("Switch to the Shop: \Cd%jS\C-\nSwitch Pages: \Cd%jS + %jS/%jS\C-\nDeposit Item: \Cd%jS\C-\nWithdraw Item: \Cd%jS + %jS\C-\nToggle Auto-Storing of item: \Cd%jS\C-\nWithdraw all items on the page: \Cd%jS\C-\nExit: \Cd%jS\C-",
                               "+jump", "+speed", "+moveleft", "+moveright", "+use", "+speed", "+use", "+attack", "+altattack", "drpg_menu");
                EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y + 30.0, 0.05);
            }
            else
            {
                if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                    HudMessage("Switch to the Locker: \Cd%S\C-\nSwitch Pages: \Cd%S + %S/%S\C-\nBuy Item: \Cd%S\C-\nSell Item: \Cd%S + %S\C-\nToggle Auto-Selling of item: \Cd%S\C-\nDrop selected item: \Cd%S\C-\nExit: \Cd%S\C-",
                               "Jump", "Run", "Left", "Right", "Use", "Run", "Use", "Attack", "Drop Item", "Menu");
                else
                    HudMessage("Switch to the Locker: \Cd%jS\C-\nSwitch Pages: \Cd%jS + %jS/%jS\C-\nBuy Item: \Cd%jS\C-\nSell Item: \Cd%jS + %jS\C-\nToggle Auto-Selling of item: \Cd%jS\C-\nDrop selected item: \Cd%jS\C-\nExit: \Cd%jS\C-",
                               "+jump", "+speed", "+moveleft", "+moveright", "+use", "+speed", "+use", "+attack", "+zoom", "drpg_menu");
                EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y + 30.0, 0.05);
            }
        }
        // For Everything Else
        else
        {
            if (Player.LockerMode)
            {
                if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                    HudMessage("Switch to the Shop: \Cd%S\C-\nNavigate: \Cd%S/%S/%S/%S\C-\nSwitch Pages: \Cd%S + %S/%S\C-\nDeposit Item: \Cd%S\C-\nWithdraw Item: \Cd%S + %S\C-\nToggle Auto-Storing of item: \Cd%S\C-\nToggle Auto-Keeping of 1 item: \Cd%S + %S\C-\nDrop selected item: \Cd%S\C-\nWithdraw all items on the page: \Cd%S\C-\nExit: \Cd%S\C-",
                               "Jump", "Up", "Down", "Left", "Right", "Run", "Left", "Right", "Use", "Run", "Use", "Attack", "Run", "Attack", "Drop Item", "Alt Attack", "Menu");
                else
                    HudMessage("Switch to the Shop: \Cd%jS\C-\nNavigate: \Cd%jS/%jS/%jS/%jS\C-\nSwitch Pages: \Cd%jS + %jS/%jS\C-\nDeposit Item: \Cd%jS\C-\nWithdraw Item: \Cd%jS + %jS\C-\nToggle Auto-Storing of item: \Cd%jS\C-\nToggle Auto-Keeping of 1 item: \Cd%jS + %jS\C-\nDrop selected item: \Cd%jS\C-\nWithdraw all items on the page: \Cd%jS\C-\nExit: \Cd%jS\C-",
                               "+jump", "+forward", "+back", "+moveleft", "+moveright", "+speed", "+moveleft", "+moveright", "+use", "+speed", "+use", "+attack", "+speed", "+attack", "+zoom", "+altattack", "drpg_menu");
                EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            }
            else
            {
                if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                    HudMessage("Switch to the Locker: \Cd%S\C-\nNavigate: \Cd%S/%S/%S/%S\C-\nSwitch Pages: \Cd%S + %S/%S\C-\nBuy Item: \Cd%S\C-\nSell Item: \Cd%S + %S\C-\nToggle Auto-Selling of item: \Cd%S\C-\nToggle Auto-Keeping of 1 item: \Cd%S + %S\C-\nDrop selected item: \Cd%S\C-\nExit: \Cd%S\C-",
                               "Jump", "Up", "Down", "Left", "Right", "Run", "Left", "Right", "Use", "Run", "Use", "Attack", "Run", "Attack", "Drop Item", "Menu");
                else
                    HudMessage("Switch to the Locker: \Cd%jS\C-\nNavigate: \Cd%jS/%jS/%jS/%jS\C-\nSwitch Pages: \Cd%jS + %jS/%jS\C-\nBuy Item: \Cd%jS\C-\nSell Item: \Cd%jS + %jS\C-\nToggle Auto-Selling of item: \Cd%jS\C-\nToggle Auto-Keeping of 1 item: \Cd%jS + %jS\C-\nDrop selected item: \Cd%jS\C-\nExit: \Cd%jS\C-",
                               "+jump", "+forward", "+back", "+moveleft", "+moveright", "+speed", "+moveleft", "+moveright", "+use", "+speed", "+use", "+attack", "+speed", "+attack", "+zoom", "drpg_menu");
                EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            }
        }
    }

    // Outpost Menus
    if (Player.OutpostMenu > 0)
    {
        switch (Player.OutpostMenu)
        {
        case OMENU_LEVELTRANSPORT:
            if (MapPacks)
            {
                if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                    HudMessage("Select Level: \Cd%S/%S/%S/%S\C-\nSelect IWAD: \Cd%S + %S/%S\C-\nTeleport to Level: \Cd%S\C-\nExit: \Cd%S\C-",
                               "Up", "Down", "Left", "Right", "Run", "Left", "Right", "Use", "Menu");
                else
                    HudMessage("Select Level: \Cd%jS/%jS/%jS/%jS\C-\nSelect IWAD: \Cd%jS + %jS/%jS\C-\nTeleport to Level: \Cd%jS\C-\nExit: \Cd%jS\C-",
                               "+forward", "+back", "+moveleft", "+moveright", "+speed", "+moveleft", "+moveright", "+use", "drpg_menu");
            }
            else
            {
                if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                    HudMessage("Select Level: \Cd%S/%S/%S/%S\C-\nTeleport to Level: \Cd%S\C-\nExit: \Cd%S\C-",
                               "Up", "Down", "Left", "Right", "Use", "Menu");
                else
                    HudMessage("Select Level: \Cd%jS/%jS/%jS/%jS\C-\nTeleport to Level: \Cd%jS\C-\nExit: \Cd%jS\C-",
                               "+forward", "+back", "+moveleft", "+moveright", "+use", "drpg_menu");
            }
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            break;
        case OMENU_ARENATRANSPORT:
            if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                HudMessage("Select Arena: \Cd%S/%S\C-\nTeleport to Arena: \Cd%S\C-\nExit: \Cd%S\C-",
                           "Left", "Right", "Use", "Menu");
            else
                HudMessage("Select Arena: \Cd%jS/%jS\C-\nTeleport to Arena: \Cd%jS\C-\nExit: \Cd%jS\C-",
                           "+moveleft", "+moveright", "+use", "drpg_menu");
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            break;
        case OMENU_SKILLCOMPUTER:
            if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                HudMessage("Navigate: \Cd%S/%S\C-\nChange Skill: \Cd%S\C-\nExit: \Cd%S\C-",
                           "Up", "Down", "Use", "Menu");
            else
                HudMessage("Navigate: \Cd%jS/%jS\C-\nChange Skill: \Cd%jS\C-\nExit: \Cd%jS\C-",
                           "+forward", "+back", "+use", "drpg_menu");
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            break;
        case OMENU_MODULECONVERTER:
            break;
        case OMENU_WAVESELECTOR:
            if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                HudMessage("Choose Wave: \Cd%S/%S/%S/%S\C-\nChange Wave: \Cd%S\C-\nExit: \Cd%S\C-",
                           "Up", "Down", "Left", "Right", "Use", "Menu");
            else
                HudMessage("Choose Wave: \Cd%jS/%jS/%jS/%jS\C-\nChange Wave: \Cd%jS\C-\nExit: \Cd%jS\C-",
                           "+forward", "+back", "+moveleft", "+moveright", "+use", "drpg_menu");
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            break;
        case OMENU_SHOPSPECIAL:
            if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                HudMessage("Buy: \Cd%S\C-\nExit: \Cd%S\C-",
                           "Use", "Menu");
            else
                HudMessage("Buy: \Cd%jS\C-\nExit: \Cd%jS\C-",
                           "+use", "drpg_menu");
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            break;
        case OMENU_BONUSSELECTOR:
            if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                HudMessage("Navigate: \Cd%S/%S\C-\nSelect: \Cd%S\C-",
                           "Up", "Down", "Use");
            else
                HudMessage("Navigate: \Cd%jS/%jS\C-\nSelect: \Cd%jS\C-",
                           "+forward", "+back", "+use");
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            break;
        case OMENU_BBS:
            if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                HudMessage("Navigate: \Cd%S/%S/%S/%S\C-\nAccept Mission: \Cd%S\C-\nAbort Mission: \Cd%S\C-\nSwitch Pages: \Cd%S + %S/%S\C-\nExit: \Cd%S\C-",
                           "Up", "Down", "Left", "Right", "Use", "Attack", "Run", "Left", "Right", "Menu");
            else
                HudMessage("Navigate: \Cd%jS/%jS/%jS/%jS\C-\nAccept Mission: \Cd%jS\C-\nAbort Mission: \Cd%jS\C-\nSwitch Pages: \Cd%jS + %jS/%jS\C-\nExit: \Cd%jS\C-",
                           "+forward", "+back", "+moveleft", "+moveright", "+use", "+attack", "+speed", "+moveleft", "+moveright", "drpg_menu");
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            break;
        case OMENU_OPERATINGCAPSULE:
            if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                HudMessage("Navigate: \Cd%S/%S\C-\nConfirm: \Cd%S + \Cd%S\C-\nExit: \Cd%S\C-",
                           "Left", "Right", "Run", "Use", "Menu");
            else
                HudMessage("Navigate: \Cd%jS/%jS\C-\nConfirm: \Cd%jS + \Cd%jS\C-\nExit: \Cd%jS\C-",
                           "+moveleft", "+moveright", "+speed", "+use", "drpg_menu");
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            break;
        case OMENU_DISASSEMBLINGDEVICE:
            if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                HudMessage("Navigate: \Cd%S/%S/%S/%S\C-\nConfirm: \Cd%S\C-\nExit: \Cd%S\C-",
                           "Up", "Down", "Left", "Right", "Use", "Menu");
            else
                HudMessage("Navigate: \Cd%jS/%jS/%jS/%jS\C-\nConfirm: \Cd%jS\C-\nExit: \Cd%jS\C-",
                           "+forward", "+back", "+moveleft", "+moveright", "+use", "drpg_menu");
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            break;
        case OMENU_DISASSEMBLING:
            if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                HudMessage("Select Category: \Cd%S/%S\C-\nSelect Item: \Cd%S/%S\C-\nConfirm: \Cd%S + \Cd%S\C-\nExit: \Cd%S\C-",
                           "Up", "Down", "Left", "Right", "Run", "Use", "Menu");
            else
                HudMessage("Select Category: \Cd%jS/%jS\C-\nSelect Item: \Cd%jS/%jS\C-\nConfirm: \Cd%jS + \Cd%jS\C-\nExit: \Cd%jS\C-",
                           "+forward", "+back", "+moveleft", "+moveright", "+speed", "+use", "drpg_menu");
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            break;
        case OMENU_ASSEMBLING:
            if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                HudMessage("Select Category: \Cd%S/%S\C-\nSelect Rare: \Cd%S + \Cd%S/%S\C-\nSelect Item: \Cd%S/%S\C-\nConfirm: \Cd%S + \Cd%S\C-\nExit: \Cd%S\C-",
                           "Up", "Down", "Run", "Up", "Down", "Left", "Right", "Run", "Use", "Menu");
            else
                HudMessage("Select Category: \Cd%jS/%jS\C-\nSelect Rare: \Cd%jS + \Cd%jS/%jS\C-\nSelect Item: \Cd%jS/%jS\C-\nConfirm: \Cd%jS + \Cd%jS\C-\nExit: \Cd%jS\C-",
                           "+forward", "+back", "+speed", "+forward", "+back", "+moveleft", "+moveright", "+speed", "+use", "drpg_menu");
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            break;
        case OMENU_DEMONSANCTUARY:
            if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
                HudMessage("Select Category: \Cd%S/%S\C-\nSelect Item: \Cd%S/%S\C-\nConfirm: \Cd%S + \Cd%S\C-\nExit: \Cd%S\C-",
                           "Up", "Down", "Left", "Right", "Run", "Use", "Menu");
            else
                HudMessage("Select Category: \Cd%jS/%jS\C-\nSelect Item: \Cd%jS/%jS\C-\nConfirm: \Cd%jS + \Cd%jS\C-\nExit: \Cd%jS\C-",
                           "+forward", "+back", "+moveleft", "+moveright", "+speed", "+use", "drpg_menu");
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
            break;
        }
    }

    // Crate Help
    if (Player.CrateOpen && Crates[Player.CrateID].Hacking == -1)
    {
        if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
            HudMessage("Navigate: \Cd%S/%S/%S/%S\C-\nTake Item: \Cd%S\C-\nTake All: \Cd%S + %S\C-\nExit: \Cd%S\C-",
                       "Up", "Down", "Left", "Right", "Use", "Run", "Use", "Menu");
        else
            HudMessage("Navigate: \Cd%jS/%jS/%jS/%jS\C-\nTake Item: \Cd%jS\C-\nTake All: \Cd%jS + %jS\C-\nExit: \Cd%jS\C-",
                       "+forward", "+back", "+moveleft", "+moveright", "+use", "+speed", "+use", "drpg_menu");
        EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
    }
    // Crate Hacking Help
    else if (Player.CrateHacking)
    {
        if (GetCVar("use_joystick") || GetActivatorCVar("drpg_deltatouch"))
            HudMessage("Select: \Cd%S\C-\nExit: \Cd%S\C-",
                       "Use", "Menu");
        else
            HudMessage("Select: \Cd%jS\C-\nExit: \Cd%jS\C-",
                       "+use", "drpg_menu");
        EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 30, Y - 125, 0.05);
    }
}

void DrawPlayerSprite(int PlayerNum, fixed X, fixed Y)
{
    // Compatibility Handling - DoomRL Arsenal
    if (CompatMode == COMPAT_DRLA)
    {
        if (PlayerClass(PlayerNum) == 0) // Marine
            PrintSprite("PMARA1", 0, X, Y, 0.05);
        if (PlayerClass(PlayerNum) == 1) // Scout
            PrintSprite("PSCOA1", 0, X + 4.0, Y - 2.0, 0.05);
        if (PlayerClass(PlayerNum) == 2) // Technician
            PrintSprite("PTECA1", 0, X + 2.0, Y - 2.0, 0.05);
        if (PlayerClass(PlayerNum) == 3) // Renegade
            PrintSprite("PRENA1", 0, X + 4.0, Y - 2.0, 0.05);
        if (PlayerClass(PlayerNum) == 4) // Demolitionist
            PrintSprite("PDMOA1", 0, X + 4.0, Y - 2.0, 0.05);

        // Compatibility Handling - DoomRL Arsenal Extended
        if (CompatModeEx == COMPAT_DRLAX)
        {
            if (PlayerClass(PlayerNum) == 6) // Mechanoid
                PrintSprite("1337A1", 0, X + 4.0, Y - 2.0, 0.05);
            if (PlayerClass(PlayerNum) == 7) // Nomad
                PrintSprite("ZHRTA1", 0, X + 4.0, Y - 2.0, 0.05);
            if (PlayerClass(PlayerNum) == 8) // Nano Maniac
                PrintSprite("DKCPA1", 0, X + 4.0, Y - 2.0, 0.05);
            if (PlayerClass(PlayerNum) == 9 && CheckActorInventory(Players(PlayerNum).TID, "RLPhaseSistersSwapToken") == 1) // Phase Sisters Portia
                PrintSprite("ILLPA1", 0, X + 4.0, Y - 2.0, 0.05);
            if (PlayerClass(PlayerNum) == 9 && CheckActorInventory(Players(PlayerNum).TID, "RLPhaseSistersSwapToken") == 0) // Phase Sisters Terri
                PrintSprite("REVYA1", 0, X + 4.0, Y - 2.0, 0.05);
            if (PlayerClass(PlayerNum) == 10) // Sarge
                PrintSprite("SUT1A1", 0, X + 4.0, Y - 2.0, 0.05);
        }
    }
    else
        PrintSprite("PLAYA1", 0, X, Y + 2.0, 0.05);
}

void DrawToxicityBar(fixed X, fixed Y, bool HideInfo)
{
    // Toxicity Penalties
    if (GetActivatorCVar("drpg_menuhelp") && !HideInfo)
    {
        SetFont("SMALLFONT");
        if (Player.Toxicity >= 25)
        {
            HudMessage("- No Regeneration");
            EndHudMessage(HUDMSG_PLAIN, 0, "Brick", X + 0.0, Y + 32.0, 0.05);
        }
        if (Player.Toxicity >= 50)
        {
            HudMessage("- Energy Loss");
            EndHudMessage(HUDMSG_PLAIN, 0, "Brick", X + 0.0, Y + 40.0, 0.05);
        }
        if (Player.Toxicity >= 75)
        {
            HudMessage("- Reduced Movement Speed");
            EndHudMessage(HUDMSG_PLAIN, 0, "Brick", X + 0.0, Y + 48.0, 0.05);
        }
    }

    // If Toaster Mode on finish it
    if (GetCVar("drpg_toaster"))
        return;

    // Pixel Color
    str Color;
    if (Player.Toxicity >= 0 && Player.Toxicity <= 24)
        Color = "PGreen";
    if (Player.Toxicity >= 25 && Player.Toxicity <= 49)
        Color = "PYellow";
    if (Player.Toxicity >= 50 && Player.Toxicity <= 74)
        Color = "POrange";
    if (Player.Toxicity >= 75)
        Color = "PRed";

    // Beat
    int BeatTics[3] = { 8 / (1 + (Player.Toxicity >= 75)), 16 / (1 + (Player.Toxicity >= 50)), 8 / (1 + (Player.Toxicity >= 25)) };
    int TotalTics = 0;
    for (int i = 0; i < 3; i++)
        TotalTics += BeatTics[i];
    if ((Player.ToxicTimer % (35 * (3 - (int)((fixed)Player.Toxicity * 0.0275k)))) < TotalTics)
    {
        if (Player.ToxicOffset >= -8 && Player.ToxicStage == 0)
            Player.ToxicOffset -= 1 + (Player.Toxicity >= 75);
        if (Player.ToxicOffset <= 8 && Player.ToxicStage == 1)
            Player.ToxicOffset += 1 + (Player.Toxicity >= 50);
        if (Player.ToxicOffset >= 0 && Player.ToxicStage == 2)
            Player.ToxicOffset -= 1 + (Player.Toxicity >= 25);

        if (Player.ToxicOffset == -8)
            Player.ToxicStage = 1;
        if (Player.ToxicOffset == 8)
            Player.ToxicStage = 2;
        if (Player.ToxicOffset == 0 && Player.ToxicStage == 2)
            Player.ToxicStage = 0;
    }

    // Increase Timer
    Player.ToxicTimer++;

    // Draw Pixel
    if (Player.Toxicity >= 100)
        PrintSpriteFade(Color, TOXMETER_ID + (Player.ToxicTimer % 100), X + (Player.ToxicTimer % 100), Y + 16.0, 0.05, 1.0);
    else
        PrintSpriteFade(Color, TOXMETER_ID + (Player.ToxicTimer % 100), X + (Player.ToxicTimer % 100), Y + 16.0 + Player.ToxicOffset, 0.05, 1.0);
}

void ClearToxicityMeter()
{
    Player.ToxicTimer = 0;
    Player.ToxicOffset = 0;
    Player.ToxicStage = 0;

    for (int i = TOXMETER_ID; i < TOXMETER_ID + 100; i++)
        ClearMessage(i);
}
