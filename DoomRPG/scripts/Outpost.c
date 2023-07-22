#include "Defs.h"

#include "Arena.h"
#include "Augs.h"
#include "ItemData.h"
#include "Map.h"
#include "Menu.h"
#include "Minigame.h"
#include "Mission.h"
#include "Outpost.h"
#include "RPG.h"
#include "Shield.h"
#include "Shop.h"
#include "Skills.h"
#include "Stats.h"
#include "Stims.h"
#include "Utils.h"

// Globals
int OutpostNotoriety;

// Spawn Spots and TIDs
int RPGMap ShopSpecialTID = 1001;
int RPGMap SecretTeleportTID = 1010;
int RPGMap MarineSpotTID = 1300;
int RPGMap NotorietySpotTID = 1301;
int RPGMap MarineTID = 1400;
int RPGMap MarineMaxTID = 1419;
int RPGMap MarineBossTID = 1421;
int RPGMap BossSpotTID = 1475;
int RPGMap ForcefieldID = 1500;
int RPGMap ForcefieldGeneratorID = 1501;
int RPGMap GeneratorTID = 1510;
int RPGMap EntrySectorID = 200;
int RPGMap HallTeleportSpotID = 1601;
int RPGMap CameraTID = 1700;
int RPGMap RiftSpotTID = 1800;
int RPGMap CreditsBlockerID = 1900;
int RPGMap DisassemblingDeviceID = 29;
int RPGMap DemonSanctuaryID = 47;

// Timers
int RPGMap ForcefieldTimer = 0;

// Choices
int RPGMap LevelChoice = 0;
int RPGMap SkillChoice = 0;
int RPGMap WaveChoice = 1;

// Flags
bool RPGMap MarinesHostile = false;
bool RPGMap PowerOut = false;
bool RPGMap BossDead = false;
bool RPGMap Invasion = false;

NamedScript MapSpecial void EnterOutpost()
{
    ForcefieldTimer = 35 * 60 * GameSkill(); // 1 Minute per skill level

    SetHudSize(320, 240, true);

    // Assign the current level to the level choice
    LevelChoice = FindLevelInfoIndex(TransporterLevel->LumpName);

    // Assign the current skill level to the skill choice
    SkillChoice = GameSkill() - 1;

    // Set the current wave to the max wave
    ArenaWave = ArenaMaxWave;

    // Spawn the Shop Special item
    SpawnShopSpecialItem();

    // Boss Placement
    PlaceBoss();

    // Get new Missions
    PopulateMissions();

    // Title map handling
    if (InTitle)
    {
        ChangeCamera(CameraTID, true, false);
        Thing_Activate(CameraTID);
        Delay(35 * Random(3, 5));

        // Make the Marines fight enemies in title map only if Toaster Mod is off
        if (!GetCVar("drpg_toaster"))
        {
            PissOffMarines(false);

            if (CompatMonMode == COMPAT_DRLA)
                AmbientSound("nightmarecyberdemon/sight", 127);
        }

        ActivatorSound("misc/skillchange", 127);

        while (InTitle)
        {
            PrintSpritePulse("M_DOOM", 1, 160.0 + 8.0, 80.0 - 8.0, 0.75, 64.0, 0.25);
            Delay(1);
        }
    }
    else
    {
        // Music
        SetOutpostMusic(OUTPOST_MUSIC_NORMAL);
    }

    // Compatibility Handling - DoomRL Arsenal Extended
    if (CompatModeEx == COMPAT_DRLAX)
    {
        // Spawn DoomRL Arsenal Extended Objects
        if (AveragePlayerRank() >= 2)
            SpawnSpotForced("DRLAX_DimensionalBoxRPG", 40, UniqueTID(), 0);
        if (AveragePlayerRank() >= 4)
            SpawnSpotForced("DRLAX_WeaponRecyclerRPG", 41, UniqueTID(), 0);
        if (AveragePlayerRank() >= 6)
            SpawnSpotForced("DRLAX_ModReviserRPG", 42, UniqueTID(), 0);
        if (AveragePlayerRank() >= 8)
            SpawnSpotForced("DRLAX_AppraiserObjectRPG", 43, UniqueTID(), 0);
        if (AveragePlayerRank() >= 10)
            SpawnSpotForced("DRLAX_SkullShrineObjectRPG", 52, UniqueTID(), 0);
    }
}

NamedScript MapSpecial void RegenArea(int ID)
{
    // If you're dead, terminate
    if (GetActorProperty(0, APROP_Health) <= 0) return;

    // Health
    if (ID == OREGEN_HEALTH)
    {
        int HealthCharges;
        bool DoMessage = false;

        if (CheckInventory("DRPGCredits") > 0 && Player.ActualHealth < Player.HealthMax)
        {
            HealthCharges = Player.HealthMax - Player.ActualHealth;
            if (HealthCharges > CheckInventory("DRPGCredits"))
                HealthCharges = CheckInventory("DRPGCredits");

            AddHealthDirect(HealthCharges, 100);
            TakeInventory("DRPGCredits", HealthCharges);
            DoMessage = true;
        }

        if (CheckInventory("DRPGCredits") > 0 && Player.Medkit < Player.MedkitMax)
        {
            HealthCharges = Player.MedkitMax - Player.Medkit;
            if (HealthCharges > CheckInventory("DRPGCredits"))
                HealthCharges = CheckInventory("DRPGCredits");

            Player.Medkit += HealthCharges;
            TakeInventory("DRPGCredits", HealthCharges);
            DoMessage = true;
        }

        if (CheckInventory("DRPGCredits") >= 5 && Player.Toxicity > 0)
        {
            HealthCharges = Player.Toxicity;
            if (HealthCharges > CheckInventory("DRPGCredits") / 5)
                HealthCharges = CheckInventory("DRPGCredits") / 5;

            ClearToxicityMeter();
            Player.Toxicity -= HealthCharges;
            TakeInventory("DRPGCredits", HealthCharges * 5);
            DoMessage = true;
        }

        if (CheckInventory("DRPGCredits") >= 50 && HaveStatusEffect())
        {
            ClearStatusEffects();
            TakeInventory("DRPGCredits", 50);
            DoMessage = true;
        }

        if (DoMessage)
        {
            SetFont("BIGFONT");
            HudMessage("HP rejuvenated");
            EndHudMessage(HUDMSG_FADEOUT, 0, "Brick", 0.5, 0.33, 2.0, 0.5);
            FadeRange(255, 0, 0, 0.5, 255, 0, 0, 0.0, 1.0);
            ActivatorSound("regen/health", 127);
        }
    }

    // Armor
    if (ID == OREGEN_ARMOR)
    {
        if (CheckInventory("Armor") >= GetArmorInfo(ARMORINFO_SAVEAMOUNT))
            return;

        if (CompatMode == COMPAT_DRLA)
        {
            if (CheckInventory("RLFireShieldArmorToken") || CheckInventory("RLTowerShieldArmorToken") || CheckInventory("RLBallisticShieldArmorToken") || CheckInventory("RLEnergyShieldArmorToken") || CheckInventory("RLPlasmaShieldArmorToken") || CheckInventory("RLRechargeableEnergyShieldArmorToken"))
            {
                SetFont("BIGFONT");
                HudMessage("This type of armor\ncan not be repaired");
                EndHudMessage(HUDMSG_FADEOUT, 1, "Red", 0.5, 0.33, 2.0, 0.5);
                ActivatorSound("menu/error", 127);
                return;
            }
        }

        int ArmorPercent = CheckInventory("Armor") * 100 / GetArmorInfo(ARMORINFO_SAVEAMOUNT);
        int ArmorFee = (100 - ArmorPercent) / 5 * 5;
        if (ArmorFee < 5)
            ArmorFee = 5;

        if (CheckInventory("DRPGCredits") < ArmorFee)
        {
            SetFont("BIGFONT");
            HudMessage("You need %d credits\nto repair this armor", ArmorFee);
            EndHudMessage(HUDMSG_FADEOUT, 1, "Red", 0.5, 0.33, 2.0, 0.5);
            ActivatorSound("menu/error", 127);
            return;
        }

        GiveInventory(GetArmorInfoString(ARMORINFO_CLASSNAME), 1);
        TakeInventory("DRPGCredits", ArmorFee);

        SetFont("BIGFONT");
        HudMessage("Armor repaired");
        EndHudMessage(HUDMSG_FADEOUT, 0, "Green", 0.5, 0.33, 2.0, 0.5);
        FadeRange(0, 255, 0, 0.5, 0, 255, 0, 0.0, 1.0);
        ActivatorSound("regen/armor", 127);
    }

    // EP (and Shields)
    if (ID == OREGEN_EP)
    {
        // Holding Run will restore your Shield capacity
        if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()) && CheckShieldValid())
        {
            if (CheckInventory("DRPGCredits") < 1 || Player.Shield.Charge >= Player.Shield.Capacity) return;

            int ShieldCharges = (Player.Shield.Capacity - Player.Shield.Charge) / 5;
            if (Player.Shield.Charge % 5 > 0)
                ShieldCharges++;
            if (CheckInventory("DRPGCredits") < ShieldCharges)
                ShieldCharges = CheckInventory("DRPGCredits");

            int ShieldGive = ShieldCharges * 5;

            if (Player.Shield.Charge + ShieldGive > Player.Shield.Capacity)
                ShieldGive = Player.Shield.Capacity - Player.Shield.Charge;

            Player.Shield.Charge += ShieldGive;

            TakeInventory("DRPGCredits", ShieldCharges);

            SetFont("BIGFONT");
            HudMessage("Shield refreshed");
            EndHudMessage(HUDMSG_FADEOUT, 0, "Cyan", 0.5, 0.33, 2.0, 0.5);
            FadeRange(0, 255, 255, 0.5, 0, 255, 255, 0.0, 1.0);
            ActivatorSound("regen/shield", 127);
        }
        else
        {
            if (CheckInventory("DRPGCredits") < 1 || Player.EP >= Player.EPMax) return;

            // EP pad Cooldown.
            // Default timer is 5 mins.
            int CurrentTime = Timer();
            if (CurrentTime + 35 * 60 * 5 + 1 < Player.EPPadCooldownTimer) Player.EPPadCooldown = false;
            if (!Player.EPPadCooldown)
            {
                Player.EPPadCooldownTimer = CurrentTime;
                Player.EPPadCooldownTimer += 35 * 60 * 5;
                Player.EPPadCooldown = true;
            }
            else
            {
                if (CurrentTime >= Player.EPPadCooldownTimer)
                {
                    Player.EPPadCooldown = false;
                }
                else
                {
                    SetFont("BIGFONT");
                    HudMessage("EP pad is cooling down: %S remaining", FormatTime(Player.EPPadCooldownTimer - CurrentTime));
                    EndHudMessage(HUDMSG_FADEOUT, 0, "LightBlue", 0.5, 0.33, 2.0, 0.5);
                    return;
                }
            }

            int EPCharges = (Player.EPMax - Player.EP) / 5;
            if (Player.EP % 5 > 0)
                EPCharges++;
            if (CheckInventory("DRPGCredits") < EPCharges)
                EPCharges = CheckInventory("DRPGCredits");

            int EPGive = EPCharges * 5;

            if (Player.EP + EPGive > Player.EPMax)
                EPGive = Player.EPMax - Player.EP;

            Player.EP += EPGive;

            TakeInventory("DRPGCredits", EPCharges);

            SetFont("BIGFONT");
            HudMessage("EP restored");
            EndHudMessage(HUDMSG_FADEOUT, 0, "LightBlue", 0.5, 0.33, 2.0, 0.5);
            FadeRange(0, 255, 255, 0.5, 0, 255, 255, 0.0, 1.0);
            ActivatorSound("regen/ep", 127);
        }
    }

    // Augmentation Battery
    if (ID == OREGEN_AUG)
    {
        if (CheckInventory("DRPGCredits") < 10 || Player.Augs.Battery >= Player.Augs.BatteryMax) return;

        int BatteryCharges = Player.Augs.BatteryMax - Player.Augs.Battery;
        if (CheckInventory("DRPGCredits") / 10 < BatteryCharges)
            BatteryCharges = CheckInventory("DRPGCredits") / 10;

        AddBattery(BatteryCharges);
        TakeInventory("DRPGCredits", BatteryCharges * 10);

        if (Player.Augs.BatteryMax - Player.Augs.Battery < 1 && Player.Augs.BatteryMax - Player.Augs.Battery > 0)
            AddBattery(1); // One more to get rid of the fraction-of-a-percent

        SetFont("BIGFONT");
        HudMessage("Augmentation battery recharged");
        EndHudMessage(HUDMSG_FADEOUT, 0, "Yellow", 0.5, 0.33, 2.0, 0.5);
        FadeRange(255, 255, 0, 0.5, 255, 255, 0, 0.0, 1.0);
        ActivatorSound("regen/battery", 127);
    }
}

NamedScript MapSpecial void LevelTransport()
{
    fixed X = 100.1;
    fixed Y = 32.0;
    fixed Y1;

    // if you're already in a menu, terminate
    if (Player.InMenu || Player.InShop || Player.OutpostMenu == OMENU_LEVELTRANSPORT) return;

    // If the Marines are hostile and the bosses aren't dead, terminate
    if (MarinesHostile && ThingCount(0, MarineBossTID) > 0) return;

    SetPlayerProperty(0, 1, PROP_TOTALLYFROZEN);
    Player.OutpostMenu = OMENU_LEVELTRANSPORT;

    str WadNames[MAX_MAPPACKS] =
    {
        "Doom",                      // WadSmoosh
        "Sigil",
        "Doom II: Hell on Earth",
        "No Rest For The Living",
        "Master Levels",
        "TNT: Evilution",
        "Plutonia",
        "Lexicon's Hub",             // Lexicon
        "Ancient Aliens",
        "A Clear Line Drawn",
        "Hot Water Music",
        "Alien Vendetta",
        "Back to Saturn X",
        "Community Chest 1",
        "Community Chest 2",
        "Community Chest 3",
        "Community Chest 4",
        "Chillax",
        "Circle Of Caina",
        "Combat Shock",
        "Combat Shock 2",
        "Chainworm",
        "Doom Core Trilogy",
        "Drown In Blood",
        "Dark Encounters",
        "Destination Unknown",
        "Deus Vult",
        "Deus Vult 2",
        "Epic 1",
        "Epic 2",
        "Estranged",
        "The Eye",
        "Forest Swords",
        "Going Down",
        "Hellcore",
        "Hellbound",
        "Hell Pike",
        "Sens",
        "Hadephobia",
        "Hell Revealed",
        "Hell Revealed 2",
        "Interception",
        "Kamasutra",
        "Khorus Speedy Shit",
        "Mayhem 17",
        "Maps Of Chaos",
        "Monuments Of Mars",
        "New Gothic Movement",
        "New Gothic Movement 2",
        "NOVA",
        "Pizza Steve",
        "Doom 2 Redux",
        "Scythe 2",
        "Stardate 20x6",
        "Stardate 20x7",
        "Swift Death",
        "Slaughterfest 2012",
        "Slaughterfest 2013",
        "Shaitans Luck",
        "Sunlust",
        "Sunder",
        "Speed Of Doom",
        "Swim With The Whales",
        "Dark Tartarus",
        "The Spire",
        "The Spire 2",
        "Congestion",
        "CLAUSTERPHOBIA 2",
        "CLAUSTERPHOBIA 1",
        "1997 Tuneup Project",
        "UAC Ultra",
        "Unholy Realms",
        "Valiant",
        "Vanguard",
        "D2 The Way ID Did",
        "Whispers Of Satan",
        "Zone 300",
        "Zones Of Fear",
        "Compendium's Hub",          // Compendium
        "Memento Mori",
        "Memento Mori II",
        "Requiem",
        "Insertion",
        "Obituary",
        "Strain",
        "Biowar",
        "The Darkening",
        "The Trooper's Playground",
        "Predition's Gate",
        "Post",
        "Revolution",
        "Scientist",
        "Icarus Alien Vanguard",
        "Hell to Pay",
        "The Abyss",
        "The Talosian Incident",
        "All Hell is Breaking Loose!",
        "Enigma",
        "Realm of Chaos",
        "Dystopia 3",
        "Eternal Doom",
        "Rebight",
        "Scythe",
        "Caverns of Darkness",
        "The Darkening 2",
        "Equinox",
        "Marsdoom",
        "Bloodrust",
        "Osiris",
        "Brotherhood of Ruin",
        "Enjay ZDoom",
        "Daedalus",
        "Cleimos",
        "Asdoom 2",
        "Pleiades",
        "Dark Covenant",
        "Slayer",
        "Hell Factory",
        "Cyberdreams",
        "The Final Gathering",
        "Earth",
        "End Game",
        "Doom Resurrection",
        "Enslavement",
        "Biotech",
        "City of Doom",
        "Project Slipgate",
        "Dissolution of Eternity",
        "Suspended in Dusk",
        "Mancubus",
        "Last Day on Earth",
        "Vile Flesh",
        "The Vilecore",
        "Twilight Zone II",
        "Neodoom",
        "Annie",
        "99 Ways to Die",
        "Back to Hell"
    };

    // So the player's initial interaction is not processed as a menu action
    Delay(1);

    while (true)
    {
        // Reset additional coordinates
        Y1 = 0.0;

        // Stop Underflow
        if (LevelChoice < 0)
            LevelChoice = 0;

        // And Overflow
        if (LevelChoice >= KnownLevels->Position)
            LevelChoice = KnownLevels->Position - 1;

        LevelInfo *TeleDest = &((LevelInfo *)KnownLevels->Data)[LevelChoice];

        // Set the HUD Size
        SetHudSize(GetActivatorCVar("drpg_menu_width"), GetActivatorCVar("drpg_menu_height"), true);

        // Draw Border
        // These are pushed back a bit so the border doesn't overlap anything
        if (GetActivatorCVar("drpg_menu_background_border"))
            DrawBorder("Bor", -1, 8, -5.0, 0.0, 470, 470);

        // Text
        SetFont("BIGFONT");
        HudMessage("Level Transporter");
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID, "White", X, Y, 0.05, 0.025);

        str TitleColor = "Gold";
        if (TeleDest->NeedsRealInfo || !(TeleDest->Completed))
            TitleColor = "Red";
        else if (TeleDest->AllBonus)
            TitleColor = "Green";

        str MapType = "Standard Map";
        if (TeleDest->SecretMap)
            MapType = "Secret Map";
        if (TeleDest->UACBase)
            MapType = "UAC Base";
        if (TeleDest->UACArena)
            MapType = "UAC Arena";
        if (MapPacks)
        {
            if (!MapPackActive[Player.SelectedMapPack])
                Player.SelectedMapPack++;
            else if (LevelChoice > 0)
            {
                HudMessage("%S", WadNames[Player.SelectedMapPack]);
                EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 1, "Red", X, Y + 32.1, 0.05, 0.025);
                Y1 = 32.0;
            }
        }

        HudMessage("%S", TeleDest->NiceName);
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 2, TitleColor, X, Y + Y1 + 32.1, 0.05, 0.025);
        SetFont("SMALLFONT");
        if (TeleDest->LevelNum > 0)
        {
            HudMessage("%S, level %d - %S", TeleDest->LumpName, TeleDest->LevelNum, MapType);
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 3, "Orange", X, Y + Y1 + 48.1, 0.05, 0.025);
        }
        else
        {
            HudMessage("%S - %S", TeleDest->LumpName, MapType);
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 3, "DarkGreen", X, Y + Y1 + 48.1, 0.05, 0.025);
        }

        if (!TeleDest->NeedsRealInfo && !TeleDest->UACBase && !TeleDest->UACArena)
        {
            str KillColor = "Brick";
            if (TeleDest->KillBonus && Timer() & 16)
                KillColor = "Green";
            str ItemColor = "LightBlue";
            if (TeleDest->ItemsBonus && Timer() & 16)
                ItemColor = "Green";
            str SecretColor = "Yellow";
            if (TeleDest->SecretsBonus && Timer() & 16)
                SecretColor = "Green";
            str ParColor = "Gold";
            if (TeleDest->ParBonus && TeleDest->Par > 0 && Timer() & 16)
                ParColor = "Green";

            if (TeleDest->MaxTotalMonsters > 0)
            {
                HudMessage("Kills: %d / %d (%d%%)", TeleDest->MaxMonstersKilled, TeleDest->MaxTotalMonsters, TeleDest->MaxMonsterPercentage);
                EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 4, KillColor, X, Y + Y1 + 64.1, 0.05, 0.025);
            }
            if (TeleDest->MaxTotalItems > 0)
            {
                HudMessage("Items: %d / %d (%d%%)", TeleDest->MaxItemsFound, TeleDest->MaxTotalItems, TeleDest->MaxItemPercentage);
                EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 5, ItemColor, X, Y + Y1 + 72.1, 0.05, 0.025);
            }
            if (TeleDest->MaxTotalSecrets > 0)
            {
                HudMessage("Secrets: %d / %d (%d%%)", TeleDest->MaxSecretsFound, TeleDest->MaxTotalSecrets, TeleDest->MaxSecretPercentage);
                EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 6, SecretColor, X, Y + Y1 + 80.1, 0.05, 0.025);
            }

            if (TeleDest->Par > 0)
            {
                HudMessage("Par Time: %S", FormatTime(TeleDest->Par * 35));
                EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 7, ParColor, X, Y + Y1 + 96.1, 0.05, 0.025);
            }
            if (TeleDest->ShortestTime < 0x7FFFFFFF)
            {
                HudMessage("Completion Time: %S", FormatTime(TeleDest->ShortestTime * 35));
                EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 8, ParColor, X, Y + Y1 + 104.1, 0.05, 0.025);
            }
            else
            {
                HudMessage("Completion Time: \C[Red]N/A");
                EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 8, ParColor, X, Y + Y1 + 104.1, 0.05, 0.025);
            }

            str AreaText = "No anomalies detected";
            switch (TeleDest->Event)
            {
            case MAPEVENT_NONE:
            default:
                break;
            case MAPEVENT_MEGABOSS:
                AreaText = "\C[Grey]Detected: Unknown Hostile Lifeform (Megaboss)\n\CgUse extreme caution when engaging target.\C-\nA quarantine has been issued for this area.";
                break;
            case MAPEVENT_TOXICHAZARD:
                AreaText = StrParam("\C[DarkGreen]Detected: High Radiation Levels (Environmental Hazard)\C-\n\CdHazard level: %d\C-\nPlease wear protective clothing when traversing this location.", TeleDest->HazardLevel);
                break;
            case MAPEVENT_NUCLEARBOMB:
                AreaText = "\C[Orange]Detected: Area-Destruction Device (Thermonuclear Bomb)\C-\nDo not stay within the detonation area.";
                break;
            case MAPEVENT_LOWPOWER:
                AreaText = "\C[Red]Detected: Electrical Grid Malfunction (Low Power)\C-\nPlease contact the maintenance team for replacement power cell.";
                break;
            case MAPEVENT_ALLAURAS:
                AreaText = "\C[Yellow]Detected: Abnormal Energy Signature (All Auras)\C-\nThreat levels have increased within this area.";
                break;
            case MAPEVENT_ONEMONSTER:
                AreaText = StrParam("\C[LightBlue]Detected: Anomalous Lifeform Distribution (One-Monster)\n\C[Red]Last seen: %S", TeleDest->SelectedMonster->Name);
                break;
            case MAPEVENT_HELLUNLEASHED:
                AreaText = "\C[Brick]Detected: Active Battle Front (Hell Unleashed)\n\CgTake preparatory measures before engaging the enemy.";
                break;
            case MAPEVENT_HARMONIZEDAURAS:
                AreaText = "\C[Green]Detected: Stable Demonic Energy Signature (Harmonized Destruction)\C-\nThreat levels have increased within this area.";
                break;
            case MAPEVENT_TELEPORTCRACKS:
                AreaText = "\C[Green]Detected: Spatial Instability (Cracks in the Veil)\C-\nTraverse the area with caution.";
                break;
            case MAPEVENT_DOOMSDAY:
                AreaText = "\C[Brick]Detected: Hell Zone (Doomsday)\n\CgUse EXTREME caution.";
                break;
            case MAPEVENT_ACIDRAIN:
                AreaText = "\C[DarkGreen]Weather Condition: Acid Rain (Vicious Downpour)\C-\nPlease wear protective clothing when traversing outdoor areas.";
                break;
            case MAPEVENT_DARKZONE:
                AreaText = "\C[Purple]Weather Condition: Dense Mist (The Dark Zone)\n\CaAvoid prolonged exposure if possible.";
                break;
            case MAPEVENT_DRLA_FEEDINGFRENZY:
                AreaText = StrParam("\C[LightBlue]Det%c%ccted: Anom%clous Lif%c%c%c%c\n\n\n       T     H  E       Y        F     %c    E  D\n\n\n          Y     %c        U      R         F     L     3      %c         H", 33 + Random(0, 94), 33 + Random(0, 94), 33 + Random(0, 94), 33 + Random(0, 94), 33 + Random(0, 94), 33 + Random(0, 94), 33 + Random(0, 94), 33 + Random(0, 94), 33 + Random(0, 94), 33 + Random(0, 94));
                break;
            case MAPEVENT_DRLA_OVERMIND:
                AreaText = StrParam("\C[Red]Er%cor: Unable to read addr%c%c%c%c\n\n\n\CgI SEE THE DARKNESS WITHIN YOUR SOUL.", 33 + Random(0, 94), 33 + Random(0, 94), 33 + Random(0, 94), 33 + Random(0, 94), 33 + Random(0, 94));
                break;
            case MAPEVENT_BONUS_RAINBOWS:
                AreaText = "\C[Grey]Detected: Internal Software Failure (\C[Red]R\C[Orange]A\C[Yellow]I\C[Green]N\C[Cyan]B\C[Blue]O\C[Purple]W\C[Brick]S\C[Grey])\n    \C[Brick]Kupo?";
                break;
            case MAPEVENT_SKILL_TECHNOPHOBIA:
                AreaText = "\C[Red]Detected: Internal Mechanism Corruption (Technophobia!)";
                break;
            case MAPEVENT_SKILL_ARMAGEDDON:
                AreaText = "\C[Grey]Detected: Internal Mechanism Corruption (Armageddon!)";
                break;
            case MAPEVENT_SPECIAL_SINSTORM:
                AreaText = "\C[Red]The source of the demon invasion is located here (Sinstorm)";
                break;
            }

            int MonsterAvgLevel = 0;
            int MonsterMinLevel = 0;
            int MonsterMaxLevel = 0;

            if (1)
            {
                // Calculate Map Level Modifier
                fixed LevelNum = TeleDest->LevelNum;
                fixed LevelMax = GetCVar("drpg_ws_use_wads") * 32.0;
                fixed MapLevelMod = LevelNum / (LevelMax / (2.0 - (LevelNum / LevelMax)));
                fixed Modifier = 1.0;
                if (MapLevelMod > 0.5)
                    MapLevelMod = LevelNum / LevelMax + (0.3 - (0.3 * LevelNum / LevelMax));
                if (MapLevelMod > 1.0 || LevelNum / LevelMax > 1.0)
                    MapLevelMod = 1.0;
                if (GetCVar("drpg_ws_use_wads") < 4)
                    Modifier = 4.0 / GetCVar("drpg_ws_use_wads") * MapLevelMod;
                if (Modifier < 1.0)
                    Modifier = 1.0;

                int MonsterLevelType = GetCVar("drpg_monster_levels");
                fixed MonsterLevelWeight = GetCVarFixed("drpg_monster_level_weight");
                fixed MonsterMapWeight = GetCVarFixed("drpg_monster_map_weight");
                fixed MonsterRandomMinWeight = GetCVarFixed("drpg_monster_random_min_mult");
                fixed MonsterRandomMaxWeight = GetCVarFixed("drpg_monster_random_max_mult");
                int MonsterLevelNum = TeleDest->LevelNum * Modifier;

                // Calculate Monster Level
                if (MonsterLevelType == 1 || MonsterLevelType == 3) // Player Level
                    MonsterAvgLevel += (int)((fixed)AveragePlayerLevel() * MonsterLevelWeight);
                if (MonsterLevelType == 2 || MonsterLevelType == 3) // Map Number
                    MonsterAvgLevel += (int)((fixed)MonsterLevelNum * MonsterMapWeight);

                MonsterMinLevel = MonsterAvgLevel * MonsterRandomMinWeight;
                MonsterMaxLevel = MonsterAvgLevel * MonsterRandomMaxWeight;

                if (MonsterMinLevel <= 0) MonsterMinLevel = 1;
                if (MonsterMaxLevel <= 0) MonsterMaxLevel = 1;
            }

            SetFont("BIGFONT");
            HudMessage("Area Status");
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 9, "Green", X, Y + Y1 + 120.1, 0.05, 0.025);
            SetFont("SMALLFONT");
            HudMessage("Monster Level (Approx.): %d - %d", MonsterMinLevel, MonsterMaxLevel);
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 10, "White", X, Y + Y1 + 136.1, 0.05, 0.025);
            HudMessage("%S", AreaText);
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 11, "White", X, Y + Y1 + 152.1, 0.05, 0.025);
        }
        else if (!TeleDest->NeedsRealInfo && TeleDest->UACBase)
        {
            str AreaText = "No anomalies detected";

            if (PowerOut)
                AreaText = "\C[Red]Detected: Electrical Grid Malfunction";
            else if (Invasion || MarinesHostile)
                AreaText = "\C[Brick]Detected: Security Breach";
            HudMessage("This is a secure UAC military base.");
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 4, "DarkGreen", X, Y + Y1 + 64.1, 0.05, 0.025);
            SetFont("BIGFONT");
            HudMessage("Area Status");
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 9, "Green", X, Y + Y1 + 120.1, 0.05, 0.025);
            SetFont("SMALLFONT");
            HudMessage("%S", AreaText);
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 10, "White", X, Y + Y1 + 136.1, 0.05, 0.025);
        }
        else if (!TeleDest->NeedsRealInfo && TeleDest->UACArena)
        {
            HudMessage("This is a known UAC battle arena site.");
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 4, "LightBlue", X, Y + Y1 + 64.1, 0.05, 0.025);
        }

        // Input
        if (CheckInput(BT_FORWARD, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            if (LevelChoice < KnownLevels->Position - 1)
                LevelChoice++;
            else
                LevelChoice = 0;
        }
        if (CheckInput(BT_BACK, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            if (LevelChoice > 0)
                LevelChoice--;
            else
                LevelChoice = KnownLevels->Position - 1;
        }

        // Map packs support
        if (MapPacks)
        {
            if ((CheckInput(BT_MOVELEFT, KEY_PRESSED, false, PlayerNumber())) && (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber())))
            {
                int MapPack = Player.SelectedMapPack;
                do
                {
                    MapPack--;
                    if (MapPackActive[MapPack])
                    {
                        break;
                    }
                }
                while (MapPack > -1);

                if (MapPack != -1)
                {
                    Player.SelectedMapPack = MapPack;
                    KnownLevels = &ExtraMapPacks[MapPack]; //ah, probably means no mp support this way
                    //will have to move the KnownLevels pointer into the Player Struct
                    //or use a new pointer for outpost text rendering and swap the knownlevels
                    //pointer before changing the map
                    LevelChoice = 1;
                    ActivatorSound("menu/move", 127);
                }
                else
                {
                    ActivatorSound("menu/error", 127);
                }
            }
            else if (CheckInput(BT_MOVELEFT, KEY_ONLYPRESSED, false, PlayerNumber()) && LevelChoice > 0)
            {
                ActivatorSound("menu/move", 127);
                LevelChoice -= 10;
            }

            if ((CheckInput(BT_MOVERIGHT, KEY_PRESSED, false, PlayerNumber())) && (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber())))
            {
                int MapPack = Player.SelectedMapPack;
                do
                {
                    MapPack++;
                    if (MapPackActive[MapPack])
                    {
                        break;
                    }
                }
                while (MapPack < MAX_MAPPACKS);

                if (MapPack != MAX_MAPPACKS)
                {
                    Player.SelectedMapPack = MapPack;
                    KnownLevels = &ExtraMapPacks[MapPack]; //ah, probably means no mp support this way
                    //will have to move the KnownLevels pointer into the Player Struct
                    //or use a new pointer for outpost text rendering and swap the knownlevels
                    //pointer before changing the map
                    LevelChoice = 1;
                    ActivatorSound("menu/move", 127);
                }
                else
                {
                    ActivatorSound("menu/error", 127);
                }
            }
            else if (CheckInput(BT_MOVERIGHT, KEY_ONLYPRESSED, false, PlayerNumber()) && LevelChoice < KnownLevels->Position - 1)
            {
                ActivatorSound("menu/move", 127);
                LevelChoice += 10;
            }
        }
        else
        {
            if (CheckInput(BT_MOVELEFT, KEY_ONLYPRESSED, false, PlayerNumber()) && LevelChoice > 0)
            {
                ActivatorSound("menu/move", 127);
                LevelChoice -= 10;
            }

            if (CheckInput(BT_MOVERIGHT, KEY_ONLYPRESSED, false, PlayerNumber()) && LevelChoice < KnownLevels->Position - 1)
            {
                ActivatorSound("menu/move", 127);
                LevelChoice += 10;
            }
        }
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
        {
            if (CurrentLevel == TeleDest)
            {
                PrintError("You are already at this location.");
                ActivatorSound("menu/error", 127);
                SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
                Player.OutpostMenu = 0;
                return;
            }

            // Special handling for Marine Hostility and Emergency Power
            if (MarinesHostile || PowerOut)
            {
                bool IsTechnician = (CompatMode == COMPAT_DRLA && PlayerClass(PlayerNumber()) == 2);
                int TransportTimerMax = (35 * (5 + GameSkill() + PlayerCount())) / (IsTechnician ? 2 : 1);
                int TransportTimer = TransportTimerMax;

                // Setup power transfer
                SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
                SetPlayerProperty(0, 1, PROP_FROZEN);
                Player.OutpostMenu = 0;
                PlaySound(0, "transfer/loop", CHAN_BODY, 1.0, true, ATTN_NORM);

                // Power transfer loop
                while (TransportTimer > 0)
                {
                    // Calculate percentage complete and input
                    int TransportPercent = (int)Abs(100 - ((fixed)TransportTimer / (fixed)TransportTimerMax) * 100.0);

                    // Transfer is cancelled either by death or input
                    if (GetActorProperty(0, APROP_Health) <= 0 || CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
                    {
                        PlaySound(0, "transfer/fail", CHAN_BODY, 1.0, false, ATTN_NORM);
                        SetPlayerProperty(0, 0, PROP_FROZEN);
                        return;
                    }

                    // Draw transfer bar
                    DrawProgressBar("Transferring Power", TransportPercent);

                    // Decrement timer
                    TransportTimer--;
                    Delay(1);
                }

                PlaySound(0, "transfer/complete", CHAN_BODY, 1.0, false, ATTN_NORM);
                Delay(10);
            }

            Player.OutpostMenu = 0;

            TransportOutFX(0);

            // Compatibility Handling - DoomRL Arsenal Extended
            if (CompatModeEx == COMPAT_DRLAX)
            {
                NomadModPacksSave();
                NanomaniacTransport();
            }

            Delay(35 * 2);

            SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
            SetPlayerProperty(0, 0, PROP_FROZEN);
            Transported = true;

            if (BossDead)
                OutpostNotoriety++;

            ChangeLevel(TeleDest->LumpName, 0, CHANGELEVEL_NOINTERMISSION, -1);

            break;
        }
        //changing exit menu to menu key from sprint - the code is in Menu.c
        /* if (Buttons == BT_SPEED)
        {
            SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
            Player.OutpostMenu = 0;
            return;
        } */
        if (Player.OutpostMenu == 0)
            return;

        Delay(1);
    }
}

