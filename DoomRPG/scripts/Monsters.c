#include "RPG.h"

#include "Arena.h"
#include "Map.h"
#include "Mission.h"
#include "Monsters.h"
#include "Namegen.h"
#include "Popoffs.h"
#include "Shield.h"
#include "Stats.h"
#include "Utils.h"

static int MonsterLevelCap;
static int MonsterStatCap;

int RPGMap MonsterID = 1;

MonsterInfoPtr MonsterData;
int MonsterDataAmount;

MegabossInfoPtr MegaBosses;
int MegaBossesAmount;

NoInit MonsterStats RPGMap Monsters[MAX_MONSTERS];

int TotalBosses;

MonsterInfo const MonsterDataDF[MAX_DEF_MONSTERS_DF] =
{
    { "ZombieMan",                          "Former Human",                      1,      0, false, "You hear shuffling footsteps and moans!" },
    { "ShotgunGuy",                         "Former Sergeant",                   2,      0, false, "You hear the sound of shotguns pumping!" },
    { "ChaingunGuy",                        "Former Commando",                   5,      0, false, "Quiet mutters and whirring set you on edge!" },
    { "DoomImp",                            "Imp",                               4,      0, false, "The walls are scratched and flame-scorched!" },
    { "Demon",                              "Demon",                             3,      0, false, "Hungry growls echo around you!" },
    { "Spectre",                            "Spectre",                           3,      0, false, "It's dark, you are likely to be eaten by a... Spectre?" },
    { "LostSoul",                           "Lost Soul",                         3,      0, false, "The sound of many flames echo about!" },
    { "Cacodemon",                          "Cacodemon",                         10,     0, false, "Screeches drown out all other sound!" },
    { "HellKnight",                         "Hell Knight",                       20,     0, false, "A battle cry chants in the distance!" },
    { "BaronOfHell",                        "Baron of Hell",                     30,     0, false, "Seems you've found Hell's nobility!" },
    { "PainElemental",                      "Pain Elemental",                    40,     0, false, "You feel you are being watched!" },
    { "Revenant",                           "Revenant",                          50,     0, false, "Bones clatter all around you!" },
    { "Fatso",                              "Mancubus",                          60,     0, false, "You hear deep, guttural noises!" },
    { "Arachnotron",                        "Arachnotron",                       60,     0, false, "Leg servos squeak and whirr nearby!" },
    { "Archvile",                           "Arch-Vile",                         70,     0, false, "You hear crackling flames!" },
    { "Cyberdemon",                         "Cyberdemon",                        85,     0, true,  "Suddenly you have a great urge to turn back! You scream in TERROR!" },
    { "SpiderMastermind",                   "Spider Mastermind",                 95,     0, true,  "You've stumbled into a hive of the deadliest kind!" }
};

MonsterInfo const MonsterDataLD[MAX_DEF_MONSTERS_DF] =
{
    { "LDZombieman",                          "Former Human",                    1,      0, false, "You hear shuffling footsteps and moans!" },
    { "LDShotgunguy",                         "Former Sergeant",                 2,      0, false, "You hear the sound of shotguns pumping!" },
    { "LDChaingunguy",                        "Former Commando",                 5,      0, false, "Quiet mutters and whirring set you on edge!" },
    { "LDDoomImp",                            "Imp",                             4,      0, false, "The walls are scratched and flame-scorched!" },
    { "LDDemon",                              "Demon",                           3,      0, false, "Hungry growls echo around you!" },
    { "LDSpectre",                            "Spectre",                         3,      0, false, "It's dark, you are likely to be eaten by a... Spectre?" },
    { "LDLostSoul",                           "Lost Soul",                       3,      0, false, "The sound of many flames echo about!" },
    { "LDCacodemon",                          "Cacodemon",                       10,     0, false, "Screeches drown out all other sound!" },
    { "LDHellKnight",                         "Hell Knight",                     20,     0, false, "A battle cry chants in the distance!" },
    { "LDBaronOfHell",                        "Baron of Hell",                   30,     0, false, "Seems you've found Hell's nobility!" },
    { "LDPainElemental",                      "Pain Elemental",                  40,     0, false, "You feel you are being watched!" },
    { "LDRevenant",                           "Revenant",                        50,     0, false, "Bones clatter all around you!" },
    { "LDFatso",                              "Mancubus",                        60,     0, false, "You hear deep, guttural noises!" },
    { "LDArachnotron",                        "Arachnotron",                     60,     0, false, "Leg servos squeak and whirr nearby!" },
    { "LDArchvile",                           "Arch-Vile",                       70,     0, false, "You hear crackling flames!" },
    { "LDCyberdemon",                         "Cyberdemon",                      85,     0, true,  "Suddenly you have a great urge to turn back! You scream in TERROR!" },
    { "LDSpiderMastermind",                   "Spider Mastermind",               95,     0, true,  "You've stumbled into a hive of the deadliest kind!" }
};

MonsterInfo const MonsterDataRAMPANCY[MAX_DEF_MONSTERS_RM] =
{
//  Normal Enemies
    { "Robot_SentryBot",                      "Sentry Bot",                      1,      0, false, "You hear shuffling footsteps and moans!" },
    { "Robot_DefenderBot",                    "Defender Bot",                    2,      0, false, "You hear the sound of shotguns pumping!" },
    { "Robot_SecurityBotSmall",               "Security Bot Small",              5,      0, false, "Quiet mutters and whirring set you on edge!" },
    { "Robot_SoldierBot",                     "Soldier Bot",                     4,      0, false, "The walls are scratched and flame-scorched!" },
    { "Robot_SpiderBotSmall",                 "Spider Bot Small",                3,      0, false, "Hungry growls echo around you!" },
    { "Robot_StealthDroid",                   "Stealth Droid",                   3,      0, false, "It's dark, you are likely to be eaten by a... Spectre?" },
    { "Robot_ScoutDrone",                     "Scout Drone",                     3,      0, false, "The sound of many flames echo about!" },
    { "Robot_MarauderDrone",                  "Marauder Drone",                  5,      0, false, "The sound of many flames echo about!" },
    { "Robot_AssassinDrone",                  "Assassin Drone",                  8,      0, false, "The sound of many flames echo about!" },
    //  Battleroids
    { "Robot_Battleroid",                     "Battleroid",                      6,      0, false, "The walls are scratched and flame-scorched!" },
    { "Robot_BattleroidBlue",                 "Battleroid Type-2",               8,      0, false, "The walls are scratched and flame-scorched!" },
    { "Robot_BattleroidRed",                  "Battleroid Type-3",               10,      0, false, "The walls are scratched and flame-scorched!" },
    { "Robot_BattleroidBlack",                "Battleroid Type-4",               12,      0, false, "The walls are scratched and flame-scorched!" },
    { "Robot_BattleroidBrown",                "Battleroid Type-5",               14,      0, false, "The walls are scratched and flame-scorched!" },
    { "Robot_BattleroidSilver",               "Battleroid Type-6",               16,     0, false, "The walls are scratched and flame-scorched!" },
    { "Robot_AssaultDrone",                   "Assault Drone",                   10,     0, false, "Screeches drown out all other sound!" },
    { "Robot_HunterKiller",                   "Hunter-Killer",                   22,     0, false, "Screeches drown out all other sound!" },
    { "Robot_SecurityBot",                    "Security Bot",                    20,     0, false, "A battle cry chants in the distance!" },
    { "Robot_HeavySecBot",                    "Heavy Security Bot Type-1",       30,     0, false, "A battle cry chants in the distance!" },
    { "Robot_HeavySecBotMilitary",            "Heavy Security Bot Type-2",       30,     0, false, "A battle cry chants in the distance!" },
    { "Robot_DreadnoughtBot",                 "Dreadnought Bot",                 30,     0, false, "Seems you've found Hell's nobility!" },
    { "Robot_HEATDrone",                      "HEAT Drone",                      35,     0, false, "Seems you've found Hell's nobility!" },
    { "Robot_HeavyWarBot",                    "Heavy War Bot",                   40,     0, false, "Seems you've found Hell's nobility!" },
    { "Robot_AHRA",                           "AHRA",                            50,     0, false, "Seems you've found Hell's nobility!" },
    { "Robot_FabricationNode",                "Fabrication Node",                40,     0, false, "You feel you are being watched!" },
    { "Robot_RollerDroid",                    "Roller Droid",                    50,     0, false, "Bones clatter all around you!" },
    { "Robot_MaintenanceBot",                 "Maintenance Bot",                 60,     0, false, "You hear deep, guttural noises!" },
    { "Robot_SpiderBot",                      "Spider Bot",                      60,     0, false, "Leg servos squeak and whirr nearby!" },
    { "Robot_GunTurret",                      "Gun Turret",                      60,     0, false, "Leg servos squeak and whirr nearby!" },
    { "Robot_SecurityTurret",                 "Security Turret",                 65,     0, false, "Leg servos squeak and whirr nearby!" },
    { "Robot_RepairBot",                      "Repair Bot",                      50,     0, false, "You hear crackling flames!" },
//  Boss Enemies.
    { "Robot_MilitaryBot",                    "Military Bot",                    85,     0, true,  "Suddenly you have a great urge to turn back! You scream in TERROR!" },
    { "Robot_MilitaryBotBoss",                "Military Bot",                    105,     0, true,  "Suddenly you have a great urge to turn back! You scream in TERROR!" },
    { "Robot_LoaderBot",                      "Loader Bot",                      95,     0, true,  "You've stumbled into a hive of the deadliest kind!" },
    { "Robot_LoaderBotBoss",                  "Loader Bot",                      115,     0, true,  "You've stumbled into a hive of the deadliest kind!" },
    { "Robot_SpiderTank",                     "Spider Tank",                     100,     0, true,  "Suddenly you have a great urge to turn back! You scream in TERROR!" },
    { "Robot_Deathwalker",                    "Deathwalker",                     120,     0, true,  "Suddenly you have a great urge to turn back! You scream in TERROR!" },
    { "Robot_SiegeDroid",                     "Siege Droid",                     125,     0, true,  "Suddenly you have a great urge to turn back! You scream in TERROR!" }
};

MonsterInfo const MonsterDataDEHACKED[MAX_DEF_MONSTERS_DF] =
{
    { "DEHZombieman",                         "Former Human",                    1,      0, false, "You hear shuffling footsteps and moans!" },
    { "DEHShotgunguy",                        "Former Sergeant",                 2,      0, false, "You hear the sound of shotguns pumping!" },
    { "DEHChaingunguy",                       "Former Commando",                 5,      0, false, "Quiet mutters and whirring set you on edge!" },
    { "DEHDoomImp",                           "Imp",                             4,      0, false, "The walls are scratched and flame-scorched!" },
    { "DEHDemon",                             "Demon",                           3,      0, false, "Hungry growls echo around you!" },
    { "DEHSpectre",                           "Spectre",                         3,      0, false, "It's dark, you are likely to be eaten by a... Spectre?" },
    { "DEHLostSoul",                          "Lost Soul",                       3,      0, false, "The sound of many flames echo about!" },
    { "DEHCacodemon",                         "Cacodemon",                       10,     0, false, "Screeches drown out all other sound!" },
    { "DEHHellKnight",                        "Hell Knight",                     20,     0, false, "A battle cry chants in the distance!" },
    { "DEHBaronOfHell",                       "Baron of Hell",                   30,     0, false, "Seems you've found Hell's nobility!" },
    { "DEHArachnotron",                       "Arachnotron",                     40,     0, false, "You feel you are being watched!" },
    { "DEHPainElemental",                     "Pain Elemental",                  50,     0, false, "Bones clatter all around you!" },
    { "DEHRevenant",                          "Revenant",                        60,     0, false, "You hear deep, guttural noises!" },
    { "DEHFatso",                             "Mancubus",                        60,     0, false, "Leg servos squeak and whirr nearby!" },
    { "DEHArchvile",                          "Arch-Vile",                       70,     0, false, "You hear crackling flames!" },
    { "DEHCyberdemon",                        "Cyberdemon",                      85,     0, true,  "Suddenly you have a great urge to turn back! You scream in TERROR!" },
    { "DEHSpiderMastermind",                  "Spider Mastermind",               95,     0, true,  "You've stumbled into a hive of the deadliest kind!" }
};

MonsterInfo const MonsterDataDRLA[MAX_DEF_MONSTERS_DRLA] =
{
    // Normal
    { "RLFormerHumanPistol",                "Former Human",                      1,      0, false, "You hear shuffling footsteps and moans!" },
    { "RLFormerHumanBattleRifle",           "Former Human",                      3,      0, false, "You hear shuffling footsteps and moans!" },
    { "RLFormerHumanBattleRifle2",          "Former Human",                      6,      0, false, "You hear shuffling footsteps and moans!" },
    { "RLFormerHumanBattleRifle3",          "Former Human",                      9,      1, false, "You hear shuffling footsteps and moans!" },
    { "RLFormerSergeantShotgun",            "Former Sergeant",                   12,     1, false, "You hear the sound of shotguns pumping!" },
    { "RLFormerSergeantCombatShotgun",      "Former Sergeant",                   18,     1, false, "You hear the sound of shotguns pumping!" },
    { "RLFormerCommandoChaingun",           "Former Commando",                   29,     2, false, "Quiet mutters and whirring set you on edge!" },
    { "RLFormerCommandoBattleRifle",        "Former Commando",                   27,     2, false, "Quiet mutters and magazines being loaded set you on edge!" },
    { "RLFormerCaptainPlasmaRifle",         "Former Captain",                    58,     5, false, "The sound of a plasma rifle priming makes you flinch!" },
    { "RLImp",                              "Imp",                               22,     2, false, "The walls are scratched and flame-scorched!" },
    { "RLDemon",                            "Demon",                             34,     4, false, "Hungry growls echo around you!" },
    { "RLSpectre",                          "Spectre",                           45,     5, false, "It's dark, you are likely to be eaten by a... Spectre?" },
    { "RLLostSoul",                         "Lost Soul",                         38,     1, false, "The sound of many flames echo about!" },
    { "RLCacodemon",                        "Cacodemon",                         54,     7, false, "Screeches drown out all other sound!" },
    { "RLHellKnight",                       "Hell Knight",                       62,     12, false, "A battle cry chants in the distance!" },
    { "RLBaronOfHell",                      "Baron of Hell",                     72,     14, false, "Seems you've found Hell's nobility!" },
    { "RLArachnotron",                      "Arachnotron",                       82,     12, false, "Leg servos squeak and whirr nearby!" },
    { "RLPainElemental",                    "Pain Elemental",                    74,     19, false, "You feel you are being watched!" },
    { "RLRevenant",                         "Revenant",                          92,     15, false, "Bones clatter all around you!" },
    { "RLMancubus",                         "Mancubus",                          102,    16, false, "You hear deep, guttural noises!" },
    { "RLArchvile",                         "Arch-Vile",                         112,    18, false, "You hear crackling flames!" },
    { "RLCyberdemon",                       "Cyberdemon",                        268,    24, true,  "Suddenly you have a great urge to turn back!\nYou scream in TERROR!" },
    { "RLSpiderMastermind",                 "Spider Mastermind",                 278,    24, true,  "The smell of sulfur and gunpowder hangs in the air, the war machine is close." },
    { "RLSpiderMastermindMortar",           "Spider Disastermind",               284,    25, true,  "Looks like this is where all the local invasions are planned!" },

    // Special Normal enemies
    { "RLBruiserBrother",                   "Bruiser Brothers",                  294,    24, true,  "The heat in here has gotten intense!" },

    // Nightmare
    { "RLEliteHumanCombatPistol",           "Elite Human",                       63,     4, false, "The sound of heavy gunshots alerts you!" },
    { "RLEliteHumanHandcannon",             "Elite Human",                       65,     4, false, "Loud gunshots echo throughout the area!" },
    { "RLEliteHumanBattleRifle",            "Elite Human",                       66,     4, false, "You notice a lot of empty rifle magazines on the ground..." },
    { "RLEliteHumanBattleRifle2",           "Elite Human",                       68,     4, false, "You notice a lot of empty rifle magazines on the ground..." },
    { "RLEliteHumanUzi",                    "Elite Human",                       69,     4, false, "Looks like a whole bunch of recon squads ended up here..." },
    { "RLEliteSergeantDoubleShotgun",       "Elite Sergeant",                    75,     6, false, "Veteran soldiers patrol the halls, seeking you!" },
    { "RLEliteSergeantAssaultShotgun",      "Elite Sergeant",                    76,     5, false, "Veteran soldiers patrol the halls, seeking you!" },
    { "RLEliteSergeantPlasmaShotgun",       "Elite Sergeant",                    78,     5, false, "Veteran soldiers patrol the halls, seeking you!" },
    { "RLEliteCommandoMinigun",             "Elite Commando",                    86,     7, false, "Heavy ammo belts rustle and clank in the dark!" },
//  { "RLEliteCommandoLaserRifle",          "Elite Commando",                    112,    9, false, "Spent laser rifle cells litter the floor..." },
    { "RLEliteCaptainTristarBlaster",       "Elite Captain",                     114,    9, false, "You notice some power cells on the floor,\nused only by the most powerful weapons!" },
//  { "RLEliteCaptainMissileLauncher",      "Elite Captain",                     115,    9, false, "Seems like this area is being used to test some tank buster weapons!" },
//  { "RLEliteCaptainNapalmLauncher",       "Elite Captain",                     117,    9, false, "Empty napalm drums and scorched walls indicate some serious weaponry!" },
    { "RLNightmareImp",                     "Nightmare Imp",                     88,     7, false, "You notice dark nebulous enemies are hunting you down!" },
    { "RLNightmareDemon",                   "Nightmare Demon",                   98,     7, false, "Roars and growls fill the air!" },
    { "RLNightmareSpectre",                 "Nightmare Spectre",                 106,    7, false, "You feel you are being stalked by something unseen!" },
    { "RLNightmareLostSoul",                "Nightmare Soul",                    109,    4, false, "You feel like you are being haunted!" },
    { "RLNightmareCacodemon",               "Nightmare Cacodemon",               119,    9, false, "The air is crackling with electricity!" },
    { "RLNightmareHellKnight",              "Nightmare Knight",                  122,    13, false, "Heavy hoofsteps echo through the chambers!" },
    { "RLNightmareBaronOfHell",             "Baron of Nightmares",               132,    16, false, "Commanding roars warn you of danger ahead!" },
    { "RLNightmareArachnotron",             "Nightmare Arachnotron",             142,    14, false, "The charging of heavy plasma guns fills the air with humming!" },
    { "RLNightmarePainElemental",           "Nightmare Elemental",               136,    20, false, "You feel like Hell's invisible influence\nis leaking into this area faster!" },
    { "RLNightmareRevenant",                "Nightmare Revenant",                154,    16, false, "Distant screams echo through the air and your mind!" },
    { "RLNightmareMancubus",                "Nightmare Mancubus",                162,    17, false, "You hear deep growling and the priming of heavy plasma cannons!" },
    { "RLNightmareArchvile",                "Nightmare Arch-Vile",               172,    19, false, "You suddenly break out in a flash sweat!" },
    { "RLNightmareCyberdemon",              "Nightmare Cyberdemon",              288,    24, true,  "Surrounded by hoofsteps and deep laughter,\nyou know you will not leave alive!" },
    { "RLNightmareSpiderMastermind",        "Nightmare Mastermind",              296,    25, true,  "Insane screeching and the pounding of\nmetal legs drowns out all other sound!" },

    // Special Nightmare enemies
    { "RLAgonyElemental",                   "Agony Elemental",                   304,    25, true, "You feel the presence of great conduits of the damned!" },
    { "RLAbominant",                  	    "Abominant",                         314,    25, true, "Death waits for the slightest lapse in concentration." },

    // Cybernetic
    { "RLFormerCyborgBattleRifle",          "Former Human Cyborg",               128,    6, false, "Sounds like a platoon of the UAC's finest got corrupted!" },
//  { "RLFormerCyborgSergeantShotgun",      "Former Human Cyborg Sergeant",      138,    10, false, "You hear whirring legs and the cracks of shotgun fire!" },
//  { "RLFormerCyborgCommandoChaingun",     "Former Human Cyborg Commando",      146,    10, false, "You realise that the UAC still hasn't\nlearned Hell likes to possess soldiers!" },
    { "RLCyberneticImp",                    "Cyber-Imp",                         148,    10, false, "Arcs of electricity and howling creatures greet you!" },
    { "RLCyberneticDemon",                  "Mech-Demon",                        158,    9, false, "Stomping metal feet ring in your ears!" },
    { "RLCyberneticSpectre",                "Mechtre",                           165,    9, false, "You hear the sound of a cloaking device shorting out!" },
    { "RLCyberneticLostSoul",               "Hellmine",                          169,    7, false, "It's quiet...\ntoo quiet..." },
//  { "RLCyberneticCacodemon",              "Cyber-Cacodemon",                   178,    11, false, "The hum of antigrav drives vibrates through your body!" },
    { "RLCyberneticHellKnight",             "Cybruiser",                         182,    15, false, "Clanking footsteps herald Hell's mechanical legions!" },
    { "RLCyberneticBaronOfHell",            "Techno-Lord",                       192,    15, false, "Uneven footsteps marks the arrival of some of Hell's cyberforces!" },
    { "RLCyberneticArachnotron",            "Arachnosentinel",                   202,    16, false, "The walls are covered in bullet holes!" },
    { "RLCyberneticPainElemental",          "Machine Elemental",                 195,    20, false, "You hear humming and beeping everywhere!" },
    { "RLCyberneticRevenant",               "Heavy Revenant",                    212,    17, false, "Seems they needed bigger rocket launchers!" },
    { "RLCyberneticMancubus",               "Volacubus",                         222,    16, false, "The smell of burning fuel fills this area!" },
    { "RLCyberneticArchvile",               "Tech-Vile",                         232,    19, false, "You hear electrical zaps and pulsing energy waves!" },
    { "RLCyberneticCyberdemon",             "Cyberdemon Mk. II",                 304,    24, true,  "You hear crashing metal footfalls! You can't stop shaking!" },
    { "RLCyberneticSpiderMastermind",       "Spider Overmind",                   308,    25, true,  "You hear endless whispering in your mind!\nYou feel your resistance slipping!" },

    // Special Technophobia enemies
    { "RLTerminator",                       "Terminator",                        316,    25, true,  "Distorted digital roars mark the beginning of your end!" },

    // Armageddon
    { "RLFormerAssaultTrooper",             "Former Human Assault Trooper",      188,    8, false, "Marching feet and magazines being slapped into\nplace alerts you of the battalion here!" },
    { "RLFormerOverwatch",                  "Former Human Overwatch",            199,    12, false, "You hear the sound of someone activating a hover drone nearby..." },
    { "RLFormerShocktrooper",               "Former Human Shocktrooper",         205,    11, false, "You get the feeling someone is about to get the drop on you..." },
    { "RLArmageddonImp",                    "The Watcher",                       209,    11, false, "You feel incredibly uneasy, as though many things are watching you!" },
    { "RLArmageddonDemon",                  "Mech-strosity",                     218,    10, false, "You feel that using a gun here might be a baaaad idea!" },
//  { "RLArmageddonSpectre",                "Rift Demon",                        215,    10, false, "There don't appear to be any monsters, but that's likely to change!" },
    { "RLArmageddonLostSoul",               "The Hungry",                        218,    7, false, "You hear endless slavering mouths, hoping to consume human flesh!" },
    { "RLArmageddonCacodemon",              "Cacobyss",                          238,    12, false, "Metallic echoing screeches herald your doom!" },
//  { "RLArmageddonHellKnight",             "Crusader of Ruination",             244,    17, false, "You hear the clanking of metal hooves and the smell of burnt flesh!" },
    { "RLArmageddonBaronOfHell",            "Lord of the Abyss",                 252,    16, false, "Deep laughter and endless flames welcome you to your death!" },
    { "RLArmageddonArachnotron",            "Deathweaver",                       264,    18, false, "You feel you've intruded somewhere that you should not have!" },
    { "RLArmageddonPainElemental",          "Hellstorm Elemental",               258,    19, false, "The air ripples with instability!\nYou feel like you are being slowly torn apart!" },
    { "RLArmageddonRevenant",               "Revenihilator",                     275,    18, false, "Beeping and the sound of bones scraping\nagainst metal alerts you to the danger ahead!" },
    { "RLArmageddonMancubus",               "Magmabus",                          282,    19, false, "Deep growling and blasts of searing heat greet you here!" },
    { "RLArmageddonArchvile",               "De-Vile",                           292,    21, false, "This entire area seems to be corrupted with great and terrible magic!" },
//  { "RLArmageddonCyberdemon",             "Nightmare Cyberdemon Mk. II",       306,    24, true,  "Hell has gone all out to stop you!\nYou are one doomed space marine..." },
//  { "RLArmageddonSpiderMastermind",       "Nightmare Overmind",                309,    25, true,  "The greatest of foul minds,\nthey taunt you telepathically before their hunt begins!" },

    // Special Armageddon enemies
    // These guys should never be one monsters or anything like that.
    // [KS] Should we add them as Megabosses instead?
//  { "RLArmageddonCyberdemon2",            "Cyber-Riftlord",                    316,    25, true, "You can feel the presence of dark entities beyond your comprehension!" },
//  { "RLArmageddonSpiderMastermind2",      "Supreme Overmind",                  318,    25, true, "Seems like you found the grand leaders of the invasion here!" }
};

