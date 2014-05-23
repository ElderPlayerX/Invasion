// Copyright (C) 1999-2000 Id Software, Inc.
//
// bg_misc.c -- both games misc functions, all completely stateless

#include "q_shared.h"
#include "bg_public.h"

/*QUAKED item_***** (0 0 0) (-16 -16 -16) (16 16 16) suspended
DO NOT USE THIS CLASS, IT JUST HOLDS GENERAL INFORMATION.
The suspended flag will allow items to hang in the air, otherwise they are dropped to the next surface.

If an item is the target of another entity, it will not spawn in until fired.

An item fires all of its targets when it is picked up.  If the toucher can't carry it, the targets won't be fired.

"notfree" if set to 1, don't spawn in free for all games
"notteam" if set to 1, don't spawn in team games
"notsingle" if set to 1, don't spawn in single player games
"wait"	override the default wait before respawning.  -1 = never respawn automatically, which can be used with targeted spawning.
"random" random number of plus or minus seconds varied from the respawn time
"count" override quantity or duration on most items.
*/

//Too: here all models used by Invasion; they should be all preloaded by client
char *AlienModel[e_Selection_MaxRace+1] = { "nrk", "orbb", "klesk", "alien", NULL };
char *MaleMarineModel[e_Selection_MaxArmor+1] = { "razor", "grunt", "keel", "visor", NULL };
char *FemaleMarineModel[e_Selection_MaxArmor+1] = { "razor", "grunt", "keel", "visor", NULL };
//char *FemaleMarineModel[e_Selection_MaxArmor+1] = { "crash", "mynx", "lucy", "hunter", NULL };

gitem_t	bg_itemlist[] =
{
	{
		NULL,
		NULL,
		{ NULL,
		NULL,
		0, 0}
		,
/* icon */		NULL,
/* pickup */	NULL,
		0,
		0,
		0,
/* precache */ "",
/* sounds */ ""
	},	// leave index 0 alone

	//
	// ARMOR
	//

/*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 1
		"item_armor_shard",
		"sound/misc/ar1_pkup.wav",
		{ "models/powerups/armor/shard.md3",
		"models/powerups/armor/shard_sphere.md3",
		0, 0}
		,
/* icon */		"icons/iconr_shard",
/* pickup */	"Armor Shard",
		5,
		IT_ARMOR,
		0,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 2
		"item_armor_combat",
		"sound/misc/ar2_pkup.wav",
        { "models/powerups/armor/armor_yel.md3",
		0, 0, 0},
/* icon */		"icons/iconr_yellow",
/* pickup */	"Armor",
		50,
		IT_ARMOR,
		0,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 3
		"item_armor_body",
		"sound/misc/ar2_pkup.wav",
        { "models/powerups/armor/armor_red.md3",
		0, 0, 0},
/* icon */		"icons/iconr_red",
/* pickup */	"Heavy Armor",
		100,
		IT_ARMOR,
		0,
/* precache */ "",
/* sounds */ ""
	},

	//
	// health
	//
/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 4
		"item_health_small",
		"sound/items/s_health.wav",
        { "models/powerups/health/small_cross.md3",
		"models/powerups/health/small_sphere.md3",
		0, 0 },
/* icon */		"icons/iconh_green",
/* pickup */	"5 Health",
		5,
		IT_HEALTH,
		0,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 5
		"item_health",
		"sound/items/n_health.wav",
        { "models/powerups/health/medium_cross.md3",
		"models/powerups/health/medium_sphere.md3",
		0, 0 },
/* icon */		"icons/iconh_yellow",
/* pickup */	"25 Health",
		25,
		IT_HEALTH,
		0,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 6
		"item_health_large",
		"sound/items/l_health.wav",
        { "models/powerups/health/large_cross.md3",
		"models/powerups/health/large_sphere.md3",
		0, 0 },
/* icon */		"icons/iconh_red",
/* pickup */	"50 Health",
		50,
		IT_HEALTH,
		0,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 7
		"item_health_mega",
		"sound/items/m_health.wav",
        { "models/powerups/health/mega_cross.md3",
		"models/powerups/health/mega_sphere.md3",
		0, 0 },
/* icon */		"icons/iconh_mega",
/* pickup */	"Mega Health",
		100,
		IT_HEALTH,
		0,
/* precache */ "",
/* sounds */ ""
	},


	//
	// WEAPONS
	//

/*QUAKED weapon_gauntlet (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 8
		"weapon_gauntlet",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/crowbarg/gauntlet.md3",
		0, 0, 0},
/* icon */		"icons/iconw_gauntlet",
/* pickup */	"Crowbar",
		0,
		IT_WEAPON,
		WP_GAUNTLET,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 9
		"weapon_shotgun",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/shotgun/shotgun.md3",
		0, 0, 0},
/* icon */		"icons/iconw_shotgun",
/* pickup */	"Shotgun",
		10,
		IT_WEAPON,
		WP_SHOTGUN,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED weapon_machinegun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 10
		"weapon_machinegun",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/m41a-Invas/machinegun.md3",
		0, 0, 0},
/* icon */		"icons/iconw_machinegun",
/* pickup */	"Machinegun",
		40,
		IT_WEAPON,
		WP_MACHINEGUN,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 11
		"weapon_grenadelauncher",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/grenadel/grenadel.md3",
		0, 0, 0},
/* icon */		"icons/iconw_grenade",
/* pickup */	"Grenade Launcher",
		10,
		IT_WEAPON,
		WP_GRENADE_LAUNCHER,
/* precache */ "",
/* sounds */ "sound/weapons/grenade/hgrenb1a.wav sound/weapons/grenade/hgrenb2a.wav"
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 12
		"weapon_rocketlauncher",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/rocketl/rocketl.md3",
		0, 0, 0},
