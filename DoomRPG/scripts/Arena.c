#include "Defs.h"

#include "Arena.h"
#include "RPG.h"
#include "Utils.h"
#include "Outpost.h"
#include "Map.h"
#include "Menu.h"
#include "Monsters.h"

int RPGGlobal ArenaMaxWave;

bool RPGMap ArenaActive = false;
bool RPGMap ArenaTimerActive = false;
int RPGMap ArenaState = ARENA_READY;
int RPGMap ArenaPlayerNumber = -1;
int RPGMap ArenaMonstersTID = 1099;
int RPGMap ArenaSpotSpawns = 1100;
int RPGMap ArenaSectorTag = 100;
int RPGMap ArenaWave;
int RPGMap ArenaTimer;
int RPGMap ArenaKeyTimer;
int RPGMap ArenaKeyTimerType;
int RPGMap ArenaCount;
int RPGMap ArenaMod;
int RPGMap ArenaEnvironment;
int RPGMap ArenaKey;

str const ArenaMods[AMOD_MAX] =
{
    "None",
    "\CgNo Regeneration"
};

str const ArenaEvents[AEVENT_MAX] =
{
    "None",
    "\CmBlackout",
    "\CjFoggy",
    "\CiThe Floor is Lava",
    "\CuGrayscale"
};

str const ArenaBonus[ABONUS_MAX] =
{
    "\CjSelect Your Own!",
    "\CvFull EP",
    "\CfMoney Drop",
    "\CfChip Drop",
    "\CeAmmo Drop",
    "\CaHealth Drop",
    "\CqPowerup Drop",
    "\CkStim Drop",
    "\CjKey Drop"
};