NamedScript MapSpecial void ArenaTransport()
{
    // if you're already in a menu, terminate
    if (Player.InMenu || Player.InShop || Player.OutpostMenu == OMENU_ARENATRANSPORT) return;

    // If Marines are hostile or the power is out, terminate
    if (MarinesHostile || PowerOut) return;

    ActivatorSound("menu/move", 127);
    Player.OutpostMenu = OMENU_ARENATRANSPORT;

    // Main coordinates
    fixed X = 0.1;
    fixed Y = 0.1;

    // Additional coordinates
    fixed X1, Y1, Y2;

    // Arena Data
    int CurrentArena;
    str ArenaLumpNames[2] =
    {
        "DAM01",
        "DAM02"
    };
    str ArenaNiceNames[2] =
    {
        "Training Ground",
        "Throne Room"
    };
    str ArenaDescription[2] =
    {
        "Arena from DoomRPG Sumwunn Edition",
        "Arena from Obsidian's contributor"
    };
    str ArenaAuthor[2] =
    {
        "Sumwunn",
        "Demiosis"
    };

    // So the player's initial interaction is not processed as a menu action
    Delay(1);

    while (Player.OutpostMenu == OMENU_ARENATRANSPORT)
    {
        SetPlayerProperty(0, 1, PROP_TOTALLYFROZEN);

        // Reset additional coordinates
        X1 = 0.0;
        Y1 = 0.0;
        Y2 = 0.0;

        // Draw the background
        if (GetCVar("drpg_menudim"))
            FadeRange(0, 0, 0, 0.65, 0, 0, 0, 0.0, 0.25);

        // Set the HUD Size
        SetHudSize(GetActivatorCVar("drpg_menu_width"), GetActivatorCVar("drpg_menu_height"), true);

        // Draw Border
        // These are pushed back a bit so the border doesn't overlap anything
        if (GetActivatorCVar("drpg_menu_background_border"))
            DrawBorder("Bor", -1, 8, -5.0, 0.0, 470, 470);

        // Text
        SetFont("BIGFONT");
        HudMessage("Arena Transport");
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID, "White", X + 132.0, Y + 16.0, 0.05, 0.05);

        SetFont("BIGFONT");
        HudMessage("\CfSelect arena to transport:\C-");
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 1, "White", X + 64.0, Y + 56.0, 0.05, 0.05);

        SetFont("BIGFONT");
        HudMessage("\CaArena:\C- %S", ArenaNiceNames[CurrentArena]);
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 2, "White", X + 32.0, Y + 96.0, 0.05, 0.05);

        SetFont("SMALLFONT");
        HudMessage("\CdDescription\C-:\n\n%S", ArenaDescription[CurrentArena]);
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 3, "White", X + 32.0, Y + 132.0, 0.05, 0.05);

        SetFont("SMALLFONT");
        HudMessage("\CdAuthor\C-:\n\n%S", ArenaAuthor[CurrentArena]);
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 4, "White", X + 32.0, Y + 164.0, 0.05, 0.05);

        // Input
        if (CheckInput(BT_MOVELEFT, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            CurrentArena--;
            if (CurrentArena < 0) CurrentArena = 1;
        }
        if (CheckInput(BT_MOVERIGHT, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            CurrentArena++;
            if (CurrentArena > 1) CurrentArena = 0;
        }
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
        {
            Player.OutpostMenu = 0;

            TransportOutFX(0);

            // Compatibility Handling - DoomRL Arsenal Extended
            if (CompatModeEx == COMPAT_DRLAX)
            {
                NomadModPacksSave();
                NanomaniacTransport();
            }

            Delay(35 * 2);

            SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
            SetPlayerProperty(0, 0, PROP_FROZEN);
            Transported = true;

            if (BossDead)
                OutpostNotoriety++;

            ChangeLevel(ArenaLumpNames[CurrentArena], 0, CHANGELEVEL_NOINTERMISSION, -1);

            break;
        }

        if (Player.OutpostMenu == 0)
            return;

        Delay(1);
    }
}

NamedScript MapSpecial void SkillComputer()
{
    // If Marines are hostile or the power is out, terminate
    if (MarinesHostile || PowerOut) return;

    // Terminate if you aren't the Arbitrator
    if (InMultiplayer && !Arbitrator) return;

    // if you're already in a menu, terminate
    if (Player.InMenu || Player.InShop || Player.OutpostMenu == OMENU_SKILLCOMPUTER) return;

    ActivatorSound("misc/edgar", 127);

    Player.OutpostMenu = OMENU_SKILLCOMPUTER;
    SetPlayerProperty(0, 1, PROP_TOTALLYFROZEN);

    // So the player's initial interaction is not processed as a menu action
    Delay(1);

    while (Player.OutpostMenu == OMENU_SKILLCOMPUTER)
    {
        // Draw the background
        if (GetCVar("drpg_menudim"))
            FadeRange(0, 0, 0, 0.65, 0, 0, 0, 0.0, 0.25);

        // Set the HUD Size
        SetHudSize(GetActivatorCVar("drpg_menu_width"), GetActivatorCVar("drpg_menu_height"), true);

        // Draw Border
        // These are pushed back a bit so the border doesn't overlap anything
        if (GetActivatorCVar("drpg_menu_background_border"))
            DrawBorder("Bor", -1, 8, -5.0, 0.0, 470, 470);

        // Text
        SetFont("BIGFONT");
        if (CompatMonMode == COMPAT_DRLA)
            HudMessage("Skill Level: \Cj%d (%S\Cj)", SkillChoice + 1, SkillLevelsDRLA[SkillChoice]);
        else
            HudMessage("Skill Level: \Cj%d (%S\Cj)", SkillChoice + 1, SkillLevelsDF[SkillChoice]);
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID, "Gold", 100.1, 200.0, 0.05, 0.5);

        // Input
        if (CheckInput(BT_FORWARD, KEY_ONLYPRESSED, false, PlayerNumber()) && SkillChoice > 0)
        {
            ActivatorSound("menu/move", 127);
            SkillChoice--;
        }
        if (CheckInput(BT_BACK, KEY_ONLYPRESSED, false, PlayerNumber()) && SkillChoice < SkillLevelsMax - 1)
        {
            ActivatorSound("menu/move", 127);
            SkillChoice++;
        }
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
        {
            SetSkill(SkillChoice);
            SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
            Player.OutpostMenu = 0;
            return;
        }
        if (CheckInput(BT_SPEED, KEY_ONLYHELD, false, PlayerNumber()))
        {
            SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
            Player.OutpostMenu = 0;
            return;
        }

        Delay(1);
    }
}

NamedScript MapSpecial void ToggleArena()
{
    // If Marines are hostile or the power is out, terminate
    if (MarinesHostile || PowerOut) return;

    // If you try to toggle the Arena and you aren't the Arena Arbitrator, terminate
    if (InMultiplayer && (ArenaPlayerNumber >= 0 && PlayerNumber() != ArenaPlayerNumber)) return;

    ArenaActive = !ArenaActive;

    SetFont("BIGFONT");

    if (ArenaActive)
    {
        ActivatorSound("arena/activate", 127);
        HudMessage("Arena Active");
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID, "Green", 0.5, 0.5, 2.0, 1.0);
        ArenaPlayerNumber = PlayerNumber();
        ArenaSetMusic();
    }
    else
    {
        ActivatorSound("arena/deactivate", 127);
        HudMessage("Arena Inactive");
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID, "Red", 0.5, 0.5, 2.0, 1.0);
        ArenaPlayerNumber = -1;
        SetOutpostMusic((PowerOut ? OUTPOST_MUSIC_LOWPOWER : OUTPOST_MUSIC_NORMAL));
    }
}

NamedScript MapSpecial void PassArenaLine()
{
    // If Marines are hostile or the power is out, terminate
    if (MarinesHostile || PowerOut) return;

    if (InMultiplayer && (ArenaPlayerNumber >= 0 && PlayerNumber() != ArenaPlayerNumber)) return;

    if (ArenaActive)
    {
        Ceiling_LowerToFloor(99, 64);
        ArenaLoop();
    }
}

