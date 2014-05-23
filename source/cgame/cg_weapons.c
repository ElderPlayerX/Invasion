// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_weapons.c -- events and effects dealing with weapons
#include "cg_local.h"

/*
==========================
CG_MachineGunEjectBrass
==========================
*/
static void CG_MachineGunEjectBrass(centity_t *cent)
{
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			velocity, xvelocity;
	vec3_t			offset, xoffset;
	float			waterScale = 1.0f;
	vec3_t			v[3];

	if (cg_brassTime.integer <= 0)
	{
		return;
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = 0;
	velocity[1] = -50 + 40 * crandom();
	velocity[2] = 100 + 50 * crandom();

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + cg_brassTime.integer + (cg_brassTime.integer / 4) * random();

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time - (rand()&15);

	AnglesToAxis(cent->lerpAngles, v);

	offset[0] = 8;
	offset[1] = -4;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd(cent->lerpOrigin, xoffset, re->origin);

	VectorCopy(re->origin, le->pos.trBase);

	if (CG_PointContents(re->origin, -1) & CONTENTS_WATER)
	{
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	VectorScale(xvelocity, waterScale, le->pos.trDelta);

	AxisCopy(axisDefault, re->axis);
	re->hModel = cgs.media.machinegunBrassModel;

	le->bounceFactor = 0.4 * waterScale;

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()&31;
	le->angles.trBase[1] = rand()&31;
	le->angles.trBase[2] = rand()&31;
	le->angles.trDelta[0] = 2;
	le->angles.trDelta[1] = 1;
	le->angles.trDelta[2] = 0;

	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}

/*
==========================
CG_ShotgunEjectBrass
==========================
*/
static void CG_ShotgunEjectBrass(centity_t *cent)
{
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			velocity, xvelocity;
	vec3_t			offset, xoffset;
	vec3_t			v[3];
	int				i;

	if (cg_brassTime.integer <= 0)
	{
		return;
	}

	for (i = 0; i < 2; i++)
	{
		float	waterScale = 1.0f;

		le = CG_AllocLocalEntity();
		re = &le->refEntity;

		velocity[0] = 60 + 60 * crandom();
		if (i == 0)
		{
			velocity[1] = 40 + 10 * crandom();
		}
		else
		{
			velocity[1] = -40 + 10 * crandom();
		}
		velocity[2] = 100 + 50 * crandom();

		le->leType = LE_FRAGMENT;
		le->startTime = cg.time;
		le->endTime = le->startTime + cg_brassTime.integer*3 + cg_brassTime.integer * random();

		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;

		AnglesToAxis(cent->lerpAngles, v);

		offset[0] = 8;
		offset[1] = 0;
		offset[2] = 24;

		xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
		xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
		xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
		VectorAdd(cent->lerpOrigin, xoffset, re->origin);
		VectorCopy(re->origin, le->pos.trBase);
		if (CG_PointContents(re->origin, -1) & CONTENTS_WATER)
		{
			waterScale = 0.10f;
		}

		xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
		xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
		xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
		VectorScale(xvelocity, waterScale, le->pos.trDelta);

		AxisCopy(axisDefault, re->axis);
		re->hModel = cgs.media.shotgunBrassModel;
		le->bounceFactor = 0.3f;

		le->angles.trType = TR_LINEAR;
		le->angles.trTime = cg.time;
		le->angles.trBase[0] = rand()&31;
		le->angles.trBase[1] = rand()&31;
		le->angles.trBase[2] = rand()&31;
		le->angles.trDelta[0] = 1;
		le->angles.trDelta[1] = 0.5;
		le->angles.trDelta[2] = 0;

		le->leFlags = LEF_TUMBLE;
		le->leBounceSoundType = LEBS_BRASS;
		le->leMarkType = LEMT_NONE;
	}
}


/*#ifdef MISSIONPACK
//==========================
//CG_NailgunEjectBrass
//==========================
static void CG_NailgunEjectBrass(centity_t *cent)
{
	localEntity_t	*smoke;
	vec3_t			origin;
	vec3_t			v[3];
	vec3_t			offset;
	vec3_t			xoffset;
	vec3_t			up;

	AnglesToAxis(cent->lerpAngles, v);

	offset[0] = 0;
	offset[1] = -12;
	offset[2] = 24;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
	VectorAdd(cent->lerpOrigin, xoffset, origin);

	VectorSet(up, 0, 0, 64);

	smoke = CG_SmokePuff(origin, up, 32, 1, 1, 1, 0.33f, 700, cg.time, 0, 0, cgs.media.smokePuffShader);
	// use the optimized local entity add
	smoke->leType = LE_SCALE_FADE;
}
#endif*/


/*
==========================
CG_RailTrail
==========================
*/
void CG_RailTrail(clientInfo_t *ci, vec3_t start, vec3_t end)
{
	vec3_t axis[36], move, move2, next_move, vec, temp;
	float  len;
	int    i, j, skip;

	localEntity_t *le;
	refEntity_t   *re;

#define RADIUS   4
#define ROTATION 1
#define SPACING  5

	start[2] -= 4;
	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);
	PerpendicularVector(temp, vec);
	for (i = 0 ; i < 36; i++) {
		RotatePointAroundVector(axis[i], vec, temp, i * 10);//banshee 2.4 was 10
	}

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + cg_railTrailTime.value;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_RAIL_CORE;
	re->customShader = cgs.media.railCoreShader;

	VectorCopy(start, re->origin);
	VectorCopy(end, re->oldorigin);

	re->shaderRGBA[0] = ci->color1[0] * 255;
    re->shaderRGBA[1] = ci->color1[1] * 255;
    re->shaderRGBA[2] = ci->color1[2] * 255;
    re->shaderRGBA[3] = 255;

	le->color[0] = ci->color1[0] * 0.75;
	le->color[1] = ci->color1[1] * 0.75;
	le->color[2] = ci->color1[2] * 0.75;
	le->color[3] = 1.0f;

	AxisClear( re->axis );

	VectorMA(move, 20, vec, move);
	VectorCopy(move, next_move);
	VectorScale (vec, SPACING, vec);

	if (cg_oldRail.integer != 0) {
		// nudge down a bit so it isn't exactly in center
		re->origin[2] -= 8;
		re->oldorigin[2] -= 8;
		return;
	}
	skip = -1;

	j = 18;
    for (i = 0; i < len; i += SPACING) {
		if (i != skip) {
			skip = i + SPACING;
			le = CG_AllocLocalEntity();
            re = &le->refEntity;
            le->leFlags = LEF_PUFF_DONT_SCALE;
			le->leType = LE_MOVE_SCALE_FADE;
            le->startTime = cg.time;
            le->endTime = cg.time + (i>>1) + 600;
            le->lifeRate = 1.0 / (le->endTime - le->startTime);

            re->shaderTime = cg.time / 1000.0f;
            re->reType = RT_SPRITE;
            re->radius = 1.1f;
			re->customShader = cgs.media.railRingsShader;

            re->shaderRGBA[0] = ci->color2[0] * 255;
            re->shaderRGBA[1] = ci->color2[1] * 255;
            re->shaderRGBA[2] = ci->color2[2] * 255;
            re->shaderRGBA[3] = 255;

            le->color[0] = ci->color2[0] * 0.75;
            le->color[1] = ci->color2[1] * 0.75;
            le->color[2] = ci->color2[2] * 0.75;
            le->color[3] = 1.0f;

            le->pos.trType = TR_LINEAR;
            le->pos.trTime = cg.time;

			VectorCopy( move, move2);
            VectorMA(move2, RADIUS , axis[j], move2);
            VectorCopy(move2, le->pos.trBase);

            le->pos.trDelta[0] = axis[j][0]*6;
            le->pos.trDelta[1] = axis[j][1]*6;
            le->pos.trDelta[2] = axis[j][2]*6;
		}

        VectorAdd (move, vec, move);

        j = j + ROTATION < 36 ? j + ROTATION : (j + ROTATION) % 36;
	}
}

/*
==========================
CG_RocketTrail
==========================
*/
static void CG_RocketTrail(centity_t *ent, const weaponInfo_t *wi)
{
	int		step;
	vec3_t	origin, lastPos;
	int		t;
	int		startTime, contents;
	int		lastContents;
	entityState_t	*es;
	vec3_t	up;
	localEntity_t	*smoke;

	if (cg_noProjectileTrail.integer)
	{
		return;
	}

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 50;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ((startTime + step) / step);

	BG_EvaluateTrajectory(&es->pos, cg.time, origin);
	contents = CG_PointContents(origin, -1);

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if (es->pos.trType == TR_STATIONARY)
	{
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory(&es->pos, ent->trailTime, lastPos);
	lastContents = CG_PointContents(lastPos, -1);

	ent->trailTime = cg.time;

	if (contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
	{
		if (contents & lastContents & CONTENTS_WATER)
		{
			CG_BubbleTrail(lastPos, origin, 8);
		}
		return;
	}

	for (; t <= ent->trailTime; t += step)
	{
		BG_EvaluateTrajectory(&es->pos, t, lastPos);

		smoke = CG_SmokePuff(lastPos, up,
					  wi->trailRadius,
					  1, 1, 1, 0.33f,
					  wi->wiTrailTime,
					  t,
					  0,
					  0,
					  cgs.media.smokePuffShader);
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}

}

/*#ifdef MISSIONPACK
//==========================
//CG_NailTrail
//==========================
static void CG_NailTrail(centity_t *ent, const weaponInfo_t *wi)
{
	int		step;
	vec3_t	origin, lastPos;
	int		t;
	int		startTime, contents;
	int		lastContents;
	entityState_t	*es;
	vec3_t	up;
	localEntity_t	*smoke;

	up[0] = 0;
	up[1] = 0;
	up[2] = 0;

	step = 50;

	es = &ent->currentState;
	startTime = ent->trailTime;
	t = step * ((startTime + step) / step);

	BG_EvaluateTrajectory(&es->pos, cg.time, origin);
	contents = CG_PointContents(origin, -1);

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if (es->pos.trType == TR_STATIONARY)
	{
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory(&es->pos, ent->trailTime, lastPos);
	lastContents = CG_PointContents(lastPos, -1);

	ent->trailTime = cg.time;

	if (contents & (CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA))
	{
		if (contents & lastContents & CONTENTS_WATER)
		{
			CG_BubbleTrail(lastPos, origin, 8);
		}
		return;
	}

	for (; t <= ent->trailTime; t += step)
	{
		BG_EvaluateTrajectory(&es->pos, t, lastPos);

		smoke = CG_SmokePuff(lastPos, up,
					  wi->trailRadius,
					  1, 1, 1, 0.33f,
					  wi->wiTrailTime,
					  t,
					  0,
					  0,
					  cgs.media.nailPuffShader);
		// use the optimized local entity add
		smoke->leType = LE_SCALE_FADE;
	}

}
#endif*/


/*
==========================
CG_PlasmaTrail
==========================
*/
static void CG_PlasmaTrail( centity_t *cent, const weaponInfo_t *wi ) {
	localEntity_t	*le;
	refEntity_t		*re;
	entityState_t	*es;
	vec3_t			velocity, xvelocity, origin;
	vec3_t			offset, xoffset;
	vec3_t			v[3];
	int				t, startTime, step;

	float	waterScale = 1.0f;

	if ( cg_noProjectileTrail.integer || cg_oldPlasma.integer ) {
		return;
	}

	step = 50;

	es = &cent->currentState;
	startTime = cent->trailTime;
	t = step * ( (startTime + step) / step );

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	velocity[0] = 60 - 120 * crandom();
	velocity[1] = 40 - 80 * crandom();
	velocity[2] = 100 - 200 * crandom();

	le->leType = LE_MOVE_SCALE_FADE;
	le->leFlags = LEF_TUMBLE;
	le->leBounceSoundType = LEBS_NONE;
	le->leMarkType = LEMT_NONE;

	le->startTime = cg.time;
	le->endTime = le->startTime + 600;

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time;

	AnglesToAxis( cent->lerpAngles, v );

	offset[0] = 2;
	offset[1] = 2;
	offset[2] = 2;

	xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
	xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
	xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];

	VectorAdd( origin, xoffset, re->origin );
	VectorCopy( re->origin, le->pos.trBase );

	if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
		waterScale = 0.10f;
	}

	xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
	xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
	xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
	VectorScale( xvelocity, waterScale, le->pos.trDelta );

	AxisCopy( axisDefault, re->axis );
	re->shaderTime = cg.time / 1000.0f;
	re->reType = RT_SPRITE;
	re->radius = 0.25f;
	re->customShader = cgs.media.railRingsShader;
	le->bounceFactor = 0.3f;

	re->shaderRGBA[0] = wi->flashDlightColor[0] * 63;
	re->shaderRGBA[1] = wi->flashDlightColor[1] * 63;
	re->shaderRGBA[2] = wi->flashDlightColor[2] * 63;
	re->shaderRGBA[3] = 63;

	le->color[0] = wi->flashDlightColor[0] * 0.2;
	le->color[1] = wi->flashDlightColor[1] * 0.2;
	le->color[2] = wi->flashDlightColor[2] * 0.2;
	le->color[3] = 0.25f;

	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	le->angles.trBase[0] = rand()&31;
	le->angles.trBase[1] = rand()&31;
	le->angles.trBase[2] = rand()&31;
	le->angles.trDelta[0] = 1;
	le->angles.trDelta[1] = 0.5;
	le->angles.trDelta[2] = 0;
}

/*
==========================
CG_GrappleTrail
==========================
*/
void CG_GrappleTrail(centity_t *ent, const weaponInfo_t *wi)
{
	vec3_t	origin;
	entityState_t	*es;
	vec3_t			forward, up;
	refEntity_t		beam;

	es = &ent->currentState;

	BG_EvaluateTrajectory(&es->pos, cg.time, origin);
	ent->trailTime = cg.time;

	memset(&beam, 0, sizeof(beam));
	//FIXME adjust for muzzle position
	VectorCopy (cg_entities[ ent->currentState.otherEntityNum ].lerpOrigin, beam.origin);
	beam.origin[2] += 26;
	AngleVectors(cg_entities[ ent->currentState.otherEntityNum ].lerpAngles, forward, NULL, up);
	VectorMA(beam.origin, -6, up, beam.origin);
	VectorCopy(origin, beam.oldorigin);

	if (Distance(beam.origin, beam.oldorigin) < 64)
		return; // Don't draw if close

	beam.reType = RT_LIGHTNING;
	beam.customShader = cgs.media.lightningShader;

	AxisClear(beam.axis);
	beam.shaderRGBA[0] = 0xff;
	beam.shaderRGBA[1] = 0xff;
	beam.shaderRGBA[2] = 0xff;
	beam.shaderRGBA[3] = 0xff;
	trap_R_AddRefEntityToScene(&beam);
}

/*
==========================
CG_GrenadeTrail
==========================
*/
static void CG_GrenadeTrail(centity_t *ent, const weaponInfo_t *wi)
{
	CG_RocketTrail(ent, wi);
}


/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeapon(int weaponNum)
{
	weaponInfo_t	*weaponInfo;
	gitem_t			*item, *ammo;
	char			path[MAX_QPATH];
	vec3_t			mins, maxs;
	int				i;

	weaponInfo = &cg_weapons[weaponNum];

	if (weaponNum == 0)
	{
		return;
	}

	if (weaponInfo->registered)
	{
		return;
	}

	memset(weaponInfo, 0, sizeof(*weaponInfo));
	weaponInfo->registered = qtrue;

	for (item = bg_itemlist + 1; item->classname; item++)
	{
		if (item->giType == IT_WEAPON && item->giTag == weaponNum)
		{
			weaponInfo->item = item;
			break;
		}
	}
	if (!item->classname)
	{
		CG_Error("Couldn't find weapon %i", weaponNum);
	}
	CG_RegisterItemVisuals(item - bg_itemlist);

	// load cmodel before model so filecache works
	weaponInfo->weaponModel = trap_R_RegisterModel(item->world_model[0]);

	// calc midpoint for rotation
	trap_R_ModelBounds(weaponInfo->weaponModel, mins, maxs);
	for (i = 0; i < 3; i++)
	{
		weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * (maxs[i] - mins[i]);
	}

	weaponInfo->weaponIcon = trap_R_RegisterShader(item->icon);
	weaponInfo->ammoIcon = trap_R_RegisterShader(item->icon);

	for (ammo = bg_itemlist + 1; ammo->classname; ammo++)
	{
		if (ammo->giType == IT_AMMO &&
			(ammo->giTag == weaponNum || (weaponNum == WP_INVCHAINGUN && ammo->giTag == WP_MACHINEGUN)))
		{
			break;
		}
	}
	if (ammo->classname && ammo->world_model[0])
	{
		weaponInfo->ammoModel = trap_R_RegisterModel(ammo->world_model[0]);
	}

	strcpy(path, item->world_model[0]);
	COM_StripExtension(path, path);
	strcat(path, "_flash.md3");
	weaponInfo->flashModel = trap_R_RegisterModel(path);

	if (weaponNum == WP_MACHINEGUN || weaponNum == WP_ALIEN_GUN || weaponNum == WP_BFG)
	{
		strcpy(path, item->world_model[0]);
		COM_StripExtension(path, path);
		strcat(path, "_barrel.md3");
		weaponInfo->barrelModel = trap_R_RegisterModel(path);
	}

	strcpy(path, item->world_model[0]);
	COM_StripExtension(path, path);
	strcat(path, "_hand.md3");
	weaponInfo->handsModel = trap_R_RegisterModel(path);

	if (!weaponInfo->handsModel)
	{
		weaponInfo->handsModel = trap_R_RegisterModel("models/weapons2/shotgun/shotgun_hand.md3");
		//weaponInfo->handsModel = trap_R_RegisterModel("models/weapons2/shotgun_hand.md3");
	}

	weaponInfo->loopFireSound = qfalse;

	switch (weaponNum)
	{
	case WP_GAUNTLET:
		MAKERGB(weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f);
		//weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/melee/fstrun.wav", qfalse);
		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/melee/cbfstatck.wav", qfalse);
		cgs.media.sfx_GauntletHit = trap_S_RegisterSound("sound/weapons/melee/cbfsthit.wav", qfalse);
		break;

	case WP_LIGHTNING:
		MAKERGB(weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f);
		weaponInfo->readySound = trap_S_RegisterSound("sound/weapons/melee/fsthum.wav", qfalse);
		weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/lightning/lg_hum.wav", qfalse);

		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/lightning/lg_fire.wav", qfalse);
		cgs.media.lightningShader = trap_R_RegisterShader("lightningBoltNew");
		cgs.media.lightningExplosionModel = trap_R_RegisterModel("models/weaphits/crackle.md3");
		cgs.media.sfx_lghit1 = trap_S_RegisterSound("sound/weapons/lightning/lg_hit.wav", qfalse);
		cgs.media.sfx_lghit2 = trap_S_RegisterSound("sound/weapons/lightning/lg_hit2.wav", qfalse);
		cgs.media.sfx_lghit3 = trap_S_RegisterSound("sound/weapons/lightning/lg_hit3.wav", qfalse);

		break;

	case WP_GRAPPLING_HOOK:
		MAKERGB(weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f);
		weaponInfo->missileModel = trap_R_RegisterModel("models/ammo/rocket/rocket.md3");
		weaponInfo->missileTrailFunc = CG_GrappleTrail;
		weaponInfo->missileDlight = 200;
		weaponInfo->wiTrailTime = 2000;
		weaponInfo->trailRadius = 64;
		MAKERGB(weaponInfo->missileDlightColor, 1, 0.75f, 0);
		weaponInfo->readySound = trap_S_RegisterSound("sound/weapons/melee/fsthum.wav", qfalse);
		weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/melee/fstrun.wav", qfalse);
		break;

	case WP_INVCHAINGUN:
		MAKERGB(weaponInfo->flashDlightColor, 1, 1, 0);
		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/chaingun/chaingf1b.wav", qfalse);
		weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/chaingun/chaingf2b.wav", qfalse);
		weaponInfo->flashSound[2] = trap_S_RegisterSound("sound/weapons/chaingun/chaingf3b.wav", qfalse);
		weaponInfo->flashSound[3] = trap_S_RegisterSound("sound/weapons/chaingun/chaingf4b.wav", qfalse);
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		cgs.media.bulletExplosionShader = trap_R_RegisterShader("bulletExplosion");
		break;

/*#ifdef MISSIONPACK
	case WP_CHAINGUN:
		weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/vulcan/wvulfire.wav", qfalse);
		weaponInfo->loopFireSound = qtrue;
		MAKERGB(weaponInfo->flashDlightColor, 1, 1, 0);
		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/vulcan/vulcanf1b.wav", qfalse);
		weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/vulcan/vulcanf2b.wav", qfalse);
		weaponInfo->flashSound[2] = trap_S_RegisterSound("sound/weapons/vulcan/vulcanf3b.wav", qfalse);
		weaponInfo->flashSound[3] = trap_S_RegisterSound("sound/weapons/vulcan/vulcanf4b.wav", qfalse);
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		cgs.media.bulletExplosionShader = trap_R_RegisterShader("bulletExplosion");
		break;
#endif*/

	case WP_ALIEN_GUN:
		MAKERGB(weaponInfo->flashDlightColor, 1, 1, 0);
		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/machinegun/machgf1b.wav", qfalse);
		weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/machinegun/machgf2b.wav", qfalse);
		weaponInfo->flashSound[2] = trap_S_RegisterSound("sound/weapons/machinegun/machgf3b.wav", qfalse);
		weaponInfo->flashSound[3] = trap_S_RegisterSound("sound/weapons/machinegun/machgf4b.wav", qfalse);
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		break;

	case WP_MACHINEGUN:
		MAKERGB(weaponInfo->flashDlightColor, 1, 1, 0);
		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/m41a-Invas/machgf1b.wav", qfalse);
		weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/m41a-Invas/machgf2b.wav", qfalse);
		weaponInfo->flashSound[2] = trap_S_RegisterSound("sound/weapons/m41a-Invas/machgf3b.wav", qfalse);
		weaponInfo->flashSound[3] = trap_S_RegisterSound("sound/weapons/m41a-Invas/machgf4b.wav", qfalse);
		weaponInfo->ejectBrassFunc = CG_MachineGunEjectBrass;
		cgs.media.bulletExplosionShader = trap_R_RegisterShader("bulletExplosion");
		break;

	case WP_SHOTGUN:
		MAKERGB(weaponInfo->flashDlightColor, 1, 1, 0);
		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/shotgun/sshotf1b.wav", qfalse);
		weaponInfo->ejectBrassFunc = CG_ShotgunEjectBrass;
		break;

	case WP_ROCKET_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel("models/ammo/rocket/rocket.md3");
		weaponInfo->missileSound = trap_S_RegisterSound("sound/weapons/rocket/rockfly.wav", qfalse);
		weaponInfo->missileTrailFunc = CG_RocketTrail;
		weaponInfo->missileDlight = 200;
		weaponInfo->wiTrailTime = 2000;
		weaponInfo->trailRadius = 64;

		MAKERGB(weaponInfo->missileDlightColor, 1, 0.75f, 0);
		MAKERGB(weaponInfo->flashDlightColor, 1, 0.75f, 0);

		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.wav", qfalse);
		cgs.media.rocketExplosionShader = trap_R_RegisterShader("rocketExplosion");
		break;

/*#ifdef MISSIONPACK
	case WP_PROX_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel("models/weaphits/proxmine.md3");
		weaponInfo->missileTrailFunc = CG_GrenadeTrail;
		weaponInfo->wiTrailTime = 700;
		weaponInfo->trailRadius = 32;
		MAKERGB(weaponInfo->flashDlightColor, 1, 0.70f, 0);
		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/proxmine/wstbfire.wav", qfalse);
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader("grenadeExplosion");
		break;
#endif*/

	case WP_GRENADE_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel("models/ammo/grenade1.md3");
		weaponInfo->missileTrailFunc = CG_GrenadeTrail;
		weaponInfo->wiTrailTime = 700;
		weaponInfo->trailRadius = 32;
		MAKERGB(weaponInfo->flashDlightColor, 1, 0.70f, 0);
		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/grenade/grenlf1a.wav", qfalse);
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader("grenadeExplosion");
		break;

/*#ifdef MISSIONPACK
	case WP_NAILGUN:
		weaponInfo->ejectBrassFunc = CG_NailgunEjectBrass;
		weaponInfo->missileTrailFunc = CG_NailTrail;
//		weaponInfo->missileSound = trap_S_RegisterSound("sound/weapons/nailgun/wnalflit.wav", qfalse);
		weaponInfo->trailRadius = 16;
		weaponInfo->wiTrailTime = 250;
		weaponInfo->missileModel = trap_R_RegisterModel("models/weaphits/nail.md3");
		MAKERGB(weaponInfo->flashDlightColor, 1, 0.75f, 0);
		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/nailgun/wnalfire.wav", qfalse);
		break;
#endif*/

	case WP_PLASMAGUN:
//		weaponInfo->missileModel = cgs.media.invulnerabilityPowerupModel;
		weaponInfo->missileTrailFunc = CG_PlasmaTrail;
		weaponInfo->missileSound = trap_S_RegisterSound("sound/weapons/plasma/lasfly.wav", qfalse);
		MAKERGB(weaponInfo->flashDlightColor, 0.2f, 1.0f, 0.2f);
		//weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/plasma/hyprbf1a.wav", qfalse);
		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/plasmag/fire1.wav", qfalse);
		weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/plasmag/fire2.wav", qfalse);
		weaponInfo->flashSound[2] = trap_S_RegisterSound("sound/weapons/plasmag/fire3.wav", qfalse);
		cgs.media.plasmaExplosionShader = trap_R_RegisterShader("Invasion/AcidExplosion");//trap_R_RegisterShader("plasmaAcidExplosion");
		cgs.media.railRingsShader = trap_R_RegisterShader("railDisc");

		cgs.media.sfx_plasmaexp = trap_S_RegisterSound ("sound/weapons/plasmag/Explode.wav", qfalse);//trap_S_RegisterSound ("sound/weapons/plasma/plasmx1a.wav", qfalse);
		cgs.media.sfx_PlasmaFireSpec = trap_S_RegisterSound ("sound/weapons/plasmag/FireSpec.wav", qfalse);
		cgs.media.sfx_PlasmaExplodeSpec = trap_S_RegisterSound ("sound/weapons/plasmag/ExplodeSpec.wav", qfalse);
		break;

	 case WP_FLAMETHROWER:
		weaponInfo->missileSound = trap_S_RegisterSound("sound/weapons/flamethrower/flamefly.wav", qfalse);
		MAKERGB(weaponInfo->flashDlightColor, 1.0f, 0.75f, 0.15f);
		//for (i = 0; i < 4; ++i)
		//	weaponInfo->flashSound[i] = trap_S_RegisterSound(va("sound/weapons/flamethrower/fire%da.wav", i + 1), qfalse);
		cgs.media.FlameExplosionShader = trap_R_RegisterShader("flamethrower/FlameExplosion");
		weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/flamethrower/firing.wav", qfalse);
		weaponInfo->firing2Sound = weaponInfo->firingSound;

		cgs.media.FlameTrail = trap_R_RegisterShader("invasion/FlameTrail");
		cgs.media.FlameTrailIdle = trap_R_RegisterShader("invasion/FlameTrailIdle");
		cgs.media.sfx_FlameExp = trap_S_RegisterSound ("sound/weapons/flamethrower/explosion.wav", qfalse);
		break;

	case WP_RAILGUN:
		weaponInfo->readySound = trap_S_RegisterSound("sound/weapons/railgun/rg_hum.wav", qfalse);
		MAKERGB(weaponInfo->flashDlightColor, 1, 0.5f, 0);
		//weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/railgun/railgf1a.wav", qfalse);
		//cgs.media.railExplosionShader = trap_R_RegisterShader("railExplosion");
		//cgs.media.railRingsShader = trap_R_RegisterShader("railDisc");
		//cgs.media.railCoreShader = trap_R_RegisterShader("railCore");
		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/railgun/chaingf1b.wav", qfalse);
		weaponInfo->flashSound[1] = trap_S_RegisterSound("sound/weapons/railgun/chaingf2b.wav", qfalse);
		weaponInfo->flashSound[2] = trap_S_RegisterSound("sound/weapons/railgun/chaingf3b.wav", qfalse);
		weaponInfo->flashSound[3] = trap_S_RegisterSound("sound/weapons/railgun/chaingf4b.wav", qfalse);
		break;

	case WP_BFG:
		weaponInfo->readySound = trap_S_RegisterSound("sound/weapons/bfg/bfg_hum.wav", qfalse);
		MAKERGB(weaponInfo->flashDlightColor, 1, 0.7f, 1);
		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/bfg/bfg_fire.wav", qfalse);
		cgs.media.bfgExplosionShader = trap_R_RegisterShader("bfgExplosion");
		weaponInfo->missileModel = trap_R_RegisterModel("models/weaphits/bfg.md3");
		weaponInfo->missileSound = trap_S_RegisterSound("sound/weapons/rocket/rockfly.wav", qfalse);
		weaponInfo->firingSound = trap_S_RegisterSound("sound/weapons/Bfg/bfg_firing.wav", qfalse);
		weaponInfo->firing2Sound = weaponInfo->firingSound;
		cgs.media.sfx_BfgCharge = trap_S_RegisterSound("sound/weapons/Bfg/bfg_Charge.wav", qfalse);
		break;

	 default:
		MAKERGB(weaponInfo->flashDlightColor, 1, 1, 1);
		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/rocket/rocklf1a.wav", qfalse);
		break;
	}
}

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals(int itemNum)
{
	itemInfo_t		*itemInfo;
	gitem_t			*item;
	int i;

	if (itemNum < 0 || itemNum >= bg_numItems)
	{
		CG_Error( "CG_RegisterItemVisuals: itemNum %d out of range [0-%d]", itemNum, bg_numItems-1 );
	}

	itemInfo = &cg_items[ itemNum ];
	if (itemInfo->registered)
	{
		return;
	}

	item = &bg_itemlist[ itemNum ];

	memset(itemInfo, 0, sizeof(&itemInfo));
	itemInfo->registered = qtrue;

	if (cgs.gametype == GT_INVASION || cgs.gametype == GT_DESTROY)
	{
		if (!Q_stricmp(item->pickup_name, "Blue Flag"))
		{
			gitem_t *NewItem = BG_FindItem("Alien Egg");

			if (NewItem)
				item = NewItem;
		}
		else if (!Q_stricmp(item->pickup_name, "Red Flag"))
		{
			gitem_t *NewItem = BG_FindItem("Teleporter");

			if (NewItem)
				item = NewItem;
		}
	}

	for (i = 0; i < MAX_ITEM_MODELS; ++i)
	{
		if (item->world_model[i])
			itemInfo->models[i] = trap_R_RegisterModel(item->world_model[i]);
	}

	itemInfo->icon = trap_R_RegisterShader(item->icon);

	if (item->giType == IT_WEAPON)
	{
		CG_RegisterWeapon(item->giTag);
	}
	else if (item->giType == IT_TEAM)
	{
		if (!Q_stricmp(item->pickup_name, "Red Flag") || !Q_stricmp(item->pickup_name, "Teleporter"))
		{
			cgs.RedFlagDec = 8;
			cgs.media.redFlagModel = itemInfo->models[0];
			if (cgs.media.redFlagShader[0] == 0)
			{
				if (!Q_stricmp(item->pickup_name, "Teleporter"))
				{
					cgs.media.redFlagShader[0] =
					cgs.media.redFlagShader[1] =
					cgs.media.redFlagShader[2] = trap_R_RegisterShaderNoMip("icons/iconf_marines");
				}
				else
				{
					cgs.media.redFlagShader[0] = trap_R_RegisterShaderNoMip("icons/iconf_red1");
					cgs.media.redFlagShader[1] = trap_R_RegisterShaderNoMip("icons/iconf_red2");
					cgs.media.redFlagShader[2] = trap_R_RegisterShaderNoMip("icons/iconf_red3");
				}

				cgs.media.RedFlagIcon = itemInfo->icon;
			}
		}
		else if (!Q_stricmp(item->pickup_name, "Blue Flag") || !Q_stricmp(item->pickup_name, "Alien Egg"))
		{
			if (!Q_stricmp(item->pickup_name, "Alien Egg"))
				cgs.BlueFlagDec = -2;
			else
				cgs.BlueFlagDec = 8;

			cgs.media.blueFlagModel = itemInfo->models[0];

			if (cgs.media.blueFlagShader[0] == 0)
			{
			 	if (!Q_stricmp(item->pickup_name, "Alien Egg"))
				{
					cgs.media.blueFlagShader[0] = trap_R_RegisterShaderNoMip("icons/iconf_egg1");
					cgs.media.blueFlagShader[1] = trap_R_RegisterShaderNoMip("icons/iconf_egg2");
					cgs.media.blueFlagShader[2] = trap_R_RegisterShaderNoMip("icons/iconf_egg3");
				}
				else
				{
					cgs.media.blueFlagShader[0] = trap_R_RegisterShaderNoMip("icons/iconf_blu1");
					cgs.media.blueFlagShader[1] = trap_R_RegisterShaderNoMip("icons/iconf_blu2");
					cgs.media.blueFlagShader[2] = trap_R_RegisterShaderNoMip("icons/iconf_blu3");
				}

				cgs.media.BlueFlagIcon = itemInfo->icon;
			}
		}
	}

	//
	// powerups have an accompanying ring or sphere
	//
	/*if (item->giType == IT_POWERUP || item->giType == IT_HEALTH ||
		item->giType == IT_ARMOR || item->giType == IT_HOLDABLE)
	{
		if (item->world_model[1])
		{
			itemInfo->models[1] = trap_R_RegisterModel(item->world_model[1]);
		}
	}*/
}


/*
========================================================================================

VIEW WEAPON

========================================================================================
*/

/*
=================
CG_MapTorsoToWeaponFrame

=================
*/
static int CG_MapTorsoToWeaponFrame(clientInfo_t *ci, int frame)
{

	// change weapon
	if (frame >= ci->animations[TORSO_DROP].firstFrame
		&& frame < ci->animations[TORSO_DROP].firstFrame + 9)
		{
		return frame - ci->animations[TORSO_DROP].firstFrame + 6;
	}

	// stand attack
	if (frame >= ci->animations[TORSO_ATTACK].firstFrame
		&& frame < ci->animations[TORSO_ATTACK].firstFrame + 6)
		{
		return 1 + frame - ci->animations[TORSO_ATTACK].firstFrame;
	}

	// stand attack 2
	if (frame >= ci->animations[TORSO_ATTACK2].firstFrame
		&& frame < ci->animations[TORSO_ATTACK2].firstFrame + 6)
	{
		return 1 + frame - ci->animations[TORSO_ATTACK2].firstFrame;
	}

	return 0;
}


/*
==============
CG_CalculateWeaponPosition
==============
*/
static void CG_CalculateWeaponPosition(vec3_t origin, vec3_t angles)
{
	float	scale;
	int		delta;
	float	fracsin;

	VectorCopy(cg.refdef.vieworg, origin);
	VectorCopy(cg.refdefViewAngles, angles);

	// on odd legs, invert some angles
	if (cg.bobcycle & 1)
	{
		scale = -cg.xyspeed;
	}
	else
	{
		scale = cg.xyspeed;
	}

	// gun angles from bobbing
	angles[ROLL] += scale * cg.bobfracsin * 0.005;
	angles[YAW] += scale * cg.bobfracsin * 0.01;
	angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.005;

	// drop the weapon when landing
	delta = cg.time - cg.landTime;
	if (delta < LAND_DEFLECT_TIME)
	{
		origin[2] += cg.landChange*0.25 * delta / LAND_DEFLECT_TIME;
	}
	else if (delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME)
	{
		origin[2] += cg.landChange*0.25 *
			(LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
	}

#if 0
	// drop the weapon when stair climbing
	delta = cg.time - cg.stepTime;
	if (delta < STEP_TIME/2)
	{
		origin[2] -= cg.stepChange*0.25 * delta / (STEP_TIME/2);
	}
	else if (delta < STEP_TIME)
	{
		origin[2] -= cg.stepChange*0.25 * (STEP_TIME - delta) / (STEP_TIME/2);
	}
#endif

	// idle drift
	scale = cg.xyspeed + 40;
	fracsin = sin(cg.time * 0.001);
	angles[ROLL] += scale * fracsin * 0.01;
	angles[YAW] += scale * fracsin * 0.01;
	angles[PITCH] += scale * fracsin * 0.01;
}


void Inv_DrawFlameTrail(vec3_t origin, vec3_t oldorigin, float NormalSize, float MaxSize, vec3_t AbsUp)
{
	vec3_t		forward;
	polyVert_t	verts[5], Poly[3];
	float		length, StartSize, FinishSize;
	vec3_t		finish;
	int i, j;
	vec3_t Up = { 0, 0, 1 }, Right = { 1, 0, 0 };
	qhandle_t Shader;

	VectorSubtract(oldorigin, origin, forward);
	length = VectorNormalize(forward);

	VectorMA(origin, length, forward, finish);

	if (AbsUp)
	{
		CrossProduct(forward, AbsUp, Right);
		CrossProduct(forward, Right, Up);
	}
	else if (fabs(DotProduct(forward, Up)) < 0.90f)
	{
		CrossProduct(forward, Up, Right);
		CrossProduct(forward, Right, Up);
	}
	else
	{
		assert(fabs(DotProduct(forward, Right)) < 0.90f);

		CrossProduct(forward, Right, Up);
		CrossProduct(forward, Up, Right);
	}

	VectorNormalize(Up);
	VectorNormalize(Right);

	for (i = 0; i < 5; ++i)
	{
		for (j = 0; j < 3; ++j)
			verts[i].modulate[j] = 255;
	}

	if (NormalSize)
	{
		length = 1 - (length / NormalSize);

		StartSize = 2;
		FinishSize = MaxSize;
		Shader = cgs.media.FlameTrail;

		verts[4].st[0] = 1;
		verts[4].st[1] = 0.5f;
		verts[4].modulate[3] = (byte) 255.9f;
		VectorCopy(origin, verts[4].xyz);
	}
	else
	{
		length = 0;
		StartSize = FinishSize = MaxSize;
		Shader = cgs.media.FlameTrailIdle;
	}

	verts[0].st[0] = length;
	verts[0].st[1] = 0;
	verts[0].modulate[3] = 0;

	verts[1].st[0] = length;
	verts[1].st[1] = 1;
	verts[1].modulate[3] = 0;

	verts[2].st[0] = 1;
	verts[2].st[1] = 1;

	verts[3].st[0] = 1;
	verts[3].st[1] = 0;

	if (NormalSize)
	{
		verts[2].modulate[3] = 0;//(byte) 255.9f;
		verts[3].modulate[3] = 0;//(byte) 255.9f;
	}
	else
	{
		verts[2].modulate[3] = (byte) 255.9f;
		verts[3].modulate[3] = (byte) 255.9f;
	}


	for (i = 0; i < 2; ++i)
	{
		VectorMA(finish, FinishSize, Right, verts[0].xyz);
		VectorMA(finish, -FinishSize, Right, verts[1].xyz);
		VectorMA(origin, -StartSize, Right, verts[2].xyz);
		VectorMA(origin, StartSize, Right, verts[3].xyz);

		if (NormalSize)
		{
			Poly[0] = verts[3];
			Poly[1] = verts[0];
			Poly[2] = verts[4];
			trap_R_AddPolyToScene(Shader, 3, Poly);

			Poly[0] = verts[0];
			Poly[1] = verts[4];
			Poly[2] = verts[1];
			trap_R_AddPolyToScene(Shader, 3, Poly);

			Poly[0] = verts[4];
			Poly[1] = verts[1];
			Poly[2] = verts[2];
			trap_R_AddPolyToScene(Shader, 3, Poly);
		}
		else
			trap_R_AddPolyToScene(Shader, 4, verts);

		VectorCopy(Up, Right);
	}
}

enum
{
	es_FlameTrailFire		= 30,
	es_FlameTrailIdle		= 5,
};

void Inv_FlameTrail(centity_t *cent, vec3_t origin, qboolean Firing)
{
	trace_t  trace;
	refEntity_t  beam;
	vec3_t   forward, right, up;
	vec3_t   endPoint;
	qboolean changed = qfalse;
	float dist;

	memset(&beam, 0, sizeof(beam));

	// CPMA  "true" lightning
	if ((cent->currentState.number == cg.predictedPlayerState.clientNum)
		&& (cg_trueLightning.value != 0))
	{
		AngleVectors(cg.predictedPlayerState.viewangles, forward, right, up);
	}
	else
		AngleVectors(cent->lerpAngles, forward, right, up);

	if (Firing)
	{
		if (cent->Flame[0] != -1 || cent->Flame[1] != -1)
		{
			vec3_t FlamePos;
			changed = qtrue;

			if (cent->Flame[0] == -1)
			{
				VectorCopy(cg_entities[cent->Flame[1]].lerpOrigin, FlamePos);
			}
			else if (cent->Flame[1] == -1)
			{
				VectorCopy(cg_entities[cent->Flame[0]].lerpOrigin, FlamePos);
			}
			else
			{
				VectorCopy(cg_entities[cent->Flame[0]].lerpOrigin, FlamePos);
				VectorAdd(FlamePos, cg_entities[cent->Flame[1]].lerpOrigin, FlamePos);
				VectorScale(FlamePos, 0.5f, FlamePos);
			}

			VectorSubtract(FlamePos, origin, forward);
			dist = VectorNormalize(forward);

			if (dist > es_FlameTrailFire)
			{
				VectorMA(origin, es_FlameTrailFire, forward, endPoint);
			}
			else if (dist < es_FlameTrailFire * 0.5f)
			{
				VectorMA(origin, es_FlameTrailFire * 0.5f, forward, endPoint);
			}
			else
			{
				VectorCopy(FlamePos, endPoint);
			}
		}
		else
		{
			VectorMA(origin, es_FlameTrailFire, forward, endPoint);
			VectorMA(endPoint, 0.5f, up, endPoint);
			dist = 10000;
		}

		// see if it hits a wall
		CG_Trace( &trace, origin, vec3_origin, vec3_origin, endPoint,
			cent->currentState.number, MASK_SHOT);

		if (dist >= 200)
			Inv_DrawFlameTrail(origin, trace.endpos, es_FlameTrailFire, 4, NULL);
		else
			Inv_DrawFlameTrail(origin, trace.endpos, es_FlameTrailFire, 10, NULL);
	}
	else
	{
		int i;
		vec3_t Delta;

		VectorAdd(forward, right, forward);
		VectorNormalize(forward);

		VectorMA(origin, es_FlameTrailIdle, up, endPoint);
		VectorMA(endPoint, -0.01f, cent->currentState.pos.trDelta, endPoint);

		VectorSubtract(cent->currentState.apos.trBase, cent->LastAngles, Delta);

		for (i = 0; i < 3; ++i)
		{
			if (Delta[i] > 180)
				Delta[i] -= 360;
			else if (Delta[i] < -180)
				Delta[i] += 360;

			if (Delta[i] > 5)
				Delta[i] = 5;
			else if (Delta[i] < -5)
				Delta[i] = -5;
		}

		VectorMA(endPoint, Delta[YAW] * 1.0f, right, endPoint);
		VectorMA(endPoint, Delta[PITCH] * 1.0f, up, endPoint);

		Inv_DrawFlameTrail(origin, endPoint, 0, 2, forward);
	}

	cent->Flame[0] = cent->Flame[1] = -1;
}


/*
===============
CG_LightningBolt

Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
The cent should be the non-predicted cent if it is from the player,
so the endpoint will reflect the simulated strike (lagging the predicted
angle)
===============
*/
static void CG_LightningBolt(centity_t *cent, vec3_t origin)
{
	trace_t  trace;
	refEntity_t  beam;
	vec3_t   forward, up;
	vec3_t   muzzlePoint, endPoint;

	if (cent->currentState.weapon == WP_FLAMETHROWER)
	{
		Inv_FlameTrail(cent, origin, qtrue);
		return;
	}

	if (cent->currentState.weapon != WP_LIGHTNING)
		return;

	memset( &beam, 0, sizeof( beam ) );

	// CPMA  "true" lightning
	if ((cent->currentState.number == cg.predictedPlayerState.clientNum)
		&& (cg_trueLightning.value != 0))
	{
		AngleVectors(cg.predictedPlayerState.viewangles, forward, NULL, up);
		VectorCopy(cent->lerpOrigin, muzzlePoint );
	}
	else
	{
		// !CPMA
		AngleVectors(cent->lerpAngles, forward, NULL, up );
		VectorCopy(cent->lerpOrigin, muzzlePoint );
	}

	// FIXME: crouch
	muzzlePoint[2] += DEFAULT_VIEWHEIGHT;
	// project forward by the lightning range
	VectorMA( muzzlePoint, 14, forward, muzzlePoint );

	VectorMA( muzzlePoint, LIGHTNING_RANGE, forward, endPoint );

	// see if it hits a wall
	CG_Trace( &trace, muzzlePoint, vec3_origin, vec3_origin, endPoint,
		cent->currentState.number, MASK_SHOT );

	// this is the endpoint
	VectorCopy( trace.endpos, beam.oldorigin );
	beam.customShader = cgs.media.lightningShader;

	// use the provided origin, even though it may be slightly
	// different than the muzzle origin
	VectorCopy( origin, beam.origin );

	beam.reType = RT_LIGHTNING;
	trap_R_AddRefEntityToScene( &beam );

	// add the impact flare if it hit something
	if (trace.fraction < 1.0)
	{
		vec3_t	angles;
		vec3_t	dir;

		VectorSubtract( beam.oldorigin, beam.origin, dir );
		VectorNormalize( dir );

		memset( &beam, 0, sizeof( beam ) );
		beam.hModel = cgs.media.lightningExplosionModel;

		VectorMA( trace.endpos, -16, dir, beam.origin );

		// make a random orientation
		angles[0] = rand() % 360;
		angles[1] = rand() % 360;
		angles[2] = rand() % 360;
		AnglesToAxis( angles, beam.axis );
		trap_R_AddRefEntityToScene( &beam );
	}
}

/*
===============
CG_SpawnRailTrail

Origin will be the exact tag point, which is slightly
different than the muzzle point used for determining hits.
===============
*/
/*static void CG_SpawnRailTrail(centity_t *cent, vec3_t origin)
{
	clientInfo_t	*ci;

	if (cent->currentState.weapon != WP_RAILGUN)
	{
		return;
	}
	if (!cent->pe.railgunFlash)
	{
		return;
	}
	cent->pe.railgunFlash = qtrue;
	ci = &cgs.clientinfo[ cent->currentState.clientNum ];
	CG_RailTrail(ci, origin, cent->pe.railgunImpact);
}*/


/*
======================
CG_MachinegunSpinAngle
======================
*/
#define		SPIN_SPEED				0.9f
#define		MACHINESPIN_SPEED		0.8f
#define		INVCHAINSPIN_SPEED	1.2f
#define		COAST_TIME	1000
static float	CG_MachinegunSpinAngle(centity_t *cent, weapon_t Weapon)
{
	int		delta;
	float	angle;
	float	speed;
	float SpinSpeed;

	switch (Weapon)
	{
		case WP_ALIEN_GUN:
		case WP_MACHINEGUN:
			SpinSpeed = MACHINESPIN_SPEED;
			break;
		case WP_INVCHAINGUN:
			SpinSpeed = INVCHAINSPIN_SPEED;
			break;
		default:
			SpinSpeed = SPIN_SPEED;
			break;
	}

	delta = cg.time - cent->pe.barrelTime;
	if (cent->pe.barrelSpinning)
	{
		if (Weapon == WP_INVCHAINGUN)
		{
			int Coeff = delta;
			if (Coeff > COAST_TIME/2)
				Coeff = COAST_TIME/2;

			speed = SpinSpeed * ((float) Coeff / (COAST_TIME/2));
			angle = cent->pe.barrelAngle + delta * speed;
		}
		else
			angle = cent->pe.barrelAngle + delta * SpinSpeed;
	}
	else
	{
		if (delta > COAST_TIME)
		{
			delta = COAST_TIME;
		}

		speed = 0.5 * (SPIN_SPEED + (float)(COAST_TIME - delta) / COAST_TIME);
		angle = cent->pe.barrelAngle + delta * speed;
	}

	if (cent->pe.barrelSpinning == !(cent->currentState.eFlags & EF_FIRING))
	{
		cent->pe.barrelTime = cg.time;
		cent->pe.barrelAngle = AngleMod(angle);
		cent->pe.barrelSpinning = !!(cent->currentState.eFlags & EF_FIRING);
/*#ifdef MISSIONPACK
		if (cent->currentState.weapon == WP_CHAINGUN && !cent->pe.barrelSpinning)
		{
			trap_S_StartSound(NULL, cent->currentState.number, CHAN_WEAPON, trap_S_RegisterSound("sound/weapons/vulcan/wvulwind.wav", qfalse));
		}
#endif*/
	}

	return angle;
}


/*
========================
CG_AddWeaponWithPowerups
========================
*/
static void CG_AddWeaponWithPowerups(refEntity_t *gun, int powerups)
{
	// add powerup effects
	if (powerups & (1 << PW_INVIS))
	{
		gun->customShader = cgs.media.invisShader;
		trap_R_AddRefEntityToScene(gun);
	}
	else
	{
		/*static int Toto = 0;

		gun->frame = (cg.time / 50) % 5;
		gun->oldframe = (gun->frame - 1) % 5;
		gun->backlerp = 1.0f - (cg.time % 50) * 0.02f;
		++Toto;
		if (Toto >= 100)
			Toto = 0;*/

		trap_R_AddRefEntityToScene(gun);

		if (powerups & (1 << PW_BATTLESUIT))
		{
			gun->customShader = cgs.media.battleWeaponShader;
			trap_R_AddRefEntityToScene(gun);
		}
		if (powerups & (1 << PW_QUAD))
		{
			gun->customShader = cgs.media.quadWeaponShader;
			trap_R_AddRefEntityToScene(gun);
		}
	}
}


/*
=============
CG_AddPlayerWeapon

Used for both the view weapon (ps is valid) and the world modelother character models (ps is NULL)
The main player will have this called for BOTH cases, so effects like light and
sound should only be done on the world model case.
=============
*/
void CG_AddPlayerWeapon(refEntity_t *parent, playerState_t *ps, centity_t *cent, int team)
{
	refEntity_t	gun;
	refEntity_t	barrel;
	refEntity_t	flash;
	vec3_t		angles;
	weapon_t	weaponNum;
	weaponInfo_t	*weapon;
	centity_t	*nonPredictedCent;
//	int	col;
	clientInfo_t *ci = &cgs.clientinfo[cent->currentState.clientNum];
	int Race = -1;

	weaponNum = cent->currentState.weapon;

	if (ci->team == cgs.InvasionInfo.AlienTeam)
	{
		if (weaponNum == WP_MACHINEGUN)
			weaponNum = WP_ALIEN_GUN;

		Race = ci->Class & e_Class_AlienRaceMask;
	}

	if (Race == e_Selection_Xenomorph)// || Race == e_Selection_Rad)
		return;


	CG_RegisterWeapon(weaponNum);
	weapon = &cg_weapons[weaponNum];

	// add the weapon
	memset(&gun, 0, sizeof(gun));
	VectorCopy(parent->lightingOrigin, gun.lightingOrigin);
	gun.shadowPlane = parent->shadowPlane;
	gun.renderfx = parent->renderfx;

	// set custom shading for railgun refire rate
	if (ps)
	{
		/*if (cg.predictedPlayerState.weapon == WP_RAILGUN		//Too:
			&& (cg.predictedPlayerState.weaponstate == WEAPON_FIRING || cg.predictedPlayerState.weaponstate == WEAPON_FIRING2))
		{
			float	f;

			f = (float)cg.predictedPlayerState.weaponTime / 1500;
			gun.shaderRGBA[1] = 0;
			gun.shaderRGBA[0] =
			gun.shaderRGBA[2] = 255 * (1.0 - f);
		}
		else */
		{
			gun.shaderRGBA[0] = 255;
			gun.shaderRGBA[1] = 255;
			gun.shaderRGBA[2] = 255;
			gun.shaderRGBA[3] = 255;
		}
	}

	gun.hModel = weapon->weaponModel;
	if (!gun.hModel)
	{
		return;
	}

	if (!ps)
	{
		// add weapon ready sound
		cent->pe.lightningFiring = qfalse;
		if ((cent->currentState.eFlags & EF_FIRING) && weapon->firingSound)
		{
			// lightning gun and guantlet make a different sound when fire is held down
			trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->firingSound);
			cent->pe.lightningFiring = qtrue;
		}
		else if (weapon->readySound)
		{
			trap_S_AddLoopingSound(cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->readySound);
		}
	}

	CG_PositionEntityOnTag(&gun, parent, parent->hModel, "tag_weapon");

	//if (Race != e_Selection_Rad)
	CG_AddWeaponWithPowerups(&gun, cent->currentState.powerups);

	// add the spinning barrel
	//if (weaponNum == WP_MACHINEGUN || weaponNum == WP_ALIEN_GUN || weaponNum == WP_BFG)
	{
		if (weapon->barrelModel)
		{
			memset(&barrel, 0, sizeof(barrel));
			VectorCopy(parent->lightingOrigin, barrel.lightingOrigin);
			barrel.shadowPlane = parent->shadowPlane;
			barrel.renderfx = parent->renderfx;

			barrel.hModel = weapon->barrelModel;
			angles[YAW] = 0;
			angles[PITCH] = 0;
			angles[ROLL] = CG_MachinegunSpinAngle(cent, weaponNum);
			AnglesToAxis(angles, barrel.axis);

			CG_PositionRotatedEntityOnTag(&barrel, &gun, weapon->weaponModel, "tag_barrel");

			CG_AddWeaponWithPowerups(&barrel, cent->currentState.powerups);
		}
	}

	// make sure we aren't looking at cg.predictedPlayerEntity for LG
	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	// if the index of the nonPredictedCent is not the same as the clientNum
	// then this is a fake player (like on the single player podiums), so
	// go ahead and use the cent
	if ((nonPredictedCent - cg_entities) != cent->currentState.clientNum)
	{
		nonPredictedCent = cent;
	}

	// add the flash
	if ((weaponNum == WP_LIGHTNING || weaponNum == WP_GAUNTLET
			|| weaponNum == WP_GRAPPLING_HOOK || weaponNum == WP_FLAMETHROWER)
		&& (nonPredictedCent->currentState.eFlags & EF_FIRING))
	{
		// continuous flash
	}
	else
	{
		// impulse flash
		if (cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME && !cent->pe.railgunFlash)
		{
			if (weaponNum == WP_FLAMETHROWER
				&& (ps || cg.renderingThirdPerson ||
					cent->currentState.number != cg.predictedPlayerState.clientNum))
			{
				angles[YAW] = angles[PITCH] = angles[ROLL] = 0;
				AnglesToAxis(angles, flash.axis);
				if (CG_PositionRotatedEntityOnTag(&flash, &gun, weapon->weaponModel, "tag_flame"))
					Inv_FlameTrail(nonPredictedCent, flash.origin, qfalse);
			}

			return;
		}
	}

	memset(&flash, 0, sizeof(flash));
	VectorCopy(parent->lightingOrigin, flash.lightingOrigin);
	flash.shadowPlane = parent->shadowPlane;
	flash.renderfx = parent->renderfx;

	flash.hModel = weapon->flashModel;
	if (!flash.hModel)
	{
		//return;
	}
	angles[YAW] = 0;
	angles[PITCH] = 0;
	angles[ROLL] = crandom() * 10;
	AnglesToAxis(angles, flash.axis);

	// colorize the railgun blast
	/*if (weaponNum == WP_RAILGUN)		//Too:
	{
		clientInfo_t	*ci;

		ci = &cgs.clientinfo[ cent->currentState.clientNum ];
	 	flash.shaderRGBA[0] = 255 * ci->color1[0];
		flash.shaderRGBA[1] = 255 * ci->color1[1];
		flash.shaderRGBA[2] = 255 * ci->color1[2];
	} */

	if (CG_PositionRotatedEntityOnTag(&flash, &gun, weapon->weaponModel, "tag_flash"))
	{
		VectorCopy(flash.origin, cent->pe.Flash);
	}
	else
	{
		VectorMA(gun.origin, 25, gun.axis[0], cent->pe.Flash);
		VectorMA(cent->pe.Flash, 5, gun.axis[2], cent->pe.Flash);
		VectorCopy(cent->pe.Flash, flash.origin);
	}

	if (flash.hModel)
	{
		trap_R_AddRefEntityToScene(&flash);
	}

	if (ps || cg.renderingThirdPerson ||
		cent->currentState.number != cg.predictedPlayerState.clientNum)
	{
		// add lightning bolt
		CG_LightningBolt(nonPredictedCent, flash.origin);

		// add rail trail
		//CG_SpawnRailTrail(cent, flash.origin);

		if (weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2])
		{
			trap_R_AddLightToScene(flash.origin, 300 + (rand()&31), weapon->flashDlightColor[0],
				weapon->flashDlightColor[1], weapon->flashDlightColor[2]);
		}
	}
}