// Arena Script
NamedScript MapSpecial void ArenaLoop()
{
    int BonusRandomizer;
    bool Ready;
    ArenaSetEnvironment(AEVENT_NONE);

    // Arena Loop
    while (true)
    {
        // Stop the script if the Arena is stopped or the Arena activator dies
        if (!ArenaActive || ClassifyActor(Players(ArenaPlayerNumber).TID) & ACTOR_DEAD)
        {
            ArenaStop();
            return;
        }

        // Arena HUD
        ArenaDrawHUD();

        // Arena Status Handling
        switch(ArenaState)
        {
        case ARENA_INTERMISSION:
        {
            BonusRandomizer = Random(-10, ABONUS_MAX - 1);

            ArenaMod = -1;
            ArenaGetBonus(BonusRandomizer);
            ArenaSetEnvironment(AEVENT_NONE);
            ArenaState = ARENA_WAITING;
        }
        break;
        case ARENA_WAITING:
        {
            Ready = true;

            SetHudSize(0, 0, false);
            SetFont("BIGFONT");

            if (ArenaPlayerNumber == PlayerNumber())
            {
                if (!Player.InMenu && !Player.InShop && !Player.OutpostMenu)
                {
                    if (GetCVar("use_joystick") || GetUserCVar(PlayerNumber(), "drpg_deltatouch"))
                        HudMessage("Hold \Cd%S\C- to start the next wave\nHold \Cd%S\C- to end the Arena", "Use", "Jump");
                    else
                        HudMessage("Hold \Cd%jS\C- to start the next wave\nHold \Cd%jS\C- to end the Arena", "+use", "+jump");
                    EndHudMessage(HUDMSG_PLAIN, 0, "White", 1.5, 0.75, 0.05);
                }

                if (CheckInput(BT_USE, KEY_HELD, false, ArenaPlayerNumber) && (!Player.InMenu && !Player.InShop && !Player.OutpostMenu && !Player.CrateOpen) && !Player.MenuBlock)
                {
                    // Check to see if others are still in the menu
                    for (int i = 0; i < MAX_PLAYERS; i++)
                        if (Players(i).InMenu || Players(i).InShop)
                            Ready = false;

                    if (Ready)
                    {
                        if (ArenaKeyTimerType == AKTIMER_CONTINUE)
                            ArenaKeyTimer++;
                        else
                        {
                            ArenaKeyTimer = 0;
                            ArenaKeyTimerType = AKTIMER_CONTINUE;
                        }

                        if (ArenaKeyTimer > ARENA_HOLDTIME)
                        {
                            ArenaKeyTimer = 0;
                            // Multiplayer Countdown
                            if (InMultiplayer)
                            {
                                SetFont("BIGFONT");
                                for (int i = 3; i > 0; i--)
                                {
                                    HudMessage("%d", i);
                                    EndHudMessageBold(HUDMSG_FADEOUT, 0, "Green", 0.5, 0.5, 0.25, 0.75);
                                    Delay(35);
                                }
                            }

                            ArenaWave++;
                            ArenaState = ARENA_READY;
                        }
                    }
                    else
                    {
                        PrintError("Someone is currently in a menu");
                        ActivatorSound("menu/error", 127);
                    }
                }
                else if (CheckInput(BT_JUMP, KEY_HELD, false, ArenaPlayerNumber) && (!Player.InMenu && !Player.InShop && !Player.OutpostMenu && !Player.CrateOpen))
                {
                    if (ArenaKeyTimerType == AKTIMER_STOP)
                        ArenaKeyTimer++;
                    else
                    {
                        ArenaKeyTimer = 0;
                        ArenaKeyTimerType = AKTIMER_STOP;
                    }

                    if (ArenaKeyTimer > ARENA_HOLDTIME)
                    {
                        ArenaKeyTimer = 0;
                        ArenaStop();
                        return;
                    }
                }
                else
                {
                    ArenaKeyTimer = 0;
                }

                // Reset menu block
                if (CheckInput(0, KEY_ANYIDLE, false, ArenaPlayerNumber))
                    Player.MenuBlock = false;
            }
        }
        break;
        case ARENA_READY:
        {
            // Clean corpses every couple of waves
            if (ArenaWave > 1 && (ArenaWave % 5) == 0)
                Thing_Remove(ArenaMonstersTID);

            // Change up the music
            if (ArenaWave > 1 && (ArenaWave % 10) == 0)
                ArenaSetMusic();

            // Update the global Max Wave
            if (ArenaWave >= ArenaMaxWave)
                ArenaMaxWave = ArenaWave;

            ArenaMod = Random(-10, AMOD_MAX - 1);
            ArenaSetEnvironment(AEVENT_RANDOM);
            ArenaSpawnMobs();
            ArenaState = ARENA_ACTIVE;
        }
        break;
        case ARENA_ACTIVE:
        {
            ArenaCheckMod();
            ArenaCount = ThingCount(0, ArenaMonstersTID);
            if (ArenaCount <= 0)
                ArenaState = ARENA_INTERMISSION;
            if (ArenaTimerActive && ArenaTimer > 0)
                ArenaTimer--;
        }
        break;
        }

        Delay(1);
    }
}

// Stop the Arena session
NamedScript MapSpecial void ArenaStop()
{
    if (ArenaWave > 1)
        ArenaState = ARENA_WAITING;
    else
        ArenaState = ARENA_READY;

    ArenaActive = false;

    if (ArenaWave >= ArenaMaxWave)
        ArenaMaxWave = ArenaWave;

    ArenaPlayerNumber = -1;
    ArenaCount = 0;
    ArenaMod = -1;
    ArenaSetEnvironment(AEVENT_NONE);

    SetOutpostMusic((PowerOut ? OUTPOST_MUSIC_LOWPOWER : OUTPOST_MUSIC_NORMAL));

    Ceiling_RaiseByValue(ArenaSectorTag - 1, 64, 128);
    Thing_Remove(ArenaMonstersTID);
}

// Arena Test Script
NamedScript MapSpecial void SetArena(int Wave, int Environment, int Bonus)
{
    if (Wave > 0)
        ArenaWave = Wave;

    if (Environment >= 0)
        ArenaSetEnvironment(Environment);

    if (Bonus >= 0)
        ArenaGetBonus(Bonus);
}

