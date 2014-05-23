// Copyright (C) 1999-2000 Id Software, Inc.
//
// bg_pmove.c -- both games player movement code
// takes a playerstate and a usercmd as input and returns a modifed playerstate

#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

pmove_t		*pm;
InvasionInfo_t *InvI;
pml_t		pml;

// movement parameters
float	pm_stopspeed = 100.0f;
float	pm_duckScale = 0.25f;
float	pm_InvduckScale = 0.5f;
float pm_WallWalkScale = 0.85f;
float	pm_swimScale = 0.50f;
float	pm_wadeScale = 0.70f;
float pm_LadderScale = 0.50f;  // Set the max movement speed to HALF of normal

float	pm_accelerate = 10.0f;
float	pm_airaccelerate = 1.0f;
float	pm_wateraccelerate = 4.0f;
float	pm_flyaccelerate = 8.0f;
float pm_LadderAccelerate = 3000.0f;  // The acceleration to friction ratio is 1:1

float	pm_friction = 6.0f;
float	pm_waterfriction = 1.0f;
float	pm_flightfriction = 3.0f;
float	pm_spectatorfriction = 5.0f;
float pm_Ladderfriction = 3000.0f;  // Friction is high enough so you don't slip down

int		c_pmove = 0;

EInvasionClass AlienRace;
EInvasionClass MarineArmor;

int InvasionMarineJumpTime[e_Selection_MaxArmor] = { 1000, 1250, 1500, 1300 };
int InvasionAlienJumpTime[e_Selection_MaxRace] = { 700, 1200, 700, 1000 };
int InvasionAlienJumpAcc[e_Selection_MaxRace] = { 270, 270, 300, 350 };

	// Charger size
int Inv_MarineCharger[WP_NUM_WEAPONS] = { 0, 0, 50, 0, 0, 0, 0, 40, 0, 0, 100, 80, 0 };
	//Too: reload time in 100th of second
int Inv_ReloadTime[WP_NUM_WEAPONS] = { 0, 0, 100, 0, 0, 0, 0, 150, 0, 0, 175, 125, 0 };

static int JumpTime = 0;
qboolean WallWalk = qfalse;
vec3_t Gravity;

static qboolean Inv_ApplyNewGround(vec3_t Normal, qboolean AdvanceOnNewGround);

/*
===============
Too: Inv_GetVectorFromStat
===============
*/
void Inv_GetVectorFromStat(int Stat, vec3_t vec)
{
	int i;
	vec3_t Ang;

	Ang[YAW] = ((Stat >> 8) & 255) * 180 / 127;
	Ang[PITCH] = (Stat & 255) * 180 / 255;
	Ang[ROLL] = 0;

	AngleVectors(Ang, NULL, NULL, vec);

	for (i = 0; i < 3; ++i)
		vec[i] = -vec[i];

	VectorNormalize(vec);
}


/*
===============
Too: Inv_QuatNormalize
===============
*/
void Inv_QuatNormalize(vec4_t Quat)
{
	int i;
	float Scale = sqrt(Square(Quat[0]) + Square(Quat[1])
						+ Square(Quat[2]) + Square(Quat[3]));

	if (Scale > 0)
	{
		Scale = 1.0f / Scale;

		for (i = 0; i < 4; ++i)
			Quat[i] *= Scale;
	}
}

/*
===============
Too: Inv_GetQuatFromStat
===============
*/
void Inv_GetQuatFromStat(int Stat, vec4_t Quat, vec4_t QuatInt)
{
	char *s = (char *) &Stat;
	int i;

	if (!Stat)
	{
		Quat[0] = Quat[1] = Quat[2] = 0;
		Quat[3] = 1.0f;

		if (QuatInt)
		{
			QuatInt[0] = QuatInt[1] = QuatInt[2] = 0;
			QuatInt[3] = 127.0f;
		}
	}
	else
	{
		for (i = 0; i < 4; ++i)
		{
			Quat[i] = s[i] / 127.0f;
			if (QuatInt)
				QuatInt[i] = s[i];
		}
	}

	Inv_QuatNormalize(Quat);
}


/*
===============
Too: Quaternion
===============
*/

void Inv_MatrixToAngles(vec3_t Matrix[3], vec3_t Ang)
{
	float cp, f;
	int i;

	assert(Matrix[0][2] >= -1.0f && Matrix[0][2] <= 1);

	Ang[PITCH] = (M_PI * 0.5) - acos(-Matrix[0][2]);
	cp = cos(Ang[PITCH]);
	if (fabs(cp) >= 0.001f)
	{
		f = Matrix[0][0] / cp;
		if (f < -1.0f)
			f = -1.0f;
		else if (f > 1.0f)
			f = 1.0f;

		Ang[YAW] = acos(f);
		if (Matrix[0][1] / cp < 0)
			Ang[YAW] = -Ang[YAW];

		f = Matrix[2][2] / cp;
		if (f < -1.0f)
			f = -1.0f;
		else if (f > 1.0f)
			f = 1.0f;

		Ang[ROLL] = acos(f);
		if (-Matrix[1][2] / cp < 0)
			Ang[ROLL] = -Ang[ROLL];
	}
	else
	{
		Ang[ROLL] = 0;

		f = Matrix[1][1];
		if (f < -1.0f)
			f = -1.0f;
		else if (f > 1.0f)
			f = 1.0f;

		Ang[YAW] = acos(f);
		if (Matrix[1][0] < 0)
			Ang[YAW] = -Ang[YAW];
	}

	for (i = 0; i < 3; ++i)
	{
		Ang[i] *= 180 / M_PI;
		assert(fabs(Ang[i]) <= 400);
	}
}

void Inv_AxisToQuat(vec4_t Axis, vec4_t Quat)
{
	float Scale;
	int i;

	Scale = sin(Axis[3] * 0.5f);
	Quat[3] = cos(Axis[3] * 0.5f);
	if (Quat[3] < 0)
	{
		Scale = -Scale;
		Quat[3] = -Quat[3];
	}

	for (i = 0; i < 3; ++i)
		Quat[i] = Axis[i] * Scale;

	Inv_QuatNormalize(Quat);
}


float Inv_GetQuatMaxDelta(vec4_t Quat, vec4_t LastQuat)
{
	float MaxDelta = 0;
	int i;

	for (i = 0; i < 4; ++i)
	{
		float Delta = fabs(Quat[i] - LastQuat[i]);

		if (Delta > MaxDelta)
			MaxDelta = Delta;
	}

	return MaxDelta;
}


void Inv_GetInvertAxis(int n, vec4_t Axis)
{
	Axis[0] = 0;
	Axis[1] = 0;
	Axis[2] = 0;
	Axis[3] = M_PI;

	switch (n)
	{
		default:
		case 0:
			Axis[0] = -1.0f;
			break;

		case 1:
			Axis[0] = 1.0f;
			break;

		case 2:
			Axis[1] = -1.0f;
			break;

		case 3:
			Axis[1] = 1.0f;
			break;
	}
}


void Inv_MulQuat(vec4_t Quat1, vec4_t Quat2, vec4_t Dest)
{
    Dest[0] = Quat2[3] * Quat1[0] + Quat2[0] * Quat1[3] +
					Quat2[1] * Quat1[2] - Quat2[2] * Quat1[1];
    Dest[1] = Quat2[3] * Quat1[1] + Quat2[1] * Quat1[3] +
					Quat2[2] * Quat1[0] - Quat2[0] * Quat1[2];
    Dest[2] = Quat2[3] * Quat1[2] + Quat2[2] * Quat1[3] +
					Quat2[0] * Quat1[1] - Quat2[1] * Quat1[0];
    Dest[3] = Quat2[3] * Quat1[3] - Quat2[0] * Quat1[0] -
					Quat2[1] * Quat1[1] - Quat2[2] * Quat1[2];

	Inv_QuatNormalize(Dest);

	/*if (Dest[3] < 0)
	{
		int i;
		for (i = 0; i < 4; ++i)
			Dest[i] = -Dest[i];
	}*/
}


void Inv_FinbBestInvertAxisForQuat(vec4_t Axis, vec4_t LastQuat, int Angle)
{
	float MaxDelta = 0;
	int i, j, Best = -1;
	vec4_t Quat, Rot, v;

	if (Angle)
	{
		v[0] = 0;
		v[1] = 0;
		v[2] = 1;
		v[3] = Angle * M_PI / 32768.0;
		Inv_AxisToQuat(v, Rot);
	}

	for (i = 0; i < 4; ++i)
	{
		float Delta;

		Inv_GetInvertAxis(i, Axis);

		if (Angle)
		{
			Inv_AxisToQuat(Axis, v);
			Inv_MulQuat(Rot, v, Quat);
		}
		else
			Inv_AxisToQuat(Axis, Quat);

		Delta = 0;
		for (j = 0; j < 4; ++j)
			Delta += Square(Quat[j] - LastQuat[j]);

		if (Delta < MaxDelta || Best == -1)
		{
			MaxDelta = Delta;
			Best = i;
		}
	}

	Inv_GetInvertAxis(Best, Axis);
}


void Inv_QuatToMatrix(vec4_t Quat, vec3_t Matrix[3])
{
	vec3_t OutMatrix[3];

	OutMatrix[0][0] = 1.0f - (2.0f * Quat[1] * Quat[1]) - (2.0f * Quat[2] * Quat[2]);
	OutMatrix[0][1] = (2.0f * Quat[0] * Quat[1]) - (2.0f * Quat[3] * Quat[2]);
	OutMatrix[0][2] = (2.0f * Quat[0] * Quat[2]) + (2.0f * Quat[3] * Quat[1]);
	OutMatrix[1][0] = (2.0f * Quat[0] * Quat[1]) + (2.0f * Quat[3] * Quat[2]);
	OutMatrix[1][1] = 1.0f - (2.0f * Quat[0] * Quat[0]) - (2.0f * Quat[2] * Quat[2]);
	OutMatrix[1][2] = (2.0f * Quat[1] * Quat[2]) - (2.0f * Quat[3] * Quat[0]);
	OutMatrix[2][0] = (2.0f * Quat[0] * Quat[2]) - (2.0f * Quat[3] * Quat[1]);
	OutMatrix[2][1] = (2.0f * Quat[1] * Quat[2]) + (2.0f * Quat[3] * Quat[0]);
	OutMatrix[2][2] = 1.0f - (2.0f * Quat[0] * Quat[0]) - (2.0f * Quat[1] * Quat[1]);

	TransposeMatrix(OutMatrix, Matrix);
}


void Inv_QuatMultiply(vec4_t Quat, vec3_t InMatrix[3])
{
	vec3_t Matrix[3], OutMatrix[3];

	if (Quat[3] != 1.0f)
	{
		Inv_QuatToMatrix(Quat, Matrix);

		MatrixMultiply(InMatrix, Matrix, OutMatrix);
		memcpy(InMatrix, OutMatrix, sizeof(vec3_t) * 3);
	}
}


/*
=============
Too: Inv_ComputeDynYaw
=============
*/

float Inv_GetAngleDiff(vec3_t Dir, vec3_t Matrix[3])
{
	vec3_t v;
	float a;

	VectorNormalize(Dir);
	assert(DotProduct(Dir, Matrix[2]) < 0.01f);

	a = DotProduct(Dir, Matrix[0]);
	a = acos(a);

	CrossProduct(Dir, Matrix[0], v);
	if (DotProduct(v, Matrix[2]) < 0)
		a = -a;

	return a;
}

int Inv_ComputeDynYaw(vec4_t Final)
{
	vec3_t Dir, v;
	vec3_t Prev[3], Matrix[3];
	vec4_t Quat;
	float a1, a2;
	int LastQuat = (pm->ps->stats[STAT_SPEC1] << 16) + (pm->ps->stats[STAT_SPEC2] & 65535);

	Inv_GetQuatFromStat(LastQuat, Quat, NULL);
	Inv_QuatToMatrix(Quat, Prev);
	Inv_QuatToMatrix(Final, Matrix);

	CrossProduct(Prev[2], Matrix[2], v);
	if (VectorNormalize(v) < 0.001f)
	{
		if (DotProduct(Prev[2], Matrix[2]) > 0)
		{
			v[0] = 1.0f;
			v[1] = v[2] = 0;
		}
		else
		{
			vec3_t Rot[3], Temp[3], Ang;

			Ang[PITCH] = Ang[ROLL] = 0;
			Ang[YAW] = pm->ps->viewangles[YAW];

			AngleVectors(Ang, Rot[0], Rot[1], Rot[2]);

			MatrixMultiply(Rot, Prev, Temp);

			a1 = Inv_GetAngleDiff(Temp[0], Prev);
			a2 = Inv_GetAngleDiff(Temp[0], Matrix);

			a1 -= a2;

			return (int) (a1 * (65536.0f / (2 * M_PI))) & 65535;
		}
	}

	CrossProduct(Prev[2], v, Dir);
	a1 = Inv_GetAngleDiff(Dir, Prev);

	CrossProduct(Matrix[2], v, Dir);
	a2 = Inv_GetAngleDiff(Dir, Matrix);

	a1 -= a2;

	a1 *= (65536.0f / (2 * M_PI));
	if (fabs(a1) < 90)		// => ~0.5°
		a1 = 0;

	return (int) a1 & 65535;
}


/*
===============
Too: Inv_ApplyGravityRotation :
		Time == -1 => Finish the interpolation, so CurrentQuat = FinalQuat
		Time == -2 => Update the CurrentQuate depending of FinalQuat (useful when leaving Ceiling),
 * 							without starting interpolation
 * 	Time == -3 => just return the final Quat
===============
*/

int Inv_QuatIntToStat(vec4_t QuatInt)
{
	uchar Stat[4];
	int i;

	for (i = 0; i < 4; ++i)
		Stat[i] = (uchar) QuatInt[i];

	if (*(int *)Stat == (127 * 256 * 256 * 256))
		Stat[3] = 0;

	return *(int *)Stat;
}

void Inv_FinishDynYaw(void)
{
	pm->ps->delta_angles[YAW] += pm->ps->stats[STAT_DYNYAW];
	pm->ps->stats[STAT_DYNYAW] = 0;

	pm->ps->viewangles[YAW] = SHORT2ANGLE((pm->cmd.angles[YAW] + pm->ps->delta_angles[YAW]) & 65535);
}

void Inv_QuatToInt(vec4_t Quat, vec4_t QuatInt)
{
	int i;
	for (i = 0; i < 4; ++i)
		QuatInt[i] = (int) (Quat[i] * 127.0f);
}

