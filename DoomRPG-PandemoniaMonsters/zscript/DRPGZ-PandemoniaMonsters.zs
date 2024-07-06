// DOOMRPG COMPATIBILITY
// --------------------------------------------------

Class PyrobruiserCometDRPGFix : PyrobruiserComet replaces PyrobruiserComet 										//Fix for Pyrobruiser Comet with DRLA i think
{
  States
  {
  Spawn:
    ZCOM AABB 2 Bright Light("Comet1")
		{
		A_SpawnItemEx("AfritCometTrail",0,0,0,frandom(-0.5,0),frandom(-0.5,-0.5),frandom(0,0.5));
		A_SpawnItemEx("FireProjectileTrail",frandom(-6,0),frandom(-6,6),frandom(-2,6),0,frandom(2,2),frandom(0,3));
		A_SpawnItemEx("FireProjectileTrail2",frandom(-6,0),frandom(-6,6),frandom(-2,6),0,frandom(1,1),frandom(0,5));
		for(user_fx = 0;user_fx<=2;user_fx++)
			A_SpawnParticle(GetParticleColor(),SPF_FULLBRIGHT|SPF_RELATIVE,random(14,22),frandom(10,14),0,frandom(-8,0),frandom(-8,8),frandom(-8,8),frandom(-4,0),frandom(-1,1),frandom(0,4),0,0,-0.03,1,-1,-0.8);
		}
    Loop;
	}
}

Class OverchargedPlasmaBallTrailRPG : OverchargedPlasmaBallTrail replaces OverchargedPlasmaBallTrail
{
  States
  {
  Spawn:
	TNT1 A 2;
	PLOV C 1
		{
		A_SetScale(Scale.X-0.15);
		A_FadeOut(0.15);
		}
	Wait;
  	}
}

Class WidowBigShotRPG : WidowBigShot replaces WidowBigShot
{
  States
  {
  Spawn:
	PLOV AAABBB 1 Bright Light("RedPlasma") 
		{
		A_SpawnItemEx("RedPlasmaLightningTrail",0,0,0,0,0,0,0,0,220);
		A_SpawnItemEx("OverchargedPlasmaBallTrail");
		for(user_fx = 0;user_fx<=2;user_fx++)
			A_SpawnParticle(GetParticleColor(),SPF_FULLBRIGHT|SPF_RELATIVE,10,frandom(5,10),0,frandom(-8,0),frandom(-5,5),frandom(-5,5),frandom(-4,0),frandom(-1,1),0,0,0,0,1.0,-1,-0.75);
		}
	Loop;
  Death:
	TNT1 A 0 
		{
		A_SetScale(1.4);
		A_SetRoll(random(0,360));
		A_SpawnItemEx("RedPlasmaExplosionFlare");
		for(user_fx = 0;user_fx<=30;user_fx++)
			A_SpawnParticle(GetParticleColor(),SPF_FULLBRIGHT|SPF_RELATIVE,random(10,17),frandom(12,14),random(0,360),frandom(0,6),0,frandom(-6,6),frandom(0,6),0,frandom(-6,6),0,0,0,1,-1,-1.2);
		}
	PLRE ABCDEFGHIJKL 2 Bright;
	Stop;
	}
}

//Pand teleport fog fix (not spawning when reinforcements arrive)
Class PandTeleportFogRPG : TeleportFog replaces NewTeleportFog
{

  States
  {
  Spawn:
	TNT1 A 0 NoDelay A_StartSound("Teleport/Normal",1);
	TNT1 AAAAAA 5 Light("DTFOG1")
		{
		A_SpawnItemEx("BFGLightningTrail",0,0,32,0,0,0,0,0,128);
		A_SpawnItemEx("BFGBallTrail",0,0,32,frandom(0,3),0,frandom(-3,3),random(0,360));
		//A_SpawnParticle(GetParticleColor(),SPF_FULLBRIGHT|SPF_RELATIVE,random(10,17),frandom(12,14),random(0,360),0,0,32,frandom(0,6),0,frandom(-6,6),0,0,0,1,-1,-1.2); //can`t add the fancy praticles
		}
	Stop;
	}
}

//Monsters Things (damage types, drop items and etc.)

//Zombies
Class PistolZombieRPG : PistolZombie replaces PistolZombie
{
	Default
	{
		DropItem "PistolZombieDropRPG";
	}
}

Class PistolZombieDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGPistol",         255, 48;
		DropItem "DRPGPistolSpawner",  255, 6;
		DropItem "DRPGClipSpawner",    255, 204;
	}
}

Class LaserRifleZombieRPG : LaserRifleZombie replaces LaserRifleZombie
{
	Default
	{
		DropItem "LaserRifleZombieDropRPG";
		DamageType "Plasma";
	}
}

Class LaserRifleZombieDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGPlasmaRifle",         255, 48;
		DropItem "DRPGPlasmaRifleSpawner",  255, 6;
		DropItem "DRPGCellSpawner",         255, 204;
	}
}

Class PlasmaPistolZombieRPG : PlasmaPistolZombie replaces PlasmaPistolZombie
{
	Default
	{
		DropItem "DRPGCellSpawner", 128;
		DamageType "Plasma";
	}
}

Class SpecOpsRiflemanRPG : SpecOpsRifleman replaces SpecOpsRifleman
{
	Default
	{
		DropItem "SpecOpsRiflemanDropRPG";
		DamageType "Plasma";
	}
}

Class SpecOpsRiflemanDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGClipSpawner",    255, 208;
		DropItem "DRPGCellSpawner",    255, 48;
	}
}

Class CinderZombieRPG : CinderZombie replaces CinderZombie
{
	Default
	{
		DropItem "PistolZombieDropRPG";
		DamageType "Fire";
	}
}

//Shotgunners
Class NewShotgunguyRPG : NewShotgunguy replaces NewShotgunguy
{
	Default
	{
		DropItem "NewShotgunguyDropRPG";
	}
}

Class NewShotgunguyDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGShotgun",         255, 48;
		DropItem "DRPGShotgunSpawner",  255, 6;
		DropItem "DRPGShellSpawner",    255, 204;
	}
}

Class AutoShotgunZombieRPG : AutoShotgunZombie replaces AutoShotgunZombie
{
	Default
	{
		DropItem "AutoShotgunZombieDropRPG";
	}
}

Class AutoShotgunZombieDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGCombatShotgun",           255, 48;
		DropItem "DRPGCombatShotgunSpawner",    255, 6;
		DropItem "DRPGShellSpawner",            255, 204;
		DropItem "DRPGShellBoxSpawner",         255, 48;
	}
}

Class RocketZombieRPG : RocketZombie replaces RocketZombie
{
	Default
	{
		DropItem "RocketZombieDropRPG";
	}
}

Class RocketZombieDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGRocketLauncher",          255, 48;
		DropItem "DRPGRocketLauncherSpawner",   255, 6;
		DropItem "DRPGRocketAmmoSpawner",       255, 204;
	}
}

Class ShotgunAdmiralRPG : ShotgunAdmiral replaces ShotgunAdmiral
{
	Default
	{
		DropItem "ShotgunAdmiralDropRPG";
	}
}

Class ShotgunAdmiralDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGChaingun",           255, 48;
		DropItem "DRPGChaingunSpawner",    255, 6;
		DropItem "DRPGShellBoxSpawner",    255, 204;
	}
}

Class SlugShotgunZombieRPG : SlugShotgunZombie replaces SlugShotgunZombie
{
	Default
	{
		DropItem "SlugShotgunZombieDropRPG";
	}
}

Class SlugShotgunZombieDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGShotgun",          255, 48;
		DropItem "DRPGShotgunSpawner",   255, 6;
		DropItem "DRPGShellSpawner",     255, 204;
	}
}

Class SpecOpsShotgunnerRPG : SpecOpsShotgunner replaces SpecOpsShotgunner
{
	Default
	{
		DropItem "SpecOpsShotgunnerDropRPG";
	}
}

Class SpecOpsShotgunnerDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGCombatShotgun",           255, 48;
		DropItem "DRPGCombatShotgunSpawner",    255, 6;
		DropItem "DRPGShellSpawner",            255, 204;
		DropItem "DRPGShellBoxSpawner",         255, 48;
	}
}

Class SuperShotgunZombieRPG : SuperShotgunZombie replaces SuperShotgunZombie
{
	Default
	{
		DropItem "SuperShotgunZombieDropRPG";
	}
}

Class SuperShotgunZombieDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGSuperShotgun",          255, 48;
		DropItem "DRPGSuperShotgunSpawner",   255, 6;
		DropItem "DRPGShellSpawner",          255, 204;
	}
}

Class ChaosWarriorRPG : ChaosWarrior replaces ChaosWarrior
{
	Default
	{
		DropItem "ChaosWarriorDropRPG";
		DamageType "Radiation";
		DamageFactor "Radiation", 0.2;
	}
}

Class ChaosWarriorDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGCellSpawner",          255, 208;
		DropItem "DRPGCellPackSpawner",      255, 48;
	}
}

Class ChaosWarriorProjectileDRPG : ChaosWarriorProjectile replaces ChaosWarriorProjectile
{
	Default
	{
		Damagetype "Radiation";
	}
}