// Colourful Hell
MonsterInfo const MonsterDataCH[MAX_DEF_MONSTERS_CH] =
{
    // Zombieman
    { "CommonZombie",                         "Former Human",                     1,      0, false, "You hear shuffling footsteps and moans!" },
    { "GreenZombie",                          "Uncommon Former Human",            2,      0, false, "You hear shuffling footsteps and moans!" },
    { "BlueZombie",                           "Rare Former Human",                2,      0, false, "You hear shuffling footsteps and moans!" },
    { "PurpleZombie",                         "Epic Former Human",                3,      0, false, "You hear shuffling footsteps and moans!" },
    { "YellowZombie",                         "Legendary Former Human",           3,      1, false, "You hear shuffling footsteps and moans!" },
    { "RedZombie",                            "Demonic Former Human",             8,      1, false, "You hear shuffling footsteps and moans!" },
    { "BlackZombie1",                         "Player 9",                         70,     3, true,  "You hear shuffling footsteps and moans!" },
    { "WhiteZombie1",                         "The Undertaker",                   90,     4, true,  "You hear shuffling footsteps and moans!" },

    // Shotgunner
    { "CommonSG",                             "Former Sergeant",                  2,      0, false, "You hear the sound of shotguns pumping!" },
    { "GreenSG",                              "Uncommon Former Sergeant",         3,      0, false, "You hear the sound of shotguns pumping!" },
    { "BlueSG",                               "Rare Former Sergeant",             4,      0, false, "You hear the sound of shotguns pumping!" },
    { "PurpleSG",                             "Epic Former Sergeant",             5,      0, false, "You hear the sound of shotguns pumping!" },
    { "YellowSG",                             "Legendary Former Sergeant",        6,      1, false, "You hear the sound of shotguns pumping!" },
    { "RedSG",                                "Demonic Former Sergeant",          10,     1, false, "You hear the sound of shotguns pumping!" },
    { "BlackSG3",                             "Shotgun Crew Commander",           75,     3, true,  "You hear the sound of shotguns pumping!" },
    { "WhiteSG2",                             "Benellus, God of Shotguns",        90,     4, true,  "You hear the sound of shotguns pumping!" },

    // Chaingunner
    { "CommonCguy",                           "Former Commando",                  5,      0, false, "Quiet mutters and whirring set you on edge!" },
    { "GreenCguy",                            "Uncommon Former Commando",         6,      0, false, "Quiet mutters and whirring set you on edge!" },
    { "BlueCguy",                             "Rare Former Commando",             8,      0, false, "Quiet mutters and whirring set you on edge!" },
    { "PurpleCguy",                           "Epic Former Commando",             10,     1, false, "Quiet mutters and whirring set you on edge!" },
    { "YellowCguy",                           "Legendary Former Commando",        14,     1, false, "Quiet mutters and whirring set you on edge!" },
    { "RedCguy",                              "Demonic Former Commando",          16,     2, false, "Quiet mutters and whirring set you on edge!" },
    { "BlackCguy2",                           "Mr. General",                      70,     3, true,  "Quiet mutters and whirring set you on edge!" },
    { "WhiteCguy2",                           "Crazy Lady Scientist",             80,     4, true,  "Quiet mutters and whirring set you on edge!" },

    // Imp
    { "CommonImp",                            "Imp",                              4,      0, false, "The walls are scratched and flame-scorched!" },
    { "GreenImp",                             "Uncommon Imp",                     5,      0, false, "The walls are scratched and flame-scorched!" },
    { "BlueImp",                              "Rare Imp",                         6,      0, false, "The walls are scratched and flame-scorched!" },
    { "PurpleImp",                            "Epic Imp",                         7,      0, false, "The walls are scratched and flame-scorched!" },
    { "YellowImp",                            "Legendary Imp",                    9,      1, false, "The walls are scratched and flame-scorched!" },
    { "RedImp",                               "Demonic Imp",                      15,     1, false, "The walls are scratched and flame-scorched!" },
    { "BlackImp1",                            "Smoking Black Imp",                50,     2, true,  "The walls are scratched and flame-scorched!" },
    { "WhiteImp2",                            "Imp Master",                       60,     3, true,  "The walls are scratched and flame-scorched!" },

    // Demon
    { "CommonDemon",                          "Demon",                            3,      0, false, "Hungry growls echo around you!" },
    { "GreenDemon",                           "Uncommon Demon",                   4,      0, false, "Hungry growls echo around you!" },
    { "BlueDemon",                            "Rare Demon",                       5,      0, false, "Hungry growls echo around you!" },
    { "PurpleDemon",                          "Epic Demon",                       6,      0, false, "Hungry growls echo around you!" },
    { "YellowDemon",                          "Legendary Demon",                  12,     1, false, "Hungry growls echo around you!" },
    { "RedDemon",                             "Demonic Demon",                    21,     2, false, "Hungry growls echo around you!" },
    { "BlackDemon3",                          "The Butcher",                      55,     3, true,  "Hungry growls echo around you!" },
    { "WhiteDemon2",                          "The Juggernaut",                   70,     4, true,  "Hungry growls echo around you!" },

    // Spectre
    { "CommonSpectre",                        "Spectre",                          3,      0, false, "It's dark, you are likely to be eaten by a... Spectre?" },
    { "GreenSpectre",                         "Uncommon Spectre",                 4,      0, false, "It's dark, you are likely to be eaten by a... Spectre?" },
    { "BlueSpectre",                          "Rare Spectre",                     5,      0, false, "It's dark, you are likely to be eaten by a... Spectre?" },
    { "PurpleSpectre",                        "Epic Spectre",                     7,      0, false, "It's dark, you are likely to be eaten by a... Spectre?" },
    { "YellowSpectre",                        "Legendary Spectre",                14,     1, false, "It's dark, you are likely to be eaten by a... Spectre?" },
    { "RedSpectre",                           "Demonic Spectre",                  20,     2, false, "It's dark, you are likely to be eaten by a... Spectre?" },
    { "BlackSpectre2",                        "The Rogue",                        64,     3, true,  "It's dark, you are likely to be eaten by a... Spectre?" },
    { "WhiteSpectre2",                        "???",                              77,     4, true,  "It's dark, you are likely to be eaten by a... Spectre?" },

    // Cacodemon
    { "CommonCaco",                           "Cacodemon",                        10,     0, false, "Screeches drown out all other sound!" },
    { "GreenCaco",                            "Uncommon Cacodemon",               11,     0, false, "Screeches drown out all other sound!" },
    { "BlueCaco",                             "Rare Cacodemon",                   12,     1, false, "Screeches drown out all other sound!" },
    { "PurpleCaco",                           "Epic Cacodemon",                   14,     1, false, "Screeches drown out all other sound!" },
    { "YellowCaco",                           "Cacolich",                         22,     2, false, "Screeches drown out all other sound!" },
    { "RedCaco",                              "Demonic Cacodemon",                30,     3, false, "Screeches drown out all other sound!" },
    { "BlackCaco2",                           "Shockmaster",                      56,     4, true,  "Screeches drown out all other sound!" },
    { "WhiteCaco2",                           "Terrifying Cacodemon",             87,     5, true,  "Screeches drown out all other sound!" },

    // Hell Knight
    { "CommonHK",                             "Hell Knight",                      20,     0, false, "A battle cry chants in the distance!" },
    { "GreenHK",                              "Uncommon Hell Knight",             22,     0, false, "A battle cry chants in the distance!" },
    { "BlueHK",                               "Rare Hell Knight",                 24,     1, false, "A battle cry chants in the distance!" },
    { "PurpleHK",                             "Epic Hell Knight",                 29,     1, false, "A battle cry chants in the distance!" },
    { "YellowHK",                             "Bruiser",                          38,     2, false, "A battle cry chants in the distance!" },
    { "RedHK",                                "Hell Knightmare",                  50,     3, false, "A battle cry chants in the distance!" },
    { "BlackHK2",                             "The Terminator",                   74,     4, true,  "A battle cry chants in the distance!" },
    { "WhiteHK3",                             "Ghost of 1993",                    84,     5, true,  "A battle cry chants in the distance!" },

    // Baron of Hell
    { "CommonBaron",                          "Baron of Hell",                    30,     0, false, "Seems you've found Hell's nobility!" },
    { "GreenBaron",                           "Uncommon Baron of Hell",           34,     1, false, "Seems you've found Hell's nobility!" },
    { "BlueBaron",                            "Rare Baron of Hell",               38,     2, false, "Seems you've found Hell's nobility!" },
    { "PurpleBaron",                          "Epic Baron of Hell",               42,     3, false, "Seems you've found Hell's nobility!" },
    { "YellowBaron",                          "Grand Baron of Hell",              55,     3, false, "Seems you've found Hell's nobility!" },
    { "RedBaron3",                            "Red Baron",                        69,     4, false, "Seems you've found Hell's nobility!" },
    { "BlackBaron2",                          "Baron of the Abyss",               90,     5, true,  "Seems you've found Hell's nobility!" },
    { "WhiteBaron2",                          "Valkyrie of Hell",                 98,     5, true,  "Seems you've found Hell's nobility!" },

    // Lost Soul
    { "CommonLSoul",                          "Lost Soul",                        3,      0, false, "The sound of many flames echo about!" },
    { "GreenLSoul",                           "Uncommon Lost Soul",               4,      0, false, "The sound of many flames echo about!" },
    { "BlueLSoul",                            "Rare Lost Soul",                   5,      0, false, "The sound of many flames echo about!" },
    { "PurpleLSoul",                          "Epic Lost Soul",                   6,      0, false, "The sound of many flames echo about!" },
    { "YellowLSoul",                          "Legendary Lost Soul",              8,      0, false, "The sound of many flames echo about!" },
    { "RedLSoul",                             "Bloody Lost Soul",                 12,     1, false, "The sound of many flames echo about!" },
    { "BlackLSoul3",                          "Queen Bee",                        18,     2, true,  "The sound of many flames echo about!" },
    { "WhiteLSoul2",                          "The Shifter",                      25,     3, true,  "The sound of many flames echo about!" },

    // Pain Elemental
    { "CommonPE",                             "Pain Elemental",                   40,     0, false, "You feel you are being watched!" },
    { "GreenPE",                              "Uncommon Pain Elemental",          42,     1, false, "You feel you are being watched!" },
    { "BluePE",                               "Rare Pain Elemental",              44,     1, false, "You feel you are being watched!" },
    { "PurplePE",                             "Epic Pain Elemental",              46,     2, false, "You feel you are being watched!" },
    { "YellowPE",                             "Volcanic Pain Elemental",          55,     3, false, "You feel you are being watched!" },
    { "RedPE",                                "Rage Elemental",                   65,     4, false, "You feel you are being watched!" },
    { "BlackPE2",                             "Hell Soul Elemental",              80,     5, true,  "You feel you are being watched!" },
    { "WhitePE2",                             "The Watcher",                      95,     5, true,  "You feel you are being watched!" },

    // Revenant
    { "CommonRevenant",                       "Revenant",                         50,     0, false, "Bones clatter all around you!" },
    { "GreenRevenant",                        "Uncommon Revenant",                52,     1, false, "Bones clatter all around you!" },
    { "BlueRevenant",                         "Rare Revenant",                    54,     1, false, "Bones clatter all around you!" },
    { "PurpleRevenant",                       "Epic Revenant",                    58,     2, false, "Bones clatter all around you!" },
    { "YellowRevenant",                       "Ghostly Revenant",                 68,     3, false, "Bones clatter all around you!" },
    { "RedRevenant",                          "Bloody Revenant",                  78,     4, false, "Bones clatter all around you!" },
    { "BlackRevenant3",                       "The Black Knight",                 88,     5, true,  "Bones clatter all around you!" },
    { "WhiteRevenant2",                       "The Lichest Lich",                 94,     5, true,  "Bones clatter all around you!" },

    // Mancubus
    { "CommonFatso",                          "Mancubus",                         50,     0, false, "You hear deep, guttural noises!" },
    { "GreenFatso",                           "Uncommon Mancubus",                54,     1, false, "You hear deep, guttural noises!" },
    { "BlueFatso",                            "Rare Mancubus",                    58,     1, false, "You hear deep, guttural noises!" },
    { "PurpleFatso",                          "Epic Mancubus",                    63,     2, false, "You hear deep, guttural noises!" },
    { "YellowFatso",                          "Legendary Mancubus",               70,     3, false, "You hear deep, guttural noises!" },
    { "RedFatso",                             "Horned Beast",                     80,     4, false, "You hear deep, guttural noises!" },
    { "BlackFatso2",                          "Dark Beast",                       91,     5, true,  "You hear deep, guttural noises!" },
    { "WhiteFatso2",                          "Angry Mama",                       98,     5, true,  "You hear deep, guttural noises!" },

    // Arachnotron
    { "CommonSP1",                            "Arachnotron",                      45,     0, false, "Leg servos squeak and whirr nearby!" },
    { "GreenSP1",                             "Uncommon Arachnotron",             50,     0, false, "Leg servos squeak and whirr nearby!" },
    { "BlueSP1",                              "Rare Arachnotron",                 55,     1, false, "Leg servos squeak and whirr nearby!" },
    { "PurpleSP1",                            "Epic Arachnotron",                 60,     2, false, "Leg servos squeak and whirr nearby!" },
    { "YellowSP1",                            "Brainchotron",                     67,     3, false, "Leg servos squeak and whirr nearby!" },
    { "RedSP1",                               "Raging Arachnotron",               78,     4, false, "Leg servos squeak and whirr nearby!" },
    { "BlackSP2",                             "Explosive Arachnotron",            87,     5, true,  "Leg servos squeak and whirr nearby!" },
    { "WhiteSP11",                            "Ungoliant",                        92,     5, true,  "Leg servos squeak and whirr nearby!" },

    // Archvile
    { "CommonArch",                           "Archvile",                         70,     0, false, "You hear crackling flames!" },
    { "GreenArch",                            "Uncommon Archvile",                74,     1, false, "You hear crackling flames!" },
    { "BlueArch",                             "Rare Archvile",                    78,     1, false, "You hear crackling flames!" },
    { "PurpleArch",                           "Epic Archvile",                    82,     2, false, "You hear crackling flames!" },
    { "YellowArch",                           "Golden Archvile",                  86,     3, false, "You hear crackling flames!" },
    { "RedArch2",                             "Infernovile",                      90,     4, false, "You hear crackling flames!" },
    { "BlackVile",                            "The Void",                         94,     5, true,  "You hear crackling flames!" },
    { "Whitevile",                            "Godly Archvile",                   98,     5, true,  "You hear crackling flames!" },

    // Cyberdemon
    { "CommonCybie",                          "Cyberdemon",                       85,     1, true,  "Suddenly you have a great urge to turn back! You scream in TERROR!" },
    { "GreenCybie",                           "Uncommon Cyberdemon",              88,     2, true,  "Suddenly you have a great urge to turn back! You scream in TERROR!" },
    { "BlueCybie",                            "Rare Cyberdemon",                  92,     3, true,  "Suddenly you have a great urge to turn back! You scream in TERROR!" },
    { "PurpleCybie",                          "Epic Cyberdemon",                  94,     4, true,  "Suddenly you have a great urge to turn back! You scream in TERROR!" },
    { "YellowCybie",                          "Legendary Cyberdemon",             96,     4, true,  "Suddenly you have a great urge to turn back! You scream in TERROR!" },
    { "RedCybie",                             "Moloch",                           98,     5, true,  "Suddenly you have a great urge to turn back! You scream in TERROR!" },
    { "BlackCybie2",                          "Cybersmith",                       99,     5, true,  "Suddenly you have a great urge to turn back! You scream in TERROR!" },
    { "WhiteCybie2",                          "DOOM.EXE",                         100,    5, true,  "Suddenly you have a great urge to turn back! You scream in TERROR!" },

    // Mastermind
    { "CommonMind",                           "Spider Mastermind",                93,     1, true,  "You've stumbled into a hive of the deadliest kind!" },
    { "GreenMind",                            "Uncommon Spider Mastermind",       94,     2, true,  "You've stumbled into a hive of the deadliest kind!" },
    { "BlueMind",                             "Rare Spider Mastermind",           95,     3, true,  "You've stumbled into a hive of the deadliest kind!" },
    { "PurpleMind",                           "Epic Spider Mastermind",           96,     4, true,  "You've stumbled into a hive of the deadliest kind!" },
    { "YellowMind",                           "Legendary Spider Mastermind",      97,     4, true,  "You've stumbled into a hive of the deadliest kind!" },
    { "RedMind",                              "Arachnophyte",                     98,     5, true,  "You've stumbled into a hive of the deadliest kind!" },
    { "BlackMind2",                           "Pseudo Old God",                   99,     5, true,  "You've stumbled into a hive of the deadliest kind!" },
};