NamedScript MapSpecial void ArenaChooseBonus()
{
    int BonusChoice = 1;
    bool CanChooseKey = false;
    fixed X, Y;

    // There's a 1/4 chance you can use Drop Key
    if (Random(1, 4) == 1) CanChooseKey = true;

    Player.OutpostMenu = OMENU_BONUSSELECTOR;

    while (true)
    {
        X = 150.1;
        Y = 100.0;

        SetPlayerProperty(0, 1, PROP_TOTALLYFROZEN);

        // Prevent the menus from being opened
        Player.InMenu = false;
        Player.InShop = false;

        // Set the HUD Size
        SetHudSize(GetActivatorCVar("drpg_menu_width"), GetActivatorCVar("drpg_menu_height"), true);

        // Draw Border
        // These are pushed back a bit so the border doesn't overlap anything
        if (GetActivatorCVar("drpg_menu_background_border"))
            DrawBorder("Bor", -1, 8, -5.0, 0.0, 470, 470);

        // Title
        SetFont("BIGFONT");
        HudMessage("Choose a Bonus");
        EndHudMessage(HUDMSG_PLAIN, 0, "Green", 150.1, 50.0, 0.05);

        // Input
        if (CheckInput(BT_FORWARD, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            BonusChoice--;
            if (BonusChoice < 1) BonusChoice = ABONUS_MAX - (CanChooseKey ? 1 : 2);
        }
        if (CheckInput(BT_BACK, KEY_ONLYPRESSED, false, PlayerNumber()))
        {
            ActivatorSound("menu/move", 127);
            BonusChoice++;
            if (BonusChoice > ABONUS_MAX - (CanChooseKey ? 1 : 2)) BonusChoice = 1;
        }
        if (CheckInput(BT_USE, KEY_PRESSED, false, PlayerNumber()))
        {
            SetPlayerProperty(0, 0, PROP_TOTALLYFROZEN);
            ArenaGetBonus(BonusChoice);
            Player.OutpostMenu = 0;
            Player.MenuBlock = true;
            return;
        }

        // Drawing
        for (int i = 1; i < ABONUS_MAX; i++)
        {
            // Skip Key Drop if the randomizer hates you
            if (i == ABONUS_KEYDROP && !CanChooseKey) continue;

            // Cursor
            if (i == BonusChoice)
            {
                HudMessage("-->");
                EndHudMessage(HUDMSG_PLAIN, 0, MenuCursorColor, X - 25.0, Y, 0.05);
            }

            // Bonus String
            HudMessage("%S", ArenaBonus[i]);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);

            // Move down Y
            Y += 16.0;
        }

        Delay(1);
    }
}