Class PyroZombieRPG : PyroZombie replaces PyroZombie
{
	Default
	{
		DropItem "NewShotgunguyDropRPG";
	}
}

//Chaingunners
Class ChaingunZombieRPG : ChaingunZombie replaces ChaingunZombie
{
	Default
	{
		DropItem "ChaingunZombieDropRPG";
	}
}

Class ChaingunZombieDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGChaingun",          255, 48;
		DropItem "DRPGChaingunSpawner",   255, 6;
		DropItem "DRPGClipBoxSpawner",    255, 204;
	}
}

Class AssaultGunnerRPG : AssaultGunner replaces AssaultGunner
{
	Default
	{
		DropItem "AssaultGunnerDropRPG";
	}
}

Class AssaultGunnerDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGBattleRifle",          255, 48;
		DropItem "DRPGBattleRifleSpawner",   255, 6;
		DropItem "DRPGClipBoxSpawner",       255, 204;
	}
}

Class NailborgCommandoRPG : NailborgCommando replaces NailborgCommando
{
	Default
	{
		DropItem "NailborgCommandoDropRPG";
	}
}

Class NailborgCommandoDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGChaingun",          255, 48;
		DropItem "DRPGChaingunSpawner",   255, 6;
		DropItem "DRPGClipBoxSpawner",    255, 204;
	}
}

Class SpecOpsGunnerRPG : SpecOpsGunner replaces SpecOpsGunner
{
	Default
	{
		DropItem "SpecOpsGunnerDropRPG";
	}
}

Class SpecOpsGunnerDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGBattleRifle",         255, 48;
		DropItem "DRPGBattleRifleSpawner",  255, 6;
		DropItem "DRPGClipBoxSpawner",      255, 204;
	}
}

Class SpecOpsLieutenantRPG : SpecOpsLieutenant replaces SpecOpsLieutenant
{
	Default
	{
		DropItem "SpecOpsLieutenantDropRPG";
	}
}

Class SpecOpsLieutenantDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGChaingun",           255, 48;
		DropItem "DRPGChaingunSpawner",    255, 6;
		DropItem "DRPGClipBoxSpawner",     255, 128;
		DropItem "DRPGRocketAmmoSpawner",  255, 128;
	}
}

Class ChaosDeathmarkRPG : ChaosDeathmark replaces ChaosDeathmark
{
	Default
	{
		DropItem "ChaosDeathmarkDropRPG";
		DamageType "Radiation";
		DamageFactor "Radiation", 0.2;
	}
}

Class ChaosDeathmarkDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGCellSpawner",          255, 208;
		DropItem "DRPGCellPackSpawner",      255, 48;
	}
}

Class DeathmarkPuffDRPG : DeathmarkPuff replaces DeathmarkPuff
{
	Default
	{
		DamageType "Radiation";
	}
}

Class PlasmaRifleZombieRPG : PlasmaRifleZombie replaces PlasmaRifleZombie
{
	Default
	{
		DropItem "PlasmaRifleZombieDropRPG";
		DamageType "Plasma";
	}
}

Class PlasmaRifleZombieDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGPlasmaRifle",         255, 48;
		DropItem "DRPGPlasmaRifleSpawner",  255, 6;
		DropItem "DRPGCellSpawner",         255, 204;
	}
}

Class BFG10kWarriorRPG : BFG10kWarrior replaces BFG10kWarrior
{
	Default
	{
		DropItem "BFG10kWarriorDropRPG";
		DamageType "Plasma";
	}
}

Class BFG10kWarriorDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGBFG9000",          255, 48;
		DropItem "DRPGBFG9000Spawner",   255, 6;
		DropItem "DRPGCellPackSpawner",  255, 204;
	}
}

Class BFGCommandoRPG : BFGCommando replaces BFGCommando
{
	Default
	{
		DropItem "BFGCommandoDropRPG";
		DamageType "Plasma";
	}
}

Class BFGCommandoDropRPG : RandomSpawner
{
	Default
	{
		DropItem "DRPGBFG9000",          255, 48;
		DropItem "DRPGBFG9000Spawner",   255, 6;
		DropItem "DRPGCellPackSpawner",  255, 204;
	}
}

Class ScourgeZombieRPG : ScourgeZombie replaces ScourgeZombie
{
	Default
	{
		DropItem "PlasmaRifleZombieDropRPG";
		DamageType "Slime";
		DamageFactor "Slime", 0.2;
	}
}
Class ScourgeProjectileDRPG : ScourgeProjectile replaces ScourgeProjectile
{
	Default
	{
		DamageType "Slime";
	}
}

Class InfernalCommandoRPG : InfernalCommando replaces InfernalCommando
{
	Default
	{
		DropItem "BFGCommandoDropRPG";
		DamageType "Fire";
	}
}

//Imps
Class DarkImpRPG : DarkImp replaces DarkImp {}
Class DevilRPG : Devil replaces Devil {}
Class CyberImpRPG : CyberImp replaces CyberImp {}
Class NewImpRPG : NewImp replaces NewImp {}
Class PhaseImpRPG : PhaseImp replaces PhaseImp {}
Class ScoundrelRPG : Scoundrel replaces Scoundrel {}
Class BrimstoneImpRPG : BrimstoneImp replaces BrimstoneImp {}

Class ChaosImpRPG : ChaosImp replaces ChaosImp
{
	Default
	{
		DamageType "Radiation";
		DamageFactor "Radiation", 0.2;
	}
}

Class PlaguelingRPG : Plagueling replaces Plagueling
{
	Default
	{
		DamageType "Slime";
		DamageFactor "Slime", 0.2;
	}
}
Class PlaguelingBallDRPG : PlaguelingBall replaces PlaguelingBall
{
	Default
	{
		DamageType "Slime";
	}
}
Class PlaguelingPoisonPodDRPG : PlaguelingPoisonPod replaces PlaguelingPoisonPod
{
	Default
	{
		DamageType "Slime";
	}
}

Class AbyssalSavageRPG : AbyssalSavage replaces AbyssalSavage
{
	Default
	{
		DamageType "Shadow";
		DamageFactor "Shadow", 0;
	}
}
Class AbyssalSavageBallRPG : AbyssalSavageBall replaces AbyssalSavageBall
{
	Default
	{
		DamageType "Shadow";
		DamageType "Fire";
	}
}
Class AbyssalSavageDarkBallRPG : AbyssalSavageDarkBall replaces AbyssalSavageDarkBall
{
	Default
	{
		DamageType "Shadow";
	}
	
	States
  {
  Spawn:
	ABI1 AABB 1 Bright
		{
		A_SpawnItemEx("AbyssalGhostProj",0,0,0,frandom(-1,1),frandom(-1,1),frandom(-1,1),0,SXF_TRANSFERSPRITEFRAME|SXF_TRANSFERSCALE);
		A_SpawnItemEx("AbyssalSavageDarkBallTrail",0,0,0,frandom(-0.5,0.5),frandom(-0.5,0.5),frandom(-0.5,0.5),0,SXF_TRANSFERRENDERSTYLE);
		A_SpawnParticle(GetParticleColor(),SPF_FULLBRIGHT|SPF_RELATIVE,random(8,16),frandom(4,8),0,frandom(-6,0),frandom(-6,6),frandom(-6,6),frandom(-2,0),0,frandom(0,3),0,0,-0.03,1,-1,-0.6);
		}
	Loop;
  Death:
	TNT1 A 0 
		{
		for(user_fx = 0;user_fx<=20;user_fx++)
			A_SpawnParticle(GetParticleColor(),SPF_FULLBRIGHT|SPF_RELATIVE,random(10,17),frandom(12,14),random(0,360),frandom(0,6),0,frandom(-6,6),frandom(0,6),0,frandom(-6,6),0,0,0,1,-1,-0.75);
		}
	ABI1 CCCDDDEEEFFF 1 Bright 
		{
		A_SpawnItemEx("AbyssalGhostProj",0,0,0,frandom(-1,1),frandom(-1,1),frandom(-1,1),0,SXF_TRANSFERSPRITEFRAME|SXF_TRANSFERSCALE);
		A_SetScale(Scale.X+0.08);
		}
	Stop;
	}
}
Class AbyssalSavageDarkBeamRPG : AbyssalSavageDarkBeam replaces AbyssalSavageDarkBeam
{
	Default
	{
		DamageType "Shadow";
	}
}
Class AbyssalSavagePhaseBallRPG : AbyssalSavagePhaseBall replaces AbyssalSavagePhaseBall
{
	Default
	{
		DamageType "Shadow";
	}
}
Class AbyssalSavageStrongBallRPG : AbyssalSavageStrongBall replaces AbyssalSavageStrongBall //
{
	Default
	{
		DamageType "Shadow";
		DamageType "Fire";
	}
  
  States
  {
  Spawn:
	ABI2 ABC 2 Bright
		{
		A_SpawnItemEx("BlueFireProjectileTrail2",frandom(-6,0),frandom(-6,6),frandom(-2,6),0,0,frandom(0,3),0,0,128);
		A_SpawnItemEx("AbyssalSavageBallTrail",0,0,0,frandom(-0.5,0.5),frandom(-0.5,0.5),frandom(-0.5,0.5));
		for(user_fx = 0;user_fx<2;user_fx++)
			A_SpawnParticle(GetParticleColor(),SPF_FULLBRIGHT|SPF_RELATIVE,random(8,16),frandom(4,8),0,frandom(-6,0),frandom(-8,8),frandom(-8,8),frandom(-4,0),0,frandom(0,4),0,0,-0.03,1,-1,-0.6);
		}
	Loop;
  Death:
	TNT1 A 0 
		{
		A_Explode(40,100,0,0,60);
		A_SpawnItemEx("BlueBossDeathExplode");
		A_SpawnItemEx("BlueFireProjectileExplode");
		for(user_fx = 0;user_fx<=20;user_fx++)
			A_SpawnParticle(GetParticleColor(),SPF_FULLBRIGHT|SPF_RELATIVE,random(10,17),frandom(12,14),random(0,360),frandom(0,6),0,frandom(-6,6),frandom(0,6),0,frandom(-6,6),0,0,0,1,-1,-0.75);
		}
	ABI2 DEFGH 2 Bright;
	Stop;
	}
}
Class AbyssalSavageFlareRPG : AbyssalSavageFlare replaces AbyssalSavageFlare //
{
	Default
	{
		DamageType "Shadow";
		DamageType "Fire";
	}
	States
  {
  Spawn:
	ABI3 AABB 2
		{
		A_ChangeVelocity(1,frandom(-3,3),frandom(-1,1),CVF_RELATIVE);
		A_SpawnItemEx("BlueFireProjectileTrail",frandom(-3,0),frandom(-3,3),frandom(-1,3),0,0,frandom(0,1.5),0,0,128);
		A_SpawnParticle(GetParticleColor(),SPF_FULLBRIGHT|SPF_RELATIVE,random(8,16),frandom(4,8),0,frandom(-6,0),frandom(-3,3),frandom(-3,3),frandom(-2,0),0,frandom(0,3),0,0,-0.03,1,-1,-0.6);
		}
	Loop;
  Death:
	TNT1 A 0
		{
		A_SpawnItemEx("BlueFireProjectileExplode");
		for(user_fx = 0;user_fx<=15;user_fx++)
			A_SpawnParticle(GetParticleColor(),SPF_FULLBRIGHT|SPF_RELATIVE,random(10,17),frandom(12,14),random(0,360),frandom(0,6),0,frandom(-6,6),frandom(0,6),0,frandom(-6,6),0,0,0,1,-1,-0.75);
		}
	ABI3 CDEFG 3;
	Stop;
	}
}
Class AbyssalSavageDarkMeleeRPG : AbyssalSavageDarkMelee replaces AbyssalSavageDarkMelee
{
	Default
	{
		DamageType "Shadow";
	}
}
Class AbyssalSavageMeleeRPG : AbyssalSavageMelee replaces AbyssalSavageMelee
{
	Default
	{
		DamageType "Shadow";
		DamageType "Fire";
	}
}