NamedScript MapSpecial void SelectArenaWave()
{
    // If Marines are hostile or the power is out, terminate
    if (MarinesHostile || PowerOut) return;

    // if you're already in a menu, terminate
    if (Player.OutpostMenu == OMENU_MODULECONVERTER) return;

    if (InMultiplayer && (ArenaPlayerNumber >= 0 && PlayerNumber() != ArenaPlayerNumber)) return;

    ActivatorSound("menu/move", 127);
    Player.OutpostMenu = OMENU_WAVESELECTOR;

    WaveChoice = ArenaMaxWave;

    // So the player's initial interaction is not processed as a menu action
    Delay(1);

    while (Player.OutpostMenu == OMENU_WAVESELECTOR)
    {
        SetPlayerProperty(0, 1, PROP_TOTALLYFROZEN);

        // Draw the background
        if (GetCVar("drpg_menudim"))
            FadeRange(0, 0, 0, 0.65, 0, 0, 0, 0.0, 0.25);

        // Set the HUD Size
        SetHudSize(GetActivatorCVar("drpg_menu_width"), GetActivatorCVar("drpg_menu_height"), true);

        // Draw Border
        // These are pushed back a bit so the border doesn't overlap anything
        if (GetActivatorCVar("drpg_menu_background_border"))
            DrawBorder("Bor", -1, 8, -5.0, 0.0, 470, 470);

        // Text
        SetFont("BIGFONT");
        HudMessage("Wave: \Cd%d\C-/\Cd%d", WaveChoice, ArenaMaxWave);
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID, "White", 200.0, 200.0, 0.05, 1.0);

        // Input
        if (CheckInput(BT_FORWARD, KEY_ONLYPRESSED, false, PlayerNumber()) && WaveChoice > 1)
        {
            ActivatorSound("menu/move", 127);
            WaveChoice--;
        }
        if (CheckInput(BT_BACK, KEY_ONLYPRESSED, false, PlayerNumber()) && WaveChoice < ArenaMaxWave)
        {
            ActivatorSound("menu/move", 127);
            WaveChoice++;
        }
        if (CheckInput(BT_MOVELEFT, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            WaveChoice -= 10;
            if (WaveChoice <= 0) WaveChoice = 1;
        }
        if (CheckInput(BT_MOVERIGHT, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            WaveChoice += 10;
            if (WaveChoice > ArenaMaxWave) WaveChoice = ArenaMaxWave;
        }
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
        {
            if (WaveChoice > 1)
                ArenaWave = WaveChoice - 1;
            else
                ArenaWave = 1;

            ActivatorSound("menu/move", 127);
            SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
            Player.OutpostMenu = 0;
            return;
        }
        if (CheckInput(BT_SPEED, KEY_ONLYHELD, false, PlayerNumber()))
        {
            SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
            Player.OutpostMenu = 0;
            return;
        }

        Delay(1);
    }
}

NamedScript MapSpecial void PissOffMarines(bool Steal)
{
    int ForcefieldState;

    // If the Marines are already hostile, terminate
    if (MarinesHostile) return;

    // Only in a base
    if (!CurrentLevel->UACBase) return;

    // Set flag
    MarinesHostile = true;

    // Demotion
    if (Player.RankLevel > 0 && Steal)
        Player.Rank -= Player.Rank + RankTable[Player.RankLevel - 1];

    // Iterate Marines pre-Delay
    if (Steal)
        for (int i = MarineTID; i <= MarineMaxTID; i++)
        {
            // Sight Alert
            if (CheckSight(i, 0, 0))
            {
                PlaySound(i, "drpgmarines/shopkeephatred", 0, 1.0, false, ATTN_NORM);
                SpawnForced("DRPGAlertIcon", GetActorX(i), GetActorY(i), GetActorZ(i) + GetActorPropertyFixed(i, APROP_Height) + 4.0, 0, 0);
            }
        }

    Delay(35 * 2);

    // Combat music
    SetOutpostMusic((PowerOut ? OUTPOST_MUSIC_LOWPOWER_COMBAT : OUTPOST_MUSIC_COMBAT));

    // Iterate Marines post-Delay
    for (int i = MarineTID; i <= MarineMaxTID; i++)
    {
        // Remove Friendly flag
        if (!InTitle)
            SetActorProperty(i, APROP_Friendly, false);

        // Enrage Marines
        if (GetActorProperty(i, APROP_Health) > 0 && !InTitle)
            SetActorState(i, "Enraged", false);

        // Give Credits to Marines
        SetActorInventory(i, "DRPGCredits", Random(10, 1000));
    }

    // Enrage placeholder bosses
    if (!InTitle)
        SetActorState(MarineBossTID - 1, "Enraged", false);

    // Drop the Credits room blocker
    Ceiling_LowerToFloor(CreditsBlockerID, 256);

    // Start the Alarm loop
    AlarmLoop();

    // Boss Spawning
    if (!InTitle) // These guys were absolutely massacring the monsters, ruining the cinematics
        SpawnBoss();

    // Raise Entry Forcefield
    if (!InTitle)
    {
        Line_SetBlocking(ForcefieldGeneratorID, BLOCKF_EVERYTHING | BLOCKF_SIGHT | BLOCKF_HITSCAN, 0);
        SetLineTexture(ForcefieldGeneratorID, SIDE_FRONT, TEXTURE_MIDDLE, "FORCEFIE");
        SpawnSpotForced("TeleportFog", GeneratorTID, 0, 0);
        SpawnSpotForced("DRPGForcefieldGenerator", GeneratorTID, GeneratorTID, 0);
    }

    // Title Map Handling
    if (InTitle)
    {
        // Create Hell Rifts
        SpawnSpotForced("DRPGTeleportRift", RiftSpotTID, RiftSpotTID, 0);

        // Alert the Marines
        for (int i = MarineTID; i <= MarineMaxTID; i++)
            SetActorState(i, "See", false);
    }

    // Synchronize delay with SpawnBoss()
    if (OutpostNotoriety >= 3)
        Delay(35 * 30);

    // Loop
    while (true)
    {
        // Move all players outside the entry sector
        if (!BossDead && !InTitle)
            for (int i = 0; i < MAX_PLAYERS; i++)
                if (ThingCountSector(0, Players(i).TID, EntrySectorID) > 0)
                    TeleportOther(Players(i).TID, HallTeleportSpotID + i, true);

        // Spawn Reinforcements
        if ((Timer() % (35 * 10)) == 0 && OutpostNotoriety <= 2)
        {
            SpawnSpotForced("TeleportFog", MarineSpotTID, 0, 0);

            if (InTitle)
            {
                for (int i = HallTeleportSpotID; i < HallTeleportSpotID + 8; i++)
                {
                    SpawnSpotForced("TeleportFog", i, 0, 0);
                    if (CompatMonMode == COMPAT_DRLA)
                        SpawnSpotFacingForced("DRPGMarineReinforcementDRLASpawner", i, MarineTID);
                    else
                        SpawnSpotFacingForced("DRPGMarineReinforcementSpawner", i, MarineTID);
                }
            }
            else
            {
                if (OutpostNotoriety == 2) // Special handling for Chasing Minigunners
                    SpawnSpotFacingForced("DRPGMarineMinigunHuntingYerAss", MarineSpotTID, MarineTID);
                else if (CompatMonMode == COMPAT_DRLA)
                    SpawnSpotFacingForced("DRPGMarineReinforcementDRLASpawner", MarineSpotTID, MarineTID);
                else
                    SpawnSpotFacingForced("DRPGMarineReinforcementSpawner", MarineSpotTID, MarineTID);

                if (!InTitle)
                    SetActorProperty(MarineTID, APROP_Friendly, false);
            }
        }

        // Make the Marines fight enemies in title map
        if (InTitle)
        {
            for (int i = MarineTID; i <= MarineMaxTID; i++)
            {
                SetActorProperty(i, APROP_Friendly, true);
                Thing_ChangeTID(i, MarineTID);
            }

            SetActorProperty(MarineBossTID, APROP_Friendly, true);
            Thing_Hate(RiftSpotTID + 1, MarineTID, 6);
        }

        // Boss Timer
        if (ForcefieldTimer > 0 && OutpostNotoriety == 2)
        {
            // Set completion state on the Shield Generator
            SetActorState(GeneratorTID, StrParam("%dPowerLoop", ForcefieldTimer / ((35 * 60 * GameSkill()) / 7)), false);

            // Decrement timer
            ForcefieldTimer--;
        }

        // Different checks for specific Notoriety levels
        if (ForcefieldState == 0)
            if (OutpostNotoriety == 2 && ForcefieldTimer <= 0)
                ForcefieldState = 1;
            else if (OutpostNotoriety != 2 && ThingCount(0, MarineBossTID) <= 0)
                ForcefieldState = 1;

        // Bosses are dead or boss timer expires
        if (ForcefieldState == 1)
        {
            AmbientSound("misc/poweroff", 127);

            // Disable Entry Forcefield
            Line_SetBlocking(ForcefieldGeneratorID, 0, BLOCKF_EVERYTHING | BLOCKF_SIGHT | BLOCKF_HITSCAN);
            SetLineTexture(ForcefieldGeneratorID, SIDE_FRONT, TEXTURE_MIDDLE, "");
            SetActorState(GeneratorTID, "Death", false);

            // Boss is dead
            BossDead = true;
            ForcefieldState = 2;
        }

        Delay(1);
    }
}

// TODO
NamedScript MapSpecial void ModuleConverter()
{
    // If Marines are hostile or the power is out, terminate
    if (MarinesHostile || PowerOut) return;

    // if you're already in a menu, terminate
    if (Player.InMenu || Player.InShop || Player.OutpostMenu == OMENU_MODULECONVERTER) return;
}

// Credit Room Script
NamedScript MapSpecial void CreditRoom(int ID)
{
    if (ID == 1) // Enter
        SetOutpostMusic(OUTPOST_MUSIC_CREDITS);
    if (ID == 2) // Exit
        SetOutpostMusic((PowerOut ? OUTPOST_MUSIC_LOWPOWER : OUTPOST_MUSIC_NORMAL));

    if (ID == 3) // Kyle873 - That's me!
    {
        SetFont("BIGFONT");
        HudMessage("Kyle873");
        EndHudMessage(HUDMSG_FADEOUT, 0, "Green", 0.5, 0.5, 3.0, 2.0);
        Delay(35);
        SetFont("SMALLFONT");
        HudMessage("What do you mean I can't nerf XP Gain to 1 per monster?!");
        EndHudMessage(HUDMSG_FADEOUT, 0, "White", 0.5, 0.55, 3.0, 2.0);

        int RealCredits = CheckInventory("DRPGCredits");

        ActivatorSound("credits/payout", 127);
        Log("\CkYou have been paid -2147483648 by the UAC!");
        TakeInventory("DRPGCredits", RealCredits);
        Delay(35 * 10);
        GiveInventory("DRPGCredits", RealCredits);
    }

    if (ID == 4) // Lord Misfit
    {
        SetFont("BIGFONT");
        HudMessage("Lord Misfit");
        EndHudMessage(HUDMSG_FADEOUT, 0, "Green", 0.5, 0.5, 3.0, 2.0);
        Delay(35);
        SetFont("SMALLFONT");
        HudMessage("I swear it's a real bug this time!");
        EndHudMessage(HUDMSG_FADEOUT, 0, "White", 0.5, 0.55, 3.0, 2.0);

        str VarString = "SetVar";

        Delay(35 * 3);
        for (int i = 0; i < 100; i++)
        {
            if (Random(1, 3) == 1) VarString = StrParam("%S%S", VarString, "Var");
            HudMessage("%S", VarString);
            EndHudMessage(HUDMSG_FADEOUT, 0, ColorNames[Random(1, 25)], RandomFixed(0.0, 1.0), RandomFixed(0.0, 1.0), 3.0, 2.0);
            Delay(1);
        }
    }

    if (ID == 5) // Ryan Cordell
    {
        SetFont("BIGFONT");
        HudMessage("Ryan Cordell");
        EndHudMessage(HUDMSG_FADEOUT, 0, "Green", 0.5, 0.5, 7.0, 2.0);
        Delay(35);

        for (int i = 0; i < 10; i++)
        {
            ActivatorSound("weapons/rocklx", 127);
            Delay(Random(5, 10));
        }

        Delay(35 * 2);
        SetFont("SMALLFONT");
        HudMessage("Did I do that?");
        EndHudMessage(HUDMSG_FADEOUT, 0, "White", 0.5, 0.55, 3.0, 2.0);
    }

    if (ID == 6) // marrub
    {
        SetFont("BIGFONT");
        HudMessage("marrub");
        EndHudMessage(HUDMSG_FADEOUT, 0, "Green", 0.5, 0.5, 3.0, 2.0);
        Delay(35);
        HudMessage("I HOPE YOU LIKE SHOTGUNS!");
        EndHudMessage(HUDMSG_FADEOUT, 0, "Brick", 0.5, 0.55, 3.0, 2.0);
        Delay(35 * 2);

        for (int i = 0; i < 50; i++)
        {
            DropItem(0, "DRPGDumbShotgun", 1, 255);
            Delay(1);
        }
    }

    if (ID == 7) // Kate
    {
        SetFont("BIGFONT");
        HudMessage("Kate");
        EndHudMessage(HUDMSG_FADEOUT, 0, "Green", 0.5, 0.5, 3.0, 2.0);
        Delay(35);
        HudMessage("\CgR\CiA\CkI\CdN\ChB\CtO\CaW\CjS");
        EndHudMessage(HUDMSG_FADEOUT, 0, "White", 0.5, 0.55, 3.0, 2.0);
        Delay(35);

        FadeTo(255, 0, 0, 0.5, 0.5);
        Delay(17);
        FadeTo(255, 128, 0, 0.5, 0.5);
        Delay(17);
        FadeTo(255, 255, 0, 0.5, 0.5);
        Delay(17);
        FadeTo(0, 255, 0, 0.5, 0.5);
        Delay(17);
        FadeTo(0, 0, 255, 0.5, 0.5);
        Delay(17);
        FadeTo(128, 0, 255, 0.5, 0.5);
        Delay(17);
        FadeTo(255, 0, 255, 0.5, 0.5);
        Delay(17);
        FadeTo(255, 255, 255, 0.5, 0.5);
        Delay(17);
        FadeTo(0, 0, 0, 0.0, 0.5);
    }

    if (ID == 8) // Yholl
    {
        SetFont("BIGFONT");
        HudMessage("Yholl");
        EndHudMessage(HUDMSG_FADEOUT, 0, "Green", 0.5, 0.5, 3.0, 2.0);
        Delay(35);
        HudMessage("\CaYour suffering pleases me");
        EndHudMessage(HUDMSG_FADEOUT, 0, "White", 0.5, 0.55, 3.0, 2.0);
        Delay(35);

        for (int i = 0; i < 50; i++)
        {
            DropItem(0, "DRPGDumbPistol", 1, 255);
            Delay(1);
        }
    }
}

// Shop Special UI
NamedScript MapSpecial void ShopSpecial()
{
    // if you're already in a menu, terminate
    if (Player.InMenu || Player.InShop || Player.OutpostMenu == OMENU_SHOPSPECIAL) return;

    // If there is no shop special
    if (ShopSpecialItem == GetBlankItem())
    {
        ActivatorSound("menu/error", 127);
        PrintError(StrParam("There is currently no Shop Special item.\n\nNext restock will be in \Cj%S\C-.", FormatTime(ShopSpecialTimer)));
        return;
    }

    // If the shop special was already bought
    if (ShopSpecialBought)
    {
        ActivatorSound("menu/error", 127);
        PrintError(StrParam("Shop Special is currently out of stock.\n\nNext restock will be in \Cj%S\C-.", FormatTime(ShopSpecialTimer)));
        return;
    }

    ActivatorSound("menu/move", 127);
    SetPlayerProperty(0, 1, PROP_TOTALLYFROZEN);
    Player.OutpostMenu = OMENU_SHOPSPECIAL;

    // So the player's initial interaction is not processed as a menu action
    Delay(1);

    while (Player.OutpostMenu == OMENU_SHOPSPECIAL)
    {
        str Name = ShopSpecialItem->Name;
        int Discount = GetCVar("drpg_shopspecial_discount") + Player.ShopDiscount;
        // Cap the Shop Special Discount at 75%
        if (Discount > 75)
            Discount = 75;
        int Cost = ShopSpecialItem->Price - ((ShopSpecialItem->Price * Discount) / 100);

        // If the item's already been bought or the timer expires, terminate
        if (ShopSpecialBought || ShopSpecialTimer <= 0)
        {
            SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
            Player.OutpostMenu = 0;
            return;
        }

        // The cost should always be at least 1 Credit
        if (Cost <= 0) Cost = 1;

        // Draw the background
        if (GetCVar("drpg_menudim"))
            FadeRange(0, 0, 0, 0.65, 0, 0, 0, 0.0, 0.25);

        // Input
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
        {
            // Buy Item
            if (CheckInventory("DRPGCredits") >= Cost)
            {
                // DoomRL Compatibility

                TakeInventory("DRPGCredits", Cost);
                SpawnForced(ShopSpecialItem->Actor, GetActorX(0), GetActorY(0), GetActorZ(0), 0, 0);
                SetActorVelocity(Player.TID, 0.01, 0.01, 0, true, false);
                ShopSpecialBought = true;

                ActivatorSound("credits/payout", 127);
                Delay(1);
                SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
                Player.OutpostMenu = 0;
                return;
            }
            else
            {
                PrintError("You don't have enough Credits to buy this item");
                ActivatorSound("menu/error", 127);
            }
        }

        // Drawing
        SetHudSize(0, 0, false);
        SetFont("BIGFONT");
        HudMessage("%S\n\Ck%d C (Discount: %d%%)\n\CdTime Left: %S",
                   Name, Cost, Discount, FormatTime(ShopSpecialTimer));
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID, "White", 1.5, 0.5, 0.05, 1.0);

        Delay(1);
    }
}

// Mission BBS
NamedScript MapSpecial void MissionBBS()
{
    int Index;
    int Difficulty = (Player.RankLevel - 1) / 2.0;
    if (Difficulty >= MAX_DIFFICULTIES - 1)
        Difficulty = MAX_DIFFICULTIES - 1;

    // If Marines are hostile, terminate
    if (MarinesHostile) return;

    // if you're already in a menu, terminate
    if (Player.InMenu || Player.InShop || Player.OutpostMenu == OMENU_BBS) return;

    // You need at least 1 Rank to get a mission
    if (Player.RankLevel == 0)
    {
        PrintError("You cannot accept any missions until you reach the first rank");
        ActivatorSound("menu/error", 127);
        return;
    }

    // Make sure the Rewards List is finished Processing
    if (!RewardsInit || !MissionInit)
    {
        PrintError("The Mission BBS is currently processing requests...");
        ActivatorSound("menu/error", 127);
        return;
    }

    // Freeze the Player
    SetPlayerProperty(0, 1, PROP_TOTALLYFROZEN);

    Player.OutpostMenu = OMENU_BBS;

    ActivatorSound("menu/move", 127);

    // So the player's initial interaction is not processed as a menu action
    Delay(1);

    while (Player.OutpostMenu == OMENU_BBS)
    {
        MissionInfo *Mission = &Missions[Difficulty][Index];

        // Set the HUD Size
        SetHudSize(GetActivatorCVar("drpg_menu_width"), GetActivatorCVar("drpg_menu_height"), true);

        // Draw Border
        // These are pushed back a bit so the border doesn't overlap anything
        if (GetActivatorCVar("drpg_menu_background_border"))
            DrawBorder("Bor", -1, 8, -5.0, 0.0, 500, 470);

        // Title
        SetFont("BIGFONT");
        HudMessage("\CdMission BBS\n\CjDifficulty: (%S\C-)", MissionDifficulties[Difficulty]);
        EndHudMessage(HUDMSG_PLAIN, 0, "White", 0.1, 24.1, 0.05);

        // Mission Labels
        for (int i = MAX_MISSIONS - 1; i >= 0; i--)
        {
            MissionInfo *MissionIter = &Missions[Difficulty][i];

            // Cursor
            if (i == Index)
                PrintSprite("MissBoxH", 0, 0.1 + (i % (MAX_MISSIONS / 3) * 48.0) + RoundInt((Cos((fixed)Timer() / 64.0) * 8.0)), 80.0 + (i / (MAX_MISSIONS / 3) * 48.0) + RoundInt((Sin((fixed)Timer() / 64.0) * 8.0)), 0.05);

            // Icon
            PrintSprite(StrParam("BBS_M%d", MissionIter->Type + 1), 0, 8.1 + (i % (MAX_MISSIONS / 3) * 48.0), 80.0 + (i / (MAX_MISSIONS / 3) * 48.0), 0.05);

            // Box
            PrintSprite("MissBoxB", 0, 0.1 + (i % (MAX_MISSIONS / 3) * 48.0), 80.0 + (i / (MAX_MISSIONS / 3) * 48.0), 0.05);
        }

        // Currently selected Mission
        DrawMissionInfo(Mission, 0, 216, false);

        // Input
        if (CheckInput(BT_FORWARD, KEY_REPEAT, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            Index -= MAX_MISSIONS / 3;
            if (Index < 0)
                Index = 0;
        };
        if (CheckInput(BT_BACK, KEY_REPEAT, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            Index += MAX_MISSIONS / 3;
            if (Index > MAX_MISSIONS - 1)
                Index = MAX_MISSIONS - 1;
        };
        if (CheckInput(BT_MOVELEFT, KEY_REPEAT, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
            {
                if (Difficulty > 0)
                    Difficulty--;
            }
            else
            {
                Index--;
                if (Index < 0)
                    Index = 0;
            }
        };
        if (CheckInput(BT_MOVERIGHT, KEY_REPEAT, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
            {
                if (Difficulty < (Player.RankLevel / 2.0) - 1)
                {
                    if (Difficulty >= MAX_DIFFICULTIES - 1)
                        Difficulty = MAX_DIFFICULTIES - 1;
                    else
                        Difficulty++;
                }
                else
                    ActivatorSound("menu/error", 127);
            }
            else
            {
                Index++;
                if (Index > MAX_MISSIONS - 1)
                    Index = MAX_MISSIONS - 1;
            }
        };
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()) && !Player.Mission.Active)
        {
            ActivatorSound("mission/get", 127);
            Mission->Active = true;
            Player.Mission = *Mission;
            Player.Mission.Current = 0;
            CreateMissionAt(Difficulty, Index);
        };

        if (CheckInput(BT_ATTACK, KEY_ONLYPRESSED, false, PlayerNumber()) && Player.Mission.Active)
        {
            ActivatorSound("mission/fail", 127);
            SetFont("BIGFONT");
            SetHudSize(640, 480, false);
            HudMessage("Mission Aborted!");
            EndHudMessage(HUDMSG_FADEOUT, MISSION_ID, "Red", 320.4, 400.0, 3.0, 2.0);
            ClearMission();
        };

        Delay(1);
    }
}

// Roulette
NamedScript MapSpecial void MinigameHandler(bool Rare)
{
    // Return if already in a minigame
    if (Player.InMenu || Player.InShop || Player.InMinigame) return;

    SetPlayerProperty(0, 1, PROP_TOTALLYFROZEN);

    // TODO: This will just play Roulette for now
    if (Rare)
    {
        if (CheckInventory("DRPGChipPlatinum") > 0)
        {
            SetFont("BIGFONT");
            HudMessage("Rare Item Roulette");
            EndHudMessage(HUDMSG_FADEOUT, 0, "Gold", 0.5, 0.75, 0.75, 0.25);
            Delay(35);
            ItemRoulette(true);
        }
        else
        {
            PrintError("You need a \CjPlatinum Chip\C- to use the Rare Item Roulette");
            ActivatorSound("menu/error", 127);
            SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
        }
    }
    else
    {
        if (CheckInventory("DRPGChipGold") > 0)
        {
            SetFont("BIGFONT");
            HudMessage("Item Roulette");
            EndHudMessage(HUDMSG_FADEOUT, 0, "Gold", 0.5, 0.75, 0.75, 0.25);
            Delay(35);
            ItemRoulette(false);
        }
        else
        {
            SetFont("BIGFONT");
            PrintError("You need a \CfGold Chip\C- to use the Item Roulette");
            ActivatorSound("menu/error", 127);
            SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
        }
    }
}

// Handles the 6 Key Secret
NamedScript MapSpecial void OutpostSecret()
{
    if (CheckInventory("DRPGRedCard") && CheckInventory("DRPGYellowCard") && CheckInventory("DRPGBlueCard") &&
            CheckInventory("DRPGRedSkull") && CheckInventory("DRPGYellowSkull") && CheckInventory("DRPGBlueSkull"))
    {
        ActivatorSound("misc/secret", 127);
        Teleport(SecretTeleportTID + PlayerNumber(), false);
    }
}

NamedScript MapSpecial void PowerOutage()
{
    // If the power is already out, terminate
    if (PowerOut) return;

    Delay(10);

    PowerOut = true;

    // Blackout
    if (!MarinesHostile)
    {
        SetMusic("");
        ActivatorSound("misc/poweroff", 127);
        for (int i = 0; i < MAX_OUTPOST_ID; i++)
            Light_Fade(i, 0, 10);
    }

    // Disable Shop Forcefields
    Line_SetBlocking(ForcefieldID, 0, 1023);
    SetLineTexture(ForcefieldID, SIDE_FRONT, TEXTURE_MIDDLE, "");

    // Randomized delay for the power to come back on
    Delay(Random(35 * 3, 35 * 5));

    // Enable Emergency Power
    if (!MarinesHostile)
    {
        SetOutpostMusic(OUTPOST_MUSIC_LOWPOWER);
        ActivatorSound("misc/poweron", 127);

        for (int i = 0; i < MAX_OUTPOST_ID; i++)
        {
            Sector_SetColor(i, 255, 0, 0, 0);
            Light_Fade(i, 160, 60);
        }
    }
}

NamedScript MapSpecial void OperatingCapsule()
{
    // if you're already in a menu, terminate
    if (Player.InMenu || Player.InShop || Player.OutpostMenu == OMENU_OPERATINGCAPSULE) return;

    // If Marines are hostile or the power is out, terminate
    if (MarinesHostile || PowerOut) return;

    // If don't use an DoomRL Arsenal, terminate
    if (CompatMode != COMPAT_DRLA)
    {
        PrintError("The Operating Capsule\nis not working right now");
        ActivatorSound("menu/error", 127);
        return;
    }

    // Close the door and change the color of the room
    Door_Close(19, 32, 13);
    Sector_SetColor(20, 200, 50, 50, 0);

    ActivatorSound("menu/move", 127);
    Player.OutpostMenu = OMENU_OPERATINGCAPSULE;

    fixed X = 0.1;
    fixed Y = 0.1;

    int OperationChoice;
    int Price;

    str ArmorNames[12] =
    {
        "Cybernano \CdSecurity\C- Armor",
        "Cybernano \CnCombat\C- Armor",
        "Cybernano \CaCommando\C- Armor",
        "Nanofiber Skin \CdSecurity\C- Armor",
        "Nanofiber Skin \CnCombat\C- Armor",
        "Nanofiber Skin \CaCommando\C- Armor",
        "\CqCybernetic\C- Armor",
        "\CaCyberwarrior\C- Armor",
        "\CaCyberwarrior\C- Armor \Cc[Onyx]\C-",
        "\CgBerserk\C- Powersuit",
        "\CuR11-n Psychic Amplifier Suit\C-",
        "\CrSoulshatter\C- Armor"
    };

    str ArmorActors[12] =
    {
        "RLCybernanoGreenArmor",
        "RLCybernanoBlueArmor",
        "RLCybernanoRedArmor",
        "RLNanofiberSkinGreenArmor",
        "RLNanofiberSkinBlueArmor",
        "RLNanofiberSkinRedArmor",
        "RLCyberneticArmor",
        "RLCyberwarriorArmor",
        "RLOModCyberwarriorArmor",
        "RLBerserkPowersuitArmor",
        "RLRyanCordellPsychicAmplifierSuitArmor",
        "RLSoulshatterArmor"
    };

    str ArmorPickups[12] =
    {
        "RLCybernanoGreenArmorPickup",
        "RLCybernanoBlueArmorPickup",
        "RLCybernanoRedArmorPickup",
        "RLNanofiberSkinGreenArmorPickup",
        "RLNanofiberSkinBlueArmorPickup",
        "RLNanofiberSkinRedArmorPickup",
        "RLCyberneticArmorPickup",
        "RLCyberwarriorArmorPickup",
        "RLOModCyberwarriorArmorPickup",
        "RLBerserkPowersuitArmorPickup",
        "RLRyanCordellPsychicAmplifierSuitArmorPickup",
        "RLSoulshatterArmorPickup"
    };

    str ArmorTokens[12] =
    {
        "RLCybernanoGreenArmorToken",
        "RLCybernanoBlueArmorToken",
        "RLCybernanoRedArmorToken",
        "RLNanofiberSkinGreenArmorToken",
        "RLNanofiberSkinBlueArmorToken",
        "RLNanofiberSkinRedArmorToken",
        "RLCyberneticArmorToken",
        "RLCyberwarriorArmorToken",
        "RLOModCyberwarriorArmorToken",
        "RLBerserkPowersuitArmorToken",
        "RLRyanCordellPsychicAmplifierSuitArmorToken",
        "RLSoulshatterArmorToken"
    };

    str ArmorRenegadeTokens[12] =
    {
        "RLCybernanoGreenArmorRenegade",
        "RLCybernanoBlueArmorRenegade",
        "RLCybernanoRedArmorRenegade",
        "RLNanofiberSkinGreenArmorRenegade",
        "RLNanofiberSkinBlueArmorRenegade",
        "RLNanofiberSkinRedArmorRenegade",
        "RLCyberneticArmorRenegade",
        "RLCyberwarriorArmorRenegade",
        "RLOModCyberwarriorArmorRenegade",
        "RLBerserkPowersuitArmorRenegade",
        "RLRyanCordellPsychicAmplifierSuitArmorRenegade",
        "RLSoulshatterArmorRenegade"
    };

    str ArmorBonusTokens[12] =
    {
        "null",
        "null",
        "null",
        "null",
        "null",
        "null",
        "RLCyberneticArmorBonus",
        "RLCyberwarriorArmorBonus",
        "null",
        "RLBerserkPowersuitArmorBonus",
        "RLRyanCordellPsychicAmplifierSuitArmorBonus",
        "RLSoulshatterArmorBonus"
    };

    str CyberneticArmorTokens[26] =
    {
        "RLCyberneticArmorBulk",
        "RLCyberneticArmorPower",
        "RLCyberneticArmorAgility",
        "RLCyberneticArmorTechnical",
        "RLCyberneticArmorSniper",
        "RLCyberneticArmorFirestorm",
        "RLCyberneticArmorNano",
        "RLCyberneticArmorOnyx",
        "RLCyberneticArmorBulkToken",
        "RLCyberneticArmorPowerToken",
        "RLCyberneticArmorAgilityToken",
        "RLCyberneticArmorTechnicalToken",
        "RLCyberneticArmorSniperToken",
        "RLCyberneticArmorFirestormToken",
        "RLCyberneticArmorNanoToken",
        "RLCyberneticArmorOnyxToken",
        "RLCyberneticArmorRenegadeBulk",
        "RLCyberneticArmorRenegadePower",
        "RLCyberneticArmorRenegadeAgility",
        "RLCyberneticArmorRenegadeTechnical",
        "RLCyberneticArmorRenegadeSniper",
        "RLCyberneticArmorRenegadeFirestorm",
        "RLCyberneticArmorRenegadeNano",
        "RLCyberneticArmorRenegadeOnyx",
        "RLCyberneticArmorModdedToken",
        "RLCyberneticArmorModdingConfirm"
    };

    str ArmorIcons[12] =
    {
        "CYN1A0",
        "CYN2A0",
        "CYN3A0",
        "NFS1A0",
        "NFS2A0",
        "NFS3A0",
        "CYNAA0",
        "CYWAA0",
        "ONYAF0",
        "BEPSB0",
        "RCPAA0",
        "SOLSA0"
    };

    // So the player's initial interaction is not processed as a menu action
    Delay(1);

    while (Player.OutpostMenu == OMENU_OPERATINGCAPSULE)
    {
        SetPlayerProperty(0, 1, PROP_TOTALLYFROZEN);

        // Draw the background
        if (GetCVar("drpg_menudim"))
            FadeRange(0, 0, 0, 0.65, 0, 0, 0, 0.0, 0.25);

        // Set the HUD Size
        SetHudSize(GetActivatorCVar("drpg_menu_width"), GetActivatorCVar("drpg_menu_height"), true);

        // Draw Border
        // These are pushed back a bit so the border doesn't overlap anything
        if (GetActivatorCVar("drpg_menu_background_border"))
            DrawBorder("Bor", -1, 8, -5.0, 0.0, 470, 470);

        // Price Calculate
        if (OperationChoice <= 2)
            Price = 12500;
        if (OperationChoice > 2 && OperationChoice <= 5)
            Price = 25000;
        if (OperationChoice == 6)
            Price = 50000;
        if (OperationChoice > 6)
            Price = 50;

        // Text
        SetFont("BIGFONT");
        HudMessage("\CdOperating Capsule\C-");
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID, "White", X + 120.0, Y + 16.0, 0.05, 0.05);

        SetFont("BIGFONT");
        HudMessage("Select the armor to remove:");
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 1, "White", X + 24.0, Y + 56.0, 0.05, 0.05);

        PrintSprite(ArmorIcons[OperationChoice], 0, X + 240.0,  Y + 180.0, 0.05);

        SetFont("BIGFONT");
        HudMessage("%S", ArmorNames[OperationChoice]);
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 2, "White", X + 72.0, Y + 208.0, 0.05, 0.05);

        SetFont("BIGFONT");
        if (OperationChoice <= 6)
            HudMessage("\CfPrice:\C- %d \CfC\C- and 1 \CdExtra Life\C-", Price);
        else
            HudMessage("\CfPrice:\C- %d \CfC\C-", Price);
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 3, "White", X + 72.0, Y + 240.0, 0.05, 0.05);

        // Input
        if (CheckInput(BT_MOVELEFT, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            OperationChoice--;
            if (OperationChoice < 0) OperationChoice = 11;
        }
        if (CheckInput(BT_MOVERIGHT, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            OperationChoice++;
            if (OperationChoice > 11) OperationChoice = 0;
        }
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
        {
            if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
            {
                if ((((OperationChoice <= 6) && CheckInventory("DRPGLife")) || (OperationChoice > 6)) && (CheckInventory("DRPGCredits") >= Price && CheckInventory(ArmorTokens[OperationChoice]) && CheckInventory("RLArmorInInventory") < DRLA_ARMOR_MAX))
                {
                    Player.OutpostMenu = 0;

                    // Take Credits and Extra Life
                    if (OperationChoice <= 6)
                        TakeInventory("DRPGLife",1);

                    TakeInventory(("DRPGCredits"), Price);

                    // The effect of sleep immersion
                    FadeRange(0, 0, 0, 0.5, 0, 0, 0, 1.0, 2.0);
                    Delay(35 * 5);

                    // Take Universe Armor Tokens
                    TakeInventory("RLUnequippingArmor", 1);
                    GiveInventory("RLArmorRemover", 1);
                    if (OperationChoice == 7)
                        TakeInventory("RL100ArmorWorn",1);
                    else
                        TakeInventory("RLIndestructibleArmorWorn",1);

                    TakeInventory("BasicArmor",99999);

                    // Take Individual Armor Tokens
                    TakeInventory(ArmorActors[OperationChoice],1);
                    TakeInventory(ArmorTokens[OperationChoice], 1);
                    TakeInventory(ArmorRenegadeTokens[OperationChoice], 1);
                    if (OperationChoice >= 6 && OperationChoice != 8)
                        TakeInventory(ArmorBonusTokens[OperationChoice], 1);

                    // Take Cybernetic Armor Addition Tokens
                    if (OperationChoice == 6)
                        for (int i = 0; i < 26; i++)
                            TakeInventory(CyberneticArmorTokens[i],1);

                    // Give Removed Armor
                    GiveInventory(ArmorPickups[OperationChoice],1);

                    SetFont("BIGFONT");
                    HudMessage("Armor removal completed");
                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 4, "Green", X + 80.0, Y + 240.0, 3.0, 2.0);
                    ActivatorSound("mission/complete", 127);
                    FadeRange(0, 0, 0, 1.0, 0, 0, 0, 0.0, 3.0);

                    SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
                    return;
                }
                else
                {
                    if (!CheckInventory(ArmorTokens[OperationChoice]))
                        HudMessage("The selected armor\nis not equipped!");
                    else if (CheckInventory("RLArmorInInventory") >= DRLA_ARMOR_MAX)
                        HudMessage("Your inventory for\narmor is full!");
                    else if (CheckInventory("DRPGCredits") < Price)
                        HudMessage("You do not have\nenough credits!");
                    else if ((OperationChoice <= 6) && !CheckInventory("DRPGLife"))
                        HudMessage("You do not have\nany extra lives!");
                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 4, "Red", X + 108.0, Y + 304.0, 3.0, 2.0);
                    ActivatorSound("menu/error", 127);
                }
            }
        }
        Delay(1);
    }
}

