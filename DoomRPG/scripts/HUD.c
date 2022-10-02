#include "Defs.h"

#include "HUD.h"
#include "ItemData.h"
#include "Map.h"
#include "Mission.h"
#include "RPG.h"
#include "Shield.h"
#include "Skills.h"
#include "Stats.h"
#include "Stims.h"
#include "Utils.h"

str StatusEffects[SE_MAX] =
{
    "\CcBlind",
    "\CfConfusion",
    "\CqPoison",
    "\CdCorrosion",
    "\CiFatigue",
    "\CtVirus",
    "\CnSilence",
    "\CgCurse",
    "\CvEMP",
    "\CdRadiation"
};

str StatusNumerals[6] =
{
    "",
    "I",
    "II",
    "III",
    "IV",
    "V"
};

NamedScript Type_ENTER void StatusEffectHUD()
{
    str const StatusColors[SE_MAX] =
    {
        "Gray",
        "Gold",
        "DarkGreen",
        "Green",
        "Orange",
        "Purple",
        "LightBlue",
        "Red",
        "Cyan",
        "Green"
    };

Start:
    NOP; // [KS] C doesn't allow declarations after labels, so we need this.

    if (Player.GUI.Open || (CompatMode == COMPAT_LEGENDOOM && CheckInventory("LDWeaponInfoScreenActive")))
    {
        Delay(1);
        goto Start;
    }

    fixed X = GetActivatorCVar("drpg_stateffect_x");
    fixed Y = GetActivatorCVar("drpg_stateffect_y");

    str Icon;
    str Fill;
    int TimerPercent;

    for (int i = 0; i < SE_MAX; i++)
    {
        Icon = "";
        Fill = "";
        TimerPercent = 0;

        switch (i)
        {
        case SE_BLIND:
            Icon = "SE_Blnd";
            Fill = "BarSBlnd";
            break;
        case SE_CONFUSION:
            Icon = "SE_Conf";
            Fill = "BarSConf";
            break;
        case SE_POISON:
            Icon = "SE_Pois";
            Fill = "BarSPois";
            break;
        case SE_CORROSION:
            Icon = "SE_Corr";
            Fill = "BarSCorr";
            break;
        case SE_FATIGUE:
            Icon = "SE_Fati";
            Fill = "BarSFati";
            break;
        case SE_VIRUS:
            Icon = "SE_Viru";
            Fill = "BarSViru";
            break;
        case SE_SILENCE:
            Icon = "SE_Sile";
            Fill = "BarSSile";
            break;
        case SE_CURSE:
            Icon = "SE_Curs";
            Fill = "BarSCurs";
            break;
        case SE_EMP:
            Icon = "SE_EMP";
            Fill = "BARSEmp";
            break;
        case SE_RADIATION:
            Icon = "SE_Radi";
            Fill = "BARSRadi";
            break;
        }

        if (Player.StatusType[i] || GetActivatorCVar("drpg_hud_preview"))
        {
            TimerPercent = (int)(((fixed)Player.StatusTimer[i] / (fixed)Player.StatusTimerMax[i]) * 100.0);
            if (TimerPercent > 100)
                TimerPercent = 100;

            // Preview
            if (GetActivatorCVar("drpg_hud_preview"))
                TimerPercent = 100;

            SetHudSize(GetActivatorCVar("drpg_hud_width"), GetActivatorCVar("drpg_hud_height"), false);

            // Name / Intensity
            SetFont("SMALLFONT");
            HudMessage("%S %S", StatusEffects[i], StatusNumerals[Player.StatusIntensity[i]]);
            EndHudMessage(HUDMSG_PLAIN | HUDMSG_ALPHA, 0, "White", X + 50.0, Y, 0.05, 0.75);

            // Time
            HudMessage("%S", FormatTime(Player.StatusTimer[i]));
            EndHudMessage(HUDMSG_PLAIN | HUDMSG_ALPHA, 0, StatusColors[i], X + 104.1, Y, 0.05, 0.75);

            // Bar
            SetHudClipRect((int)X, (int)(Y - 6), TimerPercent, (int)(Y + 6));
            PrintSpriteAlpha(Fill, 0, X + 0.1, Y, 0.05, 0.75);
            SetHudClipRect(0, 0, 0, 0);

            // Increment Y
            Y += 16.0;
        }
    }

    // Incapacitated message handling
    if (Timer() > 35 && GetCVar("drpg_multi_revives") && Player.ActualHealth <= 0)
    {
        SetHudSize(0, 0, false);
        SetFont("BIGFONT");
        if (SomePlayerAlive())
        {
            HudMessage("Incapacitated\nHealth: %d", Player.ActualHealth);
            EndHudMessage(HUDMSG_PLAIN, 0, "Brick", 1.5, 0.25, 0.05);
            if (Player.ReviveKeyTimer > 0)
            {
                int Percent = CalcPercent(Player.ReviveKeyTimer, 105);
                DrawProgressBar("Receiving treatment", Percent);
            }
        }
        else
        {
            HudMessage("YOU ARE DEAD");
            EndHudMessage(HUDMSG_PLAIN, 0, "Brick", 1.5, 0.5, 0.05);
        }
    }

    Delay(1);
    goto Start;
}

