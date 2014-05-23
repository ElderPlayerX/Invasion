// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_effects.c -- these functions generate localentities, usually as a result
// of event processing

#include "cg_local.h"


/*
==================
CG_BubbleTrail

Bullets shot underwater
==================
*/
void CG_BubbleTrail(vec3_t start, vec3_t end, float spacing)
{
	vec3_t		move;
	vec3_t		vec;
	float		len;
	int			i;

	if (cg_noProjectileTrail.integer)
	{
		return;
	}

	VectorCopy (start, move);
	VectorSubtract (end, start, vec);
	len = VectorNormalize (vec);

	// advance a random amount first
	i = rand() % (int)spacing;
	VectorMA(move, i, vec, move);

	VectorScale (vec, spacing, vec);

	for (; i < len; i += spacing)
	{
		localEntity_t	*le;
		refEntity_t		*re;

		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		le->leType = LE_MOVE_SCALE_FADE;
		le->startTime = cg.time;
		le->endTime = cg.time + 1000 + random() * 250;
		le->lifeRate = 1.0 / (le->endTime - le->startTime);

		re = &le->refEntity;
		re->shaderTime = cg.time / 1000.0f;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->radius = 3;
		re->customShader = cgs.media.waterBubbleShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;

		le->color[3] = 1.0;

		le->pos.trType = TR_LINEAR;
		le->pos.trTime = cg.time;
		VectorCopy(move, le->pos.trBase);
		le->pos.trDelta[0] = crandom()*5;
		le->pos.trDelta[1] = crandom()*5;
		le->pos.trDelta[2] = crandom()*5 + 6;

		VectorAdd (move, vec, move);
	}
}

/*
=====================
CG_SmokePuff

Adds a smoke puff or blood trail localEntity.
=====================
*/
localEntity_t *CG_SmokePuff(const vec3_t p, const vec3_t vel,
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,
				   qhandle_t hShader)
{
	static int	seed = 0x92;
	localEntity_t	*le;
	refEntity_t		*re;
//	int fadeInTime = startTime + duration / 2;

	le = CG_AllocLocalEntity();
	le->leFlags = leFlags;
	le->radius = radius;

	re = &le->refEntity;
	re->rotation = Q_random(&seed) * 360;
	re->radius = radius;
	re->shaderTime = startTime / 1000.0f;

	le->leType = LE_MOVE_SCALE_FADE;
	le->startTime = startTime;
	le->fadeInTime = fadeInTime;
	le->endTime = startTime + duration;
	if (fadeInTime > startTime)
	{
		le->lifeRate = 1.0 / (le->endTime - le->fadeInTime);
	}
	else
	{
		le->lifeRate = 1.0 / (le->endTime - le->startTime);
	}
	le->color[0] = r;
	le->color[1] = g;
	le->color[2] = b;
	le->color[3] = a;


	le->pos.trType = TR_LINEAR;
	le->pos.trTime = startTime;
	VectorCopy(vel, le->pos.trDelta);
	VectorCopy(p, le->pos.trBase);

	VectorCopy(p, re->origin);
	re->customShader = hShader;

	// rage pro can't alpha fade, so use a different shader
	if (cgs.glconfig.hardwareType == GLHW_RAGEPRO)
	{
		re->customShader = cgs.media.smokePuffRageProShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;
	}
	else
	{
		re->shaderRGBA[0] = le->color[0] * 0xff;
		re->shaderRGBA[1] = le->color[1] * 0xff;
		re->shaderRGBA[2] = le->color[2] * 0xff;
		re->shaderRGBA[3] = 0xff;
	}

	re->reType = RT_SPRITE;
	re->radius = le->radius;

	return le;
}

/*
==================
CG_SpawnEffect

Player teleporting in or out
==================
*/
void CG_SpawnEffect(vec3_t org)
{
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_FADE_RGB;
	le->startTime = cg.time;
	le->endTime = cg.time + 500;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

#ifndef MISSIONPACK
	re->customShader = cgs.media.teleportEffectShader;
#endif
	re->hModel = cgs.media.teleportEffectModel;
	AxisClear(re->axis);

	VectorCopy(org, re->origin);
#ifdef MISSIONPACK
	re->origin[2] += 16;
#else
	re->origin[2] -= 24;
#endif
}