/*
==============
CG_AddViewWeapon

Add the weapon, and flash for the player's view
==============
*/
void CG_AddViewWeapon(playerState_t *ps)
{
	refEntity_t	hand;
	centity_t	*cent;
	clientInfo_t	*ci;
	float		fovOffset;
	vec3_t		angles;
	weaponInfo_t	*weapon;
	int WeaponNum;

	if (ps->persistant[PERS_TEAM] == TEAM_SPECTATOR)
	{
		return;
	}

	if (ps->pm_type == PM_INTERMISSION)
	{
		return;
	}

	// no gun if in third person view or a camera is active
	//if ( cg.renderingThirdPerson || cg.cameraMode) {
	if (cg.renderingThirdPerson)
	{
		return;
	}

	// allow the gun to be completely removed
	if (!cg_drawGun.integer)
	{
		vec3_t		origin;

		if (cg.predictedPlayerState.eFlags & EF_FIRING)
		{
			// special hack for lightning gun...
			VectorCopy(cg.refdef.vieworg, origin);
			VectorMA(origin, -8, cg.refdef.viewaxis[2], origin);
			CG_LightningBolt(&cg_entities[ps->clientNum], origin);
		}
		return;
	}

	// don't draw if testing a gun model
	if (cg.testGun)
	{
		return;
	}

	// drop gun lower at higher fov
	if (cg_fov.integer > 90)
	{
		fovOffset = -0.13f * (cg_fov.integer - 90);	//Too : slighty changed from -0.2 to -0.13
	}
	else
	{
		fovOffset = 0;
	}

	cent = &cg.predictedPlayerEntity;	// &cg_entities[cg.snap->ps.clientNum];

	WeaponNum = ps->weapon;

	{
		clientInfo_t *ci;
		ci = &cgs.clientinfo[cent->currentState.clientNum];

		if (ci->team == cgs.InvasionInfo.AlienTeam && WeaponNum == WP_MACHINEGUN)
			WeaponNum = WP_ALIEN_GUN;
	}

	CG_RegisterWeapon(WeaponNum);
	weapon = &cg_weapons[WeaponNum];

	memset (&hand, 0, sizeof(hand));

	// set up gun position
	CG_CalculateWeaponPosition(hand.origin, angles);

	VectorMA(hand.origin, cg_gun_x.value, cg.refdef.viewaxis[0], hand.origin);
	VectorMA(hand.origin, cg_gun_y.value, cg.refdef.viewaxis[1], hand.origin);
	VectorMA(hand.origin, (cg_gun_z.value+fovOffset), cg.refdef.viewaxis[2], hand.origin);

	if (cent->pe.GunBackTime)
	{
		float coeff;

		coeff = (cg.time - cent->pe.GunBackTime) / ((float)cent->pe.GunBackTotalTime);

		if (coeff >= 1)
			cent->pe.GunBackTime = cent->pe.GunBackTotalTime = 0;
		else
		{
			int i;
			angles[PITCH] -= 60 * sin(coeff * 3.141); //Too: 80

			for (i = 0; i < 3; ++i)
				hand.origin[i] -= cg.refdef.viewaxis[1][i] * sin(coeff * 3.141) * 10;
		}
	}

	AnglesToAxis(angles, hand.axis);

	// map torso animations to weapon animations
	if (cg_gun_frame.integer)
	{
		// development tool
		hand.frame = hand.oldframe = cg_gun_frame.integer;
		hand.backlerp = 0;
	}
	else
	{
		// get clientinfo for animation map
		ci = &cgs.clientinfo[ cent->currentState.clientNum ];
		hand.frame = CG_MapTorsoToWeaponFrame(ci, cent->pe.torso.frame);
		hand.oldframe = CG_MapTorsoToWeaponFrame(ci, cent->pe.torso.oldFrame);
		hand.backlerp = cent->pe.torso.backlerp;
	}

	hand.hModel = weapon->handsModel;
	hand.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON | RF_MINLIGHT;

	// add everything onto the hand
	CG_AddPlayerWeapon(&hand, ps, &cg.predictedPlayerEntity, ps->persistant[PERS_TEAM]);
}