/* icon */		"icons/iconw_rocket",
/* pickup */	"Rocket Launcher",
		10,
		IT_WEAPON,
		WP_ROCKET_LAUNCHER,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED weapon_lightning (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 13
		"weapon_lightning",
		"sound/misc/w_pkup.wav",
        //{ "models/weapons2/lightning/lightning.md3",
        { "models/weapons2/gauntlet/gauntlet.md3",

		0, 0, 0},
/* icon */		"icons/iconw_lightning",
/* pickup */	"Lightning",		//Too: "Lightning Gun",
		100,
		IT_WEAPON,
		WP_LIGHTNING,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 14
		"weapon_railgun",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/railgun/railgun.md3",
		0, 0, 0},
/* icon */		"icons/iconw_railgun",
/* pickup */	"Sniper Rifle",
		10,
		IT_WEAPON,
		WP_RAILGUN,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED weapon_plasmagun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 15
		"weapon_plasmagun",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/plasmag/plasmag.md3",
		0, 0, 0},
/* icon */		"icons/iconw_plasma",
/* pickup */	"Acid Gun",
		50,
		IT_WEAPON,
		WP_PLASMAGUN,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED weapon_bfg (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 16
		"weapon_bfg",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/bfg/bfg.md3",
		0, 0, 0},
/* icon */		"icons/iconw_bfg",
/* pickup */	"BFG10K",
		20,
		IT_WEAPON,
		WP_BFG,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED weapon_grapplinghook (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 17
		"weapon_grapplinghook",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/grapple/grapple.md3",
		0, 0, 0},
/* icon */		"icons/iconw_grapple",
/* pickup */	"Grappling Hook",
		0,
		IT_WEAPON,
		WP_GRAPPLING_HOOK,
/* precache */ "",
/* sounds */ ""
	},

	//
	// AMMO ITEMS
	//

/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 18
		"ammo_shells",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/shotgunam.md3",
		0, 0, 0},
/* icon */		"icons/icona_shotgun",
/* pickup */	"Shells",
		10,
		IT_AMMO,
		WP_SHOTGUN,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 19
		"ammo_bullets",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/machinegunam.md3",
		0, 0, 0},
/* icon */		"icons/icona_machinegun",
/* pickup */	"Bullets",
		50,
		IT_AMMO,
		WP_MACHINEGUN,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 20
		"ammo_grenades",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/grenadeam.md3",
		0, 0, 0},
/* icon */		"icons/icona_grenade",
/* pickup */	"Grenades",
		5,
		IT_AMMO,
		WP_GRENADE_LAUNCHER,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 21
		"ammo_cells",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/plasmaam.md3",
		0, 0, 0},
/* icon */		"icons/icona_plasma",
/* pickup */	"Acid",
		30,
		IT_AMMO,
		WP_PLASMAGUN,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED ammo_lightning (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 22
		"ammo_lightning",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/lightningam.md3",
		0, 0, 0},
/* icon */		"icons/icona_lightning",
/* pickup */	"Lightning",
		60,
		IT_AMMO,
		WP_LIGHTNING,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 23
		"ammo_rockets",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/rocketam.md3",
		0, 0, 0},
/* icon */		"icons/icona_rocket",
/* pickup */	"Rockets",
		5,
		IT_AMMO,
		WP_ROCKET_LAUNCHER,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 24
		"ammo_slugs",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/machinegunam.md3",//"models/powerups/ammo/railgunam.md3",
		0, 0, 0},
/* icon */		"icons/icona_machinegun",//"icons/icona_railgun",
/* pickup */	"Slugs",
		10,
		IT_AMMO,
		WP_RAILGUN,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED ammo_bfg (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 25
		"ammo_bfg",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/bfgam.md3",
		0, 0, 0},
/* icon */		"icons/icona_bfg",
/* pickup */	"Bfg Ammo",
		15,
		IT_AMMO,
		WP_BFG,
/* precache */ "",
/* sounds */ ""
	},

	//
	// HOLDABLE ITEMS
	//
/*QUAKED holdable_teleporter (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 26
		"holdable_teleporter",
		"sound/items/holdable.wav",
        { "models/powerups/holdable/teleporter.md3",
		0, 0, 0},
/* icon */		"icons/teleporter",
/* pickup */	"Personal Teleporter",
		60,
		IT_HOLDABLE,
		HI_TELEPORTER,
/* precache */ "",
/* sounds */ ""
	},
/*QUAKED holdable_medkit (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 27
		"holdable_medkit",
		"sound/items/holdable.wav",
        {
		"models/powerups/holdable/medkit.md3",
		"models/powerups/holdable/medkit_sphere.md3",
		0, 0},
/* icon */		"icons/medkit",
/* pickup */	"Medkit",
		60,
		IT_HOLDABLE,
		HI_MEDKIT,
/* precache */ "",
/* sounds */ "sound/items/use_medkit.wav"
	},

	//
	// POWERUP ITEMS
	//
/*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 28
		"item_quad",
		"sound/items/quaddamage.wav",
        { "models/powerups/instant/quad.md3",
        "models/powerups/instant/quad_ring.md3",
		0, 0 },
/* icon */		"icons/quad",
/* pickup */	"Quad Damage",
		30,
		IT_POWERUP,
		PW_QUAD,
/* precache */ "",
/* sounds */ "sound/items/damage2.wav sound/items/damage3.wav"
	},

/*QUAKED item_enviro (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 29
		"item_enviro",
		"sound/items/protect.wav",
        { "models/powerups/instant/enviro.md3",
		"models/powerups/instant/enviro_ring.md3",
		0, 0 },
/* icon */		"icons/envirosuit",
/* pickup */	"Battle Suit",
		30,
		IT_POWERUP,
		PW_BATTLESUIT,
/* precache */ "",
/* sounds */ "sound/items/airout.wav sound/items/protect3.wav"
	},