qboolean Inv_ApplyGravityRotation(float Time, vec4_t Final)
{
	vec4_t QuatInt, CurrentInt;
	vec4_t Quat, Current, Axis, Rot, NoRot;
	int LastQuat;
	qboolean Ret = qfalse;	// qtrue ==> current changed

	if (pm && pm->ps)
		LastQuat = (pm->ps->stats[STAT_SPEC1] << 16) + (pm->ps->stats[STAT_SPEC2] & 65535);
	else
	{
		Final[0] = Final[1] = Final[2] = 0;
		Final[3] = 1.0f;
		return qfalse;
	}

	Inv_GetQuatFromStat(LastQuat, Current, CurrentInt);

	if (Gravity[2] == 1.0f)		// exactly inverse
	{
		Inv_FinbBestInvertAxisForQuat(Axis, Current, pm->ps->stats[STAT_DYNYAW]);
		Inv_AxisToQuat(Axis, Quat);
	}
	else
	{
		if (Gravity[2] == -1.0f)	// nothing to do; normal gravity
		{
			Quat[0] = Quat[1] = Quat[2] = 0;
			Quat[3] = 1.0f;
		}
		else
		{
			Axis[0] = Gravity[1];
			Axis[1] = -Gravity[0];
			Axis[2] = 0;

			VectorNormalize(Axis);

			Axis[3] = acos(-Gravity[2]);
			Inv_AxisToQuat(Axis, Quat);
		}

		if (fabs(Current[3]) < 0.002f && Time == -2)
		{
			int Delta;

			if (!pm->ps->stats[STAT_DYNYAW])
				Inv_FinbBestInvertAxisForQuat(Axis, Quat, 0);
			else
			{
				Axis[0] = Axis[1] = 0;
				Axis[2] = 1;
				Axis[3] = pm->ps->stats[STAT_DYNYAW] * M_PI / 32768.0;
				Inv_AxisToQuat(Axis, Rot);

				Inv_MulQuat(Rot, Quat, Current);
				Inv_FinbBestInvertAxisForQuat(Axis, Current, 0);
				pm->ps->stats[STAT_DYNYAW] = 0;
			}

			Inv_AxisToQuat(Axis, Current);

			Delta = Inv_ComputeDynYaw(Current);
			if (Delta)
				Ret = qtrue;
			pm->ps->delta_angles[YAW] += Delta;
			pm->ps->viewangles[YAW] = SHORT2ANGLE((pm->cmd.angles[YAW] + pm->ps->delta_angles[YAW]) & 65535);

			Inv_QuatToInt(Current, CurrentInt);
		}
	}

	if (pm->ps->stats[STAT_DYNYAW] && (Time >= 0 || Time == -3))
	{
		Axis[0] = Axis[1] = 0;
		Axis[2] = 1;
		Axis[3] = pm->ps->stats[STAT_DYNYAW] * M_PI / 32768.0;
		Inv_AxisToQuat(Axis, Rot);

		Vector4Copy(Quat, NoRot);
		Inv_MulQuat(Rot, Quat, Final);
		Vector4Copy(Final, Quat);
	}
	else
		Vector4Copy(Quat, Final);

	if (Time == -3)
		return qfalse;

	Inv_QuatToInt(Quat, QuatInt);

	if (Time == -2)
	{
		LastQuat = Inv_QuatIntToStat(CurrentInt);
	}
	else if (Time == -1)
	{
		LastQuat = Inv_QuatIntToStat(QuatInt);
		Inv_FinishDynYaw();

		Ret = qtrue;
	}
	else if (CurrentInt[0] != QuatInt[0]
			|| CurrentInt[1] != QuatInt[1]
			|| CurrentInt[2] != QuatInt[2]
			|| CurrentInt[3] != QuatInt[3])
	{
		float MaxDelta, Max;
		int i;

		if (Time <= 0.001f)
			Time = 0.001f;

		MaxDelta = Inv_GetQuatMaxDelta(Quat, Current);

		//if (MaxDelta > 1.5f)
		{
			for (i = 0; i < 4; ++i)
				Rot[i] = -Current[i];

			Max = Inv_GetQuatMaxDelta(Quat, Rot);
			if (Max < MaxDelta)
			{
				MaxDelta = Max;
				Vector4Copy(Rot, Current);
				Inv_QuatToInt(Current, CurrentInt);
			}
		}

		Time = MaxDelta * 0.1f * Time / 0.012f;
		if (Time <= 0.01f)
			Time = 0.01f;

		if (MaxDelta <= Time)		// Last step of interpolation
		{
			if (pm->ps->stats[STAT_DYNYAW])
			{
				Inv_FinishDynYaw();
				Inv_QuatToInt(NoRot, QuatInt);
			}
		}
		else
		{
			for (i = 0; i < 4; ++i)
			{
				float Delta = Quat[i] - Current[i];
				float DeltaInt = QuatInt[i] - CurrentInt[i];
				float MinDelta = Time * fabs(Delta) / MaxDelta;

				MinDelta = (int) (MinDelta * 127.0f);

				if (!MinDelta)
					MinDelta = 1;

				if (abs(DeltaInt) > MinDelta)
				{
					if (DeltaInt > 0)
						QuatInt[i] = CurrentInt[i] + MinDelta;
					else
						QuatInt[i] = CurrentInt[i] - MinDelta;
				}
			}

			Vector4Copy(QuatInt, NoRot);

			for (i = 0; i < 4; ++i)
				Quat[i] = QuatInt[i] / 127.0f;

			Inv_QuatNormalize(Quat);
			Inv_QuatToInt(Quat, QuatInt);

			if (CurrentInt[0] == QuatInt[0]
				&& CurrentInt[1] == QuatInt[1]
				&& CurrentInt[2] == QuatInt[2]
				&& CurrentInt[3] == QuatInt[3])
			{
				Vector4Copy(NoRot, QuatInt);
			}
		}

		LastQuat = Inv_QuatIntToStat(QuatInt);

		Ret = qtrue;
	}
	else if (pm->ps->stats[STAT_DYNYAW])
	{
		Inv_FinishDynYaw();
		Inv_QuatToInt(NoRot, QuatInt);
		LastQuat = Inv_QuatIntToStat(QuatInt);

		Ret = qtrue;
	}

	pm->ps->stats[STAT_SPEC1] = (LastQuat >> 16) & 65535;
	pm->ps->stats[STAT_SPEC2] = LastQuat & 65535;

	return Ret;
}

/*
===============
Too: Inv_RemoveGravityComponent
===============
*/
void Inv_RemoveGravityComponent(vec3_t vec)
{
	float d;
	int i;

	d = DotProduct(Gravity, vec);

	if (d)
	{
		for (i = 0; i < 3; ++i)
			vec[i] -= Gravity[i] * d;
	}
}

/*
===============
Too: Inv_IsFiring
===============
*/
int Inv_IsFiring(int State)
{
	switch (State)
	{
		case WEAPON_FIRING: case WEAPON_NEEDRELOAD:
			return WEAPON_FIRING;

		case WEAPON_FIRING2: case WEAPON_NEEDRELOAD2:
			return WEAPON_FIRING2;
	}

	return 0;
}

/*
===============
PM_AddEvent

===============
*/
void PM_AddEvent(int newEvent)
{
	BG_AddPredictableEventToPlayerstate(newEvent, 0, pm->ps);
}

/*
===============
PM_AddEventParm

===============
*/
void PM_AddEventParm(int newEvent, int EventParm)
{
	BG_AddPredictableEventToPlayerstate(newEvent, EventParm, pm->ps);
}


/*
===============
PM_AddTouchEnt
===============
*/
void PM_AddTouchEnt(int entityNum)
{
	int		i;

	if (entityNum == ENTITYNUM_WORLD)
	{
		return;
	}
	if (pm->numtouch == MAXTOUCH)
	{
		return;
	}

	// see if it is already added
	for (i = 0; i < pm->numtouch; i++)
	{
		if (pm->touchents[ i ] == entityNum)
		{
			return;
		}
	}

	// add it
	pm->touchents[pm->numtouch] = entityNum;
	pm->numtouch++;
}

/*
===================
PM_StartTorsoAnim
===================
*/
static void PM_StartTorsoAnim(int anim)
{
	if (pm->ps->pm_type >= PM_DEAD)
	{
		return;
	}
	pm->ps->torsoAnim = ((pm->ps->torsoAnim & ANIM_TOGGLEBIT) ^ ANIM_TOGGLEBIT)
		| anim;
}
static void PM_StartLegsAnim(int anim)
{
	if (pm->ps->pm_type >= PM_DEAD)
	{
		return;
	}
	if (pm->ps->legsTimer > 0)
	{
		return;		// a high priority animation is running
	}
	pm->ps->legsAnim = ((pm->ps->legsAnim & ANIM_TOGGLEBIT) ^ ANIM_TOGGLEBIT)
		| anim;
}

static void PM_ContinueLegsAnim(int anim)
{
	if ((pm->ps->legsAnim & ~ANIM_TOGGLEBIT) == anim)
	{
		return;
	}
	if (pm->ps->legsTimer > 0)
	{
		return;		// a high priority animation is running
	}
	PM_StartLegsAnim(anim);
}

static void PM_ContinueTorsoAnim(int anim)
{
	if ((pm->ps->torsoAnim & ~ANIM_TOGGLEBIT) == anim)
	{
		return;
	}
	if (pm->ps->torsoTimer > 0)
	{
		return;		// a high priority animation is running
	}
	PM_StartTorsoAnim(anim);
}

static void PM_ForceLegsAnim(int anim)
{
	pm->ps->legsTimer = 0;
	PM_StartLegsAnim(anim);
}


/*
==================
PM_ClipVelocity

Slide off of the impacting surface
==================
*/
void PM_ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce)
{
	float	backoff;
	float	change;
	int		i;

	backoff = DotProduct (in, normal);

	if (backoff < 0)
	{
		backoff *= overbounce;
	}
	else
	{
		backoff /= overbounce;
	}

	for (i=0; i<3; i++)
	{
		change = normal[i]*backoff;
		out[i] = in[i] - change;
	}
}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
static void PM_Friction(void)
{
	vec3_t	vec;
	float	*vel;
	float	speed, newspeed, control;
	float	drop;
	int i;

	vel = pm->ps->velocity;

	VectorCopy(vel, vec);
	if (pml.walking)
	{
		if (WallWalk)
			Inv_RemoveGravityComponent(vec);
		else
			vec[2] = 0;	// ignore slope movement
	}

	speed = VectorLength(vec);
	if (speed < 1)
	{
		if (WallWalk)
		{
			float d = DotProduct(Gravity, vec);

			if (d)
			{
				for (i = 0; i < 3; ++i)
					vec[i] = Gravity[i] * d;
			}
		}
		else
		{
			vel[0] = 0;
			vel[1] = 0;		// allow sinking underwater
		}
		// FIXME: still have z friction underwater?

		return;
	}

	drop = 0;

	// apply ground friction
	if (pm->waterlevel <= 1)
	{
		if (pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK))
		{
			// if getting knocked back, no friction
			if (! (pm->ps->pm_flags & PMF_TIME_KNOCKBACK))
			{
				control = speed < pm_stopspeed ? pm_stopspeed : speed;
				drop += control*pm_friction*pml.frametime;
			}
		}
	}

	// apply water friction even if just wading
	if (pm->waterlevel)
	{
		drop += speed*pm_waterfriction*pm->waterlevel*pml.frametime;
	}
	// apply ladder friction
	else if (pml.Ladder && !WallWalk)
	{
		drop += speed * pm_Ladderfriction * pml.frametime;  // Add ladder friction!
	}

	// apply flying friction
	if (pm->ps->powerups[PW_FLIGHT])
	{
		drop += speed*pm_flightfriction*pml.frametime;
	}

	if (pm->ps->pm_type == PM_SPECTATOR)
	{
		drop += speed*pm_spectatorfriction*pml.frametime;
	}

	// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0)
	{
		newspeed = 0;
	}
	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