//Pandemonia Monsters
MonsterInfo const MonsterDataPANDM[MAX_DEF_MONSTERS_PANDM] =
{
    // Zombieman
    { "PistolZombie",                      "Pistol Zombie",                       1,      0, false, "You hear shuffling footsteps and moans!" },
    { "PlasmaPistolZombie",                "Plasma Pistol Zombie",               32,      3, false, "Scientist notes laying around there..." },
    { "LaserRifleZombie",                  "Laser Rifle Zombie",                116,      6, false, "You feel like you're being targeted!" },
    { "CinderZombie",                  	   "Cinder Zombie",                		131,      7, false, "Someone want to take out you!" },
    { "SpecOpsRifleman",                   "Spec Ops Rifleman",                 164,      8, false, "Systems catch a mumbling radio communications.\nYou smell a plasma..." },

    // Shotgunner
    { "NewShotgunguy",                     "Shotgun Zombie",                     12,      1, false, "You hear the sound of shotguns pumping!" },
    { "SlugShotgunZombie",                 "Slug Shotgun Zombie",                50,      2, false, "You hear the sound of shotguns pumping!\nCorpses are lying around with a single hole in their heads..." },
    { "AutoShotgunZombie",                 "Auto Shotgun Zombie",                75,      5, false, "You see couple of 12cal. mags on the ground!" },
	{ "SuperShotgunZombie",                "Super Shotgun Zombie",               90,      7, false, "The sound of break action flies above splattered bodies!" },
    { "RocketZombie",                      "Rocket Zombie",                     101,      8, false, "A persistent smell of blood and rocket fuel is in the air." },
    { "PyroZombie",                		   "Pyro Zombie",               		117,      9, false, "A ship with a flamethrower supplies crashed nearby." },
    { "SpecOpsShotgunner",                 "Spec Ops Shotgunner",               138,     10, false, "Systems catch a mumbling radio communications.\nYou hear the sound of shotguns pumping behind your back!" },
    { "ChaosWarrior",                      "Chaos Warrior",                     180,     15, false, "Something is wrong here...\nAn undead, silent warriors are stalking this place." },
    { "ShotgunAdmiral",                    "Shotgun Admiral",                   199,     16, false, "Torn corpses are all around you... and the walls are covered in bullet holes!" },

    // Chaingunner
    { "AssaultGunner",                     "Assault Gunner",                     27,      1, false, "You notice a lot of empty rifle magazines on the ground..." },
    { "ChaingunZombie",                    "Chaingunner",                        29,      2, false, "Ammo belts rustle and clank in the dark!" },
    { "PlasmaRifleZombie",                 "Plasmagunner",                       40,      3, false, "The sound of a plasma rifle priming makes you flinch!" },
    { "NailborgCommando",                  "Nailborg Commando",                  75,      6, false, "Someone`s hand is nailed to the wall!" },
    { "ScourgeZombie",                 	   "Scourge Zombie",                  	 82,      7, false, "Radar detected toxicity infusion devices!" },
    { "SpecOpsGunner",                     "Spec Ops Gunner",                    88,      7, false, "Systems catch a mumbling radio communications.\nYou notice a lot of empty rifle magazines on the ground..." },
    { "BFGCommando",                       "BFG Commando",                      100,      8, false, "You notice some wasted power cells on the floor,\nused only by the most powerful weapons!" },
    { "InfernalCommando",                  "Infernal Commando",                 150,     11, false, "Enemy uses lots of pyro blast devices!" },
    { "ChaosDeathmark",                    "Chaos Deathmark",                   175,     14, false, "Something is wrong here...\nYou are marked for death!" },
	{ "SpecOpsLieutenant",                 "Spec Ops Lieutenant",               200,     15, false, "An evil laughter echoes everywhere! \nThe infernal gods demand a sacrifice..." },
    { "BFG10kWarrior",                     "BFG10k Warrior",                    225,     18, false, "Walls around you are twisted with a huge plasma holes." },

    // Imp
    { "NewImp",                            "Imp",                                22,      2, false, "The walls are scratched and flame-scorched!" },
    { "DarkImp",                           "Dark Imp",                           36,      3, false, "A sharp-toothed smiles are blinking in the darkness." },
    { "Devil",                             "Devil",                              60,      5, false, "The walls are scratched and flame-scorched!" },
    { "Scoundrel",                         "Scoundrel",                          48,      4, false, "You hear a mad beast chunter in the dark!" },
    { "PhaseImp",                          "Phase Imp",                         114,      9, false, "Something is watching you from another dimension." },
    { "BrimstoneImp",                      "Brimstone Imp",                     136,     11, false, "High concentrations of sulfur has been detected in this area." },
    { "Plagueling",                        "Plagueling",                        168,     14, false, "A gruesome toxic scent makes it hard to breathe." },
    { "ChaosImp",                          "Chaos Imp",                         204,     17, false, "Something is wrong here...\nRadioactive electrical zaps can be heard here." },
    { "AbyssalSavage",                     "Abyssal Savage",                    270,     20, false, "The Аbyss whispers to you...\nThe walls are scratched and flame-scorched." },

    // Demon
    { "NewDemon",                          "Demon",                              34,      4, false, "Hungry growls echo around you!" },
    { "MechDemon",                         "Mech Demon",                         72,      6, false, "Heavy metal stomps ringing in your ears!" },
    { "BloodFiend",                        "Blood Fiend",                       108,      9, false, "You see uncoagulated blood on the floor..." },
    { "MagmaDemon",                        "Magma Demon",                       156,     13, false, "Solidified magma particles are scattered everywhere." },
    { "PandMauler",                        "Mauler",                            168,     14, false, "Heavy stomps and menaing growls are heard around you!" },
    { "ChaosFlayer",                       "Chaos Flayer",                      204,     17, false, "Something is wrong here...\nThe dosimeter goes off the scale near THE wall scratch!" },

    // Spectre
    { "NewSpectre",                        "Spectre",                            45,      5, false, "It's dark, you are likely to be eaten by a... Spectre?" },
    { "Delusion",                          "Delusion",                           80,      7, false, "These creatures mimic a familiar aura!" },
    { "NightmareDemon",                    "Nightmare Demon",                   120,     10, false, "Shadows are lurking around!" },
    { "Wretch",                            "Wretch",                            160,     13, false, "You feel an eyeless things watching you..." },
    { "Darkbeast",                         "Darkbeast",                         188,     15, false, "Sensors detected the high concentration of metal and magic in the area!" },
    { "ChaosOphydian",                     "Chaos Ophydian",                    212,     17, false, "Something is wrong here...\nYou hear the invisibility mechanisms working!" },

    // Cacodemons
    { "NewCacodemon",                      "Cacodemon",                          54,      7, false, "Screeches drown out all other sound!" },
    { "DarkCacodemon",                     "Dark Cacodemon",                     86,      7, false, "Aggressive screeches drown out all other sound!" },
    { "Agathodemon",                       "Agathodemon",                       119,      9, false, "You hear the hum of an electrical substation...\nWhich is not there!" },
    { "Crackodemon",                       "Crackodemon",                       140,     11, false, "Your hair is on end from the electric field in this place!" },
	{ "Abaddon",                           "Abaddon",                       	155,     12, false, "The fires are looking for you!" },
    { "Witherdemon",                       "Witherdemon",                       178,     13, false, "The local corpses melted from some infection!" },
    { "Cacobot",                           "Cacobot",                           238,     15, false, "Something is interrupting the incoming signal..." },
    { "ChaosStalker",                      "Chaos Stalker",                     250,     16, false, "Something is wrong here...\nScreeches drown out all other sound!" },

    // Hell Knight
    { "NewHellKnight",                     "Hell Knight",                        64,     12, false, "A battle cry chants in the distance!" },
    { "HellViscount",                      "Hell Viscount",                     112,     13, false, "Heavy hoofsteps echo through the chambers!" },
    { "HellWarden",                        "Hell Warden",                       144,     14, false, "Shield strikes can be heard from afar!" },
    { "CyberHellKnight",                   "Cyber Hell Knight",                 182,     15, false, "Clanking footsteps herald Hell's mechanical legions!" },
    { "CyberHellViscount",                 "Cyber Hell Viscount",               204,     16, false, "You hear the clanking of metal hooves and the smell of burnt flesh!" },
    { "Infernoble",                        "Infernoble",                        244,     17, false, "An evil laugh can be heard through the flames!" },
    { "HellPrinceAsch",                    "Hell Prince Asch",                  300,     21, true,  "Beware!\nRoyalty descended on this place!" },

    // Baron of Hell
    { "NewBaronOfHell",                    "Baron Of Hell",                      72,     14, false, "Seems you've found Hell's nobility!" },
    { "Afrit",                             "Afrit",                             104,     14, false, "The heat hits you in the face!" },
    { "Cybruiser",                         "Cybruiser",                         136,     15, false, "Uneven footsteps marks the arrival of some of Hell's cyberforces!" },
    { "ArchonOfHell",                      "Archon Of Hell",                    158,     16, false, "Dying green flames are burning all around." },
    { "BruiserDemon",                      "BruiserDemon",                      182,     16, false, "The heat in here has gotten intense!" },
    { "DarkCybruiser",                     "Dark Cybruiser",                    214,     17, false, "Sensors detected dark technology in this location!" },
    { "Cyberwarden",                       "Cyberwarden",                       236,     18, false, "Sounds of shield bashing and guns loading echoes throught this sector." },
    { "Pyrobruiser",                       "Pyrobruiser",                       261,     21, false, "The walls tremble from the onslaught of the fiery army!" },
    { "Cydestructor",                      "Cydestructor",                      280,     22, false, "Hell`s armory has arrived!" },

    // Lost Souls
    { "NewLostSoul",                       "Lost Soul",                          38,      4, false, "The sound of many flames echo around!" },
    { "Phantasm",                          "Phantasm",                           60,      5, false, "An overwhelming stench of green flames is in the air." },
    { "Electrosoul",                       "Electrosoul",                        76,      6, false, "All the lighting in this place burned out..." },
    { "SoullessDrone",                     "Soulless Drone",                    109,      7, false, "Looks like the soulless automatons have overtaken this place." },
    { "PandWraith",                        "Wraith",                            169,      8, false, "You hear the whispers of lost souls!" },
    { "ChaosUmbra",                        "Chaos Umbra",                       218,      9, false, "Something is wrong here...\nAn otherworldly scream wafts above your head!" },

    // Pain Elemental
    { "NewPainElemental",                  "Pain Elemental",                     74,     17, false, "You feel like you're about to get swarmed!" },
    { "DarkElemental",                     "Dark Elemental",                    118,     18, false, "Toxic traces are everywhere!" },
    { "PlasmaticElemental",                "Plasmatic Elemental",               164,     19, false, "Sounds like a BFG shot is being prepared somewhere!" },
    { "HadesElemental",                    "Hades Elemental",                   228,     20, false, "An electric shriek can be heard from afar!" },
    { "AutomatonElemental",                "Automaton Elemental",               245,     20, false, "Advance cybernetic signal has been detected in the area." },
    { "ChaosBeholder",                     "Chaos Beholder",                    262,     21, false, "Something is wrong here...\nThe level of radioactive plasma exceeds all norms in this place!" },

    // Revevant
    { "NewRevenant",                       "Revenant",                           92,     15, false, "Bones clatter all around you!" },
    { "DarkRevenant",                      "Dark Revenant",                     110,     15, false, "Aggressive bone clatter all around you!" },
    { "MissileRevenant",                   "Missile Revenant",                  130,     16, false, "Sensors detect a large heavy rocket supplies in the area.\n...and no living sings." },
    { "Sentry",                            "Sentry",                            154,     17, false, "Screeching of bones and metal echoes from the corner!" },
    { "Apollyon",                          "Apollyon",                          170,     17, false, "A scout drone saw burning bones in this location!" },
    { "ChaosHollow",                       "Chaos Hollow",                      212,     18, false, "Something is wrong here...\nEmpty eye sockets are looking straight into your soul!" },
    { "PandSentinel",                      "Sentinel",                          240,     19, false, "Incoming radio transmission...\nEnemy heavy machines have been detected in your sector!" },
    { "AncientGladiator",                  "Ancient Gladiator",                 300,     21, true,  "Incoming radio transmission...\nYou hear the shout of the stands!" },

    // Mancubus
    { "Mancubus",                          "Mancubus",                          102,     16, false, "You hear deep, guttural noises!" },
    { "Hectebus",                          "Hectebus",                          138,     16, false, "You hear deep growling and the priming of heavy plasma cannons!" },
    { "Mafibus",                           "Mafibus",                           162,     17, false, "From the darkness, a red brawn is coming at you!" },
//  { "Tankubus",                          "Tankubus",                          224,     17, false, "Tracks of tank treads capture your attention!" },
    { "Corpulent",                         "Corpulent",                         253,     18, false, "Deep growling and blasts of searing heat greet you here!" },
    { "Maxibus",                           "Maxibus",                           282,     19, false, "You hear deeper, more guttular noises!" },
    { "ChaosPylon",                        "Chaos Pylon",                       290,     20, false, "Something is wrong here...\nAncient mechanisms detected you!" },

    // Arachnotron
    { "NewArachnotron",                    "Arachnotron",                        82,     12, false, "Leg servos squeak and whirr nearby!" },
    { "NailgunArachnotron",                "Nailgun Arachnotron",               104,     12, false, "Somebody's leg is nailed to the wall!" },
    { "FusionArachnotron",                 "Fusion Arachnotron",                128,     13, false, "The walls are covered with numberless marks of red plasma." },
    { "AugmentedArachnotron",              "Augmented Arachnotron",             142,     14, false, "The charging of heavy plasma guns fills the air with humming!" },
    { "Widow",                             "Widow",                             202,     15, false, "Surrounding corpses have railgun and plasma marks on them." },
    { "Arachnoscorcher",                   "Arachnoscorcher",                   212,     16, false, "It seems that the fire has even eaten into the walls." },
    { "Arachknight",                       "Arachknight",                       224,     17, false, "The ancient technology of hell has awakened!" },
    { "ChaosObserver",                     "Chaos Observer",                    264,     21, false, "Something is wrong here...\nFlying radiation sources detected in close proximity!" },
    { "Demolitron",                        "Demolitron",                        286,     22, false, "Alert!\nHeavy weapon platforms detected in this area!" },

    // Archvile
    { "NewArchvile",                       "Arch-Vile",                         112,     18, false, "You hear a crackling flames!" },
    { "DarkVile",                          "Dark-Vile",                         132,     18, false, "Green flames are visible around!" },
    { "Diabolist",                         "Diabolist",                         164,     18, false, "It looks like there was a big fire here..." },
    { "Infernalist",                       "Infernalist",                       192,     19, false, "An infernal levels of heat detected in this area." },
    { "Distortionist",                     "Distortionist",                     232,     19, false, "You feel the space and time distorting around you." },
    { "Cryptek",                           "Cryptek",                           292,     21, false, "Something is wrong here...\nCryptic destuctive power lingers nearby." },

    // Other
    { "Epic2Alien",                        "Alien",                              20,      4, false, "It seems aliens are hunting for our technology." },
	{ "ChaosCultist",                      "Chaos Cultist",                      30,      5, false, "You hear distant prayers in an unknown language." },
	{ "ChaosAcolyte",                      "Chaos Acolyte",                      40,      6, false, "Incoming radio transmission...\nDisrupt the summon of the Ecclesiarch, now!" },
    { "Scythe2Marine",                     "Traitor Marine",                     60,      8, false, "Traitors want to sacrifice you to the infernal gods!" },
    { "Scythe2Afrit",                      "Infernal Afrit",                     82,     14, false, "Strong flames are burning your gear!" },

    // Mastermind
    { "NewSpiderMastermind",               "Spider Mastermind",                 278,     24, true, "The smell of metal and gunpowder hangs in the air, the war machine is close." },
    { "Demolisher",                        "Demolisher",                        298,     24, true, "Superheavy weapon platforms detected in this area." },
    { "Arachnophyte",                      "Arachnophyte",                      299,     24, true, "Infernal aviation razes this place to the ground!" },
    { "SpiderMasterspark",                 "Spider Masterspark",                305,     24, true, "Some thoughts of hell have reached space..." },
    { "ChaosMonolith",                     "ChaosMonolith",                     318,     25, true, "Something is wrong here...\nAll wrong..." },

    // Cyberdemon
    { "NewCyberdemon",                     "Cyberdemon",                        268,     24, true, "Deafening metal footsteps are heard all around.\nYou hear the rocket launchers being loaded..." },
    { "Annihilator",                       "Annihilator",                       276,     24, true, "Deafening metal footsteps are heard all around.\nYou hear the rocket launchers being loaded... Twice as fast!" },
    { "DarkCyberdemon",                    "Dark Cyberdemon",                   284,     24, true, "Hell's whispering heralds a great pain and destruction!" },
    { "DarkCardinal",                      "Dark Cardinal",                     292,     24, true, "You hear screams of terror and a guttural roar!" },
    { "Terminator",                        "Terminator",                        298,     24, true, "Heavily armed demons desire your termination!" },
    { "Dreamweaver",                       "Dreamweaver",                       306,     24, true, "Something otherworldly and devastating weaves at the edge of your senses..." },
    { "Plamexis",                          "Plamexis",                          312,     25, true, "Hell's sun is shining on you!" },
    { "ChaosEcclesiarch",                  "Chaos Ecclesiarch",                 314,     25, true, "Something is wrong here...\nBow before the will of Chaos!" },
    { "Eradicator",                        "Eradicator",                        316,     25, true, "The strongest demonic beasts have come to eradicate your soul!\n...And your ears." },

    // Special Bosses
    { "GeneralRoasterock",                 "General Roasterock",                244,     24, true, "You feel like you're about to get roasted!" },
    { "Myrkura",                           "Myrkura",                           280,     24, true, "Bloodthirsty anomalous entities filled this place!" },

};

MegabossInfo const MegaBossesDF[MAX_MEGABOSSES_DF] =
{
    { "DRPGMegabossPride"},
    { "DRPGMegabossSloth"},
};

MegabossInfo const MegaBossesCH[MAX_MEGABOSSES_CH] =
{
    { "BlackCybie2RPG"},
    { "BlackMind2RPG"},
};

int NewMonsterID()
{
    int CurrentID = MonsterID++;
    if (CurrentID >= MAX_MONSTERS)
    {
        MonsterID = MAX_MONSTERS;
        return 0;
    }

    // When re-using an existing ID, all of these need to be reset so Init knows what to do with them
    Monsters[CurrentID].Actor = NULL;
    Monsters[CurrentID].ReplaceActor = "";
    Monsters[CurrentID].Flags = 0;
    Monsters[CurrentID].RenderStyle = 0;
    Monsters[CurrentID].SpawnHealth = 0;
    Monsters[CurrentID].HealthMax = 0;
    Monsters[CurrentID].Speed = 0;
    Monsters[CurrentID].Height = 0;
    Monsters[CurrentID].Radius = 0;
    Monsters[CurrentID].NeedReinit = false;
    Monsters[CurrentID].HealthBar = false;
    Monsters[CurrentID].Named = false;
    Monsters[CurrentID].Reinforcement = false;
    Monsters[CurrentID].Target = 0;
    Monsters[CurrentID].Level = 0;
    Monsters[CurrentID].LevelAdd = 0;
    Monsters[CurrentID].Aura.Time = 0;
    Monsters[CurrentID].HasAura = false;
    Monsters[CurrentID].HasShadowAura = false;
    for (int i = 0; i < AURA_MAX; i++)
    {
        Monsters[CurrentID].Aura.Type[i].Active = false;
        Monsters[CurrentID].Aura.Type[i].Level = 0;
        Monsters[CurrentID].AuraAdd[i] = false;
    }
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        Monsters[CurrentID].DamageTable[i] = 0;
    }
    Monsters[CurrentID].Threat = 0;
    Monsters[CurrentID].Strength = 0;
    Monsters[CurrentID].Defense = 0;
    Monsters[CurrentID].Vitality = 0;
    Monsters[CurrentID].Energy = 0;
    Monsters[CurrentID].Regeneration = 0;
    Monsters[CurrentID].Agility = 0;
    Monsters[CurrentID].Capacity = 0;
    Monsters[CurrentID].Luck = 0;

    return CurrentID;
}

// Monster Init Script
NamedScript DECORATE void MonsterInit(int Flags)
{
    // Get a new ID for the monster
    if (GetMonsterID(0) == 0)
        SetMonsterID(0, NewMonsterID());

    if (GetMonsterID(0) == 0)
    {
        // The monster couldn't be given an ID, so there's nothing we can possibly do here.
        Thing_Remove(0);
        return;
    }

    // Monster Stats Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

    // If initialization has already occurred on this monster, terminate
    if (Stats->Init) return;

    // Store TID
    if (ActivatorTID() == 0)
    {
        int TID = UniqueTID();
        Thing_ChangeTID(0, TID);
        Stats->TID = TID;
    }
    else
        Stats->TID = ActivatorTID();

    // Store Position
    Stats->spawnPos.X = GetActorX(0);
    Stats->spawnPos.Y = GetActorY(0);
    Stats->spawnPos.Z = GetActorZ(0);
    Stats->spawnPos.Angle = GetActorAngle(0);
    Stats->spawnPos.Pitch = GetActorPitch(0);

    // Set as initialized
    Stats->Init = true;
    SetInventory("DRPGMonsterInit", 1);

    // Store Actor Name
    Stats->Actor = GetActorClass(0);

    // Allow map events to modify our stats/replacements
    Delay(1);

    // Wait for MapInit to finish
    while (!CurrentLevel->Init)
        Delay(1);

    // When MapInit finishes, we may need to wait for replacements
    if (CurrentLevel->Event != MAPEVENT_NONE)
        while (WaitingForReplacements)
            Delay(1);

    if (!Stats->Init || Stats->TID == 0 || ClassifyActor(Stats->TID) == ACTOR_NONE)
    {
        // Some other event trigger removed us, or we removed ourselves
        Stats->Init = false;
        Stats->TID = 0;
        return;
    }

    if (StrLen(Stats->ReplaceActor) > 0)
    {
        int OldTID = ActivatorTID();
        bool Ambush = GetActorProperty(0, APROP_Ambush);
        Thing_Remove(0);
        Stats->Init = false;
        Stats->TID = 0;
        if (StrICmp(Stats->ReplaceActor, "None") != 0)
        {
            int NewTID = UniqueTID();
            if (Spawn(Stats->ReplaceActor, Stats->spawnPos.X, Stats->spawnPos.Y, Stats->spawnPos.Z, NewTID, Stats->spawnPos.Angle * 256))
            {
                SetActorProperty(NewTID, APROP_Ambush, Ambush);
                Thing_ChangeTID(NewTID, OldTID);
            }
        }
        return;
    }

    // Megabossifyer! For compatible monster mods.
    // This has been placed after Replacement so it will trigger only for the chosen megaboss actor.
    if (CompatMonMode != COMPAT_NONE && CurrentLevel->Event == MAPEVENT_MEGABOSS)
    {
        Flags = MF_MEGABOSS | MF_NOAURA;
    }

    // Store Monster Flags
    Stats->Flags = Flags;

    // Calculate Total Bosses
    if (Stats->Flags & MF_BOSS)
        TotalBosses++;

    // Set the Height and Radius
    Stats->Height = GetActorPropertyFixed(0, APROP_Height);
    Stats->Radius = GetActorPropertyFixed(0, APROP_Radius);

    // Apply base Spawn Health to HealthMax property
    Stats->SpawnHealth = GetActorProperty(0, APROP_SpawnHealth);
    Stats->HealthMax = Stats->SpawnHealth;

    // Set monster caps
    MonsterLevelCap = GetCVar("drpg_monster_level_cap");
    MonsterStatCap = GetCVar("drpg_monster_stat_cap");

    // Delay if Toaster Mode on
    if (GetCVar("drpg_toaster"))
        while (ActorNotSeePlayers(0, 1024, true)) Delay(20);

    // Give it a Health Bar
    if (!(Flags & MF_NOHEALTHBAR))
        Stats->HealthBar = true;

    // Apply Stats
    if (!(Flags & MF_NOSTATS))
        MonsterInitStats();

    // Save the Render Type
    Stats->RenderStyle = GetActorProperty(0, APROP_RenderStyle);

    // Stat-Change Handling
    MonsterStatsHandler();

    // Regeneration Stat Handling
    if (!(Flags & MF_NOSTATS))
        MonsterRegenerationHandler();

    // Energy Stat Handling
    MonsterAggressionHandler();

    // Friendly Monsters Teleport To The Player Handling
    MonsterFriendlyTeleport(false);

    // Death Handler
    // Handled via ZScript

    // Environmental Hazard Map Event
    if (CurrentLevel->Event == MAPEVENT_TOXICHAZARD && (Random(1, 24) == 1 || Stats->Flags & MF_BOSS || Stats->Flags & MF_MEGABOSS))
    {
        //GiveInventory("DRPGMonsterRadiationProtection", 1);
        GiveInventory("DRPGMonsterRadSuit", 1);
    }
}

// Modify the targeted monster (mainly for debugging)
NamedScript Console void MonsterSet(int Level, int Aura, int Flags, bool Decorate)
{
    // Move script to the targeted monster
    if (!Decorate)
        SetActivatorToTarget(Player.TID);

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

    // Set Stats/Flags
    if (Level > 0)
        Stats->LevelAdd = Level;
    if (Aura > 0)
        Stats->AuraAdd[Aura - 1] = true;
    if (Flags > 0)
        Stats->Flags = Flags;

    // Needs reinitialization to apply new info
    Stats->NeedReinit = true;
}

// Give the targeted monster a Shadow Aura (mainly for debugging)
NamedScript Console void MonsterSetShadow()
{
    // Move script to the targeted monster
    SetActivatorToTarget(Player.TID);

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

    // Apply all Auras
    for (int i = 0; i < AURA_MAX; i++)
        Stats->AuraAdd[i] = true;

    // Needs reinitialization to apply new info
    Stats->NeedReinit = true;
}

// Modify the targeted monsters stats (mainly for debugging)
NamedScript Console void MonsterModStat(int Stat, int Value)
{
    // Move script to the targeted monster
    SetActivatorToTarget(Player.TID);

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

    switch (Stat - 1)
    {
    case STAT_STRENGTH:
        Stats->Strength = Value;
        break;
    case STAT_DEFENSE:
        Stats->Defense = Value;
        break;
    case STAT_VITALITY:
        Stats->Vitality = Value;
        break;
    case STAT_ENERGY:
        Stats->Energy = Value;
        break;
    case STAT_REGENERATION:
        Stats->Regeneration = Value;
        break;
    case STAT_AGILITY:
        Stats->Agility = Value;
        break;
    case STAT_CAPACITY:
        Stats->Capacity = Value;
        break;
    case STAT_LUCK:
        Stats->Luck = Value;
        break;
    }
}

// Dump the targeted monster's stats to the console
NamedScript Console void MonsterDump()
{
    int PlayerNum = PlayerNumber();

    // Move script to the targeted monster
    SetActivatorToTarget(Player.TID);

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

    // If you have no target, terminate
    if (ActivatorTID() == Players(PlayerNum).TID)
    {
        PrintError("You have no target");
        ActivatorSound("menu/error", 127);
        return;
    }

    Log("\Ca===== MONSTER INFORMATION =====");
    Log(" Actor: %S", GetActorClass(0));
    Log(" Replacement Actor: %S", Stats->ReplaceActor);
    Log(" Species: %S", GetActorPropertyString(0, APROP_Species));
    Log(" Position: %.2k, %.2k, %.2k", Stats->spawnPos.X, Stats->spawnPos.Y, Stats->spawnPos.Z);
    Log(" Tag: %S", GetActorPropertyString(0, APROP_NameTag));
    Log(" Height: %.2k", GetActorPropertyFixed(0, APROP_Height));
    Log(" Radius: %.2k", GetActorPropertyFixed(0, APROP_Radius));
    Log(" Speed: %.2k", GetActorPropertyFixed(0, APROP_Speed));
    Log(" Damage Multiplier: %.2k", GetActorPropertyFixed(0, APROP_DamageMultiplier));
    Log(" Damage Factor: %.2k", GetActorPropertyFixed(0, APROP_DamageFactor));
    Log(" Distance: %.2k", Distance(0, Players(PlayerNum).TID));
    Log(" TID: %d", ActivatorTID());
    Log(" ID: %d", GetMonsterID(0));

    if (GetMonsterID(0) != 0)
    {
        Log("\Cf===== MONSTER FLAGS =====");
        Log(" Init: %d", Stats->Init);
        Log(" Named: %d", Stats->Named);
        Log(" HealthBar: %d", Stats->HealthBar);
        Log(" RenderStyle: %d", Stats->RenderStyle);
        Log(" DRPG Flags: %d", Stats->Flags);
        Log(" Reinforcement: %d", Stats->Reinforcement);
        Log(" Assassination Target: %d", Stats->Target);
        Log(" MegaBoss: %d", Stats->MegaBoss);

        Log("\Cd===== MONSTER STATS =====");
        Log(" Level: %d", Stats->Level);
        Log(" Threat: %d", Stats->Threat);
        Log(" Spawn Health: %d", Stats->SpawnHealth);
        Log(" Health Max: %d", Stats->HealthMax);
        Log(" Health Regenerated: %d", Stats->RegenHealth);
        Log(" Base Speed: %d", Stats->Speed);
        Log(" \CgStrength: %d", Stats->Strength);
        Log(" \CdDefense: %d", Stats->Defense);
        Log(" \CaVitality: %d", Stats->Vitality);
        Log(" \CnEnergy: %d", Stats->Energy);
        Log(" \CtRegeneration: %d", Stats->Regeneration);
        Log(" \CiAgility: %d", Stats->Agility);
        Log(" \ChCapacity: %d", Stats->Capacity);
        Log(" \CkLuck: %d", Stats->Luck);

        Log("\Cd===== MONSTER AURAS =====");
        Log(" Time: %S", FormatTime(Stats->Aura.Time));
        for (int i = 0; i < AURA_MAX; i++) // TODO: Make this prettier
            if (Stats->Aura.Type[i].Active)
                Log(" Active: %d", i);
    }
}