/*QUAKED item_haste (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 30
		"item_haste",
		"sound/items/haste.wav",
        { "models/powerups/instant/haste.md3",
		"models/powerups/instant/haste_ring.md3",
		0, 0 },
/* icon */		"icons/haste",
/* pickup */	"Speed",
		30,
		IT_POWERUP,
		PW_HASTE,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED item_invis (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 31
		"item_invis",
		"sound/items/invisibility.wav",
        { "models/powerups/instant/invis.md3",
		"models/powerups/instant/invis_ring.md3",
		0, 0 },
/* icon */		"icons/invis",
/* pickup */	"Invisibility",
		30,
		IT_POWERUP,
		PW_INVIS,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED item_regen (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 32
		"item_regen",
		"sound/items/regeneration.wav",
        { "models/powerups/instant/regen.md3",
		"models/powerups/instant/regen_ring.md3",
		0, 0 },
/* icon */		"icons/regen",
/* pickup */	"Regeneration",
		30,
		IT_POWERUP,
		PW_REGEN,
/* precache */ "",
/* sounds */ "sound/items/regen.wav"
	},

/*QUAKED item_flight (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 33
		"item_flight",
		"sound/items/flight.wav",
        { "models/powerups/instant/flight.md3",
		"models/powerups/instant/flight_ring.md3",
		0, 0 },
/* icon */		"icons/flight",
/* pickup */	"Flight",
		60,
		IT_POWERUP,
		PW_FLIGHT,
/* precache */ "",
/* sounds */ "sound/items/flight.wav"
	},