static void PM_Accelerate(vec3_t wishdir, float wishspeed, float accel)
{
#if 1
	// q2 style
	int			i;
	float		addspeed, accelspeed, currentspeed;

	currentspeed = DotProduct (pm->ps->velocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (addspeed <= 0)
	{
		return;
	}
	accelspeed = accel*pml.frametime*wishspeed;
	if (accelspeed > addspeed)
	{
		accelspeed = addspeed;
	}

	for (i=0; i<3; i++)
	{
		pm->ps->velocity[i] += accelspeed*wishdir[i];
	}
#else
	// proper way (avoids strafe jump maxspeed bug), but feels bad
	vec3_t		wishVelocity;
	vec3_t		pushDir;
	float		pushLen;
	float		canPush;

	VectorScale(wishdir, wishspeed, wishVelocity);
	VectorSubtract(wishVelocity, pm->ps->velocity, pushDir);
	pushLen = VectorNormalize(pushDir);

	canPush = accel*pml.frametime*wishspeed;
	if (canPush > pushLen)
	{
		canPush = pushLen;
	}

	VectorMA(pm->ps->velocity, canPush, pushDir, pm->ps->velocity);
#endif
}



/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
static float PM_CmdScale(usercmd_t *cmd, qboolean Up)
{
	int		max;
	float	total;
	float	scale;

	max = abs(cmd->forwardmove);
	if (abs(cmd->rightmove) > max)
	{
		max = abs(cmd->rightmove);
	}
	if (Up && abs(cmd->upmove) > max)
	{
		max = abs(cmd->upmove);
	}
	if (!max)
	{
		return 0;
	}

	if (Up)
		total = sqrt(cmd->forwardmove * cmd->forwardmove
				+ cmd->rightmove * cmd->rightmove + cmd->upmove * cmd->upmove);
	else
		total = sqrt(cmd->forwardmove * cmd->forwardmove
				+ cmd->rightmove * cmd->rightmove);

	scale = (float)pm->ps->speed * max / (127.0 * total);

	return scale;
}


/*
================
PM_SetMovementDir

Determine the rotation of the legs reletive
to the facing dir
================
*/
static void PM_SetMovementDir(void)
{
	if (pm->cmd.forwardmove || pm->cmd.rightmove)
	{
		if (pm->cmd.rightmove == 0 && pm->cmd.forwardmove > 0)
		{
			pm->ps->movementDir = 0;
		}
		else if (pm->cmd.rightmove < 0 && pm->cmd.forwardmove > 0)
		{
			pm->ps->movementDir = 1;
		}
		else if (pm->cmd.rightmove < 0 && pm->cmd.forwardmove == 0)
		{
			pm->ps->movementDir = 2;
		}
		else if (pm->cmd.rightmove < 0 && pm->cmd.forwardmove < 0)
		{
			pm->ps->movementDir = 3;
		}
		else if (pm->cmd.rightmove == 0 && pm->cmd.forwardmove < 0)
		{
			pm->ps->movementDir = 4;
		}
		else if (pm->cmd.rightmove > 0 && pm->cmd.forwardmove < 0)
		{
			pm->ps->movementDir = 5;
		}
		else if (pm->cmd.rightmove > 0 && pm->cmd.forwardmove == 0)
		{
			pm->ps->movementDir = 6;
		}
		else if (pm->cmd.rightmove > 0 && pm->cmd.forwardmove > 0)
		{
			pm->ps->movementDir = 7;
		}
	}
	else
	{
		// if they aren't actively going directly sideways,
		// change the animation to the diagonal so they
		// don't stop too crooked
		if (pm->ps->movementDir == 2)
		{
			pm->ps->movementDir = 1;
		}
		else if (pm->ps->movementDir == 6)
		{
			pm->ps->movementDir = 7;
		}

	}
}


/*
=============
PM_CheckJump
=============
*/
static qboolean PM_CheckJump(qboolean BigJump)
{
	int Velocity = JUMP_VELOCITY;
	int i;
	qboolean XenoJump = qfalse, Up = qtrue;

	if (pm->ps->pm_flags & PMF_RESPAWNED)
	{
		return qfalse;		// don't allow jump until all buttons are up
	}

	if (AlienRace == e_Selection_Xenomorph && (pm->cmd.buttons & BUTTON_ATTACK2)
		&& BigJump)
	{
		XenoJump = qtrue;
	}
	else if (pm->cmd.upmove < 10)
	{
		// not holding jump
		return qfalse;
	}

	// must wait for jump to be released
	if (pm->ps->pm_flags & PMF_JUMP_HELD)
	{
		// clear upmove so cmdscale doesn't lower running speed
		pm->cmd.upmove = 0;
		return qfalse;
	}

	{	//Too: limit rabbit jumper
		if (pm->cmd.serverTime - pm->ps->powerups[PW_JUMP] < JumpTime)
		{
			// clear upmove so cmdscale doesn't lower running speed
			pm->cmd.upmove = 0;
			return qfalse;
		}

		pm->ps->powerups[PW_JUMP] = pm->cmd.serverTime;

		if (pm->ps->persistant[PERS_TEAM] == InvI->AlienTeam)
		{
			Velocity = InvasionAlienJumpAcc[AlienRace];
			if (XenoJump)
			{
				Velocity *= 2;
				pm->ps->powerups[PW_JUMP] += 250;
			}
		}
	}

	if (!XenoJump)
	{
		for (i = 0; i < 3; ++i)
			pm->ps->velocity[i] -= Velocity * Gravity[i];
	}
	else
	{
		for (i = 0; i < 3; ++i)
			pm->ps->velocity[i] += Velocity * pml.forward[i];
		if (DotProduct(pml.forward, Gravity) >= 0)
			Up = qfalse;

		pm->ps->pm_flags |= PMF_TIME_BIGJUMP;
		pm->ps->pm_time = 250;
	}

	if (WallWalk)
	{
		if (pm->ps->stats[STAT_GRAVITY])
		{
			vec3_t Normal;
			Normal[0] = Normal[1] = 0;
			Normal[2] = 1.0f;

			Inv_ApplyNewGround(Normal, qfalse);
		}

		WallWalk = qfalse;
	}

	PM_AddEvent(EV_JUMP);

	if (pm->cmd.forwardmove >= 0)
	{
		PM_ForceLegsAnim(LEGS_JUMP);
		pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
	}
	else
	{
		PM_ForceLegsAnim(LEGS_JUMPB);
		pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
	}

	if (Up)
	{
		pml.groundPlane = qfalse;		// jumping away
		pml.walking = qfalse;
		pm->ps->pm_flags |= PMF_JUMP_HELD;

		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		return qtrue;
	}
	else
		return qfalse;
}


/*
===================
PM_LadderMove()
by: Calrathan [Arthur Tomlin]

Right now all I know is that this works for VERTICAL ladders.
Ladders with angles on them (urban2 for AQ2) haven't been tested.
===================
*/
static void PM_LadderMove(void)
{
	int i;
	vec3_t wishvel;
	float wishspeed;
	vec3_t wishdir;
	float scale;
	float vel;

	PM_Friction ();

	scale = PM_CmdScale(&pm->cmd, qtrue);

	// user intentions [what the user is attempting to do]
	if (!scale)
	{
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = 0;
	}
	else
	{   // if they're trying to move... lets calculate it
		for (i = 0; i < 3; ++i)
		{
			wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove +
				     		scale * pml.right[i]*pm->cmd.rightmove;
		}
		wishvel[2] += scale * pm->cmd.upmove;

		if (pml.walking && wishvel[2] < 0)
			wishvel[2] = 0;
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	if (wishspeed > pm->ps->speed * pm_LadderScale)
	{
		wishspeed = pm->ps->speed * pm_LadderScale;
	}

	PM_Accelerate (wishdir, wishspeed, pm_LadderAccelerate);

	// This SHOULD help us with sloped ladders, but it remains untested.
	if (pml.groundPlane && DotProduct(pm->ps->velocity, pml.groundTrace.plane.normal) < 0)
	{
		vel = VectorLength(pm->ps->velocity);
		// slide along the ground plane [the ladder section under our feet]
		PM_ClipVelocity(pm->ps->velocity, pml.groundTrace.plane.normal,
								pm->ps->velocity, OVERCLIP);

		VectorNormalize(pm->ps->velocity);
		VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
	}

	PM_SlideMove(qfalse); // move without gravity
}


/*
=============
CheckLadder [ ARTHUR TOMLIN ]
=============
*/
static void CheckLadder(void)
{
	vec3_t flatforward, spot;
	trace_t trace;

	// check for ladder
	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;
	VectorNormalize(flatforward);
	VectorMA(pm->ps->origin, 1, flatforward, spot);
	pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, spot,
					pm->ps->clientNum, MASK_PLAYERSOLID);

	if ((trace.fraction < 1) && (trace.surfaceFlags & SURF_LADDER))
	{
		pml.Ladder = qtrue;
	}
	else
	{
		VectorMA(pm->ps->origin, -1, flatforward, spot);
		pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, spot,
					pm->ps->clientNum, MASK_PLAYERSOLID);

		if ((trace.fraction < 1) && (trace.surfaceFlags & SURF_LADDER))
			pml.Ladder = qtrue;
		else
			pml.Ladder = qfalse;
	}
}


/*
=============
PM_CheckWaterJump
=============
*/
static qboolean	PM_CheckWaterJump(void)
{
	vec3_t	spot;
	int		cont;
	vec3_t	flatforward;

	if (pm->ps->pm_time)
	{
		return qfalse;
	}

	// check for water jump
	if (pm->waterlevel != 2)
	{
		return qfalse;
	}

	flatforward[0] = pml.forward[0];
	flatforward[1] = pml.forward[1];
	flatforward[2] = 0;
	VectorNormalize (flatforward);

	VectorMA (pm->ps->origin, 30, flatforward, spot);
	spot[2] += 4;
	cont = pm->pointcontents (spot, pm->ps->clientNum);
	if (!(cont & CONTENTS_SOLID))
	{
		return qfalse;
	}

	spot[2] += 16;
	cont = pm->pointcontents (spot, pm->ps->clientNum);
	if (cont)
	{
		return qfalse;
	}

	// jump out of water
	VectorScale (pml.forward, 200, pm->ps->velocity);
	pm->ps->velocity[2] = 350;

	pm->ps->pm_flags |= PMF_TIME_WATERJUMP;
	pm->ps->pm_time = 2000;

	return qtrue;
}

//============================================================================


/*
===================
PM_WaterJumpMove

Flying out of the water
===================
*/
static void PM_WaterJumpMove(void)
{
	// waterjump has no control, but falls
	int i;

	PM_StepSlideMove(qtrue);

	for (i = 0; i < 3; ++i)
		pm->ps->velocity[i] += Gravity[i] * pm->ps->gravity * pml.frametime;

	if (pm->ps->velocity[2] < 0)
	{
		// cancel as soon as we are falling down again
		pm->ps->pm_flags &= ~PMF_ALL_TIMES;
		pm->ps->pm_time = 0;
	}
}

/*
===================
PM_WaterMove

===================
*/
static void PM_WaterMove(void)
{
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;
	float	scale;
	float	vel;

	if (PM_CheckWaterJump())
	{
		PM_WaterJumpMove();
		return;
	}
#if 0
	// jump = head for surface
	if (pm->cmd.upmove >= 10)
	{
		if (pm->ps->velocity[2] > -300)
		{
			if (pm->watertype == CONTENTS_WATER)
			{
				pm->ps->velocity[2] = 100;
			}
			else if (pm->watertype == CONTENTS_SLIME)
			{
				pm->ps->velocity[2] = 80;
			}
			else
			{
				pm->ps->velocity[2] = 50;
			}
		}
	}
#endif
	PM_Friction ();

	scale = PM_CmdScale(&pm->cmd, qtrue);
	//
	// user intentions
	//
	if (!scale)
	{
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = -60;		// sink towards bottom
	}
	else
	{
		for (i=0; i<3; i++)
			wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;

		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	if (wishspeed > pm->ps->speed * pm_swimScale)
	{
		wishspeed = pm->ps->speed * pm_swimScale;
	}

	PM_Accelerate (wishdir, wishspeed, pm_wateraccelerate);

	// make sure we can go up slopes easily under water
	if (pml.groundPlane && DotProduct(pm->ps->velocity, pml.groundTrace.plane.normal) < 0)
	{
		vel = VectorLength(pm->ps->velocity);
		// slide along the ground plane
		PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
			pm->ps->velocity, OVERCLIP);

		VectorNormalize(pm->ps->velocity);
		VectorScale(pm->ps->velocity, vel, pm->ps->velocity);
	}

	PM_SlideMove(qfalse);
}

/*#ifdef MISSIONPACK
//===================
//PM_InvulnerabilityMove
//
//Only with the invulnerability powerup
//===================
static void PM_InvulnerabilityMove(void)
{
	pm->cmd.forwardmove = 0;
	pm->cmd.rightmove = 0;
	pm->cmd.upmove = 0;
	VectorClear(pm->ps->velocity);
}
#endif*/

/*
===================
PM_FlyMove

Only with the flight powerup
===================
*/
static void PM_FlyMove(void)
{
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;
	float	scale;

	// normal slowdown
	PM_Friction ();

	scale = PM_CmdScale(&pm->cmd, qtrue);
	//
	// user intentions
	//
	if (!scale)
	{
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = 0;
	}
	else
	{
		for (i=0; i<3; i++)
		{
			wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;
		}

		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	PM_Accelerate (wishdir, wishspeed, pm_flyaccelerate);

	PM_StepSlideMove(qfalse);
}


/*
===================
PM_AirMove

===================
*/
static void PM_AirMove(void)
{
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale, d;
	usercmd_t	cmd;

	PM_Friction();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale(&cmd, qtrue);

	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();

	// project moves down to flat plane
	//pml.forward[2] = 0;
	//pml.right[2] = 0;
	//VectorNormalize (pml.forward);
	//VectorNormalize (pml.right);

	for (i = 0; i < 3; i++)
	{
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	}
	//wishvel[2] = 0;

	d = DotProduct(Gravity, wishvel);

	if (d)
	{
		float l1, l2;

		l1 = VectorLength(wishvel);

		for (i = 0; i < 3; ++i)
			wishvel[i] -= Gravity[i] * d;

		l2 = VectorLength(wishvel);

		if (l1 != l2 && l2 > 0)
		{
			l1 /= l2;

			for (i = 0; i < 3; ++i)
				wishvel[i] *= l1;
		}
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	// not on ground, so little effect on velocity
	PM_Accelerate (wishdir, wishspeed, pm_airaccelerate);

	// we may have a ground plane that is very steep, even
	// though we don't have a groundentity
	// slide along the steep plane
	if (pml.groundPlane)
	{
		PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
			pm->ps->velocity, OVERCLIP);
	}

#if 0
	//ZOID:  If we are on the grapple, try stair-stepping
	//this allows a player to use the grapple to pull himself
	//over a ledge
	if (pm->ps->pm_flags & PMF_GRAPPLE_PULL)
		PM_StepSlideMove (qtrue);
	else
		PM_SlideMove (qtrue);
#endif

	PM_StepSlideMove (qtrue);
}

/*
===================
PM_GrappleMove

===================
*/
static void PM_GrappleMove(void)
{
	vec3_t vel, v;
	float vlen;

	VectorScale(pml.forward, -16, v);
	VectorAdd(pm->ps->grapplePoint, v, v);
	VectorSubtract(v, pm->ps->origin, vel);
	vlen = VectorLength(vel);
	VectorNormalize(vel);

	if (vlen <= 100)
		VectorScale(vel, 10 * vlen, vel);
	else
		VectorScale(vel, 800, vel);

	VectorCopy(vel, pm->ps->velocity);

	pml.groundPlane = qfalse;
}

/*
===================
PM_WalkMove

===================
*/
static void PM_WalkMove(void)
{
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;
	usercmd_t	cmd;
	float		accelerate;
	float		vel;

	if (pm->waterlevel > 2 && DotProduct(pml.forward, pml.groundTrace.plane.normal) > 0)
	{
		// begin swimming
		PM_WaterMove();
		return;
	}


	if (PM_CheckJump(qtrue))
	{
		// jumped away
		if (pm->waterlevel > 1)
		{
			PM_WaterMove();
		}
		else
		{
			PM_AirMove();
		}
		return;
	}

	PM_Friction ();

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	cmd = pm->cmd;
	scale = PM_CmdScale(&cmd, !WallWalk);

	// set the movementDir so clients can rotate the legs for strafing
	PM_SetMovementDir();

	// project moves down to flat plane
	if (WallWalk)
	{
		Inv_RemoveGravityComponent(pml.forward);
		Inv_RemoveGravityComponent(pml.right);
	}
	else
	{
		pml.forward[2] = 0;
		pml.right[2] = 0;
	}

	// project the forward and right directions onto the ground plane
	PM_ClipVelocity (pml.forward, pml.groundTrace.plane.normal, pml.forward, OVERCLIP);
	PM_ClipVelocity (pml.right, pml.groundTrace.plane.normal, pml.right, OVERCLIP);
	//
	VectorNormalize (pml.forward);
	VectorNormalize (pml.right);

	for (i = 0; i < 3; i++)
	{
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	}
	// when going up or down slopes the wish velocity should Not be zero
//	wishvel[2] = 0;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	// clamp the speed lower if ducking
	if (AlienRace == e_Selection_Xenomorph)
	{
		if (WallWalk && wishspeed > pm->ps->speed * pm_WallWalkScale)
		{
			wishspeed = pm->ps->speed * pm_WallWalkScale;
		}
	}
	else if (pm->ps->pm_flags & PMF_DUCKED)
	{
		float duckScale = pm_duckScale;

		if (pm->cmd.upmove >= 0)	//Too:
			duckScale = pm_InvduckScale;

		if (wishspeed > pm->ps->speed * duckScale)
		{
			wishspeed = pm->ps->speed * duckScale;
		}
	}

	if (pm->cmd.serverTime - pm->ps->powerups[PW_JUMP] < JumpTime)		//Too: we're recovering from a jump
	{
		if (JumpTime > 500)
		{
			float coeff = (pm->cmd.serverTime - pm->ps->powerups[PW_JUMP] - 500) / ((float) (JumpTime - 500));

			/*if (AlienRace == e_Selection_Xenomorph)
			{
				if (coeff < 0)
					coeff = 0;
			}
			else*/ if (coeff < -0.5f)
				coeff = -0.5f;

			wishspeed *= (coeff * 0.5f) + 0.5f;
		}
	}

	// clamp the speed lower if wading or walking on the bottom
	if (pm->waterlevel)
	{
		float	waterScale;

		waterScale = pm->waterlevel / 3.0;
		waterScale = 1.0 - (1.0 - pm_swimScale) * waterScale;
		if (wishspeed > pm->ps->speed * waterScale)
		{
			wishspeed = pm->ps->speed * waterScale;
		}
	}

	// when a player gets hit, they temporarily lose
	// full control, which allows them to be moved a bit
	if ((pml.groundTrace.surfaceFlags & SURF_SLICK) || pm->ps->pm_flags & PMF_TIME_KNOCKBACK)
	{
		accelerate = pm_airaccelerate;
	}
	else
	{
		accelerate = pm_accelerate;
	}

	PM_Accelerate (wishdir, wishspeed, accelerate);

	//Com_Printf("velocity = %1.1f %1.1f %1.1f\n", pm->ps->velocity[0], pm->ps->velocity[1], pm->ps->velocity[2]);
	//Com_Printf("velocity1 = %1.1f\n", VectorLength(pm->ps->velocity));

	if ((pml.groundTrace.surfaceFlags & SURF_SLICK)
		|| (pm->ps->pm_flags & PMF_TIME_KNOCKBACK))
	{
		for (i = 0; i < 3; ++i)
			pm->ps->velocity[i] += Gravity[i] * pm->ps->gravity * pml.frametime;
	}
	else
	{
		// don't reset the z velocity for slopes
//		pm->ps->velocity[2] = 0;
	}

	vel = VectorLength(pm->ps->velocity);

	// slide along the ground plane
	PM_ClipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal,
		pm->ps->velocity, OVERCLIP);

	// don't decrease velocity when going up or down a slope
	VectorNormalize(pm->ps->velocity);
	VectorScale(pm->ps->velocity, vel, pm->ps->velocity);

	// don't do anything if standing still
	if (!pm->ps->velocity[0] && !pm->ps->velocity[1] && !pm->ps->velocity[2])
	{
		return;
	}

	PM_StepSlideMove(qfalse);

	//Com_Printf("velocity2 = %1.1f\n", VectorLength(pm->ps->velocity));

}