// Damage the target
NamedScript Console void MonsterDamage(int Amount, int Type)
{
    SetActivatorToTarget(Player.TID);

    if (ActivatorTID() == Player.TID)
        return;

    DamageThing(Amount, Type);
}

NamedScript void CalculateMonsterStats(MonsterStatsPtr Stats)
{
    int StatEffect[8];
    int MonsterStatPool;

    // Calculate Map Level Modifier
    fixed Modifier = 1.0;

    if (GetCVar("drpg_ws_use_wads") < 4)
    {
        Modifier = 4.0 / GetCVar("drpg_ws_use_wads") * MapLevelModifier;

        if (Modifier < 1.0)
            Modifier = 1.0;
    }

    int LevelType = GetCVar("drpg_monster_levels");
    fixed LevelWeight = GetCVarFixed("drpg_monster_level_weight");
    fixed MapWeight = GetCVarFixed("drpg_monster_map_weight");
    fixed RandomMinWeight = GetCVarFixed("drpg_monster_random_min_mult");
    fixed RandomMaxWeight = GetCVarFixed("drpg_monster_random_max_mult");
    int LevelNum = CurrentLevel->LevelNum * Modifier;

    // Let's not cap Level Number to 100 anymore
    //if (LevelNum > 100)
    //    LevelNum = 100;

    // If the Arena is active, base the Monster Levels Map Number portion on the current wave
    if (CurrentLevel->UACBase && ArenaActive)
        LevelNum = ArenaWave / 3;

    // Calculate Monster Level
    if (LevelType == 1 || LevelType == 3) // Player Level
        Stats->Level += (int)((fixed)AveragePlayerLevel() * LevelWeight);
    if (LevelType == 2 || LevelType == 3) // Map Number
        Stats->Level += (int)((fixed)LevelNum * MapWeight);
    if (CurrentLevel->UACBase || CurrentLevel->UACArena) // Outpost or Arena
        Stats->Level = AveragePlayerLevel();
    if (AveragePlayerLuck() < 0)
        Stats->Level += -AveragePlayerLuck();

    // Randomization Weight
    if (RandomMinWeight > RandomMaxWeight)
        Log("\CgERROR: \C-Monster Random Min Multiplier cannot be above Monster Random Max Multiplier!");
    else
        Stats->Level = (int)(Stats->Level * RandomFixed(RandomMinWeight, RandomMaxWeight));

    // If the monster is friendly, it has the average level of all players in the game
    if (!CurrentLevel->UACBase && GetActorProperty(0, APROP_Friendly))
        Stats->Level = (int)(((fixed)AveragePlayerLevel() * LevelWeight + (fixed)LevelNum * MapWeight) * RandomFixed(0.9, 1.1));

    // Special case for Powersuit Mk. II
    if (GetActorClass(0) == "DRPGSuperPowerSuit")
        Stats->Level = 1000;

    MonsterStatPool = (40 + 5 * Stats->Level) * (GetCVar("drpg_levelup_natural") ? StatsNatMod() : 1.0);

    // Minimal Points
    Stats->Strength = Stats->Level / 2;
    Stats->Defense = Stats->Level / 2;
    Stats->Vitality = Stats->Level / 2;
    Stats->Energy = Stats->Level / 2;
    Stats->Regeneration = Stats->Level / 2;
    Stats->Agility = Stats->Level / 2;
    Stats->Capacity = Stats->Level / 2;
    Stats->Luck = Stats->Level / 2;
    MonsterStatPool -= (Stats->Level / 2) * 8;

    // Calculate the monster's cut and special stats
    if (GetCVar("drpg_monster_specialize"))
    {
        // [KS] Here's a fun and fast little way of doing it:

        // 2 specialized stats
        StatEffect[0] = 2;
        StatEffect[1] = 2;

        // 2 normal stats
        StatEffect[2] = 0;
        StatEffect[3] = 0;

        // 4 cut stats
        StatEffect[4] = 1;
        StatEffect[5] = 1;
        StatEffect[6] = 1;
        StatEffect[7] = 1;

        // Shuffle 'em up!
        for (int i = 0; i < STAT_MAX; i++)
        {
            int SwapWith = Random(0, STAT_MAX - 1);
            int Temp = StatEffect[i];

            StatEffect[i] = StatEffect[SwapWith];
            StatEffect[SwapWith] = Temp;
        }

        // Next, create a weighted list of stats.
        // Specialized stats are preferred over normal stats, and cut stats are avoided.
        int StatSelector[14];
        int j = 0;

        for (int i = 0; i < STAT_MAX; i++)
        {
            if (StatEffect[i] == 2)
            {
                StatSelector[j++] = i;
                StatSelector[j++] = i;
                StatSelector[j++] = i;
            }
            else if (StatEffect[i] == 0)
            {
                StatSelector[j++] = i;
                StatSelector[j++] = i;
            }
            else
                StatSelector[j++] = i;
        }

        // Finally, distribute the points with our weighted list.
        while (MonsterStatPool > 0)
        {
            switch (StatSelector[Random (0, 13)])
            {
            case 0:
                Stats->Strength++;
                break;
            case 1:
                Stats->Defense++;
                break;
            case 2:
                Stats->Vitality++;
                break;
            case 3:
                Stats->Energy++;
                break;
            case 4:
                Stats->Regeneration++;
                break;
            case 5:
                Stats->Agility++;
                break;
            case 6:
                Stats->Capacity++;
                break;
            case 7:
                Stats->Luck++;
                break;
            }
            MonsterStatPool--;
        }
    }
    else
    {
        // Distribute entirely at random
        while (MonsterStatPool > 0)
        {
            switch (Random (0, STAT_MAX - 1))
            {
            case 0:
                Stats->Strength++;
                break;
            case 1:
                Stats->Defense++;
                break;
            case 2:
                Stats->Vitality++;
                break;
            case 3:
                Stats->Energy++;
                break;
            case 4:
                Stats->Regeneration++;
                break;
            case 5:
                Stats->Agility++;
                break;
            case 6:
                Stats->Capacity++;
                break;
            case 7:
                Stats->Luck++;
                break;
            }
            MonsterStatPool--;
        }
    }

    // Map Event - RAINBOWS!
    if (CurrentLevel->Event == MAPEVENT_BONUS_RAINBOWS)
        Stats->Capacity *= 2;
}

// Apply stats to monsters
OptionalArgs(1) NamedScript void MonsterInitStats(int StatFlags)
{
    // Move Activator to whoever the player is targeting
    if (StatFlags & SF_PLAYERTARGET)
    {
        SetActivatorToTarget(Player.TID);
        StatFlags |= SF_RECREATE;
    }

    if (ClassifyActor(0) & ACTOR_WORLD)
        return; // The Monster Reducer can remove a monster before it attains stats, so if that happens, don't bother.

    if (GetMonsterID(0) < 1 || GetMonsterID(0) >= MAX_MONSTERS)
    {
        Thing_Remove(0);
        return;
    }

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

    int StatEffect[8];
    int MonsterStatPool;

    if (!(StatFlags & SF_RECREATE))
    {
        CalculateMonsterStats(Stats);

        // Apply Aura
        if (!GetActorProperty(0, APROP_Friendly) && !(Stats->Flags & MF_NOAURA) && !(Stats->Flags & MF_NOAURAGEN))
        {
            // Check to see if any players have a Kill Auras mission
            int AuraMissionAdd = 0;
            if (CurrentLevel && !CurrentLevel->UACBase && !CurrentLevel->UACArena)
                for (int i = 0; i < MAX_PLAYERS; i++)
                    if (Players(i).Mission.Type == MT_KILLAURAS)
                        AuraMissionAdd += Players(i).Mission.Amount;

            // Hell skill and above always has an additional +5% chance
            if (GameSkill() >= 5)
                AuraMissionAdd += 5;

            // 1st roll: Chance of having an aura at all
            bool HasAura = true;
            if (CurrentLevel->Event != MAPEVENT_ALLAURAS)
            {
                int AuraChance = 31 + ((GetCVar("drpg_aura_curve") * 100 / 97) + (Stats->Luck * 100 / 97));
                if (AuraChance < 0)    AuraChance = 0;
                if (AuraChance > 1000) AuraChance = 1000;
                AuraChance *= AuraChance;
                AuraChance /= 1000;
                if (AuraMissionAdd > 0)
                    AuraChance += (AuraMissionAdd * 10);
                HasAura = (Random(0, 1000) < AuraChance);
            }

            // 2nd roll: Number of auras to have
            int AuraRand = Random(Stats->Energy / 10, 200);
            int AuraNumber = ((AuraRand * AuraRand) / 4000) * MapLevelModifier;
            if (AuraNumber < 1) AuraNumber = 1;
            if (AuraNumber > AURA_MAX) AuraNumber = AURA_MAX;
            if (!HasAura) AuraNumber = 0;

            // Run through and decide on which Auras it should have
            for (int i = 0; i < AURA_MAX; i++)
            {
                if (AuraNumber == 0)
                    break;

                Stats->AuraAdd[i] = true;
                AuraNumber--;

                int SwapWith = Random(0, AURA_MAX - 1);
                bool Temp = Stats->AuraAdd[i];

                Stats->AuraAdd[i] = Stats->AuraAdd[SwapWith];
                Stats->AuraAdd[SwapWith] = Temp;
            }

            // Are You A Bad Enough Dude?
            if (GetCVar("drpg_monster_shadows"))
                for (int i = 0; i < AURA_MAX; i++)
                    Stats->AuraAdd[i] = true;

            if (CurrentLevel->Event == MAPEVENT_HARMONIZEDAURAS)
            {
                if (CurrentLevel->AuraType == AURA_MAX)
                    for (int j = 0; j < AURA_MAX; j++)
                        Stats->AuraAdd[j] = true;
                else
                    Stats->AuraAdd[CurrentLevel->AuraType] = true;
            }
        }
    }

    if (Stats->AuraAdd[AURA_WHITE] || Stats->Aura.Type[AURA_WHITE].Active)
        Stats->LevelAdd *= 2;

    // Apply the aura effects
    if (Stats->AuraAdd[AURA_WHITE] && !Stats->Aura.Type[AURA_WHITE].Active) // White Aura - XP
    {
        Stats->Aura.Type[AURA_WHITE].Active = true;
        Stats->AuraAdd[AURA_WHITE] = false;
        Stats->LevelAdd += Stats->Level;
        MonsterWhiteAuraCheck(true);
        MonsterLevelupHandler();
    }
    if (Stats->AuraAdd[AURA_RED] && !Stats->Aura.Type[AURA_RED].Active) // Red Aura - Strength
    {
        Stats->Aura.Type[AURA_RED].Active = true;
        Stats->AuraAdd[AURA_RED] = false;
        Stats->Strength *= 2;
        MonsterRedAuraCheck(true);
        MonsterDamageRetaliationHandler();
    }
    if (Stats->AuraAdd[AURA_GREEN] && !Stats->Aura.Type[AURA_GREEN].Active) // Green Aura - Defense
    {
        Stats->Aura.Type[AURA_GREEN].Active = true;
        Stats->AuraAdd[AURA_GREEN] = false;
        Stats->Defense *= 2;
        MonsterGreenAuraCheck(true);
    }
    if (Stats->AuraAdd[AURA_PINK] && !Stats->Aura.Type[AURA_PINK].Active) // Pink Aura - Vitality
    {
        Stats->Aura.Type[AURA_PINK].Active = true;
        Stats->AuraAdd[AURA_PINK] = false;
        Stats->Vitality *= 2;
        MonsterPinkAuraCheck(true);
        MonsterFellowResurrectionHandler();
    }
    if (Stats->AuraAdd[AURA_BLUE] && !Stats->Aura.Type[AURA_BLUE].Active) // Blue Aura - Energy
    {
        Stats->Aura.Type[AURA_BLUE].Active = true;
        Stats->AuraAdd[AURA_BLUE] = false;
        Stats->Energy *= 2;
        MonsterBlueAuraCheck(true);
        MonsterEPDrainHandler();
    }
    if (Stats->AuraAdd[AURA_PURPLE] && !Stats->Aura.Type[AURA_PURPLE].Active) // Purple Aura - Regeneration
    {
        Stats->Aura.Type[AURA_PURPLE].Active = true;
        Stats->AuraAdd[AURA_PURPLE] = false;
        Stats->Regeneration *= 2;
    }
    if (Stats->AuraAdd[AURA_ORANGE] && !Stats->Aura.Type[AURA_ORANGE].Active) // Orange Aura - Agility
    {
        Stats->Aura.Type[AURA_ORANGE].Active = true;
        Stats->AuraAdd[AURA_ORANGE] = false;
        Stats->Agility *= 2;
        MonsterOrangeAuraCheck(true);
        MonsterEpicVisitTimeHandler();
    }
    if (Stats->AuraAdd[AURA_DARKBLUE] && !Stats->Aura.Type[AURA_DARKBLUE].Active) // Dark Blue Aura - Capacity
    {
        Stats->Aura.Type[AURA_DARKBLUE].Active = true;
        Stats->AuraAdd[AURA_DARKBLUE] = false;
        Stats->Capacity *= 2;
        MonsterAmmoDrainHandler();
    }
    if (Stats->AuraAdd[AURA_YELLOW] && !Stats->Aura.Type[AURA_YELLOW].Active) // Yellow Aura - Luck
    {
        Stats->Aura.Type[AURA_YELLOW].Active = true;
        Stats->AuraAdd[AURA_YELLOW] = false;
        Stats->Luck *= 2;
        MonsterMoneyDrainHandler();
    }

    Stats->HasAura = MonsterHasAura(Stats);
    Stats->HasShadowAura = MonsterHasShadowAura(Stats);

    // Calculate Aura time
    Stats->Aura.Time = (30 * 35) + (int)((fixed)Stats->Energy * 0.57) * 35;

    // Apply extra levels from white aura and any other sources
    if (Stats->LevelAdd > 0)
    {
        if (Stats->Level + Stats->LevelAdd >= MonsterLevelCap)
            Stats->LevelAdd = MonsterLevelCap - Stats->Level;
        MonsterStatPool = 5 * Stats->LevelAdd;

        int StrengthAdd = 0;
        int DefenseAdd = 0;
        int VitalityAdd = 0;
        int EnergyAdd = 0;
        int RegenerationAdd = 0;
        int AgilityAdd = 0;
        int CapacityAdd = 0;
        int LuckAdd = 0;

        // Distribute those new stats now.
        while (MonsterStatPool > 0)
        {
            switch (Random(0, STAT_MAX - 1))
            {
            case 0:
                StrengthAdd++;
                break;
            case 1:
                DefenseAdd++;
                break;
            case 2:
                VitalityAdd++;
                break;
            case 3:
                EnergyAdd++;
                break;
            case 4:
                RegenerationAdd++;
                break;
            case 5:
                AgilityAdd++;
                break;
            case 6:
                CapacityAdd++;
                break;
            case 7:
                LuckAdd++;
                break;
            }
            MonsterStatPool--;
        }

        if (Stats->Aura.Type[AURA_RED].Active) StrengthAdd *= 2;
        if (Stats->Aura.Type[AURA_GREEN].Active) DefenseAdd *= 2;
        if (Stats->Aura.Type[AURA_PINK].Active) VitalityAdd *= 2;
        if (Stats->Aura.Type[AURA_BLUE].Active) EnergyAdd *= 2;
        if (Stats->Aura.Type[AURA_PURPLE].Active) RegenerationAdd *= 2;
        if (Stats->Aura.Type[AURA_ORANGE].Active) AgilityAdd *= 2;
        if (Stats->Aura.Type[AURA_DARKBLUE].Active) CapacityAdd *= 2;
        if (Stats->Aura.Type[AURA_YELLOW].Active) LuckAdd *= 2;

        Stats->Strength += StrengthAdd;
        Stats->Defense += DefenseAdd;
        Stats->Vitality += VitalityAdd;
        Stats->Energy += EnergyAdd;
        Stats->Regeneration += RegenerationAdd;
        Stats->Agility += AgilityAdd;
        Stats->Capacity += CapacityAdd;
        Stats->Luck += LuckAdd;

        Stats->Level += Stats->LevelAdd;
        Stats->LevelAdd = 0;
    }

    // Cap Level and Stats
    CapMonsterStats(Stats);

    // We need to make sure we only pass the speed to the monster if it's being initially created
    Stats->Speed = GetActorPropertyFixed(0, APROP_Speed);

    // [KS] We only need to do a health check here since the other checks are handled below.
    // Calculate Health
    Stats->HealthMax = CalculateMonsterMaxHealth(Stats);
    SetActorProperty(0, APROP_Health, Stats->HealthMax);

    // Generate Name
    if (!Stats->Named && (Stats->Flags & MF_MEGABOSS || Stats->Flags & MF_NAMEGEN || Stats->HasShadowAura))
    {
        SetActorPropertyString(0, APROP_NameTag, GenerateName(GetActorClass(0), GetActorPropertyString(0, APROP_NameTag)));
        Stats->Named = true;
    }

    // Reinitialize the stats handler if recreation has occurred
    if (StatFlags & SF_RECREATE)
    {
        Stats->NeedReinit = false;
        Delay(1);
        MonsterStatsHandler();
    }
}

NamedScript bool MonsterHasTarget()
{
    return SetActivator(0, AAPTR_TARGET);
}

NamedScript void MonsterAggressionHandler()
{

    if (CheckInventory("DRPGMonsterAggressionHandler"))
        return;

    GiveInventory("DRPGMonsterAggressionHandler", 1);

    // Delay Stagger
    Delay(35 + (GetMonsterID(0) % 4));

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

    // Get Monster TID
    int MonsterTID = Stats->TID;

    // Init Toaster Mode
    bool ToasterMod = GetCVar("drpg_toaster");

    // Calculate delay time
    int DelayTime = 10;
    if (ToasterMod)
        DelayTime = 35;

    int Capacity = Stats->Capacity;
    int Aggression = 25 + Capacity * 2;

Start:

    if (!CheckInventory("DRPGMonsterAggressionHandler"))
        return;

    if (GetActorProperty(0, APROP_Health) <= 0)
    {
        TakeInventory("DRPGMonsterAggressionHandler", 1);
        return;
    }

    if (ClassifyActor(0) & ACTOR_WORLD)
        return;

    // Delay if Toaster Mode on
    if (ToasterMod)
        while (ActorNotSeePlayers(0, 0, true)) Delay(35);

    // Changing the AI of monsters in case if there are summoned monsters or turrets
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!PlayerInGame(i)) continue;

        // Changing the AI of monsters in case if there are summoned monsters
        if (Players(i).Summons > 0)
        {
            for (int j = 0; j < Players(i).Summons; j++)
            {
                if (!GetActorProperty(0, APROP_Friendly))
                {
                    // If the enemy has no target and the summoned monster is in his field of vision he immediately becomes aggressive towards summoned monster
                    if (!MonsterHasTarget() && CheckSight(0, Players(i).SummonTID[j], 0) && Distance(0, Players(i).SummonTID[j]) <= 1536 && Distance(0, Players(i).SummonTID[j]) < Distance(0, Players(i).TID))
                    {
                        Thing_Hate (0, Players(i).SummonTID[j], 3);
                        Delay(DelayTime);
                    }

                    // Clearing enemy target in case a summoned monster is nearby and the player is out of sight (or vice versa)
                    if (MonsterHasTarget() && !MonsterSeeTarget(MonsterTID) && Random(0, 100) <= 15)
                    {
                        if (CheckSight(0, Players(i).SummonTID[j], 0))
                        {
                            GiveInventory("DRPGEnemyClearTarget1", 1);
                            Delay(DelayTime);
                        }
                        else
                        {
                            GiveInventory("DRPGEnemyClearTarget2", 1);
                            Delay(DelayTime);
                        }
                    }

                    // Switch to another target if it is closer than the current target
                    if (MonsterHasTarget() && MonsterSeeTarget(MonsterTID) && MonsterDistanceTarget(MonsterTID) > Random(256, 512) && Random(0, 100) <= 15)
                    {
                        if (CheckSight(0, Players(i).SummonTID[j], 0) && Distance(0, Players(i).SummonTID[j]) < Distance(0, Players(i).TID))
                        {
                            if (Distance(0, Players(i).SummonTID[j]) < MonsterDistanceTarget(MonsterTID))
                            {
                                Thing_Hate (0, Players(i).SummonTID[j], 4);
                                Delay(DelayTime);
                            }
                        }
                        else if (CheckSight(0, Players(i).TID, 0) && Distance(0, Players(i).TID) < Distance(0, Players(i).SummonTID[j]))
                        {
                            if (Distance(0, Players(i).TID) < MonsterDistanceTarget(MonsterTID))
                            {
                                Thing_Hate (0, Players(i).TID, 4);
                                Delay(DelayTime);
                            }
                        }
                    }

                    // Summons switch to another target if it is closer than the current target
                    if (MonsterHasTarget() && CheckSight(Players(i).SummonTID[j], MonsterTID, 0) && Distance(Players(i).SummonTID[j], MonsterTID) <= 256 && Random(0, 100) <= 15)
                    {
                        if (MonsterSeeTarget(Players(i).SummonTID[j]) && MonsterDistanceTarget(Players(i).SummonTID[j]) > Random(256, 512))
                        {
                            if (Distance(Players(i).SummonTID[j], MonsterTID) < MonsterDistanceTarget(Players(i).SummonTID[j]))
                            {
                                Thing_Hate (Players(i).SummonTID[j], MonsterTID);
                                Delay(DelayTime);
                            }
                        }
                        else if (!MonsterSeeTarget(Players(i).SummonTID[j]))
                        {
                            Thing_Hate (Players(i).SummonTID[j], MonsterTID);
                            Delay(DelayTime);
                        }
                    }
                }
            }
        }

        // Changing the AI of enemy monsters in case if there are turret
        if (!GetActorProperty(0, APROP_Friendly) && Players(i).Turret.Active)
        {
            // If the enemy has no target and the turret is in his field of vision he immediately becomes aggressive towards turret
            if (!MonsterHasTarget() && Distance(0, Players(i).Turret.TID) <= 1536 && CheckSight(0, Players(i).Turret.TID, 0) && Distance(0, Players(i).Turret.TID) < Distance(0, Players(i).TID))
            {
                Thing_Hate (0, Players(i).Turret.TID, 3);
                Delay(DelayTime);
            }

            // Clearing enemy target in case a turret is nearby and the player is out of sight (or vice versa)
            if (MonsterHasTarget() && !MonsterSeeTarget(MonsterTID) && Random(0, 100) <= 15)
            {
                if (CheckSight(0, Players(i).Turret.TID, 0))
                {
                    GiveInventory("DRPGEnemyClearTarget1", 1);
                    Delay(DelayTime);
                }
                else
                {
                    GiveInventory("DRPGEnemyClearTarget2", 1);
                    Delay(DelayTime);
                }
            }

            // Switch to another target if it is closer than the current target
            if (MonsterHasTarget() && MonsterSeeTarget(MonsterTID) && MonsterDistanceTarget(MonsterTID) > Random(256, 512) && Random(0, 100) <= 15)
            {
                if (CheckSight(0, Players(i).Turret.TID, 0) && Distance(0, Players(i).Turret.TID) < Distance(0, Players(i).TID))
                {
                    if (Distance(0, Players(i).Turret.TID) < MonsterDistanceTarget(MonsterTID))
                    {
                        Thing_Hate (0, Players(i).Turret.TID, 4);
                        Delay(DelayTime);
                    }
                }
                else if (CheckSight(0, Players(i).TID, 0) && Distance(0, Players(i).TID) < Distance(0, Players(i).Turret.TID))
                {
                    if (Distance(0, Players(i).TID) < MonsterDistanceTarget(MonsterTID))
                    {
                        Thing_Hate (0, Players(i).TID, 4);
                        Delay(DelayTime);
                    }
                }
            }
        }

        // Changing the AI for friendly monsters
        if (GetActorProperty(0, APROP_Friendly))
        {
            // Clearing a friendly monster's target in case the enemy is out of sight
            if (MonsterHasTarget() && !MonsterSeeTarget(MonsterTID) && Random(0, 100) <= 15)
            {
                GiveInventory("DRPGFriendlyClearTarget", 1);
                Delay(DelayTime);
            }

            // More enemies aggression to summoned monsters
            if (MonsterHasTarget() && MonsterSeeTarget(MonsterTID) && Random(0, 100) <= 15)
            {
                GiveInventory("DRPGFriendlyAlertMonsters", 1);
                Delay(DelayTime);
            }

            // Used to change the AI of friendly marines
            if (CheckInventory("DRPGMarineSummonedToken"))
            {
                if (!MonsterHasTarget() && Distance(0, Players(i).TID) <= 200 && Random(0, 100) <= 25)
                {
                    Thing_Hate(0, Players(i).TID);
                    Delay(DelayTime);
                }
            }
        }
    }

    if (!MonsterHasTarget())
    {
        Delay(DelayTime);
        goto Start;
    }

    if (Aggression < 100)
    {
        if (Random(0, 100) > Aggression)
        {
            SetActorFlag(0, "JUSTHIT", false);
            SetActorFlag(0, "JUSTATTACKED", true);
        }
    }
    else
    {
        if (Random(0, 100) < (Aggression - 100) / 4)
        {
            SetActorFlag(0, "JUSTHIT", true);
            SetActorFlag(0, "JUSTATTACKED", false);
        }
    }

    Delay(DelayTime);
    goto Start;
}