NamedScript Type_ENTER void OverviewHUD()
{
    str const CreditSprites[6] =
    {
        "CREDA0",
        "CREDB0",
        "CREDC0",
        "CREDD0",
        "CREDE0",
        "CREDF0"
    };

    // Interpolators
    InterpData Credits;
    InterpData Modules;
    InterpData Medkit;

    Credits.Value = CheckInventory("DRPGCredits");
    Credits.OldValue = Credits.Value;
    Credits.StartValue = Credits.Value;
    Credits.DisplayValue = Credits.Value;
    Credits.Timer = 0;
    Credits.TimerMax = 0;
    Credits.TimerMaxCap = 2;
    Modules.Value = CheckInventory("DRPGModule");
    Modules.OldValue = Modules.Value;
    Modules.StartValue = Modules.Value;
    Modules.DisplayValue = Modules.Value;
    Modules.Timer = 0;
    Modules.TimerMax = 0;
    Modules.TimerMaxCap = 2;
    Medkit.Value = Player.Medkit;
    Medkit.OldValue = Medkit.Value;
    Medkit.StartValue = Medkit.Value;
    Medkit.DisplayValue = Medkit.Value;
    Medkit.Timer = 0;
    Medkit.TimerMax = 0;
    Medkit.TimerMaxCap = 2;

    // Collection
    int CreditsCollected = 0;
    int CreditsCollectionTimer = 0;
    int ModulesCollected = 0;
    int ModulesCollectionTimer = 0;

    // Misc
    int CreditColor;

    // XP Bar
    str ClassName[11];
    bool XpBarAnim;
    int XPBarText;
    int XPBarType;
    int CurrentClass = PlayerClass(PlayerNumber());
    int XPPercentCurrent = Player.XPPercent;
    int XPPercentOld = Player.XPPercent;
    int XPIterations;

    // Rank Bar
    int RankBarText;
    int RankBarType;
    int RankPercentCurrent = Player.RankPercent;
    int RankPercentOld = Player.RankPercent;
    int RankIterations;

    // XP Bar Types
    str XpBarEmpty;
    str XpBarProgress;
    str XpBarProgressAnim;

    // Rank Bar Types
    str RankBarEmpty;
    str RankBarProgress;
    str RankBarProgressAnim;

    // XP Bars parameters
    fixed XpBarHeigh;
    fixed XpBarHalfLength;
    fixed XpProgressPercentModifier;
    fixed XpBarTextXOff;
    fixed XpBarTextYOff;

    // Rank Bars parameters
    fixed RankBarHeigh;
    fixed RankBarHalfLength;
    fixed RankProgressPercentModifier;
    fixed RankBarTextXOff;
    fixed RankBarTextYOff;

    // Compatibility Handling - DoomRL Arsenal
    if (CompatMode == COMPAT_DRLA)
    {
        ClassName[0] = "Marine";
        ClassName[1] = "Scout";
        ClassName[2] = "Technician";
        ClassName[3] = "Renegade";
        ClassName[4] = "Demolitionist";

        // Compatibility Handling - DoomRL Arsenal Extended
        if (CompatModeEx == COMPAT_DRLAX)
        {
            ClassName[6] = "Mechanoid";
            ClassName[7] = "Nomad";
            ClassName[8] = "Nano Maniac";
            ClassName[9] = "Phase Sisters";
            ClassName[10] = "Sarge";
        }
    }
    else
        ClassName[0] = "Doom Guy";

    // XP Bar Long Types/Color
    str XPBarShortThinEmpty[3] =
    {
        "XSTBarB1",
        "XSTBarR1",
        "XSTBarW1"
    };

    str XPBarShortThinProgress[3] =
    {
        "XSTBarB2",
        "XSTBarR2",
        "XSTBarW2"
    };

    str XPBarShortThinProgressAnim[3] =
    {
        "XSTBarB3",
        "XSTBarR3",
        "XSTBarW3"
    };

    str XPBarShortWideEmpty[3] =
    {
        "XSWBarB1",
        "XSWBarR1",
        "XSWBarW1"
    };

    str XPBarShortWideProgress[3] =
    {
        "XSWBarB2",
        "XSWBarR2",
        "XSWBarW2"
    };

    str XPBarShortWideProgressAnim[3] =
    {
        "XSWBarB3",
        "XSWBarR3",
        "XSWBarW3"
    };

    // XP Bar Long Types/Color
    str XPBarLongThinEmpty[3] =
    {
        "XLTBarB1",
        "XLTBarR1",
        "XLTBarW1"
    };

    str XPBarLongThinProgress[3] =
    {
        "XLTBarB2",
        "XLTBarR2",
        "XLTBarW2"
    };

    str XPBarLongThinProgressAnim[3] =
    {
        "XLTBarB3",
        "XLTBarR3",
        "XLTBarW3"
    };

    str XPBarLongWideEmpty[3] =
    {
        "XLWBarB1",
        "XLWBarR1",
        "XLWBarW1"
    };

    str XPBarLongWideProgress[3] =
    {
        "XLWBarB2",
        "XLWBarR2",
        "XLWBarW2"
    };

    str XPBarLongWideProgressAnim[3] =
    {
        "XLWBarB3",
        "XLWBarR3",
        "XLWBarW3"
    };

    // Rank Bar Types
    str RankBarThinEmpty[3] =
    {
        "RSTBar1",
        "RLTBar1"
    };

    str RankBarThinProgress[3] =
    {
        "RSTBar2",
        "RLTBar2"
    };

    str RankBarThinProgressAnim[3] =
    {
        "RSTBar3",
        "RLTBar3"
    };

    fixed X, X1, Y, Y1;

Start:
    NOP;

    // If we're on the title map, terminate
    if (InTitle) return;

    if (Player.GUI.Open || Player.InMenu || (CompatMode == COMPAT_LEGENDOOM && CheckInventory("LDWeaponInfoScreenActive")))
    {
        Delay(1);
        goto Start;
    }
    Credits.Value = CheckInventory("DRPGCredits");
    Modules.Value = CheckInventory("DRPGModule");
    Medkit.Value = Player.Medkit;
    CreditColor = (Timer() / (35 * 60)) % 6;

    X = GetActivatorCVar("drpg_credits_x");
    Y = GetActivatorCVar("drpg_credits_y");
    X1 = GetActivatorCVar("drpg_xp_bar_x");
    Y1 = GetActivatorCVar("drpg_xp_bar_y");

    SetHudSize(GetActivatorCVar("drpg_hud_width"), GetActivatorCVar("drpg_hud_height"), false);

    // Interpolation
    Interpolate(&Credits);
    Interpolate(&Modules);
    Interpolate(&Medkit);

    // Update the collection values
    if (Credits.Value != Credits.OldValue)
    {
        CreditsCollected += (Credits.Value - Credits.OldValue);
        CreditsCollectionTimer = 35 * 6;
    }
    if (Modules.Value != Modules.OldValue)
    {
        ModulesCollected += (Modules.Value - Modules.OldValue);
        ModulesCollectionTimer = 35 * 6;
    }

    // XP Bar
    if (GetActivatorCVar("drpg_xp_bar_enable") && !(Player.InMenu || Player.InShop || Player.InMinigame || Player.OutpostMenu > 0))
    {
        if (Timer() % 35 == 0)
        {
            // Set XP Bar Text/Type
            XPBarText = GetActivatorCVar("drpg_xp_bar_text");
            XPBarType = GetActivatorCVar("drpg_xp_bar_type");
            XpBarAnim = GetActivatorCVar("drpg_xp_bar_anim_enable");

            // Set Rank Bar Text
            RankBarText = GetActivatorCVar("drpg_rank_bar_text");

            // Set XP Bar Type/Color
            if (XPBarType == 0)
            {
                XpBarEmpty = XPBarShortThinEmpty[GetActivatorCVar("drpg_xp_bar_color")];
                XpBarProgress = XPBarShortThinProgress[GetActivatorCVar("drpg_xp_bar_color")];
                XpBarProgressAnim = XPBarShortThinProgressAnim[GetActivatorCVar("drpg_xp_bar_color")];
                XpBarHeigh = 4.0;
                XpBarHalfLength = 75.0;
                XpProgressPercentModifier = 1.5;
            }
            else if (XPBarType == 1)
            {
                XpBarEmpty = XPBarShortWideEmpty[GetActivatorCVar("drpg_xp_bar_color")];
                XpBarProgress = XPBarShortWideProgress[GetActivatorCVar("drpg_xp_bar_color")];
                XpBarProgressAnim = XPBarShortWideProgressAnim[GetActivatorCVar("drpg_xp_bar_color")];
                XpBarHeigh = 8.0;
                XpBarHalfLength = 75.0;
                XpProgressPercentModifier = 1.5;
            }
            else if (XPBarType == 2)
            {
                XpBarEmpty = XPBarLongThinEmpty[GetActivatorCVar("drpg_xp_bar_color")];
                XpBarProgress = XPBarLongThinProgress[GetActivatorCVar("drpg_xp_bar_color")];
                XpBarProgressAnim = XPBarLongThinProgressAnim[GetActivatorCVar("drpg_xp_bar_color")];
                XpBarHeigh = 4.0;
                XpBarHalfLength = 150.0;
                XpProgressPercentModifier = 3.0;
            }
            else if (XPBarType == 3)
            {
                XpBarEmpty = XPBarLongWideEmpty[GetActivatorCVar("drpg_xp_bar_color")];
                XpBarProgress = XPBarLongWideProgress[GetActivatorCVar("drpg_xp_bar_color")];
                XpBarProgressAnim = XPBarLongWideProgressAnim[GetActivatorCVar("drpg_xp_bar_color")];
                XpBarHeigh = 8.0;
                XpBarHalfLength = 150.0;
                XpProgressPercentModifier = 3.0;
            }

            // Set Text X offset for Bar Long Type
            if (XPBarType >= 2)
            {
                // If XP Bar position is equal to Rank Bar position
                if (RankBarText > 0 && ((XPBarText == RankBarText
                                         || (XPBarText + 1) == RankBarText || (XPBarText) == RankBarText + 1)))
                {
                    if (XPBarText == 1)
                        XpBarTextXOff = (XpBarHalfLength / 3) * (-1);
                    else if (XPBarText == 2)
                        XpBarTextXOff = 0;
                    else if (XPBarText == 3)
                        XpBarTextXOff = (XpBarHalfLength / 3);
                }
                else
                {
                    if (XPBarText == 1)
                        XpBarTextXOff = (XpBarHalfLength / 2) * (-1);
                    else if (XPBarText == 2)
                        XpBarTextXOff = 0;
                    else if (XPBarText == 3)
                        XpBarTextXOff = (XpBarHalfLength / 2);
                }
            }
            else
                XpBarTextXOff = 0;

            // Set Text Y offset for Bar Wide Type
            if (XPBarType == 1 || XPBarType == 3)
            {
                XpBarTextYOff = - 8.0;
            }
            else
                XpBarTextYOff = - 7.0;
        }

        // XP Bar Text set
        if (XPBarText > 0)
        {
            SetFont("SMALLFONT");

            // If XP Bar position is equal to Rank Bar position
            if (RankBarText > 0 && ((XPBarType >= 2 && (XPBarText == RankBarText
                                     || (XPBarText + 1) == RankBarText || (XPBarText) == RankBarText + 1))))
            {
                if (XPBarText == RankBarText || (XPBarText + 1) == RankBarText)
                    HudMessage("%d LVL %S %d RANK %S", Player.Level, ClassName[CurrentClass], Player.RankLevel, Ranks[Player.RankLevel]);
                else if (XPBarText == RankBarText + 1)
                    HudMessage("%d RANK %S %d LVL %S", Player.RankLevel, Ranks[Player.RankLevel], Player.Level, ClassName[CurrentClass]);
            }
            else
                HudMessage("%d LVL %S", Player.Level, ClassName[CurrentClass]);

            EndHudMessage(HUDMSG_PLAIN, 0, "White", X1 + XpBarTextXOff, Y1 + XpBarTextYOff, 0.05);
        }

        // Get XP Percent
        XPPercentCurrent = Player.XPPercent;
        if (XPPercentCurrent < 1 && Player.XP > 0)
            XPPercentCurrent = 1;

        if ((XPPercentCurrent == XPPercentOld && Player.XPGained <= 0) || !XpBarAnim)
        {
            SetHudClipRect(X1 - XpBarHalfLength, Y1 - (XpBarHeigh / 2.0), RoundInt(XPPercentCurrent * XpProgressPercentModifier), XpBarHeigh);
            PrintSprite(XpBarProgress, 0, X1, Y1, 0.05);
        }
        else
        {
            if (XPPercentCurrent > XPPercentOld)
            {
                SetHudClipRect(X1 - XpBarHalfLength, Y1 - (XpBarHeigh / 2.0), RoundInt(XPPercentOld * XpProgressPercentModifier), XpBarHeigh);
                PrintSprite(XpBarProgress, 0, X1, Y1, 0.05);
                SetHudClipRect(X1 + XPPercentOld * XpProgressPercentModifier - XpBarHalfLength, Y1 - (XpBarHeigh / 2.0), RoundInt((XPPercentCurrent - XPPercentOld) * XpProgressPercentModifier), XpBarHeigh);
                PrintSpriteAlpha(XpBarProgressAnim, 0, X1, Y1, 0.05, 0.75 + Sin((Timer()) / 32.0) * 0.25);
            }
            else if (XPPercentCurrent < XPPercentOld)
            {
                SetHudClipRect(X1 - XpBarHalfLength, Y1 - (XpBarHeigh / 2.0), RoundInt(XPPercentCurrent * XpProgressPercentModifier), XpBarHeigh);
                PrintSpriteAlpha(XpBarProgressAnim, 0, X1, Y1, 0.05, 0.75 + Sin((Timer()) / 32.0) * 0.25);
            }
            else if (Player.XPGained > 0)
            {
                SetHudClipRect(X1 - XpBarHalfLength, Y1 - (XpBarHeigh / 2.0), RoundInt(XPPercentOld * XpProgressPercentModifier) - 1, XpBarHeigh);
                PrintSprite(XpBarProgress, 0, X1, Y1, 0.05);
                SetHudClipRect(X1 + XPPercentOld * XpProgressPercentModifier - XpBarHalfLength - 1.0, Y1 - (XpBarHeigh / 2.0), RoundInt((XPPercentCurrent - XPPercentOld + 1) * XpProgressPercentModifier), XpBarHeigh);
                PrintSpriteAlpha(XpBarProgressAnim, 0, X1, Y1, 0.05, 0.75 + Sin((Timer()) / 32.0) * 0.25);
            }

            if (Player.XPGained > 0)
                XPIterations = 0;
            else
                XPIterations++;

            if (XPIterations > 35)
            {
                XPPercentOld = XPPercentCurrent;
                XPIterations = 0;
            }
        }

        SetHudClipRect(0, 0, 0, 0);
        PrintSprite(XpBarEmpty, 0, X1, Y1, 0.05);
    }

    // Rank Bar
    if (GetActivatorCVar("drpg_rank_bar_enable") && !(Player.InMenu || Player.InShop || Player.InMinigame || Player.OutpostMenu > 0))
    {
        if (Timer() % 35 == 0)
        {
            // Set Rank Bar Text/Type
            RankBarText = GetActivatorCVar("drpg_rank_bar_text");
            RankBarType = GetActivatorCVar("drpg_rank_bar_type");

            // Set XP Bar Text/Type
            XPBarText = GetActivatorCVar("drpg_xp_bar_text");
            XPBarType = GetActivatorCVar("drpg_xp_bar_type");
            XpBarAnim = GetActivatorCVar("drpg_xp_bar_anim_enable");

            // Set Rank Bar Type
            if (RankBarType == 0)
            {
                RankBarEmpty = RankBarThinEmpty[0];
                RankBarProgress = RankBarThinProgress[0];
                RankBarProgressAnim = RankBarThinProgressAnim[0];
                RankBarHeigh = 4.0;
                RankBarHalfLength = 75.0;
                RankProgressPercentModifier = 1.5;
            }
            else if (RankBarType == 1)
            {
                RankBarEmpty = RankBarThinEmpty[1];
                RankBarProgress = RankBarThinProgress[1];
                RankBarProgressAnim = RankBarThinProgressAnim[1];
                RankBarHeigh = 4.0;
                RankBarHalfLength = 150.0;
                RankProgressPercentModifier = 3.0;
            }

            // Set Text offset for Bar Long Type
            if (RankBarType == 1)
            {
                if (RankBarText == 1)
                    RankBarTextXOff = (RankBarHalfLength / 2) * (-1);
                else if (RankBarText == 2)
                    RankBarTextXOff = 0;
                else if (RankBarText == 3)
                    RankBarTextXOff = (RankBarHalfLength / 2);
            }
            else
                RankBarTextXOff = 0;

            // Set Text Y offset for Bar Long Type
            if (GetActivatorCVar("drpg_xp_bar_enable") && XPBarText > 0 && XPBarType >= 2)
            {
                if (XPBarType == 3)
                    RankBarTextYOff = - 14.0;
                else
                    RankBarTextYOff = - 11.0;
            }
            else
                RankBarTextYOff = - 7.0;
        }

        // Set Rank Bar Y offset
        if (XPBarType == 1 || XPBarType == 3)
            Y1 += 6.0;
        else
            Y1 += 4.0;

        // Rank Bar Text set
        if (RankBarText > 0 && (!GetActivatorCVar("drpg_xp_bar_enable") || (GetActivatorCVar("drpg_xp_bar_enable") && XPBarText > 0 && XPBarType >= 2
                                && XPBarText != RankBarText && (XPBarText + 1) != RankBarText) && (XPBarText) != RankBarText + 1))
        {
            SetFont("SMALLFONT");
            HudMessage("%d RANK %S", Player.RankLevel, Ranks[Player.RankLevel]);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X1 + RankBarTextXOff, Y1 + RankBarTextYOff, 0.05);
        }

        // Get Rank Percent
        RankPercentCurrent = Player.RankPercent;
        if (RankPercentCurrent < 1 && Player.Rank > 0)
            RankPercentCurrent = 1;

        if ((RankPercentCurrent == RankPercentOld && Player.RankGained <= 0) || !XpBarAnim)
        {
            SetHudClipRect(X1 - RankBarHalfLength, Y1 - (RankBarHeigh / 2.0), RoundInt(RankPercentCurrent * RankProgressPercentModifier), RankBarHeigh);
            PrintSprite(RankBarProgress, 0, X1, Y1, 0.05);
        }
        else
        {
            if (RankPercentCurrent > RankPercentOld)
            {
                SetHudClipRect(X1 - RankBarHalfLength, Y1 - (RankBarHeigh / 2.0), RoundInt(RankPercentOld * RankProgressPercentModifier), RankBarHeigh);
                PrintSprite(RankBarProgress, 0, X1, Y1, 0.05);
                SetHudClipRect(X1 + RankPercentOld * RankProgressPercentModifier - RankBarHalfLength, Y1 - (RankBarHeigh / 2.0), RoundInt((RankPercentCurrent - RankPercentOld) * RankProgressPercentModifier), RankBarHeigh);
                PrintSpriteAlpha(RankBarProgressAnim, 0, X1, Y1, 0.05, 0.75 + Sin((Timer()) / 32.0) * 0.25);
            }
            else if (RankPercentCurrent < RankPercentOld)
            {
                SetHudClipRect(X1 - RankBarHalfLength, Y1 - (RankBarHeigh / 2.0), RoundInt(RankPercentCurrent * RankProgressPercentModifier), RankBarHeigh);
                PrintSpriteAlpha(RankBarProgressAnim, 0, X1, Y1, 0.05, 0.75 + Sin((Timer()) / 32.0) * 0.25);
            }
            else if (Player.RankGained > 0)
            {
                SetHudClipRect(X1 - RankBarHalfLength, Y1 - (RankBarHeigh / 2.0), RoundInt(RankPercentOld * RankProgressPercentModifier) - 1, RankBarHeigh);
                PrintSprite(RankBarProgress, 0, X1, Y1, 0.05);
                SetHudClipRect(X1 + RankPercentOld * RankProgressPercentModifier - RankBarHalfLength - 1.0, Y1 - (RankBarHeigh / 2.0), RoundInt((RankPercentCurrent - RankPercentOld + 1) * RankProgressPercentModifier), RankBarHeigh);
                PrintSpriteAlpha(RankBarProgressAnim, 0, X1, Y1, 0.05, 0.75 + Sin((Timer()) / 32.0) * 0.25);
            }

            if (Player.RankGained > 0)
                RankIterations = 0;
            else
                RankIterations++;

            if (RankIterations > 35)
            {
                RankPercentOld = RankPercentCurrent;
                RankIterations = 0;
            }
        }

        SetHudClipRect(0, 0, 0, 0);
        PrintSprite(RankBarEmpty, 0, X1, Y1, 0.05);
    }

    // Credits
    PrintSprite(CreditSprites[CreditColor], 0, X, Y + 12.0, 0.05);
    SetFont("BIGFONT");
    HudMessage("%ld", Credits.DisplayValue);
    EndHudMessage(HUDMSG_PLAIN, 0, "Gold", X + 16.1, Y, 0.05);
    if (CreditsCollected != 0)
    {
        HudMessage("%+d", CreditsCollected);
        EndHudMessage(HUDMSG_FADEOUT, PAY_ID, (CreditsCollected > 0 ? "White" : "Red"), X + 16.1, Y + 12.0, 0.05, 2.0, 1.0);
    }

    // Modules
    PrintSprite("UMODA0", 0, X - 4.0, Y + 56.0, 0.05);
    SetFont("BIGFONT");
    HudMessage("%ld", Modules.DisplayValue);
    EndHudMessage(HUDMSG_PLAIN, 0, "Green", X + 16.1, Y + 24.0, 0.05);
    if (ModulesCollected != 0)
    {
        HudMessage("%+d", ModulesCollected);
        EndHudMessage(HUDMSG_FADEOUT, PAY_ID + 1, (ModulesCollected > 0 ? "DarkGreen" : "DarkRed"), X + 16.1, Y + 36.0, 0.05, 2.0, 1.0);
    }

    // Medkit
    PrintSprite("MEDKA0", 0, X, Y + 80.0, 0.05);
    SetFont("BIGFONT");
    HudMessage("%ld", Medkit.DisplayValue);
    if (Medkit.DisplayValue >= Player.MedkitMax)
        EndHudMessage(HUDMSG_PLAIN, 0, "BrickPink", X + 16.1, Y + 56.0, 0.05);
    else if (Medkit.DisplayValue == 0)
        EndHudMessage(HUDMSG_PLAIN, 0, "Red", X + 16.1, Y + 56.0, 0.05);
    else
        EndHudMessage(HUDMSG_PLAIN, 0, "Brick", X + 16.1, Y + 56.0, 0.05);

    // Collection timer handling
    if (CreditsCollectionTimer > 0)
        CreditsCollectionTimer--;
    else
        CreditsCollected = 0;
    if (ModulesCollectionTimer > 0)
        ModulesCollectionTimer--;
    else
        ModulesCollected = 0;

    Credits.OldValue = CheckInventory("DRPGCredits");
    Modules.OldValue = CheckInventory("DRPGModule");
    Medkit.OldValue = Player.Medkit;
    Delay(1);

    goto Start;
}