/*
==============
PM_DeadMove
==============
*/
static void PM_DeadMove(void)
{
	float	forward;

	if (!pml.walking)
	{
		return;
	}

	// extra friction

	forward = VectorLength (pm->ps->velocity);
	forward -= 20;
	if (forward <= 0)
	{
		VectorClear (pm->ps->velocity);
	}
	else
	{
		VectorNormalize (pm->ps->velocity);
		VectorScale (pm->ps->velocity, forward, pm->ps->velocity);
	}
}


/*
===============
PM_NoclipMove
===============
*/
static void PM_NoclipMove(void)
{
	float	speed, drop, friction, control, newspeed;
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;

	pm->ps->viewheight = DEFAULT_VIEWHEIGHT;

	// friction

	speed = VectorLength (pm->ps->velocity);
	if (speed < 1)
	{
		VectorCopy (vec3_origin, pm->ps->velocity);
	}
	else
	{
		drop = 0;

		friction = pm_friction*1.5;	// extra friction
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control*friction*pml.frametime;

		// scale the velocity
		newspeed = speed - drop;
		if (newspeed < 0)
			newspeed = 0;
		newspeed /= speed;

		VectorScale (pm->ps->velocity, newspeed, pm->ps->velocity);
	}

	// accelerate
	scale = PM_CmdScale(&pm->cmd, qtrue);

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;

	for (i=0; i<3; i++)
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	wishvel[2] += pm->cmd.upmove;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	PM_Accelerate(wishdir, wishspeed, pm_accelerate);

	// move
	VectorMA (pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin);
}

//============================================================================

/*
================
PM_FootstepForSurface

Returns an event number apropriate for the groundsurface
================
*/
static int PM_FootstepForSurface(void)
{
	if (pml.groundTrace.surfaceFlags & SURF_NOSTEPS)
	{
		return 0;
	}
	if (pml.groundTrace.surfaceFlags & SURF_METALSTEPS)
	{
		return EV_FOOTSTEP_METAL;
	}
	return EV_FOOTSTEP;
}


/*
=================
PM_CrashLand

Check for hard landings that generate sound events
=================
*/
static void PM_CrashLand(void)
{
	float		delta;
	float		dist;
	float		vel, acc;
	float		t;
	float		a, b, c, den;
	vec3_t vec;

	// decide which landing animation to use
	if (pm->ps->pm_flags & PMF_BACKWARDS_JUMP)
	{
		PM_ForceLegsAnim(LEGS_LANDB);
	}
	else
	{
		PM_ForceLegsAnim(LEGS_LAND);
	}

	pm->ps->legsTimer = TIMER_LAND;

	// calculate the exact velocity on landing
	//dist = pm->ps->origin[2] - pml.previous_origin[2];
	//vel = pml.previous_velocity[2];

	VectorSubtract(pml.previous_origin, pm->ps->origin, vec);
	dist = DotProduct(vec, Gravity);
	vel = -DotProduct(pml.previous_velocity, Gravity);

	acc = -pm->ps->gravity;

	a = acc / 2;
	b = vel;
	c = -dist;

	den =  b * b - 4 * a * c;
	if (den < 0)
	{
		return;
	}
	t = (-b - sqrt(den)) / (2 * a);

	delta = vel + t * acc;
	delta = delta*delta * 0.0001;

	// ducking while falling doubles damage
	if (pm->ps->pm_flags & PMF_DUCKED)
	{
		delta *= 2;
	}

	// never take falling damage if completely underwater
	if (pm->waterlevel == 3)
	{
		return;
	}

	// reduce falling damage if there is standing water
	if (pm->waterlevel == 2)
	{
		delta *= 0.25;
	}
	if (pm->waterlevel == 1)
	{
		delta *= 0.5;
	}

	if (delta < 1)
	{
		return;
	}

	// create a local entity event to play the sound

	// SURF_NODAMAGE is used for bounce pads where you don't ever
	// want to take damage or play a crunch sound
	if (!(pml.groundTrace.surfaceFlags & SURF_NODAMAGE))
	{
		if (pm->ps->persistant[PERS_TEAM] == InvI->MarineTeam)
		{
			if (delta > 75)
			{
				pm->ps->powerups[PW_JUMP] = pm->cmd.serverTime + 2000;
				PM_AddEventParm(EV_FALL_FARTHEST, delta);
			}
			else if (delta > 60)
			{
				pm->ps->powerups[PW_JUMP] = pm->cmd.serverTime + 1000;
				PM_AddEventParm(EV_FALL_FARTHER, delta);
			}
			else if (delta > 45)
			{
				pm->ps->powerups[PW_JUMP] = pm->cmd.serverTime + 500;
				PM_AddEventParm(EV_FALL_FAR, delta);
			}
			else if (delta > 30)
			{
				pm->ps->powerups[PW_JUMP] = pm->cmd.serverTime;
				// this is a pain grunt, so don't play it if dead
				if (pm->ps->stats[STAT_HEALTH] > 0)
				{
					PM_AddEventParm(EV_FALL_MEDIUM, delta);
				}
			}
			else if (delta > 7)
			{
				PM_AddEventParm(EV_FALL_SHORT, delta);
			}
			else
			{
				PM_AddEvent(PM_FootstepForSurface());
			}
		}
		else if (pm->ps->persistant[PERS_TEAM] != InvI->AlienTeam || AlienRace != e_Selection_Xenomorph)
		{
			if (delta > 60)
			{
				PM_AddEventParm(EV_FALL_FAR, delta);
			}
			else if (delta > 40)
			{
				// this is a pain grunt, so don't play it if dead
				if (pm->ps->stats[STAT_HEALTH] > 0)
				{
					PM_AddEventParm(EV_FALL_MEDIUM, delta);
				}
			}
			else if (delta > 7)
			{
				PM_AddEventParm(EV_FALL_SHORT, delta);
			}
			else
			{
				PM_AddEvent(PM_FootstepForSurface());
			}
		}
	}

	// start footstep cycle over
	pm->ps->bobCycle = 0;
}

/*
=============
PM_CheckStuck
=============
*/
/*
void PM_CheckStuck(void)
{
	trace_t trace;

	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);
	if (trace.allsolid)
	{
		//int shit = qtrue;
	}
}
*/