/*
====================
CG_MakeExplosion
====================
*/
localEntity_t *CG_MakeExplosion(vec3_t origin, vec3_t dir,
								qhandle_t hModel, qhandle_t shader,
								int msec, qboolean isSprite)
{
	float			ang;
	localEntity_t	*ex;
	int				offset;
	vec3_t			tmpVec, newOrigin;

	if (msec <= 0)
	{
		CG_Error("CG_MakeExplosion: msec = %i", msec);
	}

	// skew the time a bit so they aren't all in sync
	offset = rand() & 63;

	ex = CG_AllocLocalEntity();
	if (isSprite)
	{
		ex->leType = LE_SPRITE_EXPLOSION;

		ex->radius = 42;

		// randomly rotate sprite orientation
		ex->refEntity.rotation = rand() % 360;
		VectorScale(dir, 16, tmpVec);
		VectorAdd(tmpVec, origin, newOrigin);
	}
	else
	{
		ex->leType = LE_EXPLOSION;
		VectorCopy(origin, newOrigin);

		// set axis with random rotate
		if (!dir)
		{
			AxisClear(ex->refEntity.axis);
		}
		else
		{
			ang = rand() % 360;
			VectorCopy(dir, ex->refEntity.axis[0]);
			RotateAroundDirection(ex->refEntity.axis, ang);
		}
	}

	ex->startTime = cg.time - offset;
	ex->endTime = ex->startTime + msec;

	// bias the time so all shader effects start correctly
	ex->refEntity.shaderTime = ex->startTime / 1000.0f;

	ex->refEntity.hModel = hModel;
	ex->refEntity.customShader = shader;

	// set origin
	VectorCopy(newOrigin, ex->refEntity.origin);
	VectorCopy(newOrigin, ex->refEntity.oldorigin);

	ex->color[0] = ex->color[1] = ex->color[2] = 1.0;
	ex->color[3] = 0.33f;

	return ex;
}

/*
=================
Too: GetBloodClor
=================
*/

int Inv_GetAlienBloodColor(int entityNum, vec4_t color)
{
	int Race;
	clientInfo_t *ci;

	if (entityNum < 0)
		return -1;

	if (entityNum >= MAX_CLIENTS)
	{
		centity_t *cent = cg_entities + entityNum;

		if (!(cent->currentState.eFlags & EF_DEAD))
			return -1;

		ci = Inv_GetInfoForDead(cent->currentState.generic1);

		if (ci == NULL)
			return -1;
	}
	else
		ci = &cgs.clientinfo[entityNum];

	if (!ci->infoValid || ci->team != cgs.InvasionInfo.AlienTeam)
		return -1;

	Race = ci->Class & e_Class_AlienRaceMask;

	color[3] = 1.0f;

	switch (Race)
	{
		default:
		case e_Selection_Xenomorph:
			color[0] = 0.1f;
			color[1] = 0.6f;
			color[2] = 0.1f;
			break;

		case e_Selection_Trooper:
			color[0] = 0.2f;
			color[1] = 0.0f;
			color[2] = 0.05f;
			break;

		case e_Selection_Bloomy:
			color[0] = 0.8f;
			color[1] = 0.6f;
			color[2] = 0.2f;
			break;

		case e_Selection_Rad:
			color[0] = 0.2f;
			color[1] = 1.0f;
			color[2] = 0.5f;
			break;
	}

	return Race;
}


/*
=================
CG_Bleed

This is the spurt of blood when a character gets hit
=================
*/
void CG_Bleed(vec3_t origin, int entityNum)
{
	localEntity_t	*ex;
	vec4_t color;

	if (!cg_blood.integer)
	{
		return;
	}

	ex = CG_AllocLocalEntity();
	ex->leType = LE_EXPLOSION;

	ex->startTime = cg.time;
	ex->endTime = ex->startTime + 500;

	VectorCopy(origin, ex->refEntity.origin);
	ex->refEntity.reType = RT_SPRITE;
	ex->refEntity.rotation = rand() % 360;
	ex->refEntity.radius = 24;

	// don't show player's own blood in view
	if (entityNum == cg.snap->ps.clientNum)
	{
		ex->refEntity.renderfx |= RF_THIRD_PERSON;
	}

	if (Inv_GetAlienBloodColor(entityNum, color) != -1)
	{
		int i;

		ex->refEntity.customShader = cgs.media.AcidExplosion;

		for (i = 0; i < 4; ++i)
			ex->refEntity.shaderRGBA[i] = color[i] * 255.9f;
	}
	else
		ex->refEntity.customShader = cgs.media.bloodExplosionShader;
}