void ArenaGetBonus(int Bonus)
{
    int SpawnAmount = ArenaWave * 5;
    str SpawnItem;

    // Cap the spawn amount so the game doesn't lag and crash into oblivion
    if (SpawnAmount > 1000)
        SpawnAmount = 1000;

    switch (Bonus)
    {
    case ABONUS_SELECT: // Choose Your Own!
        ActivatorSound("arena/pickbonus", 127);
        ArenaChooseBonus();
        break;
    case ABONUS_FULLEP: // Full EP
        if (InMultiplayer)
            for (int i = 0; i < MAX_PLAYERS; i++)
                Players(i).EP = Players(i).EPMax;
        else
        {
            if (Player.EP >= Player.EPMax)
            {
                ArenaGetBonus(Random(0, ABONUS_MAX - 1));
                return;
            }
            else
                Player.EP = Player.EPMax;
        }
        break;
    case ABONUS_MONEYDROP: // Money Drop
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (!PlayerInGame(i)) continue;

            GiveActorInventory(Players(i).TID, "DRPGCredits", SpawnAmount);
        }
        break;
    case ABONUS_CHIPDROP: // Chip Drop
        if (Random(1, 16) == 1)
            DropArenaItem("DRPGChipPlatinum");
        DropArenaItem("DRPGChipGold");
        break;
    case ABONUS_AMMODROP: // Ammo Drop
        DropArenaItem("DRPGBackpackRandomizer");
        break;
    case ABONUS_HEALTHDROP: // Health Drop
        if (Random(1, 4) == 1)
            SpawnItem = "Megasphere";
        else if (Random(1, 2) == 1)
            SpawnItem = "Soulsphere";
        else
            SpawnItem = "DRPGHealthDropper";
        DropArenaItem(SpawnItem);
        break;
    case ABONUS_POWERUPDROP: // Powerup Drop
        DropArenaItem("DRPGPowerupDropper");
        break;
    case ABONUS_STIMDROP: // Stim Drop
        if (Random(1, 4) == 1)
            DropArenaItem("DRPGStimPackageStat");
        else if (Random(1, 8) == 1)
            DropArenaItem("DRPGStimPackagePowerup");
        DropArenaItem("DRPGStimDropper");
        break;
    case ABONUS_KEYDROP: // Key Drop
        if (ArenaKey == 0) SpawnItem = "DRPGRedCard";
        if (ArenaKey == 1) SpawnItem = "DRPGYellowCard";
        if (ArenaKey == 2) SpawnItem = "DRPGBlueCard";
        if (ArenaKey == 3) SpawnItem = "DRPGRedSkull";
        if (ArenaKey == 4) SpawnItem = "DRPGYellowSkull";
        if (ArenaKey == 5) SpawnItem = "DRPGBlueSkull";
        if (ArenaKey == 6)
        {
            ArenaGetBonus(Random(0, ABONUS_MAX - 1));
            return;
        }
        DropArenaItem(SpawnItem);
        if (ArenaKey < 6) ArenaKey++;
        ActivatorSound("arena/keydrop", 127);
        break;
    }

    // Bonus Message
    if (Bonus > 0)
    {
        SetHudSize(0, 0, false);
        SetFont("BIGFONT");
        HudMessage("%S", ArenaBonus[Bonus]);
        EndHudMessageBold(HUDMSG_FADEOUT, MENU_ID, "White", 0.5, 0.5, 2.0, 1.0);
        if (Bonus != 10)
            ActivatorSound("arena/drop", 127);
    }
}

void DropArenaItem(str Item)
{
    int TID;
    bool Success;
    fixed X, Y, Z, XSpeed, YSpeed, ZSpeed;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!PlayerInGame(i)) continue;

        TID = UniqueTID();
        X = GetActorX(Players(i).TID);
        Y = GetActorY(Players(i).TID);
        Z = GetActorCeilingZ(Players(i).TID) - 32.0;
        XSpeed = GetCVarFixed("drpg_monster_dropdist");
        YSpeed = GetCVarFixed("drpg_monster_dropdist");
        ZSpeed = 8.0;

        Success = SpawnForced(Item, X, Y, Z, TID, 0);

        if (Success)
        {
            // Set Velocity
            SetActorVelocity(TID, RandomFixed(-XSpeed, XSpeed), RandomFixed(-YSpeed, YSpeed), ZSpeed, false, false);

            // Array has grown too big, resize it
            if (Players(i).DropTID.Position == Players(i).DropTID.Size)
                ArrayResize(&Players(i).DropTID);

            // Add item's TID to drop array
            ((int *)Players(i).DropTID.Data)[Players(i).DropTID.Position++] = TID;
        }
    }
}