//Pinkys
Class MagmaDemonRPG : MagmaDemon replaces MagmaDemon {}
Class PandMaulerRPG : PandMauler replaces PandMauler {}
Class MechDemonRPG : MechDemon replaces MechDemon {}
Class NewDemonRPG : NewDemon replaces NewDemon {}

Class BloodFiendRPG : BloodFiend replaces BloodFiend 
{
	Default
	{
		DamageType "Slime";
	}
}
Class BloodFiendBileDRPG : BloodFiendBile replaces BloodFiendBile
{
	Default
	{
		DamageType "Slime";
	}
}

Class ChaosFlayerRPG : ChaosFlayer replaces ChaosFlayer
{
	Default
	{
		DamageType "Radiation";
		DamageFactor "Radiation", 0.2;
	}
}
Class ChaosFlayerProjectileDRPG : ChaosFlayerProjectile replaces ChaosFlayerProjectile
{
	Default
	{
		DamageType "Radiation";
	}
}

//Spectres
Class DarkbeastRPG : Darkbeast replaces Darkbeast {}
Class DelusionRPG : Delusion replaces Delusion {}
Class NightmareDemonRPG : NightmareDemon replaces NightmareDemon {}
Class NewSpectreRPG : NewSpectre replaces NewSpectre {}
Class WretchRPG : Wretch replaces Wretch {}

Class ChaosOphydianRPG : ChaosOphydian replaces ChaosOphydian
{
	Default
	{
		DamageType "Radiation";
		DamageFactor "Radiation", 0.2;
	}
}
Class OphydianHomingFluxDRPG : OphydianHomingFlux replaces OphydianHomingFlux
{
	Default
	{
		DamageType "Radiation";
	}
}

//Lost Souls
Class NewLostSoulRPG : NewLostSoul replaces NewLostSoul
{
  States
  {
  Spawn:
  	TNT1 A 0 NoDelay 
					{
					if ( GetLevelSpawnTime() == 0 ) {													//If LostSoul spawn on the map...
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOAURA | MF_NODROPS);				//...player got XP when kill it, else...
					} else {
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOXP | MF_NOAURA | MF_NODROPS);	//...don`t get XP lol.
						ClearCounters();
					}
					}
	TNT1 A 0 
		{
		A_SpawnItemEx("LostSoulFireSpawner",0,0,0,0,0,0,0,SXF_SETMASTER);
		soul_firespawned = 1;
		}
  Idle:
		SKUL AB 2 A_Look();
		Goto Super::Idle;
  See:
  	TNT1 A 0 
					{
					if ( GetLevelSpawnTime() == 0 ) {
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOAURA | MF_NODROPS);
					} else {
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOXP | MF_NOAURA | MF_NODROPS);
						ClearCounters();
					}
					}
		SKUL AB 2 A_Chase();
		Goto Super::See;
  }
}

Class PhantasmRPG : Phantasm replaces Phantasm
{
  States
  {
  Spawn:
    TNT1 A 0 NoDelay 
					{
					if ( GetLevelSpawnTime() == 0 ) {													//If LostSoul spawn on the map...
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOAURA | MF_NODROPS);				//...player got XP when kill it, else...
					} else {
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOXP | MF_NOAURA | MF_NODROPS);	//...don`t get XP lol
						ClearCounters();
					}
					}
	TNT1 A 0
		{
		A_SpawnItemEx("PhantasmFireSpawner",0,0,0,0,0,0,0,SXF_SETMASTER);
		soul_firespawned = 1;
		}
  Idle:
    PHNT AB 2 A_Look;
    Loop;
  See:
    TNT1 A 0 
					{
					if ( GetLevelSpawnTime() == 0 ) {													//If LostSoul spawn on the map...
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOAURA | MF_NODROPS);				//...player got XP when kill it, else...
					} else {
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOXP | MF_NOAURA | MF_NODROPS);	//...don`t get XP lol
						ClearCounters();
					}
					}
	PHNT AB 2 A_Chase();
    Loop;
  }
}

Class SoullessDroneRPG : SoullessDrone replaces SoullessDrone
{
  States
  {
  Spawn:
    TNT1 A 0 NoDelay 
					{
					if ( GetLevelSpawnTime() == 0 ) {													//If LostSoul spawn on the map...
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOAURA | MF_NODROPS);				//...player got XP when kill it, else...
					} else {
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOXP | MF_NOAURA | MF_NODROPS);	//...don`t get XP lol
						ClearCounters();
					}
					}
	TNT1 A 0
		{
		A_SpawnItemEx("SoullessDroneEffectSpawner",0,0,0,0,0,0,0,SXF_SETMASTER);
		soul_firespawned = 1;
		}
  Idle:
    DRON A 2 A_Look;
    Loop;
  See:
    TNT1 A 0 
					{
					if ( GetLevelSpawnTime() == 0 ) {													//If LostSoul spawn on the map...
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOAURA | MF_NODROPS);				//...player got XP when kill it, else...
					} else {
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOXP | MF_NOAURA | MF_NODROPS);	//...don`t get XP lol
						ClearCounters();
					}
					}
    DRON A 2 A_Chase();
    Loop;
  }
}

Class PandWraithRPG : PandWraith replaces PandWraith
{
  States
  {
  Spawn:
    TNT1 A 0 NoDelay 
					{
					if ( GetLevelSpawnTime() == 0 ) {													//If LostSoul spawn on the map...
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOAURA | MF_NODROPS);				//...player got XP when kill it, else...
					} else {
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOXP | MF_NOAURA | MF_NODROPS);	//...don`t get XP lol
						ClearCounters();
					}
					}
	TNT1 A 0
		{
		A_SpawnItemEx("WraithEffectSpawner",0,0,0,0,0,0,0,SXF_SETMASTER);
		soul_firespawned = 1;
		}
  Idle:
    WRTH ABCD 2 A_Look;
    Loop;
  See:
    TNT1 A 0
					{
					if ( GetLevelSpawnTime() == 0 ) {													//If LostSoul spawn on the map...
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOAURA | MF_NODROPS);				//...player got XP when kill it, else...
					} else {
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOXP | MF_NOAURA | MF_NODROPS);	//...don`t get XP lol
						ClearCounters();
					}
					}
    WRTH ABCD 2 
		{
		A_Chase();
		if(A_Jump(2,"Null")) { return ResolveState("GoPhase"); } return ResolveState(null);
		}
    Loop;
  }
}