/*
=============
PM_CorrectAllSolid
=============
*/
static int PM_CorrectAllSolid(trace_t *trace)
{
	int			i, j, k, l;
	vec3_t		point;

	if (pm->debugLevel)
	{
		Com_Printf("%i:allsolid\n", c_pmove);
	}

	// jitter around
	for (i = -1; i <= 1; i++)
	{
		for (j = -1; j <= 1; j++)
		{
			for (k = -1; k <= 1; k++)
			{
				VectorCopy(pm->ps->origin, point);
				point[0] += (float) i;
				point[1] += (float) j;
				point[2] += (float) k;
				pm->trace (trace, point, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
				if (!trace->allsolid)
				{
					VectorCopy(point, pm->ps->origin);		//Too: check if it's correct ..?

					for (l = 0; l < 3; ++l)
						point[l] = pm->ps->origin[l] + 0.25f * Gravity[l];

					pm->trace (trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
					pml.groundTrace = *trace;

					assert(!trace->allsolid);

					return qtrue;
				}
			}
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;

	return qfalse;
}


/*
=============
PM_GroundTraceMissed

The ground trace didn't hit a surface, so we are in freefall
=============
*/
static void PM_GroundTraceMissed(void)
{
	trace_t		trace;
	vec3_t		point;
	int i;

	if (!(pm->ps->pm_flags & PMF_TIME_WALLWALK) && pm->ps->stats[STAT_GRAVITY])
	{
		pm->ps->pm_flags |= PMF_TIME_WALLWALK;
		pm->ps->pm_time = 250;
	}

	if (pm->ps->groundEntityNum != ENTITYNUM_NONE)
	{
		// we just transitioned into freefall
		if (pm->debugLevel)
		{
			Com_Printf("%i:lift\n", c_pmove);
		}

		// if they aren't in a jumping animation and the ground is a ways away, force into it
		// if we didn't do the trace, the player would be backflipping down staircases
		VectorCopy(pm->ps->origin, point);

		for (i = 0; i < 3; ++i)
			point[i] += 64 * Gravity[i];

		pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
		if (trace.fraction == 1.0)
		{
			if (pm->cmd.forwardmove >= 0)
			{
				PM_ForceLegsAnim(LEGS_JUMP);
				pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
			}
			else
			{
				PM_ForceLegsAnim(LEGS_JUMPB);
				pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
			}
		}
	}

	pm->ps->groundEntityNum = ENTITYNUM_NONE;
	pml.groundPlane = qfalse;
	pml.walking = qfalse;
}


/*
==============
PM_CheckDuck

Sets mins, maxs, and pm->ps->viewheight
==============
*/
static qboolean PM_CheckDuck(qboolean PutOnGround, vec3_t NewGravity)
{
	trace_t	trace;
	vec3_t OldMins, OldMaxs, Grav;
	float View = pm->ps->viewheight;

	if (NewGravity != NULL)
	{
		VectorCopy(NewGravity, Grav);
	}
	else
	{
		VectorCopy(Gravity, Grav);
	}

	VectorCopy(pm->mins, OldMins);
	VectorCopy(pm->maxs, OldMaxs);

	if (OldMins[2] != MINS_Z || (OldMaxs[2] != 16 && OldMaxs[2] != 32))	//Too: not too clean, but efficient... ;-|
		trace.allsolid = qtrue;
	else
		trace.allsolid = qfalse;

	if (pm->ps->powerups[PW_INVULNERABILITY])
	{
		if (pm->ps->pm_flags & PMF_INVULEXPAND)
		{
			// invulnerability sphere has a 42 units radius
			VectorSet(pm->mins, -42, -42, -42);
			VectorSet(pm->maxs, 42, 42, 42);
		}
		else
		{
			VectorSet(pm->mins, -15, -15, MINS_Z);
			VectorSet(pm->maxs, 15, 15, 16);
		}
		pm->ps->pm_flags |= PMF_DUCKED;
		pm->ps->viewheight = CROUCH_VIEWHEIGHT;
		return qtrue;
	}
	pm->ps->pm_flags &= ~PMF_INVULEXPAND;

	pm->mins[0] = -15;
	pm->mins[1] = -15;

	pm->maxs[0] = 15;
	pm->maxs[1] = 15;

	pm->mins[2] = MINS_Z;

	if (pm->ps->pm_type == PM_DEAD)
	{
		pm->maxs[2] = -8;
		pm->ps->viewheight = DEAD_VIEWHEIGHT;
		return qtrue;
	}

	if (pm->cmd.upmove < 0)
	{	// duck
		pm->ps->pm_flags |= PMF_DUCKED;
	}
	else
	{	// stand up if possible
		if (pm->ps->pm_flags & PMF_DUCKED)
		{
			// try to stand up
			pm->maxs[2] = 32;
			pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);
			if (!trace.allsolid)
				pm->ps->pm_flags &= ~PMF_DUCKED;
		}
	}

	if (pm->ps->pm_flags & PMF_DUCKED)
	{
		pm->maxs[2] = 16;
		pm->ps->viewheight = CROUCH_VIEWHEIGHT;
	}
	else
	{
		pm->maxs[2] = 32;
		pm->ps->viewheight = DEFAULT_VIEWHEIGHT;
	}

	if (WallWalk)
	{
		vec3_t NewMins, NewMaxs;
		int i, Axe;
		float Min = -pm->mins[2], Max = -pm->maxs[2],	//Too: inverted
				MinDelta = 30;

		if (fabs(Grav[0]) > fabs(Grav[1]))
		{
			if (fabs(Grav[0]) > fabs(Grav[2]))
				Axe = 0;
			else
				Axe = 2;
		}
		else if (fabs(Grav[1]) > fabs(Grav[2]))
			Axe = 1;
		else
			Axe = 2;

		MinDelta /= fabs(Grav[Axe]);
		pm->ps->viewheight -= 24 * (1 - fabs(Grav[Axe]));

		for (i = 0; i < 3; ++i)
		{
			float d;

			NewMins[i] = Min * Grav[i];
			NewMaxs[i] = Max * Grav[i];

			if (NewMins[i] > NewMaxs[i])
			{
				float f = NewMins[i];
				NewMins[i] = NewMaxs[i];
				NewMaxs[i] = f;
			}

			if (i != Axe)
			{
				d = NewMaxs[i] - NewMins[i];
				if (d < MinDelta)
				{
					d = (MinDelta - d) * 0.5f;
					NewMins[i] -= d;
					NewMaxs[i] += d;
				}
			}
		}

		pm->trace(&trace, pm->ps->origin, NewMins, NewMaxs, pm->ps->origin,
								pm->ps->clientNum, pm->tracemask);
		if (trace.allsolid)
		{
			vec3_t Point;

			for (i = 0; i < 3; ++i)
				Point[i] = pm->ps->origin[i] + Grav[i] * (13 + MINS_Z);

			pm->trace(&trace, Point, NewMins, NewMaxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);

			if (trace.allsolid)
			{
				for (i = 0; i < 3; ++i)
				{
					NewMins[i] = MINS_Z;
					NewMaxs[i] = -MINS_Z;
				}

				pm->trace(&trace, pm->ps->origin, NewMins, NewMaxs, pm->ps->origin,
										pm->ps->clientNum, pm->tracemask);
			}
			else
			{
				if (trace.fraction == 1.0f)
				{
					//assert(0);		//Too: it can happen with curves (SO WEIRD !?!)
					VectorCopy(Point, pm->ps->origin);
				}
				else
				{
					PutOnGround = qfalse;
					VectorCopy(trace.endpos, pm->ps->origin);
					for (i = 0; i < 3; ++i)
						pm->ps->origin[i] -= Grav[i] * 0.15f;
				}

				pm->trace(&trace, pm->ps->origin, NewMins, NewMaxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);
			}
		}

		if (!trace.allsolid)
		{
			VectorCopy(NewMins, pm->mins);
			VectorCopy(NewMaxs, pm->maxs);
		}
	}

	if (trace.allsolid)
	{
		pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);

		if (trace.allsolid)
		{
			VectorCopy(OldMins, pm->mins);
			VectorCopy(OldMaxs, pm->maxs);
			pm->ps->viewheight = View;
			return qfalse;
		}
	}

	if (PutOnGround)
	{
		vec3_t Point;
		int i;

		for (i = 0; i < 3; ++i)
			Point[i] = pm->ps->origin[i] + Grav[i] * 32;

		pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, Point, pm->ps->clientNum, pm->tracemask);

		if (!trace.allsolid)
		{
			VectorCopy(trace.endpos, pm->ps->origin);
			for (i = 0; i < 3; ++i)
				pm->ps->origin[i] -= Grav[i] * 0.15f;
		}
	}

	return qtrue;
}


/*
=============
Too: Inv_ApplyNewGround
=============
*/
static qboolean Inv_ApplyNewGround(vec3_t Normal, qboolean AdvanceOnNewGround)
{
	int Stat, Current = 0, DynYaw = 0;
	vec4_t Quat;
	vec3_t NewGravity, Origin, vec;
	float	delta, u, v;

	VectorNormalize(Normal);

	vec[0] = Normal[0];
	vec[1] = Normal[1];
	vec[2] = 0;
	if (VectorNormalize(vec) < 0.001f)
	{
		vec[0] = 1.0f;
		vec[1] = 0;
	}

	u = acos(vec[0]);
	if (vec[1] < 0)
		u = -u;

	v = acos(Normal[2]);

	Stat = (((int)(u * 127.9 / M_PI) & 255) << 8) + ((int) (v * 255.9 / M_PI) & 255);

	if ((Stat & 65535) == (pm->ps->stats[STAT_GRAVITY] & 65535))
		return qfalse;

	Inv_GetVectorFromStat(Stat, NewGravity);

	VectorCopy(pm->ps->origin, Origin);

	if (AdvanceOnNewGround)
	{
		int i;
		vec3_t Dir, point, v;
		trace_t trace;
		float Dist = (1 - DotProduct(NewGravity, Gravity)) * 20.0f;

		if (Dist > 10.0f)
			Dist = 10.0f;

		CrossProduct(Normal, Gravity, v);
		CrossProduct(v, Normal, Dir);
		VectorNormalize(Dir);

		for (i = 0; i < 3; ++i)
			point[i] = pm->ps->origin[i] + Dir[i] * Dist - NewGravity[i] * 0.25f;

		pm->trace(&trace, point, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
		if (!trace.allsolid)
			VectorCopy(point, pm->ps->origin);
	}

	if (!PM_CheckDuck(qtrue, NewGravity))
	{
		//if (Stat)
		{
			trace_t trace;
			pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, pm->ps->origin, pm->ps->clientNum, pm->tracemask);

			if (trace.allsolid)
			{
				VectorCopy(Origin, pm->ps->origin);
				return qfalse;
			}
		}
	}

	if (pm->ps->stats[STAT_GRAVITY] == 255)
	{
		Current = (pm->ps->stats[STAT_SPEC1] << 16) + (pm->ps->stats[STAT_SPEC2] & 65535);
		DynYaw = pm->ps->stats[STAT_DYNYAW];

		Inv_ApplyGravityRotation(-1, Quat);		// Put CurrentQuat to Final State of Interpolation
	}

	VectorCopy(NewGravity, Gravity);

	if (Stat == 0 && pm->ps->stats[STAT_GRAVITY] == 255)
	{
		Quat[0] = Quat[1] = Quat[2] = 0;
		Quat[3] = 1.0f;
		pm->ps->stats[STAT_DYNYAW] += Inv_ComputeDynYaw(Quat);

		pm->ps->delta_angles[PITCH] -= pm->ps->viewangles[PITCH] * (2 * 32768 / 180.0f);

		Inv_ApplyGravityRotation(-2, Quat);
	}
	else if (pm->ps->stats[STAT_GRAVITY] == 255)
	{
		if (!Inv_ApplyGravityRotation(-2, Quat))		// Update CurrentQuat when leaving Ceiling
		{
			pm->ps->stats[STAT_SPEC1] = (Current >> 16) & 65535;
			pm->ps->stats[STAT_SPEC2] = Current & 65535;
			pm->ps->stats[STAT_DYNYAW] = DynYaw;
			Inv_ApplyGravityRotation(-3, Quat);
		}
	}
	else
		Inv_ApplyGravityRotation(-3, Quat);

	pm->ps->stats[STAT_DYNYAW] += Inv_ComputeDynYaw(Quat);

	pm->ps->stats[STAT_GRAVITY] = Stat;

	Inv_ApplyGravityRotation(-3, Quat);		// Get Final Quat

	{
		vec3_t Matrix[3];

		PM_UpdateViewAngles(pm->ps, &pm->cmd);
		AngleVectors(pm->ps->viewangles, Matrix[0], Matrix[1], Matrix[2]);
		Inv_QuatMultiply(Quat, Matrix);

		VectorCopy(Matrix[0], pml.forward);
		VectorCopy(Matrix[1], pml.right);
		VectorCopy(Matrix[2], pml.up);
	}

	// use the step move
	VectorSubtract(Origin, pm->ps->origin, Origin);
	delta = DotProduct(Origin, Gravity);

	if (delta > 2)
	{
		if (delta < 7)
		{
			PM_AddEvent(EV_STEP_4);
		}
		else if (delta < 11)
		{
			PM_AddEvent(EV_STEP_8);
		}
		else if (delta < 15)
		{
			PM_AddEvent(EV_STEP_12);
		}
		else
		{
			PM_AddEvent(EV_STEP_16);
		}
	}

	return qtrue;
}


/*
=============
Too: Inv_CheckNewGround
=============
*/
static qboolean Inv_CheckNewGround(void)
{
	vec3_t point;
	trace_t trace;
	vec3_t	wishvel;
	int i, j;
	float Dist = 40, Mul[3];
	vec3_t mins, maxs;

	for (i = 0; i < 3; ++i)
	{
		wishvel[i] = pml.forward[i] * pm->cmd.forwardmove
						+ pml.right[i] * pm->cmd.rightmove;
	}

	if (!VectorNormalize(wishvel))
	{
		VectorCopy(pm->ps->velocity, wishvel);
		if (!VectorNormalize(wishvel))
			return qfalse;
	}

	for (i = 0; i < 3; ++i)
	{
		if (wishvel[i] < 0)
		{
			Mul[i] = pm->mins[i];
		}
		else
			Mul[i] = pm->maxs[i];

		if (wishvel[i])
			Mul[i] = fabs(Mul[i] / wishvel[i]);
		else
			Mul[i] = 10000000.0f;
	}

	if (Mul[0] < Mul[1])
	{
		if (Mul[0] < Mul[2])
			j = 0;
		else
			j = 2;
	}
	else if (Mul[1] < Mul[2])
		j = 1;
	else
		j = 2;

	for (i = 0; i < 3; ++i)
		point[i] = wishvel[i] * Mul[j];

	Dist = VectorLength(point) + 1;
	VectorSet(mins, -5, -5, -5);
	VectorSet(maxs, 5, 5, 5);

	VectorMA(pm->ps->origin, Dist, wishvel, point);
	pm->trace(&trace, pm->ps->origin, mins, maxs, point,
					pm->ps->clientNum, MASK_PLAYERSOLID);

	if (trace.fraction == 1.0f)
		return qfalse;

	if (trace.contents & CONTENTS_PLAYERCLIP)
	{
		VectorMA(trace.endpos, 8, wishvel, point);
		pm->trace(&trace, pm->ps->origin, mins, maxs, point,
					pm->ps->clientNum, MASK_PLAYERSOLID & ~CONTENTS_PLAYERCLIP);
	}

	if (trace.fraction < 1.0f && !(trace.surfaceFlags & SURF_SKY))
	{
		return Inv_ApplyNewGround(trace.plane.normal, qfalse);
		//return qtrue;
	}

	return qfalse;
}


/*
=============
PM_GroundTrace
=============
*/
static void PM_GroundTrace(qboolean Recurs)
{
	vec3_t		point;
	trace_t		trace;
	int i;
	float	d = 0.25f;

	if (WallWalk)
		d = 0.33f;

	for (i = 0; i < 3; ++i)
		point[i] = pm->ps->origin[i] + d * Gravity[i];

	pm->trace(&trace, pm->ps->origin, pm->mins, pm->maxs, point, pm->ps->clientNum, pm->tracemask);
	pml.groundTrace = trace;

	// do something corrective if the trace starts in a solid...
	if (trace.allsolid)
	{
		if (!PM_CorrectAllSolid(&trace))
			return;
	}

	// if the trace didn't hit anything, we are in free fall
	if (trace.fraction == 1.0)
	{
		if (WallWalk && Recurs)
		{
			vec3_t Dir, point2;
			trace_t trace2;
			float l;

			if (Inv_CheckNewGround())
			{
				PM_GroundTrace(qfalse);
				return;
			}

			for (i = 0; i < 3; ++i)
				point2[i] = pm->ps->origin[i] + 32 * Gravity[i];	//Too: Max_Step_Change

			pm->trace(&trace2, pm->ps->origin, pm->mins, pm->maxs, point2, pm->ps->clientNum, pm->tracemask);
			if (trace2.fraction == 1.0)
			{
				VectorCopy(pm->ps->velocity, Dir);
				Inv_RemoveGravityComponent(Dir);
				l = VectorNormalize(Dir) * pml.frametime * 3;
				if (l < 6)
					l = 6;

				for (i = 0; i < 3; ++i)
				{
					point[i] = pm->ps->origin[i] + 5.0f * Gravity[i];
					point2[i] = pm->ps->origin[i] - Dir[i] * l;
				}

				pm->trace(&trace2, point, pm->mins, pm->maxs, point2, pm->ps->clientNum, pm->tracemask);

				if (trace2.fraction < 1.0)
				{
					vec3_t Origin;
					trace = trace2;
					pml.groundTrace = trace;

					VectorCopy(pm->ps->origin, Origin);
					VectorCopy(trace2.endpos, pm->ps->origin);

					if (!Inv_ApplyNewGround(trace.plane.normal, qtrue))
						VectorCopy(Origin, pm->ps->origin);
				}
			}
			else
			{
				qboolean InAir = qtrue;

				if (trace2.fraction < 0.5)
				{
					if (Inv_ApplyNewGround(trace2.plane.normal, qtrue))
					{
						InAir = qfalse;

						trace = trace2;
						pml.groundTrace = trace;
					}
				}

				if (InAir)
				{
					pm->ps->pm_flags |= PMF_TIME_WALLWALK;
					pm->ps->pm_time = 250;
				}
			}
		}

		if (trace.fraction == 1.0)
		{
			PM_GroundTraceMissed();
			pml.groundPlane = qfalse;
			pml.walking = qfalse;
			return;
		}
	}

	// check if getting thrown off the ground
	if (!WallWalk
		&& pm->ps->velocity[2] > 0
		&& DotProduct(pm->ps->velocity, trace.plane.normal) > 10)
	{
		if (pm->debugLevel)
		{
			Com_Printf("%i:kickoff\n", c_pmove);
		}
		// go into jump animation
		if (pm->cmd.forwardmove >= 0)
		{
			PM_ForceLegsAnim(LEGS_JUMP);
			pm->ps->pm_flags &= ~PMF_BACKWARDS_JUMP;
		}
		else
		{
			PM_ForceLegsAnim(LEGS_JUMPB);
			pm->ps->pm_flags |= PMF_BACKWARDS_JUMP;
		}

		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qfalse;
		pml.walking = qfalse;
		return;
	}

	// slopes that are too steep will not be considered onground
	if (trace.plane.normal[2] < MIN_WALK_NORMAL && AlienRace != e_Selection_Xenomorph)
	{
		if (pm->debugLevel)
		{
			Com_Printf("%i:steep\n", c_pmove);
		}
		// FIXME: if they can't slide down the slope, let them
		// walk (sharp crevices)
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = qtrue;
		pml.walking = qfalse;
		return;
	}

	pml.groundPlane = qtrue;
	if (!(pm->ps->pm_flags & PMF_TIME_BIGJUMP))
		pml.walking = qtrue;

	if (WallWalk)
	{
		qboolean ClearEnd = (pm->ps->pm_flags & PMF_TIME_WALLWALK);

		if (!Inv_CheckNewGround())
		{
			qboolean GoodContent = qtrue;

			if (trace.contents & CONTENTS_PLAYERCLIP)
			{
				trace_t trace2;

				for (i = 0; i < 3; ++i)
					point[i] = pm->ps->origin[i] + (0.25f + 8) * Gravity[i];
				//VectorMA(pm->ps->origin, 0.25f + 5, Gravity, point);

				pm->trace(&trace2, pm->ps->origin, pm->mins, pm->maxs,
							point, pm->ps->clientNum, pm->tracemask & ~CONTENTS_PLAYERCLIP);

				if (trace2.fraction == 1.0 || (trace2.surfaceFlags & SURF_SKY))
					GoodContent = qfalse;
			}

			if (!(trace.surfaceFlags & SURF_SKY) && GoodContent)
			{
				Inv_ApplyNewGround(trace.plane.normal, qfalse);
			}
			else
			{
				ClearEnd = qfalse;

				if (!(pm->ps->pm_flags & PMF_TIME_WALLWALK))
				{
					pm->ps->pm_flags |= PMF_TIME_WALLWALK;
					pm->ps->pm_time = 250;
				}
			}
		}

		if (ClearEnd)
		{
			pm->ps->pm_flags &= ~PMF_TIME_WALLWALK;
			if (!(pm->ps->pm_flags & PMF_ALL_TIMES))
				pm->ps->pm_time = 0;
		}
	}

	// hitting solid ground will end a waterjump
	if (pm->ps->pm_flags & PMF_TIME_WATERJUMP)
	{
		pm->ps->pm_flags &= ~(PMF_TIME_WATERJUMP | PMF_TIME_LAND);
		pm->ps->pm_time = 0;
	}

	if (pm->ps->groundEntityNum == ENTITYNUM_NONE)
	{
		// just hit the ground
		if (pm->debugLevel)
		{
			Com_Printf("%i:Land\n", c_pmove);
		}

		PM_CrashLand();

		// don't do landing time if we were just going down a slope
		//if (pml.previous_velocity[2] < -200)		//Too: check that ..?
		{
			// don't allow another jump for a little while
			//pm->ps->pm_flags |= PMF_TIME_LAND;
			//pm->ps->pm_time = 250;
		}
	}

	pm->ps->groundEntityNum = trace.entityNum;

	// don't reset the z velocity for slopes
//	pm->ps->velocity[2] = 0;

	PM_AddTouchEnt(trace.entityNum);
}


/*
=============
PM_SetWaterLevel	FIXME: avoid this twice?  certainly if not moving
=============
*/
static void PM_SetWaterLevel(void)
{
	vec3_t		point;
	int			cont;
	int			sample1;
	int			sample2;

	//
	// get waterlevel, accounting for ducking
	//
	pm->waterlevel = 0;
	pm->watertype = 0;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] + MINS_Z + 1;
	cont = pm->pointcontents(point, pm->ps->clientNum);

	if (cont & MASK_WATER)
	{
		sample2 = pm->ps->viewheight - MINS_Z;
		sample1 = sample2 / 2;

		pm->watertype = cont;
		pm->waterlevel = 1;
		point[2] = pm->ps->origin[2] + MINS_Z + sample1;
		cont = pm->pointcontents (point, pm->ps->clientNum);
		if (cont & MASK_WATER)
		{
			pm->waterlevel = 2;
			point[2] = pm->ps->origin[2] + MINS_Z + sample2;
			cont = pm->pointcontents (point, pm->ps->clientNum);
			if (cont & MASK_WATER){
				pm->waterlevel = 3;
			}
		}
	}

}