NamedScript Type_ENTER void ComboHUD()
{
    int TimerPercent;

    // Interpolators
    InterpData Combo;
    InterpData XP;
    InterpData Rank;
    InterpData Bonus;

    Combo.Value = 0;
    Combo.OldValue = 0;
    Combo.StartValue = 0;
    Combo.DisplayValue = 0;
    Combo.Timer = 0;
    Combo.TimerMax = 0;
    Combo.TimerMaxCap = 1;

    XP.Value = 0;
    XP.OldValue = 0;
    XP.StartValue = 0;
    XP.DisplayValue = 0;
    XP.Timer = 0;
    XP.TimerMax = 0;
    XP.TimerMaxCap = 1;

    Rank.Value = 0;
    Rank.OldValue = 0;
    Rank.StartValue = 0;
    Rank.DisplayValue = 0;
    Rank.Timer = 0;
    Rank.TimerMax = 0;
    Rank.TimerMaxCap = 1;

    Bonus.Value = 0;
    Bonus.OldValue = 0;
    Bonus.StartValue = 0;
    Bonus.DisplayValue = 0;
    Bonus.Timer = 0;
    Bonus.TimerMax = 0;
    Bonus.TimerMaxCap = 1;

    fixed X, Y;

Start:
    NOP;

    // If we're on the title map, terminate
    if (InTitle) return;

    if (Player.GUI.Open || (CompatMode == COMPAT_LEGENDOOM && CheckInventory("LDWeaponInfoScreenActive")))
    {
        Delay(1);
        goto Start;
    }

    X = GetActivatorCVar("drpg_combo_x");
    Y = GetActivatorCVar("drpg_combo_y");

    SetHudSize(GetActivatorCVar("drpg_hud_width"), GetActivatorCVar("drpg_hud_height"), false);

    Combo.Value = Player.Combo;
    XP.Value = Player.XPGained;
    Rank.Value = Player.RankGained;
    Bonus.Value = Player.BonusGained;

    // Interpolation
    Interpolate(&Combo);
    Interpolate(&XP);
    Interpolate(&Rank);
    Interpolate(&Bonus);

    if (Player.Combo > 0 && Player.ComboTimer <= COMBO_MAX)
        TimerPercent = (int)(((fixed)Player.ComboTimer / (fixed)COMBO_MAX) * 100.0);
    else if (GetActivatorCVar("drpg_hud_preview"))
        TimerPercent = 100;
    else
        TimerPercent = 0;
    if (TimerPercent > 100)
        TimerPercent = 100;

    // Bar
    if (TimerPercent > 0)
    {
        DrawBar("CBar1", X, Y, (TimerPercent > 50 ? 50 : TimerPercent), true);
        DrawBar("CBar2", X + 50, Y, TimerPercent - 50, true);
    }

    // Combo Info
    SetFont("BIGFONT");
    if (Combo.DisplayValue > 0 && Combo.DisplayValue <= (GetLevelInfo(LEVELINFO_TOTAL_MONSTERS) * 2 + 100) || GetActivatorCVar("drpg_hud_preview"))
    {
        HudMessage("%ld", Combo.DisplayValue);
        EndHudMessage(HUDMSG_PLAIN, 0, "Purple", X + 0.1, Y + 10.0, 0.05);
    }
    if (XP.DisplayValue != 0 && Combo.DisplayValue <= (GetLevelInfo(LEVELINFO_TOTAL_MONSTERS) * 2 + 100) || GetActivatorCVar("drpg_hud_preview"))
    {
        if (XP.DisplayValue >= 10000l || Bonus.DisplayValue >= 1000l)
        {
            if (XP.DisplayValue >= 1000l)
                HudMessage("%ld k.", XP.DisplayValue / 1000l);
            else
                HudMessage("< 1 k.");
        }
        else
            HudMessage("%ld", XP.DisplayValue);

        EndHudMessage(HUDMSG_PLAIN, 0, (XP.DisplayValue >= 0 ? "White" : "Gray"), X + 0.1, Y + 22.0, 0.05);
    }
    if (Rank.DisplayValue != 0 && Combo.DisplayValue <= (GetLevelInfo(LEVELINFO_TOTAL_MONSTERS) * 2 + 100) || GetActivatorCVar("drpg_hud_preview"))
    {
        if (XP.DisplayValue >= 10000l || Bonus.DisplayValue >= 1000l)
        {
            if (Rank.DisplayValue >= 1000l)
                HudMessage("%ld k.", Rank.DisplayValue / 1000l);
            else
                HudMessage("< 1 k.");
        }
        else
            HudMessage("%ld", Rank.DisplayValue);

        EndHudMessage(HUDMSG_PLAIN, 0, (Rank.DisplayValue >= 0 ? "Yellow" : "DarkBrown"), X + 0.1, Y + 34.0, 0.05);
    }
    if (Bonus.DisplayValue != 0 && Combo.DisplayValue <= (GetLevelInfo(LEVELINFO_TOTAL_MONSTERS) * 2 + 100) || GetActivatorCVar("drpg_hud_preview"))
    {
        if (XP.DisplayValue >= 10000l || Bonus.DisplayValue >= 1000l)
        {
            if (Bonus.DisplayValue >= 1000l)
                HudMessage("%ld k.", Bonus.DisplayValue / 1000l);
            else
                HudMessage("< 1 k.");
        }
        else
            HudMessage("%ld", Bonus.DisplayValue);

        EndHudMessage(HUDMSG_PLAIN, 0, (Bonus.DisplayValue >= 0 ? "Green" : "DarkGreen"), X + 0.1, Y + 46.0, 0.05);
    }

    Combo.OldValue = Player.Combo;
    XP.OldValue = Player.XPGained;
    Rank.OldValue = Player.RankGained;
    Bonus.OldValue = Player.BonusGained;
    Delay(1);

    goto Start;
}

NamedScript Type_ENTER void SkillHUD()
{
    fixed X, Y;
    int Cost;
    str Color;

Start:
    NOP;

    if (Player.GUI.Open || (CompatMode == COMPAT_LEGENDOOM && CheckInventory("LDWeaponInfoScreenActive")))
    {
        Delay(1);
        goto Start;
    }

    X = GetActivatorCVar("drpg_skill_x");
    Y = GetActivatorCVar("drpg_skill_y");

    SetHudSize(GetActivatorCVar("drpg_hud_width"), GetActivatorCVar("drpg_hud_height"), false);

    if (Timer() > 4 && !(GetActivatorCVar("drpg_menu_hideskills") && (Player.InMenu || Player.InShop || Player.OutpostMenu > 0)))
    {
        // Current Skill
        if (Player.SkillCategory[Player.SkillSelected] != -1 && Player.SkillIndex[Player.SkillSelected] != -1)
        {
            SkillPtr CurrentSkill = &Skills[Player.SkillCategory[Player.SkillSelected]][Player.SkillIndex[Player.SkillSelected]];
            SkillLevelInfo *SkillLevel = &Player.SkillLevel[Player.SkillCategory[Player.SkillSelected]][Player.SkillIndex[Player.SkillSelected]];

            Cost = ScaleEPCost(CurrentSkill->Cost * SkillLevel->CurrentLevel);
            Color = "LightBlue";

            if (Player.EP < Cost)
                Color = "Red";

            // Cost
            SetFont("SMALLFONT");
            HudMessage("%d", Cost);
            EndHudMessage(HUDMSG_PLAIN, 0, Color, X, Y - 16.0, 0.05);

            // Level
            HudMessage("%d/%d", SkillLevel->CurrentLevel, SkillLevel->Level);
            EndHudMessage(HUDMSG_PLAIN, 0, "Green", X, Y + 16.0, 0.05);

            // Icon
            PrintSprite(CurrentSkill->Icon, 0, X, Y, 0.05);
        }
    }

    Delay(1);
    goto Start;
}

NamedScript Type_ENTER void StimHUD()
{
    fixed X, Y;
    int TimerPercent;

Start:
    NOP;

    if (Player.GUI.Open || (CompatMode == COMPAT_LEGENDOOM && CheckInventory("LDWeaponInfoScreenActive")))
    {
        Delay(1);
        goto Start;
    }

    X = GetActivatorCVar("drpg_stim_x");
    Y = GetActivatorCVar("drpg_stim_y");

    SetHudSize(GetActivatorCVar("drpg_hud_width"), GetActivatorCVar("drpg_hud_height"), false);

    // Stat Boosts
    if (Player.Stim.Timer > 0 || GetActivatorCVar("drpg_hud_preview"))
    {
        TimerPercent = (int)(((fixed)Player.Stim.Timer / (fixed)Player.Stim.TimerMax) * 100.0);
        if (TimerPercent > 100)
            TimerPercent = 100;

        // Preview
        if (GetActivatorCVar("drpg_hud_preview"))
            TimerPercent = 100;

        // Time Bar
        DrawBar("Stim10", X, Y, TimerPercent, true);

        // Time
        SetFont("BIGFONT");
        HudMessage("%S", FormatTime(Player.Stim.Timer));
        EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 110.1, Y, 0.05);

        // Icons
        for (int i = 0; i < StimStatsEnd; i++)
            if (Player.Stim.ActiveBonus[i] || GetActivatorCVar("drpg_hud_preview"))
            {
                SetFont("BIGFONT");
                HudMessage("+%d", Player.Stim.Last[i]);
                EndHudMessage(HUDMSG_PLAIN, 0, CompoundColors[i], X + 8.0 + ((i % 4) * 36.0), Y + 20.0 + ((i / 4) * 24.0), 0.05);
                PrintSpritePulse(StrParam("STAT%dS", i + 1), 0, X + 20.0 + ((i % 4) * 36.0), Y + 40.0 + ((i / 4) * 24.0), 0.75, 32.0, 0.25);
            }
    }

    // Powerups
    for (int i = StimPowerupStart; i < StimPowerupEnd; i++)
        if ((Player.Stim.ActiveBonus[i] && Player.Stim.PowerupTimer[i] > 0) || GetActivatorCVar("drpg_hud_preview"))
        {
            TimerPercent = (int)(((fixed)Player.Stim.PowerupTimer[i] / (fixed)Player.Stim.PowerupTimerMax[i]) * 100.0);
            if (TimerPercent > 100)
                TimerPercent = 100;

            // Preview
            if (GetActivatorCVar("drpg_hud_preview"))
                TimerPercent = 100;

            SetFont("SMALLFONT");
            HudMessage("%S", FormatTime(Player.Stim.PowerupTimer[i]));
            EndHudMessage(HUDMSG_PLAIN, 0, CompoundColors[i], X + 112.1, Y + 64.0, 0.05);
            DrawBar(StrParam("Stim%d", i + 1), X, Y + 64.0, TimerPercent, true);

            Y += 8.0;
        }

    Delay(1);
    goto Start;
}

NamedScript Type_ENTER void MissionHUD()
{
    int OldAmount;
    fixed X, Y;

Start:
    NOP;

    if (Player.GUI.Open || (CompatMode == COMPAT_LEGENDOOM && CheckInventory("LDWeaponInfoScreenActive")))
    {
        Delay(1);
        goto Start;
    }

    OldAmount = Player.Mission.Current;
    X = GetActivatorCVar("drpg_mission_x");
    Y = GetActivatorCVar("drpg_mission_y");

    Delay(1);

    if (Player.Mission.Active && Player.Mission.Current != OldAmount || GetActivatorCVar("drpg_hud_preview") || GetActivatorCVar("drpg_notifications_preview"))
    {
        SetHudSize(GetActivatorCVar("drpg_hud_width"), GetActivatorCVar("drpg_hud_height"), false);
        SetFont("BIGFONT");

        if (GetActivatorCVar("drpg_hud_preview") || GetActivatorCVar("drpg_notifications_preview")) // Preview
        {
            // Mission's Notifications
            if (GetActivatorCVar("drpg_notifications_preview"))
            {
                if (GetActivatorCVar("drpg_notifications_detailed"))
                    HudMessage("Mission Complete!\n\n\Cj+%d XP\n\Ck+%d Rank\n\Cf+%d Credits\n\Cd+%d Modules\n\n\CiItem: \Cj%S",
                               0, 0, 0, 0, "Reward");
                else
                    HudMessage("Mission Complete!");
                EndHudMessage(HUDMSG_FADEOUT, MISSION_ID + 2, "Green", GetActivatorCVar("drpg_mission_complete_x") + 0.4, GetActivatorCVar("drpg_mission_complete_y"), 3.0, 2.0);
            }

            // Mission's HUD
            if (GetActivatorCVar("drpg_hud_preview"))
            {
                HudMessage("0 / 0");
                EndHudMessage(HUDMSG_FADEOUT, MISSION_ID, "Green", X + 0.1, Y, 2.0, 1.0);
                PrintSpriteFade("PISTA0", MISSION_ID + 1, X + 11.0 - 40.0 + 0.4, Y + 15.0 + 0.4, 2.0, 1.0);
            }
        }
        else
        {
            switch (Player.Mission.Type)
            {
            case MT_COLLECT:
                HudMessage("%d / %d", Player.Mission.Current, Player.Mission.Amount);
                EndHudMessage(HUDMSG_FADEOUT, MISSION_ID, "Green", X + 0.1, Y, 2.0, 1.0);
                PrintSpriteFade(Player.Mission.Item->Sprite.Name, MISSION_ID + 1, X + Player.Mission.Item->Sprite.XOff - 40.0 + 0.4, Y + Player.Mission.Item->Sprite.YOff + 0.4, 2.0, 1.0);
                break;
            case MT_KILL:
                HudMessage("\Cg%S\n\C-%d / %d", Player.Mission.Monster->Name, Player.Mission.Current, Player.Mission.Amount);
                EndHudMessage(HUDMSG_FADEOUT, MISSION_ID, "Brick", X + 0.4, Y, 2.0, 1.0);
                break;
            case MT_KILLAURAS:
                HudMessage("Auras: %d / %d", Player.Mission.Current, Player.Mission.Amount);
                EndHudMessage(HUDMSG_FADEOUT, MISSION_ID, "Brick", X, Y, 2.0, 1.0);
                break;
            case MT_REINFORCEMENTS:
                HudMessage("Reinforcements: %d / %d", Player.Mission.Current, Player.Mission.Amount);
                EndHudMessage(HUDMSG_FADEOUT, MISSION_ID, "Brick", X, Y, 2.0, 1.0);
                break;
            case MT_SECRETS:
                HudMessage("Secrets: %d / %d", Player.Mission.Current, Player.Mission.Amount);
                EndHudMessage(HUDMSG_FADEOUT, MISSION_ID, "Yellow", X, Y, 2.0, 1.0);
                break;
            case MT_ITEMS:
                HudMessage("Items: %d / %d", Player.Mission.Current, Player.Mission.Amount);
                EndHudMessage(HUDMSG_FADEOUT, MISSION_ID, "LightBlue", X, Y, 2.0, 1.0);
                break;
            case MT_COMBO:
                HudMessage("Combo: %d / %d", Player.Mission.Current, Player.Mission.Amount);
                EndHudMessage(HUDMSG_FADEOUT, MISSION_ID, "Purple", X, Y, 2.0, 1.0);
                break;
            }
        }
    }

    goto Start;
}

NamedScript Type_ENTER void AuraTimerHUD()
{
    fixed Offset, X, Y, Angle, XOff, YOff;
    int AuraCount, AuraAdd, Radius;

Start:
    NOP;

    if (Player.GUI.Open || (CompatMode == COMPAT_LEGENDOOM && CheckInventory("LDWeaponInfoScreenActive")))
    {
        Delay(1);
        goto Start;
    }

    Offset = 0;

    while (PlayerHasAura(PlayerNumber()) || GetActivatorCVar("drpg_hud_preview"))
    {
        X = GetActivatorCVar("drpg_auratimer_x");
        Y = GetActivatorCVar("drpg_auratimer_y");
        AuraCount = 0;
        AuraAdd = 0;
        Radius = 16;

        SetHudSize(GetActivatorCVar("drpg_hud_width"), GetActivatorCVar("drpg_hud_height"), false);

        // Calculate number of active Auras
        for (int i = 0; i < AURA_MAX; i++)
            if (Player.Aura.Type[i].Active)
                AuraCount++;

        // Preview
        if (GetActivatorCVar("drpg_hud_preview"))
            AuraCount = AURA_MAX;

        // Timer
        if (Player.Aura.Time > 0 || GetActivatorCVar("drpg_hud_preview"))
        {
            SetFont("BIGFONT");
            HudMessage("%S", FormatTime(Player.Aura.Time));
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
        }

        // Main Icon
        if (Player.Aura.Time > 0 && PlayerHasShadowAura(PlayerNumber()))
            PrintSpritePulse("AuraBlac", 0, X, Y + 32.0, 0.75, 64.0, 0.25);
        else if (Player.Aura.Time > 0)
            PrintSpritePulse("Aura", 0, X, Y, 0.75, 64.0, 0.25);

        // Team Aura Icon
        if (Player.Aura.Team)
            PrintSpritePulse("SAURK0", 0, X + 18.0, Y + 24.0, 0.75, 64.0, 0.25);

        // Orbiting Icons
        for (int i = 0; i < AURA_MAX; i++)
            if (Player.Aura.Type[i].Active || GetActivatorCVar("drpg_hud_preview"))
            {
                Angle = -0.25 + ((1.0 / AuraCount) * AuraAdd++) + Offset;
                XOff = X + (Radius * Cos(Angle));
                YOff = Y + (Radius * Sin(Angle)) + 32.0;
                PrintSpriteAlpha(AuraIcons[i], 0, (int)XOff, (int)YOff, 0.05, 0.75);
            }

        // Offset
        Offset += 0.0025;

        Delay(1);
    }

    Delay(1);
    goto Start;
}