/*
==================
CG_LaunchGib
==================
*/
int CG_LaunchGib(vec3_t origin, vec3_t velocity, qhandle_t hModel, int entityNum, int Phase)
{
	static vec4_t StaticColor;
	static MarkType;
	localEntity_t	*le;
	refEntity_t		*re;
	int n;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + 5000 + random() * 3000;

	VectorCopy(origin, re->origin);
	AxisCopy(axisDefault, re->axis);
	re->hModel = hModel;

	le->pos.trType = TR_GRAVITY;
	VectorCopy(origin, le->pos.trBase);
	VectorCopy(velocity, le->pos.trDelta);
	le->pos.trTime = cg.time;

	le->bounceFactor = 0.6f;

	le->leBounceSoundType = LEBS_BLOOD;
	if (entityNum == -2)
	{
		le->color[0] = (rand()&7) * 0.8f / 7.0f;
		le->color[1] = 0.6f;
		le->color[2] = 0.1f;
		le->color[3] = 1;

		le->leMarkType = LEMT_ACID;

		return 0;
	}

	if (Phase == 0)
	{
		if ((n = Inv_GetAlienBloodColor(entityNum, StaticColor)) != -1)
		{
			Vector4Copy(StaticColor, le->color);

			if (n == e_Selection_Rad)
				le->leMarkType = LEMT_RADBLOOD;
			else
				le->leMarkType = LEMT_ACID;

			MarkType = le->leMarkType;

			return 1;
		}
	}
	else if (Phase == 1)
	{
		Vector4Copy(StaticColor, le->color);
		le->leMarkType = MarkType;
		return 1;
	}

	le->leMarkType = LEMT_BLOOD;

	return 2;
}

/*
===================
CG_GibPlayer

Generated a bunch of gibs launching out from the bodies location
===================
*/
#define	GIB_VELOCITY	250
#define	GIB_JUMP		250
void CG_GibPlayer(vec3_t playerOrigin, int entityNum)
{
	vec3_t origin, velocity;
	int Phase;

	if (!cg_blood.integer)
	{
		return;
	}

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	if (rand() & 1)
	{
		Phase = CG_LaunchGib(origin, velocity, cgs.media.gibSkull, entityNum, 0);
	}
	else
	{
		Phase = CG_LaunchGib(origin, velocity, cgs.media.gibBrain, entityNum, 0);
	}

	// allow gibs to be turned off for speed
	if (!cg_gibs.integer)
	{
		return;
	}

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibAbdomen, entityNum, Phase);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibArm, entityNum, Phase);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibChest, entityNum, Phase);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibFist, entityNum, Phase);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibFoot, entityNum, Phase);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibForearm, entityNum, Phase);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibIntestine, entityNum, Phase);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibLeg, entityNum, Phase);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibLeg, entityNum, Phase);
}


void CG_GibEgg(vec3_t playerOrigin)
{
	vec3_t origin, velocity;

	if (!cg_blood.integer)
	{
		return;
	}

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibBrain, -2, 0);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibAbdomen, -2, 0);

	// allow gibs to be turned off for speed
	if (!cg_gibs.integer)
	{
		return;
	}

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibAbdomen, -2, 0);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibIntestine, -2, 0);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*GIB_VELOCITY;
	velocity[1] = crandom()*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + crandom()*GIB_VELOCITY;
	CG_LaunchGib(origin, velocity, cgs.media.gibIntestine, -2, 0);
}

/*
==================
CG_LaunchGib
==================
*/
void CG_LaunchExplode(vec3_t origin, vec3_t velocity, qhandle_t hModel)
{
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + 10000 + random() * 6000;

	VectorCopy(origin, re->origin);
	AxisCopy(axisDefault, re->axis);
	re->hModel = hModel;

	le->pos.trType = TR_GRAVITY;
	VectorCopy(origin, le->pos.trBase);
	VectorCopy(velocity, le->pos.trDelta);
	le->pos.trTime = cg.time;

	le->bounceFactor = 0.1f;

	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}

#define	EXP_VELOCITY	100
#define	EXP_JUMP		150
/*
===================
CG_GibPlayer

Generated a bunch of gibs launching out from the bodies location
===================
*/
/*void CG_BigExplode(vec3_t playerOrigin)			//Too:
{
	vec3_t	origin, velocity;

	if (!cg_blood.integer)
	{
		return;
	}

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*EXP_VELOCITY;
	velocity[1] = crandom()*EXP_VELOCITY;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode(origin, velocity, cgs.media.smoke2);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*EXP_VELOCITY;
	velocity[1] = crandom()*EXP_VELOCITY;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode(origin, velocity, cgs.media.smoke2);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*EXP_VELOCITY*1.5;
	velocity[1] = crandom()*EXP_VELOCITY*1.5;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode(origin, velocity, cgs.media.smoke2);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*EXP_VELOCITY*2.0;
	velocity[1] = crandom()*EXP_VELOCITY*2.0;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode(origin, velocity, cgs.media.smoke2);

	VectorCopy(playerOrigin, origin);
	velocity[0] = crandom()*EXP_VELOCITY*2.5;
	velocity[1] = crandom()*EXP_VELOCITY*2.5;
	velocity[2] = EXP_JUMP + crandom()*EXP_VELOCITY;
	CG_LaunchExplode(origin, velocity, cgs.media.smoke2);
}*/