//===================================================================


/*
===============
PM_Footsteps
===============
*/
static void PM_Footsteps(void)
{
	float		bobmove;
	int			old;
	qboolean	footstep;

	//
	// calculate speed and cycle to be used for
	// all cyclic walking effects
	//

	if (WallWalk)
	{
		vec3_t vec;

		VectorCopy(pm->ps->velocity, vec);
		Inv_RemoveGravityComponent(vec);
		pm->xyspeed = VectorLength(vec);
	}
	else
		pm->xyspeed = sqrt(pm->ps->velocity[0] * pm->ps->velocity[0]
							+  pm->ps->velocity[1] * pm->ps->velocity[1]);

	if (pm->ps->groundEntityNum == ENTITYNUM_NONE)
	{

		if (pm->ps->powerups[PW_INVULNERABILITY])
		{
			PM_ContinueLegsAnim(LEGS_IDLECR);
		}
		// airborne leaves position in cycle intact, but doesn't advance
		if (pm->waterlevel > 1)
		{
			PM_ContinueLegsAnim(LEGS_SWIM);
		}
		if (pml.Ladder != qtrue)
			return;
	}

	// if not trying to move
	if (!pm->cmd.forwardmove && !pm->cmd.rightmove)
	{
		if (pm->xyspeed < 5)
		{
			pm->ps->bobCycle = 0;	// start at beginning of cycle again
			if (pm->ps->pm_flags & PMF_DUCKED)
			{
				PM_ContinueLegsAnim(LEGS_IDLECR);
			}
			else
			{
				PM_ContinueLegsAnim(LEGS_IDLE);
			}
		}
		return;
	}


	footstep = qfalse;

	if (pm->ps->pm_flags & PMF_DUCKED)
	{
		if (!WallWalk)
			bobmove = 0.5;	// ducked characters bob much faster
		else
			bobmove = 0.4f;

		if (pm->ps->pm_flags & PMF_BACKWARDS_RUN)
		{
			PM_ContinueLegsAnim(LEGS_BACKCR);
		}
		else
		{
			PM_ContinueLegsAnim(LEGS_WALKCR);
		}
		// ducked characters never play footsteps
	/*
	}
	else 	if (pm->ps->pm_flags & PMF_BACKWARDS_RUN)
	{
		if (!(pm->cmd.buttons & BUTTON_WALKING))
		{
			bobmove = 0.4;	// faster speeds bob faster
			footstep = qtrue;
		}
		else
		{
			bobmove = 0.3;
		}
		PM_ContinueLegsAnim(LEGS_BACK);
	*/
	}
	else
	{
		if (!(pm->cmd.buttons & BUTTON_WALKING))
		{
			bobmove = 0.4f;	// faster speeds bob faster
			if (pm->ps->pm_flags & PMF_BACKWARDS_RUN)
			{
				PM_ContinueLegsAnim(LEGS_BACK);
			}
			else
			{
				PM_ContinueLegsAnim(LEGS_RUN);
			}
			footstep = qtrue;
		}
		else
		{
			bobmove = 0.3f;	// walking bobs slow
			if (pm->ps->pm_flags & PMF_BACKWARDS_RUN)
			{
				PM_ContinueLegsAnim(LEGS_BACKWALK);
			}
			else
			{
				PM_ContinueLegsAnim(LEGS_WALK);
			}
		}
	}

	bobmove *= pm->ps->speed * (1.0f / 320.0f);

	// check for footstep / splash sounds
	old = pm->ps->bobCycle;
	pm->ps->bobCycle = (int)(old + bobmove * pml.msec) & 255;

	// if we just crossed a cycle boundary, play an apropriate footstep event
	if (((old + 64) ^ (pm->ps->bobCycle + 64)) & 128)
	{
		if (pm->waterlevel == 0)
		{
			// on ground will only play sounds if running
			if (footstep && !pm->noFootsteps)
			{
				PM_AddEvent(PM_FootstepForSurface());
			}
		}
		else if (pm->waterlevel == 1)
		{
			// splashing
			PM_AddEvent(EV_FOOTSPLASH);
		}
		else if (pm->waterlevel == 2)
		{
			// wading / swimming at surface
			PM_AddEvent(EV_SWIM);
		}
		else if (pm->waterlevel == 3)
		{
			// no sound when completely underwater

		}
	}
}

/*
==============
PM_WaterEvents

Generate sound events for entering and leaving water
==============
*/
static void PM_WaterEvents(void)
{		// FIXME?
	//
	// if just entered a water volume, play a sound
	//
	if (!pml.previous_waterlevel && pm->waterlevel)
	{
		PM_AddEvent(EV_WATER_TOUCH);
	}

	//
	// if just completely exited a water volume, play a sound
	//
	if (pml.previous_waterlevel && !pm->waterlevel)
	{
		PM_AddEvent(EV_WATER_LEAVE);
	}

	//
	// check for head just going under water
	//
	if (pml.previous_waterlevel != 3 && pm->waterlevel == 3)
	{
		PM_AddEvent(EV_WATER_UNDER);
	}

	//
	// check for head just coming out of water
	//
	if (pml.previous_waterlevel == 3 && pm->waterlevel != 3)
	{
		PM_AddEvent(EV_WATER_CLEAR);
	}
}


/*
===============
PM_BeginWeaponChange
===============
*/
static void PM_BeginWeaponChange(int weapon)
{
	if (weapon <= WP_NONE || weapon >= WP_NUM_WEAPONS)
	{
		return;
	}

	if (!(pm->ps->stats[STAT_WEAPONS] & (1 << weapon)))
	{
		return;
	}

	if (pm->ps->weaponstate == WEAPON_DROPPING)
	{
		return;
	}

	PM_AddEvent(EV_CHANGE_WEAPON);
	pm->ps->weaponstate = WEAPON_DROPPING;
	pm->ps->weaponTime += 200;
	PM_StartTorsoAnim(TORSO_DROP);
}


/*
===============
PM_FinishWeaponChange
===============
*/
static void PM_FinishWeaponChange(void)
{
	int		weapon;

	weapon = pm->cmd.weapon;
	if (weapon < WP_NONE || weapon >= WP_NUM_WEAPONS)
	{
		weapon = WP_NONE;
	}

	if (!(pm->ps->stats[STAT_WEAPONS] & (1 << weapon)))
	{
		weapon = WP_NONE;
	}

	pm->ps->weapon = weapon;
	pm->ps->weaponstate = WEAPON_RAISING;
	pm->ps->weaponTime += 250;
	PM_StartTorsoAnim(TORSO_RAISE);
}


/*
==============
PM_TorsoAnimation

==============
*/
static void PM_TorsoAnimation(void)
{
	if (pm->ps->weaponstate == WEAPON_READY)
	{
		if (pm->ps->weapon == WP_GAUNTLET)
		{
			PM_ContinueTorsoAnim(TORSO_STAND2);
		}
		else
		{
			PM_ContinueTorsoAnim(TORSO_STAND);
		}
		return;
	}
}


/*
==============
Too : PM_Fire

Fire the current Weapon
==============
*/
void PM_Fire(int Weapon, int Ammo, int addTime)
{
	if (Ammo)			// If we actually fire
	{
		int Parm = Ammo;

		if (Inv_IsFiring(pm->ps->weaponstate) == WEAPON_FIRING2)
		{
			Parm |= WP_FIRE2BIT;
		}
		//else if (pm->ps->weapon == WP_SHOTGUN)
		//	pm->ps->torsoAnim ^= ANIM_TOGGLEBIT;

		// fire weapon
		PM_AddEventParm(EV_FIRE_WEAPON, Parm);

		// take an ammo away if not infinite
		if (pm->ps->ammo[Weapon] != -1)
		{
			int Current = pm->ps->ammo[Weapon];

			if (MarineArmor != -1 && Current)
			{
				int n = Inv_MarineCharger[Weapon];

				if (n)
				{
					Current = Current % n;
					if (!Current)
						Current = n;
				}
			}

			if (Current >= Ammo)
				Current -= Ammo;
			else
			{
				Ammo = Current;
				Current = 0;
			}

			pm->ps->ammo[Weapon] -= Ammo;

			if (Current == 0 && pm->ps->ammo[Weapon])
			{
				if (Inv_IsFiring(pm->ps->weaponstate) == WEAPON_FIRING)
					pm->ps->weaponstate = WEAPON_NEEDRELOAD;
				else
					pm->ps->weaponstate = WEAPON_NEEDRELOAD2;
			}
		}

		// start the animation even if out of ammo
		if (Weapon == WP_GAUNTLET)
			PM_StartTorsoAnim(TORSO_ATTACK2);
		else
			PM_StartTorsoAnim(TORSO_ATTACK);
	}

	if (pm->ps->powerups[PW_HASTE])
	{
		addTime /= 1.3;
	}

	pm->ps->weaponTime += addTime;
}


/*
==============
Too : PM_SpecialWeapon

do anim/script for Fire
==============
*/

qboolean PM_SpecialWeapon(void)
{
	int addTime, Ammo;

	if (pm->ps->weaponstate == WEAPON_FIRING)	// do special fire
	{
		addTime = 0;
		Ammo = 0;

		switch (pm->ps->weapon)
		{
			default:
				pm->ps->weaponstate = WEAPON_READY;		// nothing special for this weapon
				break;

			case WP_GAUNTLET:
				if (AlienRace == e_Selection_Xenomorph && pm->ps->stats[STAT_WEAPON_SPECIAL] > 0)
				{
					--pm->ps->stats[STAT_WEAPON_SPECIAL];

					if (pm->cmd.upmove >= 0)
					{
						PM_AddEventParm(EV_FIRE_WEAPON, 1);
						PM_StartTorsoAnim(TORSO_ATTACK);
					}
					pm->ps->weaponTime += 250;

					return qtrue;
				}
				else
					pm->ps->weaponstate = WEAPON_READY;		// nothing special for this weapon
				break;

			case WP_INVCHAINGUN:
				if (pm->ps->ammo[pm->ps->weapon])
				{
					int n = pm->ps->stats[STAT_WEAPON_SPECIAL];

					if (n < 0)
					{
						addTime = 67 - n * 16;
						Ammo = 1;

						if (pm->cmd.buttons & BUTTON_ATTACK)
						{
							++n;

							if (n == 0)
								n = 6;
						}
						else
							n = 6 + n;
					}
					else if (n > 0)
					{
						Ammo = 1;
						if (pm->cmd.buttons & BUTTON_ATTACK)
							n = 6;		// will do X more shots
						else
							--n;
						addTime = 67 + (6 - n) * 16;
					}
					pm->ps->stats[STAT_WEAPON_SPECIAL] = n;
				}
				else
					pm->ps->weaponstate = WEAPON_READY;	// Special Fire finished
				break;

			case WP_FLAMETHROWER:
				if (pm->ps->stats[STAT_WEAPON_SPECIAL] > 0)
				{
					--pm->ps->stats[STAT_WEAPON_SPECIAL];
					Ammo = 0;
					addTime = es_FlameThrowerTick;
					PM_StartTorsoAnim(TORSO_ATTACK);
					PM_AddEventParm(EV_FIRE_WEAPON, 0);
				}
				break;
		}

		if (Ammo || addTime)
		{
			PM_Fire(pm->ps->weapon, Ammo, addTime);
			return qtrue;
		}
	}

	return qfalse;
}


/*
==============
Too : PM_SpecialWeapon2

do anim/script for Fire2 (alternate fire)
==============
*/