/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/

/*
===================
CG_DrawWeaponSelect
===================
*/
void CG_DrawWeaponSelect(void)
{
	int		i;
	int		bits;
	int		count;
	int		x, y, w;
	char	*name = NULL;
	float	*color;
	clientInfo_t *ci;

	// don't display if dead
	if (cg.predictedPlayerState.stats[STAT_HEALTH] <= 0)
	{
		return;
	}

	color = CG_FadeColor(cg.weaponSelectTime, WEAPON_SELECT_TIME);
	if (!color)
	{
		return;
	}
	trap_R_SetColor(color);

	// showing weapon select clears pickup item display, but not the blend blob
	cg.itemPickupTime = 0;

	// count the number of weapons owned
	bits = cg.snap->ps.stats[ STAT_WEAPONS ];
	count = 0;
	for (i = 1; i < 16; i++)
	{
		if (bits & (1 << i))
		{
			count++;
		}
	}

	x = 320 - count * 20;
	y = 380;

	for (i = 1; i < 16; i++)
	{
		if (!(bits & (1 << i)))
		{
			continue;
		}

		CG_RegisterWeapon(i);

		// draw weapon icon
		CG_DrawPic(x, y, 32, 32, cg_weapons[i].weaponIcon);

		// draw selection marker
		if (i == cg.weaponSelect)
		{
			CG_DrawPic(x-4, y-4, 40, 40, cgs.media.selectShader);
		}

		// no ammo cross on top
		if (!cg.snap->ps.ammo[ i ])
		{
			CG_DrawPic(x, y, 32, 32, cgs.media.noammoShader);
		}

		x += 40;
	}

	// draw the selected name
	ci = &cgs.clientinfo[cg.predictedPlayerState.clientNum];

	if (cg.weaponSelect == WP_GAUNTLET
		&& ci->team == cgs.InvasionInfo.AlienTeam
		&& (ci->Class & e_Class_AlienRaceMask) == e_Selection_Xenomorph)
	{
		name = "Claws";
	}
	else if (cg_weapons[ cg.weaponSelect ].item)
		name = cg_weapons[ cg.weaponSelect ].item->pickup_name;

	if (name)
	{
		w = CG_DrawStrlen(name) * BIGCHAR_WIDTH;
		x = (SCREEN_WIDTH - w) / 2;
		CG_DrawBigStringColor(x, y - 22, name, color);
	}

	trap_R_SetColor(NULL);
}