/*QUAKED team_CTF_redflag (1 0 0) (-16 -16 -16) (16 16 16)
Only in CTF games
*/
	{// 34
		"team_CTF_redflag",
		NULL,
        { "models/flags/r_flag.md3",
		0, 0, 0 },
/* icon */		"icons/iconf_red1",
/* pickup */	"Red Flag",
		0,
		IT_TEAM,
		PW_REDFLAG,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED team_CTF_blueflag (0 0 1) (-16 -16 -16) (16 16 16)
Only in CTF games
*/
	{// 35
		"team_CTF_blueflag",
		NULL,
        { "models/flags/b_flag.md3",
		0, 0, 0 },
/* icon */		"icons/iconf_blu1",
/* pickup */	"Blue Flag",
		0,
		IT_TEAM,
		PW_BLUEFLAG,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED team_CTF_AlienEgg (0 0 1) (-16 -16 -16) (16 16 16)
Only in CTF games
*/
	{// 36
		"team_CTF_alienegg",
		"sound/teamplay/flagtk_blu.wav",
        { "models/flags/b_egg.md3",
		0, 0, 0 },
/* icon */		"icons/iconf_egg1",
/* pickup */	"Alien Egg",
		0,
		IT_TEAM,
		PW_BLUEFLAG,
/* precache */ "",
/* sounds */ "sound/teamplay/flagcap_blu.wav sound/teamplay/flagtk_blu.wav sound/teamplay/flagret_blu.wav"
	},

	{//37
		"team_CTF_teleporter",
		"sound/teamplay/flagtk_red.wav",
        { "models/flags/r_teleporter.md3",
		0, 0, 0 },
/* icon */		"icons/iconf_marines",
/* pickup */	"Teleporter",
		0,
		IT_TEAM,
		PW_REDFLAG,
/* precache */ "",
/* sounds */ "sound/teamplay/flagcap_red.wav sound/teamplay/flagtk_red.wav sound/teamplay/flagret_red.wav"
	},

/*QUAKED weapon_invchaingun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 38
		"weapon_invchaingun",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/chaingun/chaingun.md3",
		0, 0, 0},
/* icon */		"icons/iconw_chaingun",
/* pickup */	"Chaingun",
		80,
		IT_WEAPON,
		WP_INVCHAINGUN,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED weapon_flamethrower (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 39
		"weapon_flamethrower",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/flamethrower/flamethrower.md3",
		0, 0, 0},
/* icon */		"icons/iconw_flamethrower",
/* pickup */	"FlameThrower",
		50,
		IT_WEAPON,
		WP_FLAMETHROWER,
/* precache */ "",
/* sounds */ ""
	},

/*#ifdef MISSIONPACK
//QUAKED holdable_kamikaze (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	{// 40
		"holdable_kamikaze",
		"sound/items/holdable.wav",
        { "models/powerups/kamikazi.md3",
		0, 0, 0},
		"icons/kamikaze",
		"Kamikaze",
		60,
		IT_HOLDABLE,
		HI_KAMIKAZE,
		 "",
		 "sound/items/kamikazerespawn.wav"
	},

//QUAKED holdable_portal (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	{// 41
		"holdable_portal",
		"sound/items/holdable.wav",
        { "models/powerups/holdable/porter.md3",
		0, 0, 0},
		"icons/portal",
	"Portal",
		60,
		IT_HOLDABLE,
		HI_PORTAL,
 "",
 ""
	},

//QUAKED holdable_invulnerability (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	{// 42
		"holdable_invulnerability",
		"sound/items/holdable.wav",
        { "models/powerups/holdable/invulnerability.md3",
		0, 0, 0},
		"icons/invulnerability",
	"Invulnerability",
		60,
		IT_HOLDABLE,
		HI_INVULNERABILITY,
 "",
 ""
	},

//QUAKED ammo_nails (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	{// 43
		"ammo_nails",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/nailgunam.md3",
		0, 0, 0},
		"icons/icona_nailgun",
	"Nails",
		20,
		IT_AMMO,
		WP_NAILGUN,
 "",
 ""
	},

//QUAKED ammo_mines (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	{// 44
		"ammo_mines",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/proxmineam.md3",
		0, 0, 0},
		"icons/icona_proxlauncher",
	"Proximity Mines",
		10,
		IT_AMMO,
		WP_PROX_LAUNCHER,
 "",
 ""
	},

//QUAKED ammo_belt (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	{// 45
		"ammo_belt",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/chaingunam.md3",
		0, 0, 0},
		"icons/icona_chaingun",
	"Chaingun Belt",
		100,
		IT_AMMO,
		WP_CHAINGUN,
 "",
 ""
	},

	//
	// PERSISTANT POWERUP ITEMS
	//
//QUAKED item_scout (.3 .3 1) (-16 -16 -16) (16 16 16) suspended redTeam blueTeam
	{// 46
		"item_scout",
		"sound/items/scout.wav",
        { "models/powerups/scout.md3",
		0, 0, 0 },
		"icons/scout",
	"Scout",
		30,
		IT_PERSISTANT_POWERUP,
		PW_SCOUT,
 "",
 ""
	},

//QUAKED item_guard (.3 .3 1) (-16 -16 -16) (16 16 16) suspended redTeam blueTeam
	{// 47
		"item_guard",
		"sound/items/guard.wav",
        { "models/powerups/guard.md3",
		0, 0, 0 },
		"icons/guard",
	"Guard",
		30,
		IT_PERSISTANT_POWERUP,
		PW_GUARD,
 "",
 ""
	},

//QUAKED item_doubler (.3 .3 1) (-16 -16 -16) (16 16 16) suspended redTeam blueTeam
	{// 48
		"item_doubler",
		"sound/items/doubler.wav",
        { "models/powerups/doubler.md3",
		0, 0, 0 },
		"icons/doubler",
	"Doubler",
		30,
		IT_PERSISTANT_POWERUP,
		PW_DOUBLER,
 "",
 ""
	},

//QUAKED item_doubler (.3 .3 1) (-16 -16 -16) (16 16 16) suspended redTeam blueTeam
	{// 49
		"item_ammoregen",
		"sound/items/ammoregen.wav",
        { "models/powerups/ammo.md3",
		0, 0, 0 },
		"icons/ammo_regen",
	"Ammo Regen",
		30,
		IT_PERSISTANT_POWERUP,
		PW_AMMOREGEN,
 "",
 ""
	},

//QUAKED team_CTF_neutralflag (0 0 1) (-16 -16 -16) (16 16 16) Only in One Flag CTF games
	{// 50
		"team_CTF_neutralflag",
		NULL,
        { "models/flags/n_flag.md3",
		0, 0, 0 },
		"icons/iconf_neutral1",
	"Neutral Flag",
		0,
		IT_TEAM,
		PW_NEUTRALFLAG,
 "",
 ""
	},

	{// 51
		"item_redcube",
		"sound/misc/am_pkup.wav",
        { "models/powerups/orb/r_orb.md3",
		0, 0, 0 },
		"icons/iconh_rorb",
	"Red Cube",
		0,
		IT_TEAM,
		0,
 "",
 ""
	},

	{// 52
		"item_bluecube",
		"sound/misc/am_pkup.wav",
        { "models/powerups/orb/b_orb.md3",
		0, 0, 0 },
		"icons/iconh_borb",
	"Blue Cube",
		0,
		IT_TEAM,
		0,
 "",
 ""
	},
//QUAKED weapon_nailgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	{// 53
		"weapon_nailgun",
		"sound/misc/w_pkup.wav",
        { "models/weapons/nailgun/nailgun.md3",
		0, 0, 0},
		"icons/iconw_nailgun",
	"Nailgun",
		10,
		IT_WEAPON,
		WP_NAILGUN,
 "",
 ""
	},

//QUAKED weapon_prox_launcher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	{// 54
		"weapon_prox_launcher",
		"sound/misc/w_pkup.wav",
        { "models/weapons/proxmine/proxmine.md3",
		0, 0, 0},
		"icons/iconw_proxlauncher",
	"Prox Launcher",
		5,
		IT_WEAPON,
		WP_PROX_LAUNCHER,
 "",
 "sound/weapons/proxmine/wstbtick.wav "
			"sound/weapons/proxmine/wstbactv.wav "
			"sound/weapons/proxmine/wstbimpl.wav "
			"sound/weapons/proxmine/wstbimpm.wav "
			"sound/weapons/proxmine/wstbimpd.wav "
			"sound/weapons/proxmine/wstbactv.wav"
	},

//QUAKED weapon_chaingun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
	{// 55
		"weapon_chaingun",
		"sound/misc/w_pkup.wav",
        { "models/weapons/vulcan/vulcan.md3",
		0, 0, 0},
		"icons/iconw_chaingun",
	"Chaingun",
		80,
		IT_WEAPON,
		WP_CHAINGUN,
 "",
 "sound/weapons/vulcan/wvulwind.wav"
	},
#endif */

/*QUAKED ammo_flame (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{// 56
		"ammo_flame",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/flameammo.md3",
		0, 0, 0},
/* icon */		"icons/iconw_flamethrower",
/* pickup */	"Flame",
		50,
		IT_AMMO,
		WP_FLAMETHROWER,
/* precache */ "",
/* sounds */ ""
	},

	{// xx
		"weapon_machinegun",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/machinegun/machinegun.md3",
		0, 0, 0},
/* icon */		"icons/iconw_machinegun",
/* pickup */	"Machinegun",
		40,
		IT_WEAPON,
		WP_ALIEN_GUN,
/* precache */ "",
/* sounds */ ""
	},

	{// xx
		"grenade_spike",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/grenades/sg_full.md3",
			"models/weapons2/grenades/sg_hull.md3", 0, 0},
/* icon */		"icons/icona_grenade",
/* pickup */	"Spike Grenade",
		3,
		IT_AMMO,
		WP_GRENADE_LAUNCHER,
/* precache */ "",
/* sounds */ ""
	},

	// end of list marker
	{NULL}
};

int		bg_numItems = sizeof(bg_itemlist) / sizeof(bg_itemlist[0]) - 1;