void ArenaDrawHUD()
{
    fixed X = GetActivatorCVar("drpg_event_x") + 0.1;
    fixed Y = GetActivatorCVar("drpg_event_y") + 0.1;

    // Don't draw the HUD if you're in a menu
    if (Player.InMenu || Player.InShop || Player.OutpostMenu > 0) return;

    SetHudSize(GetActivatorCVar("drpg_hud_width"), GetActivatorCVar("drpg_hud_height"), false);
    SetFont("SMALLFONT");

    // Wave
    HudMessage("Wave: %d", ArenaWave);
    EndHudMessageBold(HUDMSG_PLAIN, 0, "Green", X, Y, 0.05);
    Y += 8.0;

    // Monster Count
    if (ArenaCount > 0)
    {
        HudMessage("Monsters: %d", ArenaCount);
        EndHudMessageBold(HUDMSG_PLAIN, 0, "Green", X, Y, 0.05);
        Y += 8.0;
    }

    // Timer
    if (ArenaTimerActive && ArenaTimer > 0)
    {
        HudMessage("Time: %d", ArenaTimer / 35 + 1);
        EndHudMessageBold(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
        Y += 8.0;
    }

    // Mod
    if (ArenaMod > 0)
    {
        HudMessage("%S", ArenaMods[ArenaMod]);
        EndHudMessageBold(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
        Y += 8.0;
    }

    // Environment
    if (ArenaEnvironment > 0)
    {
        HudMessage("%S", ArenaEvents[ArenaEnvironment]);
        EndHudMessageBold(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
        Y += 8.0;
    }

    // Cancel Timer
    if (ArenaKeyTimer > 0 && ArenaKeyTimer <= ARENA_HOLDTIME)
    {
        int Percent = CalcPercent(ArenaKeyTimer, ARENA_HOLDTIME);
        str Text;

        switch (ArenaKeyTimerType)
        {
        case AKTIMER_STOP:
            Text = "Stopping Arena";
            break;
        case AKTIMER_CONTINUE:
            Text = "\CaNext Wave";
            break;
        }

        DrawProgressBar(Text, Percent);
    }
}

void ArenaCheckMod()
{
    switch (ArenaMod)
    {
    case AMOD_NOREGEN:
        Player.HPRate = 0;
        Player.EPRate = 0;
        break;
    }
}

void ArenaSpawnMobs()
{
    int MonsterDiffLimit = 2 + ArenaWave;
    int Spawned;
    int SpawnChanceModifier;
    bool Boss = false;

    // [SW] Not enough monsters spawn in the Oblige Arena however it could get crowded in the UAC arena so lets have separate values for both.
    if (CurrentLevel->UACArena)
        SpawnChanceModifier = 1;
    else
        SpawnChanceModifier = 5;

    // Slots 0 - 2 are for normal monsters. Slot 3 is for boss. Slot 4 is NULL.
    MonsterInfoPtr MonsterList[3];
    MonsterInfoPtr TempMonster;
    // Store 3 monster actors.
    for (int i = 0; i < (2 + 1); i++)
    {
        TempMonster = &MonsterData[Random(0, MonsterDataAmount - 1)];

        // Skip bosses & monsters above difficulty limit.
        if (!TempMonster->Boss && TempMonster->Difficulty <= MonsterDiffLimit)
        {
            MonsterList[i] = TempMonster;
        }
        else
            i--;
    }
    // Decide if boss should spawn.
    Boss = (ArenaWave >= 50) && !Random(0, 8);
    // Store 1 boss actor.
    // This was a part of the above loop but was separated because it could complete before a boss was found.
    if (Boss)
        for (int i = 0; i < 1; i++)
        {
            TempMonster = &MonsterData[Random(0, MonsterDataAmount - 1)];

            if (TempMonster->Boss)
            {
                MonsterList[3] = TempMonster;
            }
            else
                i--;
        }

    // Monsters.
    for (int i = ArenaSpotSpawns; i <= ArenaSpotSpawns + 30; i++)
    {
        // Check to make sure there wasn't a chance that nothing at all spawned
        if (i == ArenaSpotSpawns + 30)
        {
            if (Spawned == 0)
                i = ArenaSpotSpawns;
            else
                break;
        }

        // Spawn the monster
        if (!Random(0, SpawnChanceModifier) > 0 && SpawnSpotFacing(MonsterList[Random(0, 2)]->Actor, i, ArenaMonstersTID))
        {
            SpawnSpotForced("TeleportFog", i, 0, 0);
            Spawned++;
        }
    }

    // Bosses.
    if (Boss)
    {
        int SpotTID;

        for (int i = 0; i < 32; i++)
        {
            SpotTID = ArenaSpotSpawns + Random(0, 30);

            if (SpawnSpotFacing(MonsterList[3]->Actor, SpotTID, ArenaMonstersTID))
            {
                SpawnSpot("TeleportFog", SpotTID, 0, 0);
                break;
            }
        }
    }
}

void ArenaSetEnvironment(int ID)
{
    int R, G, B;

    if (ID == AEVENT_RANDOM)
    {
        int EnvironmentRandomizer = Random(-10, AEVENT_MAX - 1);
        if (EnvironmentRandomizer > 0)
            ID = EnvironmentRandomizer;
    }

    // Avoiding lava for Arena WADs because mucking with the floor texture is too much compatibility wise.
    // Swapping it for Fog.
    if (CurrentLevel->UACArena)
        if (ID == AEVENT_LAVA)
            ID = AEVENT_FOGGY;

    switch (ID)
    {
    case AEVENT_NONE:
        Light_Fade(ArenaSectorTag, 192, 35);
        Light_Fade(ArenaSectorTag + 1, 192, 35);
        Light_Fade(ArenaSectorTag + 2, 192, 35);
        Sector_SetFade(ArenaSectorTag, 0, 0, 0);
        Sector_SetFade(ArenaSectorTag + 1, 0, 0, 0);
        Sector_SetFade(ArenaSectorTag + 2, 0, 0, 0);
        // Only for UAC Arena.
        if (!CurrentLevel->UACArena)
        {
            ChangeFloor(ArenaSectorTag, "CEIL5_1");
            ChangeFloor(ArenaSectorTag + 1, "CEIL5_2");
            ChangeFloor(ArenaSectorTag + 2, "FLAT3");
        }
        Sector_SetDamage(ArenaSectorTag, 0, 0);
        Sector_SetDamage(ArenaSectorTag + 1, 0, 0);
        Sector_SetDamage(ArenaSectorTag + 2, 0, 0);
        Sector_SetColor(ArenaSectorTag, 255, 255, 255);
        Sector_SetColor(ArenaSectorTag + 1, 255, 255, 255);
        Sector_SetColor(ArenaSectorTag + 2, 255, 255, 255);
        break;
    case AEVENT_BLACKOUT:
        Light_Fade(ArenaSectorTag, 0, 35);
        Light_Fade(ArenaSectorTag + 1, 0, 35);
        Light_Fade(ArenaSectorTag + 2, 0, 35);
        break;
    case AEVENT_FOGGY:
        R = Random(0, 255);
        G = Random(0, 255);
        B = Random(0, 255);
        Light_Fade(ArenaSectorTag, 128, 35);
        Light_Fade(ArenaSectorTag + 1, 128, 35);
        Light_Fade(ArenaSectorTag + 2, 128, 35);
        Sector_SetFade(ArenaSectorTag, R, G, B);
        Sector_SetFade(ArenaSectorTag + 1, R, G, B);
        Sector_SetFade(ArenaSectorTag + 2, R, G, B);
        break;
    case AEVENT_LAVA:
        ChangeFloor(ArenaSectorTag, "LAVA1");
        ChangeFloor(ArenaSectorTag + 1, "LAVA1");
        ChangeFloor(ArenaSectorTag + 2, "LAVA1");
        Sector_SetDamage(ArenaSectorTag, 1 + GameSkill(), MOD_SLIME);
        Sector_SetDamage(ArenaSectorTag + 1, 1 + GameSkill(), MOD_SLIME);
        Sector_SetDamage(ArenaSectorTag + 2, 1 + GameSkill(), MOD_SLIME);
        break;
    case AEVENT_GRAYSCALE:
        Sector_SetColor(ArenaSectorTag, 255, 255, 255, 255);
        Sector_SetColor(ArenaSectorTag + 1, 255, 255, 255, 255);
        Sector_SetColor(ArenaSectorTag + 2, 255, 255, 255, 255);
        break;
    }

    ArenaEnvironment = ID;
}

void ArenaSetMusic()
{
    SetMusic(StrParam("Arena%d", Random(1, MAX_ARENA_MUSIC)));
}