NamedScript Type_ENTER void PowerupHUD()
{
    fixed BaseX, BaseY, X, Y;
    int GridCount, InvulnTime, InvisTime, ShadowTime, GhostTime, ActualInvisTime, Lives;
    int FreezeTime, LightAmpPowerupTime, LightAmpTime, IronFeetPowerTime, IronFeetTime;
    bool HaveIronFeet;

Start:
    NOP;

    if (Player.GUI.Open || (CompatMode == COMPAT_LEGENDOOM && CheckInventory("LDWeaponInfoScreenActive")))
    {
        Delay(1);
        goto Start;
    }

    BaseX = GetActivatorCVar("drpg_powerup_x");
    BaseY = GetActivatorCVar("drpg_powerup_y");
    X = BaseX;
    Y = BaseY;
    GridCount = 0;

    SetHudSize(GetActivatorCVar("drpg_hud_width"), GetActivatorCVar("drpg_hud_height"), false);

    // Invulnerability
    InvulnTime = GetActorPowerupTics(0, "PowerInvulnerable");
    if (InvulnTime > 0 || GetActivatorCVar("drpg_hud_preview"))
    {
        SetFont("SMALLFONT");
        HudMessage("%S", FormatTime(InvulnTime));
        EndHudMessage(HUDMSG_PLAIN, 0, "Green", X, Y, 0.05);
        PrintSpritePulse("PINVA0", 0, X + 10.0, Y + 22.0, 0.75, 32.0, 0.25);
        X += 36.0;
        GridCount++;
        if ((GridCount % 4) == 0)
        {
            X = BaseX;
            Y += 32.0;
        }
    }

    // Invisibility
    InvisTime = GetActorPowerupTics(0, "PowerInvisibility");
    ShadowTime = GetActorPowerupTics(0, "PowerShadow");
    GhostTime = GetActorPowerupTics(0, "PowerGhost");
    if (InvisTime > 0 || ShadowTime > 0 || GhostTime > 0 || GetActivatorCVar("drpg_hud_preview"))
    {
        ActualInvisTime = 0;
        if (InvisTime > 0)
            ActualInvisTime = InvisTime;
        else if (ShadowTime > 0)
            ActualInvisTime = ShadowTime;
        else if (GhostTime > 0)
            ActualInvisTime = GhostTime;

        SetFont("SMALLFONT");
        HudMessage("%S", FormatTime(ActualInvisTime));
        EndHudMessage(HUDMSG_PLAIN, 0, "LightBlue", X, Y, 0.05);
        PrintSpritePulse("PINSA0", 0, X + 10.0, Y + 38.0, 0.75, 32.0, 0.25);
        X += 36.0;
        GridCount++;
        if ((GridCount % 4) == 0)
        {
            X = BaseX;
            Y += 32.0;
        }
    }

    // Time Freeze
    FreezeTime = GetActorPowerupTics(0, "PowerTimeFreezer");
    if (FreezeTime > 0 || GetActivatorCVar("drpg_hud_preview"))
    {
        SetFont("SMALLFONT");
        HudMessage("%S", FormatTime(FreezeTime));
        EndHudMessage(HUDMSG_PLAIN, 0, "Green", X, Y, 0.05);
        PrintSpritePulse("TIMEA0", 0, X + 17.0, Y + 44.0, 0.75, 32.0, 0.25);
        X += 36.0;
        GridCount++;
        if ((GridCount % 4) == 0)
        {
            X = BaseX;
            Y += 32.0;
        }
    }

    // Regeneration Sphere
    if (Player.RegenBoostTimer > 0 || GetActivatorCVar("drpg_hud_preview"))
    {
        SetFont("SMALLFONT");
        HudMessage("%S", FormatTime(Player.RegenBoostTimer));
        EndHudMessage(HUDMSG_PLAIN, 0, "Purple", X, Y, 0.05);
        PrintSpritePulse("REGNA0", 0, X + 12.0, Y + 37.0, 0.75, 32.0, 0.25);
        X += 36.0;
        GridCount++;
        if ((GridCount % 4) == 0)
        {
            X = BaseX;
            Y += 32.0;
        }
    }

    // Light Amp
    LightAmpPowerupTime = GetActorPowerupTics(0, "PowerLightAmp");
    LightAmpTime = LightAmpPowerupTime;
    if (LightAmpTime > 0 || GetActivatorCVar("drpg_hud_preview"))
    {
        SetFont("SMALLFONT");
        HudMessage("%S", FormatTime(LightAmpTime));
        EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y, 0.05);
        PrintSpritePulse("PVISA0", 0, X + 13.0, Y + 8.0, 0.75, 32.0, 0.25);
        X += 36.0;
        GridCount++;
        if ((GridCount % 4) == 0)
        {
            X = BaseX;
            Y += 32.0;
        }
    }

    // Computer Map/Scanner
    if (CheckInventory("PowerScanner") || GetActivatorCVar("drpg_hud_preview"))
    {
        PrintSpritePulse("PMAPA0", 0, X + 14.0, Y + 22.0, 0.75, 32.0, 0.25);
        X += 36.0;
        GridCount++;
        if ((GridCount % 4) == 0)
        {
            X = BaseX;
            Y += 32.0;
        }
    }

    // Berserk
    if (CheckInventory("PowerStrength") || GetActivatorCVar("drpg_hud_preview"))
    {
        PrintSpritePulse("PSTRA0", 0, X + 12.0, Y + 13.0, 0.75, 32.0, 0.25);
        X += 36.0;
        GridCount++;
        if ((GridCount % 4) == 0)
        {
            X = BaseX;
            Y += 32.0;
        }
    }

    // Wings
    if (CheckInventory("PowerFlight") || GetActivatorCVar("drpg_hud_preview"))
    {
        PrintSpritePulse("WINGA0", 0, X + 13.0, Y + 35.0, 0.75, 32.0, 0.25);
        X += 36.0;
        GridCount++;
        if ((GridCount % 4) == 0)
        {
            X = BaseX;
            Y += 32.0;
        }
    }

    // Iron Feet
    HaveIronFeet = false;
    IronFeetPowerTime = GetActorPowerupTics(0, "PowerIronFeet");
    IronFeetTime = IronFeetPowerTime;
    if (IronFeetTime > 0 || GetActivatorCVar("drpg_hud_preview"))
    {
        if (GridCount > 0)
        {
            X = BaseX - 36.0;
            Y = BaseY;
        }
        SetFont("SMALLFONT");
        HudMessage("%S", FormatTime(IronFeetTime));
        EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y + 16.0, 0.05);
        PrintSpritePulse("SUITA0", 0, X + 10.0, Y + 66.0, 0.75, 32.0, 0.25);
        HaveIronFeet = true;
    }

    // 1-Ups
    str LivesSprite;
    str LivesTextColour;
    Lives = CheckInventory("DRPGLife");
    if (Lives > 0 || GetActivatorCVar("drpg_hud_preview"))
    {
        if (HaveIronFeet)
            X -= 40.0;
        else if (GridCount > 0)
            X = BaseX - 40.0;

        if (Lives < 3)
        {
            LivesSprite = "P1UBA0";
            LivesTextColour = "Green";
        }
        else if (Lives >= 3 && Lives < 5)
        {
            LivesSprite = "P3UBA0";
            LivesTextColour = "Blue";
        }
        else if (Lives >= 5)
        {
            LivesSprite = "P5UBA0";
            LivesTextColour = "Red";
        }

        SetFont("BIGFONT");
        HudMessage("%d", Lives);
        EndHudMessage(HUDMSG_PLAIN, 0, LivesTextColour, X, Y + 24.0, 0.05);
        PrintSpritePulse(LivesSprite, 0, X + 14.0, Y + 88.0, 0.75, 32.0, 0.25);
    }

    Delay(1);
    goto Start;
}

NamedScript Type_ENTER void EventHUD()
{
    str const KeySprites[MAX_NUKE_KEYS] =
    {
        "NKEYA0",
        "NKEYB0",
        "NKEYC0",
        "NKEYD0",
        "NKEYE0",
        "NKEYF0",
        "NKEYG0",
        "NKEYH0",
        "NKEYI0"
    };

    fixed X, Y;
    int KeyOffset;
    str GeneratorStatus;

Start:
    NOP;

    if (Player.GUI.Open || (CompatMode == COMPAT_LEGENDOOM && CheckInventory("LDWeaponInfoScreenActive")))
    {
        Delay(1);
        goto Start;
    }

    X = GetActivatorCVar("drpg_event_x");
    Y = GetActivatorCVar("drpg_event_y");

    SetHudSize(GetActivatorCVar("drpg_hud_width"), GetActivatorCVar("drpg_hud_height"), false);

    // Thermonuclear Bomb
    if (CurrentLevel->Event == MAPEVENT_NUCLEARBOMB && CurrentLevel->BombTime > 0 && !CurrentLevel->EventCompleted)
    {
        SetFont("BIGFONT");
        HudMessage("\CgT - %S", FormatTime(CurrentLevel->BombTime));
        EndHudMessage(HUDMSG_PLAIN, NUKE_ID, "Red", X + 0.1, Y + 0.1, 0.05);

        KeyOffset = 0;
        for (int i = 0; i < MAX_NUKE_KEYS; i++)
        {
            // Skip drawing this key if it's disabled
            if (!CurrentLevel->BombKeyActive[i]) continue;

            // Timer
            SetFont("BIGFONT");
            HudMessage("%S", FormatTime(CurrentLevel->BombKeyTimer[i]));
            EndHudMessage(HUDMSG_PLAIN, 0, (CurrentLevel->BombKeyDisarming[i] ? "Red" : "White"), X + (KeyOffset % 3 * 48.0), Y + 48.0 + (KeyOffset / 3 * 48.0), 0.05);

            // Key
            for (int j = 0; j < MAX_PLAYERS; j++)
                if (CheckActorInventory(Players(j).TID, StrParam("DRPGNukeKey%d", i + 1)))
                    PrintSprite(StrParam("%S", KeySprites[i]), 0, X - 12.0 + (KeyOffset % 3 * 48.0), Y + 52.0 + (KeyOffset / 3 * 48.0), 0.05);

            // Icon
            PrintSprite(StrParam("E_NKEY%d", i + 1), 0, X + (KeyOffset % 3 * 48.0), Y + 34.0 + (KeyOffset / 3 * 48.0), 0.05);

            KeyOffset++;
        }
    }

    // Low Power
    if (CurrentLevel->Event == MAPEVENT_LOWPOWER && !CurrentLevel->PowerGeneratorActive)
    {
        // Draw if someone has a working power cell
        for (int i = 0; i < MAX_PLAYERS; i++)
            if (CheckActorInventory(Players(i).TID, "DRPGGeneratorCell") > 0)
                PrintSprite("GCELA0", 0, X + 8.1, Y + 33.1, 0.05);
    }

    // Environmental Hazard
    if (CurrentLevel->Event == MAPEVENT_TOXICHAZARD && !CurrentLevel->EventCompleted)
    {
        GeneratorStatus = "\CgStopped\C-";
        if (CurrentLevel->GeneratorFuel > 0)
            GeneratorStatus = "\CdRunning\C-";
        SetFont("BIGFONT");
        HudMessage("\CdHazard Level %d", CurrentLevel->HazardLevel);
        EndHudMessage(HUDMSG_PLAIN, 0, "Red", X + 0.1, Y + 0.1, 0.05);

        PrintSprite("RADMA0", 0, X + 33.1, Y + 56.1, 0.05);
        SetFont("SMALLFONT");
        HudMessage("\CqToxicity: \Cd%d%%\n\CcGenerator status: %S \Ck(%S left)\n\CqSpare fuel tanks: \Cd%d", ((CurrentLevel->HazardLevel - 1) * 100) + CurrentLevel->RadLeft, GeneratorStatus, FormatTime(CurrentLevel->GeneratorFuel), CheckInventory("DRPGNeutralizerFuel"));
        EndHudMessage(HUDMSG_PLAIN, 0, "Red", X + 71.1, Y + 16.1, 0.05);
    }

    // Hell Unleashed
    if ((CurrentLevel->Event == MAPEVENT_HELLUNLEASHED && CurrentLevel->HellUnleashedActive >= 2) || GetActivatorCVar("drpg_hud_preview"))
    {
        PrintSpritePulse("MonLevel", 0, X + 0.1, Y, 0.75, 32.0, 0.25);
        PrintSpritePulse("STAT8S", 0, X + 8.1, Y + 38.0, 0.75, 32.0, 0.25);

        SetFont("BIGFONT");
        HudMessage("+%.2k", CurrentLevel->LevelAdd);
        EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 24.1, Y, 0.05);
        HudMessage("+%.2k%%", CurrentLevel->RareAdd);
        EndHudMessage(HUDMSG_PLAIN, 0, "Gold", X + 24.1, Y + 18.0, 0.05);
    }

    // Doomsday
    if (CurrentLevel->Event == MAPEVENT_DOOMSDAY)
    {
        SetFont("BIGFONT");
        HudMessage("\CgDEATH IN %S", FormatTime(CurrentLevel->DoomTime));
        EndHudMessage(HUDMSG_PLAIN, NUKE_ID, "Red", X + 0.1, Y + 0.1, 0.05);
    }


    Delay(1);
    goto Start;
}

NamedScript Type_ENTER void CoopViewHUD()
{
    fixed X, Y;

Start:
    NOP;

    if (Player.GUI.Open || (CompatMode == COMPAT_LEGENDOOM && CheckInventory("LDWeaponInfoScreenActive")))
    {
        Delay(1);
        goto Start;
    }

    while (Player.PlayerView != PlayerNumber())
    {
        X = GetActivatorCVar("drpg_coopview_x");
        Y = GetActivatorCVar("drpg_coopview_y");

        SetHudSize(GetActivatorCVar("drpg_hud_width"), GetActivatorCVar("drpg_hud_height"), false);

        if (!Player.InMenu && !Player.InShop && !Player.OutpostMenu && !Player.InMinigame)
        {
            SetFont("BIGFONT");
            HudMessage("%tS", PlayerNumber() + 1);
            EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 0.1, Y + 8.0, 0.05);
            PrintSprite(StrParam("P%dSVIEW", Player.PlayerView + 1), 0, X + 0.1, Y + 0.1, 0.05);
        }

        Delay(1);
    }
}

NamedScript Type_ENTER void MultiplayerHUD()
{
    //fixed X, Y, Alpha;
    //int HealthPercent, ShieldPercent;

Start:
    NOP;

    if (InMultiplayer && GetCVar("drpg_multiplayer_hud") && Players(0).HealthMax > 0)
    {
        str data = "";

        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (PlayerInGame(i))
                data = StrParam("%S%i|%i|%i|%i|", data, Players(i).ActualHealth, Players(i).HealthMax, Players(i).Shield.Charge, Players(i).Shield.Capacity);
            else
                data = StrParam("%S0|0|0|0|", data);
        }

        SetCVarString("drpg_multi_data", data);
    }
    /*
    if (Player.GUI.Open || (CompatMode == COMPAT_LEGENDOOM && CheckInventory("LDWeaponInfoScreenActive")))
    {
        Delay(1);
        goto Start;
    }

    while ((InMultiplayer && GetActivatorCVar("drpg_multiplayer_hud")) || GetActivatorCVar("drpg_hud_preview"))
    {
        X = GetActivatorCVar("drpg_multiplayer_x");
        Y = GetActivatorCVar("drpg_multiplayer_y");

        SetHudSize(GetActivatorCVar("drpg_hud_width"), GetActivatorCVar("drpg_hud_height"), false);

        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            // Skip this player if they aren't in-game
            if (!PlayerInGame(i)) continue;

            HealthPercent = (int)(((fixed)Players(i).ActualHealth / (fixed)Players(i).HealthMax) * 100.0);
            if (HealthPercent > 100)
                HealthPercent = 100;
            ShieldPercent = (int)(((fixed)Players(i).Shield.Charge / (fixed)Players(i).Shield.Capacity) * 100.0);
            if (ShieldPercent > 100)
                ShieldPercent = 100;
            Alpha = 1.0;

            // Health Critical
            if (HealthPercent <= 10)
            {
                Alpha = 0.75 + (Sin((fixed)Timer() / 32.0) * 0.25);
                PrintSpritePulse("HLTHCRIT", 0, X - 8.0, Y + 5.0, 0.75, 32.0, 0.25);
            }

            SetFont("SMALLFONT");

            if (Players(i).Shield.Active && Players(i).Shield.Charge > 0)
            {
                HudMessage("%tS\C- (\Cv%d/%d\C-)", i + 1, Players(i).Shield.Charge, Players(i).Shield.Capacity);
                EndHudMessage(HUDMSG_PLAIN | HUDMSG_ALPHA, 0, "White", X + 0.1, Y, 0.05, Alpha);
                DrawBar("MPFill2", X, Y + 8.0, ShieldPercent, true);
            }
            else
            {
                HudMessage("%tS\C- (\Ca%d/%d\C-)", i + 1, Players(i).ActualHealth, Players(i).HealthMax);
                EndHudMessage(HUDMSG_PLAIN | HUDMSG_ALPHA, 0, "White", X + 0.1, Y, 0.05, Alpha);
            }

            DrawBar("MPFill", X, Y + 8.0, HealthPercent, true);

            Y += 16.0;
        }

        Delay(1);
    }
    */
    Delay(1);
    goto Start;
}