NamedScript void MonsterFriendlyTeleport(bool SpecialMonster)
{
    if (!GetActorProperty(0, APROP_Friendly) || CurrentLevel->UACBase)
        return;

    // Check class
    if (GetActorClass(0) == "DRPGForceWall" || GetActorClass(0) == "DRPGPortableTurret")
        return;
    if (GetActorClass(0) == "RLDRPGSummonedLostSoulPE" || GetActorClass(0) == "RLDRPGSummonedLostSoulNPE" || GetActorClass(0) == "RLDRPGSummonedNightmareLostSoulNPE")
        SpecialMonster = true;

    // Delay Stagger
    Delay(35 + (GetMonsterID(0) % 4));

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

    // Init Toaster Mode
    bool ToasterMod = GetCVar("drpg_toaster");

    // Calculate delay time
    int DelayTime = 35;
    if (ToasterMod)
        DelayTime = 50;

    int PlayerTID;
    int PlayerNumber;
    int MonsterTID = Stats->TID;
    int MasterTID = GetActorProperty(0, APROP_MasterTID);
    int TeleportDistance;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!PlayerInGame(i)) continue;

        // Checking if a player is monster's master
        if (SpecialMonster && MasterTID == Players(i).TID)
        {
            PlayerTID = Players(i).TID;
            PlayerNumber = i;
            break;
        }

        if (Players(i).Summons == 0) continue;

        // Checking if a monster is summoned by a player
        for (int j = 0; j < Players(i).Summons; j++)
        {
            if (MonsterTID == Players(i).SummonTID[j])
            {
                PlayerTID = Players(i).TID;
                PlayerNumber = i;
                break;
            }
        }
    }

Start:

    if (!GetUserCVar(PlayerNumber, "drpg_monster_friendly_teleport_enable"))
    {
        Delay(35);
        goto Start;
    }

    if (ClassifyActor(0) & ACTOR_WORLD || ClassifyActor(0) & ACTOR_DEAD) return;

    while (MonsterHasTarget() || ActorSeePlayers(0, 0)) Delay(35);

    TeleportDistance = 512 + (512 * GetUserCVar(PlayerNumber, "drpg_monster_friendly_teleport_distance"));

    if (Distance(0, PlayerTID) > TeleportDistance)
    {
        int TeleportSpotTID = UniqueTID();

        bool Success = false;
        fixed TeleportAngle;
        fixed TeleportDistance;
        fixed X, Y, Z, Angle;

        for (int i = 0; i < 10; i++)
        {
            fixed TeleportAngle = RandomFixed(0.0, 1.0);

            X = GetActorX(PlayerTID) + Random(-256, 256);
            Y = GetActorY(PlayerTID) + Random(-256, 256);
            Z = GetActorZ(PlayerTID);
            Angle = TeleportAngle + 0.5;
            Angle %= 1.0;

            Success = Spawn("MapSpot", X, Y, Z, TeleportSpotTID, Angle);
            if (Success && !SetActorPosition(TeleportSpotTID, GetActorX(TeleportSpotTID), GetActorY(TeleportSpotTID), GetActorFloorZ(TeleportSpotTID), false)) Success = false;
            if (Success && !CheckSight(PlayerTID, TeleportSpotTID, 0)) Success = false;
            if (Success)
                break;
        }

        if (Success)
        {
            // Surprise!!
            SpawnSpotFacingForced("TeleportFog", 0, 0);
            SetActorPosition(0, GetActorX(TeleportSpotTID), GetActorY(TeleportSpotTID), GetActorZ(TeleportSpotTID), false);
            SetActorAngle(0, Angle);
            SpawnSpotFacingForced("TeleportFog", 0, 0);
            Thing_Remove(TeleportSpotTID);
        }
    }

    Delay(DelayTime);
    goto Start;
}

NamedScript void MonsterStatsHandler()
{
    if (CheckInventory("DRPGMonsterStatsHandler"))
        return;

    GiveInventory("DRPGMonsterStatsHandler", 1);

    // Delay Stagger
    Delay(35 + (GetMonsterID(0) % 4));

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

    // Init Toaster Mode
    bool ToasterMod = GetCVar("drpg_toaster");

    // Calculate delay time
    int DelayTime = 4;
    if (ToasterMod)
        DelayTime = 15;

    // Set standart variables
    int OldStrength;
    int OldDefense;
    int OldCapacity;
    int OldVitality;
    int OldAgility;
    int OldRegeneration;
    int OldEnergy;
    int OldLuck;
    int StolenCredits;
    int HealthPercentage;
    int LevelNum = CurrentLevel->LevelNum;
    int PopoffsDrawDist = GetCVar("drpg_popoffs_drawdistance");
    bool StatsChanged;
    bool HasAuraDisplay = false;
    bool MonsterWasDisrupted = false;
    bool HasDamageNumbers = false;
    bool Friendly = GetActorProperty(0, APROP_Friendly); // Sanity check for when APROP_Friendly gets removed from summons

    // Set variables for summons (for AUG)
    int MonsterTID = Stats->TID;
    int MasterTID = GetActorProperty(0, APROP_MasterTID);
    int PlayerTID;
    int PlayerNumber;
    int OldAugStatus;
    int OldStrengthAug = Stats->Strength;
    int OldDefenseAug = Stats->Defense;
    int OldCapacityAug = Stats->Capacity;
    int OldVitalityAug = Stats->Vitality;
    int OldAgilityAug = Stats->Agility;
    int OldRegenerationAug = Stats->Regeneration;
    int OldEnergyAug = Stats->Energy;
    int OldLuckAug = Stats->Luck;

Start:

    if (!CheckInventory("DRPGMonsterStatsHandler"))
        return;

    if (GetActorProperty(0, APROP_Health) <= 0)
    {
        if (OldStrength)
        {
            OldStrength = 0;
            OldDefense = 0;
            OldCapacity = 0;
            OldVitality = 0;
            OldAgility = 0;
            OldRegeneration = 0;
            OldEnergy = 0;
            OldLuck = 0;
        }
        TakeInventory("DRPGMonsterStatsHandler", 1);
        return;
    }

    if (ClassifyActor(0) & ACTOR_WORLD)
        return;

    if (Stats->NeedReinit)
    {
        MonsterInitStats(SF_RECREATE);
        TakeInventory("DRPGMonsterStatsHandler", 1);
        return;
    }

    // Monster is no longer friendly, remove their summon bonuses and species
    // This causes issues with infighting and I don't remember why I did this in the first place
    // Just comment it out and leave it alone for now
    // [SW, 2/20/2019] Need this to prevent our summons from being unkillable if they lose APROP_Friendly
    if (Friendly && !GetActorProperty(0, APROP_Friendly))
    {
        GiveInventory("DRPGFriendlyRemover", 1);
        SetActorPropertyString(0, APROP_Species, "None");
        Friendly = false;
    }

    // Levelup handling via White Aura radius
    if (CheckInventory("DRPGMonsterLevelup"))
        MonsterLevelup(Stats);

    // Cap Level and Stats
    CapMonsterStats(Stats);

    // Strength
    if (Stats->Strength != OldStrength)
    {
        StatsChanged = true;

        if (GetActorProperty(0, APROP_Friendly) || Stats->Flags & MF_BOSS || Stats->Flags & MF_MEGABOSS)
            SetActorPropertyFixed(0, APROP_DamageMultiplier, 1.0 + (((fixed)(Stats->Strength * (fixed)GameSkill()) / 400.0)));
        else
            SetActorPropertyFixed(0, APROP_DamageMultiplier, 1.0 + (((fixed)(Stats->Strength * (fixed)GameSkill()) / 400.0) + ((fixed)LevelNum / (GetCVar("drpg_ws_use_wads") * 25.0))));

        OldStrength = Stats->Strength;
    }

    // Defense
    if (Stats->Defense != OldDefense)
    {
        StatsChanged = true;
        fixed DamageFactor;

        if (GetActorProperty(0, APROP_Friendly))
            DamageFactor = 1.0 - ((fixed)Stats->Defense / 400.0);
        else
            DamageFactor = 1.0 - (((fixed)Stats->Defense / 400.0) + ((fixed)LevelNum / (GetCVar("drpg_ws_use_wads") * 100.0)));

        if (DamageFactor < 0.251)
            DamageFactor = 0.251;

        SetActorPropertyFixed(0, APROP_DamageFactor, DamageFactor);

        OldDefense = Stats->Defense;
    }

    // Capacity
    if (Stats->Capacity != OldCapacity)
    {
        StatsChanged = true;

        StolenCredits = 0;
        if (CheckInventory("DRPGCredits") > OldCapacity)
            StolenCredits = CheckInventory("DRPGCredits") - OldCapacity;

        // Calculation of credits depending of WADs that you plan to go through
        CalculateMonsterCredits(Stats, StolenCredits);

        OldCapacity = Stats->Capacity;
    }

    // Agility
    if (Stats->Agility != OldAgility)
    {
        StatsChanged = true;

        SetActorPropertyFixed(0, APROP_Speed, CalculateMonsterSpeed(Stats));
        OldAgility = Stats->Agility;
    }

    // Vitality
    if (Stats->Vitality != OldVitality)
    {
        StatsChanged = true;

        HealthPercentage = 100;
        if (Stats->HealthMax > 0)
            HealthPercentage = (long long)GetActorProperty(0, APROP_Health) * 100 / Stats->HealthMax;
        Stats->HealthMax = CalculateMonsterMaxHealth(Stats);

        if (OldVitality == 0)
        {
            // This will trigger if the monster was previously dead and revived, so heal up fully
            SetActorProperty(0, APROP_Health, Stats->HealthMax);
        }
        else
        {
            SetActorProperty(0, APROP_Health, (long long)Stats->HealthMax * HealthPercentage / 100);
        }

        OldVitality = Stats->Vitality;
    }

    // Nothing much to do for these

    // Regeneration
    if (Stats->Regeneration != OldRegeneration)
    {
        StatsChanged = true;
        OldRegeneration = Stats->Regeneration;
    }

    // Energy
    if (Stats->Energy != OldEnergy)
    {
        StatsChanged = true;
        OldEnergy = Stats->Energy;
    }

    // Luck
    if (Stats->Luck != OldLuck)
    {
        StatsChanged = true;
        OldLuck = Stats->Luck;
    }

    if (StatsChanged)
    {
        // Re-calculate the monster's threat level
        Stats->Threat = CalculateMonsterThreatLevel(Stats);

        StatsChanged = false;
    }

    // Disruption timer
    if (CheckInventory("DRPGMonsterDisrupted"))
    {
        if (!MonsterWasDisrupted)
        {
            if (Stats->Aura.Type[AURA_WHITE].Active)
                MonsterWhiteAuraCheck(false);
            if (Stats->Aura.Type[AURA_RED].Active)
                MonsterRedAuraCheck(false);
            if (Stats->Aura.Type[AURA_GREEN].Active)
                MonsterGreenAuraCheck(false);
            if (Stats->Aura.Type[AURA_PINK].Active)
                MonsterPinkAuraCheck(false);
            if (Stats->Aura.Type[AURA_BLUE].Active)
                MonsterBlueAuraCheck(false);
            if (Stats->Aura.Type[AURA_ORANGE].Active)
                MonsterOrangeAuraCheck(false);

            ActivatorSound("skills/disruption", 127);
            MonsterWasDisrupted = true;
        }

        TakeInventory("DRPGMonsterDisrupted", Stats->HasShadowAura ? 12 : 4);
    }
    else
    {
        if (MonsterWasDisrupted)
        {
            if (Stats->Aura.Type[AURA_WHITE].Active)
                MonsterWhiteAuraCheck(true);
            if (Stats->Aura.Type[AURA_RED].Active)
                MonsterRedAuraCheck(true);
            if (Stats->Aura.Type[AURA_GREEN].Active)
                MonsterGreenAuraCheck(true);
            if (Stats->Aura.Type[AURA_PINK].Active)
                MonsterPinkAuraCheck(true);
            if (Stats->Aura.Type[AURA_BLUE].Active)
                MonsterBlueAuraCheck(true);
            if (Stats->Aura.Type[AURA_ORANGE].Active)
                MonsterOrangeAuraCheck(true);

            MonsterWasDisrupted = false;
        }
    }

    if (Friendly && !CurrentLevel->UACBase)
    {
        // Find master player
        if (PlayerTID == 0)
        {
            for (int i = 0; i < MAX_PLAYERS; i++)
            {
                if (!PlayerInGame(i)) continue;

                // Checking if a player is monster's master
                if (MasterTID == Players(i).TID)
                {
                    PlayerTID = Players(i).TID;
                    PlayerNumber = i;
                    break;
                }

                if (Players(i).Summons == 0) continue;

                // Checking if a monster is summoned by a player
                for (int j = 0; j < Players(i).Summons; j++)
                {
                    if (MonsterTID == Players(i).SummonTID[j])
                    {
                        PlayerTID = Players(i).TID;
                        PlayerNumber = i;
                        break;
                    }
                }
            }
        }

        // Check AUG Summoner status for master player
        if (Players(PlayerNumber).Augs.Active[AUG_SUMMONER] * Players(PlayerNumber).Augs.CurrentLevel[AUG_SUMMONER] != OldAugStatus)
        {
            if (Players(PlayerNumber).Augs.Active[AUG_SUMMONER])
            {
                fixed BaseStatePoint = (40.0 + 5.0 * (fixed)Stats->Level) / 8.0;
                fixed AugSummonerModifier;
                int AddStrength;
                int AddDefense;
                int AddCapacity;
                int AddVitality;
                int AddAgility;
                int AddRegeneration;
                int AddEnergy;
                int AddLuck;

                // Calculate add stats
                if (Players(PlayerNumber).Augs.CurrentLevel[AUG_SUMMONER] >= 1)
                    AddVitality = 20 + RoundInt(BaseStatePoint * ((fixed)Players(PlayerNumber).EnergyTotal * (0.5 - (fixed)Players(PlayerNumber).EnergyTotal * 0.0025)) / 100.0);
                if (Players(PlayerNumber).Augs.CurrentLevel[AUG_SUMMONER] >= 2)
                    AddDefense = 20 + RoundInt(BaseStatePoint * ((fixed)Players(PlayerNumber).EnergyTotal * (0.5 - (fixed)Players(PlayerNumber).EnergyTotal * 0.0025)) / 100.0);
                if (Players(PlayerNumber).Augs.CurrentLevel[AUG_SUMMONER] >= 3)
                    AddStrength = 20 + RoundInt(BaseStatePoint * ((fixed)Players(PlayerNumber).EnergyTotal * (0.5 - (fixed)Players(PlayerNumber).EnergyTotal * 0.0025)) / 100.0);
                if (Players(PlayerNumber).Augs.CurrentLevel[AUG_SUMMONER] >= 4)
                    SetActorInventory(MonsterTID, "DRPGSummonedRegenerationBoosterToken", 1);

                // Calculate add level
                if (Players(PlayerNumber).Augs.CurrentLevel[AUG_SUMMONER] == 5)
                    AugSummonerModifier = 0.10;
                if (Players(PlayerNumber).Augs.CurrentLevel[AUG_SUMMONER] == 6)
                    AugSummonerModifier = 0.15;
                if (Players(PlayerNumber).Augs.CurrentLevel[AUG_SUMMONER] == 7)
                    AugSummonerModifier = 0.20;
                if (Players(PlayerNumber).Augs.CurrentLevel[AUG_SUMMONER] >= 8)
                    AugSummonerModifier = 0.25;
                if (AugSummonerModifier > 0)
                {
                    AddStrength += Stats->Strength * AugSummonerModifier;
                    AddDefense += Stats->Defense * AugSummonerModifier;
                    AddCapacity += Stats->Capacity * AugSummonerModifier;
                    AddVitality += Stats->Vitality * AugSummonerModifier;
                    AddAgility += Stats->Agility * AugSummonerModifier;
                    AddRegeneration += Stats->Regeneration * AugSummonerModifier;
                    AddEnergy += Stats->Energy * AugSummonerModifier;
                    AddLuck += Stats->Luck * AugSummonerModifier;
                }

                // Set actual stats
                Stats->Strength = OldStrengthAug + AddStrength;
                Stats->Defense = OldDefenseAug + AddDefense;
                Stats->Capacity = OldCapacityAug + AddCapacity;
                Stats->Vitality = OldVitalityAug + AddVitality;
                Stats->Agility = OldAgilityAug + AddAgility;
                Stats->Regeneration = OldRegenerationAug + AddRegeneration;
                Stats->Energy = OldEnergyAug + AddEnergy;
                Stats->Luck = OldLuckAug + AddLuck;
            }
            else if (!Players(PlayerNumber).Augs.Active[AUG_SUMMONER])
            {
                // Set actual stats if AUG is off
                Stats->Strength = OldStrengthAug;
                Stats->Defense = OldDefenseAug;
                Stats->Capacity = OldCapacityAug;
                Stats->Vitality = OldVitalityAug;
                Stats->Agility = OldAgilityAug;
                Stats->Regeneration = OldRegenerationAug;
                Stats->Energy = OldEnergyAug;
                Stats->Luck = OldLuckAug;
                SetActorInventory(MonsterTID, "DRPGSummonedRegenerationBoosterToken", 0);
            }

            // Set actual health
            int Health = GetActorProperty(0, APROP_Health);
            if (Health > Stats->HealthMax)
                SetActorProperty(0, APROP_Health, Stats->HealthMax);
            else
                SetActorProperty(0, APROP_Health, Health);

            // Re-calculate threat level
            Stats->Threat = CalculateMonsterThreatLevel(&Monsters[GetMonsterID(MonsterTID)]);

            OldAugStatus = Players(PlayerNumber).Augs.Active[AUG_SUMMONER] * Players(PlayerNumber).Augs.CurrentLevel[AUG_SUMMONER];
        }
    }

    // Aura Spawner
    if (!HasAuraDisplay && (Stats->HasAura || Stats->Target > 0 || (GetActorProperty(0, APROP_Friendly) && !CurrentLevel->UACBase)))
    {
        MonsterAuraDisplayHandler();
        HasAuraDisplay = true;
    }

    // Damage Numbers
    if (!HasDamageNumbers && !ActorNotSeePlayers(0, PopoffsDrawDist, true))
    {
        DamageNumbers();
        HasDamageNumbers = true;
    }

    // Delay if Toaster Mode on
    if (ToasterMod)
        while (ActorNotSeePlayers(0, 0, true)) Delay(35);

    Delay(DelayTime);
    goto Start;
}

NamedScript void MonsterAuraDisplayHandler()
{
    // Delay Stagger
    Delay(35 + (GetMonsterID(0) % 4));

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

    int DrawDist = GetCVar("drpg_auras_drawdistance");

Start:

    if (GetActorProperty(0, APROP_Health) <= 0)
        return;

    if (ClassifyActor(0) & ACTOR_WORLD)
        return;

    // Distance and sight checks
    while (ActorNotSeePlayers(0, DrawDist, false)) Delay(35);

    // Spawn Auras
    if (Stats->HasAura || (GetActorProperty(0, APROP_Friendly) && !CurrentLevel->UACBase))
        SpawnAuras(0, false);

    // Shadow Aura Effects
    if (Stats->HasShadowAura)
        SetActorProperty(0, APROP_RenderStyle, STYLE_Subtract);

    // Spawn Assassination Target
    if (Stats->Target > 0)
        SpawnForced("DRPGAssassinationIcon", GetActorX(0), GetActorY(0), GetActorZ(0) + GetActorPropertyFixed(0, APROP_Height) + 16.0 + (Sin(Timer() / 64.0) * 8.0), 0, 0);

    // Spawn Disruption Icon
    if (Stats->HasAura && CheckInventory("DRPGMonsterDisrupted"))
        SpawnForced("DRPGDisruptionIcon", GetActorX(0), GetActorY(0), GetActorZ(0) + GetActorPropertyFixed(0, APROP_Height) + 16.0 + (Sin(Timer() / 64.0) * 8.0), 0, 0);

    Delay(1);
    goto Start;
}

NamedScript void MonsterLevelupHandler()
{
    if (CheckInventory("DRPGMonsterLevelupHandler"))
        return;

    GiveInventory("DRPGMonsterLevelupHandler", 1);

    // Delay Stagger
    Delay(35 + (GetMonsterID(0) % 4));

    int Energy;
    int DelayTime;

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

Start:

    // Calculate the Delay Time
    Energy = Stats->Energy;
    DelayTime = 35 * (int)((60.0 - ((fixed)Clamp(0, Energy, 1000) / 16.75)));
    if (DelayTime < 35)
        DelayTime = 35;
    Delay(DelayTime);

    if (ClassifyActor(0) & ACTOR_WORLD)
        return;

    if ((ClassifyActor(0) & ACTOR_DEAD) || !GetCVar("drpg_monster_white_masslevel"))
    {
        Delay(35);
        goto Start;
    }

    if (!CheckInventory("DRPGMonsterLevelupHandler"))
        return;

    // Loop back to the top if the monster is max level
    if (Stats->Level >= MonsterLevelCap)
        goto Start;

    if (!Stats->Aura.Type[AURA_WHITE].Active || CheckInventory("DRPGMonsterDisrupted"))
        goto Start;

    // White Aura monsters levelup monsters near them
    if (Stats->Aura.Type[AURA_WHITE].Active && !CheckInventory("DRPGMonsterDisrupted"))
        for (int i = 0; i < MAX_PLAYERS; i++) // LOS Check on players
            if (CheckSight(0, Players(i).TID, 0))
            {
                if (!PlayerInGame(i)) continue;

                GiveInventory("DRPGMonsterLevelupGiver", 1);
                break;
            }

    goto Start;
}