/*
==============
BG_FindItemForPowerup
==============
*/
gitem_t	*BG_FindItemForPowerup(powerup_t pw)
{
	int		i;

	for (i = 0; i < bg_numItems; i++)
	{
		if ((bg_itemlist[i].giType == IT_POWERUP
			|| bg_itemlist[i].giType == IT_TEAM
			|| bg_itemlist[i].giType == IT_PERSISTANT_POWERUP)
			&& bg_itemlist[i].giTag == pw)
		{
			return &bg_itemlist[i];
		}
	}

	return NULL;
}


/*
==============
BG_FindItemForHoldable
==============
*/
gitem_t	*BG_FindItemForHoldable(holdable_t pw)
{
	int		i;

	for (i = 0; i < bg_numItems; i++)
	{
		if (bg_itemlist[i].giType == IT_HOLDABLE && bg_itemlist[i].giTag == pw)
		{
			return &bg_itemlist[i];
		}
	}

	Com_Error(ERR_DROP, "HoldableItem not found");

	return NULL;
}


/*
===============
BG_FindItemForWeapon

===============
*/
gitem_t	*BG_FindItemForWeapon(weapon_t weapon)
{
	gitem_t	*it;

	for (it = bg_itemlist + 1; it->classname; it++)
	{
		if (it->giType == IT_WEAPON && it->giTag == weapon)
		{
			return it;
		}
	}

	Com_Error(ERR_DROP, "Couldn't find item for weapon %i", weapon);
	return NULL;
}

/*
===============
BG_FindItem

===============
*/
gitem_t	*BG_FindItem(const char *pickupName)
{
	gitem_t	*it;

	for (it = bg_itemlist + 1; it->classname; it++)
	{
		if (!Q_stricmp(it->pickup_name, pickupName))
			return it;
	}

	return NULL;
}

/*
============
BG_PlayerTouchesItem

Items can be picked up without actually touching their physical bounds to make
grabbing them easier
============
*/
qboolean	BG_PlayerTouchesItem(playerState_t *ps, entityState_t *item, int atTime)
{
	vec3_t		origin;

	BG_EvaluateTrajectory(&item->pos, atTime, origin);

	// we are ignoring ducked differences here
	if (ps->origin[0] - origin[0] > 44
		|| ps->origin[0] - origin[0] < -50
		|| ps->origin[1] - origin[1] > 36
		|| ps->origin[1] - origin[1] < -36
		|| ps->origin[2] - origin[2] > 36
		|| ps->origin[2] - origin[2] < -36)
		{
		return qfalse;
	}

	return qtrue;
}