NamedScript Type_ENTER void TurretHUD()
{
    str const AmmoColors[5] =
    {
        "Brick",
        "Orange",
        "Gray",
        "LightBlue",
        "Green"
    };

    // Interpolators
    InterpData Health;

    Health.Value = 0;
    Health.OldValue = 0;
    Health.StartValue = 0;
    Health.DisplayValue = 0;
    Health.Timer = 0;
    Health.TimerMax = 0;
    Health.TimerMaxCap = 2;

    fixed X, Y;
    str AmmoIcon;
    int Ammo[TW_MAX];

Start:
    NOP;

    if (Player.GUI.Open || (CompatMode == COMPAT_LEGENDOOM && CheckInventory("LDWeaponInfoScreenActive")))
    {
        Delay(1);
        goto Start;
    }

    while (Player.Turret.Active || Player.Turret.Maintenance || GetActivatorCVar("drpg_hud_preview"))
    {
        X = GetActivatorCVar("drpg_turret_x");
        Y = GetActivatorCVar("drpg_turret_y");

        Ammo[TW_BULLET] = Player.Turret.BulletAmmo;
        Ammo[TW_PELLET] = Player.Turret.ShellAmmo;
        Ammo[TW_ROCKET] = Player.Turret.RocketAmmo;
        Ammo[TW_PLASMA] = Player.Turret.PlasmaAmmo;
        Ammo[TW_RAILGUN] = Player.Turret.RailAmmo;

        AmmoIcon = "";

        SetHudSize(GetActivatorCVar("drpg_hud_width"), GetActivatorCVar("drpg_hud_height"), false);

        Health.Value = Player.Turret.Health;

        // Interpolation
        Interpolate(&Health);

        // Determine Ammo Icon
        switch (Player.Turret.Weapon)
        {
        case TW_BULLET:
            AmmoIcon = "CLIPA0";
            break;
        case TW_PELLET:
            AmmoIcon = "SHELA0";
            break;
        case TW_ROCKET:
            AmmoIcon = "ROCKA0";
            break;
        case TW_PLASMA:
        case TW_RAILGUN:
            AmmoIcon = "CELLA0";
            break;
        }

        if (Player.Turret.Maintenance)
        {
            // Icon
            PrintSprite("TMaint", 0, X + -16.1, Y - 4.0 + (int)(Sin((fixed)Timer() / 128.0) * 8.0), 0.05);

            // Timers
            SetFont("BIGFONT");
            if (Player.Turret.RepairTimer > 0)
            {
                HudMessage("%S", FormatTime(Player.Turret.RepairTimer * 35));
                EndHudMessage(HUDMSG_PLAIN, 0, (Player.Turret.PaidForRepair ? "Brick" : "Red"), X + 24.1, Y - 16.0, 0.05);
            }
            if (Player.Turret.ChargeTimer > 0)
            {
                HudMessage("%S", FormatTime(Player.Turret.ChargeTimer * 35));
                EndHudMessage(HUDMSG_PLAIN, 0, "Yellow", X + 24.1, Y, 0.05);
            }
            if (Player.Turret.RefitTimer > 0)
            {
                HudMessage("%S", FormatTime(Player.Turret.RefitTimer * 35));
                EndHudMessage(HUDMSG_PLAIN, 0, "LightBlue", X + 24.1, Y + 16.0, 0.05);
            }
        }
        else if (!Player.Turret.Maintenance || GetActivatorCVar("drpg_hud_preview"))
        {
            // Icons
            PrintSprite("PTURA3A7", 0, X + 0.1, Y + (int)(Sin((fixed)Timer() / 64.0) * 8.0), 0.05);
            if (Player.Turret.Weapon > 0)
                PrintSprite(AmmoIcon, 0, X + 0.1, Y + 24.0, 0.05);

            // Health, Battery, Ammo
            SetFont("BIGFONT");
            HudMessage("%ld", Health.DisplayValue);
            EndHudMessage(HUDMSG_PLAIN, 0, "Red", X + 24.1, Y - 16.0, 0.05);
            if (!Player.Turret.AugBattery)
            {
                HudMessage("%S", FormatTime(Player.Turret.Battery * 35));
                EndHudMessage(HUDMSG_PLAIN, 0, "Yellow", X + 24.1, Y, 0.05);
            }
            if (Player.Turret.Weapon != TW_NONE)
            {
                HudMessage("%d", Ammo[Player.Turret.Weapon]);
                EndHudMessage(HUDMSG_PLAIN, 0, AmmoColors[Player.Turret.Weapon - 1], X + 24.1, Y + 16.0, 0.05);
            }
        }

        Health.OldValue = Player.Turret.Health;
        Delay(1);
    }

    Delay(1);
    goto Start;
}

NamedScript Type_ENTER void StatHUD()
{
    int *Stats[STAT_MAX] =
    {
        &Player.StrengthTotal,
        &Player.DefenseTotal,
        &Player.VitalityTotal,
        &Player.EnergyTotal,
        &Player.RegenerationTotal,
        &Player.AgilityTotal,
        &Player.CapacityTotal,
        &Player.LuckTotal
    };

    int PrevStats[STAT_MAX] =
    {
        Player.StrengthTotal,
        Player.DefenseTotal,
        Player.VitalityTotal,
        Player.EnergyTotal,
        Player.RegenerationTotal,
        Player.AgilityTotal,
        Player.CapacityTotal,
        Player.LuckTotal
    };

    bool Change[STAT_MAX];
    fixed X, Y;

Start:
    NOP;

    if (Player.GUI.Open || (CompatMode == COMPAT_LEGENDOOM && CheckInventory("LDWeaponInfoScreenActive")))
    {
        Delay(1);
        goto Start;
    }

    for (int i = 0; i < STAT_MAX; i++)
        Change[i] = false;

    // If we're on the title map, terminate
    if (InTitle) return;

    X = GetActivatorCVar("drpg_stats_x");
    Y = GetActivatorCVar("drpg_stats_y");

    SetHudSize(GetActivatorCVar("drpg_hud_width"), GetActivatorCVar("drpg_hud_height"), false);

    // Determine if there was a change in your stats
    for (int i = 0; i < STAT_MAX; i++)
        if (*Stats[i] != PrevStats[i])
            Change[i] = true;

    for (int i = 0; i < STAT_MAX; i++)
        if (Change[i] || GetActivatorCVar("drpg_stats_alwaysshow") || GetActivatorCVar("drpg_hud_preview"))
        {
            SetFont("BIGFONT");
            HudMessage("%d", *Stats[i]);
            EndHudMessage(HUDMSG_FADEOUT, STAT_ID + i, StatColors[i], X + 9.0 + ((i % 4) * 36.0), Y + ((i / 4) * 24.0), 2.0, 3.0);
            PrintSpriteFade(StrParam("STAT%dS", i + 1), STAT_ID + STAT_MAX + i, X + 20.0 + ((i % 4) * 36.0), Y + 20.0 + ((i / 4) * 24.0), 2.0, 3.0);
        }

    // Store stats for comparison next tic
    for (int i = 0; i < STAT_MAX; i++)
        PrevStats[i] = *Stats[i];

    Delay(1);
    goto Start;
}

NamedScript void DamageHUD(int Amount, bool Critical)
{
    if (Player.GUI.Open || (CompatMode == COMPAT_LEGENDOOM && CheckInventory("LDWeaponInfoScreenActive")))
        return;

    // Return if the CVAR is disabled
    if (!GetActivatorCVar("drpg_damagenumbers_hud")) return;

    str Color = "White";
    str Text = "";
    fixed Time = 1.0;

    // Size
    if (Amount == 1)
        SetFont("SMALLFONT");
    else
        SetFont("BIGFONT");

    // Color
    switch (Player.DamageType)
    {
    case DT_NORMAL:
        Color = "White";
        break;
    case DT_TOXIC:
    case DT_RADIATION:
        Color = "Green";
        break;
    case DT_MELEE:
        Color = "Gray";
        break;
    case DT_FIRE:
        Color = "Red";
        break;
    case DT_PLASMA:
    case DT_LIGHTNING:
        Color = "LightBlue";
        break;
    }

    if (Amount > 0 && Amount < MAX_HEALTH - Player.HealthMax - Amount)
    {
        // Fatal Blow
        if (!Player.Shield.Active && Player.ActualHealth - Amount <= 0)
        {
            SetFont("BIGFONT");
            Color = "DarkRed";
            Text = StrParam("%S\CrFATAL\n", Text);
            Time += 4.0;
        }

        // Critical Hit
        if (Critical)
        {
            SetFont("BIGFONT");
            Text = StrParam("%S\CgCRITICAL\n", Text);
            Time += 1.0;
        }

        // Status Effect
        if (Player.StatusTypeHUD >= 0)
        {
            SetFont("BIGFONT");
            Text = StrParam("%S%S %S\n", Text, StatusEffects[Player.StatusTypeHUD], StatusNumerals[Player.StatusIntensity[Player.StatusTypeHUD]]);
            Time += 1.0;
            Player.StatusTypeHUD = -1;
        }

        // Loss
        Text = StrParam("%S%d", Text, Amount);

        // Display
        HudMessage("%S", Text);
        EndHudMessage(HUDMSG_FADEOUT, 0, Color, 1.4 + RandomFixed(0.0, 0.4), 0.6 + RandomFixed(0.0, 0.3), 0.5, Time);
    }
}