Class ChaosUmbraRPG : ChaosUmbra replaces ChaosUmbra
{
	Default
  {
	DamageType "Radiation";
	DamageFactor "Radiation", 0.2;
  }
  States
  {
  Spawn:
    TNT1 A 0 NoDelay 
					{
					if ( GetLevelSpawnTime() == 0 ) {													//If LostSoul spawn on the map...
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOAURA | MF_NODROPS);				//...player got XP when kill it, else...
					} else {
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOXP | MF_NOAURA | MF_NODROPS);	//...don`t get XP lol
						ClearCounters();
					}
					}
  Idle:
    CHMB A 2 A_Look();
    Loop;
  See:
    TNT1 A 0
					{
					if ( GetLevelSpawnTime() == 0 ) {													//If LostSoul spawn on the map...
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOAURA | MF_NODROPS);				//...player got XP when kill it, else...
					} else {
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOXP | MF_NOAURA | MF_NODROPS);	//...don`t get XP lol
						ClearCounters();
					}
					}
	TNT1 A 0 A_WeaveInit();
    CHMB A 2 A_UmbraChase();
    Wait;
  Dash:
	TNT1 A 0 
		{
		bNOPAIN = true;
		ThrustThing(Angle*256/360+randompick(64,192),16,0,0);
		}
	CHMB AAAAAAAAAAAAAAA 1 A_SpawnItemEx("GenericPhaseGhost",0,0,0,0,0,0,0,SXF_TRANSFERSCALE|SXF_TRANSFERSTENCILCOL|SXF_TRANSFERSPRITEFRAME);
	TNT1 A 0
		{
		bNOPAIN = false;
		A_Stop();
		}
	Goto See;
  }
}

Class ElectrosoulRPG : Electrosoul replaces Electrosoul
{
 States
  {
  Spawn:
    TNT1 A 0 NoDelay 
					{
					if ( GetLevelSpawnTime() == 0 ) {													//If LostSoul spawn on the map...
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOAURA | MF_NODROPS);				//...player got XP when kill it, else...
					} else {
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOXP | MF_NOAURA | MF_NODROPS);	//...don`t get XP lol
						ClearCounters();
					}
					}
	TNT1 A 0
		{
		A_SpawnItemEx("ElectrosoulEffectSpawner",0,0,0,0,0,0,0,SXF_SETMASTER);
		soul_firespawned = 1;
		}
  Idle:
	ELSL A 2 A_Look;
	Loop;
  See:
    TNT1 A 0
					{
					if ( GetLevelSpawnTime() == 0 ) {													//If LostSoul spawn on the map...
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOAURA | MF_NODROPS);				//...player got XP when kill it, else...
					} else {
						ACS_NamedExecuteAlways ("MonsterInit", 0, MF_NOXP | MF_NOAURA | MF_NODROPS);	//...don`t get XP lol
						ClearCounters();
					}
					}
	ELSL A 2 A_Chase();
	Loop;
  }
}

//Revevants
Class DarkRevenantRPG : DarkRevenant replaces DarkRevenant {}
Class MissileRevenantRPG : MissileRevenant replaces MissileRevenant {}
Class NewRevenantRPG : NewRevenant replaces NewRevenant {}
Class ApollyonRPG : Apollyon replaces Apollyon {}

Class PandSentinelRPG : PandSentinel replaces PandSentinel
{
	Default
	{
		DropItem "DRPGShellSpawner", 128;
		DropItem "DRPGRocketAmmoSpawner", 128;
	}
}

Class SentryRPG : Sentry replaces Sentry
{
	Default
	{
		DropItem "DRPGShellSpawner";
	}
}

Class ChaosHollowRPG : ChaosHollow replaces ChaosHollow
{
	Default
	{
		DropItem "DRPGCellSpawner", 128;
		DamageFactor "Radiation", 0.2;
	}
}
Class HollowBallDRPG : HollowBall replaces HollowBall
{
	Default
	{
		DamageType "Radiation";
	}
}
Class HollowSkullBallDRPG : HollowSkullBall replaces HollowSkullBall
{
	Default
	{
		DamageType "Radiation";
	}
}

Class AncientGladiatorRPG : AncientGladiator replaces AncientGladiator
{
	Default
	{
		DamageType "Radiation";
		DamageFactor "Radiation", 0.4;
		
	}
}
Class DropsAncientGladiatorRPG : DropsAncientGladiator replaces DropsAncientGladiator
{
  Default
  {
  DropItem "DRPGHealthDropper", 256, 1;
  DropItem "DRPGBlueArmorRandomizer", 265, 5;
  }
}
Class GladiatorFireMissileDRPG : GladiatorFireMissile replaces GladiatorFireMissile
{
	Default
	{
		DamageType "Fire";
	}
}

//Cacos
Class AgathodemonRPG : Agathodemon replaces Agathodemon {}
Class AbaddonRPG : Abaddon replaces Abaddon {}

Class CacobotRPG : Cacobot replaces Cacobot
{
	Default
	{
		DropItem "DRPGCellSpawner";
		DropItem "DRPGCellSpawner", 32;
	}
}

Class NewCacodemonRPG : NewCacodemon replaces NewCacodemon {}
Class CrackodemonRPG : Crackodemon replaces Crackodemon {}
Class DarkCacodemonRPG : DarkCacodemon replaces DarkCacodemon {}

Class CacodemonBall2DRPG1 : PandProjectile
{
  Default
  {
  Radius 6;
  Height 8;
  Speed 15;
  FastSpeed 30;
  DamageFunction 20;
  Damagetype "Electric";
  Renderstyle "Add";
  PandProjectile.ParticleColors "FFD7BB", "FFA35B", "FF3F3F", "9F009B";
  SeeSound "Cacodemon/Ball";
  DeathSound "Cacodemon/BExplode";
  Decal "CacoScorch";
  }
  
  States
  {
  Spawn:
	BAL2 AABB 2 Bright 
		{
		A_SpawnItemEx("LightningProjectileTrail",frandom(-3,3),frandom(-3,3),frandom(-3,3),frandom(-2,2),frandom(-1,1),frandom(-1,1));
		A_SpawnItemEx("RedPlasmaLightningTrail",0,0,0,0,0,0,0,0,192);
		for(user_fx = 0;user_fx<=3;user_fx++)
			{
			A_SpawnParticle(GetParticleColor(),SPF_FULLBRIGHT|SPF_RELATIVE,random(7,15),frandom(6,8),0,frandom(-6,0),frandom(-6,6),frandom(-6,6),frandom(-2,2),frandom(-1,1),frandom(-1,1),0,0,0,1,-1,-1);
			}
		}
    Loop;
  Death:
	TNT1 A 0 
		{
		A_SpawnItemEx("LightningProjectileExplode");
		for(user_fx = 0;user_fx<=30;user_fx++)
			{
			A_SpawnParticle(GetParticleColor(),SPF_FULLBRIGHT|SPF_RELATIVE,random(10,17),frandom(12,14),random(0,360),frandom(0,6),0,frandom(-6,6),frandom(0,6),0,frandom(-6,6),0,0,0,1,-1,-1.2);
			}
		}
    BAL2 CDE 4 Bright;
    Stop;
	}
}
Class CacobotBallDRPG : CacodemonBall2DRPG1 replaces CacobotBall {}

Class WitherdemonRPG : Witherdemon replaces Witherdemon
{
	Default
	{
		DamageType "Slime";
	}
}
Class WitherdemonBallDRPG : WitherdemonBall replaces WitherdemonBall
{
	Default
	{
		DamageType "Slime";
	}
}

Class WitherdemonEyeProjectileDRPG : WitherdemonBallDRPG replaces WitherdemonEyeProjectile {}
Class WitherdemonEyeProjectile2DRPG : WitherdemonEyeProjectileDRPG replaces WitherdemonEyeProjectile2 {}
Class WitherdemonCometDRPG : PlaguelingPoisonPodDRPG replaces WitherdemonComet {}

Class ChaosStalkerRPG : ChaosStalker replaces ChaosStalker
{
	Default
	{
		DamageType "Radiation";
		DamageFactor "Radiation", 0.2;
	}
}

//Arachnotrons
Class ArachknightRPG : Arachknight replaces Arachknight
{
	Default
	{
		DropItem "DRPGCellSpawner", 128;
	}
}

Class NewArachnotronRPG : NewArachnotron replaces NewArachnotron {}
Class ArachnoscorcherRPG : Arachnoscorcher replaces Arachnoscorcher {}

Class AugmentedArachnotronRPG : AugmentedArachnotron replaces AugmentedArachnotron
{
	Default
	{
		DropItem "DRPGCellSpawner", 128;
	}
}

Class FusionArachnotronRPG : FusionArachnotron replaces FusionArachnotron
{
	Default
	{
		DropItem "DRPGCellSpawner", 128;
	}
}

Class NailgunArachnotronRPG : NailgunArachnotron replaces NailgunArachnotron
{
	Default
	{
		DropItem "DRPGClipSpawner", 128;
	}
}