#ifdef MISSIONPACK
/*
===============
CG_LightningBoltBeam
===============
*/
void CG_LightningBoltBeam(vec3_t start, vec3_t end)
{
	localEntity_t	*le;
	refEntity_t		*beam;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_SHOWREFENTITY;
	le->startTime = cg.time;
	le->endTime = cg.time + 50;

	beam = &le->refEntity;

	VectorCopy(start, beam->origin);
	// this is the end point
	VectorCopy(end, beam->oldorigin);

	beam->reType = RT_LIGHTNING;
	beam->customShader = cgs.media.lightningShader;
}

/*
==================
CG_KamikazeEffect
==================
*/
void CG_KamikazeEffect(vec3_t org)
{
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_KAMIKAZE;
	le->startTime = cg.time;
	le->endTime = cg.time + 3000;//2250;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	VectorClear(le->angles.trBase);

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	re->hModel = cgs.media.kamikazeEffectModel;

	VectorCopy(org, re->origin);

}

/*
==================
CG_ObeliskExplode
==================
*/
void CG_ObeliskExplode(vec3_t org, int entityNum)
{
	localEntity_t	*le;
	vec3_t origin;

	// create an explosion
	VectorCopy(org, origin);
	origin[2] += 64;
	le = CG_MakeExplosion(origin, vec3_origin,
						   cgs.media.dishFlashModel,
						   cgs.media.rocketExplosionShader,
						   600, qtrue);
	le->light = 300;
	le->lightColor[0] = 1;
	le->lightColor[1] = 0.75;
	le->lightColor[2] = 0.0;
}

/*
==================
CG_ObeliskPain
==================
*/
void CG_ObeliskPain(vec3_t org)
{
	float r;
	sfxHandle_t sfx;

	// hit sound
	r = rand() & 3;
	if (r < 2)
	{
		sfx = cgs.media.obeliskHitSound1;
	}
	else if (r == 2)
	{
		sfx = cgs.media.obeliskHitSound2;
	}
	else
	{
		sfx = cgs.media.obeliskHitSound3;
	}
	trap_S_StartSound (org, ENTITYNUM_NONE, CHAN_BODY, sfx);
}


/*
==================
CG_InvulnerabilityImpact
==================
*/
void CG_InvulnerabilityImpact(vec3_t org, vec3_t angles)
{
	localEntity_t	*le;
	refEntity_t		*re;
	int				r;
	sfxHandle_t		sfx;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_INVULIMPACT;
	le->startTime = cg.time;
	le->endTime = cg.time + 1000;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	re->hModel = cgs.media.invulnerabilityImpactModel;

	VectorCopy(org, re->origin);
	AnglesToAxis(angles, re->axis);

	r = rand() & 3;
	if (r < 2)
	{
		sfx = cgs.media.invulnerabilityImpactSound1;
	}
	else if (r == 2)
	{
		sfx = cgs.media.invulnerabilityImpactSound2;
	}
	else
	{
		sfx = cgs.media.invulnerabilityImpactSound3;
	}
	trap_S_StartSound (org, ENTITYNUM_NONE, CHAN_BODY, sfx);
}

/*
==================
CG_InvulnerabilityJuiced
==================
*/
void CG_InvulnerabilityJuiced(vec3_t org)
{
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			angles;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_INVULJUICED;
	le->startTime = cg.time;
	le->endTime = cg.time + 10000;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	re = &le->refEntity;

	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	re->hModel = cgs.media.invulnerabilityJuicedModel;

	VectorCopy(org, re->origin);
	VectorClear(angles);
	AnglesToAxis(angles, re->axis);

	trap_S_StartSound (org, ENTITYNUM_NONE, CHAN_BODY, cgs.media.invulnerabilityJuicedSound);
}

#endif

/*
==================
CG_ScorePlum
==================
*/
/*void CG_ScorePlum(int client, vec3_t org, int score)
{
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			angles;
	static vec3_t lastPos;

	// only visualize for the client that scored
	if (client != cg.predictedPlayerState.clientNum || cg_scorePlum.integer == 0)
	{
		return;
	}

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_SCOREPLUM;
	le->startTime = cg.time;
	le->endTime = cg.time + 4000;
	le->lifeRate = 1.0 / (le->endTime - le->startTime);


	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;
	le->radius = score;

	VectorCopy(org, le->pos.trBase);
	if (org[2] >= lastPos[2] - 20 && org[2] <= lastPos[2] + 20)
	{
		le->pos.trBase[2] -= 20;
	}

	//CG_Printf("Plum origin %i %i %i -- %i\n", (int)org[0], (int)org[1], (int)org[2], (int)Distance(org, lastPos));
	VectorCopy(org, lastPos);


	re = &le->refEntity;

	re->reType = RT_SPRITE;
	re->radius = 16;

	VectorClear(angles);
	AnglesToAxis(angles, re->axis);
}*/


/*==================== EOF because of buggy VSS ===========*/