qboolean PM_SpecialWeapon2(void)
{
	int addTime, Ammo;

	if (pm->ps->weaponstate == WEAPON_FIRING2)	// do special fire
	{
		addTime = 0;
		Ammo = 0;

		switch (pm->ps->weapon)
		{
			default:
				pm->ps->weaponstate = WEAPON_READY;		// nothing special for this weapon
				break;

			case WP_SHOTGUN:
				if (pm->ps->stats[STAT_WEAPON_SPECIAL] > 0 && pm->ps->ammo[pm->ps->weapon])
				{
					--pm->ps->stats[STAT_WEAPON_SPECIAL];
					Ammo = 1;
					addTime = 200;
				}
				else if (pm->ps->stats[STAT_WEAPON_SPECIAL] >= 0)
				{
					addTime = 300;

					if (pm->ps->ammo[pm->ps->weapon] > 0)
						pm->ps->stats[STAT_WEAPON_SPECIAL] = -1;
					else
						pm->ps->weaponstate = WEAPON_READY;	// no more ammo ! don't reload !
				}
				else if (pm->ps->stats[STAT_WEAPON_SPECIAL] == -1)
				{
					pm->ps->stats[STAT_WEAPON_SPECIAL] = -2;
					addTime = 400;
					PM_AddEventParm(EV_BACK_WEAPON, 90);
				}
				else
				{
					addTime = 700;
					PM_AddEvent(EV_RELOAD_WEAPON);
					//pm->ps->weaponstate = WEAPON_READY;	// Special Fire finished
					pm->ps->weaponstate = WEAPON_RELOADINGLAST;
				}
				break;

			case WP_FLAMETHROWER:
				if (pm->ps->stats[STAT_WEAPON_SPECIAL] > 0)
				{
					--pm->ps->stats[STAT_WEAPON_SPECIAL];
					Ammo = 0;
					addTime = es_FlameThrowerTick;
					PM_StartTorsoAnim(TORSO_ATTACK);
					//PM_AddEventParm(EV_FIRE_WEAPON, WP_FIRE2BIT);
				}
				break;

			case WP_BFG:
			{
				qboolean Fire = qfalse;

				if (pm->ps->stats[STAT_WEAPON_SPECIAL] == -1)
				{
					pm->ps->weaponstate = WEAPON_READY;	// Special Fire finished
					addTime = 200;
				}
				else if (pm->cmd.buttons & BUTTON_ATTACK2)
				{
					if (pm->ps->ammo[pm->ps->weapon])
					{
						if (pm->ps->stats[STAT_WEAPON_SPECIAL] >= 4)
							Fire = qtrue;
						else
						{
							PM_AddEvent(EV_BFG_CHARGE);
							--pm->ps->ammo[pm->ps->weapon];
							++pm->ps->stats[STAT_WEAPON_SPECIAL];	// 1 more charge in the BFG
							addTime = 250;
						}
					}
					else
						addTime = 100;

					PM_StartTorsoAnim(TORSO_ATTACK);
				}
				else
					Fire = qtrue;

				if (Fire)
				{
					Ammo = pm->ps->stats[STAT_WEAPON_SPECIAL];
					pm->ps->ammo[pm->ps->weapon] += Ammo;
					pm->ps->stats[STAT_WEAPON_SPECIAL] = -1;
					addTime = 200;
				}
				break;
			}

			case WP_INVCHAINGUN:
				if (pm->ps->ammo[pm->ps->weapon])
				{
					int n = pm->ps->stats[STAT_WEAPON_SPECIAL];

					if (n <= 0)
						pm->ps->weaponstate = WEAPON_READY;	// Special Fire finished
					else
					{
						Ammo = 1;
						--n;

						if (n <= 0)
							addTime = 500;
						else
							addTime = 67;
					}

					pm->ps->stats[STAT_WEAPON_SPECIAL] = n;
				}
				else
				{
					addTime = 500;
					pm->ps->weaponstate = WEAPON_READY;	// Special Fire finished
				}
				break;
		}

		if (Ammo || addTime)
		{
			PM_Fire(pm->ps->weapon, Ammo, addTime);
			return qtrue;
		}
	}

	return qfalse;
}



/*
==============
PM_Weapon

Generates weapon events and modifes the weapon counter
==============
*/
static void PM_Weapon(void)
{
	int addTime, Ammo;
	int Weap;

	// don't allow attack until all buttons are up
	if (pm->ps->pm_flags & PMF_RESPAWNED)
	{
		return;
	}

	// ignore if spectator
	if (pm->ps->persistant[PERS_TEAM] == TEAM_SPECTATOR)
	{
		return;
	}

	// check for dead player
	if (pm->ps->stats[STAT_HEALTH] <= 0)
	{
		pm->ps->weapon = WP_NONE;
		return;
	}

	// check for item using
	if (pm->cmd.buttons & BUTTON_USE_HOLDABLE)
	{
		if (!(pm->ps->pm_flags & PMF_USE_ITEM_HELD))
		{
			if (bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag == HI_MEDKIT
				&& pm->ps->stats[STAT_HEALTH] >= (pm->ps->stats[STAT_MAX_HEALTH] + 25))
				{
				// don't use medkit if at max health
			}
			else
			{
				pm->ps->pm_flags |= PMF_USE_ITEM_HELD;
				PM_AddEvent(EV_USE_ITEM0 + bg_itemlist[pm->ps->stats[STAT_HOLDABLE_ITEM]].giTag);
				pm->ps->stats[STAT_HOLDABLE_ITEM] = 0;
			}
			return;
		}
	}
	else
	{
		pm->ps->pm_flags &= ~PMF_USE_ITEM_HELD;
	}


	// make weapon function
	if (pm->ps->weaponTime > 0)
	{
		pm->ps->weaponTime -= pml.msec;
	}

	// check for weapon change
	// can't change if weapon is firing, but can change
	// again if lowering or raising
	if ((pm->ps->weaponTime <= 0 && pm->ps->weaponstate == WEAPON_READY)
		|| (!Inv_IsFiring(pm->ps->weaponstate)
			&& (pm->ps->weaponstate == WEAPON_RAISING
				|| pm->ps->weaponstate == WEAPON_DROPPING)))
	{
		if (pm->ps->weapon != pm->cmd.weapon)
		{
			PM_BeginWeaponChange(pm->cmd.weapon);
		}
	}

	if (pm->ps->weaponTime > 0)
	{
		return;
	}

	if (PM_SpecialWeapon() == qtrue)
		return;
	if (PM_SpecialWeapon2() == qtrue)
		return;

	// change weapon if time
	if (pm->ps->weaponstate == WEAPON_DROPPING)
	{
		PM_FinishWeaponChange();
		return;
	}

	if ((pm->cmd.buttons & BUTTON_RELOAD) && pm->ps->weaponstate == WEAPON_READY)
	{
		if (MarineArmor != -1)
		{
			int Current = pm->ps->ammo[pm->ps->weapon];

			if (Current)
			{
				int n = Inv_MarineCharger[pm->ps->weapon];

				if (n)
				{
					int Mul = Current / n;

					if (Mul)
					{
						Current = Current % n;
						if (!Current)
						{
							Current = n;
							--Mul;
						}

						if (Mul)
						{
							pm->ps->ammo[pm->ps->weapon] -= Current;
							pm->ps->weaponstate = WEAPON_NEEDRELOAD;
						}
					}
				}
			}
		}
	}

	if (pm->ps->weaponstate == WEAPON_NEEDRELOAD
		|| pm->ps->weaponstate == WEAPON_NEEDRELOAD2)
	{
		int Time = Inv_ReloadTime[pm->ps->weapon] >> 1;

		pm->ps->weaponstate = WEAPON_RELOADING;
		pm->ps->weaponTime += Time * 10;
		PM_AddEventParm(EV_BACK_WEAPON, Time << 1);
		return;
	}

	if (pm->ps->weaponstate == WEAPON_RELOADING)
	{
		int Time = Inv_ReloadTime[pm->ps->weapon] >> 1;

		pm->ps->weaponstate = WEAPON_RELOADINGLAST;
		pm->ps->weaponTime += Time * 10;
		PM_AddEvent(EV_RELOAD_WEAPON);
		return;
	}

	if (pm->ps->weaponstate == WEAPON_RAISING)
	{
		pm->ps->weaponstate = WEAPON_READY;
		if (pm->ps->weapon == WP_GAUNTLET)
		{
			PM_StartTorsoAnim(TORSO_STAND2);
		}
		else
		{
			PM_StartTorsoAnim(TORSO_STAND);
		}
		return;
	}

	// check for fire
	if (!(pm->cmd.buttons & (BUTTON_ATTACK | BUTTON_ATTACK2)))
	{
		pm->ps->weaponTime = 0;
		pm->ps->weaponstate = WEAPON_READY;
		return;
	}

	// start the animation even if out of ammo
	if (pm->ps->weapon == WP_GAUNTLET)
	{
		// the guantlet only "fires" when it actually hits something
		/*if (!pm->gauntletHit)
		{
			pm->ps->weaponTime = 0;
			pm->ps->weaponstate = WEAPON_READY;
			return;
		} */

		PM_AddEventParm(EV_FIRE_WEAPON, 0);
		//PM_StartTorsoAnim(TORSO_ATTACK2);
	}
	else
	{
		//PM_StartTorsoAnim(TORSO_ATTACK);
	}

	pm->ps->weaponstate = WEAPON_FIRING;

	// check for out of ammo
	if (!pm->ps->ammo[pm->ps->weapon])
	{
		if (pm->ps->weapon != WP_BFG)
		{
			PM_AddEvent(EV_NOAMMO);
			pm->ps->weaponTime += 500;
		}
		else
			pm->ps->weaponTime += 250;
		return;
	}

	Weap = pm->ps->weapon;

	if (pm->cmd.buttons & BUTTON_ATTACK2)
		Weap |= WP_FIRE2BIT;

	Ammo = 1;	// Set default ammo

	switch (Weap & WP_NO_FIRE2BIT)
	{
	default:
	case WP_GAUNTLET:
		if (AlienRace == e_Selection_Xenomorph)
		{
			//if (pm->cmd.upmove < 0)
			//	addTime = 500;
			//else
			pm->ps->stats[STAT_WEAPON_SPECIAL] = 1;
			addTime = 250;
		}
		else
			addTime = 400;
		break;
	case WP_LIGHTNING:
		addTime = 50;
		break;
	case WP_SHOTGUN:
		if (Weap & WP_FIRE2BIT)
		{
			addTime = 200;
			Ammo = 1;
			pm->ps->stats[STAT_WEAPON_SPECIAL] = 2;		// will do 2 more shots
			pm->ps->weaponstate = WEAPON_FIRING2;	// indicate we are in special fire mode
		}
		else
			addTime = 1000;
		break;
	case WP_MACHINEGUN:
		if ((Weap & WP_FIRE2BIT) && AlienRace == e_Selection_Trooper)
		{
			addTime = 400;
			if (!pm->ps->ammo[WP_GRENADE_LAUNCHER])
				return;

			pm->ps->weaponstate = WEAPON_FIRING2;
			Weap = WP_GRENADE_LAUNCHER;
		}
		else
			addTime = 100;
		break;
	case WP_INVCHAINGUN:
		if (Weap & WP_FIRE2BIT)
		{
			pm->ps->weaponstate = WEAPON_FIRING2;		// indicate we are in special fire mode
			pm->ps->stats[STAT_WEAPON_SPECIAL] = 4;	// will do X more shots
			addTime = 67;
		}
		else
		{
			addTime = 67 + 5 * 16;
			pm->ps->stats[STAT_WEAPON_SPECIAL] = -4;		// will do X more shots before full speed
		}
		break;
	case WP_GRENADE_LAUNCHER:
		addTime = 800;
		break;
	case WP_ROCKET_LAUNCHER:
		addTime = 800;
		break;
	case WP_PLASMAGUN:
		if (Weap & WP_FIRE2BIT)
		{
			if (pm->ps->ammo[WP_PLASMAGUN] >= 5)
			{
				pm->ps->weaponstate = WEAPON_FIRING2;
				addTime = 1000;
				Ammo = 5;
			}
			else
			{
				Ammo = 0;
				addTime = 100;
			}
		}
		else
			addTime = 200;//100;
		break;
	case WP_FLAMETHROWER:
		if (Weap & WP_FIRE2BIT)
			pm->ps->weaponstate = WEAPON_FIRING2;		// indicate we are in special fire mode
		pm->ps->stats[STAT_WEAPON_SPECIAL] = 2;		// will do X more animation (to block anim in fire mode)
		addTime = es_FlameThrowerTime - 2 * es_FlameThrowerTick;
		break;
	case WP_RAILGUN:
		addTime = 166;//500;		//Too: 400
		if (Weap & WP_FIRE2BIT)
		{
			pm->ps->weaponTime = 0;
			pm->ps->weaponstate = WEAPON_READY;
			return;
		}
		break;
	case WP_BFG:
//		addTime = 100;
		if (Weap & WP_FIRE2BIT)
		{
			--pm->ps->ammo[pm->ps->weapon];
			pm->ps->weaponstate = WEAPON_FIRING2;
			pm->ps->stats[STAT_WEAPON_SPECIAL] = 1;	// 1 charge in the BFG
			Ammo = 0;
			PM_AddEvent(EV_BFG_CHARGE);
			addTime = 250;
			PM_StartTorsoAnim(TORSO_ATTACK);
		}
		else
			addTime = 200;
		break;
	case WP_GRAPPLING_HOOK:
		addTime = 400;
		break;
/*#ifdef MISSIONPACK
	case WP_NAILGUN:
		addTime = 1000;
		break;
	case WP_PROX_LAUNCHER:
		addTime = 800;
		break;
	case WP_CHAINGUN:
		addTime = 30;
		break;
#endif*/
	}

/*#ifdef MISSIONPACK
	if (bg_itemlist[pm->ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_SCOUT)
	{
		addTime /= 1.5;
	}
	else
	if (bg_itemlist[pm->ps->stats[STAT_PERSISTANT_POWERUP]].giTag == PW_AMMOREGEN)
	{
		addTime /= 1.3;
  }
  else
#endif*/
	if (pm->ps->powerups[PW_HASTE])
	{
		addTime /= 1.3;
	}

	PM_Fire(Weap & WP_NO_FIRE2BIT, Ammo, addTime);
}

/*
================
PM_Animate
================
*/