/*
===============
CG_WeaponSelectable
===============
*/
static qboolean CG_WeaponSelectable(int i)
{
	if (!cg.snap->ps.ammo[i])
	{
		return qfalse;
	}
	if (! (cg.snap->ps.stats[ STAT_WEAPONS ] & (1 << i)))
	{
		return qfalse;
	}

	return qtrue;
}

/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f(void)
{
	int		i;
	int		original;

	if (!cg.snap)
	{
		return;
	}
	if (cg.snap->ps.pm_flags & PMF_FOLLOW)
	{
		++cgs.Inv_FollowMode;
		if (cgs.Inv_FollowMode > 2)
			cgs.Inv_FollowMode = 0;
		return;
	}
	else if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
		trap_SendClientCommand("follownext\n");

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for (i = 0; i < 16; i++)
	{
		cg.weaponSelect++;
		if (cg.weaponSelect == 16)
		{
			cg.weaponSelect = 0;
		}
		if (cg.weaponSelect == WP_GAUNTLET)
		{
			//continue;		//Too: yup, use it ;) // never cycle to gauntlet
		}
		if (CG_WeaponSelectable(cg.weaponSelect))
		{
			break;
		}
	}
	if (i == 16)
	{
		cg.weaponSelect = original;
	}
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f(void)
{
	int		i;
	int		original;

	if (!cg.snap)
	{
		return;
	}
	if (cg.snap->ps.pm_flags & PMF_FOLLOW)
	{
		--cgs.Inv_FollowMode;
		if (cgs.Inv_FollowMode < 0)
			cgs.Inv_FollowMode = 2;
		return;
	}
	else if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
		trap_SendClientCommand("followprev");

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	for (i = 0; i < 16; i++)
	{
		cg.weaponSelect--;
		if (cg.weaponSelect == -1)
		{
			cg.weaponSelect = 15;
		}
		if (cg.weaponSelect == WP_GAUNTLET)
		{
			//continue;		//Too: use it		// never cycle to gauntlet
		}
		if (CG_WeaponSelectable(cg.weaponSelect))
		{
			break;
		}
	}
	if (i == 16)
	{
		cg.weaponSelect = original;
	}
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f(void)
{
	int		num;

	if (!cg.snap)
	{
		return;
	}
	if (cg.snap->ps.pm_flags & PMF_FOLLOW)
	{
		return;
	}

	num = atoi(CG_Argv(1));

	if (num < 1 || num > 15)
	{
		return;
	}

	cg.weaponSelectTime = cg.time;

	if (! (cg.snap->ps.stats[STAT_WEAPONS] & (1 << num)))
	{
		return;		// don't have the weapon
	}

	cg.weaponSelect = num;
}

/*
===================
CG_OutOfAmmoChange

The current weapon has just run out of ammo
===================
*/
void CG_OutOfAmmoChange(void)
{
	int		i;

	cg.weaponSelectTime = cg.time;

	for (i = 15; i > 0; i--)
	{
		if (CG_WeaponSelectable(i))
		{
			cg.weaponSelect = i;
			break;
		}
	}
}



/*
===================================================================================================

WEAPON EVENTS

===================================================================================================
*/

/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon(centity_t *cent, int Parm)
{
	entityState_t *ent;
	int				c;
	weaponInfo_t	*weap;

	ent = &cent->currentState;

	if (ent->weapon == WP_MACHINEGUN && (Parm & WP_FIRE2BIT))
	{
		trap_S_StartSound(NULL, ent->number, CHAN_WEAPON, cgs.media.sfx_LaunchGrenade);
		return;
	}

	if (ent->weapon == WP_NONE)
	{
		return;
	}
	if (ent->weapon >= WP_NUM_WEAPONS)
	{
		CG_Error("CG_FireWeapon: ent->weapon >= WP_NUM_WEAPONS");
		return;
	}
	weap = &cg_weapons[ ent->weapon ];

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	cent->muzzleFlashTime = cg.time;

	// lightning gun only does this this on initial press
	if (ent->weapon == WP_LIGHTNING)
	{
		if (cent->pe.lightningFiring)
		{
			return;
		}
	}

	// play quad sound if needed
	if (cent->currentState.powerups & (1 << PW_QUAD))
	{
		trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.media.quadSound);
	}

	if ((Parm & WP_FIRE2BIT) && ent->weapon == WP_PLASMAGUN)
	{
		trap_S_StartSound(NULL, ent->number, CHAN_WEAPON, cgs.media.sfx_PlasmaFireSpec);
	}
	else
	{
		// play a sound
		for (c = 0; c < 4; c++)
		{
			if (!weap->flashSound[c])
			{
				break;
			}
		}
		if (c > 0)
		{
			c = rand() % c;
			if (weap->flashSound[c])
			{
				trap_S_StartSound(NULL, ent->number, CHAN_WEAPON, weap->flashSound[c]);
			}
		}
	}

	// do brass ejection
	if (weap->ejectBrassFunc && cg_brassTime.integer > 0)
	{
		weap->ejectBrassFunc(cent);
	}
}


/*
=================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CG_MissileHitWall(int weapon, int clientNum, vec3_t AbsOrigin, vec3_t dir, impactSound_t soundType)
{
	qhandle_t		mod;
	qhandle_t		mark;
	qhandle_t		shader;
	sfxHandle_t		sfx;
	float			radius;
	float			light;
	vec3_t			lightColor;
	localEntity_t	*le;
	int				r;
	qboolean		alphaFade;
	qboolean		isSprite;
	int				duration;
	vec3_t			origin;
	vec3_t			sprOrg;
	vec3_t			sprVel;

	VectorCopy(AbsOrigin, origin);

	mark = 0;
	radius = 32;
	sfx = 0;
	mod = 0;
	shader = 0;
	light = 0;
	lightColor[0] = 1;
	lightColor[1] = 1;
	lightColor[2] = 0;

	// set defaults
	isSprite = qfalse;
	duration = 600;

	switch (weapon)
	{
	default:
/*#ifdef MISSIONPACK
	case WP_NAILGUN:
		if (soundType == IMPACTSOUND_FLESH)
		{
			sfx = cgs.media.sfx_nghitflesh;
		}
		else if (soundType == IMPACTSOUND_METAL)
		{
			sfx = cgs.media.sfx_nghitmetal;
		}
		else
		{
			sfx = cgs.media.sfx_nghit;
		}
		mark = cgs.media.holeMarkShader;
		radius = 12;
		break;
#endif*/
	case WP_LIGHTNING:
		// no explosion at LG impact, it is added with the beam
		r = rand() & 3;
		if (r < 2)
		{
			sfx = cgs.media.sfx_lghit2;
		}
		else if (r == 2)
		{
			sfx = cgs.media.sfx_lghit1;
		}
		else
		{
			sfx = cgs.media.sfx_lghit3;
		}
		mark = cgs.media.holeMarkShader;
		radius = 12;
		break;
/*#ifdef MISSIONPACK
	case WP_PROX_LAUNCHER:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.grenadeExplosionShader;
		sfx = cgs.media.sfx_proxexp;
		mark = cgs.media.burnMarkShader;
		radius = 64;
		light = 300;
		isSprite = qtrue;
		break;
#endif*/
	case WP_BLOOMY_EXPLOSION:
	case WP_BLOOMY_EXPLOSION2:
	case WP_GRENADE_LAUNCHER:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.grenadeExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 64;
		light = 300;
		isSprite = qtrue;
		break;
	case WP_EGG_EXPLOSION:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.AcidExplosion;
		sfx = cgs.media.sfx_rockexp;
		origin[2] += 10;
		radius = 32;
		light = 300;
		isSprite = qtrue;
		lightColor[0] = 0.5f;
		lightColor[1] = 1.0f;
		lightColor[2] = 0.0f;
		break;
	case WP_ROCKET_LAUNCHER:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.rocketExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 64;
		light = 300;
		isSprite = qtrue;
		duration = 1000;
		lightColor[0] = 1;
		lightColor[1] = 0.75;
		lightColor[2] = 0.0;
		if (cg_oldRocket.integer == 0)
		{
			// explosion sprite animation
			VectorMA(origin, 24, dir, sprOrg);
			VectorScale(dir, 64, sprVel);

			CG_ParticleExplosion("explode1", sprOrg, sprVel, 1400, 20, 30);
		}
		break;
	/*case WP_RAILGUN:
		mod = cgs.media.ringFlashModel;
		shader = cgs.media.railExplosionShader;
		sfx = cgs.media.sfx_plasmaexp;
		mark = cgs.media.energyMarkShader;
		radius = 24;
		break; */
	case WP_PLASMAGUN2:
	case WP_PLASMAGUN:
		mod = cgs.media.ringFlashModel;
		shader = cgs.media.plasmaExplosionShader;
		isSprite = qtrue;
		mark = cgs.media.energyMarkShader;
		duration = 300;

		if (weapon == WP_PLASMAGUN2)
		{
			sfx = cgs.media.sfx_PlasmaExplodeSpec;
			radius = 32;
		}
		else
		{
			sfx = cgs.media.sfx_plasmaexp;
			radius = 16;
		}

		break;

	case WP_FLAMETHROWER:
	{
		mod = cgs.media.ringFlashModel;
		shader = cgs.media.FlameExplosionShader;
		sfx = cgs.media.sfx_FlameExp;
		mark = cgs.media.burnMarkShader;
		radius = es_FlameMulRadius - 16;// + es_FlameBaseRadius;
		break;
	}
	case WP_BFG:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.bfgExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 32;
		isSprite = qtrue;
		break;
	case WP_SHOTGUN:
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;
		sfx = 0;
		radius = 4;
		break;

/*#ifdef MISSIONPACK
	case WP_CHAINGUN:
		mod = cgs.media.bulletFlashModel;
		if (soundType == IMPACTSOUND_FLESH)
		{
			sfx = cgs.media.sfx_chghitflesh;
		}
		else if (soundType == IMPACTSOUND_METAL)
		{
			sfx = cgs.media.sfx_chghitmetal;
		}
		else
		{
			sfx = cgs.media.sfx_chghit;
		}
		mark = cgs.media.bulletMarkShader;

		r = rand() & 3;
		if (r < 2)
		{
			sfx = cgs.media.sfx_ric1;
		}
		else if (r == 2)
		{
			sfx = cgs.media.sfx_ric2;
		}
		else
		{
			sfx = cgs.media.sfx_ric3;
		}

		radius = 8;
		break;
#endif*/

	case WP_GAUNTLET:
		if (soundType != IMPACTSOUND_FLESH)
		{
			mark = cgs.media.bulletMarkShader;
			radius = 8;
		}

		sfx = cgs.media.sfx_GauntletHit;
		break;

	case WP_RAILGUN:
	case WP_INVCHAINGUN:
	case WP_MACHINEGUN:
	case WP_ALIEN_GUN:
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;

		r = rand() & 3;
		if (r == 0)
		{
			sfx = cgs.media.sfx_ric1;
		}
		else if (r == 1)
		{
			sfx = cgs.media.sfx_ric2;
		}
		else
		{
			sfx = cgs.media.sfx_ric3;
		}

		radius = 8;
		break;
	}

	if (sfx)
	{
		trap_S_StartSound(origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx);
	}

	//
	// create the explosion
	//
	if (mod)
	{
		le = CG_MakeExplosion(origin, dir,
							   mod,	shader,
							   duration, isSprite);

		if (weapon == WP_BLOOMY_EXPLOSION)
			le->radius *= 2;
		else if (weapon == WP_BLOOMY_EXPLOSION2)
			le->radius *= 3;
		else if (weapon == WP_PLASMAGUN || weapon == WP_PLASMAGUN2 || weapon == WP_EGG_EXPLOSION)
		{
			if (weapon != WP_EGG_EXPLOSION)
			{
				if (weapon == WP_PLASMAGUN2)
					le->radius = 30;
				else
					le->radius = 15;

				VectorMA(le->refEntity.origin, (radius * 0.25f - 16), dir, le->refEntity.origin);
				VectorCopy(le->refEntity.origin, le->refEntity.oldorigin);
			}
			else
				le->radius = 25;

			VectorCopy(cg_weapons[WP_PLASMAGUN].flashDlightColor, le->color);
			le->color[3] = 2.0f;
		}

		le->light = light;
		VectorCopy(lightColor, le->lightColor);
		/*if (weapon == WP_RAILGUN)		//Too:
		{
			// colorize with client color
			VectorCopy(cgs.clientinfo[clientNum].color, le->color);
		}*/
	}

	//
	// impact mark
	//
	alphaFade = (mark == cgs.media.energyMarkShader);	// plasma fades alpha, all others fade color
	/*if (weapon == WP_RAILGUN)		//Too:
	{
		float	*color;

		// colorize with client color
		color = cgs.clientinfo[clientNum].color2;
		CG_ImpactMark(mark, origin, dir, random()*360, color[0],color[1], color[2],1, alphaFade, radius, qfalse);
	}
	else */
	if (mark)
	{
		CG_ImpactMark(mark, origin, dir, random()*360, colorWhite, alphaFade, radius, qfalse);
	}
}