Class WidowRPG : Widow replaces Widow
{
	Default
	{
		DropItem "DRPGCellSpawner", 128;
	}
}

Class ChaosObserverRPG : ChaosObserver replaces ChaosObserver
{
	Default
	{
		DropItem "DRPGCellSpawner", 128;
		DamageType "Radiation";
		DamageFactor "Radiation", 0.2;
	}
}
Class ObserverFluxDRPG : ObserverFlux replaces ObserverFlux
{
	Default
	{
		DamageType "Radiation";
	}
}

Class DemolitronRPG : Demolitron replaces Demolitron
{
	Default
	{
		DropItem "DRPGCellSpawner", 255;
		DropItem "DRPGClipSpawner", 180;
	}
}

//PainElementals
Class HadesElementalRPG : HadesElemental replaces HadesElemental {}
Class NewPainElementalRPG : NewPainElemental replaces NewPainElemental {}
Class PlasmaticElementalRPG : PlasmaticElemental replaces PlasmaticElemental {}

Class ChaosBeholderRPG : ChaosBeholder replaces ChaosBeholder
{
	Default
	{
		DropItem "DRPGCellSpawner", 128;
		DamageType "Radiation";
		DamageFactor "Radiation", 0.2;
	}
}
Class BeholderHomingBallDRPG : ChaosWarriorProjectileDRPG replaces BeholderHomingBall {}
Class BeholderMegaBallDRPG : BeholderMegaBall replaces BeholderMegaBall
{
	Default
	{
		DamageType "Radiation";
	}
}
Class BeholderStormBoltDRPG : BeholderStormBolt replaces BeholderStormBolt
{
	Default
	{
		DamageType "Radiation";
	}
}

Class DarkElementalRPG : DarkElemental replaces DarkElemental
{
	Default
	{
		DamageType "Slime";
	}
}
Class DarkElementalBileDRPG : BloodFiendBileDRPG replaces DarkElementalBile {}

Class HadesBallDRPG : CacodemonBall2DRPG1 replaces HadesBall {}
Class HadesBoltDRPG : HadesBallDRPG replaces HadesBolt {}
Class HadesBoltSmallDRPG : HadesBallDRPG replaces HadesBoltSmall {}
Class HadesSphereDRPG : HadesSphere replaces HadesSphere
{
	Default
	{
		Damagetype "Electric";
	}
}

Class AutomatonElementalRPG : AutomatonElemental replaces AutomatonElemental
{
	Default
	{
		DropItem "DRPGCellSpawner", 180;
	}
}
Class AutomatonElementalBombDRPG : AutomatonElementalBomb replaces AutomatonElementalBomb
{
	Default
	{
		DamageType "Fire";
	}
}

//Mancubuses
Class CorpulentRPG : Corpulent replaces Corpulent
{
	Default
	{
		DropItem "DRPGCellSpawner";
	}
}

Class HectebusRPG : Hectebus replaces Hectebus {}
Class MafibusRPG : Mafibus replaces Mafibus {}
Class MancubusRPG : Mancubus replaces Mancubus {}
Class MaxibusRPG : Maxibus replaces Maxibus {}

Class TankubusSpawner : RandomSpawner replaces Tankubus
{
	Default
	{
		DropItem "TankubusRPG";
	}
}

Class TankubusRPG : Mancubus
{
  Default
  {
  Radius 48;
  Height 64;
  Mass 1000;
  +BOSSDEATH;
  +DONTHARMSPECIES;
  +NOINFIGHTSPECIES;
  +NOPAIN;
  -SHOOTABLE;
  +NODAMAGE;
  -COUNTKILL;
  Speed 10;
  SeeSound "";
  ActiveSound "";
  }
  actor tank2;

  States
  {
  Spawn:
	TCU1 A 15 A_Look;
	TNT1 A 0
		{
		if(!tank2 || tank2.bISMONSTER == false)
			{
			ClearCounters();
			Destroy();
			}
		}
	Loop;
  See:
	TCU1 AB 3 
		{
		if(!tank2 || tank2.bISMONSTER == false)
			Destroy();
		A_Chase(null,null);
		}
	Loop;
	}

  override void PostBeginPlay()
	{
	Super.PostBeginPlay();
	tank2 = Spawn("Tankubus2RPG",pos);
	if(tank2) tank2.master = self;
	A_StartSound("Tankubus/Loop",7,CHANF_LOOPING);
	}
	
  override bool CanCollideWith(Actor other, bool passive)
	{
	if(other == tank2) return false;
	if(other.bNOBLOCKMAP == true) return false;
		return true;
	}
}

Class Tankubus2RPG : Mancubus
{
  Default
  {
  Radius 48;
  Height 64;
  Mass 1000;
  Health 1200;
  Speed 0;
  PainChance 40;
  PainChance "Bullet", 20;
  DamageFactor "Bullet", 0.5;
  DamageFactor "Plasma", 1.2;
  +BOSSDEATH;
  +DONTHARMSPECIES;
  +NOINFIGHTSPECIES;
  +THRUSPECIES;
  DropItem "DRPGShellSpawner", 64;
  DropItem "DRPGShellBoxSpawner", 64;
  DropItem "DRPGRocketAmmoSpawner", 64;
  DropItem "DRPGRocketBoxSpawner", 64;
  Obituary "%o was tracked down by a Tankubus.";
  SeeSound "Tankubus/Sight";
  ActiveSound "Tankubus/Active";
  PainSound "Tankubus/Pain";
  DeathSound "Tankubus/Death";
  PandMonster.TagColor "[i6]";
  Tag "Tankubus";
  }

  States
  {
  Spawn:
	TCU2 A 15 A_Look;
	TNT1 A 0
		{
		if(!master || master.bISMONSTER == false)
			{
			ClearCounters();
			Destroy();
			}
		}
	Loop;
  See:
	TCU2 A 4 
		{
		if(!master || master.bISMONSTER == false)
			{
			ClearCounters();
			Destroy();
			}
		A_Chase();
		}
	Loop;
  Missile:
	TNT1 A 0 A_SetSpeed(5,AAPTR_MASTER);
	TNT1 A 0 A_StartSound("Mafibus/Attack",2,pitch:0.9);
	TNT1 A 0 A_Jump(128,"Rockets");
	TCU2 AA 6 A_FaceTarget;
 StreetsweeperLoop:
	TCU2 B 4 Light("ZOMBIEATK")
		{
		A_FaceTarget();
		A_StartSound("RapidShotgun/Fire",1);
		A_SpawnItemEx("ShellCasing",10,34,47,Random(2,4),Random(3,6),Random(3,6),0);
		for(int a = 0;a<4;a++)
	  		A_SpawnProjectile("EnemyBulletTracer",50,31,frandom(-10,10),CMF_OFFSETPITCH|CMF_TRACKOWNER,frandom(-5,5));
		}
	TCU2 C 4 Light("ZOMBIEATK")
		{
		A_FaceTarget();
		A_StartSound("RapidShotgun/Fire",1);
		A_SpawnItemEx("ShellCasing",10,-34,47,Random(2,4),Random(-6,-3),Random(3,6),0);
		for(int a = 0;a<4;a++)
	  		A_SpawnProjectile("EnemyBulletTracer",50,-31,frandom(-10,10),CMF_OFFSETPITCH|CMF_TRACKOWNER,frandom(-5,5));
		}
	TNT1 A 0 A_MonsterRefire(0,"AttackEnd");
	Loop;
  Rockets:
	TCU2 AA 8 A_FaceTarget;
	TCU2 D 6 Bright
		{
		A_FaceTarget();
		A_SpawnProjectile("NobleRocket",70,45,-5);
		A_SpawnProjectile("NobleRocket",70,-45,5);
		}
	TCU2 A 6 A_FaceTarget;
	TCU2 E 6 Bright
		{
		A_FaceTarget();
		A_SpawnProjectile("NobleRocket",60,45,-5);
		A_SpawnProjectile("NobleRocket",60,-45,5);
		}
	TCU2 A 6 A_FaceTarget;
	TCU2 F 6 Bright
		{
		A_FaceTarget();
		A_SpawnProjectile("NobleRocket",50,45,-5);
		A_SpawnProjectile("NobleRocket",50,-45,5);
		}
  AttackEnd:
	TCU2 A 10 A_SetSpeed(10,AAPTR_MASTER);
	Goto See;
  Pain:
	TCU2 A 3 A_SetSpeed(10,AAPTR_MASTER);
	TCU2 A 3 A_Pain;
	Goto See;
  Death:
  XDeath:
	TNT1 A 0 A_RemoveMaster(RMVF_EVERYTHING);
	TCU3 A 6;
	TCU3 B 6 A_Scream;
	TCU3 C 6 A_NoBlocking;
	TCU3 DEFG 6
		{
		A_SpawnItemEx("NormalBossDeathExplode",random(-18,18),random(-18,18),random(15,40),0,0,0,0,0,0);
		A_PandSmallGib();
		}
	TCU3 HIJK 6;
	TCU3 L -1 A_BossDeath;
	Stop;
  Raise:
	Stop;
	}
  
  override void PostBeginPlay()
	{
	A_Warp(AAPTR_MASTER,0,0,0,0,WARPF_INTERPOLATE);
	}
	
  override void Tick()
	{
	Super.Tick();
	A_Warp(AAPTR_MASTER,0,0,0,0,WARPF_INTERPOLATE|WARPF_NOCHECKPOSITION|WARPF_USECALLERANGLE);
	}
}