NamedScript MapSpecial void DisassemblingDevice()
{
    // if you're already in a menu, terminate
    if (Player.InMenu || Player.InShop || Player.OutpostMenu == OMENU_DISASSEMBLINGDEVICE) return;

    // If Marines are hostile or the power is out, terminate
    if (MarinesHostile || PowerOut) return;

    // If don't use an DoomRL Arsenal, terminate
    if (CompatMode != COMPAT_DRLA)
    {
        PrintError("The Disassembling Device is not working right now");
        ActivatorSound("menu/error", 127);
        return;
    }

    ActivatorSound("menu/move", 127);
    Player.OutpostMenu = OMENU_DISASSEMBLINGDEVICE;

    fixed X = 0.1;
    fixed Y = 0.1;

    // Categories Data
    int CurrentAction;
    str ActionNames[2] =
    {
        "\CaDisassembling",
        "  \CdAssembling"
    };

    // So the player's initial interaction is not processed as a menu action
    Delay(1);

    while (Player.OutpostMenu == OMENU_DISASSEMBLINGDEVICE)
    {
        SetPlayerProperty(0, 1, PROP_TOTALLYFROZEN);

        // Draw the background
        if (GetCVar("drpg_menudim"))
            FadeRange(0, 0, 0, 0.65, 0, 0, 0, 0.0, 0.25);

        // Set the HUD Size
        SetHudSize(GetActivatorCVar("drpg_menu_width"), GetActivatorCVar("drpg_menu_height"), true);

        // Draw Border
        // These are pushed back a bit so the border doesn't overlap anything
        if (GetActivatorCVar("drpg_menu_background_border"))
            DrawBorder("Bor", -1, 8, -5.0, 0.0, 470, 470);

        // Text
        SetFont("BIGFONT");
        HudMessage("\CdDisassembling Device\C-");
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID, "White", X + 108.0, Y + 16.0, 0.05, 0.05);

        SetFont("BIGFONT");
        HudMessage("Select action:");
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 1, "White", X + 156.0, Y + 112.0, 0.05, 0.05);

        SetFont("BIGFONT");
        HudMessage("%S", ActionNames[CurrentAction]);
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 2, "White", X + 154.0, Y + 188.0, 0.05, 0.05);

        // Input
        if (CheckInput(BT_FORWARD, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            CurrentAction++;
            if (CurrentAction > 1) CurrentAction = 0;
        }
        if (CheckInput(BT_BACK, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            CurrentAction--;
            if (CurrentAction < 0) CurrentAction = 1;
        }
        if (CheckInput(BT_MOVELEFT, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            CurrentAction--;
            if (CurrentAction < 0) CurrentAction = 1;
        }
        if (CheckInput(BT_MOVERIGHT, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            CurrentAction++;
            if (CurrentAction > 1) CurrentAction = 0;
        }
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
        {
            // Disassembling
            if (CurrentAction == 0)
            {
                ActivatorSound("menu/move", 127);
                Player.OutpostMenu = OMENU_DISASSEMBLING;

                // Categories Data
                int CurrentCategory;
                int CategoriesData[4] = {0, 3, 5, 9};
                str CategoriesNames[3] =
                {
                    "\CaWeapons",
                    "\CdArmors/Boots",
                    "\CnShield Parts"
                };

                str ExtentExtraction[6] =
                {
                    "\CdVery Low",
                    "    \CdLow",
                    "  \CiMedium",
                    "    \CaHigh",
                    "\CgVery High",
                    "\CfVery High+"
                };

                str PossibleExtraction[6] =
                {
                    "\CdDetails\C-\n\CfChips\C-\n\CnRecipes\C-",
                    "\CdDetails\C-\n\CfChips\C-\n\CaBattery\C-\n\CnRecipes\C-",
                    "\CdDetails\C-\n\CfChips\C-\n\CaBattery\C-\n\CgTurret Parts\C-\n\CnRecipes\C-\n\CrModPacks\C-",
                    "\CdDetails\C-\n\CfChips\C-\n\CaBattery\C-\n\CgTurret Parts\C-\n\CnRecipes\C-\n\CrModPacks\C-\n\CqModule\C-",
                    "\CdDetails\C-\n\CfChips\C-\n\CaBattery\C-\n\CgTurret Parts\C-\n\CnRecipes\C-\n\CrModPacks\C-\n\CqModule\C-\n\CkAug\C-",
                    "\CdDetails\C-\n\CfChips\C-\n\CaBattery\C-\n\CgTurret Parts\C-\n\CnRecipes\C-\n\CrModPacks\C-\n\CqModule\C-\n\CkAug\C-"
                };

                // Weapons Data
                int WeaponData;
                int WeaponIndexes[10];
                str WeaponActors[10];
                str WeaponNames[10];
                str WeaponIcons[10];
                int WeaponCost[10];

                // Armors/Boots Data
                int ArmorData;
                int ArmorIndexes[10];
                str ArmorActors[10];
                str ArmorNames[10];
                str ArmorIcons[10];
                int ArmorCost[10];

                // Shield Parts Data
                int ShieldData;
                int ShieldIndexes[MAX_BODIES + MAX_BATTERIES + MAX_CAPACITORS + MAX_ACCESSORIES];
                str ShieldActors[MAX_BODIES + MAX_BATTERIES + MAX_CAPACITORS + MAX_ACCESSORIES];
                str ShieldNames[MAX_BODIES + MAX_BATTERIES + MAX_CAPACITORS + MAX_ACCESSORIES];
                str ShieldIcons[MAX_BODIES + MAX_BATTERIES + MAX_CAPACITORS + MAX_ACCESSORIES];
                int ShieldCost[MAX_BODIES + MAX_BATTERIES + MAX_CAPACITORS + MAX_ACCESSORIES];

                // Current Item
                int CurrentItem;
                int CurrentData;
                int CurrentIndex;
                str CurrentActor;
                str CurrentName;
                str CurrentIcon;
                int CurrentCost;
                int CostMin;
                int CostMax;
                int CurrentExtraction;
                int CurrentExtractionCost;

                // Chances of getting parts
                int MaxAmount;
                fixed ChanceDetails;
                fixed ChanceChips;
                fixed ChanceBattery;
                fixed ChanceTurret;
                fixed ChanceModule;
                fixed ChanceAug;
                fixed ChanceBluePrint;
                fixed ChanceModPacks;

                // Extraction Accounting
                int AmountGunParts;
                int AmountAmmoCasings;
                int AmountFuelCapsule;
                int AmountScrapMetals;
                int AmountScrapElectronics;
                int AmountExperimentalParts;
                int AmountDemonArtifacts;
                int AmountDifferentDetails;
                int AmountChipGold;
                int AmountChipPlatinum;
                int AmountBattery;
                int AmountTurret;
                int AmountModule;
                int AmountAug;
                int AmountBluePrint;
                int AmountModPacks;

                // Database creation
                for (int i = 0; i < 4; i++)
                    for (int j = 0; j < ItemMax[CategoriesData[i]]; j++)
                        if (CheckInventory(ItemData[CategoriesData[i]][j].Actor))
                        {
                            // Weapons
                            if (i == 0)
                            {
                                ItemInfoPtr Item = &ItemData[CategoriesData[i]][j];
                                WeaponIndexes[WeaponData] = j;
                                WeaponActors[WeaponData] = Item->Actor;
                                WeaponNames[WeaponData] = Item->Name;
                                WeaponIcons[WeaponData] = Item->Sprite.Name;
                                WeaponCost[WeaponData] = Item->Price;
                                WeaponData++;
                                if (WeaponData >= 10 - 1) continue;
                            }

                            // Armors/Boots
                            if (i == 1 || i == 3)
                            {
                                ItemInfoPtr Item = &ItemData[CategoriesData[i]][j];
                                ArmorIndexes[ArmorData] = j;
                                ArmorActors[ArmorData] = Item->Actor;
                                ArmorNames[ArmorData] = Item->Name;
                                ArmorIcons[ArmorData] = Item->Sprite.Name;
                                ArmorCost[ArmorData] = Item->Price;
                                ArmorData++;
                                if (ArmorData >= 10 - 1) continue;
                            }

                            // Shield Parts
                            if (i == 2)
                            {
                                ItemInfoPtr Item = &ItemData[CategoriesData[i]][j];
                                ShieldIndexes[ShieldData] = j;
                                ShieldActors[ShieldData] = Item->Actor;
                                ShieldNames[ShieldData] = Item->Name;
                                ShieldIcons[ShieldData] = Item->Sprite.Name;
                                ShieldCost[ShieldData] = Item->Price;
                                ShieldData++;
                                if (ShieldData >= MAX_BODIES + MAX_BATTERIES + MAX_CAPACITORS + MAX_ACCESSORIES - 1) continue;
                            }
                        }

                // So the player's initial interaction is not processed as a menu action
                Delay(1);

                while (Player.OutpostMenu == OMENU_DISASSEMBLING)
                {
                    SetPlayerProperty(0, 1, PROP_TOTALLYFROZEN);

                    // Draw the background
                    if (GetCVar("drpg_menudim"))
                        FadeRange(0, 0, 0, 0.65, 0, 0, 0, 0.0, 0.25);

                    // Set the HUD Size
                    SetHudSize(GetActivatorCVar("drpg_menu_width"), GetActivatorCVar("drpg_menu_height"), true);

                    // Draw Border
                    // These are pushed back a bit so the border doesn't overlap anything
                    if (GetActivatorCVar("drpg_menu_background_border"))
                        DrawBorder("Bor", -1, 8, -5.0, 0.0, 470, 470);

                    // Info for Current Item
                    // For Weapons
                    if (CurrentCategory == 0 && WeaponData > 0)
                    {
                        CurrentData = WeaponData;
                        CurrentIndex = WeaponIndexes[CurrentItem];
                        CurrentActor = WeaponActors[CurrentItem];
                        CurrentName = WeaponNames[CurrentItem];
                        CurrentIcon = WeaponIcons[CurrentItem];
                        CurrentCost = WeaponCost[CurrentItem] / 20;
                    }
                    // For Armors/Boots
                    if (CurrentCategory == 1 && ArmorData > 0)
                    {
                        CurrentData = ArmorData;
                        CurrentIndex = ArmorIndexes[CurrentItem];
                        CurrentActor = ArmorActors[CurrentItem];
                        CurrentName = ArmorNames[CurrentItem];
                        CurrentIcon = ArmorIcons[CurrentItem];
                        CurrentCost = ArmorCost[CurrentItem] / 20;
                    }
                    // For Shield Parts
                    if (CurrentCategory == 2 && ShieldData > 0)
                    {
                        CurrentData = ShieldData;
                        CurrentIndex = ShieldIndexes[CurrentItem];
                        CurrentActor = ShieldActors[CurrentItem];
                        CurrentName = ShieldNames[CurrentItem];
                        CurrentIcon = ShieldIcons[CurrentItem];
                        CurrentCost = ShieldCost[CurrentItem] / 20;
                    }

                    // Calculate Extent Extraction
                    // For Weapons and Shield Parts
                    if (CurrentCategory == 0 || CurrentCategory == 2)
                    {
                        if (CurrentCost < 100)
                        {
                            CostMin = 0;
                            CostMax = 100;
                            CurrentExtraction = 0;
                        }
                        if (CurrentCost >= 100)
                        {
                            CostMin = 100;
                            CostMax = 1500;
                            CurrentExtraction = 1;
                        }
                        if (CurrentCost > 1500)
                        {
                            CostMin = 1500;
                            CostMax = 3000;
                            CurrentExtraction = 2;
                        }
                        if (CurrentCost > 3000)
                        {
                            CostMin = 3000;
                            CostMax = 6000;
                            CurrentExtraction = 3;
                        }
                        if (CurrentCost > 6000)
                        {
                            CostMin = 6000;
                            CostMax = 10000;
                            CurrentExtraction = 4;
                        }
                        if (CurrentCost > 10000)
                        {
                            CostMin = 10000;
                            CostMax = 20000;
                            CurrentExtraction = 5;
                        }
                    }
                    // For Armor/Boots
                    if (CurrentCategory == 1)
                    {
                        if (CurrentCost <= 50)
                        {
                            CostMin = 0;
                            CostMax = 50;
                            CurrentExtraction = 0;
                        }
                        if (CurrentCost > 50)
                        {
                            CostMin = 50;
                            CostMax = 225;
                            CurrentExtraction = 1;
                        }
                        if (CurrentCost > 225)
                        {
                            CostMin = 225;
                            CostMax = 900;
                            CurrentExtraction = 2;
                        }
                        if (CurrentCost > 900)
                        {
                            CostMin = 900;
                            CostMax = 1500;
                            CurrentExtraction = 3;
                        }
                        if (CurrentCost > 1500)
                        {
                            CostMin = 1500;
                            CostMax = 2500;
                            CurrentExtraction = 4;
                        }
                        if (CurrentCost > 2500)
                        {
                            CostMin = 2500;
                            CostMax = 5000;
                            CurrentExtraction = 5;
                        }
                    }

                    // Calculate Chances
                    // For Very Low Extent Extraction
                    if (CurrentExtraction == 0)
                    {
                        MaxAmount = Curve(CurrentCost, CostMin, CostMax, 1, 3);
                        ChanceChips = Curve(CurrentCost, CostMin, CostMax, 0.5, 5.0);
                        ChanceBattery = 0.0;
                        ChanceTurret = 0.0;
                        ChanceBluePrint = Curve(CurrentCost, CostMin, CostMax, 0.5, 2.5);
                        ChanceModPacks = 0.0;
                        ChanceModule = 0.0;
                        ChanceAug = 0.0;
                        ChanceDetails = 100.0 -  ChanceChips - ChanceBattery - ChanceTurret - ChanceBluePrint - ChanceModPacks - ChanceModule - ChanceAug;
                    }
                    // For Low Extent Extraction
                    if (CurrentExtraction == 1)
                    {
                        MaxAmount = Curve(CurrentCost, CostMin, CostMax, 4, 12);
                        ChanceChips = Curve(CurrentCost, CostMin, CostMax, 5.0, 7.5);
                        ChanceBattery = Curve(CurrentCost, CostMin, CostMax, 0.5, 5.0);
                        ChanceTurret = 0.0;
                        ChanceBluePrint = Curve(CurrentCost, CostMin, CostMax, 2.5, 3.5);
                        ChanceModPacks = 0.0;
                        ChanceModule = 0.0;
                        ChanceAug = 0.0;
                        ChanceDetails = 100.0 -  ChanceChips - ChanceBattery - ChanceTurret - ChanceBluePrint - ChanceModPacks - ChanceModule - ChanceAug;
                    }
                    // For Medium Extent Extraction
                    if (CurrentExtraction == 2)
                    {
                        MaxAmount = Curve(CurrentCost, CostMin, CostMax, 13, 18);
                        ChanceChips = Curve(CurrentCost, CostMin, CostMax, 7.5, 10.0);
                        ChanceBattery = Curve(CurrentCost, CostMin, CostMax, 5.0, 7.5);
                        ChanceTurret = Curve(CurrentCost, CostMin, CostMax, 0.5, 5.0);
                        ChanceBluePrint = Curve(CurrentCost, CostMin, CostMax, 3.5, 5.0);
                        ChanceModPacks = Curve(CurrentCost, CostMin, CostMax, 0.5, 5.0);
                        ChanceModule = 0.0;
                        ChanceAug = 0.0;
                        ChanceDetails = 100.0 -  ChanceChips - ChanceBattery - ChanceTurret - ChanceBluePrint - ChanceModPacks - ChanceModule - ChanceAug;
                    }
                    // For High Extent Extraction
                    if (CurrentExtraction == 3)
                    {
                        MaxAmount = Curve(CurrentCost, CostMin, CostMax, 19, 24);
                        ChanceChips = Curve(CurrentCost, CostMin, CostMax, 10.0, 12.5);
                        ChanceBattery = Curve(CurrentCost, CostMin, CostMax, 7.5, 10.0);
                        ChanceTurret = Curve(CurrentCost, CostMin, CostMax, 5.0, 7.5);
                        ChanceBluePrint = Curve(CurrentCost, CostMin, CostMax, 5.0, 7.5);
                        ChanceModPacks = Curve(CurrentCost, CostMin, CostMax, 5.0, 7.5);
                        ChanceModule = Curve(CurrentCost, CostMin, CostMax, 0.5, 5.0);
                        ChanceAug = 0.0;
                        ChanceDetails = 100.0 -  ChanceChips - ChanceBattery - ChanceTurret - ChanceBluePrint - ChanceModPacks - ChanceModule - ChanceAug;
                    }
                    // For Very High Extent Extraction
                    if (CurrentExtraction == 4)
                    {
                        MaxAmount = Curve(CurrentCost, CostMin, CostMax, 25, 35);
                        ChanceChips = Curve(CurrentCost, CostMin, CostMax, 12.5, 15.0);
                        ChanceBattery = Curve(CurrentCost, CostMin, CostMax, 10.0, 12.5);
                        ChanceTurret = Curve(CurrentCost, CostMin, CostMax, 7.5, 10.0);
                        ChanceBluePrint = Curve(CurrentCost, CostMin, CostMax, 7.5, 10.0);
                        ChanceModPacks = Curve(CurrentCost, CostMin, CostMax, 7.5, 10.0);
                        ChanceModule = Curve(CurrentCost, CostMin, CostMax, 5.0, 7.5);
                        ChanceAug = Curve(CurrentCost, CostMin, CostMax, 0.5, 5.0);
                        ChanceDetails = 100.0 -  ChanceChips - ChanceBattery - ChanceTurret - ChanceBluePrint - ChanceModPacks - ChanceModule - ChanceAug;
                    }
                    // For Very High+ Extent Extraction
                    if (CurrentExtraction == 5)
                    {
                        MaxAmount = Curve(CurrentCost, CostMin, CostMax, 36, 50);
                        ChanceChips = Curve(CurrentCost, CostMin, CostMax, 15.0, 17.5);
                        ChanceBattery = Curve(CurrentCost, CostMin, CostMax, 12.5, 15.0);
                        ChanceTurret = Curve(CurrentCost, CostMin, CostMax, 10.0, 12.5);
                        ChanceBluePrint = Curve(CurrentCost, CostMin, CostMax, 10.0, 12.5);
                        ChanceModPacks = Curve(CurrentCost, CostMin, CostMax, 10.0, 12.5);
                        ChanceModule = Curve(CurrentCost, CostMin, CostMax, 7.5, 10.0);
                        ChanceAug = Curve(CurrentCost, CostMin, CostMax, 5.0, 7.5);
                        ChanceDetails = 100.0 -  ChanceChips - ChanceBattery - ChanceTurret - ChanceBluePrint - ChanceModPacks - ChanceModule - ChanceAug;
                    }

                    // Calculate Current Extraction Cost
                    CurrentExtractionCost = (25 * MaxAmount - (25 * MaxAmount * Player.ShopDiscount / 100)) / 5 * 5;

                    // Text
                    SetFont("BIGFONT");
                    HudMessage("\CdDisassembling Device\C-");
                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID, "White", X + 108.0, Y + 16.0, 0.05, 0.05);

                    SetFont("BIGFONT");
                    HudMessage("Select item for disassembling:");
                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 1, "White", X + 64.0, Y + 56.0, 0.05, 0.05);

                    SetFont("BIGFONT");
                    HudMessage("Category: %S", CategoriesNames[CurrentCategory]);
                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 2, "White", X + 124.0, Y + 112.0, 0.05, 0.05);

                    if (CurrentCategory == 0 && WeaponData > 0 || CurrentCategory == 1 && ArmorData > 0 || CurrentCategory == 2 && ShieldData > 0)
                    {
                        if(CheckInventory(CurrentActor) > 1)
                        {
                            ItemInfoPtr Item = &ItemData[CategoriesData[CurrentCategory]][CurrentIndex];
                            int CurrentXOffset = Item->Sprite.XOff;
                            int CurrentYOffset = Item->Sprite.YOff;
                            SetFont("SMALLFONT");
                            HudMessage("%d", CheckInventory(CurrentActor));
                            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 3, "White", X + 240.2 + CurrentXOffset, Y + 188.2, 0.05, 0.05);
                        }

                        PrintSprite(CurrentIcon, 0, X + 240.0,  Y + 188.0, 0.05);

                        SetFont("BIGFONT");
                        HudMessage("Item: %S", CurrentName);
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 4, "White", X + 32.0, Y + 230.0, 0.05, 0.05);

                        SetFont("SMALLFONT");
                        HudMessage("Extent of Extraction:");
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 5, "White", X + 280.0, Y + 272.0, 0.05, 0.05);

                        SetFont("BIGFONT");
                        HudMessage("%S", ExtentExtraction[CurrentExtraction]);
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 6, "White", X + 304.0, Y + 304.0, 0.05, 0.05);

                        SetFont("SMALLFONT");
                        HudMessage("Cost of Extraction:");
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 7, "White", X + 292.0, Y + 336.0, 0.05, 0.05);

                        SetFont("BIGFONT");
                        HudMessage("\Cf%d C\C-", CurrentExtractionCost);
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 8, "White", X + 336.0, Y + 368.0, 0.05, 0.05);

                        SetFont("SMALLFONT");
                        HudMessage("\Ck(Discount: %d%%)", Player.ShopDiscount);
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 9, "White", X + 308.0, Y + 400.0, 0.05, 0.05);

                        SetFont("SMALLFONT");
                        HudMessage("Possible Extraction:\n\nMaximum amount: \Cd%d pcs.\C-", MaxAmount);
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 10, "White", X + 32.0, Y + 272.0, 0.05, 0.05);

                        // Possible Extraction
                        // For Details
                        if (ChanceDetails > 0)
                        {
                            SetFont("SMALLFONT");
                            HudMessage("\CdDetails\C- rate: \Cf%.2k%%\C-", ChanceDetails);
                            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 11, "White", X + 32.0, Y + 280.0 + 24.0, 0.05, 0.05);
                        }
                        // For Chips
                        if (ChanceChips > 0)
                        {
                            SetFont("SMALLFONT");
                            HudMessage("\CfChips\C- rate: \Cf%.2k%%\C-", ChanceChips);
                            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 12, "White", X + 32.0, Y + 280.0 + 32.0, 0.05, 0.05);
                        }
                        // For Recipes
                        if (ChanceBluePrint > 0)
                        {
                            SetFont("SMALLFONT");
                            HudMessage("\CnRecipes\C- rate: \Cf%.2k%%\C-", ChanceBluePrint);
                            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 13, "White", X + 32.0, Y + 280.0 + 40.0, 0.05, 0.05);
                        }
                        // For Battery
                        if (ChanceBattery > 0)
                        {
                            SetFont("SMALLFONT");
                            HudMessage("\CaBattery\C- rate: \Cf%.2k%%\C-", ChanceBattery);
                            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 14, "White", X + 32.0, Y + 280.0 + 48.0, 0.05, 0.05);
                        }
                        // For Turret Parts
                        if (ChanceTurret > 0)
                        {
                            SetFont("SMALLFONT");
                            HudMessage("\CgTurret Parts\C- rate: \Cf%.2k%%\C-", ChanceTurret);
                            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 15, "White", X + 32.0, Y + 280.0 + 56.0, 0.05, 0.05);
                        }
                        // For ModPacks
                        if (ChanceModPacks > 0)
                        {
                            SetFont("SMALLFONT");
                            HudMessage("\CrModPacks\C- rate: \Cf%.2k%%\C-", ChanceModPacks);
                            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 16, "White", X + 32.0, Y + 280.0 + 64.0, 0.05, 0.05);
                        }
                        // For Module
                        if (ChanceModule > 0)
                        {
                            SetFont("SMALLFONT");
                            HudMessage("\CqModule\C- rate: \Cf%.2k%%\C-", ChanceModule);
                            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 17, "White", X + 32.0, Y + 280.0 + 72.0, 0.05, 0.05);
                        }
                        // For Augmentation
                        if (ChanceAug > 0)
                        {
                            SetFont("SMALLFONT");
                            HudMessage("\CkAugmentation\C- rate: \Cf%.2k%%\C-", ChanceAug);
                            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 18, "White", X + 32.0, Y + 280.0 + 80.0, 0.05, 0.05);
                        }
                    }
                    else
                    {
                        SetFont("BIGFONT");
                        HudMessage("No have items in this category");
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 3, "Red", X + 56.0, Y + 208.0, 0.05, 0.05);
                    }

                    // Input
                    if (CheckInput(BT_FORWARD, KEY_ONLYPRESSED, false, PlayerNumber()))
                    {
                        ActivatorSound("menu/move", 127);
                        CurrentCategory++;
                        CurrentItem = 0;
                        if (CurrentCategory > 2) CurrentCategory = 0;
                    }
                    if (CheckInput(BT_BACK, KEY_ONLYPRESSED, false, PlayerNumber()))
                    {
                        ActivatorSound("menu/move", 127);
                        CurrentCategory--;
                        CurrentItem = 0;
                        if (CurrentCategory < 0) CurrentCategory = 2;
                    }
                    if (CheckInput(BT_MOVELEFT, KEY_ONLYPRESSED, false, PlayerNumber()))
                    {
                        ActivatorSound("menu/move", 127);
                        CurrentItem--;
                        if (CurrentItem < 0) CurrentItem = CurrentData - 1;
                    }
                    if (CheckInput(BT_MOVERIGHT, KEY_ONLYPRESSED, false, PlayerNumber()))
                    {
                        ActivatorSound("menu/move", 127);
                        CurrentItem++;
                        if (CurrentItem > CurrentData - 1) CurrentItem = 0;
                    }
                    if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
                    {
                        if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
                        {
                            if (CheckInventory("DRPGCredits") >= CurrentExtractionCost && (CurrentCategory == 0 && WeaponData > 0 || CurrentCategory == 1 && ArmorData > 0 || CurrentCategory == 2 && ShieldData > 0))
                            {
                                Player.OutpostMenu = 0;
                                str ActorToSpawn;
                                bool PartReceived;
                                int Attempts = MaxAmount;
                                int AddMenuID = 4;
                                fixed Y1;

                                // Take Current Extraction Cost
                                TakeInventory("DRPGCredits", CurrentExtractionCost);

                                // Take Current Item
                                TakeInventory(CurrentActor, 1);

                                // Take tokens from DoomRL Arsenal
                                if (CompatMode == COMPAT_DRLA);
                                {
                                    if (CurrentCategory == 0) RemoveDRLAItem(0, CurrentIndex);
                                    if (CurrentCategory == 1) RemoveDRLAItem(3, CurrentIndex);
                                }

                                // The effect of sleep immersion
                                FadeRange(0, 0, 0, 0.5, 0, 0, 0, 1.0, 1.0);
                                Delay(35 * 1);

                                // Get Part
                                while (Attempts > 0 && CurrentCost > 0)
                                {
                                    // For Details
                                    if (RandomFixed(0.0, 100.0) <  ChanceDetails)
                                    {
                                        if (CurrentCategory == 0)
                                        {
                                            if (Random(0,5) <= 0 && !PartReceived)
                                            {
                                                if (Random(0,1) <= 0)
                                                {
                                                    ActorToSpawn = ItemData[7][8].Actor;
                                                    AmountFuelCapsule++;
                                                    PartReceived = true;
                                                }
                                                else
                                                {
                                                    ActorToSpawn = ItemData[7][11].Actor;
                                                    AmountExperimentalParts++;
                                                    PartReceived = true;
                                                }
                                            }
                                            else if (Random(0,2) <= 0 && !PartReceived)
                                            {
                                                ActorToSpawn = ItemData[7][7].Actor;
                                                AmountAmmoCasings++;
                                                PartReceived = true;
                                            }
                                            else if (!PartReceived)
                                            {
                                                ActorToSpawn = ItemData[7][6].Actor;
                                                AmountGunParts++;
                                                PartReceived = true;
                                            }
                                        }
                                        if (CurrentCategory == 1)
                                        {
                                            if (Random(0,5) <= 0 && !PartReceived)
                                            {
                                                if (Random(0,1) <= 0)
                                                {
                                                    ActorToSpawn = ItemData[7][8].Actor;
                                                    AmountFuelCapsule++;
                                                    PartReceived = true;
                                                }
                                                else
                                                {
                                                    ActorToSpawn = ItemData[7][11].Actor;
                                                    AmountExperimentalParts++;
                                                    PartReceived = true;
                                                }
                                            }
                                            else if (Random(0,2) <= 0 && !PartReceived)
                                            {
                                                ActorToSpawn = ItemData[7][10].Actor;
                                                AmountScrapElectronics++;
                                                PartReceived = true;
                                            }
                                            else if (!PartReceived)
                                            {
                                                ActorToSpawn = ItemData[7][9].Actor;
                                                AmountScrapMetals++;
                                                PartReceived = true;
                                            }
                                        }
                                        if (CurrentCategory == 2)
                                        {
                                            ActorToSpawn = ItemData[7][Random(0, 5)].Actor;
                                            AmountDifferentDetails++;
                                            PartReceived = true;
                                        }
                                    }
                                    // For Other Chances
                                    else
                                    {
                                        if (RandomFixed(0.0, 100.0) <  ChanceChips && !PartReceived)
                                        {
                                            if (CurrentExtraction < 3)
                                            {
                                                if (Random(0, 32) <= 0 && !PartReceived)
                                                {
                                                    ActorToSpawn = "DRPGChipPlatinum";
                                                    AmountChipPlatinum++;
                                                    PartReceived = true;
                                                    CurrentCost -= 250;
                                                }
                                                else if (!PartReceived)
                                                {
                                                    ActorToSpawn = "DRPGChipGold";
                                                    AmountChipGold++;
                                                    PartReceived = true;
                                                    CurrentCost -= 125;
                                                }
                                            }
                                            else
                                            {
                                                if (Random(0, 32) <= 0 && !PartReceived)
                                                {
                                                    ActorToSpawn = "DRPGChipPlatinumPack";
                                                    AmountChipPlatinum += 3;
                                                    PartReceived = true;
                                                    CurrentCost -= 750;
                                                }
                                                else if (!PartReceived)
                                                {
                                                    ActorToSpawn = "DRPGChipGoldPack";
                                                    AmountChipGold += 3;
                                                    PartReceived = true;
                                                    CurrentCost -= 375;
                                                }
                                            }
                                        }
                                        if (RandomFixed(0.0, 100.0) <  ChanceBattery && !PartReceived)
                                        {
                                            if (Random(0,1) <= 0 && !PartReceived)
                                            {
                                                ActorToSpawn = "DRPGLootDemonArtifact";
                                                AmountDemonArtifacts++;
                                                PartReceived = true;
                                                CurrentCost -= 100;
                                            }
                                            else if (!PartReceived)
                                            {
                                                ActorToSpawn = "DRPGBatteryDropper";
                                                AmountBattery++;
                                                PartReceived = true;
                                                CurrentCost -= 200;
                                            }
                                        }
                                        if (RandomFixed(0.0, 100.0) <  ChanceTurret && !PartReceived)
                                        {
                                            if (Random(0, 20) <= 0 && !PartReceived)
                                            {
                                                ActorToSpawn = "DRPGTurretPartCrate";
                                                AmountTurret += 5;
                                                PartReceived = true;
                                                CurrentCost -= 625;
                                            }
                                            else if (!PartReceived)
                                            {
                                                ActorToSpawn = "DRPGTurretPart";
                                                AmountTurret++;
                                                PartReceived = true;
                                                CurrentCost -= 125;
                                            }
                                        }
                                        if (RandomFixed(0.0, 100.0) <  ChanceBluePrint && !PartReceived)
                                        {
                                            ActorToSpawn = "RLBlueprintComputer";
                                            AmountBluePrint++;
                                            PartReceived = true;
                                            CurrentCost -= 2000;
                                        }
                                        if (RandomFixed(0.0, 100.0) <  ChanceModPacks && !PartReceived)
                                        {
                                            ActorToSpawn = (Random(0, 4) <= 0 ? ItemData[8][Random(4, 8)].Actor : ItemData[8][Random(0, 3)].Actor);
                                            AmountModPacks++;
                                            PartReceived = true;
                                            CurrentCost -= 2500;
                                        }
                                        if (RandomFixed(0.0, 100.0) <  ChanceModule && !PartReceived)
                                        {
                                            ActorToSpawn = "DRPGModuleDropper";
                                            AmountModule++;
                                            PartReceived = true;
                                            CurrentCost -= 2500;
                                        }
                                        if (RandomFixed(0.0, 100.0) <  ChanceAug && !PartReceived)
                                        {
                                            ActorToSpawn = "DRPGAugDropper";
                                            AmountAug++;
                                            PartReceived = true;
                                            CurrentCost -= 5000;
                                        }
                                    }
                                    // For cases where no chance was successful
                                    if (!PartReceived)
                                    {
                                        if (CurrentCategory == 0)
                                        {
                                            ActorToSpawn = ItemData[7][6].Actor;
                                            AmountGunParts++;
                                            PartReceived = true;
                                        }
                                        if (CurrentCategory == 1)
                                        {
                                            ActorToSpawn = ItemData[7][9].Actor;
                                            AmountScrapMetals++;
                                            PartReceived = true;
                                        }
                                        if (CurrentCategory == 2)
                                        {
                                            ActorToSpawn = ItemData[7][Random(0, 5)].Actor;
                                            AmountDifferentDetails++;
                                            PartReceived = true;
                                        }
                                    }

                                    // Spawn Part in Disassembling Device
                                    SpawnSpotForced(ActorToSpawn, DisassemblingDeviceID, UniqueTID(), 0);

                                    PartReceived = false;
                                    Attempts--;
                                }

                                SetFont("BIGFONT");
                                HudMessage("Item disassembly is complete");
                                EndHudMessage(HUDMSG_FADEOUT, MENU_ID, "Green", X + 64.0, Y + 240.0, 3.0, 3.0);

                                // Extraction Accounting
                                SetFont("SMALLFONT");
                                HudMessage("Managed to extract:");
                                EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + 264.0, 3.0, 3.0);

                                // For Different Details (from Shield Parts)
                                if (AmountDifferentDetails > 0)
                                {
                                    Y1 += 8.0;
                                    AddMenuID += 1;
                                    SetFont("SMALLFONT");
                                    HudMessage("Different Details: \Cd%d pcs.\C-", AmountDifferentDetails);
                                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + Y1 + 272.0, 3.0, 3.0);
                                }
                                // For Gun Parts
                                if (AmountGunParts > 0)
                                {
                                    Y1 += 8.0;
                                    AddMenuID += 1;
                                    SetFont("SMALLFONT");
                                    HudMessage("Gun Parts: \Cd%d pcs.\C-", AmountGunParts);
                                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + Y1 + 272.0, 3.0, 3.0);
                                }
                                // For Ammo Casings
                                if (AmountAmmoCasings > 0)
                                {
                                    Y1 += 8.0;
                                    AddMenuID += 1;
                                    SetFont("SMALLFONT");
                                    HudMessage("Ammo Casings: \Cd%d pcs.\C-", AmountAmmoCasings);
                                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + Y1 + 272.0, 3.0, 3.0);
                                }
                                // For Fuel Capsule
                                if (AmountFuelCapsule > 0)
                                {
                                    Y1 += 8.0;
                                    AddMenuID += 1;
                                    SetFont("SMALLFONT");
                                    HudMessage("Fuel Capsule: \Cd%d pcs.\C-", AmountFuelCapsule);
                                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + Y1 + 272.0, 3.0, 3.0);
                                }
                                // For Scrap Metals
                                if (AmountScrapMetals > 0)
                                {
                                    Y1 += 8.0;
                                    AddMenuID += 1;
                                    SetFont("SMALLFONT");
                                    HudMessage("Scrap Metals: \Cd%d pcs.\C-", AmountScrapMetals);
                                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + Y1 + 272.0, 3.0, 3.0);
                                }
                                // For Scrap Electronics
                                if (AmountScrapElectronics > 0)
                                {
                                    Y1 += 8.0;
                                    AddMenuID += 1;
                                    SetFont("SMALLFONT");
                                    HudMessage("Scrap Electronics: \Cd%d pcs.\C-", AmountScrapElectronics);
                                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + Y1 + 272.0, 3.0, 3.0);
                                }
                                // For Experimental Parts
                                if (AmountExperimentalParts > 0)
                                {
                                    Y1 += 8.0;
                                    AddMenuID += 1;
                                    SetFont("SMALLFONT");
                                    HudMessage("Experimental Parts: \Cd%d pcs.\C-", AmountExperimentalParts);
                                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + Y1 + 272.0, 3.0, 3.0);
                                }
                                // For Demon Artifacts
                                if (AmountDemonArtifacts > 0)
                                {
                                    Y1 += 8.0;
                                    AddMenuID += 1;
                                    SetFont("SMALLFONT");
                                    HudMessage("\CaDemon Artifacts\C-: \Cd%d pcs.\C-", AmountDemonArtifacts);
                                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + Y1 + 272.0, 3.0, 3.0);
                                }
                                // For Chip Gold
                                if (AmountChipGold > 0)
                                {
                                    Y1 += 8.0;
                                    AddMenuID += 1;
                                    SetFont("SMALLFONT");
                                    HudMessage("\CfChip Gold\C-: \Cd%d pcs.\C-", AmountChipGold);
                                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + Y1 + 272.0, 3.0, 3.0);
                                }
                                // For Chip Platinum
                                if (AmountChipPlatinum > 0)
                                {
                                    Y1 += 8.0;
                                    AddMenuID += 1;
                                    SetFont("SMALLFONT");
                                    HudMessage("\CwChip Platinum\C-: \Cd%d pcs.\C-", AmountChipPlatinum);
                                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + Y1 + 272.0, 3.0, 3.0);
                                }
                                // For Recipes
                                if (AmountBluePrint > 0)
                                {
                                    Y1 += 8.0;
                                    AddMenuID += 1;
                                    SetFont("SMALLFONT");
                                    HudMessage("\CnRecipes\C-: \Cd%d pcs.\C-", AmountBluePrint);
                                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + Y1 + 272.0, 3.0, 3.0);
                                }
                                // For Battery
                                if (AmountBattery > 0)
                                {
                                    Y1 += 8.0;
                                    AddMenuID += 1;
                                    SetFont("SMALLFONT");
                                    HudMessage("\CaBattery\C-: \Cd%d pcs.\C-", AmountBattery);
                                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + Y1 + 272.0, 3.0, 3.0);
                                }
                                // For Turret
                                if (AmountTurret > 0)
                                {
                                    Y1 += 8.0;
                                    AddMenuID += 1;
                                    SetFont("SMALLFONT");
                                    HudMessage("\CgTurret Parts\C-: \Cd%d pcs.\C-", AmountTurret);
                                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + Y1 + 272.0, 3.0, 3.0);
                                }
                                // For ModPacks
                                if (AmountModPacks > 0)
                                {
                                    Y1 += 8.0;
                                    AddMenuID += 1;
                                    SetFont("SMALLFONT");
                                    HudMessage("\CrModPacks\C-: \Cd%d pcs.\C-", AmountModPacks);
                                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + Y1 + 272.0, 3.0, 3.0);
                                }
                                // For Module
                                if (AmountModule > 0)
                                {
                                    Y1 += 8.0;
                                    AddMenuID += 1;
                                    SetFont("SMALLFONT");
                                    HudMessage("\CqModule\C-: \Cd%d pcs.\C-", AmountModule);
                                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + Y1 + 272.0, 3.0, 3.0);
                                }
                                // For Augmentation
                                if (AmountAug > 0)
                                {
                                    Y1 += 8.0;
                                    AddMenuID += 1;
                                    SetFont("SMALLFONT");
                                    HudMessage("\CkAugmentation\C-: \Cd%d pcs.\C-", AmountAug);
                                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + AddMenuID, "White", X + 64.0, Y + Y1 + 272.0, 3.0, 3.0);
                                }

                                ActivatorSound("mission/complete", 127);
                                FadeRange(0, 0, 0, 1.0, 0, 0, 0, 0.0, 2.0);

                                SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
                                return;
                            }
                            else
                            {
                                SetFont("BIGFONT");
                                if (CurrentCategory == 0 && WeaponData <= 0 || CurrentCategory == 1 && ArmorData <= 0 || CurrentCategory == 2 && ShieldData <= 0)
                                    PrintError("Item is not selected for disassembly");
                                else if (CheckInventory("DRPGCredits") < CurrentExtractionCost)
                                    PrintError("Not enough credits to disassemble this item");

                                ActivatorSound("menu/error", 127);
                            }
                        }
                    }
                    Delay(1);
                }
            }
            // Assembling
            else if (CurrentAction == 1)
            {
                ActivatorSound("menu/move", 127);
                Player.OutpostMenu = OMENU_ASSEMBLING;

                // Additional coordinates
                fixed X1, Y1, Y2;

                // Categories Data
                int CurrentCategory;
                int CategoriesData[3] = {0, 3, 9};
                str CategoriesNames[3] =
                {
                    "\CaWeapons",
                    "\CdArmors",
                    "\CqBoots"
                };

                // Rare
                int CurrentRare;
                str RareNames[2] =
                {
                    "\CtExotic",
                    "\CdUnique"
                };

                // Current Item
                int CurrentItem;
                int CurrentItemMin;
                int CurrentItemMax;
                int CurrentCostMin;
                int CurrentCostMax;
                int CurrentCost;
                int CurrentRank;

                // Current Required Basiс Item
                int CurrentIndexBasic;

                // Current Required Items
                int CurrentTypeDetails1;
                int CurrentAmountDetails1;
                int CurrentTypeDetails2;
                int CurrentAmountDetails2;
                int CurrentTypeDetails3;
                int CurrentAmountDetails3;
                int CurrentTypeDetails4;
                int CurrentAmountDetails4;
                int CurrentTypeDetails5;
                int CurrentAmountDetails5;
                int CurrentTypeDetails6;
                int CurrentAmountDetails6;

                // So the player's initial interaction is not processed as a menu action
                Delay(1);

                while (Player.OutpostMenu == OMENU_ASSEMBLING)
                {
                    SetPlayerProperty(0, 1, PROP_TOTALLYFROZEN);

                    // Reset additional coordinates
                    X1 = 0.0;
                    Y1 = 0.0;
                    Y2 = 0.0;

                    // Get Min/Max Item Index
                    // For Weapon
                    if (CurrentCategory == 0)
                    {
                        if (CurrentRare == 0)
                        {
                            CurrentItemMin = 10;
                            CurrentItemMax = 26;
                        }
                        if (CurrentRare == 1)
                        {
                            CurrentItemMin = 34;
                            CurrentItemMax = 62;
                        }
                    }
                    // For Armor
                    if (CurrentCategory == 1)
                    {
                        if (CurrentRare == 0)
                        {
                            CurrentItemMin = 29;
                            CurrentItemMax = 43;
                        }
                        if (CurrentRare == 1)
                        {
                            CurrentItemMin = 59;
                            CurrentItemMax = 84;
                        }
                    }
                    // For Boots
                    if (CurrentCategory == 2)
                    {
                        if (CurrentRare == 0)
                        {
                            CurrentItemMin = 17;
                            CurrentItemMax = 20;
                        }
                        if (CurrentRare == 1)
                        {
                            CurrentItemMin = 21;
                            CurrentItemMax = 30;
                        }
                    }
                    if (CurrentItem < CurrentItemMin) CurrentItem = CurrentItemMin;
                    if (CurrentItem > CurrentItemMax) CurrentItem = CurrentItemMax;

                    // Calculate Required Items
                    ItemInfoPtr Item = &ItemData[CategoriesData[CurrentCategory]][CurrentItem];
                    // Cost Item
                    // Min/Max Cost
                    CurrentCostMin = ItemData[CategoriesData[CurrentCategory]][CurrentItemMin].Price / 20;
                    CurrentCostMax = ItemData[CategoriesData[CurrentCategory]][CurrentItemMax].Price / 20;
                    for (int i = CurrentItemMin; i <= CurrentItemMax; i++)
                    {
                        if ((ItemData[CategoriesData[CurrentCategory]][i].Price / 20) < CurrentCostMin)
                            CurrentCostMin = ItemData[CategoriesData[CurrentCategory]][i].Price / 20;
                        if ((ItemData[CategoriesData[CurrentCategory]][i].Price / 20) > CurrentCostMax)
                            CurrentCostMax = ItemData[CategoriesData[CurrentCategory]][i].Price / 20;
                    }
                    // Rank
                    if (CurrentRare == 0)
                        CurrentRank = ItemData[CategoriesData[CurrentCategory]][CurrentItem].Rank - 2;
                    else
                        CurrentRank = 6 + (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 0, 8));
                    // Type/Amount Required Details
                    // For Weapons
                    if (CurrentCategory == 0)
                    {
                        // Set Default Current Type Details
                        CurrentTypeDetails1 =  6;
                        CurrentTypeDetails2 =  7;
                        CurrentTypeDetails3 =  9;

                        // Set Global Required
                        // For Exotic Rare
                        if (CurrentRare == 0)
                        {
                            // Calculate Current Cost
                            CurrentCost = ((Item->Price - Item->Price * Player.ShopDiscount / 100) / 3) / 250 * 250;

                            // Calculate Amount Details
                            CurrentAmountDetails1 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 15, 100)) / 5 * 5;
                            CurrentAmountDetails2 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 10, 50)) / 5 * 5;

                            // Set Default Current Craft Part
                            if (CurrentRank >= 5)
                            {
                                CurrentTypeDetails4 =  10;
                                CurrentAmountDetails4 = 1;
                            }
                            if (CurrentRank >= 6 )
                            {
                                CurrentAmountDetails3 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 5, 30)) / 5 * 5;
                            }

                            // Set Individual Required
                            // For Blaster
                            if (ItemData[0][CurrentItem].Actor == "RLBlaster")
                            {
                                CurrentIndexBasic = 1;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails6 =  29;
                                CurrentAmountDetails1 = 10;
                                CurrentAmountDetails6 = 2;
                            }
                            // For Combat Pistol
                            if (ItemData[0][CurrentItem].Actor == "RLCombatPistol")
                            {
                                CurrentIndexBasic = 1;
                                CurrentAmountDetails1 = 10;
                            }
                            // For Handcannon
                            if (ItemData[0][CurrentItem].Actor == "RLHandCannon")
                            {
                                CurrentIndexBasic = 11;
                            }
                            // For Marksman Pistol
                            if (ItemData[0][CurrentItem].Actor == "RLMarksmanPistol")
                            {
                                CurrentIndexBasic = 12;
                            }
                            // For Uzi
                            if (ItemData[0][CurrentItem].Actor == "RLUzi")
                            {
                                CurrentIndexBasic = 11;
                            }
                            // For Assault Shotgun
                            if (ItemData[0][CurrentItem].Actor == "RLAssaultShotgun")
                            {
                                CurrentIndexBasic = 3;
                            }
                            // For Plasma Shotgun
                            if (ItemData[0][CurrentItem].Actor == "RLPlasmaShotgun")
                            {
                                CurrentIndexBasic = 8;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails6 =  29;
                                CurrentAmountDetails6 = 6;
                            }
                            // For Combat Translocator
                            if (ItemData[0][CurrentItem].Actor == "RLCombatTranslocator")
                            {
                                CurrentIndexBasic = 8;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails6 =  30;
                                CurrentAmountDetails6 = 1;
                            }
                            // For Missile Launcher
                            if (ItemData[0][CurrentItem].Actor == "RLMissileLauncher")
                            {
                                CurrentIndexBasic = 7;
                                CurrentTypeDetails2 =  8;
                                CurrentAmountDetails2 = 20;
                            }
                            // For Napalm Launcher
                            if (ItemData[0][CurrentItem].Actor == "RLNapalmLauncher")
                            {
                                CurrentIndexBasic = 7;
                                CurrentTypeDetails2 =  8;
                                CurrentAmountDetails2 = 30;
                            }
                            // For Minigun
                            if (ItemData[0][CurrentItem].Actor == "RLMinigun")
                            {
                                CurrentIndexBasic = 5;
                                CurrentAmountDetails1 = 60;
                                CurrentAmountDetails2 = 40;
                                CurrentAmountDetails3 = 20;
                            }
                            // For Super Shotgun
                            if (ItemData[0][CurrentItem].Actor == "RLSuperShotgun")
                            {
                                CurrentIndexBasic = 4;
                            }
                            // For Laser Rifle
                            if (ItemData[0][CurrentItem].Actor == "RLLaserRifle")
                            {
                                CurrentIndexBasic = 10;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails3 =  11;
                                CurrentTypeDetails6 =  30;
                                CurrentAmountDetails6 = 2;
                            }
                            // For Tristar Blaster
                            if (ItemData[0][CurrentItem].Actor == "RLTristarBlaster")
                            {
                                CurrentIndexBasic = 9;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails3 =  11;
                                CurrentTypeDetails6 =  30;
                                CurrentAmountDetails6 = 2;
                            }

                            // For Nuclear Plasma Pistol
                            if (ItemData[0][CurrentItem].Actor == "RLNuclearPlasmaPistol")
                            {
                                CurrentIndexBasic = 1;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails3 =  11;
                                CurrentTypeDetails5 =  16;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Nuclear Plasma Rifle
                            if (ItemData[0][CurrentItem].Actor == "RLNuclearPlasmaRifle")
                            {
                                CurrentIndexBasic = 8;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails3 =  11;
                                CurrentTypeDetails5 =  16;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Nuclear BFG9000
                            if (ItemData[0][CurrentItem].Actor == "RLNuclearBFG9000")
                            {
                                CurrentIndexBasic = 9;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails3 =  11;
                                CurrentTypeDetails5 =  16;
                                CurrentAmountDetails5 = 1;
                            }
                        }
                        // For Unique Rare
                        if (CurrentRare == 1)
                        {
                            // Set Default Current Type Details
                            CurrentTypeDetails4 =  11;
                            CurrentAmountDetails4 = 1;

                            // Calculate Current Cost
                            CurrentCost = ((Item->Price - Item->Price * Player.ShopDiscount / 100) / 2) / 250 * 250;

                            // Calculate Amount Details
                            CurrentAmountDetails1 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 30, 120)) / 5 * 5;
                            CurrentAmountDetails2 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 20, 60)) / 5 * 5;
                            CurrentAmountDetails3 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 10, 40)) / 5 * 5;

                            // Set Individual Required
                            // For Grammaton Cleric Beretta
                            if (ItemData[0][CurrentItem].Actor == "RLGrammatonClericBeretta")
                            {
                                CurrentIndexBasic = 11;
                            }
                            // For Widowmaker SMG
                            if (ItemData[0][CurrentItem].Actor == "RLWidowmakerSMG")
                            {
                                CurrentIndexBasic = 6;
                                CurrentAmountDetails1 = 45;
                                CurrentAmountDetails2 = 30;
                                CurrentAmountDetails3 = 15;
                            }
                            // For Revenant's Launcher
                            if (ItemData[0][CurrentItem].Actor == "RLRevenantsLauncher")
                            {
                                CurrentIndexBasic = -1;
                                CurrentTypeDetails1 =  12;
                                CurrentTypeDetails2 =  8;
                                CurrentAmountDetails1 = 40;
                                CurrentAmountDetails2 = 20;
                                CurrentAmountDetails3 = 20;
                            }
                            // For Plasma Redirection Cannon
                            if (ItemData[0][CurrentItem].Actor == "RLPlasmaRedirectionCannon")
                            {
                                CurrentIndexBasic = 9;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails3 =  11;
                                CurrentTypeDetails5 =  13;
                                CurrentTypeDetails6 =  30;
                                CurrentAmountDetails5 = 1;
                                CurrentAmountDetails6 = 2;
                            }
                            // For Trigun
                            if (ItemData[0][CurrentItem].Actor == "RLTrigun")
                            {
                                CurrentIndexBasic = 75;
                            }
                            // For Chameleon Rifle
                            if (ItemData[0][CurrentItem].Actor == "RLChameleonRifle")
                            {
                                CurrentIndexBasic = 98;
                                CurrentTypeDetails3 =  11;
                                CurrentTypeDetails5 =  11;
                                CurrentAmountDetails1 = 40;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Frag Shotgun
                            if (ItemData[0][CurrentItem].Actor == "RLFragShotgun")
                            {
                                CurrentIndexBasic = 2;
                                CurrentAmountDetails1 = 50;
                                CurrentAmountDetails2 = 30;
                                CurrentAmountDetails3 = 20;
                            }
                            // For Railgun
                            if (ItemData[0][CurrentItem].Actor == "RLRailgun")
                            {
                                CurrentIndexBasic = 8;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails3 =  11;
                                CurrentTypeDetails6 =  30;
                                CurrentAmountDetails6 = 1;
                                CurrentAmountDetails1 = 50;
                                CurrentAmountDetails2 = 30;
                                CurrentAmountDetails3 = 20;
                            }
                            // For Hellsing ARMS Casull
                            if (ItemData[0][CurrentItem].Actor == "RLHellsingARMSCasull")
                            {
                                CurrentIndexBasic = 13;
                                CurrentTypeDetails5 =  14;
                                CurrentAmountDetails5 = 1;
                            }
                            // For MA-75B Assault Rifle
                            if (ItemData[0][CurrentItem].Actor == "RLMarathonAssaultRifle")
                            {
                                CurrentIndexBasic = 6;
                                CurrentTypeDetails5 =  11;
                                CurrentAmountDetails5 = 1;
                            }
                            // For WSTE-M5 Shotgun
                            if (ItemData[0][CurrentItem].Actor == "RLMarathonShotguns")
                            {
                                CurrentIndexBasic = 4;
                                CurrentTypeDetails5 =  11;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Jackhammer
                            if (ItemData[0][CurrentItem].Actor == "RLJackhammer")
                            {
                                CurrentIndexBasic = 15;
                                CurrentAmountDetails1 = 60;
                                CurrentAmountDetails2 = 40;
                                CurrentAmountDetails3 = 20;
                            }
                            // For Steel Beast
                            if (ItemData[0][CurrentItem].Actor == "RLSteelBeast")
                            {
                                CurrentIndexBasic = 89;
                                CurrentTypeDetails5 =  11;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Quantum Tantrum Cannon
                            if (ItemData[0][CurrentItem].Actor == "RLTantrumCannon")
                            {
                                CurrentIndexBasic = 101;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails3 =  11;
                                CurrentTypeDetails5 =  12;
                                CurrentTypeDetails6 =  29;
                                CurrentAmountDetails5 = 1;
                                CurrentAmountDetails6 = 6;
                            }
                            // For Mysterious Magnum
                            if (ItemData[0][CurrentItem].Actor == "RLMysteriousMagnum")
                            {
                                CurrentIndexBasic = 75;
                            }
                            // For Unknown Herald
                            if (ItemData[0][CurrentItem].Actor == "RLUnknownHerald")
                            {
                                CurrentIndexBasic = 12;
                                CurrentTypeDetails2 =  8;
                                CurrentTypeDetails5 =  12;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Nanomachic Armament Generator
                            if (ItemData[0][CurrentItem].Actor == "RLNanomachicArmamentGenerator")
                            {
                                CurrentIndexBasic = 199;
                                CurrentTypeDetails5 =  11;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Suss Gun
                            if (ItemData[0][CurrentItem].Actor == "RLSussGun")
                            {
                                CurrentIndexBasic = 20;
                                CurrentTypeDetails5 =  12;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails3 =  11;
                                CurrentAmountDetails5 = 1;
                                CurrentTypeDetails6 =  30;
                                CurrentAmountDetails6 = 4;
                            }
                            // For Charch's Null Pointer
                            if (ItemData[0][CurrentItem].Actor == "RLNullPointer")
                            {
                                CurrentIndexBasic = 41;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails3 =  11;
                                CurrentTypeDetails5 =  14;
                                CurrentAmountDetails5 = 1;
                                CurrentTypeDetails6 =  30;
                                CurrentAmountDetails6 = 1;
                            }
                            // For MIRV Launcher
                            if (ItemData[0][CurrentItem].Actor == "RLMIRVLauncher")
                            {
                                CurrentIndexBasic = 18;
                                CurrentTypeDetails2 =  8;
                                CurrentTypeDetails5 =  12;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Anti-Freak Jackal
                            if (ItemData[0][CurrentItem].Actor == "RLAntiFreakJackal")
                            {
                                CurrentIndexBasic = 104;
                                CurrentTypeDetails5 =  14;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Particle Beam Cannon
                            if (ItemData[0][CurrentItem].Actor == "RLParticleBeamCannon")
                            {
                                CurrentIndexBasic = 41;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails3 =  11;
                                CurrentTypeDetails5 =  13;
                                CurrentAmountDetails5 = 1;
                                CurrentTypeDetails6 =  30;
                                CurrentAmountDetails6 = 2;
                            }
                            // For BFG10k
                            if (ItemData[0][CurrentItem].Actor == "RLBFG10k")
                            {
                                CurrentIndexBasic = 9;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails3 =  11;
                                CurrentTypeDetails5 =  16;
                                CurrentAmountDetails5 = 1;
                                CurrentTypeDetails6 =  30;
                                CurrentAmountDetails6 = 2;
                            }
                            // For Quad Shotgun
                            if (ItemData[0][CurrentItem].Actor == "RLQuadShotgun")
                            {
                                CurrentIndexBasic = 91;
                                CurrentTypeDetails5 =  11;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Lightweaver
                            if (ItemData[0][CurrentItem].Actor == "RLLightweaver")
                            {
                                CurrentIndexBasic = 101;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails3 =  11;
                                CurrentTypeDetails5 =  14;
                                CurrentAmountDetails5 = 1;
                                CurrentTypeDetails6 =  29;
                                CurrentAmountDetails6 = 6;
                            }
                            // For Lucifer Cannon
                            if (ItemData[0][CurrentItem].Actor == "RLLuciferCannon")
                            {
                                CurrentIndexBasic = 41;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails3 =  11;
                                CurrentTypeDetails5 =  14;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Direct Hit
                            if (ItemData[0][CurrentItem].Actor == "RLDirectHit")
                            {
                                CurrentIndexBasic = 18;
                                CurrentTypeDetails2 =  8;
                                CurrentTypeDetails5 =  12;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Nuclear Onslaught
                            if (ItemData[0][CurrentItem].Actor == "RLNuclearOnslaught")
                            {
                                CurrentIndexBasic = 25;
                                CurrentTypeDetails2 =  10;
                                CurrentTypeDetails3 =  11;
                                CurrentTypeDetails5 =  16;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Triad Cannon
                            if (ItemData[0][CurrentItem].Actor == "RLTriadCannon")
                            {
                                CurrentIndexBasic = 53;
                                CurrentTypeDetails2 =  8;
                                CurrentTypeDetails5 =  12;
                                CurrentAmountDetails5 = 1;
                            }
                        }
                    }
                    // For Armor
                    if (CurrentCategory == 1)
                    {
                        // Set Default Current Type Details
                        CurrentTypeDetails1 =  9;
                        CurrentTypeDetails2 =  10;
                        CurrentTypeDetails3 =  11;

                        // Set Global Required
                        // For Exotic Rare
                        if (CurrentRare == 0)
                        {
                            // Calculate Current Cost
                            CurrentCost = ((Item->Price - Item->Price * Player.ShopDiscount / 100) / 6) / 250 * 250;

                            // Calculate Amount Details
                            CurrentAmountDetails1 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 15, 35)) / 5 * 5;
                            CurrentAmountDetails2 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 10, 20)) / 5 * 5;
                            CurrentAmountDetails3 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 5, 10)) / 5 * 5;

                            // Set Individual Required
                            // For Medical Armor
                            if (ItemData[3][CurrentItem].Actor == "RLMedicalArmorPickup")
                            {
                                CurrentIndexBasic = 1;
                                CurrentTypeDetails2 = 0;
                                CurrentAmountDetails1 = 10;
                                CurrentAmountDetails2 = 15;
                            }
                            // For Overcharge System
                            if (ItemData[3][CurrentItem].Actor == "RLOverchargeSystemArmorPickup")
                            {
                                CurrentAmountDetails1 = 10;
                                CurrentTypeDetails6 = 29;
                                CurrentAmountDetails6 = 2;
                            }
                            // For Repulsion Wave Suit
                            if (ItemData[3][CurrentItem].Actor == "RLRepulsionWaveSuitArmorPickup")
                            {
                                CurrentIndexBasic = 1;
                                CurrentTypeDetails6 = 29;
                                CurrentAmountDetails1 = 15;
                                CurrentAmountDetails2 = 5;
                                CurrentAmountDetails3 = 5;
                                CurrentAmountDetails6 = 1;
                            }
                            // For Ballistic Vest
                            if (ItemData[3][CurrentItem].Actor == "RLBallisticVestArmorPickup")
                            {
                                CurrentIndexBasic = 1;
                                CurrentAmountDetails1 = 10;
                                CurrentAmountDetails2 = 0;
                                CurrentAmountDetails3 = 0;
                            }
                            // For Duelist Armor
                            if (ItemData[3][CurrentItem].Actor == "RLDuelistArmorPickup")
                            {
                                CurrentIndexBasic = 1;
                                CurrentAmountDetails1 = 15;
                                CurrentAmountDetails2 = 0;
                                CurrentAmountDetails3 = 0;
                            }
                            // For Survival MediArmor
                            if (ItemData[3][CurrentItem].Actor == "RLSurvivalMediArmorPickup")
                            {
                                CurrentRank = 3;
                                CurrentIndexBasic = 3;
                                CurrentTypeDetails2 = 0;
                                CurrentTypeDetails6 = 29;
                                CurrentAmountDetails1 = 10;
                                CurrentAmountDetails2 = 20;
                                CurrentAmountDetails6 = 1;
                            }
                            // For Cyberwarrior Armor
                            if (ItemData[3][CurrentItem].Actor == "RLCyberwarriorArmorPickup")
                            {
                                CurrentRank = 3;
                                CurrentIndexBasic = 3;
                                CurrentTypeDetails6 = 29;
                                CurrentAmountDetails6 = 1;
                            }
                            // For Bulletproof Vest
                            if (ItemData[3][CurrentItem].Actor == "RLBulletproofVestArmorPickup")
                            {
                                CurrentRank = 4;
                                CurrentIndexBasic = 1;
                                CurrentAmountDetails2 = 0;
                            }
                            // For Energy-Shielded Vest
                            if (ItemData[3][CurrentItem].Actor == "RLEnergyShieldedVestArmorPickup")
                            {
                                CurrentRank = 4;
                                CurrentIndexBasic = 2;
                                CurrentTypeDetails6 = 29;
                                CurrentAmountDetails1 = 10;
                                CurrentAmountDetails2 = 10;
                                CurrentAmountDetails6 = 1;
                            }
                            // For Onyx Armor
                            if (ItemData[3][CurrentItem].Actor == "RLOnyxArmorPickup")
                            {
                                CurrentIndexBasic = 1;
                                CurrentAmountDetails2 = 0;
                                CurrentAmountDetails3 *= 2;
                            }
                            // For Ballistic Shield
                            if (ItemData[3][CurrentItem].Actor == "RLBallisticShieldArmorPickup")
                            {
                                CurrentIndexBasic = 4;
                                CurrentTypeDetails6 = 29;
                                CurrentAmountDetails6 = 1;
                            }
                            // For Plasma Shield
                            if (ItemData[3][CurrentItem].Actor == "RLPlasmaShieldArmorPickup")
                            {
                                CurrentIndexBasic = 5;
                                CurrentTypeDetails6 = 29;
                                CurrentAmountDetails1 = 20;
                                CurrentAmountDetails2 = 25;
                                CurrentAmountDetails6 = 2;
                            }
                            // For Energy Shield
                            if (ItemData[3][CurrentItem].Actor == "RLEnergyShieldArmorPickup")
                            {
                                CurrentIndexBasic = 2;
                                CurrentTypeDetails6 = 29;
                                CurrentAmountDetails1 = 15;
                                CurrentAmountDetails2 = 20;
                                CurrentAmountDetails6 = 1;
                            }
                            // For Gothic Armor
                            if (ItemData[3][CurrentItem].Actor == "RLGothicArmorPickup")
                            {
                                CurrentIndexBasic = 4;
                                CurrentAmountDetails2 = 0;
                                CurrentAmountDetails3 *= 2;
                            }

                            // For Phaseshift Armor
                            if (ItemData[3][CurrentItem].Actor == "RLPhaseshiftArmorPickup")
                            {
                                CurrentIndexBasic = 4;
                                CurrentTypeDetails6 = 29;
                                CurrentAmountDetails1 = 25;
                                CurrentAmountDetails2 = 15;
                                CurrentAmountDetails3 = 15;
                                CurrentAmountDetails6 = 1;
                            }
                        }
                        // For Unique Rare
                        if (CurrentRare == 1)
                        {
                            // Set Default Current Craft Part
                            CurrentTypeDetails4 =  11;
                            CurrentAmountDetails4 = 1;

                            // Calculate Current Cost
                            CurrentCost = (Item->Price - Item->Price * Player.ShopDiscount / 100) / 250 * 250;

                            // Calculate Amount Details
                            CurrentAmountDetails1 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 30, 80)) / 5 * 5;
                            CurrentAmountDetails2 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 20, 40)) / 5 * 5;
                            CurrentAmountDetails3 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 10, 20)) / 5 * 5;

                            // Set Individual Required
                            // For Geo's Golden Gauntlet
                            if (ItemData[3][CurrentItem].Actor == "RLGeosGoldenGauntletArmorPickup")
                            {
                                CurrentRank = 4;
                                CurrentCost = (CurrentCost / 6) / 250 * 250;
                                CurrentTypeDetails2 = 3;
                                CurrentTypeDetails5 =  10;
                                CurrentAmountDetails1 = 10;
                                CurrentAmountDetails3 = 0;
                                CurrentAmountDetails4 = 0;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Necro Armor
                            if (ItemData[3][CurrentItem].Actor == "RLNecroArmorPickup")
                            {
                                CurrentCost = (CurrentCost / 4) / 250 * 250;
                                CurrentIndexBasic = 4;
                                CurrentTypeDetails2 = 12;
                                CurrentAmountDetails1 = 15;
                                CurrentAmountDetails2 = 30;
                                CurrentAmountDetails4 = 0;
                            }
                            // Tactical Assembler Suit
                            if (ItemData[3][CurrentItem].Actor == "RLTacticalAssemblerSuitArmorPickup")
                            {
                                CurrentCost = (CurrentCost / 5) / 250 * 250;
                                CurrentIndexBasic = 6;
                                CurrentTypeDetails2 = 0;
                                CurrentAmountDetails1 = 15;
                                CurrentAmountDetails2 = 20;
                                CurrentAmountDetails3 = 15;
                                CurrentAmountDetails4 = 0;
                            }
                            // For Frontline Engineer Suit
                            if (ItemData[3][CurrentItem].Actor == "RLFrontlineEngineerSuitArmorPickup")
                            {
                                CurrentCost = (CurrentCost / 5) / 250 * 250;
                                CurrentIndexBasic = 5;
                                CurrentAmountDetails1 = 15;
                                CurrentAmountDetails2 = 20;
                                CurrentAmountDetails3 = 15;
                                CurrentAmountDetails4 = 0;
                            }
                            // For Phoenix-B Device Suit
                            if (ItemData[3][CurrentItem].Actor == "RLBlazingPhoenixDeviceSuitArmorPickup")
                            {
                                CurrentCost = (CurrentCost / 5) / 250 * 250;
                                CurrentIndexBasic = 19;
                                CurrentTypeDetails6 = 30;
                                CurrentAmountDetails1 = 25;
                                CurrentAmountDetails2 = 15;
                                CurrentAmountDetails3 = 10;
                                CurrentAmountDetails4 = 0;
                                CurrentAmountDetails6 = 1;
                            }
                            // For Energy Discharge Harness
                            if (ItemData[3][CurrentItem].Actor == "RLEnergyDischargeHarnessArmorPickup")
                            {
                                CurrentIndexBasic = 31;
                                CurrentTypeDetails5 =  12;
                                CurrentTypeDetails6 = 30;
                                CurrentAmountDetails5 = 1;
                                CurrentAmountDetails6 = 1;
                            }
                            // For Medical Power Armor
                            if (ItemData[3][CurrentItem].Actor == "RLMedicalPowerArmorPickup")
                            {
                                CurrentIndexBasic = 29;
                                CurrentTypeDetails2 = 0;
                                CurrentAmountDetails2 *= 2;
                            }
                            // For Roysten's Command Armor
                            if (ItemData[3][CurrentItem].Actor == "RLRoystensCommandArmorPickup")
                            {
                                CurrentIndexBasic = 33;
                                CurrentAmountDetails2 = 0;
                                CurrentAmountDetails3 *= 2;
                                CurrentTypeDetails5 =  11;
                                CurrentAmountDetails5 = 1;
                            }
                            // For R11-n Psychic Amplifier Suit
                            if (ItemData[3][CurrentItem].Actor == "RLRyanCordellPsychicAmplifierSuitArmorPickup")
                            {
                                CurrentIndexBasic = 18;
                                CurrentTypeDetails6 = 30;
                                CurrentAmountDetails6 = 1;
                            }
                            // For Berserk Powersuit
                            if (ItemData[3][CurrentItem].Actor == "RLBerserkPowersuitArmorPickup")
                            {
                                CurrentIndexBasic = 35;
                                CurrentTypeDetails5 =  10;
                                CurrentTypeDetails6 = 30;
                                CurrentAmountDetails5 = 1;
                                CurrentAmountDetails6 = 1;
                            }
                            // For WW-41 Peacekeeper Armor
                            if (ItemData[3][CurrentItem].Actor == "RLWildWeaselPeacekeeperArmorPickup")
                            {
                                CurrentCost = (CurrentCost / 5) / 250 * 250;
                                CurrentIndexBasic = 33;
                                CurrentTypeDetails5 =  11;
                                CurrentAmountDetails1 = 30;
                                CurrentAmountDetails2 = 0;
                                CurrentAmountDetails3 *= 2;
                                CurrentAmountDetails4 = 0;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Cybernetic Armor
                            if (ItemData[3][CurrentItem].Actor == "RLCyberneticArmorPickup")
                            {
                                CurrentIndexBasic = 26;
                                CurrentTypeDetails5 =  11;
                                CurrentTypeDetails6 = 30;
                                CurrentAmountDetails2 *= 2;
                                CurrentAmountDetails5 = 1;
                                CurrentAmountDetails6 = 2;
                            }
                            // For Nano Ablative Armor
                            if (ItemData[3][CurrentItem].Actor == "RLNanoAblativeArmorPickup")
                            {
                                CurrentCost = (CurrentCost / 4) / 250 * 250;
                                CurrentIndexBasic = 15;
                                CurrentTypeDetails2 = 3;
                                CurrentAmountDetails1 = 20;
                                CurrentAmountDetails2 = 30;
                                CurrentAmountDetails3 = 10;
                                CurrentAmountDetails4 = 0;
                            }
                            // For Shielded Armor
                            if (ItemData[3][CurrentItem].Actor == "RLShieldedArmorPickup")
                            {
                                CurrentIndexBasic = 19;
                                CurrentTypeDetails6 = 30;
                                CurrentAmountDetails6 = 1;
                                CurrentTypeDetails5 =  11;
                                CurrentAmountDetails5 = 1;
                            }
                            // Terminus Battlesuit
                            if (ItemData[3][CurrentItem].Actor == "RLTerminusEst13BattlesuitArmorPickup")
                            {
                                CurrentIndexBasic = 18;
                                CurrentTypeDetails2 =  8;
                                CurrentTypeDetails5 =  12;
                                CurrentAmountDetails5 = 1;
                            }
                            // For 0D-1a Assaultforce Armor
                            if (ItemData[3][CurrentItem].Actor == "RLZeroDiamondAssaultforceArmorPickup")
                            {
                                CurrentIndexBasic = 18;
                                CurrentTypeDetails2 =  6;
                                CurrentTypeDetails5 =  11;
                                CurrentAmountDetails5 = 1;
                            }
                            // For K-8 Matterstorm Harness
                            if (ItemData[3][CurrentItem].Actor == "RLKateMatterstormHarnessArmorPickup")
                            {
                                CurrentIndexBasic = 19;
                                CurrentTypeDetails2 =  3;
                                CurrentTypeDetails5 =  13;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Jetpack
                            if (ItemData[3][CurrentItem].Actor == "RLJetpackArmorPickup")
                            {
                                CurrentIndexBasic = 1;
                                CurrentTypeDetails6 =  30;
                                CurrentAmountDetails6 = 2;
                            }
                            // For Solo Operative Suit
                            if (ItemData[3][CurrentItem].Actor == "RLSoloOperativeSuitArmorPickup")
                            {
                                CurrentIndexBasic = 18;
                                CurrentTypeDetails5 =  11;
                                CurrentAmountDetails5 = 1;
                                CurrentTypeDetails6 = 30;
                                CurrentAmountDetails6 = 1;
                            }
                            // For Prototype Assault Shield
                            if (ItemData[3][CurrentItem].Actor == "RLPrototypeAssaultShieldArmorPickup")
                            {
                                CurrentIndexBasic = 41;
                                CurrentTypeDetails6 = 30;
                                CurrentAmountDetails6 = 2;
                            }
                            // For Malek's Armor
                            if (ItemData[3][CurrentItem].Actor == "RLMaleksArmorPickup")
                            {
                                CurrentIndexBasic = 20;
                                CurrentTypeDetails5 =  12;
                                CurrentTypeDetails6 =  30;
                                CurrentAmountDetails5 = 1;
                                CurrentAmountDetails6 = 2;
                            }
                            // For K1-L3 Teslabolt Armor
                            if (ItemData[3][CurrentItem].Actor == "RLKyleTeslaboltArmorPickup")
                            {
                                CurrentIndexBasic = 19;
                                CurrentTypeDetails5 =  14;
                                CurrentTypeDetails6 =  30;
                                CurrentAmountDetails5 = 1;
                                CurrentAmountDetails6 = 2;
                            }
                            // For Torgue Blastplate Armor
                            if (ItemData[3][CurrentItem].Actor == "RLTorgueBlastplateArmorPickup")
                            {
                                CurrentIndexBasic = 20;
                                CurrentTypeDetails2 =  8;
                                CurrentTypeDetails5 =  12;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Lava Armor
                            if (ItemData[3][CurrentItem].Actor == "RLLavaArmorPickup")
                            {
                                CurrentIndexBasic = 20;
                                CurrentTypeDetails2 = 3;
                                CurrentTypeDetails5 =  12;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Nuclear Armor
                            if (ItemData[3][CurrentItem].Actor == "RLNuclearPowerArmorPickup")
                            {
                                CurrentIndexBasic = 20;
                                CurrentTypeDetails5 =  16;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Xaser Inc. Powerarmor
                            if (ItemData[3][CurrentItem].Actor == "RLXaserPowerarmorPickup")
                            {
                                CurrentIndexBasic = 19;
                                CurrentTypeDetails5 =  13;
                                CurrentAmountDetails5 = 1;
                                CurrentTypeDetails6 =  30;
                                CurrentAmountDetails6 = 2;
                            }
                        }
                    }

                    // For Boots
                    if (CurrentCategory == 2)
                    {
                        // Set Default Current Type Details
                        CurrentTypeDetails1 =  9;
                        CurrentTypeDetails2 =  10;
                        CurrentTypeDetails3 =  11;

                        // Set Global Required
                        // For Exotic Rare
                        if (CurrentRare == 0)
                        {
                            // Calculate Current Cost
                            CurrentCost = ((Item->Price - Item->Price * Player.ShopDiscount / 100) / 3) / 250 * 250;

                            // Calculate Amount Details
                            CurrentAmountDetails1 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 15, 60)) / 5 * 5;
                            if (CurrentRank >= 5)
                            {
                                // Set Default Current Craft Part
                                CurrentTypeDetails4 =  10;
                                CurrentAmountDetails4 = 1;

                                // Calculate Amount Details
                                CurrentAmountDetails2 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 10, 30)) / 5 * 5;
                            }
                            if (CurrentRank >= 7)
                            {
                                // Calculate Amount Details
                                CurrentAmountDetails3 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 5, 15)) / 5 * 5;
                            }

                            // Set Individual Required
                            // For Acid-Proof Boots
                            if (ItemData[9][CurrentItem].Actor == "RLAcidProofBootsPickup")
                            {
                                CurrentIndexBasic = 8;
                                CurrentTypeDetails5 =  15;
                                CurrentAmountDetails5 = 1;
                                CurrentTypeDetails6 = 29;
                                CurrentAmountDetails6 = 2;
                            }
                            // For Gothic Boots
                            if (ItemData[9][CurrentItem].Actor == "RLGothicBootsPickup")
                            {
                                CurrentIndexBasic = 1;
                                CurrentAmountDetails2 = 0;
                                CurrentAmountDetails3 *= 2;
                            }
                            // For Phaseshift Boots
                            if (ItemData[9][CurrentItem].Actor == "RLPhaseshiftBootsPickup")
                            {
                                CurrentIndexBasic = 2;
                                CurrentTypeDetails6 = 29;
                                CurrentAmountDetails6 = 2;
                            }
                            // For Shockwave Boots
                            if (ItemData[9][CurrentItem].Actor == "RLShockwaveBootsPickup")
                            {
                                CurrentIndexBasic = 0;
                                CurrentTypeDetails6 = 29;
                                CurrentAmountDetails6 = 2;
                            }
                        }
                        // For Unique Rare
                        if (CurrentRare == 1)
                        {
                            // Calculate Current Cost
                            CurrentCost = ((Item->Price - Item->Price * Player.ShopDiscount / 100) / 3) / 250 * 250;

                            // Calculate Amount Details
                            CurrentTypeDetails4 =  11;
                            CurrentAmountDetails4 = 1;

                            CurrentAmountDetails1 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 30, 80)) / 5 * 5;
                            CurrentAmountDetails2 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 20, 40)) / 5 * 5;
                            CurrentAmountDetails3 = (int)(Curve(Item->Price / 20, CurrentCostMin, CurrentCostMax, 10, 20)) / 5 * 5;

                            // Set Individual Required
                            // For Enviro Boots
                            if (ItemData[9][CurrentItem].Actor == "RLEnviroBootsPickup")
                            {
                                CurrentIndexBasic = 8;
                                CurrentTypeDetails5 =  15;
                                CurrentAmountDetails5 = 1;
                                CurrentTypeDetails6 = 29;
                                CurrentAmountDetails6 = 2;
                            }
                            // For Frontline Engineer Boots
                            if (ItemData[9][CurrentItem].Actor == "RLFrontlineEngineerBootsPickup")
                            {
                                CurrentIndexBasic = 2;
                                CurrentTypeDetails5 =  13;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Lava Boots
                            if (ItemData[9][CurrentItem].Actor == "RLLavaBootsPickup")
                            {
                                CurrentIndexBasic = 2;
                                CurrentTypeDetails5 =  12;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Leonidas' Boots
                            if (ItemData[9][CurrentItem].Actor == "RLLeonidasBootsPickup")
                            {
                                CurrentIndexBasic = 2;
                                CurrentTypeDetails5 =  10;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Nuclear Boots
                            if (ItemData[9][CurrentItem].Actor == "RLNuclearPowerBootsPickup")
                            {
                                CurrentIndexBasic = 2;
                                CurrentTypeDetails5 =  16;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Nyarlaptotep's Boots
                            if (ItemData[9][CurrentItem].Actor == "RLNyarlaptotepsBootsPickup")
                            {
                                CurrentIndexBasic = 2;
                                CurrentTypeDetails5 =  12;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Roysten's Combat Boots
                            if (ItemData[9][CurrentItem].Actor == "RLRoystensCombatBootsPickup")
                            {
                                CurrentIndexBasic = 2;
                                CurrentTypeDetails5 =  11;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Solo Operative Boots
                            if (ItemData[9][CurrentItem].Actor == "RLSoloOperativeBootsPickup")
                            {
                                CurrentIndexBasic = 2;
                                CurrentTypeDetails5 =  11;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Tactical Assembler Boots
                            if (ItemData[9][CurrentItem].Actor == "RLTacticalAssemblerBootsPickup")
                            {
                                CurrentIndexBasic = 2;
                                CurrentTypeDetails5 =  12;
                                CurrentAmountDetails5 = 1;
                            }
                            // For Torgue Blastboots
                            if (ItemData[9][CurrentItem].Actor == "RLTorgueBlastBootsPickup")
                            {
                                CurrentIndexBasic = 2;
                                CurrentTypeDetails5 =  12;
                                CurrentAmountDetails5 = 1;
                            }
                        }
                    }
                    // Draw the background
                    if (GetCVar("drpg_menudim"))
                        FadeRange(0, 0, 0, 0.65, 0, 0, 0, 0.0, 0.25);

                    // Set the HUD Size
                    SetHudSize(GetActivatorCVar("drpg_menu_width"), GetActivatorCVar("drpg_menu_height"), true);

                    // Draw Border
                    // These are pushed back a bit so the border doesn't overlap anything
                    if (GetActivatorCVar("drpg_menu_background_border"))
                        DrawBorder("Bor", -1, 8, -5.0, 0.0, 470, 470);

                    // Text
                    SetFont("BIGFONT");
                    HudMessage("\CdDisassembling Device\C-");
                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID, "White", X + 108.0, Y + 16.0, 0.05, 0.05);

                    SetFont("BIGFONT");
                    HudMessage("Select item for assembling:");
                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 1, "White", X + 64.0, Y + 56.0, 0.05, 0.05);

                    SetFont("BIGFONT");
                    HudMessage("Category: %S", CategoriesNames[CurrentCategory]);
                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 2, "White", X + 124.0, Y + 96.0, 0.05, 0.05);

                    SetFont("BIGFONT");
                    HudMessage("Rare: %S", RareNames[CurrentRare]);
                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 3, "White", X + 164.0, Y + 132.0, 0.05, 0.05);

                    PrintSprite(ItemData[CategoriesData[CurrentCategory]][CurrentItem].Sprite.Name, 0, X + 240.0,  Y + 212.0, 0.05);

                    SetFont("BIGFONT");
                    HudMessage("Item: %S", ItemData[CategoriesData[CurrentCategory]][CurrentItem].Name);
                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 4, "White", X + 32.0, Y + 240.0, 0.05, 0.05);

                    SetFont("SMALLFONT");
                    HudMessage("Required \Cddetails\C-:");
                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 5, "White", X + 32.0, Y + 268.0, 0.05, 0.05);

                    // Required Details
                    // For Details #1
                    if (CurrentAmountDetails1 > 0)
                    {
                        SetFont("SMALLFONT");
                        HudMessage("%S: %d", ItemData[7][CurrentTypeDetails1].Name, CurrentAmountDetails1);
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 6, "White", X + 32.0, Y + 276.0, 0.05, 0.05);
                    }
                    // For Details #2
                    if (CurrentAmountDetails2 > 0)
                    {
                        SetFont("SMALLFONT");
                        HudMessage("%S: %d", ItemData[7][CurrentTypeDetails2].Name, CurrentAmountDetails2);
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 7, "White", X + 32.0, Y + 284.0, 0.05, 0.05);
                    }
                    // For Details #3
                    if (CurrentAmountDetails3 > 0)
                    {
                        SetFont("SMALLFONT");
                        HudMessage("%S: %d", ItemData[7][CurrentTypeDetails3].Name, CurrentAmountDetails3);
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 8, "White", X + 32.0, Y + (CurrentAmountDetails2 > 0 ? 292.0 : 284.0), 0.05, 0.05);
                    }
                    // For Additional Details
                    if (CurrentAmountDetails4 > 0 || CurrentAmountDetails5 > 0 || CurrentAmountDetails6 > 0)
                    {
                        SetFont("SMALLFONT");
                        HudMessage("Required \Cqadditional details\C-:");
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 9, "White", X + 32.0, Y + 308.0, 0.05, 0.05);

                        // Set additional coordinates
                        if (CurrentAmountDetails5 > 0 && CurrentAmountDetails4 + CurrentAmountDetails6 > 0)
                            Y2 += 8.0;
                        if (CurrentAmountDetails6 > 0 && CurrentAmountDetails4 + CurrentAmountDetails5 > 0)
                            Y2 += 8.0;
                    }
                    // For Details #4 (Additional)
                    if (CurrentAmountDetails4 > 0)
                    {
                        SetFont("SMALLFONT");
                        HudMessage("%S: %d", ItemData[8][CurrentTypeDetails4].Name, CurrentAmountDetails4);
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 10, "White", X + 32.0, Y + Y1 + 316.0, 0.05, 0.05);

                        PrintSprite(ItemData[8][CurrentTypeDetails4].Sprite.Name, 0, X + X1 + 32.0,  Y + Y2 + 315.0, 0.05);

                        X1 += 44.0;
                        Y1 += 8.0;
                    }
                    // For Details #5 (Additional)
                    if (CurrentAmountDetails5 > 0)
                    {
                        SetFont("SMALLFONT");
                        HudMessage("%S: %d", ItemData[4][CurrentTypeDetails5].Name, CurrentAmountDetails5);
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 11, "White", X + 32.0, Y + Y1 + 316.0, 0.05, 0.05);

                        PrintSprite(ItemData[4][CurrentTypeDetails5].Sprite.Name, 0, X + X1 + 48.0,  Y + Y2 + 376.0, 0.05);

                        X1 += 32.0;
                        Y1 += 8.0;
                    }
                    // For Details #6 (Additional)
                    if (CurrentAmountDetails6 > 0)
                    {
                        SetFont("SMALLFONT");
                        HudMessage("%S: %d", ItemData[6][CurrentTypeDetails6].Name, CurrentAmountDetails6);
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 12, "White", X + 32.0, Y + Y1 + 316.0, 0.05, 0.05);

                        PrintSprite(ItemData[6][CurrentTypeDetails6].Sprite.Name, 0, X + X1 + 48.0,  Y + Y2 + 348.0, 0.05);

                        X1 += 32.0;
                        Y1 += 8.0;
                    }

                    SetFont("SMALLFONT");
                    HudMessage("Required \Cfcredits\C-:\n%d \CfC\C-", CurrentCost);
                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 13, "White", X + 272.0, Y + 268.0, 0.05, 0.05);

                    if (CurrentRank > 0)
                    {
                        SetFont("SMALLFONT");
                        HudMessage("Required \CkRank\C-:\n%d \Ck(%S)\C-", CurrentRank, LongRanks[CurrentRank]);
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 14, "White", X + 272.0, Y + 290.0, 0.05, 0.05);
                    }

                    // Required basic Item
                    if (CurrentIndexBasic >= 0)
                    {
                        SetFont("SMALLFONT");
                        HudMessage("Required \Cabasic item\C-:\n%S", StrLeft(ItemData[CategoriesData[CurrentCategory]][CurrentIndexBasic].Name, 26));
                        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 15, "White", X + 272.0, Y + 314.0, 0.05, 0.05);

                        PrintSprite(ItemData[CategoriesData[CurrentCategory]][CurrentIndexBasic].Sprite.Name, 0, X + 336.0,  Y + 372.0, 0.05);
                    }

                    // Input
                    if (CheckInput(BT_FORWARD, KEY_PRESSED, false, PlayerNumber()))
                    {
                        ActivatorSound("menu/move", 127);
                        if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
                        {
                            CurrentRare++;
                            CurrentItem = 0;
                            CurrentIndexBasic = -1;
                            CurrentAmountDetails1 = 0;
                            CurrentAmountDetails2 = 0;
                            CurrentAmountDetails3 = 0;
                            CurrentAmountDetails4 = 0;
                            CurrentAmountDetails5 = 0;
                            CurrentAmountDetails6 = 0;
                            if (CurrentRare > 1) CurrentRare = 0;
                        }
                        else
                        {
                            CurrentCategory++;
                            CurrentItem = 0;
                            CurrentIndexBasic = -1;
                            CurrentAmountDetails1 = 0;
                            CurrentAmountDetails2 = 0;
                            CurrentAmountDetails3 = 0;
                            CurrentAmountDetails4 = 0;
                            CurrentAmountDetails5 = 0;
                            CurrentAmountDetails6 = 0;
                            if (CurrentCategory > 2) CurrentCategory = 0;
                        }

                    }
                    if (CheckInput(BT_BACK, KEY_PRESSED, false, PlayerNumber()))
                    {
                        ActivatorSound("menu/move", 127);
                        if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
                        {
                            CurrentRare--;
                            CurrentItem = 0;
                            CurrentIndexBasic = -1;
                            CurrentAmountDetails1 = 0;
                            CurrentAmountDetails2 = 0;
                            CurrentAmountDetails3 = 0;
                            CurrentAmountDetails4 = 0;
                            CurrentAmountDetails5 = 0;
                            CurrentAmountDetails6 = 0;
                            if (CurrentRare < 0) CurrentRare = 1;
                        }
                        else
                        {
                            CurrentCategory--;
                            CurrentItem = 0;
                            CurrentIndexBasic = -1;
                            CurrentAmountDetails1 = 0;
                            CurrentAmountDetails2 = 0;
                            CurrentAmountDetails3 = 0;
                            CurrentAmountDetails4 = 0;
                            CurrentAmountDetails5 = 0;
                            CurrentAmountDetails6 = 0;
                            if (CurrentCategory < 0) CurrentCategory = 2;
                        }
                    }
                    if (CheckInput(BT_MOVELEFT, KEY_ONLYPRESSED, false, PlayerNumber()))
                    {
                        ActivatorSound("menu/move", 127);
                        CurrentItem--;
                        CurrentIndexBasic = -1;
                        CurrentAmountDetails1 = 0;
                        CurrentAmountDetails2 = 0;
                        CurrentAmountDetails3 = 0;
                        CurrentAmountDetails4 = 0;
                        CurrentAmountDetails5 = 0;
                        CurrentAmountDetails6 = 0;
                        if (CurrentItem < CurrentItemMin) CurrentItem = CurrentItemMax;
                    }
                    if (CheckInput(BT_MOVERIGHT, KEY_ONLYPRESSED, false, PlayerNumber()))
                    {
                        ActivatorSound("menu/move", 127);
                        CurrentItem++;
                        CurrentIndexBasic = -1;
                        CurrentAmountDetails1 = 0;
                        CurrentAmountDetails2 = 0;
                        CurrentAmountDetails3 = 0;
                        CurrentAmountDetails4 = 0;
                        CurrentAmountDetails5 = 0;
                        CurrentAmountDetails6 = 0;
                        if (CurrentItem > CurrentItemMax) CurrentItem = CurrentItemMin;
                    }
                    if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
                    {
                        if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
                        {
                            if ((CurrentIndexBasic >= 0 && CheckInventory(ItemData[CategoriesData[CurrentCategory]][CurrentIndexBasic].Actor) || CurrentIndexBasic < 0) && Player.RankLevel >= CurrentRank && CheckInventory("DRPGCredits") >= CurrentCost && CheckInventory(ItemData[7][CurrentTypeDetails1].Actor) >= CurrentAmountDetails1 &&  CheckInventory(ItemData[7][CurrentTypeDetails2].Actor) >= CurrentAmountDetails2
                                    && CheckInventory(ItemData[7][CurrentTypeDetails3].Actor) >= CurrentAmountDetails3 && CheckInventory(ItemData[8][CurrentTypeDetails4].Actor) >= CurrentAmountDetails4 && CheckInventory(ItemData[4][CurrentTypeDetails5].Actor) >= CurrentAmountDetails5 && CheckInventory(ItemData[6][CurrentTypeDetails6].Actor) >= CurrentAmountDetails6)
                            {
                                Player.OutpostMenu = 0;
                                str ActorToSpawn;

                                // Take Basic Item
                                if (CurrentIndexBasic >= 0)
                                {
                                    // Take Current Basic Item
                                    TakeInventory(ItemData[CategoriesData[CurrentCategory]][CurrentIndexBasic].Actor, 1);

                                    // Take tokens from DoomRL Arsenal
                                    if (CompatMode == COMPAT_DRLA)
                                        RemoveDRLAItem(CategoriesData[CurrentCategory], CurrentIndexBasic);
                                }

                                // Take Details
                                TakeInventory("DRPGCredits", CurrentCost);
                                TakeInventory(ItemData[7][CurrentTypeDetails1].Actor, CurrentAmountDetails1);
                                TakeInventory(ItemData[7][CurrentTypeDetails2].Actor, CurrentAmountDetails2);
                                TakeInventory(ItemData[7][CurrentTypeDetails3].Actor, CurrentAmountDetails3);
                                TakeInventory(ItemData[8][CurrentTypeDetails4].Actor, CurrentAmountDetails4);
                                TakeInventory(ItemData[4][CurrentTypeDetails5].Actor, CurrentAmountDetails5);
                                TakeInventory(ItemData[6][CurrentTypeDetails6].Actor, CurrentAmountDetails6);

                                // The effect of sleep immersion
                                FadeRange(0, 0, 0, 0.5, 0, 0, 0, 1.0, 1.0);
                                Delay(35 * 1);

                                // Set actor to spawn
                                ActorToSpawn = ItemData[CategoriesData[CurrentCategory]][CurrentItem].Actor;
                                if (CompatMode == COMPAT_DRLA)
                                {
                                    // For exotic weapon
                                    if (CurrentCategory == 0 && CurrentRare == 0)
                                        ActorToSpawn = StrParam("%SPickup", ActorToSpawn);
                                    // For unique weapon
                                    if (CurrentCategory == 0 && CurrentRare == 1)
                                        ActorToSpawn = StrParam("%SWorldSpawnPickup", ActorToSpawn);
                                    // For unique armor
                                    if (CurrentCategory == 1 && CurrentRare == 1)
                                        ActorToSpawn = StrParam("%SWorldSpawnPickup", StrLeft(ActorToSpawn, StrLen(ActorToSpawn) - 6));
                                    // For unique boots
                                    if (CurrentCategory == 2 && CurrentRare == 1)
                                        ActorToSpawn = StrParam("%SWorldSpawnPickup", StrLeft(ActorToSpawn, StrLen(ActorToSpawn) - 6));
                                }

                                // Spawn Item in Disassembling Device
                                SpawnSpotForced(ActorToSpawn, DisassemblingDeviceID, UniqueTID(), 0);

                                SetFont("BIGFONT");
                                HudMessage("Item assembly is complete");
                                EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 4, "Green", X + 64.0, Y + 240.0, 3.0, 2.0);
                                ActivatorSound("mission/complete", 127);
                                FadeRange(0, 0, 0, 1.0, 0, 0, 0, 0.0, 2.0);

                                SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
                                return;
                            }
                            else
                            {
                                SetFont("BIGFONT");
                                if (Player.RankLevel < CurrentRank)
                                    PrintError("You need higher Rank to assembly this item");
                                else if (CurrentIndexBasic >= 0 && !CheckInventory(ItemData[CategoriesData[CurrentCategory]][CurrentIndexBasic].Actor))
                                    PrintError("You need basic item");
                                else if (CheckInventory("DRPGCredits") < CurrentCost)
                                    PrintError("Not enough credits to assembly this item");
                                else if (CheckInventory(ItemData[7][CurrentTypeDetails1].Actor) < CurrentAmountDetails1 ||  CheckInventory(ItemData[7][CurrentTypeDetails2].Actor) < CurrentAmountDetails2
                                         || CheckInventory(ItemData[7][CurrentTypeDetails3].Actor) < CurrentAmountDetails3)
                                    PrintError("Not enough details to assembly this item");
                                else if (CheckInventory(ItemData[8][CurrentTypeDetails4].Actor) < CurrentAmountDetails4 || CheckInventory(ItemData[4][CurrentTypeDetails5].Actor) < CurrentAmountDetails5 || CheckInventory(ItemData[6][CurrentTypeDetails6].Actor) < CurrentAmountDetails6)
                                    PrintError("Not enough additional details to assembly this item");
                                ActivatorSound("menu/error", 127);
                            }
                        }
                    }

                    Delay(1);
                }
            }
        }

        Delay(1);
    }
}