/*
=================
CG_MissileHitPlayer
=================
*/
void CG_MissileHitPlayer(int weapon, vec3_t origin, vec3_t dir, int entityNum)
{
	CG_Bleed(origin, entityNum);

	// some weapons will make an explosion with the blood, while
	// others will just make the blood
	switch (weapon)
	{
		case WP_GAUNTLET:
		case WP_GRENADE_LAUNCHER:
		case WP_ROCKET_LAUNCHER:
/*#ifdef MISSIONPACK
		case WP_NAILGUN:
		case WP_CHAINGUN:
		case WP_PROX_LAUNCHER:
#endif*/
		CG_MissileHitWall(weapon, 0, origin, dir, IMPACTSOUND_FLESH);
		break;
	default:
		break;
	}
}



/*
============================================================================

SHOTGUN TRACING

============================================================================
*/

/*
================
CG_ShotgunPellet
================
*/
static void CG_ShotgunPellet(vec3_t start, vec3_t end, int skipNum)
{
	trace_t		tr;
	int sourceContentType, destContentType;

	CG_Trace(&tr, start, NULL, NULL, end, skipNum, MASK_SHOT);

	sourceContentType = trap_CM_PointContents(start, 0);
	destContentType = trap_CM_PointContents(tr.endpos, 0);

	// FIXME: should probably move this cruft into CG_BubbleTrail
	if (sourceContentType == destContentType)
	{
		if (sourceContentType & CONTENTS_WATER)
		{
			CG_BubbleTrail(start, tr.endpos, 32);
		}
	}
	else if (sourceContentType & CONTENTS_WATER)
	{
		trace_t trace;

		trap_CM_BoxTrace(&trace, end, start, NULL, NULL, 0, CONTENTS_WATER);
		CG_BubbleTrail(start, trace.endpos, 32);
	}
	else if (destContentType & CONTENTS_WATER)
	{
		trace_t trace;

		trap_CM_BoxTrace(&trace, start, end, NULL, NULL, 0, CONTENTS_WATER);
		CG_BubbleTrail(tr.endpos, trace.endpos, 32);
	}

	if ( tr.surfaceFlags & SURF_NOIMPACT)
	{
		return;
	}

	if (cg_entities[tr.entityNum].currentState.eType == ET_PLAYER)
	{
		CG_MissileHitPlayer(WP_SHOTGUN, tr.endpos, tr.plane.normal, tr.entityNum);
	}
	else
	{
		if (tr.surfaceFlags & SURF_NOIMPACT)
		{
			// SURF_NOIMPACT will not make a flame puff or a mark
			return;
		}
		if (tr.surfaceFlags & SURF_METALSTEPS)
		{
			CG_MissileHitWall(WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_METAL);
		}
		else
		{
			CG_MissileHitWall(WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_DEFAULT);
		}
	}
}