Class ChaosPylonRPG : ChaosPylon replaces ChaosPylon
{
	Default
	{
		DropItem "DRPGCellSpawner", 200;
		DamageFactor "Radiation", 0.2;
	}
}

//Knights
Class CyberHellKnightRPG : CyberHellKnight replaces CyberHellKnight
{
	Default
	{
		DropItem "DRPGRocketAmmoSpawner";
	}
}

Class CyberHellViscountRPG : CyberHellViscount replaces CyberHellViscount
{
	Default
	{
		DropItem "DRPGPlasmaRifle", 32;
		DropItem "DRPGCellPackSpawner", 224;
	}
}

Class NewHellKnightRPG : NewHellKnight replaces NewHellKnight {}
Class HellViscountRPG : HellViscount replaces HellViscount {}
Class HellWardenRPG : HellWarden replaces HellWarden {}
Class InfernobleRPG : Infernoble replaces Infernoble {}

Class HellPrinceAschRPG : HellPrinceAsch replaces HellPrinceAsch
{
	Default
	{
		DamageFactor "Radiation", 0.4;
	}
}

Class DropsHellPrinceRPG : DropsHellPrince replaces DropsHellPrince
{
  Default
  {
  DropItem "DRPGHealthDropper", 256, 1;
  DropItem "DRPGBlueArmorRandomizer", 256, 1;
  }
}

//Barons
Class AfritRPG : Afrit replaces Afrit {}
Class ArchonOfHellRPG : ArchonOfHell replaces ArchonOfHell {}
Class NewBaronOfHellRPG : NewBaronOfHell replaces NewBaronOfHell {}
Class BruiserDemonRPG : BruiserDemon replaces BruiserDemon {}

Class CyberwardenRPG : Cyberwarden replaces Cyberwarden
{
	Default
	{
		DropItem "DRPGRocketLauncher", 32;
		DropItem "DRPGRocketAmmoSpawner", 192;
		DropItem "DRPGRocketBoxSpawner", 32;
	}
}

Class CybruiserRPG : Cybruiser replaces Cybruiser
{
	Default
	{
		DropItem "DRPGRocketLauncher", 32;
		DropItem "DRPGRocketAmmoSpawner", 224;
	}
}

Class DarkCybruiserRPG : DarkCybruiser replaces DarkCybruiser
{
	Default
	{
		DropItem "DRPGPlasmaRifle", 32;
		DropItem "DRPGCellSpawner", 192;
		DropItem "DRPGCellPackSpawner", 32;
	}
}

Class PyrobruiserRPG : Pyrobruiser replaces Pyrobruiser
{
	Default
	{
		DropItem "DRPGRocketLauncher", 32;
		DropItem "DRPGRocketAmmoSpawner", 192;
		DropItem "DRPGRocketBoxSpawner", 32;
	}
}

Class CydestructorRPG : Cydestructor replaces Cydestructor
{
	Default
	{
		DropItem "DRPGRocketLauncher", 32;
		DropItem "DRPGRocketAmmoSpawner", 192;
		DropItem "DRPGRocketBoxSpawner", 32;
		DropItem "DRPGBlueArmorRandomizer", 128;
	}
}

//Viles
Class NewArchvileRPG : NewArchvile replaces NewArchvile {}
Class DarkVileRPG : DarkVile replaces DarkVile {}
Class DiabolistRPG : Diabolist replaces Diabolist {}
//Class PulsedemonRPG : Pulsedemon replaces Pulsedemon {} //Not implemented in 2.4.1
Class DistortionistRPG : Distortionist replaces Distortionist
{
	Default
	{
		DropItem "DRPGCellSpawner";
	}
}

Class InfernalistRPG : Infernalist replaces Infernalist
{
	Default
	{
		DropItem "DRPGCellSpawner";
	}
}

Class CryptekRPG : Cryptek replaces Cryptek
{
	Default
	{
		DropItem "DRPGCellSpawner";
		DamageType "Radiation";
		DamageFactor "Radiation", 0;
	}
}
Class CryptekEnergyBallDRPG : CryptekEnergyBall replaces CryptekEnergyBall
{
	Default
	{
		DamageType "Radiation";
	}
}
Class CryptekBigEnergyBallDRPG : CryptekEnergyBallDRPG replaces CryptekBigEnergyBall {}
Class CryptekElectricFieldDRPG : CryptekElectricField replaces CryptekElectricField
{
	Default
	{
		DamageType "Radiation";
	}
}

//Patch
Class Epic2AlienRPG : Epic2Alien replaces Epic2Alien
{
	Default
	{
		DropItem "DRPGClipSpawner";
	}
}

Class Scythe2AfritRPG : Scythe2Afrit replaces Scythe2Afrit {}
Class Scythe2MarineRPG : Scythe2Marine replaces Scythe2Marine
{
	Default
	{
		DropItem "DRPGCellSpawner";
	}
}


//Cybies
Class AnnihilatorRPG : Annihilator replaces Annihilator
{
	Default
	{
		DropItem "DRPGRocketBoxSpawner";
		DropItem "DRPGRocketBoxSpawner", 32;
	}
}

Class NewCyberdemonRPG : NewCyberdemon replaces NewCyberdemon {}
//Class HellfireAvatarRPG : HellfireAvatar replaces HellfireAvatar {} //Not implemented

Class DarkCardinalRPG : DarkCardinal replaces DarkCardinal
{
	Default
	{
		DropItem "DRPGCellPackSpawner";
		DropItem "DRPGCellPackSpawner", 32;
	}
}

Class DarkCyberdemonRPG : DarkCyberdemon replaces DarkCyberdemon
{
	Default
	{
		DropItem "DRPGCellPackSpawner";
		DropItem "DRPGCellPackSpawner", 32;
	}
}

Class EradicatorRPG : Eradicator replaces Eradicator
{
	Default
	{
		DropItem "DRPGCellPackSpawner";
		DropItem "DRPGCellPackSpawner", 32;
	}
}

Class PlamexisRPG : Plamexis replaces Plamexis
{
	Default
	{
		DropItem "DRPGCellPackSpawner";
		DropItem "DRPGCellPackSpawner", 32;
	}
}

Class TerminatorRPG : Terminator replaces Terminator
{
	Default
	{
		DropItem "DRPGCellPackSpawner";
		DropItem "DRPGCellPackSpawner", 32;
	}
}

Class DreamweaverRPG : Dreamweaver replaces Dreamweaver //I dreamt about a fucking blue Terminator, one year later and it's in Pandemonia now
{
	Default
	{
		DropItem "DRPGCellPackSpawner";
		DropItem "DRPGCellPackSpawner", 32;
		DamageFactor "Fire", 1.5;
	}
}

Class ChaosEcclesiarchRPG : ChaosEcclesiarch replaces ChaosEcclesiarch
{
	Default
	{
		DropItem "DRPGCellPackSpawner", 256, 100;
		DropItem "DRPGCellSpawner", 128, 20;
		DropItem "DRPGBFG9000Spawner", 256;
		DropItem "DRPGSoulSphereRandomizer", 200;
		DamageFactor "Radiation", 0.2;
		DamageFactor "Fire", 0.8;
		DamageType "Radiation";
	}
}

//Masterminds
Class ArachnophyteRPG : Arachnophyte replaces Arachnophyte
{
	Default
	{
		DropItem "DRPGClipBoxSpawner", 128;
		DropItem "DRPGCellPackSpawner", 128;
	}
}

Class DemolisherRPG : Demolisher replaces Demolisher
{
	Default
	{
		DropItem "DRPGCellPackSpawner", 128;
	}
}

Class NewSpiderMastermindRPG : NewSpiderMastermind replaces NewSpiderMastermind {}
Class SpiderMastersparkRPG : SpiderMasterspark replaces SpiderMasterspark
{
	Default
	{
		DropItem "DRPGCellPackSpawner", 128;
	}
}

Class ChaosMonolithRPG : ChaosMonolith replaces ChaosMonolith
{
	Default
	{
		DropItem "DRPGCellPackSpawner";
		DropItem "DRPGBFG9000Spawner", 256;
		DropItem "DRPGSoulSphereRandomizer", 100;
		DamageType "Radiation";
		DamageFactor "Radiation", 0.2;
	}
}
Class MonolithHomingBFGDRPG : MonolithHomingBFG replaces MonolithHomingBFG
{
	Default
	{
		DamageType "Radiation";
	}
}
Class MonolithHomingBFG2DRPG : MonolithHomingBFGDRPG replaces MonolithHomingBFG2 {}
Class MonolithFluxBallDRPG : MonolithFluxBall replaces MonolithFluxBall
{
	Default
	{
		DamageType "Radiation";
	}
}
Class MonolithHomingFluxDRPG : MonolithFluxBallDRPG replaces MonolithHomingFlux {}