NamedScript void MonsterEPDrainHandler()
{
    if (CheckInventory("DRPGMonsterEPDrainHandler"))
        return;

    GiveInventory("DRPGMonsterEPDrainHandler", 1);

    // Delay Stagger
    Delay(35 + (GetMonsterID(0) % 4));

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

Start:

    if (ClassifyActor(0) & ACTOR_WORLD)
        return;

    if ((ClassifyActor(0) & ACTOR_DEAD) || !GetCVar("drpg_monster_blue_epdrain"))
    {
        Delay(35);
        goto Start;
    }

    if (!CheckInventory("DRPGMonsterEPDrainHandler"))
        return;

    if (!Stats->Aura.Type[AURA_BLUE].Active || CheckInventory("DRPGMonsterDisrupted") || GetActorProperty(0, APROP_Friendly))
    {
        Delay(35);
        goto Start;
    }

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        // Stupid hack to prevent stealing from triggering at the beginning of a level
        if (Timer() <= 4) break;

        if (!PlayerInGame(i)) continue;

        if (Distance(0, Players(i).TID) < Stats->Radius * GameSkill() * 4 + Stats->Radius && CheckSight(0, Players(i).TID, 0) && Players(i).EP > 0)
        {
            // Continue if this player is immune to stealing
            if (Players(i).Aura.Type[AURA_PINK].Active && Players(i).Aura.Type[AURA_PINK].Level >= 3) continue;

            int Previous = Players(i).EP;

            // [KS] Okay, so at natural max, a player has 2000 EP. We want that to drain in 20 seconds rather than just instantly.
            // This should also make specialized Energy auras more tolerable in general.
            int DrainAmount = Stats->Energy / 10 * GetCVarFixed("drpg_aurasteal_amount");
            if (DrainAmount < 1)
                DrainAmount = 1;

            Players(i).EP -= DrainAmount;

            if (Players(i).EP < 0)
                Players(i).EP = 0;

            if (Players(i).EP < Previous)
            {
                Popoff(0, Previous - Players(i).EP, DNUM_EPGAIN, "DRPGDigit", true);

                int OrigTID = ActivatorTID();
                int NewTID = UniqueTID(0, 0);
                Thing_ChangeTID(0, NewTID);
                SetActivator(Players(i).TID);
                FadeRangeFlash(64, 128, 128, 0.5, 64, 128, 128, 0.0, 0.5);
                ActivatorSound("drain/ep", 32);
                SetActivator(NewTID);
                Thing_ChangeTID(0, OrigTID);
            }
        }
    }

    Delay(GetCVar("drpg_aurasteal_rate"));

    goto Start;
}

NamedScript void MonsterRegenerationHandler()
{
    if (CheckInventory("DRPGMonsterRegenerationHandler"))
        return;

    int DelayTime;
    GiveInventory("DRPGMonsterRegenerationHandler", 1);

    // Delay Stagger
    Delay(35 + (GetMonsterID(0) % 4));

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

Start:

    if (GetActorProperty(0, APROP_Health) <= 0)
    {
        TakeInventory("DRPGMonsterRegenerationHandler", 1);
        return;
    }

    if (ClassifyActor(0) & ACTOR_WORLD)
        return;

    if (!CheckInventory("DRPGMonsterRegenerationHandler"))
        return;

    int RegenAmount = (Stats->HealthMax / 50) * (1 + Stats->Regeneration / (25 + Stats->Regeneration / 4));

    if (Stats->Flags & MF_BOSS)
        RegenAmount /= 2;

    if (Stats->Flags & MF_MEGABOSS)
        RegenAmount /= 2;

    if (Stats->RegenHealth >= Stats->HealthMax)
        RegenAmount /= 2;

    if (GetActorProperty(0, APROP_Friendly) && !CheckInventory("DRPGSummonedRegenerationBoosterToken"))
        RegenAmount /= 2;

    if (RegenAmount > INT_MAX)
        RegenAmount = INT_MAX;

    SetActorProperty(0, APROP_Health, GetActorProperty(0, APROP_Health) + RegenAmount);
    Stats->RegenHealth += RegenAmount;

    if (Stats->Aura.Type[AURA_PURPLE].Active && !CheckInventory("DRPGMonsterDisrupted") && GetCVar("drpg_monster_purple_massheal"))
    {
        GiveInventory("DRPGMonsterRadiusHealer", 1);
        DelayTime = 35 * 10;
    }
    else
    {
        if (CheckInventory("DRPGSummonedRegenerationBoosterToken"))
        {
            DelayTime = 35 * 20;
        }
        else
            DelayTime = 35 * 30;
    }

    // Prevent going over 100%
    if (GetActorProperty(0, APROP_Health) > Stats->HealthMax)
        SetActorProperty(0, APROP_Health, Stats->HealthMax);

    Delay(DelayTime);
    goto Start;
}

NamedScript DECORATE void MonsterRegenHealer()
{
    if (GetActorProperty(0, APROP_Friendly))
        return;

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

    int Health = GetActorProperty(0, APROP_Health);

    int HealAmount = Stats->HealthMax / 10;
    if (HealAmount < 1)
        HealAmount = 1;

    if (Health + HealAmount > Stats->HealthMax)
        HealAmount = Stats->HealthMax - Health;

    if (HealAmount <= 0)
        return;

    GiveInventory("DRPGMonsterHealEffect", 1);
    SetActorProperty(0, APROP_Health, Health + HealAmount);
}

NamedScript void MonsterDamageRetaliationHandler()
{
    if (CheckInventory("DRPGMonsterDamageRetaliationHandler"))
        return;

    GiveInventory("DRPGMonsterDamageRetaliationHandler", 1);

    // Delay Stagger
    Delay(35 + (GetMonsterID(0) % 4));

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];
    int Health = GetActorProperty(0, APROP_Health);
    int OldHealth = GetActorProperty(0, APROP_Health);

Start:
    if (ClassifyActor(0) & ACTOR_WORLD)
        return;

    if ((ClassifyActor(0) & ACTOR_DEAD) || !GetCVar("drpg_monster_red_retaliation"))
    {
        Delay(35);
        goto Start;
    }

    if (!CheckInventory("DRPGMonsterDamageRetaliationHandler"))
        return;

    if (!Stats->Aura.Type[AURA_RED].Active || CheckInventory("DRPGMonsterDisrupted"))
    {
        Delay(35);
        goto Start;
    }

    Health = GetActorProperty(0, APROP_Health);

    if (Health < OldHealth)
    {
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (!PlayerInGame(i)) continue;
            if (!CheckSight(0, Players(i).TID, 0))  continue;

            fixed CurseChance = ((fixed)Stats->Strength / 10.0) - (Distance(0, Players(i).TID) / 5.12);

            if (CurseChance <= 0)
                continue;

            if (CurseChance >= 100.0 || RandomFixed(0.0, 100.0) <= CurseChance)
            {
                int Intensity = ((fixed)Stats->Strength / 200.0) - (Distance(0, Players(i).TID) / 102.4);
                if (Intensity < 1) Intensity = 1;
                int OrigTID = ActivatorTID();
                int NewTID = UniqueTID(0, 0);
                Thing_ChangeTID(0, NewTID);
                SetActivator(Players(i).TID);
                FadeRangeFlash(255, 0, 0, 0.5, 255, 0, 0, 0.0, 0.5);
                TryStatusEffect(SE_CURSE, Random(30, 90), Intensity);
                SetActivator(NewTID);
                Thing_ChangeTID(0, OrigTID);
            }
        }
    }

    if (CheckInventory("DRPGMonsterEPAttacked"))
    {
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (!PlayerInGame(i)) continue;
            if (!CheckSight(0, Players(i).TID, 0)) continue;

            fixed SilenceChance = (Stats->Strength / 5) - (Distance(0, Players(i).TID) / 10.24);

            if (SilenceChance <= 0)
                continue;

            if (SilenceChance >= 100.0 || RandomFixed(0.0, 100.0) <= SilenceChance)
            {
                int Intensity = (Stats->Strength / 200) - (Distance(0, Players(i).TID) / 102.4);
                if (Intensity < 1) Intensity = 1;
                int OrigTID = ActivatorTID();
                int NewTID = UniqueTID(0, 0);
                Thing_ChangeTID(0, NewTID);
                SetActivator(Players(i).TID);
                FadeRangeFlash(255, 0, 0, 0.5, 255, 0, 0, 0.0, 0.5);
                TryStatusEffect(SE_SILENCE, Random(30, 90), Intensity);
                SetActivator(NewTID);
                Thing_ChangeTID(0, OrigTID);
            }
        }

        SetInventory("DRPGMonsterEPAttacked", 0);
    }

    Delay(4);

    OldHealth = Health;

    goto Start;
}

NamedScript void MonsterMoneyDrainHandler()
{
    if (CheckInventory("DRPGMonsterMoneyDrainHandler"))
        return;

    GiveInventory("DRPGMonsterMoneyDrainHandler", 1);

    // Delay Stagger
    Delay(35 + (GetMonsterID(0) % 4));

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

Start:
    if (ClassifyActor(0) & ACTOR_WORLD)
        return;

    if ((ClassifyActor(0) & ACTOR_DEAD) || !GetCVar("drpg_monster_yellow_moneydrain"))
    {
        Delay(35);
        goto Start;
    }

    if (!CheckInventory("DRPGMonsterMoneyDrainHandler"))
        return;

    if (!Stats->Aura.Type[AURA_YELLOW].Active || CheckInventory("DRPGMonsterDisrupted") || GetActorProperty(0, APROP_Friendly))
    {
        Delay(35);
        goto Start;
    }

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        // Stupid hack to prevent stealing from triggering at the beginning of a level
        if (Timer() <= 4) break;

        if (!PlayerInGame(i)) continue;

        if (Distance(0, Players(i).TID) < Stats->Radius * GameSkill() * 4 + Stats->Radius && CheckSight(0, Players(i).TID, 0))
        {
            // Continue if this player is immune to stealing
            if (Players(i).Aura.Type[AURA_PINK].Active && Players(i).Aura.Type[AURA_PINK].Level >= 3) continue;

            int Previous = CheckActorInventory(Players(i).TID, "DRPGCredits");
            int OrigTID = ActivatorTID();
            int NewTID = UniqueTID(0, 0);
            Thing_ChangeTID(0, NewTID);
            TakeActorInventory(Players(i).TID, "DRPGCredits", (int)(Stats->Luck * GetCVarFixed("drpg_aurasteal_amount")));
            GiveInventory("DRPGCredits", (int)(Stats->Luck * GetCVarFixed("drpg_aurasteal_amount")));
            SetActivator(Players(i).TID);
            FadeRangeFlash(255, 255, 0, 0.5, 255, 255, 0, 0.0, 0.5);
            ActivatorSound("drain/money", 32);
            SetActivator(NewTID);
            Thing_ChangeTID(0, OrigTID);

            if (CheckActorInventory(Players(i).TID, "DRPGCredits") < Previous)
                Popoff(0, Previous - CheckActorInventory(Players(i).TID, "DRPGCredits"), DNUM_CREDGAIN, "DRPGDIGIT", true);
        }
    }

    Delay(GetCVar("drpg_aurasteal_rate"));

    goto Start;
}

NamedScript void MonsterAmmoDrainHandler()
{
    if (CheckInventory("DRPGMonsterAmmoDrainHandler"))
        return;

    GiveInventory("DRPGMonsterAmmoDrainHandler", 1);

    // Delay Stagger
    Delay(35 + (GetMonsterID(0) % 4));

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

Start:
    if (ClassifyActor(0) & ACTOR_WORLD)
        return;

    if ((ClassifyActor(0) & ACTOR_DEAD) || !GetCVar("drpg_monster_darkblue_ammodrain"))
    {
        Delay(35);
        goto Start;
    }

    if (!CheckInventory("DRPGMonsterAmmoDrainHandler"))
        return;

    if (!Stats->Aura.Type[AURA_DARKBLUE].Active || CheckInventory("DRPGMonsterDisrupted") || GetActorProperty(0, APROP_Friendly))
    {
        Delay(35);
        goto Start;
    }

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        // Stupid hack to prevent stealing from triggering at the beginning of a level
        if (Timer() <= 4) break;

        if (!PlayerInGame(i)) continue;

        if (Distance(0, Players(i).TID) < Stats->Radius * GameSkill() * 4 + Stats->Radius && CheckSight(0, Players(i).TID, 0))
        {
            // Continue if this player is immune to stealing
            if (Players(i).Aura.Type[AURA_PINK].Active && Players(i).Aura.Type[AURA_PINK].Level >= 3) continue;

            // [KS] We'll give the player 30 seconds of ammo if they were full at stat max.
            int ClipSteal = (int)((fixed)Stats->Capacity / 7.5 * GetCVarFixed("drpg_aurasteal_amount"));
            int ShellSteal = (int)((fixed)Stats->Capacity / 30 * GetCVarFixed("drpg_aurasteal_amount"));
            int RocketSteal = (int)((fixed)Stats->Capacity / 120 * GetCVarFixed("drpg_aurasteal_amount"));
            int CellSteal = (int)((fixed)Stats->Capacity / 20 * GetCVarFixed("drpg_aurasteal_amount"));

            if (ClipSteal < 2) ClipSteal = 2;
            if (ShellSteal < 1) ShellSteal = 1;
            if (RocketSteal < 1) RocketSteal = 1;
            if (CellSteal < 3) CellSteal = 3;

            if (CheckActorInventory(Players(i).TID, "Clip") >= ClipSteal)
            {
                TakeActorInventory(Players(i).TID, "Clip",       ClipSteal);
                GiveInventory("Clip",       ClipSteal);
            }
            if (CheckActorInventory(Players(i).TID, "Shell") >= ShellSteal)
            {
                TakeActorInventory(Players(i).TID, "Shell",      ShellSteal);
                GiveInventory("Shell",      ShellSteal);
            }
            if (CheckActorInventory(Players(i).TID, "RocketAmmo") >= RocketSteal)
            {
                TakeActorInventory(Players(i).TID, "RocketAmmo", RocketSteal);
                GiveInventory("RocketAmmo", RocketSteal);
            }
            if (CheckActorInventory(Players(i).TID, "Cell") >= CellSteal)
            {
                TakeActorInventory(Players(i).TID, "Cell",       CellSteal);
                GiveInventory("Cell",       CellSteal);
            }

            int OrigTID = ActivatorTID();
            int NewTID = UniqueTID(0, 0);
            Thing_ChangeTID(0, NewTID);
            SetActivator(Players(i).TID);
            FadeRangeFlash(0, 0, 255, 0.5, 0, 0, 255, 0.0, 0.5);
            ActivatorSound("drain/ammo", 32);
            SetActivator(NewTID);
            Thing_ChangeTID(0, OrigTID);
        }
    }

    Delay(GetCVar("drpg_aurasteal_rate"));

    goto Start;
}

NamedScript void MonsterFellowResurrectionHandler()
{
    if (CheckInventory("DRPGMonsterFellowResurrectionHandler"))
        return;

    GiveInventory("DRPGMonsterFellowResurrectionHandler", 1);

    // Delay Stagger
    Delay(35 + (GetMonsterID(0) % 4));

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

Start:
    if (ClassifyActor(0) & ACTOR_WORLD)
        return;

    if ((ClassifyActor(0) & ACTOR_DEAD) || !GetCVar("drpg_monster_pink_resurrect"))
    {
        Delay(35);
        goto Start;
    }

    if (!CheckInventory("DRPGMonsterFellowResurrectionHandler"))
        return;

    if (!Stats->Aura.Type[AURA_PINK].Active || CheckInventory("DRPGMonsterDisrupted"))
    {
        Delay(35);
        goto Start;
    }

    GiveInventory("DRPGMonsterRadiusResurrection", 1);

    Delay(35 * (30 - (20 * Clamp(0, Stats->Vitality, 1000) / 1000)));

    goto Start;
}

NamedScript void MonsterEpicVisitTimeHandler()
{
    if (CheckInventory("DRPGMonsterEpicVisitTimeHandler"))
        return;

    GiveInventory("DRPGMonsterEpicVisitTimeHandler", 1);

    // Delay Stagger
    Delay(35 + (GetMonsterID(0) % 4));

    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

Start:
    if (ClassifyActor(0) & ACTOR_WORLD)
        return;

    if ((ClassifyActor(0) & ACTOR_DEAD) || !GetCVar("drpg_monster_orange_teleport"))
    {
        Delay(35);
        goto Start;
    }

    if (!CheckInventory("DRPGMonsterEpicVisitTimeHandler"))
        return;

    if (!Stats->Aura.Type[AURA_ORANGE].Active || CheckInventory("DRPGMonsterDisrupted"))
    {
        Delay(35);
        goto Start;
    }

    fixed TeleportDistance = 0;
    int TeleportPlayer = -1;

    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        if (!PlayerInGame(i)) continue;
        if (!CheckSight(0, Players(i).TID, 0)) continue;
        if (Distance(0, Players(i).TID) <= TeleportDistance) continue;

        TeleportDistance = Distance(0, Players(i).TID);
        TeleportPlayer = i;
    }

    fixed TeleportChance = ((fixed)TeleportDistance / 40.96) + (25.0 * (Stats->Agility / 1000.0)) ;

    if (RandomFixed(0.0, 100.0) <= TeleportChance)
    {
        int TeleportSpotTID = UniqueTID();
        int TargetPlayerTID = Players(TeleportPlayer).TID;

        bool Success = false;
        fixed TeleportAngle;
        fixed TeleportDistance;
        fixed X, Y, Z, Angle;

        for (int i = 0; i < 10; i++)
        {
            fixed TeleportAngle = RandomFixed(0.0, 1.0);
            fixed TeleportDistance = RandomFixed(128.0, 384.0);

            X = GetActorX(TargetPlayerTID) + (Cos(TeleportAngle) * TeleportDistance);
            Y = GetActorY(TargetPlayerTID) + (Sin(TeleportAngle) * TeleportDistance);
            Z = GetActorZ(TargetPlayerTID);
            Angle = TeleportAngle + 0.5;
            Angle %= 1.0;

            Success = Spawn("MapSpot", X, Y, Z, TeleportSpotTID, Angle);
            if (Success && !SetActorPosition(TeleportSpotTID, GetActorX(TeleportSpotTID), GetActorY(TeleportSpotTID), GetActorFloorZ(TeleportSpotTID), false)) Success = false;
            if (Success && !CheckSight(TargetPlayerTID, TeleportSpotTID, 0)) Success = false;
            if (Success)
                break;
        }

        if (Success)
        {
            // Surprise!!
            SpawnSpotFacingForced("TeleportFog", 0, 0);
            SetActorPosition(0, GetActorX(TeleportSpotTID), GetActorY(TeleportSpotTID), GetActorZ(TeleportSpotTID), false);
            SetActorAngle(0, Angle);
            SpawnSpotFacingForced("TeleportFog", 0, 0);
            Thing_Remove(TeleportSpotTID);
        }
    }

    Delay(35 * 10);

    goto Start;
}

NamedScript DECORATE void MonsterRevive()
{
    Delay(1);

    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

    if (ClassifyActor(0) & ACTOR_WORLD)
    {
        Stats->Init = false; // Mark as unused
        Stats->TID = 0;
        return;
    }

    // Account for monsters revived by friendly Archvile
    if (GetActorProperty(0, APROP_Friendly))
    {
        int MonsterTID = Stats->TID;
        int MasterTID = GetActorProperty(MonsterTID, APROP_MasterTID);
        int PlayerNumber;

        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (!PlayerInGame(i)) continue;

            // Checking if a player is monster's master
            if (MasterTID == Players(i).TID)
            {
                PlayerNumber = i;
                break;
            }
        }

        // If summons more than the maximum number or failure - the corpse is burned
        if (Players(PlayerNumber).Summons >= MAX_SUMMONS || Random(0, 100) <= 80)
        {
            // Chance 10% for get stuff from corpse
            if (Random(0, 100) <= 10)
            {
                switch (Random(1, 8))
                {
                case 1:
                    Spawn("DRPGMoneyDropper", GetActorX(MonsterTID), GetActorY(MonsterTID), GetActorZ(MonsterTID) + 48, 0, 0);
                    break;
                case 2:
                    Spawn("DRPGStimpack", GetActorX(MonsterTID), GetActorY(MonsterTID), GetActorZ(MonsterTID) + 48, 0, 0);
                    break;
                case 3:
                    Spawn("DRPGClip", GetActorX(MonsterTID), GetActorY(MonsterTID), GetActorZ(MonsterTID) + 48, 0, 0);
                    break;
                case 4:
                    Spawn("DRPGShell", GetActorX(MonsterTID), GetActorY(MonsterTID), GetActorZ(MonsterTID) + 48, 0, 0);
                    break;
                case 5:
                    if (CompatMode == COMPAT_DRLA)
                    {
                        Spawn("RLArmorBonusPickup", GetActorX(MonsterTID), GetActorY(MonsterTID), GetActorZ(MonsterTID) + 48, 0, 0);
                    }
                    else
                        Spawn("DRPGArmorBonus", GetActorX(MonsterTID), GetActorY(MonsterTID), GetActorZ(MonsterTID) + 48, 0, 0);
                    break;
                case 6:
                    Spawn("DRPGEPCapsule", GetActorX(MonsterTID), GetActorY(MonsterTID), GetActorZ(MonsterTID) + 48, 0, 0);
                    break;
                case 7:
                    Spawn("DRPGRocketAmmo", GetActorX(MonsterTID), GetActorY(MonsterTID), GetActorZ(MonsterTID) + 48, 0, 0);
                    break;
                case 8:
                    Spawn("DRPGCell", GetActorX(MonsterTID), GetActorY(MonsterTID), GetActorZ(MonsterTID) + 48, 0, 0);
                    break;
                }
            }

            ActivatorSound("vile/firestrt", 127);
            SpawnForced("SpawnFire", GetActorX(MonsterTID), GetActorY(MonsterTID), GetActorZ(MonsterTID), 0, 0);
            SpawnForced("DRPGBurnedCorpse", GetActorX(MonsterTID), GetActorY(MonsterTID), GetActorZ(MonsterTID), 0, 0);
            Thing_Remove(MonsterTID);
            return;
        }

        // Set property and handlers
        SetActorProperty(MonsterTID, APROP_Friendly, true);
        SetActorPropertyString(MonsterTID, APROP_Species, "Player");
        GiveActorInventory(MonsterTID, "DRPGFriendlyBooster", 1);
        MonsterFriendlyTeleport(false);

        // Add summon to your summon array
        for (int h = 0; h < MAX_SUMMONS; h++)
            if (Players(PlayerNumber).SummonTID[h] == 0)
            {
                Players(PlayerNumber).SummonTID[h] = MonsterTID;
                Players(PlayerNumber).Summons++;
                break;
            }

        // Set stats
        Stats->Threat = CalculateMonsterThreatLevel(&Monsters[GetMonsterID(MonsterTID)]);
        Stats->Flags |= MF_NOXP;
        Stats->Flags |= MF_NODROPS;
        Stats->NeedReinit = true;
    }
    else
        SetActorPropertyString(0, APROP_Species, "None");

    // Reboot handlers
    MonsterStatsHandler();
    MonsterAggressionHandler();
    if (!(Stats->Flags & MF_NOSTATS))
        MonsterRegenerationHandler();

    // Give full health
    SetActorProperty(0, APROP_Health, Stats->HealthMax);
}