NamedScript Type_ENTER void DRLAHUD()
{
    str const RaritySuffix[6] =
    {
        " \Ct[Exotic]\C-",
        " \Ci[Superior]\C-",
        " \Cv[Assembled]\C-",
        " \Cd[Unique]\C-",
        " \Cg[Demonic]\C-",
        " \Cf[Legendary]\C-"
        // " \Ct[Pony]\C-"
    };

    if (CompatMode != COMPAT_DRLA)
        return;

    fixed Offset, X, Y, Angle, XOff, YOff;
    bool IsTechnician, Duel;
    int Weapons, Armors, ModPacks, Skulls, Devices, TotalMax, Count, Add, ModAdd, Radius;
    int Total[2] = {0, 0};
    int Power[2] = {0, 0};
    int Bulk[2] = {0, 0};
    int Agility[2] = {0, 0};
    int Tech[2] = {0, 0};
    int Sniper[2] = {0, 0};
    int Firestorm[2] = {0, 0};
    int Nano[2] = {0, 0};
    int DemonArtifacts[2] = {0, 0};
    ItemInfoPtr ItemPtr, JackelItem, CasullItem;
    int Mods[5];
    str const DukeModIcons[5] =
    {
        "DN2PAICO",
        "DN2PBICO",
        "DN2PCICO",
        "DN2PDICO",
        "DN2PEICO"
    };

    str Name, Color;
    str BarGraphic;
    fixed Alpha;
    int Mode = 2;

Start:
    NOP;

    // If we're on the title map, terminate
    if (InTitle) return;

    Mode = GetCVar("drpg_drla_hud_mode");

    if (Player.GUI.Open || Player.InMenu || Player.InShop || Player.InMinigame || Player.OutpostMenu || Mode == 0)
    {
        Delay(1);
        goto Start;
    }

    // Offset = 0.0;

    X = GetActivatorCVar("drpg_drla_x");
    Y = GetActivatorCVar("drpg_drla_y");

    if (Mode > 1)
    {
        IsTechnician = (PlayerClass(PlayerNumber()) == 2);
        Weapons = CheckInventory("RLWeaponLimit");
        Armors = CheckInventory("RLArmorInInventory");
        ModPacks = (IsTechnician ? CheckInventory("RLScavengerModLimit") : CheckInventory("RLModLimit"));
        Skulls = CheckInventory("RLSkullLimit");
        Devices = CheckInventory("RLPhaseDeviceLimit");

        // Compatibility Handling - DoomRL Arsenal Extended
        // Phase Sisters - Change the Weapons calculation
        if (CompatModeEx == COMPAT_DRLAX && PlayerClass(PlayerNumber()) == 9)
            Weapons -= 2;

        SetHudSize(GetActivatorCVar("drpg_hud_width"), GetActivatorCVar("drpg_hud_height"), false);

        // Counters
        if (!(GetCVar("drpg_debug_drla") & DDM_NOLIMITS))
        {
            fixed XOff = X - 54.0; // +27.0 offset

            if (Devices > 0 || GetCVar("drpg_hud_preview"))
            {
                SetFont("BIGFONT");
                if (Devices >= DRLA_DEVICE_MAX)
                {
                    HudMessage("%d", Devices);
                    EndHudMessage(HUDMSG_ALPHA, 0, "Gold", XOff, Y + 12.0, 0.05, 0.75 + (Sin((fixed)Timer() / 32.0) * 0.25));
                }
                else
                {
                    HudMessage("%d", Devices);
                    EndHudMessage(HUDMSG_PLAIN, 0, "White", XOff, Y + 12.0, 0.05);
                }
                PrintSprite("PHS1I0", 0, XOff - 7.0, Y, 0.05);
                XOff -= 27.0;
            }
            if (Skulls > 0 || GetCVar("drpg_hud_preview"))
            {
                SetFont("BIGFONT");
                if (Skulls >= DRLA_SKULL_MAX)
                {
                    HudMessage("%d", Skulls);
                    EndHudMessage(HUDMSG_ALPHA, 0, "Gold", XOff, Y + 12.0, 0.05, 0.75 + (Sin((fixed)Timer() / 32.0) * 0.25));
                }
                else
                {
                    HudMessage("%d", Skulls);
                    EndHudMessage(HUDMSG_PLAIN, 0, "White", XOff, Y + 12.0, 0.05);
                }
                PrintSprite("ISKLC0", 0, XOff + 11.0, Y + 16.0, 0.05);
                XOff -= 27.0;
            }
            if (ModPacks > 0 || GetCVar("drpg_hud_preview"))
            {
                SetFont("BIGFONT");
                if (ModPacks >= DRLA_MODPACKS_MAX)
                {
                    HudMessage("%d", ModPacks);
                    EndHudMessage(HUDMSG_ALPHA, 0, "Gold", XOff, Y + 12.0, 0.05, 0.75 + (Sin((fixed)Timer() / 32.0) * 0.25));
                }
                else
                {
                    HudMessage("%d", ModPacks);
                    EndHudMessage(HUDMSG_PLAIN, 0, "White", XOff, Y + 12.0, 0.05);
                }
                PrintSprite("GMODICON", 0, XOff + 1.0, Y, 0.05);
                XOff -= 27.0;
            }
            if (Armors > 0 || GetCVar("drpg_hud_preview"))
            {
                SetFont("BIGFONT");
                if (Armors >= DRLA_ARMOR_MAX)
                {
                    HudMessage("%d", Armors);
                    EndHudMessage(HUDMSG_ALPHA, 0, "Gold", XOff, Y + 12.0, 0.05, 0.75 + (Sin((fixed)Timer() / 32.0) * 0.25));
                }
                else
                {
                    HudMessage("%d", Armors);
                    EndHudMessage(HUDMSG_PLAIN, 0, "White", XOff, Y + 12.0, 0.05);
                }
                PrintSprite("HARMOR", 0, XOff + 1.0, Y + 4.0, 0.05);
                XOff -= 27.0;
            }
            if (Weapons > 0 || GetCVar("drpg_hud_preview"))
            {
                SetFont("BIGFONT");
                if (Weapons >= DRLA_WEAPON_MAX)
                {
                    HudMessage("%d", Weapons);
                    EndHudMessage(HUDMSG_ALPHA, 0, "Gold", XOff, Y + 12.0, 0.05, 0.75 + (Sin((fixed)Timer() / 32.0) * 0.25));
                }
                else
                {
                    HudMessage("%d", Weapons);
                    EndHudMessage(HUDMSG_PLAIN, 0, "White", XOff, Y + 12.0, 0.05);
                }
                PrintSprite("PISGX0", 0, XOff + 11.0, Y + 12.0, 0.05);
            }
        }
    }

    if (Mode > 0)
    {
        for (int i = 0; i < ItemMax[0]; i++)
        {
            ItemPtr = &ItemData[0][i];

            if (CheckWeapon(ItemPtr->Actor))
            {
                Duel = false;
                Name = StrParam("%S", ItemPtr->Name);
                Color = "";
                TotalMax = 0;
                Total[0] = 0, Total[1] = 0;
                if (ItemPtr->CompatMods & RL_MOD_LIMIT)
                    Total[0] = CheckInventory(StrParam("%SModLimit", ItemPtr->Actor));
                Power[0] = 0, Power[1] = 0;
                if (ItemPtr->CompatMods & RL_POWER_MOD)
                    Power[0] = CheckInventory(StrParam("%SPowerMod", ItemPtr->Actor));
                Bulk[0] = 0, Bulk[1] = 0;
                if (ItemPtr->CompatMods & RL_BULK_MOD)
                    Bulk[0] = CheckInventory(StrParam("%SBulkMod", ItemPtr->Actor));
                Agility[0] = 0, Agility[1] = 0;
                if (ItemPtr->CompatMods & RL_AGILITY_MOD)
                    Agility[0] = CheckInventory(StrParam("%SAgilityMod", ItemPtr->Actor));
                Tech[0] = 0, Tech[1] = 0;
                if (ItemPtr->CompatMods & RL_TECH_MOD)
                    Tech[0] = CheckInventory(StrParam("%STechnicalMod", ItemPtr->Actor));
                Sniper[0] = 0, Sniper[1] = 0;
                if (ItemPtr->CompatMods & RL_SNIPER_MOD)
                    Sniper[0] = CheckInventory(StrParam("%SSniperMod", ItemPtr->Actor));
                Firestorm[0] = 0, Firestorm[1] = 0;
                if (ItemPtr->CompatMods & RL_FIREST_MOD)
                    Firestorm[0] = CheckInventory(StrParam("%SFirestormMod", ItemPtr->Actor));
                Nano[0] = 0, Nano[1] = 0;
                if (ItemPtr->CompatMods & RL_NANO_MOD)
                    Nano[0] = CheckInventory(StrParam("%SNanoMod", ItemPtr->Actor));
                DemonArtifacts[0] = 0, DemonArtifacts[1] = 0;
                if (ItemPtr->CompatMods & RL_DEMON_MOD)
                    DemonArtifacts[0] = CheckInventory(StrParam("%SDemonArtifacts", ItemPtr->Actor));

                // Determine total modpacks and the color char to use
                if (CheckInventory("RLStandardWeaponToken"))
                {
                    Color = "\Cj";
                    TotalMax = 4;
                }
                else if (CheckInventory("RLExoticWeaponToken"))
                {
                    Name = StrLeft(Name, StrLen(Name) - StrLen(RaritySuffix[0]));
                    Color = "\Ct";
                    TotalMax = 4;
                }
                else if (CheckInventory("RLSuperiorWeaponToken"))
                {
                    Name = StrLeft(Name, StrLen(Name) - StrLen(RaritySuffix[1]));
                    Color = "\Ci";
                    TotalMax = 2;
                }
                else if (CheckInventory("RLAssembledWeaponToken"))
                {
                    Name = StrLeft(Name, StrLen(Name) - StrLen(RaritySuffix[2]));
                    Color = "\Cv";
                    TotalMax = 2;
                }
                else if (CheckInventory("RLUniqueWeaponToken"))
                {
                    Name = StrLeft(Name, StrLen(Name) - StrLen(RaritySuffix[3]));
                    Color = "\Cd";
                    TotalMax = 1;
                }
                else if (CheckInventory("RLDemonicWeaponToken"))
                {
                    Name = StrLeft(Name, StrLen(Name) - StrLen(RaritySuffix[4]));
                    Color = "\Cg";
                    TotalMax = 1;
                }
                else if (CheckInventory("RLLegendaryWeaponToken"))
                {
                    Name = StrLeft(Name, StrLen(Name) - StrLen(RaritySuffix[5]));
                    Color = "\Cf";
                    TotalMax = 1;
                }
                else break; // Kinda lolhax

                // Synthfire/Duel-wielded weapons
                if (CheckWeapon("RLAntiFreakJackal") && CheckInventory("RLAntiFreakJackalDemonArtifacts")) // Jackal/Casull
                {
                    Duel = true;
                    Name = "\CdAnti-Freak Jackal & Hellsing ARMS Casull";
                    Total[1] = CheckInventory("RLHellsingARMSCasullModLimit");
                    Power[1] = CheckInventory("RLHellsingARMSCasullPowerMod");
                    Bulk[1] = CheckInventory("RLHellsingARMSCasullBulkMod");
                    Agility[1] = CheckInventory("RLHellsingARMSCasullAgilityMod");
                    Tech[1] = CheckInventory("RLHellsingARMSCasullTechnicalMod");
                    Sniper[1] = CheckInventory("RLHellsingARMSCasullSniperMod");
                    Firestorm[1] = CheckInventory("RLHellsingARMSCasullFirestormMod");
                    Nano[1] = CheckInventory("RLHellsingARMSCasullNanoMod");
                }
                else if (CheckWeapon("RLMarathonShotguns") && CheckInventory("RLMarathonShotgunsDemonArtifacts")) // Twin WSTE-M5's
                {
                    Duel = true;
                    Name = "\CdTwin WSTE-M5 Shotguns";
                }
                else if (CheckWeapon("RLUzi") && CheckInventory("RLUziDemonArtifacts")) // Duel Uzis
                    Duel = true;

                // Name
                SetFont("RLFONT");
                HudMessage("%S%S", Color, Name);
                EndHudMessage(HUDMSG_PLAIN, 0, "White", X, Y - 20.0, 0.05);

                // Duke 2 Rifle special icon fancyness
                if (CheckWeapon("RLDuke2Rifle"))
                {
                    Mods[0] = 1; // You always have the default shot
                    Mods[1] = Sniper[0];
                    Mods[2] = Firestorm[0];
                    Mods[3] = Nano[0];
                    Mods[4] = (GetActorPowerupTics(0, "PowerRLDuke2RifleRapidFire") > 0);

                    Count = 0;
                    ModAdd = 0;
                    Radius = 24;

                    // Count the mods
                    for (int i = 0; i < 5; i++)
                        if (Mods[i])
                            Count++;

                    // Draw the mods
                    for (int i = 0; i < 5; i++)
                        if (Mods[i])
                        {
                            Angle = -0.25 + ((1.0 / Count) * ModAdd++) + Offset;
                            XOff = X + (Radius * Cos(Angle));
                            YOff = Y + (Radius * Sin(Angle));
                            PrintSprite(DukeModIcons[i], 0, (int)XOff + ItemPtr->Sprite.XOff - 12.0, (int)YOff + ItemPtr->Sprite.YOff + 8.0, 0.05);
                        }

                    // Offset
                    Offset += 0.005;
                }
                else // Everything else
                {
                    // Demonic Weapons special artifact icons and animation
                    if (CheckInventory("RLDemonicWeaponToken"))
                    {
                        Count = DemonArtifacts[0] + DemonArtifacts[1];
                        Add = 0;
                        Radius = 24 + (int)(Sin((fixed)Offset / 64.0) * 4.0);

                        // Draw the mods
                        for (int i = 0; i < Count; i++)
                        {
                            Angle = -0.25 + ((1.0 / Count) * Add++) + Offset;
                            XOff = X + (Radius * Cos(Angle));
                            YOff = Y + (Radius * Sin(Angle));
                            PrintSpritePulse("DMNAA0", 0, (int)XOff + 16.0, (int)YOff + 44.0, 0.5, 256.0, 0.25);
                        }

                        // Offset
                        Offset += 0.005;
                    }

                    // Mod Packs
                    SetFont("SMALLFONT");
                    /* TODO: Turn this into a loop
                     * Maybe I'll bring these back if I can come up with a decent way to get the actual total mods the weapon can ahve
                       otherwise right now it's just too inaccurate to be useful
                    if (Total[0] > 0 && Total[0] >= TotalMax)
                    {
                        HudMessage("%d", Total[0]);
                        EndHudMessage(HUDMSG_ALPHA, 0, "Gold", X - 50.0, Y, 0.05, 0.75 + (Sin((fixed)Timer() / 32.0) * 0.25));
                    }
                    else if (Total[0] > 0)
                    {
                        HudMessage("%d", Total[0]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "White", X - 50.0, Y, 0.05);
                    }
                    if (Total[1] > 0 && Total[0] >= TotalMax)
                    {
                        HudMessage("%d", Total[1]);
                        EndHudMessage(HUDMSG_ALPHA, 0, "Gold", X - 50.0, Y + 12.0, 0.05, 0.75 + (Sin((fixed)Timer() / 32.0) * 0.25));
                    }
                    else if (Total[1] > 0)
                    {
                        HudMessage("%d", Total[1]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "White", X - 50.0, Y + 12.0, 0.05);
                    }
                    */
                    if (Power[0] > 0 || GetCVar("drpg_hud_preview"))
                    {
                        HudMessage("%d", Power[0]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "Red", X - 30.0, Y + 20.0, 0.05);
                    }
                    if (Bulk[0] > 0 || GetCVar("drpg_hud_preview"))
                    {
                        HudMessage("%d", Bulk[0]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "Blue", X - 20.0, Y + 20.0, 0.05);
                    }
                    if (Agility[0] > 0 || GetCVar("drpg_hud_preview"))
                    {
                        HudMessage("%d", Agility[0]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "Green", X - 10.0, Y + 20.0, 0.05);
                    }
                    if (Tech[0] > 0 || GetCVar("drpg_hud_preview"))
                    {
                        HudMessage("%d", Tech[0]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "Yellow", X, Y + 20.0, 0.05);
                    }
                    if (Sniper[0] > 0 || GetCVar("drpg_hud_preview"))
                    {
                        HudMessage("%d", Sniper[0]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "Purple", X + 10.0, Y + 20.0, 0.05);
                    }
                    if (Firestorm[0] > 0 || GetCVar("drpg_hud_preview"))
                    {
                        HudMessage("%d", Firestorm[0]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "Orange", X + 20.0, Y + 20.0, 0.05);
                    }
                    if (Nano[0] > 0 || GetCVar("drpg_hud_preview"))
                    {
                        HudMessage("%d", Nano[0]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 30.0, Y + 20.0, 0.05);
                    }
                    if (DemonArtifacts[0] > 0 || GetCVar("drpg_hud_preview"))
                    {
                        HudMessage("%d", DemonArtifacts[0]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "DarkRed", X + 40.0, Y + 20.0, 0.05);
                    }
                    if (Power[1] > 0 || GetCVar("drpg_hud_preview"))
                    {
                        HudMessage("%d", Power[1]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "Red", X - 30.0, Y + 32.0, 0.05);
                    }
                    if (Bulk[1] > 0 || GetCVar("drpg_hud_preview"))
                    {
                        HudMessage("%d", Bulk[1]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "Blue", X - 20.0, Y + 32.0, 0.05);
                    }
                    if (Agility[1] > 0 || GetCVar("drpg_hud_preview"))
                    {
                        HudMessage("%d", Agility[1]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "Green", X - 10.0, Y + 32.0, 0.05);
                    }
                    if (Tech[1] > 0 || GetCVar("drpg_hud_preview"))
                    {
                        HudMessage("%d", Tech[1]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "Yellow", X, Y + 32.0, 0.05);
                    }
                    if (Sniper[1] > 0 || GetCVar("drpg_hud_preview"))
                    {
                        HudMessage("%d", Sniper[1]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "Purple", X + 10.0, Y + 32.0, 0.05);
                    }
                    if (Firestorm[1] > 0 || GetCVar("drpg_hud_preview"))
                    {
                        HudMessage("%d", Firestorm[1]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "Orange", X + 20.0, Y + 32.0, 0.05);
                    }
                    if (Nano[1] > 0 || GetCVar("drpg_hud_preview"))
                    {
                        HudMessage("%d", Nano[1]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "White", X + 30.0, Y + 32.0, 0.05);
                    }
                    if (DemonArtifacts[1] > 0 || GetCVar("drpg_hud_preview"))
                    {
                        HudMessage("%d", DemonArtifacts[1]);
                        EndHudMessage(HUDMSG_PLAIN, 0, "DarkRed", X + 40.0, Y + 32.0, 0.05);
                    }
                }

                // Drop Icon
                if (CheckInventory("RLWeaponDrop") || CheckInventory("RLScavengerDrop") || GetCVar("drpg_hud_preview"))
                    PrintSpritePulse("DROPICON", 0, X - 30.0, Y + 12.0, 0.75, 32.0, 0.25);

                // Icon
                if (Duel) // Duel-wielding
                {
                    if (CheckWeapon("RLAntiFreakJackal") && CheckInventory("RLAntiFreakJackalDemonArtifacts")) // Jackal/Casull
                    {
                        JackelItem = &ItemData[0][42]; // Should probably come up with a better way to reference these?
                        CasullItem = &ItemData[0][43];

                        PrintSprite(JackelItem->Sprite.Name, 0, X + JackelItem->Sprite.XOff, Y + JackelItem->Sprite.YOff + (int)(Sin((fixed)Timer() / 128.0) * 4.0), 0.05);
                        PrintSprite(CasullItem->Sprite.Name, 0, X + CasullItem->Sprite.XOff + 32.0, Y + CasullItem->Sprite.YOff + (int)(Cos((fixed)Timer() / 128.0) * 4.0), 0.05);
                    }
                    else if (CheckWeapon("RLUzi") && CheckInventory("RLUziDemonArtifacts") ||
                             CheckWeapon("RLMarathonShotguns") && CheckInventory("RLMarathonShotgunsDemonArtifacts")) // Everything else that is just duel wielding
                    {
                        PrintSprite(ItemPtr->Sprite.Name, 0, X + ItemPtr->Sprite.XOff, Y + ItemPtr->Sprite.YOff + (int)(Sin((fixed)Timer() / 128.0) * 4.0), 0.05);
                        PrintSprite(ItemPtr->Sprite.Name, 0, X + ItemPtr->Sprite.XOff + 32.0, Y + ItemPtr->Sprite.YOff + (int)(Cos((fixed)Timer() / 128.0) * 4.0), 0.05);
                    }
                }
                else
                {
                    if (CheckInventory("RLUsePowerMod") || CheckInventory("RLUseBulkMod") ||
                            CheckInventory("RLUseAgilityMod") || CheckInventory("RLUseTechnicalMod") ||
                            CheckInventory("RLUseFirestormMod") || CheckInventory("RLUseSniperMod") ||
                            CheckInventory("RLUseNanoMod") || CheckInventory("RLUseDemonArtifact"))
                        PrintSpritePulse(ItemPtr->Sprite.Name, 0, X + ItemPtr->Sprite.XOff, Y + ItemPtr->Sprite.YOff, 0.75, 32.0, 0.25);
                    else
                        PrintSprite(ItemPtr->Sprite.Name, 0, X + ItemPtr->Sprite.XOff, Y + ItemPtr->Sprite.YOff, 0.05);
                }

                break;
            }
        }
    }

    // Stamina Bar
    if (GetCVar("drpg_drla_hud_staminabar"))
    {
        for (int i = 0; i < 15; i++)
        {
            if (i * 6.66 > CheckInventory("RLStamina"))
                break;

            if (i >= 0 && i <= 2)
            {
                BarGraphic = "StamBar4";
                Alpha = 0.9 + Sin((Timer() + (i * 4)) / 16.0) * 0.1;
            }
            if (i >= 3 && i <= 5)
            {
                BarGraphic = "StamBar3";
                Alpha = 0.9 + Sin((Timer() + (i * 4)) / 32.0) * 0.1;
            }
            if (i >= 6 && i <= 9)
            {
                BarGraphic = "StamBar2";
                Alpha = 0.9 + Sin((Timer() + (i * 4)) / 48.0) * 0.1;
            }
            if (i >= 10 && i <= 14)
            {
                BarGraphic = "StamBar1";
                Alpha = 0.9 + Sin((Timer() + (i * 4)) / 64.0) * 0.1;
            }

            PrintSpriteAlpha(BarGraphic, 0, X - 160 + (i * 8), Y + 20, 0.05, Alpha);
        }
    }

    Delay(1);
    goto Start;
}