Class MonolithDeployerRPG : MonolithDeployer replaces MonolithDeployer
{
  actor a;
  int spawnchance;
  States
  {
  Spawn:
	TNT1 A 1 NoDelay 
		{
		A_Warp(AAPTR_DEFAULT,0,0,-1000);
		A_SpawnItemEx("MonolithDeployerEffectSpawner");
		A_StartSound("Monolith/Spawn",1);
		MonolithEnemySpawn();
		}
	Stop;
	}
  void MonolithEnemySpawn()
	{
	spawnchance = random(1,55);
	if(spawnchance <= 10)
		{
		for(int i = 0;i<4;i++)
			SpawnChaosEnemy("ChaosWarriorRPG",1);
		}
	else if(spawnchance <= 20)
		{
		for(int i = 0;i<3;i++)
			SpawnChaosEnemy("ChaosImpRPG",1);
		}
	else if(spawnchance <= 30)
		{
		for(int i = 0;i<3;i++)
			SpawnChaosEnemy("ChaosDeathmarkRPG",1);
		}
	else if(spawnchance <= 35)
		{
		for(int i = 0;i<2;i++)
			SpawnChaosEnemy("ChaosFlayerRPG",1);
		}
	else if(spawnchance <= 40)
		{
		for(int i = 0;i<2;i++)
			SpawnChaosEnemy("ChaosOphydianRPG",1);
		}
	else if(spawnchance <= 45)
		{
		for(int i = 0;i<2;i++)
			SpawnChaosEnemy("ChaosHollowRPG",1);
		}
	else if(spawnchance <= 49)
		SpawnChaosEnemy("ChaosStalkerRPG");
	else if(spawnchance <= 51)
		SpawnChaosEnemy("ChaosPylonRPG");
	else if(spawnchance <= 53)
		SpawnChaosEnemy("ChaosObserverRPG");
	else if(spawnchance <= 54)
		SpawnChaosEnemy("ChaosBeholderRPG");
	else if(spawnchance <= 55)
		SpawnChaosEnemy("CryptekRPG");
	}
  void SpawnChaosEnemy(name enemy, bool shouldthrust = 0)
	{
	a = Spawn(enemy,self.pos);
	a.A_SetAngle(random(0,360));
	if(shouldthrust)
		a.Thrust(random(2,6),a.angle);
	a.master = master;
	a.Species = "Chaos";
	master.GiveInventory("MonolithSpawnCount",1);
	}
}

Class MonolithEnemySpawnerRPG : MonolithEnemySpawner replaces MonolithEnemySpawner
{
	Default
	{
		DropItem "ChaosWarriorRPG", 256, 10;
		DropItem "ChaosDeathmarkRPG", 256, 10;
		DropItem "ChaosImpRPG", 256, 10;
		DropItem "ChaosHollowRPG", 256, 5;
		DropItem "ChaosObserverRPG", 256, 3;
		DropItem "ChaosBeholderRPG", 256, 1;
		DropItem "CryptekRPG", 256, 1;
	}
}

//SpecialsBosses
Class GeneralRoasterockRPG : GeneralRoasterock replaces GeneralRoasterock
{
	Default
	{
		DropItem "DRPGBackpackRandomizer";
	}
}

Class MyrkuraRPG : Myrkura replaces Myrkura
{
	Default
	{
		DropItem "DRPGBackpackRandomizer";
	}
}

//Specials Boss spawners
Class PandPursuerBossSpawnerRPG : PandPursuerBossSpawner replaces PandPursuerBossSpawner
{
  States
  {
  Spawn:
	TNT1 A 0 NoDelay
		{
		let g = PandGlobalVariables.Get();
		if(g && g.PickedBoss == 2)
			{
			if(pan_debugmsg)
				Console.printf("Myrkura");
			Destroy();
			}
		else
			{
			if(pan_debugmsg)
				Console.printf("Roasterock");
			SetStateLabel("SpawnRoasterock");
			targetplayer = target;
			}
		}
	Stop;
  SpawnRoasterock:
	TNT1 A 1
		{
		let RoastWait = random(1050,10500);
		A_SetTics(RoastWait);
		if(pan_debugmsg)
			Console.Printf("Roasterock spawning in: %d", RoastWait);
		}
	TNT1 A 100 SetOrigin(targetplayer.pos,0);
	TNT1 A 0
		{
		bFLATSPRITE = true;
		bROLLSPRITE = true;
		A_SetScale(0.05);
		A_SetRenderstyle(0.0,STYLE_ADD);
		A_StartSound("Roasterock/Spawn",1);
		effectloop = 0;
		}
  SpawnRoasterockLoop:
	SPKR A 1 Bright Light("RBFGBALL")
		{
		if(effectloop == 80)
			SetStateLabel("SpawnRoasterockFinish");
		if(Scale.x < 1.5)
			A_SetScale(Scale.x+0.03);
		A_FadeIn(0.08);
		A_SetRoll(roll+15,SPF_INTERPOLATE);
		for(user_fx = 0;user_fx<=2;user_fx++)
			{
			user_pcolor = random(1,4);
			if(user_pcolor == 1) { A_SpawnParticle("FF4949",SPF_FULLBRIGHT|SPF_RELATIVE,25,frandom(15,16),random(0,360),frandom(0,20),0,frandom(0,4),0,0,frandom(2,3),0,0,0,1,-1,-1); }
			else if(user_pcolor == 2) { A_SpawnParticle("FF2424",SPF_FULLBRIGHT|SPF_RELATIVE,25,frandom(15,16),random(0,360),frandom(0,20),0,frandom(0,4),0,0,frandom(2,3),0,0,0,1,-1,-1); }
			else if(user_pcolor == 3) { A_SpawnParticle("FF6D6D",SPF_FULLBRIGHT|SPF_RELATIVE,25,frandom(15,16),random(0,360),frandom(0,20),0,frandom(0,4),0,0,frandom(2,3),0,0,0,1,-1,-1); }
			else if(user_pcolor == 4) { A_SpawnParticle("FF0000",SPF_FULLBRIGHT|SPF_RELATIVE,25,frandom(15,16),random(0,360),frandom(0,20),0,frandom(0,4),0,0,frandom(2,3),0,0,0,1,-1,-1); }
			}
		effectloop++;
		}
	Wait;
  SpawnRoasterockFinish:
	TNT1 A 0
		{
		let rockerstocker = Spawn("GeneralRoasterockRPG",pos);
		Spawn("TeleportFog",pos);
		let g = PandGlobalVariables.Get();
		let BeatRoast = g.BeatRoast;
		if(BeatRoast >= 5)
			rockerstocker.health = 5000;
		else
			rockerstocker.health = (rockerstocker.health) + (BeatRoast*500);
		if(pan_bossmusic)
			S_ChangeMusic("P_ROAST");
		}	
	SPKR A 1 Bright Light("RBFGBALL")
		{
		A_SetRoll(roll+20,SPF_INTERPOLATE);
		A_FadeOut(0.1);
		A_SetScale(Scale.x-0.03);
		effectloop++;
		}
	Wait;
	}
}

Class MyrkuraSpawnerRPG : MyrkuraSpawner replaces MyrkuraSpawner
{
  States
  {
  Spawn:
	TNT1 A 0 NoDelay 
		{
		A_StartSound("Myrkura/Spawn2",15,0,1.0,0);
		A_QuakeEx(4,4,4,88,0,3500,"",QF_SCALEUP,0,1500,0,256);
		if(pan_bossmusic)
			S_ChangeMusic("P_MYRKRA");
		}
	MRC3 AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA 1
		{
		if(alpha < 1)
			A_FadeIn(0.03);
		if(Scale.X < 1)
			A_SetScale(Scale.X+0.025);
		for(user_fx = 0;user_fx<3;user_fx++)
			A_SpawnParticle(GetParticleColor(),SPF_FULLBRIGHT|SPF_RELATIVE,25,frandom(15,16),random(0,360),frandom(0,250),0,frandom(0,4),0,0,frandom(5,10),0,0,0,1,-1,-1);
		}
	MRC3 AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA 1
		{
		A_SpawnItemEx("RedFireProjectileTrail2",frandom(0,250),0,frandom(0,4),0,0,frandom(5,10),random(0,360));
		for(user_fx = 0;user_fx<3;user_fx++)
			A_SpawnParticle(GetParticleColor(),SPF_FULLBRIGHT|SPF_RELATIVE,25,frandom(15,16),random(0,360),frandom(0,250),0,frandom(0,4),0,0,frandom(5,10),0,0,0,1,-1,-1);
		}
	TNT1 A 0
		{
		A_QuakeEx(10,10,10,50,0,4500,"",QF_SCALEDOWN,0,0,0,256);
		A_StartSound("Myrkura/Spawn",1,0,1.0,0);
		for(fxloop = 0;fxloop<=360;fxloop = fxloop+3)
			A_SpawnItemEx("RedFireProjectileTrail2",265,0,random(0,4),0,0,frandom(2,6),fxloop);
		let mykura = Spawn("MyrkuraRPG",pos);
		if(target)
			mykura.target = target;
		}
	MRC3 A 1 A_FadeOut;
	Wait;
	}
}