NamedScript MapSpecial void OpenSanctuary()
{
    if (AveragePlayerRank() >= 10)
    {
        Door_Open(45, 32, 13);
    }
    else
    {
        PrintError("You need to be of Rank 10 to enter the Sanctuary");
        ActivatorSound("menu/error", 127);
        return;
    }
}

NamedScript MapSpecial void DemonAssemblingSanctuary()
{
    // if you're already in a menu, terminate
    if (Player.InMenu || Player.InShop || Player.OutpostMenu == OMENU_DEMONSANCTUARY) return;

    // If Marines are hostile or the power is out, terminate
    if (MarinesHostile || PowerOut) return;

    // If don't use an DoomRL Arsenal, terminate
    if (CompatMode != COMPAT_DRLA)
    {
        PrintError("You don't hear the whispers of another world");
        ActivatorSound("menu/error", 127);
        return;
    }

    ActivatorSound("menu/move", 127);
    Player.OutpostMenu = OMENU_DEMONSANCTUARY;

    // Main coordinates
    fixed X = 0.1;
    fixed Y = 0.1;

    // Additional coordinates
    fixed X1, Y1, Y2;

    // Categories Data
    int CurrentCategory;
    int CategoriesData[3] = {0, 3, 9};
    str CategoriesNames[3] =
    {
        "\CaWeapons",
        "\CdArmors",
        "\CqBoots"
    };

    // Rare
    int CurrentRare;
    str RareNames[1] =
    {
        "\CgDemonic"
    };

    // Current Item
    int CurrentItem;
    int CurrentItemMin;
    int CurrentItemMax;
    int CurrentCost;
    int CurrentRank;

    // Current Required Basiс Item
    int CurrentIndexBasic;

    // Current Required Items
    int CurrentTypeDetails1;
    int CurrentAmountDetails1;
    int CurrentTypeDetails2;
    int CurrentAmountDetails2;
    int CurrentTypeDetails3;
    int CurrentAmountDetails3;
    int CurrentTypeDetails4;
    int CurrentAmountDetails4;
    int CurrentTypeDetails5;
    int CurrentAmountDetails5;
    int CurrentTypeDetails6;
    int CurrentAmountDetails6;

    // So the player's initial interaction is not processed as a menu action
    Delay(1);

    while (Player.OutpostMenu == OMENU_DEMONSANCTUARY)
    {
        SetPlayerProperty(0, 1, PROP_TOTALLYFROZEN);

        // Reset additional coordinates
        X1 = 0.0;
        Y1 = 0.0;
        Y2 = 0.0;

        // Get Min/Max Item Index
        // For Weapon
        if (CurrentCategory == 0)
        {
            if (CurrentRare == 0)
            {
                CurrentItemMin = 63;
                CurrentItemMax = 66;
            }
        }
        // For Armor
        if (CurrentCategory == 1)
        {
            if (CurrentRare == 0)
            {
                CurrentItemMin = 85;
                CurrentItemMax = 85;
            }
        }
        // For Boots
        if (CurrentCategory == 2)
        {
            if (CurrentRare == 0)
            {
                CurrentItemMin = 31;
                CurrentItemMax = 31;
            }
        }
        if (CurrentItem < CurrentItemMin) CurrentItem = CurrentItemMin;
        if (CurrentItem > CurrentItemMax) CurrentItem = CurrentItemMax;

        // Calculate Required Items
        ItemInfoPtr Item = &ItemData[CategoriesData[CurrentCategory]][CurrentItem];

        // Type/Amount Required Details
        // For Weapons
        if (CurrentCategory == 0)
        {
            // Set Global Current Type Details
            CurrentTypeDetails2 =  3;
            CurrentTypeDetails3 =  6;

            // Set Global Required
            // For Demonic Rare
            if (CurrentRare == 0)
            {
                // Calculate Current Cost
                CurrentCost = (Item->Price - Item->Price * Player.ShopDiscount / 100) / 250 * 250;

                // Set Default Current Type Details
                CurrentTypeDetails1 =  12;
                CurrentTypeDetails4 =  9;

                // Set Individual Required
                // For Deaths Gaze
                if (ItemData[0][CurrentItem].Actor == "RLDeathsGaze")
                {
                    CurrentRank = 10;
                    CurrentIndexBasic = -1;
                    CurrentAmountDetails1 = 30;
                    CurrentAmountDetails2 = 20;
                    CurrentAmountDetails3 = 10;
                    CurrentAmountDetails4 = 1;
                }
                // For Soulstorm Rifle
                if (ItemData[0][CurrentItem].Actor == "RLSoulstormRifle")
                {
                    CurrentRank = 11;
                    CurrentIndexBasic = -1;
                    CurrentAmountDetails1 = 40;
                    CurrentAmountDetails2 = 30;
                    CurrentAmountDetails3 = 15;
                    CurrentAmountDetails4 = 1;
                }
                // For Hells Reign
                if (ItemData[0][CurrentItem].Actor == "RLHellsReign")
                {
                    CurrentRank = 12;
                    CurrentIndexBasic = -1;
                    CurrentAmountDetails1 = 60;
                    CurrentAmountDetails2 = 40;
                    CurrentAmountDetails3 = 20;
                    CurrentAmountDetails4 = 2;
                }
                // For Unmaker
                if (ItemData[0][CurrentItem].Actor == "RLUnmaker")
                {
                    CurrentRank = 13;
                    CurrentIndexBasic = -1;
                    CurrentAmountDetails1 = 80;
                    CurrentAmountDetails2 = 50;
                    CurrentAmountDetails3 = 25;
                    CurrentAmountDetails4 = 3;
                }
            }
        }
        // For Demonic Armor
        if (CurrentCategory == 1)
        {
            // Set Basic Item
            CurrentIndexBasic = -1;

            // Set Current Cost
            CurrentCost = (Item->Price - Item->Price * Player.ShopDiscount / 100) / 250 * 250;

            // Set Individual Required
            // For Demonic Carapace Armor
            if (ItemData[3][CurrentItem].Actor == "RLDemonicCarapaceArmorPickup")
            {
                // Set Current Rank
                CurrentRank = 12;

                // Set Type Details
                CurrentTypeDetails1 =  12;
                CurrentTypeDetails2 =  3;
                CurrentTypeDetails3 =  9;
                CurrentTypeDetails4 =  9;

                // Set Amount Details
                CurrentAmountDetails1 = 60;
                CurrentAmountDetails2 = 40;
                CurrentAmountDetails3 = 20;
                CurrentAmountDetails4 = 2;
            }
        }
        // For Demonic Boots
        if (CurrentCategory == 2)
        {
            // Set Global Basic Item
            CurrentIndexBasic = -1;

            // Set Current Cost
            CurrentCost = (Item->Price - Item->Price * Player.ShopDiscount / 100) / 250 * 250;

            // Set Individual Required
            // For Demonic Boots
            if (ItemData[9][CurrentItem].Actor == "RLDemonicBootsPickup")
            {
                // Set Current Rank
                CurrentRank = 12;

                // Set Type Details
                CurrentTypeDetails1 =  12;
                CurrentTypeDetails2 =  3;
                CurrentTypeDetails3 =  9;
                CurrentTypeDetails4 =  9;

                // Set Amount Details
                CurrentAmountDetails1 = 30;
                CurrentAmountDetails2 = 20;
                CurrentAmountDetails3 = 10;
                CurrentAmountDetails4 = 1;
            }
        }
        // Draw the background
        if (GetCVar("drpg_menudim"))
            FadeRange(0, 0, 0, 0.65, 0, 0, 0, 0.0, 0.25);

        // Set the HUD Size
        SetHudSize(GetActivatorCVar("drpg_menu_width"), GetActivatorCVar("drpg_menu_height"), true);

        // Draw Border
        // These are pushed back a bit so the border doesn't overlap anything
        if (GetActivatorCVar("drpg_menu_background_border"))
            DrawBorder("Bor", -1, 8, -5.0, 0.0, 470, 470);

        // Text
        SetFont("BIGFONT");
        HudMessage("\CgDemon Sanctuary\C-");
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID, "White", X + 132.0, Y + 16.0, 0.05, 0.05);

        SetFont("BIGFONT");
        HudMessage("Select item to summoning:");
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 1, "White", X + 88.0, Y + 56.0, 0.05, 0.05);

        SetFont("BIGFONT");
        HudMessage("Category: %S", CategoriesNames[CurrentCategory]);
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 2, "White", X + 124.0, Y + 96.0, 0.05, 0.05);

        SetFont("BIGFONT");
        HudMessage("Rare: %S", RareNames[CurrentRare]);
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 3, "White", X + 164.0, Y + 132.0, 0.05, 0.05);

        PrintSprite(ItemData[CategoriesData[CurrentCategory]][CurrentItem].Sprite.Name, 0, X + 240.0,  Y + 212.0, 0.05);

        SetFont("BIGFONT");
        HudMessage("Item: %S", ItemData[CategoriesData[CurrentCategory]][CurrentItem].Name);
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 4, "White", X + 32.0, Y + 240.0, 0.05, 0.05);

        SetFont("SMALLFONT");
        HudMessage("Required \Cddetails\C-:");
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 5, "White", X + 32.0, Y + 268.0, 0.05, 0.05);

        // Required Details
        // For Details #1
        if (CurrentAmountDetails1 > 0)
        {
            SetFont("SMALLFONT");
            HudMessage("%S: %d", ItemData[7][CurrentTypeDetails1].Name, CurrentAmountDetails1);
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 6, "White", X + 32.0, Y + 276.0, 0.05, 0.05);
        }
        // For Details #2
        if (CurrentAmountDetails2 > 0)
        {
            SetFont("SMALLFONT");
            HudMessage("%S: %d", ItemData[7][CurrentTypeDetails2].Name, CurrentAmountDetails2);
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 7, "White", X + 32.0, Y + 284.0, 0.05, 0.05);
        }
        // For Details #3
        if (CurrentAmountDetails3 > 0)
        {
            SetFont("SMALLFONT");
            HudMessage("%S: %d", ItemData[7][CurrentTypeDetails3].Name, CurrentAmountDetails3);
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 8, "White", X + 32.0, Y + (CurrentAmountDetails2 > 0 ? 292.0 : 284.0), 0.05, 0.05);
        }
        // For Additional Details
        if (CurrentAmountDetails4 > 0 || CurrentAmountDetails5 > 0 || CurrentAmountDetails6 > 0)
        {
            SetFont("SMALLFONT");
            HudMessage("Required \Cqadditional details\C-:");
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 9, "White", X + 32.0, Y + 308.0, 0.05, 0.05);

            // Set additional coordinates
            if (CurrentAmountDetails5 > 0 && CurrentAmountDetails4 + CurrentAmountDetails6 > 0)
                Y2 += 8.0;
            if (CurrentAmountDetails6 > 0 && CurrentAmountDetails4 + CurrentAmountDetails5 > 0)
                Y2 += 8.0;
        }
        // For Details #4 (Additional)
        if (CurrentAmountDetails4 > 0)
        {
            SetFont("SMALLFONT");
            HudMessage("%S: %d", ItemData[8][CurrentTypeDetails4].Name, CurrentAmountDetails4);
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 10, "White", X + 32.0, Y + Y1 + 316.0, 0.05, 0.05);

            PrintSprite(ItemData[8][CurrentTypeDetails4].Sprite.Name, 0, X + X1 + (CurrentTypeDetails4 == 9 ? 52.0 : 32.0),  Y + Y2 + (CurrentTypeDetails4 == 9 ? 392.0 : 315.0), 0.05);

            X1 += 44.0;
            Y1 += 8.0;
        }
        // For Details #5 (Additional)
        if (CurrentAmountDetails5 > 0)
        {
            SetFont("SMALLFONT");
            HudMessage("%S: %d", ItemData[4][CurrentTypeDetails5].Name, CurrentAmountDetails5);
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 11, "White", X + 32.0, Y + Y1 + 316.0, 0.05, 0.05);

            PrintSprite(ItemData[4][CurrentTypeDetails5].Sprite.Name, 0, X + X1 + 48.0,  Y + Y2 + 376.0, 0.05);

            X1 += 32.0;
            Y1 += 8.0;
        }
        // For Details #6 (Additional)
        if (CurrentAmountDetails6 > 0)
        {
            SetFont("SMALLFONT");
            HudMessage("%S: %d", ItemData[6][CurrentTypeDetails6].Name, CurrentAmountDetails6);
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 12, "White", X + 32.0, Y + Y1 + 316.0, 0.05, 0.05);

            PrintSprite(ItemData[6][CurrentTypeDetails6].Sprite.Name, 0, X + X1 + 48.0,  Y + Y2 + 348.0, 0.05);

            X1 += 32.0;
            Y1 += 8.0;
        }

        SetFont("SMALLFONT");
        HudMessage("Required \Cfcredits\C-:\n%d \CfC\C-", CurrentCost);
        EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 13, "White", X + 272.0, Y + 268.0, 0.05, 0.05);

        if (CurrentRank > 0)
        {
            SetFont("SMALLFONT");
            HudMessage("Required \CkRank\C-:\n%d \Ck(%S)\C-", CurrentRank, LongRanks[CurrentRank]);
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 14, "White", X + 272.0, Y + 290.0, 0.05, 0.05);
        }

        // Required basic Item
        if (CurrentIndexBasic >= 0)
        {
            SetFont("SMALLFONT");
            HudMessage("Required \Cabasic item\C-:\n%S", StrLeft(ItemData[CategoriesData[CurrentCategory]][CurrentIndexBasic].Name, 26));
            EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 15, "White", X + 272.0, Y + 314.0, 0.05, 0.05);

            PrintSprite(ItemData[CategoriesData[CurrentCategory]][CurrentIndexBasic].Sprite.Name, 0, X + 336.0,  Y + 372.0, 0.05);
        }

        // Input
        if (CheckInput(BT_FORWARD, KEY_PRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            CurrentCategory++;
            CurrentItem = 0;
            CurrentIndexBasic = -1;
            CurrentAmountDetails1 = 0;
            CurrentAmountDetails2 = 0;
            CurrentAmountDetails3 = 0;
            CurrentAmountDetails4 = 0;
            CurrentAmountDetails5 = 0;
            CurrentAmountDetails6 = 0;
            if (CurrentCategory > 2) CurrentCategory = 0;
        }
        if (CheckInput(BT_BACK, KEY_PRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            CurrentCategory--;
            CurrentItem = 0;
            CurrentIndexBasic = -1;
            CurrentAmountDetails1 = 0;
            CurrentAmountDetails2 = 0;
            CurrentAmountDetails3 = 0;
            CurrentAmountDetails4 = 0;
            CurrentAmountDetails5 = 0;
            CurrentAmountDetails6 = 0;
            if (CurrentCategory < 0) CurrentCategory = 2;
        }
        if (CheckInput(BT_MOVELEFT, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            CurrentItem--;
            CurrentIndexBasic = -1;
            CurrentAmountDetails1 = 0;
            CurrentAmountDetails2 = 0;
            CurrentAmountDetails3 = 0;
            CurrentAmountDetails4 = 0;
            CurrentAmountDetails5 = 0;
            CurrentAmountDetails6 = 0;
            if (CurrentItem < CurrentItemMin) CurrentItem = CurrentItemMax;
        }
        if (CheckInput(BT_MOVERIGHT, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            CurrentItem++;
            CurrentIndexBasic = -1;
            CurrentAmountDetails1 = 0;
            CurrentAmountDetails2 = 0;
            CurrentAmountDetails3 = 0;
            CurrentAmountDetails4 = 0;
            CurrentAmountDetails5 = 0;
            CurrentAmountDetails6 = 0;
            if (CurrentItem > CurrentItemMax) CurrentItem = CurrentItemMin;
        }
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
        {
            if (CheckInput(BT_SPEED, KEY_HELD, false, PlayerNumber()))
            {
                if ((CurrentIndexBasic >= 0 && CheckInventory(ItemData[CategoriesData[CurrentCategory]][CurrentIndexBasic].Actor) || CurrentIndexBasic < 0) && Player.RankLevel >= CurrentRank && CheckInventory("DRPGCredits") >= CurrentCost && CheckInventory(ItemData[7][CurrentTypeDetails1].Actor) >= CurrentAmountDetails1 &&  CheckInventory(ItemData[7][CurrentTypeDetails2].Actor) >= CurrentAmountDetails2
                        && CheckInventory(ItemData[7][CurrentTypeDetails3].Actor) >= CurrentAmountDetails3 && CheckInventory(ItemData[8][CurrentTypeDetails4].Actor) >= CurrentAmountDetails4 && CheckInventory(ItemData[4][CurrentTypeDetails5].Actor) >= CurrentAmountDetails5 && CheckInventory(ItemData[6][CurrentTypeDetails6].Actor) >= CurrentAmountDetails6)
                {
                    Player.OutpostMenu = 0;
                    str ActorToSpawn;

                    // Take Basic Item
                    if (CurrentIndexBasic >= 0)
                    {
                        // Take Current Basic Item
                        TakeInventory(ItemData[CategoriesData[CurrentCategory]][CurrentIndexBasic].Actor, 1);

                        // Take tokens from DoomRL Arsenal
                        if (CompatMode == COMPAT_DRLA)
                            RemoveDRLAItem(CategoriesData[CurrentCategory], CurrentIndexBasic);
                    }

                    // Take Details
                    TakeInventory("DRPGCredits", CurrentCost);
                    TakeInventory(ItemData[7][CurrentTypeDetails1].Actor, CurrentAmountDetails1);
                    TakeInventory(ItemData[7][CurrentTypeDetails2].Actor, CurrentAmountDetails2);
                    TakeInventory(ItemData[7][CurrentTypeDetails3].Actor, CurrentAmountDetails3);
                    TakeInventory(ItemData[8][CurrentTypeDetails4].Actor, CurrentAmountDetails4);
                    TakeInventory(ItemData[4][CurrentTypeDetails5].Actor, CurrentAmountDetails5);
                    TakeInventory(ItemData[6][CurrentTypeDetails6].Actor, CurrentAmountDetails6);

                    // Take Demon Artifacts Limit tokens from DoomRL Arsenal
                    for(int i = 0; i < CurrentAmountDetails4; i++)
                        RemoveDRLAItem(8, CurrentTypeDetails4);

                    // The effect of sleep immersion
                    FadeRange(0, 0, 0, 0.5, 0, 0, 0, 1.0, 1.0);
                    Delay(35 * 1);

                    // Set actor to spawn
                    ActorToSpawn = ItemData[CategoriesData[CurrentCategory]][CurrentItem].Actor;
                    if (CompatMode == COMPAT_DRLA)
                    {
                        // For demonic weapon
                        if (CurrentCategory == 0)
                            ActorToSpawn = StrParam("%SWorldSpawnPickup", ActorToSpawn);
                        // For demonic armor
                        if (CurrentCategory == 1)
                            ActorToSpawn = StrParam("%SWorldSpawnPickup", StrLeft(ActorToSpawn, StrLen(ActorToSpawn) - 6));
                        // For demonic boots
                        if (CurrentCategory == 2)
                            ActorToSpawn = StrParam("%SWorldSpawnPickup", StrLeft(ActorToSpawn, StrLen(ActorToSpawn) - 6));
                    }

                    // Spawn Item in Disassembling Device
                    SpawnSpotForced(ActorToSpawn, DemonSanctuaryID, UniqueTID(), 0);

                    SetFont("BIGFONT");
                    HudMessage("Item summoning is complete");
                    EndHudMessage(HUDMSG_FADEOUT, MENU_ID + 4, "Green", X + 64.0, Y + 240.0, 3.0, 2.0);
                    ActivatorSound("mission/complete", 127);
                    FadeRange(0, 0, 0, 1.0, 0, 0, 0, 0.0, 2.0);

                    SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
                    return;
                }
                else
                {
                    SetFont("BIGFONT");
                    if (Player.RankLevel < CurrentRank)
                        PrintError("You need higher Rank to assembly this item");
                    else if (CurrentIndexBasic >= 0 && !CheckInventory(ItemData[CategoriesData[CurrentCategory]][CurrentIndexBasic].Actor))
                        PrintError("You need basic item");
                    else if (CheckInventory("DRPGCredits") < CurrentCost)
                        PrintError("Not enough credits to assembly this item");
                    else if (CheckInventory(ItemData[7][CurrentTypeDetails1].Actor) < CurrentAmountDetails1 ||  CheckInventory(ItemData[7][CurrentTypeDetails2].Actor) < CurrentAmountDetails2
                             || CheckInventory(ItemData[7][CurrentTypeDetails3].Actor) < CurrentAmountDetails3)
                        PrintError("Not enough details to assembly this item");
                    else if (CheckInventory(ItemData[8][CurrentTypeDetails4].Actor) < CurrentAmountDetails4 || CheckInventory(ItemData[4][CurrentTypeDetails5].Actor) < CurrentAmountDetails5 || CheckInventory(ItemData[6][CurrentTypeDetails6].Actor) < CurrentAmountDetails6)
                        PrintError("Not enough additional details to assembly this item");
                    ActivatorSound("menu/error", 127);
                }
            }
        }

        Delay(1);
    }
}

NamedScript void AlarmLoop()
{
    // Animate Outpost alarm
    SetActorState(1600, "ACTIVE");
}

NamedScript void PlaceBoss()
{
    // Minigunners
    if (OutpostNotoriety == 0)
        SpawnSpotFacingForced("DRPGMarineMinigunNotActuallyPeople", BossSpotTID, MarineBossTID - 1);

    // Small Powersuit
    if (OutpostNotoriety == 1)
        SpawnSpotFacingForced("DRPGPowerSuitSlackingOffDownstairs", BossSpotTID + 1, MarineBossTID - 1);

    // Heavy Marines
    if (OutpostNotoriety == 2)
        SpawnSpotFacingForced("DRPGMarineMinigunNotActuallyPeople", BossSpotTID, MarineBossTID - 1);

    // Heavy Powersuit & Heavy Powersuit Mk. II
    if (OutpostNotoriety >= 3)
        SpawnSpotFacingForced("DRPGHeavyPowerSuitHavingASiesta", BossSpotTID + 2, MarineBossTID - 1);
}

NamedScript void SpawnBoss()
{
    // Minigunners
    if (OutpostNotoriety == 0)
    {
        SpawnSpotForced("TeleportFog", NotorietySpotTID, 0, 0);
        SpawnSpotFacingForced("DRPGMarineMinigun", NotorietySpotTID, MarineBossTID);
    }

    // Small Powersuit
    if (OutpostNotoriety == 1)
    {
        SpawnSpotForced("DRPGBigTeleportFog", NotorietySpotTID, 0, 0);
        SpawnSpotFacingForced("DRPGPowerSuit", NotorietySpotTID, MarineBossTID);
    }

    // Heavy Marines
    if (OutpostNotoriety == 2)
    {
        SpawnSpotForced("TeleportFog", NotorietySpotTID, 0, 0);
        SpawnSpotFacingForced("DRPGMarineMinigunHuntingYerAss", NotorietySpotTID, MarineBossTID);
    }

    // Heavy Powersuit
    if (OutpostNotoriety >= 3)
    {
        // Teleport out guards
        for (int i = MarineTID; i <= MarineMaxTID; i++)
        {
            SpawnSpot("TeleportFog", i, 0, 0);
            Thing_Remove(i);
        }

        SetMusic("");
        Delay(35 * 30);
        if (OutpostNotoriety >= 4)
            SetOutpostMusic(OUTPOST_MUSIC_BOSS);

        SpawnSpotForced("DRPGHugeTeleportFog", NotorietySpotTID + 1, 0, 0);
        SpawnSpotFacingForced((OutpostNotoriety >= 4 ? "DRPGSuperPowerSuit" : "DRPGHeavyPowerSuit"), NotorietySpotTID + 1, MarineBossTID);
    }
}

void SpawnShopSpecialItem()
{
    if (CurrentLevel->UACBase)
    {
        // Remove old item
        Thing_Remove(ShopSpecialTID + 1);

        // Spawn new item
        SpawnSpotForced(ShopSpecialItem->Actor, ShopSpecialTID, ShopSpecialTID + 1, 0);
        SetActorProperty(ShopSpecialTID + 1, APROP_Invulnerable, true);
        Thing_SetSpecial(ShopSpecialTID + 1, 80, 8);
    }
}

void SetOutpostMusic(int Type)
{
    str Music;

    switch (Type)
    {
    case OUTPOST_MUSIC_NORMAL:
        Music = StrParam("OUTPO1_%i", Random(1, 4));
        break;
    case OUTPOST_MUSIC_COMBAT:
        Music = "Outpost2";
        break;
    case OUTPOST_MUSIC_LOWPOWER:
        Music = "Outpost3";
        break;
    case OUTPOST_MUSIC_LOWPOWER_COMBAT:
        Music = "Outpost4";
        break;
    case OUTPOST_MUSIC_BOSS:
        Music = "OPBoss";
        break;
    case OUTPOST_MUSIC_MEGABOSS:
        Music = "OPBoss2";
        break;
    case OUTPOST_MUSIC_CREDITS:
        Music = "OutpostC";
        break;
    }

    SetMusic(Music);
}