NamedScript Type_ENTER void LegenDoomHUD()
{
    if (CompatMode != COMPAT_LEGENDOOM)
        return;

    LegendaryDef const LegendaryTypesCommon[] =
    {
        {"Exorcist",        0},
        {"Extinguishing",   0},
        {"Wrangler",        0},
        {"Determined",      0},
        {"Violent",         0},
        {"Kinetic",         0},
        {"Penetrating",     0},
        {"Sustained",       0},
        {"Spiritual",       0},
        {"Tricky",          0},
        {"Raging",          0},
        {"Unknown",         0},
        {"Hoarder",         0},
        {"Survivor",        0},
        {"ColdBlooded",     0},
        {"Battlerage",      0},
        {"RazorEdge",       0},
        {"Renegade",        0},
        {"Choked",          0},
        {"Slug",            0},
        {"Spreadfire",      0},
        {"Nova",            0},
        {"Stasis",          0},
        {"Energizing",      0},
        {"Replicating",     0},
        {"Recursive",       0},
        {"Quickdraw",       0},
        {"Stealth",         0},
        {"Blastwave",       0},
        {"Agitating",       0},
        {"Pristine",        0},
        {"Gravitic",        0},
        {"Annoying",        0},
        {"Trailblazing",    0},
        {"Regenerating",    0},
        {"Upgrading",       0},
        {"AntiTree",        0},
        {"Scanner",         0},
        {"Restoring",       0},
        {"Vanguard",        0},
        {"Resupplying",     0},
        {"Quartermaster",   0},
        {"Armorer",         0},
        {"Scrambling",      0}
    };

    LegendaryDef const LegendaryTypesUncommon[] =
    {
        {"Unlikely",        0},
        {"AntiAir",         0},
        {"Berserker",       0},
        {"Chargeup",        0},
        {"Efficient",       0},
        {"Cluster",         0},
        {"Medic",           0},
        {"Optional",        0},
        {"Painful",         0},
        {"Scattering",      0},
        {"Energetic",       0},
        {"Shockwave",       0},
        {"Heavy",           0},
        {"Hunting",         0},
        {"Redirecting",     0},
        {"Mysterious",      0},
        {"Bouncy",          0},
        {"Sluggish",        0},
        {"Hyper",           0},
        {"Arcing",          0},
        {"GroupTherapy",    0},
        {"Recycling",       0},
        {"Shielding",       0},
        {"Focusing",        0},
        {"Flak",            0},
        {"Burst",           0},
        {"Thieving",        0},
        {"Bloody",          0},
        {"Proximity",       0},
        {"Transmogrifier",  0},
        {"Illuminating",    0},
        {"Lunging",         0},
        {"Rampaging",       0},
        {"Serious",         0},
        {"ProStrat",        0},
        {"Sensing",         0},
        {"WarmUp",          0},
        {"Infiltrator",     0},
        {"Incinerating",    0},
        {"Igniting",        0},
        {"Comrade",         0},
        {"Hardened",        0},
        {"Scrounger",       0},
        {"ActionHero",      0}
    };

    LegendaryDef const LegendaryTypesRare[] =
    {
        {"Improbable",      0},
        {"Scrapper",        0},
        {"Blessed",         0},
        {"Seeker",          0},
        {"Killstreaker",    0},
        {"Nuclear",         0},
        {"Lucky",           0},
        {"Desperate",       0},
        {"Whittling",       0},
        {"Shaman",          0},
        {"Accelerator",     0},
        {"Explosive",       0},
        {"Vidmaster",       0},
        {"HighExplosive",   0},
        {"ShapedCharge",    0},
        {"Rapid",           0},
        {"Marksman",        0},
        {"Sniper",          0},
        {"Extended",        0},
        {"CQC",             0},
        {"Vampiric",        0},
        {"Scavenging",      0},
        {"PointBlank",      0},
        {"Perfect",         0},
        {"Doubletap",       0},
        {"Spamtastic",      0},
        {"Concentrating",   0},
        {"Vacuum",          0},
        {"Blazing",         0},
        {"WindUp",          0},
        {"Entrenched",      0},
        {"TORGUE",          0},
        {"Suppression",     0},
        {"Waveform",        0},
        {"Beam",            0},
        {"Illegal",         0},
        {"Patient",         0},
        {"Advanced",        0},
        {"DualWield",       0},
        {"SteadyShot",      0},
        {"Suspenseful",     0},
        {"Blaster",         0},
        {"Decaying",        0},
        {"Hopeful",         0},
        {"Sadistic",        0},
        {"Intimidating",    0},
        {"Ghostly",         0},
        {"Resourceful",     0},
        {"Wildfire",        0},
        {"Chronostrike",    0}
    };

    LegendaryDef const LegendaryTypesEpic[] =
    {
        {"Impossible",      0},
        {"Heroic",          0},
        {"Murderous",       0},
        {"Unstoppable",     0},
        {"Irradiated",      0},
        {"Bloodthirsty",    0},
        {"Convincing",      0},
        {"Plasmatic",       0},
        {"Persistent",      0},
        {"TripleThreat",    0},
        {"TwinShot",        0},
        {"Gauss",           0},
        {"OnePunch",        0},
        {"NorthernStar",    0},
        {"Facekicking",     0},
        {"Lacerating",      0},
        {"Imaginary",       0},
        {"SixShooter",      0},
        {"Machine",         0},
        {"ShortRange",      0},
        {"Prototype",       0},
        {"Gyrojet",         0},
        {"Overkill",        0},
        {"Goldeneye",       0},
        {"Raycaster",       0},
        {"Annihilating",    0},
        {"Cursed",          0},
        {"Unfathomable",    0},
        {"Overflowing",     0},
        {"Artifact",        0},
        {"DemonSlayer",     0},
        {"Divine",          0},
        {"Endless",         0},
        {"NanoCircuit",     0},
        {"Crippling",       0},
        {"Freezing",        0},
        {"Favoured",        0},
        {"Thematic",        0},
        {"Soulstorm",       0},
        {"Collector",       0},
        {"Champion",        0},
        {"Augmented",       0}
    };

    LegendaryDef const LegendaryFistsRarity[] =
    {
        {"Standard", "IFISSTAN"},
        {"Common", "IFISCOMM"},
        {"Uncommon", "IFISUNCO"},
        {"Rare", "IFISRARE"},
        {"Epic", "IFISEPIC"}
    };

    LegendaryDef const LegendaryChainsawRarity[] =
    {
        {"Standard", "ISAWSTAN"},
        {"Common", "ISAWCOMM"},
        {"Uncommon", "ISAWUNCO"},
        {"Rare", "ISAWRARE"},
        {"Epic", "ISAWEPIC"}
    };

    LegendaryDef const LegendaryPistolRarity[] =
    {
        {"Standard", "IPISSTAN"},
        {"Common", "IPISCOMM"},
        {"Uncommon", "IPISUNCO"},
        {"Rare", "IPISRARE"},
        {"Epic", "IPISEPIC"}
    };

    LegendaryDef const LegendaryShotgunRarity[] =
    {
        {"Standard", "ISH1STAN"},
        {"Common", "ISH1COMM"},
        {"Uncommon", "ISH1UNCO"},
        {"Rare", "ISH1RARE"},
        {"Epic", "ISH1EPIC"}
    };

    LegendaryDef const LegendarySuperShotgunRarity[] =
    {
        {"Standard", "ISH2STAN"},
        {"Common", "ISH2COMM"},
        {"Uncommon", "ISH2UNCO"},
        {"Rare", "ISH2RARE"},
        {"Epic", "ISH2EPIC"}
    };

    LegendaryDef const LegendaryChaingunRarity[] =
    {
        {"Standard", "ICHGSTAN"},
        {"Common", "ICHGCOMM"},
        {"Uncommon", "ICHGUNCO"},
        {"Rare", "ICHGRARE"},
        {"Epic", "ICHGEPIC"}
    };

    LegendaryDef const LegendaryRocketLauncherRarity[] =
    {
        {"Standard", "IRLASTAN"},
        {"Common", "IRLACOMM"},
        {"Uncommon", "IRLAUNCO"},
        {"Rare", "IRLARARE"},
        {"Epic", "IRLAEPIC"}
    };

    LegendaryDef const LegendaryPlasmaRifleRarity[] =
    {
        {"Standard", "IPLSSTAN"},
        {"Common", "IPLSCOMM"},
        {"Uncommon", "IPLSUNCO"},
        {"Rare", "IPLSRARE"},
        {"Epic", "IPLSEPIC"}
    };

    LegendaryDef const LegendaryBFG9000Rarity[] =
    {
        {"Standard", "IBFGSTAN"},
        {"Common", "IBFGCOMM"},
        {"Uncommon", "IBFGUNCO"},
        {"Rare", "IBFGRARE"},
        {"Epic", "IBFGEPIC"}
    };

    fixed X, Y;
    str FistsIcon, FistsEffect, ChainsawIcon, ChainsawEffect, PistolIcon, PistolEffect;
    str ShotgunIcon, ShotgunEffect, SuperShotgunIcon, SuperShotgunEffect;
    str ChaingunIcon, ChaingunEffect, RocketLauncherIcon, RocketLauncherEffect;
    str PlasmaRifleIcon, PlasmaRifleEffect, BFG9000Icon, BFG9000Effect;
    int i;

Start:
    NOP;

    // If we're on the title map, terminate
    if (InTitle) return;

    if (Player.GUI.Open || Player.InMenu || Player.InShop || Player.OutpostMenu || (CompatMode == COMPAT_LEGENDOOM && CheckInventory("LDWeaponInfoScreenActive")))
    {
        Delay(1);
        goto Start;
    }

    X = GetActivatorCVar("drpg_ld_x");
    Y = GetActivatorCVar("drpg_ld_y");

    // ----------------------------------------
    // Update

    // Fists
    if (!CheckInventory("LDFists"))
    {
        FistsIcon = "";
        FistsEffect = "";
    }
    else if (!CheckInventory("LDFistsEffectActive"))
    {
        FistsIcon = LegendaryFistsRarity[0].Image;
        FistsEffect = "";
    }
    else
    {
        for (i = LD_COMMON; i <= sizeof(LegendaryFistsRarity); i++)
        {
            if (CheckInventory(StrParam("LDFistsLegendary%S", LegendaryFistsRarity[i].Effect)))
            {
                FistsIcon = LegendaryFistsRarity[i].Image;
                break;
            }
        }

        switch(i)
        {
        case LD_COMMON:
            FistsEffect = GetLegendaryEffectImage(&LegendaryTypesCommon[0], sizeof(LegendaryTypesCommon), "LDFistsEffect_");
            break;

        case LD_UNCOMMON:
            FistsEffect = GetLegendaryEffectImage(&LegendaryTypesUncommon[0], sizeof(LegendaryTypesUncommon), "LDFistsEffect_");
            break;

        case LD_RARE:
            FistsEffect = GetLegendaryEffectImage(&LegendaryTypesRare[0], sizeof(LegendaryTypesRare), "LDFistsEffect_");
            break;

        case LD_EPIC:
            FistsEffect = GetLegendaryEffectImage(&LegendaryTypesEpic[0], sizeof(LegendaryTypesEpic), "LDFistsEffect_");
            break;
        }
    }

    // Chainsaw
    if (!CheckInventory("LDChainsaw"))
    {
        ChainsawIcon = "";
        ChainsawEffect = "";
    }
    else if (!CheckInventory("LDChainsawEffectActive"))
    {
        ChainsawIcon = LegendaryChainsawRarity[0].Image;
        ChainsawEffect = "";
    }
    else
    {
        for (i = LD_COMMON; i <= sizeof(LegendaryChainsawRarity); i++)
        {
            if (CheckInventory(StrParam("LDChainsawLegendary%S", LegendaryChainsawRarity[i].Effect)))
            {
                ChainsawIcon = LegendaryChainsawRarity[i].Image;
                break;
            }
        }

        switch(i)
        {
        case LD_COMMON:
            ChainsawEffect = GetLegendaryEffectImage(&LegendaryTypesCommon[0], sizeof(LegendaryTypesCommon), "LDChainsawEffect_");
            break;

        case LD_UNCOMMON:
            ChainsawEffect = GetLegendaryEffectImage(&LegendaryTypesUncommon[0], sizeof(LegendaryTypesUncommon), "LDChainsawEffect_");
            break;

        case LD_RARE:
            ChainsawEffect = GetLegendaryEffectImage(&LegendaryTypesRare[0], sizeof(LegendaryTypesRare), "LDChainsawEffect_");
            break;

        case LD_EPIC:
            ChainsawEffect = GetLegendaryEffectImage(&LegendaryTypesEpic[0], sizeof(LegendaryTypesEpic), "LDChainsawEffect_");
            break;
        }
    }

    // Pistol
    if (!CheckInventory("LDPistol"))
    {
        PistolIcon = "";
        PistolEffect = "";
    }
    else if (!CheckInventory("LDPistolEffectActive"))
    {
        PistolIcon = LegendaryPistolRarity[0].Image;
        PistolEffect = "";
    }
    else
    {
        for (i = LD_COMMON; i <= sizeof(LegendaryPistolRarity); i++)
        {
            if (CheckInventory(StrParam("LDPistolLegendary%S", LegendaryPistolRarity[i].Effect)))
            {
                PistolIcon = LegendaryPistolRarity[i].Image;
                break;
            }
        }

        switch(i)
        {
        case LD_COMMON:
            PistolEffect = GetLegendaryEffectImage(&LegendaryTypesCommon[0], sizeof(LegendaryTypesCommon), "LDPistolEffect_");
            break;

        case LD_UNCOMMON:
            PistolEffect = GetLegendaryEffectImage(&LegendaryTypesUncommon[0], sizeof(LegendaryTypesUncommon), "LDPistolEffect_");
            break;

        case LD_RARE:
            PistolEffect = GetLegendaryEffectImage(&LegendaryTypesRare[0], sizeof(LegendaryTypesRare), "LDPistolEffect_");
            break;

        case LD_EPIC:
            PistolEffect = GetLegendaryEffectImage(&LegendaryTypesEpic[0], sizeof(LegendaryTypesEpic), "LDPistolEffect_");
            break;
        }
    }

    // Shotgun
    if (!CheckInventory("LDShotgun"))
    {
        ShotgunIcon = "";
        ShotgunEffect = "";
    }
    else if (!CheckInventory("LDShotgunEffectActive"))
    {
        ShotgunIcon = LegendaryShotgunRarity[0].Image;
        ShotgunEffect = "";
    }
    else
    {
        for (i = LD_COMMON; i <= sizeof(LegendaryShotgunRarity); i++)
        {
            if (CheckInventory(StrParam("LDShotgunLegendary%S", LegendaryShotgunRarity[i].Effect)))
            {
                ShotgunIcon = LegendaryShotgunRarity[i].Image;
                break;
            }
        }

        switch(i)
        {
        case LD_COMMON:
            ShotgunEffect = GetLegendaryEffectImage(&LegendaryTypesCommon[0], sizeof(LegendaryTypesCommon), "LDShotgunEffect_");
            break;

        case LD_UNCOMMON:
            ShotgunEffect = GetLegendaryEffectImage(&LegendaryTypesUncommon[0], sizeof(LegendaryTypesUncommon), "LDShotgunEffect_");
            break;

        case LD_RARE:
            ShotgunEffect = GetLegendaryEffectImage(&LegendaryTypesRare[0], sizeof(LegendaryTypesRare), "LDShotgunEffect_");
            break;

        case LD_EPIC:
            ShotgunEffect = GetLegendaryEffectImage(&LegendaryTypesEpic[0], sizeof(LegendaryTypesEpic), "LDShotgunEffect_");
            break;
        }
    }

    // Super Shotgun
    if (!CheckInventory("LDSuperShotgun"))
    {
        SuperShotgunIcon = "";
        SuperShotgunEffect = "";
    }
    else if (!CheckInventory("LDSuperShotgunEffectActive"))
    {
        SuperShotgunIcon = LegendarySuperShotgunRarity[0].Image;
        SuperShotgunEffect = "";
    }
    else
    {
        for (i = LD_COMMON; i <= sizeof(LegendarySuperShotgunRarity); i++)
        {
            if (CheckInventory(StrParam("LDSuperShotgunLegendary%S", LegendarySuperShotgunRarity[i].Effect)))
            {
                SuperShotgunIcon = LegendarySuperShotgunRarity[i].Image;
                break;
            }
        }

        switch(i)
        {
        case LD_COMMON:
            SuperShotgunEffect = GetLegendaryEffectImage(&LegendaryTypesCommon[0], sizeof(LegendaryTypesCommon), "LDSuperShotgunEffect_");
            break;

        case LD_UNCOMMON:
            SuperShotgunEffect = GetLegendaryEffectImage(&LegendaryTypesUncommon[0], sizeof(LegendaryTypesUncommon), "LDSuperShotgunEffect_");
            break;

        case LD_RARE:
            SuperShotgunEffect = GetLegendaryEffectImage(&LegendaryTypesRare[0], sizeof(LegendaryTypesRare), "LDSuperShotgunEffect_");
            break;

        case LD_EPIC:
            SuperShotgunEffect = GetLegendaryEffectImage(&LegendaryTypesEpic[0], sizeof(LegendaryTypesEpic), "LDSuperShotgunEffect_");
            break;
        }
    }

    // Chaingun
    if (!CheckInventory("LDChaingun"))
    {
        ChaingunIcon = "";
        ChaingunEffect = "";
    }
    else if (!CheckInventory("LDChaingunEffectActive"))
    {
        ChaingunIcon = LegendaryChaingunRarity[0].Image;
        ChaingunEffect = "";
    }
    else
    {
        for (i = LD_COMMON; i <= sizeof(LegendaryChaingunRarity); i++)
        {
            if (CheckInventory(StrParam("LDChaingunLegendary%S", LegendaryChaingunRarity[i].Effect)))
            {
                ChaingunIcon = LegendaryChaingunRarity[i].Image;
                break;
            }
        }

        switch(i)
        {
        case LD_COMMON:
            ChaingunEffect = GetLegendaryEffectImage(&LegendaryTypesCommon[0], sizeof(LegendaryTypesCommon), "LDChaingunEffect_");
            break;

        case LD_UNCOMMON:
            ChaingunEffect = GetLegendaryEffectImage(&LegendaryTypesUncommon[0], sizeof(LegendaryTypesUncommon), "LDChaingunEffect_");
            break;

        case LD_RARE:
            ChaingunEffect = GetLegendaryEffectImage(&LegendaryTypesRare[0], sizeof(LegendaryTypesRare), "LDChaingunEffect_");
            break;

        case LD_EPIC:
            ChaingunEffect = GetLegendaryEffectImage(&LegendaryTypesEpic[0], sizeof(LegendaryTypesEpic), "LDChaingunEffect_");
            break;
        }
    }

    // Rocket Launcher
    if (!CheckInventory("LDRocketLauncher"))
    {
        RocketLauncherIcon = "";
        RocketLauncherEffect = "";
    }
    else if (!CheckInventory("LDRocketLauncherEffectActive"))
    {
        RocketLauncherIcon = LegendaryRocketLauncherRarity[0].Image;
        RocketLauncherEffect = "";
    }
    else
    {
        for (i = LD_COMMON; i <= sizeof(LegendaryRocketLauncherRarity); i++)
        {
            if (CheckInventory(StrParam("LDRocketLauncherLegendary%S", LegendaryRocketLauncherRarity[i].Effect)))
            {
                RocketLauncherIcon = LegendaryRocketLauncherRarity[i].Image;
                break;
            }
        }

        switch(i)
        {
        case LD_COMMON:
            RocketLauncherEffect = GetLegendaryEffectImage(&LegendaryTypesCommon[0], sizeof(LegendaryTypesCommon), "LDRocketLauncherEffect_");
            break;

        case LD_UNCOMMON:
            RocketLauncherEffect = GetLegendaryEffectImage(&LegendaryTypesUncommon[0], sizeof(LegendaryTypesUncommon), "LDRocketLauncherEffect_");
            break;

        case LD_RARE:
            RocketLauncherEffect = GetLegendaryEffectImage(&LegendaryTypesRare[0], sizeof(LegendaryTypesRare), "LDRocketLauncherEffect_");
            break;

        case LD_EPIC:
            RocketLauncherEffect = GetLegendaryEffectImage(&LegendaryTypesEpic[0], sizeof(LegendaryTypesEpic), "LDRocketLauncherEffect_");
            break;
        }
    }

    // Plasma Rifle
    if (!CheckInventory("LDPlasmaRifle"))
    {
        PlasmaRifleIcon = "";
        PlasmaRifleEffect = "";
    }
    else if (!CheckInventory("LDPlasmaRifleEffectActive"))
    {
        PlasmaRifleIcon = LegendaryPlasmaRifleRarity[0].Image;
        PlasmaRifleEffect = "";
    }
    else
    {
        for (i = LD_COMMON; i <= sizeof(LegendaryPlasmaRifleRarity); i++)
        {
            if (CheckInventory(StrParam("LDPlasmaRifleLegendary%S", LegendaryPlasmaRifleRarity[i].Effect)))
            {
                PlasmaRifleIcon = LegendaryPlasmaRifleRarity[i].Image;
                break;
            }
        }

        switch(i)
        {
        case LD_COMMON:
            PlasmaRifleEffect = GetLegendaryEffectImage(&LegendaryTypesCommon[0], sizeof(LegendaryTypesCommon), "LDPlasmaRifleEffect_");
            break;

        case LD_UNCOMMON:
            PlasmaRifleEffect = GetLegendaryEffectImage(&LegendaryTypesUncommon[0], sizeof(LegendaryTypesUncommon), "LDPlasmaRifleEffect_");
            break;

        case LD_RARE:
            PlasmaRifleEffect = GetLegendaryEffectImage(&LegendaryTypesRare[0], sizeof(LegendaryTypesRare), "LDPlasmaRifleEffect_");
            break;

        case LD_EPIC:
            PlasmaRifleEffect = GetLegendaryEffectImage(&LegendaryTypesEpic[0], sizeof(LegendaryTypesEpic), "LDPlasmaRifleEffect_");
            break;
        }
    }

    // BFG9000
    if (!CheckInventory("LDBFG9000"))
    {
        BFG9000Icon = "";
        BFG9000Effect = "";
    }
    else if (!CheckInventory("LDBFG9000EffectActive"))
    {
        BFG9000Icon = LegendaryBFG9000Rarity[0].Image;
        BFG9000Effect = "";
    }
    else
    {
        for (i = LD_COMMON; i <= sizeof(LegendaryBFG9000Rarity); i++)
        {
            if (CheckInventory(StrParam("LDBFG9000Legendary%S", LegendaryBFG9000Rarity[i].Effect)))
            {
                BFG9000Icon = LegendaryBFG9000Rarity[i].Image;
                break;
            }
        }

        switch(i)
        {
        case LD_COMMON:
            BFG9000Effect = GetLegendaryEffectImage(&LegendaryTypesCommon[0], sizeof(LegendaryTypesCommon), "LDBFG9000Effect_");
            break;

        case LD_UNCOMMON:
            BFG9000Effect = GetLegendaryEffectImage(&LegendaryTypesUncommon[0], sizeof(LegendaryTypesUncommon), "LDBFG9000Effect_");
            break;

        case LD_RARE:
            BFG9000Effect = GetLegendaryEffectImage(&LegendaryTypesRare[0], sizeof(LegendaryTypesRare), "LDBFG9000Effect_");
            break;

        case LD_EPIC:
            BFG9000Effect = GetLegendaryEffectImage(&LegendaryTypesEpic[0], sizeof(LegendaryTypesEpic), "LDBFG9000Effect_");
            break;
        }
    }

    // ----------------------------------------
    // HUD Preview

    if (GetActivatorCVar("drpg_hud_preview"))
    {
        if (FistsIcon == "")
            FistsIcon = LegendaryFistsRarity[0].Image;
        if (FistsEffect == "")
            FistsEffect = LegendaryTypesCommon[0].Image;

        if (ChainsawIcon == "")
            ChainsawIcon = LegendaryChainsawRarity[0].Image;
        if (ChainsawEffect == "")
            ChainsawEffect = LegendaryTypesCommon[0].Image;

        if (PistolIcon == "")
            PistolIcon = LegendaryPistolRarity[0].Image;
        if (PistolEffect == "")
            PistolEffect = LegendaryTypesCommon[0].Image;

        if (ShotgunIcon == "")
            ShotgunIcon = LegendaryShotgunRarity[0].Image;
        if (ShotgunEffect == "")
            ShotgunEffect = LegendaryTypesCommon[0].Image;

        if (SuperShotgunIcon == "")
            SuperShotgunIcon = LegendarySuperShotgunRarity[0].Image;
        if (SuperShotgunEffect == "")
            SuperShotgunEffect = LegendaryTypesCommon[0].Image;

        if (ChaingunIcon == "")
            ChaingunIcon = LegendaryChaingunRarity[0].Image;
        if (ChaingunEffect == "")
            ChaingunEffect = LegendaryTypesCommon[0].Image;

        if (RocketLauncherIcon == "")
            RocketLauncherIcon = LegendaryRocketLauncherRarity[0].Image;
        if (RocketLauncherEffect == "")
            RocketLauncherEffect = LegendaryTypesCommon[0].Image;

        if (PlasmaRifleIcon == "")
            PlasmaRifleIcon = LegendaryPlasmaRifleRarity[0].Image;
        if (PlasmaRifleEffect == "")
            PlasmaRifleEffect = LegendaryTypesCommon[0].Image;

        if (BFG9000Icon == "")
            BFG9000Icon = LegendaryBFG9000Rarity[0].Image;
        if (BFG9000Effect == "")
            BFG9000Effect = LegendaryTypesCommon[0].Image;
    }

    // ----------------------------------------
    // Draw

    SetHudSize(GetActivatorCVar("drpg_hud_width"), GetActivatorCVar("drpg_hud_height"), false);

    // Fists
    if (FistsIcon != "")
        PrintSprite(FistsIcon, 0, X + 0.1, Y + 0.1, 0.05);
    if (FistsEffect != "")
        PrintSprite(FistsEffect, 0, X + 4 + 0.1, Y + 16 + 0.1, 0.05);

    // Chainsaw
    if (ChainsawIcon != "")
        PrintSprite(ChainsawIcon, 0, X + 24 + 0.1, Y + 0.1, 0.05);
    if (ChainsawEffect != "")
        PrintSprite(ChainsawEffect, 0, X + 24 + 4 + 0.1, Y + 16 + 0.1, 0.05);

    // Pistol
    if (PistolIcon != "")
        PrintSprite(PistolIcon, 0, X + 48 + 0.1, Y + 0.1, 0.05);
    if (PistolEffect != "")
        PrintSprite(PistolEffect, 0, X + 48 + 4 + 0.1, Y + 16 + 0.1, 0.05);

    // Shotgun
    if (ShotgunIcon != "")
        PrintSprite(ShotgunIcon, 0, X + 72 + 0.1, Y + 0.1, 0.05);
    if (ShotgunEffect != "")
        PrintSprite(ShotgunEffect, 0, X + 72 + 4 + 0.1, Y + 16 + 0.1, 0.05);

    // Super Shotgun
    if (SuperShotgunIcon != "")
        PrintSprite(SuperShotgunIcon, 0, X + 96 + 0.1, Y + 0.1, 0.05);
    if (SuperShotgunEffect != "")
        PrintSprite(SuperShotgunEffect, 0, X + 96 + 4 + 0.1, Y + 16 + 0.1, 0.05);

    // Chaingun
    if (ChaingunIcon != "")
        PrintSprite(ChaingunIcon, 0, X + 120 + 0.1, Y + 0.1, 0.05);
    if (ChaingunEffect != "")
        PrintSprite(ChaingunEffect, 0, X + 120 + 4 + 0.1, Y + 16 + 0.1, 0.05);

    // Rocket Launcher
    if (RocketLauncherIcon != "")
        PrintSprite(RocketLauncherIcon, 0, X + 144 + 0.1, Y + 0.1, 0.05);
    if (RocketLauncherEffect != "")
        PrintSprite(RocketLauncherEffect, 0, X + 144 + 4 + 0.1, Y + 16 + 0.1, 0.05);

    // Plasma Rifle
    if (PlasmaRifleIcon != "")
        PrintSprite(PlasmaRifleIcon, 0, X + 168 + 0.1, Y + 0.1, 0.05);
    if (PlasmaRifleEffect != "")
        PrintSprite(PlasmaRifleEffect, 0, X + 168 + 4 + 0.1, Y + 16 + 0.1, 0.05);

    // BFG9000
    if (BFG9000Icon != "")
        PrintSprite(BFG9000Icon, 0, X + 192 + 0.1, Y + 0.1, 0.05);
    if (BFG9000Effect != "")
        PrintSprite(BFG9000Effect, 0, X + 192 + 4 + 0.1, Y + 16 + 0.1, 0.05);

    Delay(1);
    goto Start;
}

str GetLegendaryEffectImage(LegendaryDef const *Type, int Length, str Prefix)
{
    str Image;

    for (int i = 0; i < Length; i++)
    {
        if (CheckInventory(StrParam("%S%S", Prefix, Type[i].Effect)))
        {
            Image = StrParam("R_%S", StrLeft(Type[i].Effect, 6));
            break;
        }
    }

    return Image;
}