/*
================
BG_CanItemBeGrabbed

Returns false if the item should not be picked up.
This needs to be the same for client side prediction and server use.
================
*/
qboolean BG_CanItemBeGrabbed(int gametype, const entityState_t *ent, const playerState_t *ps, const InvasionInfo_t *InvInfo)
{
	gitem_t	*item;
/*#ifdef MISSIONPACK
	int		upperBound;
#endif*/
	int AlienRace = ps->persistant[PERS_CLASS] & e_Class_AlienRaceMask;

	if (ent->modelindex < 1 || ent->modelindex >= bg_numItems)
	{
		Com_Error(ERR_DROP, "BG_CanItemBeGrabbed: index out of range");
	}

	item = &bg_itemlist[ent->modelindex];

/*	if (ps->persistant[PERS_TEAM] == InvInfo->AlienTeam)
	{
		int AlienRace = ps->persistant[PERS_CLASS] & e_Class_AlienRaceMask;

		if (AlienRace == e_Selection_Trooper && item->giType == IT_WEAPON
			&& !(ps->stats[STAT_WEAPONS] & ~((1 << WP_MACHINEGUN) | (1 << WP_GAUNTLET))))
		{
			return qtrue;
		}

		return qfalse;				//Too: aliens can't take anything (neither flag or dropped weapon)
	} */

	switch (item->giType)
	{
	case IT_WEAPON:
		if (ps->persistant[PERS_TEAM] == InvInfo->AlienTeam)
		{
			if (AlienRace == e_Selection_Trooper
				&& !(ps->stats[STAT_WEAPONS] & ~((1 << WP_MACHINEGUN) | (1 << WP_GAUNTLET))))
			{
				return qtrue;
			}

			return qfalse;
		}

		if (ps->stats[STAT_WEAPONS] & ~((1 << WP_MACHINEGUN) | (1 << WP_GAUNTLET)))
			return qfalse;			// can't take more than 1 weapon
		return qtrue;	// weapons are always picked up

	case IT_AMMO:
		if (ps->ammo[ item->giTag ] >= 200)
		{
			return qfalse;		// can't hold any more
		}
		return qtrue;

	case IT_ARMOR:
/*#ifdef MISSIONPACK
		if (bg_itemlist[ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_SCOUT)
		{
			return qfalse;
		}

		// we also clamp armor to the maxhealth for handicapping
		if (bg_itemlist[ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_GUARD)
		{
			upperBound = ps->stats[STAT_MAX_HEALTH];
		}
		else
		{
			upperBound = ps->stats[STAT_MAX_HEALTH] * 2;
		}

		if (ps->stats[STAT_ARMOR] >= upperBound)
		{
			return qfalse;
		}
#else*/
		if (ps->stats[STAT_ARMOR] >= ps->stats[STAT_MAX_HEALTH] * 2)
		{
			return qfalse;
		}
//#endif
		return qtrue;

	case IT_HEALTH:
		// small and mega healths will go over the max, otherwise
		// don't pick up if already at max
/*#ifdef MISSIONPACK
		if (bg_itemlist[ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_GUARD)
		{
			upperBound = ps->stats[STAT_MAX_HEALTH];
		}
		else
#endif*/
		if (item->quantity == 5 || item->quantity == 100)
		{
			if (ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH] * 2)
			{
				return qfalse;
			}
			return qtrue;
		}

		if (ps->stats[STAT_HEALTH] >= ps->stats[STAT_MAX_HEALTH])
		{
			return qfalse;
		}
		return qtrue;

	case IT_POWERUP:
		return qtrue;	// powerups are always picked up

/*#ifdef MISSIONPACK
	case IT_PERSISTANT_POWERUP:
		// can only hold one item at a time
		if (ps->stats[STAT_PERSISTANT_POWERUP])
		{
			return qfalse;
		}

		// check team only
		if ((ent->generic1 & 2) && (ps->persistant[PERS_TEAM] != TEAM_RED))
		{
			return qfalse;
		}
		if ((ent->generic1 & 4) && (ps->persistant[PERS_TEAM] != TEAM_BLUE))
		{
			return qfalse;
		}

		return qtrue;
#endif*/

	case IT_TEAM: // team items, such as flags
/*#ifdef MISSIONPACK
		if (gametype == GT_1FCTF)
		{
			// neutral flag can always be picked up
			if (item->giTag == PW_NEUTRALFLAG)
			{
				return qtrue;
			}
			if (ps->persistant[PERS_TEAM] == TEAM_RED)
			{
				if (item->giTag == PW_BLUEFLAG  && ps->powerups[PW_NEUTRALFLAG])
				{
					return qtrue;
				}
			}
			else if (ps->persistant[PERS_TEAM] == TEAM_BLUE)
			{
				if (item->giTag == PW_REDFLAG  && ps->powerups[PW_NEUTRALFLAG])
				{
					return qtrue;
				}
			}
		}
#endif*/
		if (gametype == GT_CTF || gametype == GT_INVASION)
		{
			// ent->modelindex2 is non-zero on items if they are dropped
			// we need to know this because we can pick up our dropped flag (and return it)
			// but we can't pick up our flag at base
			if (ps->persistant[PERS_TEAM] == TEAM_RED)
			{
				if ((InvInfo->gametype == GT_INVASION && InvInfo->MarineTeam != TEAM_RED)
					|| InvInfo->Period == e_Period_WaitRestart)		//ToInv: maybe defensers could pickup their flag to run away with it ?
					return qfalse;

				if (item->giTag == PW_BLUEFLAG ||
					(item->giTag == PW_REDFLAG && ent->modelindex2) ||
					(item->giTag == PW_REDFLAG && ps->powerups[PW_BLUEFLAG]))
					return qtrue;
			}
			else if (ps->persistant[PERS_TEAM] == TEAM_BLUE)
			{
				if ((InvInfo->gametype == GT_INVASION && InvInfo->MarineTeam != TEAM_BLUE)
					|| InvInfo->Period == e_Period_WaitRestart)
					return qfalse;

				if (item->giTag == PW_REDFLAG ||
					(item->giTag == PW_BLUEFLAG && ent->modelindex2) ||
					(item->giTag == PW_BLUEFLAG && ps->powerups[PW_REDFLAG]))
					return qtrue;
			}
		}

/*#ifdef MISSIONPACK
		if (gametype == GT_HARVESTER)
		{
			return qtrue;
		}
#endif*/
		return qfalse;

	case IT_HOLDABLE:
		// can only hold one item at a time
		if (ps->stats[STAT_HOLDABLE_ITEM])
		{
			return qfalse;
		}
		return qtrue;

	case IT_BAD:
		Com_Error(ERR_DROP, "BG_CanItemBeGrabbed: IT_BAD");

	default:
#ifndef Q3_VM
#ifndef NDEBUG // bk0001204
		Com_Printf("BG_CanItemBeGrabbed: unknown enum %d\n", item->giType);
#endif
#endif
		break;
	}

	return qfalse;
}

//======================================================================