//RandomSpawner of the Chaos event i think
Class ChaosRandomSpawnerRPG : ChaosRandomSpawner replaces ChaosRandomSpawner 			
{
  override void PostSpawn(Actor spawned)
    {
    if(spawned && spawned is "PandMonster")
        {
        let spawnspec = PandMonster(spawned).spec2;
        if(spawned.Health <= 300 && spawnspec == "Chaos")
            {
            for(int i = 0;i<2;i++)
			    SpawnChaosEnemy(spawned.GetClassName(),1);
            }
        else if(spawned.Health < 1000 && spawnspec == "Chaos")
            {
			SpawnChaosEnemy(spawned.GetClassName(),1);
            }
        else if(spawned.Health >= 1000 && random(1,4) == 1 && spawnspec == "Chaos")
            {
            MonolithEnemySpawn();
            }
        }
    }

  void MonolithEnemySpawn()
	{
	int spawnchance = random(1,50);
	if(spawnchance <= 10)
		{
		for(int i = 0;i<4;i++)
			SpawnChaosEnemy("ChaosWarriorRPG",1);
		}
	else if(spawnchance <= 20)
		{
		for(int i = 0;i<3;i++)
			SpawnChaosEnemy("ChaosImpRPG",1);
		}
	else if(spawnchance <= 30)
		{
		for(int i = 0;i<3;i++)
			SpawnChaosEnemy("ChaosDeathmarkRPG",1);
		}
	else if(spawnchance <= 35)
		{
		for(int i = 0;i<2;i++)
			SpawnChaosEnemy("ChaosFlayerRPG",1);
		}
	else if(spawnchance <= 40)
		{
		for(int i = 0;i<2;i++)
			SpawnChaosEnemy("ChaosOphydianRPG",1);
		}
	else if(spawnchance <= 45)
		{
		for(int i = 0;i<2;i++)
			SpawnChaosEnemy("ChaosHollowRPG",1);
		}
	else if(spawnchance <= 49)
		SpawnChaosEnemy("ChaosStalkerRPG");
	else if(spawnchance <= 51)
		SpawnChaosEnemy("ChaosPylonRPG");
	else if(spawnchance <= 53)
		SpawnChaosEnemy("ChaosObserverRPG");
	else if(spawnchance <= 54)
		SpawnChaosEnemy("ChaosBeholderRPG");
	else if(spawnchance <= 55)
		SpawnChaosEnemy("CryptekRPG");
	}

  void SpawnChaosEnemy(name enemy, bool shouldthrust = 0)
	{
	let a = Spawn(enemy,self.pos);
	a.A_SetAngle(random(0,360));
	if(shouldthrust)
		a.Thrust(random(2,6),a.angle);
	}
}

//Nassis replacer
Class ChaosCultistRPG : ChaosCultist replaces ChaosCultist
{
	Default
	{
		DamageType "Radiation";
		DamageFactor "Radiation", 0.5;
	}
}

Class ChaosAcolyteRPG : ChaosAcolyte replaces ChaosAcolyte
{
	Default
	{
		DamageType "Radiation";
		DamageFactor "Radiation", 0.5;
	}
}

// Calling the ACS MonsterInit script for monsters
class Pandemonia_MonsterInit : EventHandler
{
    override void WorldThingSpawned(WorldEvent e)
    {
        if (e.Thing && e.Thing.bIsMonster)
        {
			 static const string PandemoniaMonsters[] =
            {
                "LaserRifleZombieRPG", "PistolZombieRPG", "PlasmaPistolZombieRPG",											//Zombies
				"SpecOpsRiflemanRPG", "CinderZombieRPG",
				
               	"AutoShotgunZombieRPG", "ChaosWarriorRPG", "RocketZombieRPG", "ShotgunAdmiralRPG", 							//Shotgunners
				"NewShotgunguyRPG",	"SlugShotgunZombieRPG", "SpecOpsShotgunnerRPG", "SuperShotgunZombieRPG",
				"PyroZombieRPG",
				
				"AssaultGunnerRPG", "BFG10kWarriorRPG", "BFGCommandoRPG", "ChaingunZombieRPG", 								//Commandos
				"ChaosDeathmarkRPG", "NailborgCommandoRPG", "PlasmaRifleZombieRPG", "SpecOpsGunnerRPG", 
				"SpecOpsLieutenantRPG", "ScourgeZombieRPG", "InfernalCommandoRPG",
				
				"ChaosImpRPG",	"DarkImpRPG", "DevilRPG", "CyberImpRPG", "NewImpRPG", "PhaseImpRPG", 						//Imps
				"PlaguelingRPG", "ScoundrelRPG", "BrimstoneImpRPG", "AbyssalSavageRPG",
				
				"BloodFiendRPG", "ChaosFlayerRPG", "MagmaDemonRPG", "PandMaulerRPG", "MechDemonRPG", 						//Pinkys
				"NewDemonRPG",
				
				"ChaosOphydianRPG", "DarkbeastRPG", "DelusionRPG", "NightmareDemonRPG",	"NewSpectreRPG", 					//Spectres
				"WretchRPG",
				
				"ChaosUmbraRPG", "ElectrosoulRPG", "NewLostSoulRPG", "PhantasmRPG", "SoullessDroneRPG",						//Lost Souls
				"PandWraithRPG",			
				
				"ChaosHollowRPG", "DarkRevenantRPG", "MissileRevenantRPG", "NewRevenantRPG", "PandSentinelRPG",				//Revevants
				"SentryRPG", "ApollyonRPG",
				
				"AgathodemonRPG", "CacobotRPG", "NewCacodemonRPG", "CrackodemonRPG", "DarkCacodemonRPG",					//Cacodemons
				"WitherdemonRPG", "AbaddonRPG", "ChaosStalkerRPG", 
				
				"ArachknightRPG", "NewArachnotronRPG", "AugmentedArachnotronRPG", "ChaosObserverRPG", 						//Arachnotrons
				"FusionArachnotronRPG", "NailgunArachnotronRPG", "WidowRPG", "ArachnoscorcherRPG", 
				"DemolitronRPG",
				
				"ChaosBeholderRPG", "DarkElementalRPG", "HadesElementalRPG", "NewPainElementalRPG", 						//Pain Elementals
				"PlasmaticElementalRPG", "AutomatonElementalRPG",	
				
				"CorpulentRPG", "HectebusRPG", "MafibusRPG", "MancubusRPG", "MaxibusRPG", "Tankubus2RPG", 					//Mancubuses
				"ChaosPylonRPG",
				
				"CyberHellKnightRPG", "CyberHellViscountRPG", "NewHellKnightRPG", "HellViscountRPG", 						//Knights
				"HellWardenRPG", "InfernobleRPG",
				
				"AfritRPG", "ArchonOfHellRPG", "NewBaronOfHellRPG", "BruiserDemonRPG", "CyberwardenRPG", 					//Barons
				"CybruiserRPG",	"DarkCybruiserRPG", "PyrobruiserRPG", "CydestructorRPG",
				
				"NewArchvileRPG", "CryptekRPG", "DarkVileRPG", "DiabolistRPG", "DistortionistRPG", 							//Arch-Viles
				"InfernalistRPG",
				
				"ChaosCultistRPG", "ChaosAcolyteRPG",																		//Nazi
				
				"Epic2AlienRPG", "Scythe2AfritRPG", "Scythe2MarineRPG"														//Patch
            };
			
            static const string PandemoniaBosses[] =
            {
				"AncientGladiatorRPG", "HellPrinceAschRPG",																	//Normal Bosses
				
               	"AnnihilatorRPG", "NewCyberdemonRPG", "DarkCardinalRPG", "DarkCyberdemonRPG", "DreamweaverRPG",				//Cyberdemons
				"EradicatorRPG", "PlamexisRPG", "TerminatorRPG", "ChaosEcclesiarchRPG",
				
				"ArachnophyteRPG", "ChaosMonolithRPG", "DemolisherRPG", "NewSpiderMastermindRPG", 							//Masterminds
				"SpiderMastersparkRPG",	
				
				"MyrkuraRPG", "GeneralRoasterockRPG"																		//Special Bosses
			};

			static const string PandemoniaOtherThings[] =
            {
                "SayoriRPG" 																						//Other Things
            };

            for (int i = 0; i < PandemoniaMonsters.size(); i++)
            {
                if (e.Thing.GetClassName() == PandemoniaMonsters[i])
                {
                    if (e.Thing.master is "ChaosMonolithRPG")                        
						e.Thing.ACS_ScriptCall("MonsterInit", MF_NOXP | MF_NOAURA | MF_NODROPS);
                    else
                        e.Thing.ACS_ScriptCall("MonsterInit", 0);
                }
            }
				
			for (int i = 0; i < PandemoniaBosses.size(); i++)
				if (e.Thing.GetClassName() == PandemoniaBosses[i])
					e.Thing.ACS_ScriptCall("MonsterInit", MF_BOSS);
					
			for (int i = 0; i < PandemoniaOtherThings.size(); i++)
				if (e.Thing.GetClassName() == PandemoniaOtherThings[i])
					e.Thing.ACS_ScriptCall("MonsterInit", MF_NOXP | MF_NOAURA | MF_NODROPS);
        }
    }
}