/*
================
CG_ShotgunPattern

Perform the same traces the server did to locate the
hit splashes
================
*/
static void CG_ShotgunPattern(vec3_t origin, vec3_t origin2, int seed, int otherEntNum, qboolean Heavy)
{
	int			i;
	float		r, u;
	vec3_t		end;
	vec3_t		forward, right, up;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2(origin2, forward);
	PerpendicularVector(right, forward);
	CrossProduct(forward, right, up);

	// generate the "random" spread pattern
	for (i = 0; i < DEFAULT_SHOTGUN_COUNT; i++)
	{
		if (Heavy == qtrue)
		{
			r = Q_crandom(&seed) * HEAVY_SHOTGUN_SPREAD;
			u = Q_crandom(&seed) * HEAVY_SHOTGUN_SPREAD;
		}
		else
		{
			r = Q_crandom(&seed) * DEFAULT_SHOTGUN_SPREAD;
			u = Q_crandom(&seed) * DEFAULT_SHOTGUN_SPREAD;
		}

		r *= 16;
		u *= 16;
		VectorMA(origin, 8192 * 16, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		CG_ShotgunPellet(origin, end, otherEntNum);
	}
}

/*
==============
CG_ShotgunFire
==============
*/
void CG_ShotgunFire(entityState_t *es, qboolean Heavy)
{
	vec3_t	v;
	int		contents;

	VectorSubtract(es->origin2, es->pos.trBase, v);
	VectorNormalize(v);
	VectorScale(v, 32, v);
	VectorAdd(es->pos.trBase, v, v);
	if (cgs.glconfig.hardwareType != GLHW_RAGEPRO)
	{
		// ragepro can't alpha fade, so don't even bother with smoke
		vec3_t			up;

		contents = trap_CM_PointContents(es->pos.trBase, 0);
		if (!(contents & CONTENTS_WATER))
		{
			VectorSet(up, 0, 0, 8);
			CG_SmokePuff(v, up, 32, 1, 1, 1, 0.33f, 900, cg.time, 0, LEF_PUFF_DONT_SCALE, cgs.media.shotgunSmokePuffShader);
		}
	}
	CG_ShotgunPattern(es->pos.trBase, es->origin2, es->eventParm, es->otherEntityNum, Heavy);
}

/*
============================================================================

BULLETS

============================================================================
*/


/*
===============
CG_Tracer
===============
*/
/*void CG_Tracer(vec3_t source, vec3_t dest)
{
	vec3_t		forward, right;
	polyVert_t	verts[4];
	vec3_t		line;
	float		len, begin, end;
	vec3_t		start, finish;
	vec3_t		midpoint;

	// tracer
	VectorSubtract(dest, source, forward);
	len = VectorNormalize(forward);

	// start at least a little ways from the muzzle
	if (len < 100)
	{
		return;
	}
	begin = 50 + random() * (len - 60);
	end = begin + cg_tracerLength.value;
	if (end > len)
	{
		end = len;
	}
	VectorMA(source, begin, forward, start);
	VectorMA(source, end, forward, finish);

	line[0] = DotProduct(forward, cg.refdef.viewaxis[1]);
	line[1] = DotProduct(forward, cg.refdef.viewaxis[2]);

	VectorScale(cg.refdef.viewaxis[1], line[1], right);
	VectorMA(right, -line[0], cg.refdef.viewaxis[2], right);
	VectorNormalize(right);

	VectorMA(finish, cg_tracerWidth.value, right, verts[0].xyz);
	verts[0].st[0] = 0;
	verts[0].st[1] = 1;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorMA(finish, -cg_tracerWidth.value, right, verts[1].xyz);
	verts[1].st[0] = 1;
	verts[1].st[1] = 0;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorMA(start, -cg_tracerWidth.value, right, verts[2].xyz);
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorMA(start, cg_tracerWidth.value, right, verts[3].xyz);
	verts[3].st[0] = 0;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene(cgs.media.tracerShader, 4, verts);

	midpoint[0] = (start[0] + finish[0]) * 0.5;
	midpoint[1] = (start[1] + finish[1]) * 0.5;
	midpoint[2] = (start[2] + finish[2]) * 0.5;

	// add the tracer sound
	trap_S_StartSound(midpoint, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tracerSound);

}*/


/*
====================
Too: Inv_CreateTracer
====================
*/
localEntity_t *Inv_CreateTracer(vec3_t source, vec3_t dest)
{
	localEntity_t	*le;
	vec3_t midpoint;
	int i;

	le = CG_AllocLocalEntity();
	le->leType = LE_TRACER;

	le->startTime = cg.time;
	le->endTime = le->startTime + 500;

	// bias the time so all shader effects start correctly
	le->refEntity.shaderTime = le->startTime / 1000.0f;

	// set origin
	VectorCopy(source, le->refEntity.origin);
	VectorCopy(dest, le->refEntity.oldorigin);

	le->color[0] = le->color[1] = le->color[2] = 1.0f;

	for (i = 0; i < 3; ++i)
		midpoint[i] = (source[i] + dest[i]) * 0.5f;

	// add the tracer sound
	trap_S_StartSound(midpoint, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tracerSound);

	return le;
};


/*
======================
CG_CalcMuzzlePoint
======================
*/
static qboolean	CG_CalcMuzzlePoint(int entityNum, vec3_t muzzle)
{
	vec3_t		forward;
	centity_t	*cent;
	int			anim;

	if (entityNum == cg.snap->ps.clientNum)
	{
		VectorCopy(cg.snap->ps.origin, muzzle);
		muzzle[2] += cg.snap->ps.viewheight;
		AngleVectors(cg.snap->ps.viewangles, forward, NULL, NULL);
		VectorMA(muzzle, 14, forward, muzzle);
		return qtrue;
	}

	cent = &cg_entities[entityNum];
	if (!cent->currentValid)
	{
		return qfalse;
	}

	VectorCopy(cent->currentState.pos.trBase, muzzle);

	AngleVectors(cent->currentState.apos.trBase, forward, NULL, NULL);
	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if (anim == LEGS_WALKCR || anim == LEGS_IDLECR || anim == LEGS_BACKCR)
	{
		muzzle[2] += CROUCH_VIEWHEIGHT;
	}
	else
	{
		muzzle[2] += DEFAULT_VIEWHEIGHT;
	}

	VectorMA(muzzle, 14, forward, muzzle);

	return qtrue;

}


/*
======================
Too: Inv_CalcMuzzlePoint
======================
*/
static qboolean Inv_CalcMuzzlePoint(int entityNum, vec3_t muzzle)
{
	centity_t	*cent;

	if (entityNum == cg.snap->ps.clientNum)
	{
		VectorCopy(cg.predictedPlayerEntity.pe.Flash, muzzle);
		return qtrue;
	}

	cent = &cg_entities[entityNum];
	if (!cent->currentValid)
		return qfalse;

	VectorCopy(cent->pe.Flash, muzzle);
	return qtrue;
}

/*
======================
CG_Bullet

Renders bullet effects.
======================
*/
void CG_Bullet(vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum, qboolean Tracer)
{
	trace_t trace;
	int sourceContentType, destContentType;
	vec3_t		start;

	// if the shooter is currently valid, calc a source point and possibly
	// do trail effects
	if (sourceEntityNum >= 0 && cg_tracerChance.value > 0)
	{
		if (CG_CalcMuzzlePoint(sourceEntityNum, start))
		{
			sourceContentType = trap_CM_PointContents(start, 0);
			destContentType = trap_CM_PointContents(end, 0);

			// do a complete bubble trail if necessary
			if ((sourceContentType == destContentType) && (sourceContentType & CONTENTS_WATER))
			{
				CG_BubbleTrail(start, end, 32);
			}
			// bubble trail from water into air
			else if ((sourceContentType & CONTENTS_WATER))
			{
				trap_CM_BoxTrace(&trace, end, start, NULL, NULL, 0, CONTENTS_WATER);
				CG_BubbleTrail(start, trace.endpos, 32);
			}
			// bubble trail from air into water
			else if ((destContentType & CONTENTS_WATER))
			{
				trap_CM_BoxTrace(&trace, start, end, NULL, NULL, 0, CONTENTS_WATER);
				CG_BubbleTrail(trace.endpos, end, 32);
			}

			// draw a tracer
			//if (random() < cg_tracerChance.value)
				//CG_Tracer(start, end);
			if (Tracer)
			{
				Inv_CalcMuzzlePoint(sourceEntityNum, start);
				Inv_CreateTracer(start, end);
			}
		}
	}

	// impact splash and mark
	if (flesh)
	{
		CG_Bleed(end, fleshEntityNum);
	}
	else
	{
		CG_MissileHitWall(WP_MACHINEGUN, 0, end, normal, IMPACTSOUND_DEFAULT);
	}

}

//Too :
void CG_BackWeaponInit(centity_t *ent, int Time)
{
	ent->pe.GunBackTime = cg.time;
	ent->pe.GunBackTotalTime = Time;
}

/*==================== EOF because of buggy VSS ===========*/