/*
================
BG_EvaluateTrajectory

================
*/
void BG_EvaluateTrajectory(const trajectory_t *tr, int atTime, vec3_t result)
{
	float		deltaTime;
	float		phase;

	switch (tr->trType)
	{
	case TR_STATIONARY:
	case TR_INTERPOLATE:
		VectorCopy(tr->trBase, result);
		break;
	case TR_LINEAR:
		deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
		VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
		break;
	case TR_SINE:
		deltaTime = (atTime - tr->trTime) / (float) tr->trDuration;
		phase = sin(deltaTime * M_PI * 2);
		VectorMA(tr->trBase, phase, tr->trDelta, result);
		break;
	case TR_LINEAR_STOP:
		if (atTime > tr->trTime + tr->trDuration)
		{
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
		if (deltaTime < 0)
		{
			deltaTime = 0;
		}
		VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
		break;
	case TR_GRAVITY:
		deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
		VectorMA(tr->trBase, deltaTime, tr->trDelta, result);
		result[2] -= 0.5 * DEFAULT_GRAVITY * deltaTime * deltaTime;		// FIXME: local gravity...
		break;
	default:
		Com_Error(ERR_DROP, "BG_EvaluateTrajectory: unknown trType: %i", tr->trTime);
		break;
	}
}

/*
================
BG_EvaluateTrajectoryDelta

For determining velocity at a given time
================
*/
void BG_EvaluateTrajectoryDelta(const trajectory_t *tr, int atTime, vec3_t result)
{
	float	deltaTime;
	float	phase;

	switch (tr->trType)
	{
	case TR_STATIONARY:
	case TR_INTERPOLATE:
		VectorClear(result);
		break;
	case TR_LINEAR:
		VectorCopy(tr->trDelta, result);
		break;
	case TR_SINE:
		deltaTime = (atTime - tr->trTime) / (float) tr->trDuration;
		phase = cos(deltaTime * M_PI * 2);	// derivative of sin = cos
		phase *= 0.5;
		VectorScale(tr->trDelta, phase, result);
		break;
	case TR_LINEAR_STOP:
		if (atTime > tr->trTime + tr->trDuration)
		{
			VectorClear(result);
			return;
		}
		VectorCopy(tr->trDelta, result);
		break;
	case TR_GRAVITY:
		deltaTime = (atTime - tr->trTime) * 0.001;	// milliseconds to seconds
		VectorCopy(tr->trDelta, result);
		result[2] -= DEFAULT_GRAVITY * deltaTime;		// FIXME: local gravity...
		break;
	default:
		Com_Error(ERR_DROP, "BG_EvaluateTrajectoryDelta: unknown trType: %i", tr->trTime);
		break;
	}
}

char *eventnames[] =
{
	"EV_NONE",

	"EV_FOOTSTEP",
	"EV_FOOTSTEP_METAL",
	"EV_FOOTSPLASH",
	"EV_FOOTWADE",
	"EV_SWIM",

	"EV_STEP_4",
	"EV_STEP_8",
	"EV_STEP_12",
	"EV_STEP_16",

	"EV_FALL_SHORT",
	"EV_FALL_MEDIUM",
	"EV_FALL_FAR",

	"EV_JUMP_PAD",			// boing sound at origin", jump sound on player

	"EV_JUMP",
	"EV_WATER_TOUCH",	// foot touches
	"EV_WATER_LEAVE",	// foot leaves
	"EV_WATER_UNDER",	// head touches
	"EV_WATER_CLEAR",	// head leaves

	"EV_ITEM_PICKUP",			// normal item pickups are predictable
	"EV_GLOBAL_ITEM_PICKUP",	// powerup / team sounds are broadcast to everyone

	"EV_NOAMMO",
	"EV_CHANGE_WEAPON",
	"EV_FIRE_WEAPON",

	"EV_USE_ITEM0",
	"EV_USE_ITEM1",
	"EV_USE_ITEM2",
	"EV_USE_ITEM3",
	"EV_USE_ITEM4",
	"EV_USE_ITEM5",
	"EV_USE_ITEM6",
	"EV_USE_ITEM7",
	"EV_USE_ITEM8",
	"EV_USE_ITEM9",
	"EV_USE_ITEM10",
	"EV_USE_ITEM11",
	"EV_USE_ITEM12",
	"EV_USE_ITEM13",
	"EV_USE_ITEM14",
	"EV_USE_ITEM15",

	"EV_ITEM_RESPAWN",
	"EV_ITEM_POP",
	"EV_PLAYER_TELEPORT_IN",
	"EV_PLAYER_TELEPORT_OUT",

	"EV_GRENADE_BOUNCE",		// eventParm will be the soundindex

	"EV_GENERAL_SOUND",
	"EV_GLOBAL_SOUND",		// no attenuation
	"EV_GLOBAL_TEAM_SOUND",

	"EV_BULLET_HIT_FLESH",
	"EV_BULLET_HIT_WALL",

	"EV_MISSILE_HIT",
	"EV_MISSILE_MISS",
	"EV_MISSILE_MISS_METAL",
	"EV_RAILTRAIL",
	"EV_SHOTGUN",
	"EV_BULLET",				// otherEntity is the shooter

	"EV_PAIN",
	"EV_DEATH1",
	"EV_DEATH2",
	"EV_DEATH3",
	"EV_OBITUARY",

	"EV_POWERUP_QUAD",
	"EV_POWERUP_BATTLESUIT",
	"EV_POWERUP_REGEN",

	"EV_GIB_PLAYER",			// gib a previously living player
	"EV_SCOREPLUM",			// score plum

//#ifdef MISSIONPACK
	"EV_PROXIMITY_MINE_STICK",
	"EV_PROXIMITY_MINE_TRIGGER",
	"EV_KAMIKAZE",			// kamikaze explodes
	"EV_OBELISKEXPLODE",		// obelisk explodes
	"EV_INVUL_IMPACT",		// invulnerability sphere impact
	"EV_JUICED",				// invulnerability juiced effect
	"EV_LIGHTNINGBOLT",		// lightning bolt bounced of invulnerability sphere
//#endif

	"EV_DEBUG_LINE",
	"EV_STOPLOOPINGSOUND",
	"EV_TAUNT"

};

/*
===============
BG_AddPredictableEventToPlayerstate

Handles the sequence numbers
===============
*/

void	trap_Cvar_VariableStringBuffer(const char *var_name, char *buffer, int bufsize);

void BG_AddPredictableEventToPlayerstate(int newEvent, int eventParm, playerState_t *ps)
{

#ifdef _DEBUG
	{
		char buf[256];
		trap_Cvar_VariableStringBuffer("showevents", buf, sizeof(buf));
		if (atof(buf) != 0)
		{
#ifdef QAGAME
			Com_Printf(" game event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount/*ps->commandTime*/, ps->eventSequence, eventnames[newEvent], eventParm);
#else
			Com_Printf("Cgame event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount/*ps->commandTime*/, ps->eventSequence, eventnames[newEvent], eventParm);
#endif
		}
	}
#endif
	ps->events[ps->eventSequence & (MAX_PS_EVENTS-1)] = newEvent;
	ps->eventParms[ps->eventSequence & (MAX_PS_EVENTS-1)] = eventParm;
	ps->eventSequence++;
}

/*
========================
BG_TouchJumpPad
========================
*/
void BG_TouchJumpPad(playerState_t *ps, entityState_t *jumppad)
{
	vec3_t	angles;
	float p;
	int effectNum;

	// spectators don't use jump pads
	if (ps->pm_type != PM_NORMAL)
	{
		return;
	}

	// flying characters don't hit bounce pads
	if (ps->powerups[PW_FLIGHT])
	{
		return;
	}

	// if we didn't hit this same jumppad the previous frame
	// then don't play the event sound again if we are in a fat trigger
	if (ps->jumppad_ent != jumppad->number)
	{

		vectoangles(jumppad->origin2, angles);
		p = fabs(AngleNormalize180(angles[PITCH]));
		if (p < 45)
		{
			effectNum = 0;
		}
		else
		{
			effectNum = 1;
		}
		BG_AddPredictableEventToPlayerstate(EV_JUMP_PAD, effectNum, ps);
	}
	// remember hitting this jumppad this frame
	ps->jumppad_ent = jumppad->number;
	ps->jumppad_frame = ps->pmove_framecount;
	// give the player the velocity from the jumppad
	VectorCopy(jumppad->origin2, ps->velocity);
}

/*
========================
BG_PlayerStateToEntityState

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityState(playerState_t *ps, entityState_t *s, qboolean snap)
{
	int		i;

	if (ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR)
	{
		s->eType = ET_INVISIBLE;
	}
	else if (ps->stats[STAT_HEALTH] <= GIB_HEALTH)
	{
		s->eType = ET_INVISIBLE;
	}
	else
	{
		s->eType = ET_PLAYER;
	}

	s->number = ps->clientNum;

	s->pos.trType = TR_INTERPOLATE;
	VectorCopy(ps->origin, s->pos.trBase);
	if (snap)
	{
		SnapVector(s->pos.trBase);
	}
	// set the trDelta for flag direction
	VectorCopy(ps->velocity, s->pos.trDelta);

	s->apos.trType = TR_INTERPOLATE;
	VectorCopy(ps->viewangles, s->apos.trBase);
	if (snap)
	{
		SnapVector(s->apos.trBase);
	}

	s->angles2[YAW] = ps->movementDir;
	s->angles2[PITCH] = ps->stats[STAT_SPEC1] & 65535;	//Too:
	s->angles2[ROLL] = ps->stats[STAT_SPEC2] & 65535;	//Too:
	s->origin2[0] = (int) ps->speed * 10;					//Too:

	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;
	s->clientNum = ps->clientNum;		// ET_PLAYER looks here instead of at number
										// so corpses can also reference the proper config
	s->eFlags = ps->eFlags;
	if (ps->stats[STAT_HEALTH] <= 0)
	{
		s->eFlags |= EF_DEAD;
	}
	else
	{
		s->eFlags &= ~EF_DEAD;
	}

	if (ps->externalEvent)
	{
		s->event = ps->externalEvent;
		s->eventParm = ps->externalEventParm;
	}
	else if (ps->entityEventSequence < ps->eventSequence)
	{
		int		seq;

		if (ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS)
		{
			ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;
		}
		seq = ps->entityEventSequence & (MAX_PS_EVENTS-1);
		s->event = ps->events[ seq ] | ((ps->entityEventSequence & 3) << 8);
		s->eventParm = ps->eventParms[seq];
		ps->entityEventSequence++;
	}

	s->weapon = ps->weapon;
	s->groundEntityNum = ps->groundEntityNum;

	s->powerups = 0;
	for (i = 1; i < MAX_POWERUPS; i++)
	{
		if (ps->powerups[i])
		{
			s->powerups |= 1 << i;
		}
	}

	s->loopSound = ps->loopSound;
	s->generic1 = ps->generic1;
}

/*
========================
BG_PlayerStateToEntityStateExtraPolate

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityStateExtraPolate(playerState_t *ps, entityState_t *s, int time, qboolean snap)
{
	int		i;

	if (ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR)
	{
		s->eType = ET_INVISIBLE;
	}
	else if (ps->stats[STAT_HEALTH] <= GIB_HEALTH)
	{
		s->eType = ET_INVISIBLE;
	}
	else
	{
		s->eType = ET_PLAYER;
	}

	s->number = ps->clientNum;

	s->pos.trType = TR_LINEAR_STOP;
	VectorCopy(ps->origin, s->pos.trBase);
	if (snap)
	{
		SnapVector(s->pos.trBase);
	}
	// set the trDelta for flag direction and linear prediction
	VectorCopy(ps->velocity, s->pos.trDelta);
	// set the time for linear prediction
	s->pos.trTime = time;
	// set maximum extra polation time
	s->pos.trDuration = 50; // 1000 / sv_fps (default = 20)

	s->apos.trType = TR_INTERPOLATE;
	VectorCopy(ps->viewangles, s->apos.trBase);
	if (snap)
	{
		SnapVector(s->apos.trBase);
	}

	s->angles2[YAW] = ps->movementDir;
	s->angles2[PITCH] = ps->stats[STAT_SPEC1] & 65535;	//Too:
	s->angles2[ROLL] = ps->stats[STAT_SPEC2] & 65535;	//Too:
	s->origin2[0] = (int) ps->speed * 10;					//Too:

	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;
	s->clientNum = ps->clientNum;		// ET_PLAYER looks here instead of at number
										// so corpses can also reference the proper config
	s->eFlags = ps->eFlags;
	if (ps->stats[STAT_HEALTH] <= 0)
	{
		s->eFlags |= EF_DEAD;
	}
	else
	{
		s->eFlags &= ~EF_DEAD;
	}

	if (ps->externalEvent)
	{
		s->event = ps->externalEvent;
		s->eventParm = ps->externalEventParm;
	}
	else if (ps->entityEventSequence < ps->eventSequence)
	{
		int		seq;

		if (ps->entityEventSequence < ps->eventSequence - MAX_PS_EVENTS)
		{
			ps->entityEventSequence = ps->eventSequence - MAX_PS_EVENTS;
		}
		seq = ps->entityEventSequence & (MAX_PS_EVENTS-1);
		s->event = ps->events[seq] | ((ps->entityEventSequence & 3) << 8);
		s->eventParm = ps->eventParms[seq];
		ps->entityEventSequence++;
	}

	s->weapon = ps->weapon;
	s->groundEntityNum = ps->groundEntityNum;

	s->powerups = 0;
	for (i = 0; i < MAX_POWERUPS; i++)
	{
		if (ps->powerups[ i ])
		{
			s->powerups |= 1 << i;
		}
	}

	s->loopSound = ps->loopSound;
	s->generic1 = ps->generic1;
}

/*==================== EOF because of buggy VSS ===========*/