static void PM_Animate(void)
{
	if (pm->cmd.buttons & BUTTON_GESTURE)
	{
		if (pm->ps->torsoTimer == 0)
		{
			PM_StartTorsoAnim(TORSO_GESTURE);
			pm->ps->torsoTimer = TIMER_GESTURE;
			PM_AddEvent(EV_TAUNT);
		}
/*#ifdef MISSIONPACK
	}
	else if (pm->cmd.buttons & BUTTON_GETFLAG)
	{
		if (pm->ps->torsoTimer == 0)
		{
			PM_StartTorsoAnim(TORSO_GETFLAG);
			pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
		}
	}
	else if (pm->cmd.buttons & BUTTON_GUARDBASE)
	{
		if (pm->ps->torsoTimer == 0)
		{
			PM_StartTorsoAnim(TORSO_GUARDBASE);
			pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
		}
	}
	else if (pm->cmd.buttons & BUTTON_PATROL)
	{
		if (pm->ps->torsoTimer == 0)
		{
			PM_StartTorsoAnim(TORSO_PATROL);
			pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
		}
	}
	else if (pm->cmd.buttons & BUTTON_FOLLOWME)
	{
		if (pm->ps->torsoTimer == 0)
		{
			PM_StartTorsoAnim(TORSO_FOLLOWME);
			pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
		}
	}
	else if (pm->cmd.buttons & BUTTON_AFFIRMATIVE) //Too: Affirmative Button disabled (used for 2nd fire)
	{
		if (pm->ps->torsoTimer == 0)
		{
			PM_StartTorsoAnim(TORSO_AFFIRMATIVE);
			pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
		}
	}
	else if (pm->cmd.buttons & BUTTON_NEGATIVE)
	{
		if (pm->ps->torsoTimer == 0)
		{
			PM_StartTorsoAnim(TORSO_NEGATIVE);
			pm->ps->torsoTimer = 600;	//TIMER_GESTURE;
		}
#endif*/
	}
}


/*
================
PM_DropTimers
================
*/
static void PM_DropTimers(void)
{
	// drop misc timing counter
	if (pm->ps->pm_time)
	{
		if (pml.msec >= pm->ps->pm_time)
		{
			if (pm->ps->pm_flags & PMF_TIME_WALLWALK)
			{
				vec3_t Normal;

				Normal[0] = Normal[1] = 0;
				Normal[2] = 1.0f;

				Inv_ApplyNewGround(Normal, qfalse);

				WallWalk = qfalse;
			}

			pm->ps->pm_flags &= ~PMF_ALL_TIMES;
			pm->ps->pm_time = 0;
		}
		else
		{
			pm->ps->pm_time -= pml.msec;
		}
	}

	// drop animation counter
	if (pm->ps->legsTimer > 0)
	{
		pm->ps->legsTimer -= pml.msec;
		if (pm->ps->legsTimer < 0)
		{
			pm->ps->legsTimer = 0;
		}
	}

	if (pm->ps->torsoTimer > 0)
	{
		pm->ps->torsoTimer -= pml.msec;
		if (pm->ps->torsoTimer < 0)
		{
			pm->ps->torsoTimer = 0;
		}
	}
}

/*
================
PM_UpdateViewAngles

This can be used as another entry point when only the viewangles
are being updated isntead of a full move
================
*/
void PM_UpdateViewAngles(playerState_t *ps, const usercmd_t *cmd)
{
	short		temp;
	int		i;

	if (ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPINTERMISSION)
	{
		return;		// no view changes at all
	}

	if (ps->pm_type != PM_SPECTATOR && ps->stats[STAT_HEALTH] <= 0)
	{
		return;		// no view changes at all
	}

	// circularly clamp the angles with deltas
	for (i=0; i<3; i++)
	{
		temp = cmd->angles[i] + ps->delta_angles[i];
		if (i == PITCH)
		{
			// don't let the player look up or down more than 90 degrees
			if (temp > 16000)
			{
				ps->delta_angles[i] = 16000 - cmd->angles[i];
				temp = 16000;
			}
			else if (temp < -16000)
			{
				ps->delta_angles[i] = -16000 - cmd->angles[i];
				temp = -16000;
			}
		}
		ps->viewangles[i] = SHORT2ANGLE(temp);
	}

}


/* ================
//Too: can only jump
================ */
void PM_InvasionFreezed(void)
{
	qboolean gravity = qtrue;

	if (pml.walking && !PM_CheckJump(qfalse))
	{
		// jumped away
		gravity = qfalse;
	}

	PM_Friction();
	PM_StepSlideMove(gravity);
}


/*
================
PmoveSingle

================
*/
void trap_SnapVector(float *v);

void PmoveSingle(pmove_t *pmove, InvasionInfo_t *InvInfo)
{
	pm = pmove;
	InvI = InvInfo;

	// this counter lets us debug movement problems with a journal
	// by setting a conditional breakpoint fot the previous frame
	c_pmove++;

	// clear results
	pm->numtouch = 0;
	pm->watertype = 0;
	pm->waterlevel = 0;

	if (pm->ps->stats[STAT_HEALTH] <= 0)
	{
		pm->tracemask &= ~CONTENTS_BODY;	// corpses can fly through bodies
	}

	// make sure walking button is clear if they are running, to avoid
	// proxy no-footsteps cheats
	if (abs(pm->cmd.forwardmove) > 64 || abs(pm->cmd.rightmove) > 64)
	{
		pm->cmd.buttons &= ~BUTTON_WALKING;
	}

	// set the talk balloon flag
	if (pm->cmd.buttons & BUTTON_TALK)
	{
		pm->ps->eFlags |= EF_TALK;
	}
	else
	{
		pm->ps->eFlags &= ~EF_TALK;
	}

	// set the firing flag for continuous beam weapons
	if (!(pm->ps->pm_flags & PMF_RESPAWNED) && pm->ps->pm_type != PM_INTERMISSION
		&& (pm->cmd.buttons & (BUTTON_ATTACK | BUTTON_ATTACK2)) && (pm->ps->ammo[pm->ps->weapon] || pm->ps->weapon == WP_BFG))
	{
		pm->ps->eFlags |= EF_FIRING;
	}
	else
	{
		pm->ps->eFlags &= ~EF_FIRING;
	}

	// clear the respawned flag if attack and use are cleared
	if (pm->ps->stats[STAT_HEALTH] > 0 &&
		!(pm->cmd.buttons & (BUTTON_ATTACK | BUTTON_ATTACK2 | BUTTON_USE_HOLDABLE)))
	{
		pm->ps->pm_flags &= ~PMF_RESPAWNED;
	}

	// if talk button is down, dissallow all other input
	// this is to prevent any possible intercept proxy from
	// adding fake talk balloons
	if (pmove->cmd.buttons & BUTTON_TALK)
	{
		// keep the talk button set tho for when the cmd.serverTime > 66 msec
		// and the same cmd is used multiple times in Pmove
		pmove->cmd.buttons = BUTTON_TALK;
		pmove->cmd.forwardmove = 0;
		pmove->cmd.rightmove = 0;
		pmove->cmd.upmove = 0;
	}

	// clear all pmove local vars
	memset (&pml, 0, sizeof(pml));

	// determine the time
	pml.msec = pmove->cmd.serverTime - pm->ps->commandTime;
	if (pml.msec < 1)
	{
		pml.msec = 1;
	}
	else if (pml.msec > 200)
	{
		pml.msec = 200;
	}
	pm->ps->commandTime = pmove->cmd.serverTime;

	// save old org in case we get stuck
	VectorCopy (pm->ps->origin, pml.previous_origin);

	// save old velocity for crashlanding
	VectorCopy (pm->ps->velocity, pml.previous_velocity);

	pml.frametime = pml.msec * 0.001;

	Inv_GetVectorFromStat(pmove->ps->stats[STAT_GRAVITY], Gravity);

	if (AlienRace == e_Selection_Xenomorph
		&& (pm->cmd.buttons & BUTTON_ATTACK2)
		&& !pm->ps->stats[STAT_GRAVITY]
		&& !(pm->ps->pm_flags & PMF_JUMP_HELD))
	{
		pm->cmd.upmove = 10;
	}

	if (AlienRace == e_Selection_Xenomorph && pm->cmd.upmove < 0)
	{
		WallWalk = qtrue;
	}
	else
	{
		if (pm->ps->stats[STAT_GRAVITY])
		{
			vec3_t Normal;
			Normal[0] = Normal[1] = 0;
			Normal[2] = 1.0f;

			Inv_ApplyNewGround(Normal, qfalse);
		}

		WallWalk = qfalse;
	}

	// update the viewangles
	if (WallWalk
		|| (AlienRace == e_Selection_Xenomorph
			&& (pm->ps->stats[STAT_SPEC1] || pm->ps->stats[STAT_SPEC2])))
	{
		vec3_t Matrix[3];
		vec4_t Quat;

		Inv_ApplyGravityRotation(pml.frametime, Quat);

		PM_UpdateViewAngles(pm->ps, &pm->cmd);
		AngleVectors(pm->ps->viewangles, Matrix[0], Matrix[1], Matrix[2]);

		Inv_QuatMultiply(Quat, Matrix);

		VectorCopy(Matrix[0], pml.forward);
		VectorCopy(Matrix[1], pml.right);
		VectorCopy(Matrix[2], pml.up);
	}
	else
	{
		PM_UpdateViewAngles(pm->ps, &pm->cmd);
		AngleVectors (pm->ps->viewangles, pml.forward, pml.right, pml.up);
	}

	if (pm->cmd.upmove < 10
		&& (AlienRace != e_Selection_Xenomorph
			|| !(pm->cmd.buttons & BUTTON_ATTACK2)))
	{
		// not holding jump
		pm->ps->pm_flags &= ~PMF_JUMP_HELD;
	}

	// decide if backpedaling animations should be used
	if (pm->cmd.forwardmove < 0)
	{
		pm->ps->pm_flags |= PMF_BACKWARDS_RUN;
	}
	else if (pm->cmd.forwardmove > 0 || (pm->cmd.forwardmove == 0 && pm->cmd.rightmove))
	{
		pm->ps->pm_flags &= ~PMF_BACKWARDS_RUN;
	}

	if (pm->ps->pm_type >= PM_DEAD)
	{
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
	}

	if (pm->ps->pm_type == PM_NOCLIP || pm->ps->pm_type == PM_SPECTATOR)
	{
		PM_NoclipMove ();
		PM_DropTimers ();
		return;
	}

	/*if (pm->ps->pm_type == PM_SPECTATOR)	//Too:
	{
		PM_CheckDuck(qfalse);
		PM_FlyMove();
		PM_DropTimers();
		return;
	}*/

	if (pm->ps->pm_type == PM_FREEZE)
	{
		return;		// no movement at all
	}

	if (pm->ps->pm_type == PM_INTERMISSION || pm->ps->pm_type == PM_SPINTERMISSION)
	{
		return;		// no movement at all
	}

	// set watertype, and waterlevel
	PM_SetWaterLevel();
	pml.previous_waterlevel = pmove->waterlevel;

	// set mins, maxs, and viewheight
	PM_CheckDuck(qfalse, NULL);

	// set groundentity
	PM_GroundTrace(qtrue);

	if (pm->ps->pm_type == PM_DEAD)
	{
		PM_DeadMove ();
	}

	PM_DropTimers();

	CheckLadder();  // ARTHUR TOMLIN check and see if they're on a ladder

	if (InvInfo->Period == e_Period_Selection)		//Too: freeze player before the round start
	{
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
	}

/*#ifdef MISSIONPACK
	if (pm->ps->powerups[PW_INVULNERABILITY])
	{
		PM_InvulnerabilityMove();
	}
	else
#endif*/
	if (pm->ps->powerups[PW_FLIGHT])
	{
		// flight powerup doesn't allow jump and has different friction
		PM_FlyMove();
	}
	else if (pm->ps->pm_flags & PMF_GRAPPLE_PULL)
	{
		PM_GrappleMove();
		// We can wiggle a bit
		PM_AirMove();
	}
	else if (pm->ps->pm_flags & PMF_TIME_WATERJUMP)
	{
		PM_WaterJumpMove();
	}
	else if (pm->waterlevel > 1)
	{
		// swimming
		PM_WaterMove();
	}
	else if (pml.Ladder && !WallWalk)
	{
		PM_LadderMove();
	}
	else if (pml.walking)
	{
		// walking on ground
		PM_WalkMove();
	}
	else
	{
		// airborne
		PM_AirMove();
	}

	PM_Animate();

	// set groundentity, watertype, and waterlevel
	PM_GroundTrace(qtrue);
	PM_SetWaterLevel();

	// weapons
	if (InvInfo->Period != e_Period_Selection)
		PM_Weapon();

	// torso animation
	PM_TorsoAnimation();

	// footstep events / legs animations
	PM_Footsteps();

	// entering / leaving water splashes
	PM_WaterEvents();

	/*if (WallWalk
		|| (AlienRace == e_Selection_Xenomorph
			&& (pm->ps->stats[STAT_SPEC1] || pm->ps->stats[STAT_SPEC2])))
	{
		vec3_t Matrix[3];
		vec4_t Quat;
		int Current = (pm->ps->stats[STAT_SPEC1] << 16) + (pm->ps->stats[STAT_SPEC2] & 65535);

		Inv_GetQuatFromStat(Current, Quat, NULL);
		AngleVectors(pm->ps->viewangles, Matrix[0], Matrix[1], Matrix[2]);
		Inv_QuatMultiply(Quat, Matrix);

		Inv_MatrixToAngles(Matrix, pm->ps->viewangles);
	}*/

	// snap some parts of playerstate to save network bandwidth
	trap_SnapVector(pm->ps->velocity);
}


/*
================
Pmove

Can be called by either the server or the client
================
*/
void Pmove(pmove_t *pmove, InvasionInfo_t *InvInfo)
{
	int			finalTime;

	finalTime = pmove->cmd.serverTime;

	if (finalTime < pmove->ps->commandTime)
	{
		return;	// should not happen
	}

	if (finalTime > pmove->ps->commandTime + 1000)
	{
		pmove->ps->commandTime = finalTime - 1000;
	}

	AlienRace = -1;
	MarineArmor = -1;

	JumpTime = 0;

	if (pmove->ps->persistant[PERS_TEAM] == InvInfo->MarineTeam)
	{
		MarineArmor = pmove->ps->persistant[PERS_CLASS] & e_Class_MarineArmorMask;

		JumpTime = InvasionMarineJumpTime[MarineArmor];
	}
	else if (pmove->ps->persistant[PERS_TEAM] == InvInfo->AlienTeam)
	{
		AlienRace = pmove->ps->persistant[PERS_CLASS] & e_Class_AlienRaceMask;

		JumpTime = InvasionAlienJumpTime[AlienRace];
	}

	pmove->ps->pmove_framecount = (pmove->ps->pmove_framecount+1) & ((1<<PS_PMOVEFRAMECOUNTBITS)-1);

	// chop the move up if it is too long, to prevent framerate
	// dependent behavior
	while (pmove->ps->commandTime != finalTime)
	{
		int		msec;

		msec = finalTime - pmove->ps->commandTime;

		if (pmove->pmove_fixed)
		{
			if (msec > pmove->pmove_msec)
			{
				msec = pmove->pmove_msec;
			}
		}
		else
		{
			if (msec > 66)
			{
				msec = 66;
			}
		}

		pmove->cmd.serverTime = pmove->ps->commandTime + msec;
		PmoveSingle(pmove, InvInfo);

		if (pmove->ps->pm_flags & PMF_JUMP_HELD)
		{
			pmove->cmd.upmove = 20;
		}
	}

	//PM_CheckStuck();

}

/*==================== EOF because of buggy VSS ===========*/