NamedScript DECORATE void PropDeathCheck(int PropType)
{
    if (CurrentLevel && !CurrentLevel->UACBase && !CurrentLevel->UACArena)
    {
        Delay(1);

        int Killer = WhoKilledMe();

        // Calculate and set prop's DRPGCredits value
        SetInventory("DRPGCredits", Random(0, 100 * MapLevelModifier));

        if(PropType)
        {
            switch(PropType)
            {
            case 1: // tech
                DropMonsterItem(Killer, 0, "DRPGLootScrapMetals2",     37); // ~30%
                DropMonsterItem(Killer, 0, "DRPGTurretPart",           25); // ~20%
                DropMonsterItem(Killer, 0, "DRPGBatterySmall",         10); // ~4%
                break;

            case 2: // gore
                DropMonsterItem(Killer, 0, "DRPGAmmoMonsterDropper",   37); // ~30%
                DropMonsterItem(Killer, 0, "DRPGMedikitRandomizer",    37); // ~30%
                DropMonsterItem(Killer, 0, "DRPGWeaponDropper",        25); // ~20% - vanilla weapons only
                DropMonsterItem(Killer, 0, "DRPGGreenArmorRandomizer", 25); // ~20% - vanilla armors only
                break;

            case 3: // barrels
                DropMonsterItem(Killer, 0, "DRPGLootChemicals2",       12); // ~10%
                break;
            }
        }

        DropCredits(Killer, NULL);
    }
}

OptionalArgs(1) NamedScript void CalculateMonsterCredits(MonsterStatsPtr Stats, int StolenCredits)
{
    int Amount = Stats->Capacity;
    if(StolenCredits)
        Amount = Stats->Capacity + StolenCredits;

    if (GetCVar("drpg_ws_use_wads") < 4)
    {
        fixed Modifier = 4.0 / GetCVar("drpg_ws_use_wads") * PowFixed(MapLevelModifier, GetCVar("drpg_ws_use_wads") / 2);

        if (Modifier < 1.0)
            Modifier = 1.0;

        SetInventory("DRPGCredits", (int)(Amount / 2 * Modifier));
    }
    else
    {
        SetInventory("DRPGCredits", (int)(Amount / 2));
    }
}

NamedScript DECORATE void MonsterDeathCheck()
{
    Delay(1);

    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

    if (ClassifyActor(0) & ACTOR_WORLD)
    {
        Stats->Init = false; // Mark as unused
        Stats->TID = 0;
        return;
    }

    MonsterDeath();
}

NamedScript void DropCredits(int Killer, MonsterStatsPtr Stats)
{
    int LuckMult;
    int CreditsMin;
    int CreditsMax;
    int CreditsAmount;
    int CreditsTable[MAX_PLAYERS];
    int CreditsUAC;

    // Fair sharing
    if (GetCVar("drpg_multi_sharecredits") && Stats != NULL)
    {
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            LuckMult = 100 + (GetCVar("drpg_levelup_natural") ? Players(i).LuckTotal / 2 : Players(i).LuckTotal);
            CreditsMin = (CheckInventory("DRPGCredits") * LuckMult) / 1000;
            CreditsMax = (CheckInventory("DRPGCredits") * LuckMult) / 100;
            CreditsTable[i] = (Random(CreditsMin, CreditsMax) * (Stats->DamageTable[i] * 100) / Stats->HealthMax) / 100;

            // REK-T50 accessory
            if (Players(i).Shield.Active && Players(i).Shield.Accessory && Players(i).Shield.Accessory->PassiveEffect == SHIELD_PASS_EPICMEGACASH)
                CreditsTable[i] *= 2;

            // RAINBOWS Event
            if (CurrentLevel->Event == MAPEVENT_BONUS_RAINBOWS)
                CreditsTable[i] *= 2;

            // UAC Premium
            if (GetCVar("drpg_uac_premium"))
            {
                CreditsUAC = (fixed)Stats->Threat + CreditsTable[i] / 100.0 * (4.0 + (fixed)Players(i).RankLevel * 4.0);
                GiveActorInventory(Players(i).TID, "DRPGCredits", CreditsUAC);
            }
        }
    }
    else
    {
        LuckMult = 100 + (GetCVar("drpg_levelup_natural") ? Players(Killer).LuckTotal / 2 : Players(Killer).LuckTotal);
        CreditsMin = (CheckInventory("DRPGCredits") * LuckMult) / 1000;
        CreditsMax = (CheckInventory("DRPGCredits") * LuckMult) / 100;
        CreditsAmount = Random(CreditsMin, CreditsMax);

        // REK-T50 accessory
        if (Players(Killer).Shield.Active && Players(Killer).Shield.Accessory && Players(Killer).Shield.Accessory->PassiveEffect == SHIELD_PASS_EPICMEGACASH)
            CreditsAmount *= 2;

        // RAINBOWS Event
        if (CurrentLevel->Event == MAPEVENT_BONUS_RAINBOWS)
            CreditsAmount *= 2;

        // UAC Premium
        if (GetCVar("drpg_uac_premium") && Stats != NULL)
        {
            CreditsUAC = (fixed)Stats->Threat + CreditsAmount / 100.0 * (4.0 + (fixed)Players(Killer).RankLevel * 4.0);
            GiveActorInventory(Players(Killer).TID, "DRPGCredits", CreditsUAC);
        }
    }

    //Log("\CfInitial Amount: %d\n\CfLuck Mult: %d\n\CfMin: %d\n\CfMax: %d\n\CfAmount: %d", CheckInventory("DRPGCredits"), LuckMult, CreditsMin, CreditsMax, CreditsAmount);
    if (GetCVar("drpg_virtual_credits"))
    {
        if (GetCVar("drpg_multi_sharecredits") && Stats != NULL)
        {
            for (int i = 0; i < MAX_PLAYERS; i++)
                if (PlayerInGame(i))
                    GiveActorInventory(Players(i).TID, "DRPGCredits", CreditsTable[i]);
        }
        else
            GiveActorInventory(Players(Killer).TID, "DRPGCredits", CreditsAmount);
    }
    else
    {
        if (GetCVar("drpg_multi_sharecredits") && Stats != NULL)
        {
            for (int i = 0; i < MAX_PLAYERS; i++)
                if (PlayerInGame(i))
                    DropMoney(i, 0, CreditsTable[i]);
        }
        else
            DropMoney(Killer, 0, CreditsAmount);
    }
}

NamedScript void MonsterDeath()
{
    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];
    // Don't forget to remove stupid fixed-point avoidance code after migration (if it happens)
    int Killer = WhoKilledMe();
    long int HealthXP;

    if (GetCVarFixed("drpg_xp_health_awareness") < 1.0)
        HealthXP = (long int)Stats->SpawnHealth + (((long int)Stats->HealthMax - (long int)Stats->SpawnHealth) * (long int)(GetCVarFixed("drpg_xp_health_awareness") * 10l)) / 10l;
    else
        HealthXP = ((long int)Stats->HealthMax * (long int)(GetCVarFixed("drpg_xp_health_awareness") * 10l)) / 10l;

    fixed ThreatMult = (fixed)Stats->Threat + ((fixed)Stats->Level / ((fixed)GetCVar("drpg_ws_use_wads") * 6.0)) + 8.0 * PowFixed(MapLevelModifier, GetCVar("drpg_ws_use_wads") / 2);
    if (ThreatMult < 1.0)
        ThreatMult = 1.0;

    long int XPAmount = Random(HealthXP / 2l, HealthXP) * (long fixed)ThreatMult;
    long int RankAmount = HealthXP * (long fixed)ThreatMult;

    if (Players(Killer).Shield.Accessory)
    {
        switch (Players(Killer).Shield.Accessory->PassiveEffect)
        {
        case SHIELD_PASS_KILLSCHARGE:
            AddRemoteShield(Players(Killer).TID, Stats->HealthMax / 20);
            break;
        case SHIELD_PASS_BLOODYSHIELDSOREAL:
            Players(Killer).Shield.AccessoryBattery = 35 * 3;
            break;
        }
    }

    // Credit Rain
    if (Killer > -1 && (Players(Killer).LuckTotal >= 100 || CurrentLevel->Event == MAPEVENT_BONUS_RAINBOWS) && Random(1, 256) == 1)
    {
        fixed Z = GetActorCeilingZ(Players(Killer).TID) - GetActorPropertyFixed(Players(Killer).TID, APROP_Height);
        fixed Radius = GetActorPropertyFixed(Players(Killer).TID, APROP_Radius);

        ThingSound(Players(Killer).TID, "menu/buy", 127);
        Log("\CdSHINY CREDITS!");

        for (int i = 0; i < 20; i++)
        {
            int CreditType = Random(1, 8);
            str CreditName;

            switch (CreditType)
            {
            case 1:
                CreditName = "DRPGCredits1";
                break;
            case 2:
                CreditName = "DRPGCredits5";
                break;
            case 3:
                CreditName = "DRPGCredits10";
                break;
            case 4:
                CreditName = "DRPGCredits20";
                break;
            case 5:
                CreditName = "DRPGCredits50";
                break;
            case 6:
                CreditName = "DRPGCredits100";
                break;
            case 7:
                CreditName = "DRPGCredits250";
                break;
            case 8:
                CreditName = "DRPGCredits500";
                break;
            }

            DropMonsterItem(Killer, Players(Killer).TID, CreditName, 256, RandomFixed(-Radius, Radius), RandomFixed(-Radius, Radius), Z);
            Delay(1);
        }
    }

    // Add XP/Rank
    if (!(Stats->Flags & MF_NOXP) && !GetActorProperty(0, APROP_Friendly))
    {
        // Take Rank if the enemy has MF_RANKLOSS
        if (Stats->Flags & MF_RANKLOSS)
            RankAmount = -RankAmount;

        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (!PlayerInGame(i)) continue;

            Stats->DamageTable[i] = Clamp(0, Stats->DamageTable[i], Stats->HealthMax);

            long fixed mult = 1.0;
            if (InMultiplayer && !GetCVar("drpg_multi_sharexp") && (long fixed)Stats->HealthMax > 0)
                mult = (long fixed)Stats->DamageTable[i] / (long fixed)Stats->HealthMax;
            else if (Killer < 0)
                mult = 0;

            if (mult > 0)
            {
                XPAmount = (int)(XPAmount * mult);
                RankAmount = (int)(RankAmount * mult);

                AddXP(i, XPAmount, RankAmount);
                if (GetCVar("drpg_levelup_natural"))
                {
                    long fixed Scale = GetCVarFixed("drpg_strength_scalexp") / GetCVar("drpg_ws_use_wads");
                    if (GetCVar("drpg_allow_spec"))
                    {
                        if (GetActivatorCVar("drpg_character_spec") == 1)
                            Scale *= 2;
                    }
                    Players(i).StrengthXP += (long int)(HealthXP * mult * Scale);
                }
            }
        }
    }

    // Mission Handling
    MissionDeathCheck(Killer, Stats);

    // SHOTGUNS!
    if (GetCVar("drpg_shotguns"))
        DropMonsterItem(Killer, 0, "DRPGShotSpawn", 1, 256);

    // Drops
    if (!(Stats->Flags & MF_NODROPS) && !GetActorProperty(0, APROP_Friendly) && !GetCVar("drpg_monster_shadows"))
    {
        // Calculate current map total monsters modifier
        fixed DropMonsterModifier = MapTotalMonstersMod();

        // Auras have a chance of rare vial drops
        for (int i = 0; i < AURA_MAX; i++)
            if (Stats->Aura.Type[i].Active)
                DropMonsterItem(Killer, 0, "DRPGVialDropperRare", 16 * DropMonsterModifier);

        // Aura Drops
        if (Stats->Aura.Type[AURA_RED].Active) // Red Aura - Strength
            DropMonsterItem(Killer, 0, "DRPGVialDropper", 64 * DropMonsterModifier);
        if (Stats->Aura.Type[AURA_GREEN].Active) // Green Aura - Defense
            DropMonsterItem(Killer, 0, "DRPGVialDropper", 64 * DropMonsterModifier);
        if (Stats->Aura.Type[AURA_WHITE].Active) // White Aura - XP
            DropMonsterItem(Killer, 0, "DRPGVialDropper", 64 * DropMonsterModifier);
        if (Stats->Aura.Type[AURA_PINK].Active) // Pink Aura - Vitality
            DropMonsterItem(Killer, 0, "DRPGVialDropper", 64 * DropMonsterModifier);
        if (Stats->Aura.Type[AURA_BLUE].Active) // Blue Aura - Energy
            DropMonsterItem(Killer, 0, "DRPGVialDropper", 64 * DropMonsterModifier);
        if (Stats->Aura.Type[AURA_PURPLE].Active) // Purple Aura - Regeneration
            DropMonsterItem(Killer, 0, "DRPGVialDropper", 64 * DropMonsterModifier);
        if (Stats->Aura.Type[AURA_ORANGE].Active) // Orange Aura - Agility
            DropMonsterItem(Killer, 0, "DRPGVialDropper", 64 * DropMonsterModifier);
        if (Stats->Aura.Type[AURA_DARKBLUE].Active) // Dark Blue Aura - Capacity
            DropMonsterItem(Killer, 0, "DRPGVialDropper", 64 * DropMonsterModifier);
        if (Stats->Aura.Type[AURA_YELLOW].Active) // Yellow Aura - Luck
        {
            DropMonsterItem(Killer, 0, "DRPGVialDropper", 64 * DropMonsterModifier);
            DropMonsterItem(Killer, 0, "DRPGChipDropper", 64 * DropMonsterModifier);
        }

        // Luck-based Drops
        if (Killer > -1 && !(Stats->Flags & MF_MEGABOSS))
        {
            if (Players(Killer).HealthDrop && RandomFixed(0.0, 100.0) < Players(Killer).HealthChance)    DropMonsterItem(Killer, 0, "DRPGHealthMonsterDropper", 256 * DropMonsterModifier);
            if (Players(Killer).EPDrop && RandomFixed(0.0, 100.0) < Players(Killer).EPChance)            DropMonsterItem(Killer, 0, "DRPGEPMonsterDropper", 256 * DropMonsterModifier);
            if (Players(Killer).AmmoDrop && RandomFixed(0.0, 100.0) < Players(Killer).AmmoChance)        DropMonsterItem(Killer, 0, "DRPGAmmoMonsterDropper", 256 * DropMonsterModifier);
            if (Players(Killer).TurretDrop && RandomFixed(0.0, 100.0) < Players(Killer).TurretChance)    DropMonsterItem(Killer, 0, "DRPGTurretMonsterDropper", 256 * DropMonsterModifier);
            if (Players(Killer).ModuleDrop && RandomFixed(0.0, 100.0) < Players(Killer).ModuleChance)    DropMonsterItem(Killer, 0, "DRPGModuleDropper", 256 * DropMonsterModifier);
            if (Players(Killer).ArmorDrop && RandomFixed(0.0, 100.0) < Players(Killer).ArmorChance)      DropMonsterItem(Killer, 0, "DRPGArmorDropper", 256 * DropMonsterModifier);
            if (Players(Killer).WeaponDrop && RandomFixed(0.0, 100.0) < Players(Killer).WeaponChance)    DropMonsterItem(Killer, 0, "DRPGWeaponDropper", 256 * DropMonsterModifier);
            if (Players(Killer).ShieldDrop && RandomFixed(0.0, 100.0) < Players(Killer).ShieldChance)    DropMonsterItem(Killer, 0, "DRPGShieldDropper", 256 * DropMonsterModifier);
            if (Players(Killer).AugDrop && RandomFixed(0.0, 100.0) < Players(Killer).AugChance)          DropMonsterItem(Killer, 0, "DRPGAugDropper", 256 * DropMonsterModifier);
        }

        Delay(1);

        // Compatibility Handling - LegenDoom
        // LegenDoom monster drops
        if (CompatMode == COMPAT_LEGENDOOM || CompatModeLite == COMPAT_LEGENDOOMLITE)
        {
            if (CheckInventory("LDLegendaryMonsterToken") && GetCVar("LD_droptype") == 1)
            {
                DropMonsterItem(Killer, 0, "DRPGLuckDropper", 256);
            }
        }

        // Boss Drops
        if (Stats->Flags & MF_BOSS)
        {
            fixed DropBossModifier = MapTotalBossesMod();

            DropMonsterItem(Killer, 0, "DRPGCredits250", 200 * DropBossModifier);
            DropMonsterItem(Killer, 0, "DRPGCredits500", 170 * DropBossModifier);
            DropMonsterItem(Killer, 0, "DRPGCredits1000", 256 * DropBossModifier);
            DropMonsterItem(Killer, 0, "DRPGSoulsphereRandomizer", 256 * DropBossModifier);
            DropMonsterItem(Killer, 0, "DRPGLifeDropper", 80 * DropBossModifier);
            DropMonsterItem(Killer, 0, "DRPGModulePickup", 256 * DropBossModifier);
            DropMonsterItem(Killer, 0, "DRPGAugDropper", 96 * DropBossModifier);
            DropMonsterItem(Killer, 0, "DRPGUACCard", (40 / (Players(Killer).ShopCard + 1)) * DropBossModifier);
            DropMonsterItem(Killer, 0, "DRPGStimPackageStat", 48 * DropBossModifier);
            DropMonsterItem(Killer, 0, "DRPGStimPackagePowerup", 24 * DropBossModifier);
            DropMonsterItem(Killer, 0, "DRPGShieldSpawner", 48 * DropBossModifier);
            DropMonsterItem(Killer, 0, "DRPGArmorDropper", 32 * DropBossModifier);
            DropMonsterItem(Killer, 0, "DRPGWeaponDropper", 16 * DropBossModifier);
            DropMonsterItem(Killer, 0, "DRPGImmunityCrystalDropper", 32 * DropBossModifier);

            if (CompatMode == COMPAT_DRLA)
            {
                DropMonsterItem(Killer, 0, "DRPGCraftPartsExotic", 32 * DropBossModifier);
                if (MapLevelModifier > 0.25)
                    DropMonsterItem(Killer, 0, "DRPGCraftPartsUnique", 16 * DropBossModifier);
                DropMonsterItem(Killer, 0, "RLBlueprintComputer", 32 * DropBossModifier);
                DropMonsterItem(Killer, 0, "RLBasicModPackSpawner", 16 * DropBossModifier);
                DropMonsterItem(Killer, 0, "RLExoticModPackSpawner", 8 * DropBossModifier);
            }
        }

        // Megaboss Drops
        if (Stats->Flags & MF_MEGABOSS)
        {
            AmbientSound("megaboss/lootsplosion", 127);
            Delay(35 * 2.25); // Just long enough to sync up with the sound nicely

            // LOOTSPLOOOOOOOOOOOOOSIIIOOOOOOOOOOOOOONNNNNN
            int Rolls = (200 / 8) * PlayerCount();
            while (Rolls--)
            {
                DropMonsterItem(Killer, 0, "DRPGCredits250", 256, 0, 0, 0, 16, 16, Random(8, 16));
                DropMonsterItem(Killer, 0, "DRPGCredits500", 256, 0, 0, 0, 16, 16, Random(8, 16));
                DropMonsterItem(Killer, 0, "DRPGCredits1000", 256, 0, 0, 0, 16, 16, Random(8, 16));
                for (int i = 0; i < PlayerCount(); i++)
                    DropMonsterItem(Killer, 0, "DRPGGenericBossDropper", 256, 0, 0, 0, 16, 16, Random(8, 16));
                if ((Rolls % 5) == 0)
                    Delay(1);
            }

            DropMonsterItem(Killer, 0, "DRPGImmunityCrystalDropper", 128);
            DropMonsterItem(Killer, 0, "DRPGArmorDropper", 128);
            DropMonsterItem(Killer, 0, "DRPGWeaponDropper", 64);
            DropMonsterItem(Killer, 0, "DRPGUACCard", 16);

            if (CompatMode == COMPAT_DRLA)
            {
                DropMonsterItem(Killer, 0, "DRPGCraftPartsExotic", 128);
                DropMonsterItem(Killer, 0, "DRPGCraftPartsUnique", 64);
                DropMonsterItem(Killer, 0, "RLBlueprintComputer", 128);
                DropMonsterItem(Killer, 0, "RLBasicModPackSpawner", 64);
                DropMonsterItem(Killer, 0, "RLExoticModPackSpawner", 32);
            }
        }
    }

    // Drop Radiation Suits during the Environmental Hazard event
    if (!(Stats->Flags & MF_NODROPS) && !GetActorProperty(0, APROP_Friendly) && CheckInventory("DRPGMonsterRadSuit"))
        DropMonsterItem(Killer, 0, "RadSuit", 256);

    // Drop Nuke Keys during the Thermonuclear Bomb Map Event
    if (!(Stats->Flags & MF_NODROPS) && !GetActorProperty(0, APROP_Friendly) && CurrentLevel->Event == MAPEVENT_NUCLEARBOMB && (Random(1, 2) == 1 || Stats->Flags & MF_BOSS || Stats->Flags & MF_MEGABOSS))
        DropMonsterItem(Killer, 0, StrParam("DRPGNukeKey%d", Random(1, MAX_NUKE_KEYS)), 256);

    // Drop Generator Power Cells during the Low Power Event
    if (!(Stats->Flags & MF_NODROPS) && !GetActorProperty(0, APROP_Friendly) && CurrentLevel->Event == MAPEVENT_LOWPOWER && !CurrentLevel->PowerGeneratorActive)
        if (Random(1, CurrentLevel->MaxTotalMonsters / 4) == 1)
            DropMonsterItem(Killer, 0, "DRPGGeneratorCell", 256);
        else
            DropMonsterItem(Killer, 0, "DRPGGeneratorCellDead", 128);

    // Drop Credits
    if (!(Stats->Flags & MF_NODROPS) && CheckInventory("DRPGCredits") > 0)
    {
        DropCredits(Killer, Stats);
    }

    // Drop stolen ammo
    int Count = 0;
    while (CheckInventory("Clip") >= 10)
    {
        DropMonsterItem(Killer, 0, "DRPGStolenClip", 256);
        TakeInventory("Clip", 10);
        if ((Count++ % 1000) == 0) Delay(1);
    }
    while (CheckInventory("Shell") >= 4)
    {
        DropMonsterItem(Killer, 0, "DRPGStolenShell", 256);
        TakeInventory("Shell", 4);
        if ((Count++ % 1000) == 0) Delay(1);
    }
    while (CheckInventory("RocketAmmo") >= 1)
    {
        DropMonsterItem(Killer, 0, "DRPGStolenRocketAmmo", 256);
        TakeInventory("RocketAmmo", 1);
        if ((Count++ % 1000) == 0) Delay(1);
    }
    while (CheckInventory("Cell") >= 20)
    {
        DropMonsterItem(Killer, 0, "DRPGStolenCell", 256);
        TakeInventory("Cell", 20);
        if ((Count++ % 1000) == 0) Delay(1);
    }

    // [KS] We killed this monster now, so if an Arch-vile resurrects it, it doesn't count anymore to re-kill
    // TODO: Need to maybe make sure that for Kill Missions, we can't just gimp Arch-vile resurrection.
    Stats->Flags |= MF_NOXP;
    Stats->Flags |= MF_NODROPS;
    SetActorProperty(0, APROP_RenderStyle, Stats->RenderStyle); // In case of Shadow Aura
    Stats->Target = false;
    Stats->Reinforcement = false;

    // [KS] Nuke auras so we don't cause bizarre problems
    // [KS] 5/11/2015 - Actually shouldn't shouldn't anymore, uncomment it if it still does
    // [SW] 10/22/2018 - This is an option now, yee.
    if (GetCVar("drpg_aura_removeondeath"))
        RemoveMonsterAura(Stats);

    // Corpses cleanup
    if (GetCVar("drpg_corpses_cleanup") > 0)
        CorpsesCleanup();
}

//[[alloc_Aut(16384)]]
NamedScript DECORATE void MonsterTransport(int Difficulty, int Time, int Radius)
{
    MonsterInfoPtr MonsterList[MAX_TEMP_MONSTERS];
    int MonsterListLength;
    int BossesSpawned = 0;

    MonsterInfoPtr TempMonster;

    // Build a list of monsters
    for (int i = 0; i < MonsterDataAmount && MonsterListLength < MAX_TEMP_MONSTERS; i++)
    {
        TempMonster = &MonsterData[i];

        if (CompatMonMode == COMPAT_DRLA || CompatMonMode == COMPAT_PANDEMONIA)
        {
            if (Difficulty == 0)
                if (TempMonster->Difficulty > 0 & TempMonster->Difficulty < 60 & TempMonster->ThreatLevel < 24)
                    MonsterList[MonsterListLength++] = TempMonster;
            if (Difficulty == 1)
                if (TempMonster->Difficulty > 20 & TempMonster->Difficulty < 80 & TempMonster->ThreatLevel < 24)
                    MonsterList[MonsterListLength++] = TempMonster;
            if (Difficulty == 2)
                if (TempMonster->Difficulty > 40 & TempMonster->Difficulty < 100 & TempMonster->ThreatLevel < 24)
                    MonsterList[MonsterListLength++] = TempMonster;
            if (Difficulty == 3)
                if (TempMonster->Difficulty > 100 & TempMonster->Difficulty < 160 & TempMonster->ThreatLevel < 24)
                    MonsterList[MonsterListLength++] = TempMonster;
            if (Difficulty == 4)
                if (TempMonster->Difficulty > 130 & TempMonster->Difficulty < 190 & TempMonster->ThreatLevel < 24)
                    MonsterList[MonsterListLength++] = TempMonster;
            if (Difficulty == 5)
                if (TempMonster->Difficulty > 160 & TempMonster->Difficulty < 220 & TempMonster->ThreatLevel < 24)
                    MonsterList[MonsterListLength++] = TempMonster;
            if (Difficulty == 6)
                if (TempMonster->Difficulty > 220 & TempMonster->Difficulty < 280 & TempMonster->ThreatLevel < 24)
                    MonsterList[MonsterListLength++] = TempMonster;
            if (Difficulty == 7)
                if (TempMonster->Difficulty > 240 & TempMonster->Difficulty < 300 & TempMonster->ThreatLevel < 24)
                    MonsterList[MonsterListLength++] = TempMonster;
            if (Difficulty == 8)
                if (TempMonster->Difficulty > 260 & TempMonster->Difficulty < 320 & TempMonster->ThreatLevel < 24)
                    MonsterList[MonsterListLength++] = TempMonster;
        }
        else
        {
            if (TempMonster->Difficulty >= ((Difficulty + 1) * 10) - 40 &&
                    TempMonster->Difficulty <= ((Difficulty + 1) * 10) + 10)
                MonsterList[MonsterListLength++] = TempMonster;
        }
    }

    if (!MonsterListLength)
    {
        Log("\CiWARNING: \CaReinforcement Mission on %S\Ca has no monsters!", MissionDifficulties[Difficulty]);
        return;
    }

    bool Complete, Success, IsBoss;
    fixed X, Y, Z, SpawnX, SpawnY;
    int MonsterIndex, TID, SpawnTries, RadiusMin, RadiusMax;
    MonsterStatsPtr Stats;

    while (true)
    {
        Delay((Time > 0 ? (35 * Time) : -Time));

        Complete = true;
        X = GetActorX(0);
        Y = GetActorY(0);
        Z = GetActorZ(0);

        // Stop spawning if time is frozen
        while (IsTimeFrozen()) Delay(1);

        // Check to see if the mission is complete
        for (int i = 0; i < MAX_PLAYERS; i++)
            if (PlayerInGame(i) && Players(i).Mission.Active && Players(i).Mission.Type == MT_REINFORCEMENTS)
                Complete = false;

        // If the mission is complete, terminate
        if (Complete) return;

        TID = UniqueTID();
        Success = false;
        SpawnTries = 0;
        RadiusMin = 256;
        RadiusMax = Radius;
        Stats = NULL;
        IsBoss = false;

        while (!Success && SpawnTries < 3 && CurrentLevel->Event != MAPEVENT_MEGABOSS)
        {
            MonsterIndex = Random(0, MonsterListLength - 1);

            SpawnX = RandomFixed(-(fixed)RadiusMax, (fixed)RadiusMax);
            SpawnY = RandomFixed(-(fixed)RadiusMax, (fixed)RadiusMax);

            // Get the floor Z position at this spot
            SpawnForced("MapSpot", X + SpawnX, Y + SpawnY, Z, TID, 0);
            Z = GetActorFloorZ(TID);
            Thing_Remove(TID);

            Success = Spawn(GetMissionMonsterActor(MonsterList[MonsterIndex]->Actor), X + SpawnX, Y + SpawnY, Z, TID, 0);

            IsBoss = CheckFlag(TID, "BOSS");

            if (Success)
                Success = CheckSight(0, TID, 0) && Distance(0, TID) > RadiusMin;
            if (Success)
                Success = !IsBoss || (!Random (0, 3) && BossesSpawned < 3);

            if (!Success)
            {
                // Try again, closer to the player each time, up to 3 times, before giving up.
                Thing_Remove(TID);
                RadiusMax /= 2;
                if (SpawnTries == 1)
                    RadiusMin /= 2;
            }
            else
            {
                if (IsBoss)
                    BossesSpawned++;
            }

            SpawnTries++;
            Delay(1);
        }

        if (Success)
        {
            // Delay here to make sure the init script has time to run
            Delay(4);

            // Pointer
            Stats = &Monsters[GetMonsterID(TID)];

            Thing_Hate(TID, Player.TID);
            Stats->Reinforcement = true;
            Thing_ChangeTID(TID, 0); // Get rid of the ID

            Spawn("TeleportFog", X + SpawnX, Y + SpawnY, Z, 0, 0);

            if (DebugLog)
                Log("\CdDEBUG: \C-Reinforcements Spawning Index %d (\Ca%S\C-)", MonsterIndex, MonsterList[MonsterIndex]->Name);
        }
    }
}

NamedScript int WhoKilledMe()
{
    SetActivatorToTarget(0);

    // Killed by a player
    if (PlayerNumber() > -1)
        return PlayerNumber();

    // Killed by a player's turret
    for (int i = 0; i < MAX_PLAYERS; i++)
    {
        // Player is not in-game
        if (!PlayerInGame(i)) continue;

        // Don't have a turret or it isn't out
        if (!Players(i).Turret.Upgrade[TU_BUILD] || !Players(i).Turret.Active) continue;

        if (ActivatorTID() == Players(i).Turret.TID)
            return i;
    }

    // Killed by a player's summons
    if (ClassifyActor(0) & ACTOR_MONSTER && GetMonsterID(0) && GetActorProperty(0, APROP_MasterTID))
    {
        SetActivator(GetActorProperty(0, APROP_MasterTID));
        return PlayerNumber();
    }

    // Killed by another monster or the world
    return -1;
}

void MonsterLevelup(MonsterStatsPtr Stats)
{
    // Update monster caps
    MonsterLevelCap = GetCVar("drpg_monster_level_cap");
    MonsterStatCap = GetCVar("drpg_monster_stat_cap");

    // If the monster is max level, return
    if (Stats->Level >= MonsterLevelCap) return;

    // Apply the stats to the monster
    Stats->Level++;

    int Pool = 5;

    while (Pool > 0)
    {
        switch (Random (0, STAT_MAX - 1))
        {
        case 0:
            Stats->Strength++;
            break;
        case 1:
            Stats->Defense++;
            break;
        case 2:
            Stats->Vitality++;
            break;
        case 3:
            Stats->Energy++;
            break;
        case 4:
            Stats->Regeneration++;
            break;
        case 5:
            Stats->Agility++;
            break;
        case 6:
            Stats->Capacity++;
            break;
        case 7:
            Stats->Luck++;
            break;
        }
        Pool--;
    }

    // Spawn Level-up Arrow
    SpawnForced("DRPGLevelUpArrow", GetActorX(0), GetActorY(0), GetActorZ(0) + GetActorPropertyFixed(0, APROP_Height), 0, 0);

    if (CheckInventory("DRPGMonsterLevelup"))
        TakeInventory("DRPGMonsterLevelup", 1);
}

void CapMonsterStats(MonsterStatsPtr Stats)
{
    if (Stats->Level <= 0)
        Stats->Level = 1;
    if (Stats->Level > MonsterLevelCap)
        Stats->Level = MonsterLevelCap;
    if (Stats->Strength > MonsterStatCap)
        Stats->Strength = MonsterStatCap;
    if (Stats->Defense > MonsterStatCap)
        Stats->Defense = MonsterStatCap;
    if (Stats->Vitality > MonsterStatCap)
        Stats->Vitality = MonsterStatCap;
    if (Stats->Energy > MonsterStatCap)
        Stats->Energy = MonsterStatCap;
    if (Stats->Regeneration > MonsterStatCap)
        Stats->Regeneration = MonsterStatCap;
    if (Stats->Agility > MonsterStatCap)
        Stats->Agility = MonsterStatCap;
    if (Stats->Capacity > MonsterStatCap)
        Stats->Capacity = MonsterStatCap;
    if (Stats->Luck > MonsterStatCap)
        Stats->Luck = MonsterStatCap;
}

int CalculateMonsterThreatLevel(MonsterStatsPtr Stats)
{
    fixed Threat;

    // Stats
    Threat += Stats->Strength;
    Threat += Stats->Defense;
    Threat += Stats->Vitality;
    Threat += Stats->Energy;
    Threat += Stats->Regeneration;
    Threat += Stats->Agility;
    Threat += Stats->Capacity;
    Threat += Stats->Luck;
    Threat /= 200.0;

    // Auras
    for (int i = 0; i < AURA_MAX; i++)
        if (Stats->Aura.Type[i].Active)
            Threat += 0.25;

    // Bosses
    if (CheckFlag(0, "BOSS") || Stats->Flags & MF_BOSS)
        Threat += 1.0;
    // Megaboss
    if (Stats->Flags & MF_MEGABOSS)
        Threat += 2.0;

    // Monsters
    if (CompatMonMode != COMPAT_NONE)
    {
        str Actor = GetActorClass(0);

        for (int i = 0; i < MonsterDataAmount; i++)
        {
            MonsterInfoPtr MonsterIterPtr = &MonsterData[i];

            if (StartsWith(Actor, MonsterIterPtr->Actor))
            {
                Threat += (fixed)((0.25 - 0.01 * MonsterIterPtr->ThreatLevel) + MonsterIterPtr->ThreatLevel / 10.0);
                break;
            }
        }
    }

    // Cap
    if (Threat > 10.0)
        Threat = 10.0;

    return RoundInt(Threat);
}

int CalculateMonsterMaxHealth(MonsterStatsPtr Stats)
{
    long int Health = Stats->SpawnHealth;
    long int HealthAddition = Stats->SpawnHealth * GetCVar("drpg_monster_vitality_effect");
    int HealthBoost = (Stats->Vitality * (int)(GetCVarFixed("drpg_monster_vitality_boost") * 100)) / 100;

    HealthAddition *= Stats->Vitality;
    HealthAddition /= 1000;

    Health += HealthAddition + HealthBoost;

    // Compatibility Handling - LegenDoom
    // Add health for Legendary monsters
    if (CompatMode == COMPAT_LEGENDOOM || CompatModeLite == COMPAT_LEGENDOOMLITE)
    {
        if (CheckInventory("LDLegendaryMonsterToken"))
        {
            if (CheckFlag(0, "BOSS"))
                Health *= GetCVar("LD_legendaryhealthboss") / 100;
            else
                Health *= GetCVar("LD_legendaryhealth") / 100;

            GiveInventory("DRPGLegenDoomMonsterInit", 1);
        }
    }

    return Health;
}

fixed CalculateMonsterSpeed(MonsterStatsPtr Stats)
{
    fixed Speed = (fixed)Stats->Speed;
    fixed SpeedMax = Stats->Radius - 1.0;

    Speed *= 1.0 + ((fixed)Stats->Agility / 1000.0 * 4.0);
    if (Speed > SpeedMax)
        Speed = SpeedMax;

    return Speed;
}

void RemoveMonsterAura(MonsterStatsPtr Stats)
{
    Stats->Aura.Time = 0;
    Stats->Aura.Team = false;
    Stats->HasAura = false;
    Stats->HasShadowAura = false;

    for (int i = 0; i < AURA_MAX; i++)
    {
        Stats->Aura.Type[i].Active = false;
        Stats->Aura.Type[i].Level = 0;
    };

    // Flag Removers
    MonsterRedAuraCheck(false);
    MonsterGreenAuraCheck(false);
    MonsterWhiteAuraCheck(false);
    MonsterPinkAuraCheck(false);
    MonsterBlueAuraCheck(false);
    MonsterOrangeAuraCheck(false);

    // Reset Renderstyle
    SetActorProperty(0, APROP_RenderStyle, Stats->RenderStyle);
}

str DetermineBestStatColor(MonsterStatsPtr Stats)
{
    str const StatColorsM[STAT_MAX] =
    {
        "\Cg",
        "\Cd",
        "\Ca",
        "\Cn",
        "\Ct",
        "\Ci",
        "\Ch",
        "\Cf"
    };

    int BaseStat[STAT_MAX];
    int HighestStat = 0;
    int HighestStatType = 0;

    if (Stats->Flags & MF_NOSTATS)
        return "\Cc";

    BaseStat[0] = Stats->Strength;
    BaseStat[1] = Stats->Defense;
    BaseStat[2] = Stats->Vitality;
    BaseStat[3] = Stats->Energy;
    BaseStat[4] = Stats->Regeneration;
    BaseStat[5] = Stats->Agility;
    BaseStat[6] = Stats->Capacity;
    BaseStat[7] = Stats->Luck;

    for (int i = 0; i < STAT_MAX; i++)
        if (BaseStat[i] > HighestStat)
        {
            HighestStat = BaseStat[i];
            HighestStatType = i;
        }

    return StatColorsM[HighestStatType];
}

int BestStat(MonsterStatsPtr Stats)
{
    int BaseStat[STAT_MAX];
    int HighestStat = 0;
    int HighestStatType = 0;

    if (Stats->Flags & MF_NOSTATS)
        return Random(0, STAT_MAX);

    BaseStat[0] = Stats->Strength;
    BaseStat[1] = Stats->Defense;
    BaseStat[2] = Stats->Vitality;
    BaseStat[3] = Stats->Energy;
    BaseStat[4] = Stats->Regeneration;
    BaseStat[5] = Stats->Agility;
    BaseStat[6] = Stats->Capacity;
    BaseStat[7] = Stats->Luck;

    for (int i = 0; i < STAT_MAX; i++)
        if (BaseStat[i] > HighestStat)
        {
            HighestStat = BaseStat[i];
            HighestStatType = i;
        }

    return HighestStatType;
}

int FindMonster(int TID)
{
    for (int i = 1; i < MonsterID; i++)
    {
        if (!Monsters[i].Init)
            continue;

        if (Monsters[i].TID == TID)
            return i;
    }

    return -1;
}

// For Debugging
NamedScript Console void OutputMonsters()
{
    for (int i = 1; i < MonsterID; i++)
    {
        //Log("\CdMonster %d:", i);
        //Output Monsters[i];
    }
}

NamedScript void MonsterRedAuraCheck(bool Enable)
{
    if (Enable)
    {
        SetActorFlag(0, "MISSILEMORE", GetCVar("drpg_monster_red_missilemore"));
        SetActorFlag(0, "MISSILEEVENMORE", GetCVar("drpg_monster_red_missileevenmore"));
        SetActorFlag(0, "NOFEAR", GetCVar("drpg_monster_red_nofear"));
    }
    else
    {
        SetActorFlag(0, "MISSILEMORE", false);
        SetActorFlag(0, "MISSILEEVENMORE", false);
        SetActorFlag(0, "NOFEAR", false);
    }
}

NamedScript void MonsterGreenAuraCheck(bool Enable)
{
    if (Enable)
    {
        SetActorFlag(0, "REFLECTIVE", GetCVar("drpg_monster_green_reflective"));
        SetActorFlag(0, "SHIELDREFLECT", GetCVar("drpg_monster_green_shieldreflect"));
        SetActorFlag(0, "NOPAIN", GetCVar("drpg_monster_green_nopain"));
        SetActorFlag(0, "NORADIUSDMG", GetCVar("drpg_monster_green_noradiusdmg"));
        SetActorFlag(0, "DONTBLAST", GetCVar("drpg_monster_green_dontblast"));
        SetActorFlag(0, "DONTRIP", GetCVar("drpg_monster_green_dontrip"));
    }
    else
    {
        SetActorFlag(0, "REFLECTIVE", false);
        SetActorFlag(0, "SHIELDREFLECT", false);
        SetActorFlag(0, "NOPAIN", false);
        SetActorFlag(0, "NORADIUSDMG", false);
        SetActorFlag(0, "DONTBLAST", false);
        SetActorFlag(0, "DONTRIP", false);
    }
}

NamedScript void MonsterWhiteAuraCheck(bool Enable)
{
    if (Enable)
    {
        SetActorFlag(0, "LOOKALLAROUND", GetCVar("drpg_monster_white_lookallaround"));
        SetActorFlag(0, "NOTARGETSWITCH", GetCVar("drpg_monster_white_notargetswitch"));
        SetActorFlag(0, "NOTARGET", GetCVar("drpg_monster_white_notarget"));
        SetActorFlag(0, "NOINFIGHTING", GetCVar("drpg_monster_white_noinfighting"));
        SetActorFlag(0, "BRIGHT", GetCVar("drpg_monster_white_bright"));
    }
    else
    {
        SetActorFlag(0, "LOOKALLAROUND", false);
        SetActorFlag(0, "NOTARGETSWITCH", false);
        SetActorFlag(0, "NOTARGET", false);
        SetActorFlag(0, "NOINFIGHTING", false);
        SetActorFlag(0, "BRIGHT", false);
    }
}

NamedScript void MonsterPinkAuraCheck(bool Enable)
{
    if (Enable)
    {
        SetActorFlag(0, "DONTDRAIN", GetCVar("drpg_monster_pink_dontdrain"));
    }
    else
    {
        SetActorFlag(0, "DONTDRAIN", false);
    }
}

NamedScript void MonsterBlueAuraCheck(bool Enable)
{
    if (Enable)
    {
        SetActorFlag(0, "NOTIMEFREEZE", GetCVar("drpg_monster_blue_notimefreeze"));
        SetActorFlag(0, "SEEINVISIBLE", GetCVar("drpg_monster_blue_seeinvisible"));
    }
    else
    {
        SetActorFlag(0, "NOTIMEFREEZE", false);
        SetActorFlag(0, "SEEINVISIBLE", false);
    }
}

NamedScript void MonsterOrangeAuraCheck(bool Enable)
{
    if (Enable)
    {
        SetActorFlag(0, "ALWAYSFAST", GetCVar("drpg_monster_orange_alwaysfast"));
        SetActorFlag(0, "QUICKTORETALIATE", GetCVar("drpg_monster_orange_quicktoretaliate"));
        SetActorFlag(0, "JUMPDOWN", GetCVar("drpg_monster_orange_jumpdown"));
        SetActorFlag(0, "NOTELESTOMP", GetCVar("drpg_monster_orange_notelestomp"));
    }
    else
    {
        SetActorFlag(0, "ALWAYSFAST", false);
        SetActorFlag(0, "QUICKTORETALIATE", false);
        SetActorFlag(0, "JUMPDOWN", false);
        SetActorFlag(0, "NOTELESTOMP", false);
    }
}

//NamedScript void MonsterPurpleAuraCheck(bool Enable)
//NamedScript void MonsterDarkBlueAuraCheck(bool Enable)
//NamedScript void MonsterYellowBlueAuraCheck(bool Enable)

NamedScript Console void MonsterDamaged(int SourceTID, int Damage)
{
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

    SetActivator(SourceTID);

    // Damaged by a player
    int PlayerNum = PlayerNumber();
    if (PlayerNum > -1)
    {
        Stats->DamageTable[PlayerNum] += Damage;
        if (GetCVar("drpg_combo_damage") || Player.Aura.Type[AURA_WHITE].Active && Player.Aura.Type[AURA_WHITE].Level >= 2)
        {
            if (Players(PlayerNum).Combo > 0)
                Players(PlayerNum).ComboTimer = COMBO_MAX;
        }
        return;
    }

    // Damaged by a player's turret
    for (int i = 0; i < MAX_PLAYERS; i++)
        if (ActivatorTID() == Players(i).Turret.TID)
        {
            Stats->DamageTable[i] += (Damage * GetCVar("drpg_xp_summon_percent")) / 100;
            return;
        }

    // Damaged by a player's summons
    if (ClassifyActor(0) & ACTOR_MONSTER && GetMonsterID(0) && GetActorProperty(0, APROP_MasterTID))
    {
        SetActivator(GetActorProperty(0, APROP_MasterTID));
        Stats->DamageTable[PlayerNumber()] += (Damage * GetCVar("drpg_xp_summon_percent")) / 100;
    }
}

// Calculate current map total monsters modifier
fixed MapTotalMonstersMod()
{
    fixed MapTotalMonstersMod = 1.0;
    int TotalMonsters = GetLevelInfo(LEVELINFO_TOTAL_MONSTERS);

    if (TotalMonsters > 256)
        MapTotalMonstersMod = 256.0 / TotalMonsters;
    if (MapTotalMonstersMod < 0.25)
        MapTotalMonstersMod = 0.25;

    return MapTotalMonstersMod;
}

// Calculate current map total bosses modifier
fixed MapTotalBossesMod()
{
    fixed MapTotalBossesMod = 1.0;

    if (TotalBosses > 2)
        MapTotalBossesMod = 2.0 / TotalBosses;
    if (MapTotalBossesMod < 0.25)
        MapTotalBossesMod = 0.25;

    return MapTotalBossesMod;
}

// Compatibility Handling - DoomRL Arsenal Extended
// Familiar Init Script
NamedScript DECORATE void FamiliarInit(int PlayerNum, int Slot)
{
    // Get a new ID for the familiar
    if (GetMonsterID(0) == 0)
        SetMonsterID(0, NewMonsterID());

    // Store TID
    if (ActivatorTID() == 0)
    {
        int TID = UniqueTID();
        Thing_ChangeTID(0, TID);
        Players(PlayerNum).FamiliarTID[Slot] = TID;
    }
    else
        Players(PlayerNum).FamiliarTID[Slot] = ActivatorTID();

    Players(PlayerNum).Familiars = true;
}

NamedScript DECORATE int GetMonsterHealthMax()
{
    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

    // Move script to the targeted monster
    SetActivatorToTarget(Player.TID);

    // Return value monster maximum health
    return Stats->HealthMax;
}

NamedScript DECORATE int GetMonsterLevel()
{
    // Pointer
    MonsterStatsPtr Stats = &Monsters[GetMonsterID(0)];

    // Move script to the targeted monster
    SetActivatorToTarget(Player.TID);

    // Return value monster level
    return Stats->Level;
}