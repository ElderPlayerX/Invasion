//
// Copyright (C) 2000 ManuTOO ! (quite impressive, isn't it ? :o)))
//


#include "g_local.h"
#include "g_invasion.h"

#include "botlib.h"
#include "be_aas.h"
#include "be_ea.h"
#include "be_ai_char.h"
#include "be_ai_chat.h"
#include "be_ai_gen.h"
#include "be_ai_goal.h"
#include "be_ai_move.h"
#include "be_ai_weap.h"

#include "ai_main.h"
#include "ai_dmq3.h"

#define	MAX_EGGS		100


// Invasion internal functions
void InvasionUpdateClientSideStat(void);
int NbMemberAlive(team_t Team);
void SetEverybodyBaseTeam(void);


// Invasion CVar
vmCvar_t	g_InvSwapPeriod;
vmCvar_t	g_InvRoundTime;
vmCvar_t	g_InvWarmUp;
vmCvar_t	g_InvAnarchy;
vmCvar_t	g_InvEggHealth;
vmCvar_t g_InvAutoMode;

float InvArmorSpeed[e_Selection_MaxArmor] = { 1.00f, 0.85f, 0.70f, 0.80f };
float InvAlienSpeed[e_Selection_MaxRace] = { 1.2f, 1.1f, 1.15f, 1.5f };
float InvArmorProtection[e_Selection_MaxArmor] = { 0.85f, 0.90f, 0.95f, 0.95f };
int InvMarineArmorMax[e_Selection_MaxArmor] = { 300, 500, 750, 200 };
int InvMarineWeaponAmmunitions[WP_NUM_WEAPONS] = { 0, -1, 200, 75, 0, 30, 0, 320, 0, 4, 800, 400, -1 };

char *MarineGoalClassName = NULL;
char *MarineGoalPickupName = NULL;
char *AlienGoalClassName = NULL;
char *AlienGoalPickupName = NULL;
gentity_t *InvasionGoal[4] = { NULL };

static int GetDroppedGoal = 0;
static char *FinishedToSend = NULL;

static gentity_t *Eggs[MAX_EGGS];
static int TeamEgg = 0, NumEgg = MAX_EGGS - 1, NbTeamEgg = 0, EggLeft = 0;


// ==============================================
// 	Destroy All Eggs
// ==============================================

void Inv_SetDestroyableEgg(gentity_t *egg)
{
	int Team = egg->count / 100;

	assert(egg->chain == NULL);

	if (Team >= MAX_EGGS
		|| NumEgg <= Team)
	{
		G_Error("Team for Egg is too high (Number = %d => Team = %d; NumEgg = %d; MaxEggs = %d)",
					egg->count, Team, NumEgg, MAX_EGGS);
	}

	if (Eggs[Team])
	{
		if (Eggs[Team]->count < egg->count)
		{
			gentity_t *list = Eggs[Team];

			Eggs[NumEgg] = egg;

			while (1)
			{
				if (list->chain == NULL)
				{
					list->chain = egg;
					break;
				}

				if (list->chain->count > egg->count)
				{
					egg->chain = list->chain;
					list->chain = egg;
					break;
				}

				list = list->chain;
			}
		}
		else
		{
			Eggs[NumEgg] = Eggs[Team];
			Eggs[Team] = egg;
			egg->chain = Eggs[NumEgg];
		}

		--NumEgg;
	}
	else
	{
		Eggs[Team] = egg;
		++NbTeamEgg;
		if (Team > TeamEgg)
			TeamEgg = Team;
	}

	if (EggLeft >= MAX_EGGS)
	{
		G_Error("Too much Eggs in the map !!! (Max = %d)", MAX_EGGS);
	}

	++EggLeft;
}


gentity_t *Inv_1stEggInTeam(int Team)
{
	gentity_t *list = Eggs[Team];

	assert((uint) Team < MAX_EGGS);

	while (list)
	{
		if (list->health > 0)
			return list;

		list = list->chain;
	}

	return NULL;
}


void Inv_ComputeDestroyGoals(void)
{
	int i, j, n, l;
	char s[MAX_STRING_CHARS], *AddS;
	vec3_t Origin;

	l = 0;

	for (i = 0; i <= TeamEgg; ++i)
	{
		gentity_t *list = Eggs[i];
		qboolean Alive = qfalse;

		if (list == NULL)
			continue;

		Origin[0] = Origin[1] = Origin[2] = 0;
		n = 0;

		do
		{
			for (j = 0; j < 3; ++j)
				Origin[j] += list->r.currentOrigin[j];
			++n;

			if (list->health > 0)
				Alive = qtrue;

			list = list->chain;
		}
		while (list);

		if (!Alive)
			continue;

		for (j = 0; j < 3; ++j)
		{
			AddS = Inv_StringFromNum((int)(Origin[j] / n), 3);

			if (l +  3 > sizeof(s))
				break;

			strcpy(s + l, AddS);
			l += 3;
		}
	}

	trap_SetConfigstring(CS_INVGOALPOS, s);
}


void Inv_EggDie(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath)
{
	int Team = self->count / 100;

	assert(attacker != NULL);

	GibEntity(self, attacker->s.number);

	G_AddEvent(self, EV_GIB_EGG, attacker->s.number);
	self->takedamage = qfalse;
	self->s.eType = ET_INVISIBLE;
	self->r.contents = 0;

	--EggLeft;

	assert(EggLeft >= 0);

	if (!EggLeft)
	{
		if (level.Period == e_Period_WaitPlayers)
		{
			int i;

			for (i = 0; i < MAX_EGGS; ++i)
			{
				if (Eggs[i] == NULL)
					continue;

				Eggs[i]->takedamage = qtrue;
				Eggs[i]->s.eType = ET_ITEM;
				Eggs[i]->r.contents = CONTENTS_SOLID;
				Eggs[i]->health = 50;

				G_AddEvent(Eggs[i], EV_ITEM_RESPAWN, 0);

				++EggLeft;
			}

			Inv_ComputeDestroyGoals();
		}
		else
			InvasionWin(level.MarineTeam, qfalse);
	}

	if (Inv_1stEggInTeam(Team) == NULL)
	{
		gentity_t *te = te = G_TempEntity(self->r.currentOrigin, EV_DECON_INPROGRESS);
		te->r.svFlags |= SVF_BROADCAST;
		te->s.generic1 = Team;
		te->s.eventParm = meansOfDeath;

		if (attacker != NULL)
		{
			te->s.otherEntityNum2 = attacker->s.number;
			AddScore(attacker, self->r.currentOrigin, 3);
		}
		else
			te->s.otherEntityNum2 = -1;

		if (EggLeft)
		{
			Inv_ComputeDestroyGoals();
			Inv_SendServerCommand(-1, va("cp \""S_COLOR_RED"Zone "S_COLOR_WHITE"%d"S_COLOR_RED" decontaminated\n", Team));
		}
	}
}


gentity_t *Inv_GetClosestEgg(bot_state_t *bs)
{
	gentity_t *list, *Current = g_entities + bs->teamgoal.entitynum;
	float MinDist = -1, Dist;
	vec3_t Delta;
	int i;

	if (bs->attackaway_time >= FloatTime())
		return NULL;

	bs->attackaway_time = FloatTime() + 1.0f;

	for (i = 0; i <= TeamEgg; ++i)
	{
		list = Inv_1stEggInTeam(i);

		if (list == NULL)
			continue;

		VectorSubtract(list->r.currentOrigin, bs->origin, Delta);
		Dist = VectorLengthSquared(Delta);
		if (Dist < MinDist || MinDist == -1)
		{
			MinDist = Dist;
			Current = list;
		}
	}

	assert(Current != NULL || level.Period != e_Period_Playing);

	if (Current == NULL)
		return NULL;

	list = Current->chain;

	while (list)
	{
		if (list->health > 0)
		{
			VectorSubtract(list->r.currentOrigin, bs->origin, Delta);
			Dist = VectorLengthSquared(Delta);
			if (Dist < MinDist)
			{
				MinDist = Dist;
				Current = list;
			}
		}

		list = list->chain;
	}

	return Current;
}


void Inv_GetNearestDestroyGoal(bot_state_t *bs)
{
	gentity_t *list, *Current = g_entities + bs->teamgoal.entitynum;
	float MinDist = -1, Dist;
	vec3_t Delta;
	int i, AreaNum;

	assert((uint) bs->teamgoal.entitynum < MAX_GENTITIES);

	if (!Q_stricmp(Current->classname, "team_CTF_alienegg"))
	{
		list = Inv_1stEggInTeam(Current->count / 100);
		Current = NULL;

		while (list)
		{
			if (list->health > 0)
			{
				VectorSubtract(list->r.currentOrigin, bs->origin, Delta);
				Dist = VectorLengthSquared(Delta);
				if (Dist < MinDist || MinDist == -1)
				{
					MinDist = Dist;
					Current = list;
				}
			}

			list = list->chain;
		}
	}
	else
		Current = NULL;

	if (Current == NULL)
	{
		for (i = 0; i <= TeamEgg; ++i)
		{
			list = Inv_1stEggInTeam(i);

			if (list == NULL)
				continue;

			VectorCopy(list->r.currentOrigin, Delta);

			Delta[2] += list->r.maxs[2] + 2;
			AreaNum = BotPointAreaNum(Delta);

			assert(AreaNum);
			if (!AreaNum)
				continue;

			Dist = trap_AAS_AreaTravelTimeToGoalArea(bs->areanum, bs->origin, AreaNum, TFL_DEFAULT);

			if (Dist < MinDist || MinDist == -1)
			{
				MinDist = Dist;
				Current = list;
			}
		}
	}

	assert(Current != NULL || level.Period != e_Period_Playing);

	if (Current != NULL)
	{
		VectorCopy(Current->r.currentOrigin, bs->teamgoal.origin);
		VectorCopy(Current->r.mins, bs->teamgoal.mins);
		VectorCopy(Current->r.maxs, bs->teamgoal.maxs);

		bs->teamgoal.origin[2] += bs->teamgoal.maxs[2] + 2;
		bs->teamgoal.areanum = BotPointAreaNum(bs->teamgoal.origin);
		bs->teamgoal.origin[2] -= 10;	//Too: 10 coz of BotCode who looks 10 unit up of the object

		bs->teamgoal.entitynum = Current->s.number;
		bs->teamgoal.number = bs->teamgoal.entitynum + 2;
		bs->teamgoal.flags = GFL_ITEM;
		bs->teamgoal.iteminfo = 0;
	}
}


void Inv_DestroyInit(void)
{
	//int i;

	Inv_ComputeDestroyGoals();

	/*for (i = 0; i < MAX_EGGS; ++i)
	{
		if (Eggs[i] != NULL)
			Eggs[i]->r.contents = CONTENTS_SOLID;
	}*/
}


// ===========================================================
// * Game Rules
// ===========================================================

void InvasionCheckAllEntities(void)
{
	int i;
	gentity_t *ent = g_entities;

	if (g_gametype.integer == GT_INVASION || g_gametype.integer == GT_DESTROY)
	{
		MarineGoalClassName = "team_CTF_AlienEgg";
		MarineGoalPickupName = "Alien Egg";
		AlienGoalClassName = "team_CTF_teleporter";
		AlienGoalPickupName = "Teleporter";
	}
	else
	{
		MarineGoalClassName = "team_CTF_blueflag";
		MarineGoalPickupName = "Blue Flag";
		AlienGoalClassName = "team_CTF_redflag";
		AlienGoalPickupName = "Red Flag";
	}

	for (i = 0; i < MAX_GENTITIES; ++i, ++ent)
	{
		if (ent->client != NULL || ent->inuse == qfalse)// || ent->neverFree == qtrue)
			continue;

		if (ent->item != NULL)
		{
			if (ent->item->pickup_name && !Q_stricmp(ent->item->pickup_name, "Alien Egg"))
			{
				MarineGoalClassName = "team_CTF_AlienEgg";
				MarineGoalPickupName = "Alien Egg";
			}

			if (ent->item->pickup_name && !Q_stricmp(ent->item->pickup_name, "Teleporter"))
			{
				AlienGoalClassName = "team_CTF_teleporter";
				AlienGoalPickupName = "Teleporter";
			}

			if (ent->item->giType == IT_TEAM)				// Get Red & Blue Flags
			{
				if (ent->item->pickup_name &&
					(!Q_stricmp(ent->item->pickup_name, "Alien Egg") || !Q_stricmp(ent->item->pickup_name, "Blue Flag")))
				{
					InvasionGoal[0] = ent;
				}

				continue;
			}
		}
	}
}


void InvasionStart(int Restart)
{
	//if (Restart == 0)
	//	printf("**********************************************************\n");

	//printf("\nRestart = %d\n", Restart);

	qboolean Init = qfalse;

	level.RoundStartTime = level.time;

	Inv_InitAlphaToNum();

	if (Restart && level.InternalRestart == qtrue)
	{
		int EndTime;

		level.newGame = qfalse;
		level.startTime = level.GameStartTime;

		if (level.Period == e_Period_Playing)
		{
			EndTime = level.RoundStartTime + g_InvRoundTime.integer * 1000;
			if (g_gametype.integer != GT_CTF)
				trap_SetConfigstring(CS_GAMETIMELEFT, va("%i", EndTime));
		}
		else
			Init = qtrue;
	}
	else
	{
		level.warmupTime = -1;
		level.newGame = qtrue;
		level.GameStartTime = level.startTime;

		level.teamScores[TEAM_RED] = level.teamScores[TEAM_BLUE] = 0;
		Init = qtrue;
	}

	if (Init)
	{
		InvasionCheckAllEntities();
		InvasionSetPeriod(e_Period_Selection);
	}

	level.InternalRestart = qfalse;

	GetDroppedGoal = 0;
	FinishedToSend = NULL;
}


void InvasionSetMarineTeam(team_t Team)
{
	level.MarineTeam = Team;
	level.AlienTeam = (Team == TEAM_RED) ? TEAM_BLUE : TEAM_RED;

	InvasionSendInfo();
}


void InvasionSetPeriod(EPeriod Period)
{
	if (g_gametype.integer == GT_DESTROY
		&& ((Period == e_Period_Playing && level.Period == e_Period_Selection)
			|| (Period == e_Period_WaitPlayers && level.Period != e_Period_WaitPlayers)))
	{
		Inv_DestroyInit();
	}

	level.InvasionInfo.Period = level.Period = Period;

	InvasionSendInfo();
}

void SetEverybodyBaseTeam(void)
{
	gclient_t	*cl;
	int i;

	for (i = 0; i < g_maxclients.integer; ++i)
	{
		cl = level.clients + i;

		if (cl->pers.connected != CON_CONNECTED)
			continue;

		if (cl->sess.sessionTeam != cl->sess.BaseTeam)
		{
			cl->sess.sessionTeam = cl->sess.BaseTeam;
			ClientUserinfoChanged(i, qfalse);
		}
	}
}


void SwapEverybodyBaseTeam(void)
{
	gclient_t	*cl;
	int i;

	for (i = 0; i < g_maxclients.integer; ++i)
	{
		//char *st;
		//gentity_t *ent = g_entities + i;

		cl = level.clients + i;

		if (cl->pers.connected != CON_CONNECTED)
			continue;

		if (cl->sess.BaseTeam == TEAM_RED)
			//st = "b";
			cl->sess.BaseTeam = TEAM_BLUE;
		else if (cl->sess.BaseTeam == TEAM_BLUE)
			//st = "r";
			cl->sess.BaseTeam = TEAM_RED;

		//SetTeam(ent, st, NULL, qfalse);
	}
}


void InvasionFinishRound(qboolean Restart)
{
	int Swap = g_InvSwapPeriod.integer;

	if (g_InvSwapPeriod.integer > 30)
	{
		Swap = 30;
		trap_Cvar_Set("Inv_SwapPeriod", "30");
	}
	else if (g_InvSwapPeriod.integer < 0)
	{
		Swap = 0;
		trap_Cvar_Set("Inv_SwapPeriod", "0");
	}

	++level.RoundNum;
	if (!Swap)
	{
		if (level.RoundNum >= 100)									// Reset RoundNumber to don't get to hi numbers
			level.RoundNum = 0;
	}
	else if (!(level.RoundNum % Swap))
	{
		int t;

		//if (level.RoundNum >= 100)									// Reset RoundNumber to don't get to hi numbers
			level.RoundNum = 0;

		SwapEverybodyBaseTeam();
		t = level.teamScores[TEAM_RED];
		level.teamScores[TEAM_RED] = level.teamScores[TEAM_BLUE];
		level.teamScores[TEAM_BLUE] = t;
		CalculateRanks();

		//InvasionSetMarineTeam(level.AlienTeam);
		InvasionSetMarineTeam(TEAM_RED);
	}

	if (Restart == qtrue)
	{
		//Team_ResetFlags();
		level.InternalRestart = qtrue;
		level.restarted = qtrue;
		trap_SendConsoleCommand(EXEC_APPEND, "map_restart 0\n");
	}

	SetEverybodyBaseTeam();
	InvasionSetPeriod(e_Period_Finished);
}


void Inv_WinSound(int Team)
{
	gentity_t	*te;
	vec3_t Origin = { 0, 0, 0 };

	te = G_TempEntity(Origin, EV_GLOBAL_TEAM_SOUND);
	te->r.svFlags |= SVF_BROADCAST;

	if (Team == level.MarineTeam)
		te->s.eventParm = GTS_REDTEAM_TOOK_LEAD;
	else if (Team == level.AlienTeam)
		te->s.eventParm = GTS_BLUETEAM_TOOK_LEAD;
	else
		te->s.eventParm = GTS_TEAMS_ARE_TIED;
}


void InvasionWin(team_t Team, qboolean FlagLost)
{
	if (level.Period >= e_Period_WaitRestart)
		return;

	if (level.Period == e_Period_WaitPlayers)
	{
		Team_ResetFlags();
		return;
	}

	InvasionSetPeriod(e_Period_WaitRestart);
	level.RoundStartTime = level.time;

	if (Team == level.MarineTeam)
	{
		Inv_WinSound(level.MarineTeam);
		Inv_SendServerCommand(-1, "cp \"" S_COLOR_CYAN "Marines win the Round\n");
	}
	else
	{
		if (FlagLost == qtrue)
		{
			Inv_WinSound(0);
			Inv_SendServerCommand(-1, "cp \"" S_COLOR_YELLOW "*** Egg Lost ***\n" S_COLOR_MAGENTA "Aliens win the Round\n");
		}
		else
		{
			Inv_WinSound(level.AlienTeam);
			Inv_SendServerCommand(-1, "cp \"" S_COLOR_MAGENTA "Aliens win the Round\n");
		}
	}

	++level.teamScores[Team];
	CalculateRanks();
	SendScoreboardMessageToAllClients();
}


void InvasionMain(void)
{
	switch (level.Period)
	{
		case e_Period_Playing:
		{
			if (g_InvRoundTime.modificationCount != level.RoundTimeModifCount || g_InvRoundTime.integer < 60)
			{
				int EndTime;

				if (g_InvRoundTime.integer < 60)
					g_InvRoundTime.integer = 60;

				level.RoundTimeModifCount = g_InvRoundTime.modificationCount;
				EndTime = level.RoundStartTime + g_InvRoundTime.integer * 1000;
				trap_SetConfigstring(CS_GAMETIMELEFT, va("%i", EndTime));
			}

			if (level.time - level.RoundStartTime > g_InvRoundTime.integer * 1000)
			{
				InvasionWin(level.AlienTeam, qfalse);
				return;
			}

			if (NbMemberAlive(level.MarineTeam) == 0)
			{
				InvasionWin(level.AlienTeam, qfalse);
			}

			if (NbMemberAlive(level.AlienTeam) == 0)
			{
				InvasionWin(level.MarineTeam, qfalse);
			}

			if (GetDroppedGoal > 0)
			{
				--GetDroppedGoal;
				if (!GetDroppedGoal)
				{
					InvasionSetDroppedGoal(InvasionGoal[0]);
				}
			}

			InvasionUpdateClientSideStat();

			break;
		}
		case e_Period_WaitRestart:
		{
			if (FinishedToSend)
			{
				Inv_SendServerCommand(-1, FinishedToSend);
				FinishedToSend = NULL;
			}

			if (level.time > level.RoundStartTime + 4 * 1000)
			{
				if (g_capturelimit.integer &&
					(level.teamScores[TEAM_RED] >= g_capturelimit.integer ||
					level.teamScores[TEAM_BLUE] >= g_capturelimit.integer))
				{
					//Too: capturelimit hit, do nothing, wait Q3 normal capture limit handling to restart the map
					InvasionSetPeriod(e_Period_Finished);
					return;
				}

				InvasionFinishRound(qtrue);
				return;
			}

			InvasionUpdateClientSideStat();

			break;
		}

		case e_Period_Selection:
			if (!ctf_redflag.areanum)
				ctf_redflag = Inv_GetFlagItem("Red", qfalse);
			if (!ctf_blueflag.areanum)
				ctf_blueflag = Inv_GetFlagItem("Blue", qfalse);
			break;

		case e_Period_Finished: default:
			break;	// do nothing

		case e_Period_WaitPlayers:
		{
			gclient_t	*cl;
			int i;
			char *st;

			for (i = 0; i < g_maxclients.integer; ++i)
			{
				cl = level.clients + i;

				if (cl->pers.connected != CON_CONNECTED)
					continue;

				if (cl->sess.sessionTeam != cl->sess.BaseTeam)
				{
					if (cl->sess.BaseTeam == TEAM_RED)
						st = "r";
					else
						st = "b";

					SetTeam(g_entities + i, st, NULL, qtrue);
				}
			}
		}
	}
}


// ===========================================================
// * Net Comm
// ===========================================================

void Inv_SendServerCommand(int n, char *str)
{
	gclient_t	*cl;
	int i;

	if (n != -1)
	{
		trap_SendServerCommand(n, str);
		return;
	}

	for (i = 0; i < g_maxclients.integer; ++i)
	{
		cl = level.clients + i;

		if (cl->pers.connected != CON_CONNECTED)
			continue;

		trap_SendServerCommand(i, str);
	}
}


void InvasionSendInfo(void)
{
	//Full InvasionInfo structure
	char s[4];

	level.InvasionInfo.gametype = g_gametype.integer;
	level.InvasionInfo.MarineTeam = level.MarineTeam;
	level.InvasionInfo.AlienTeam = level.AlienTeam;

	s[0] = level.InvasionInfo.MarineTeam + '0';
	s[1] = level.InvasionInfo.AlienTeam + '0';
	s[2] = level.Period + '0';
	s[3] = 0;

	trap_SetConfigstring(CS_INVASIONINFO, s);
}

enum
{
	es_UpdateTime	=	500,
};

void InvasionUpdateClientSideStat(void)
{
	static int ClientNum = 0;
	static int LastLoopTime = 0;
	gclient_t *cl, *ToClient;
	gentity_t *ent;
	char s[MAX_STRING_CHARS - 16] = "IS ";

	int i, NbClient, slen, Team;

	if (ClientNum >= g_maxclients.integer)
		ClientNum = g_maxclients.integer - 1;

	i = ClientNum;
	if (i == -1)
	{
		if (level.time - LastLoopTime < es_UpdateTime)
			return;

		LastLoopTime = level.time;
		ClientNum = g_maxclients.integer - 1;
	}

	do									// Look for next connected client
	{
		++i;
		if (i >= g_maxclients.integer)
		{
			if (level.time - LastLoopTime < es_UpdateTime)
			{
				ClientNum = -1;
				return;
			}

			LastLoopTime = level.time;

			i = 0;
		}

		if (i == ClientNum)
			return;

		cl = level.clients + i;
	}
	while (cl->pers.connected != CON_CONNECTED);
		//|| (cl->sess.sessionTeam == TEAM_SPECTATOR
		//	&& cl->sess.spectatorState == SPECTATOR_SCOREBOARD));

	if (!cl->sess.EnterTime || level.time - cl->sess.EnterTime < 5 * 1000)
		return;


	ent = g_entities + i;
	ClientNum = i;

	ToClient = cl;
	NbClient = 0;
	slen = strlen(s);

	//InvStartTrapCmd("ICS");
	//InvAddTrapCmdByte(1);

	Team = ToClient->sess.sessionTeam;

	/*if (ToClient->sess.spectatorState == SPECTATOR_FOLLOW)
	{
		int		clientNum = ToClient->sess.spectatorClient;

		if (clientNum == -1)
			clientNum = level.follow1;
		else if (clientNum == -2)
			clientNum = level.follow2;
		if (clientNum >= 0)
			Team = level.clients[clientNum].sess.sessionTeam;
	}*/

	for (i = 0; i < g_maxclients.integer; ++i)
	{
		char *AddS;
		int j, h, a;

		if (i == ClientNum)
			continue;

		cl = level.clients + i;

		if (cl->pers.connected != CON_CONNECTED || cl->sess.sessionTeam == TEAM_SPECTATOR)
			continue;

		if (Team == level.MarineTeam && cl->sess.sessionTeam != Team)	// Opposite Team; don't need info
			continue;

		h = cl->ps.stats[STAT_HEALTH];
		a = cl->ps.stats[STAT_ARMOR];
		if (h < 0)
			h = 0;
		if (a < 0)
			a = 0;

		if (Team == level.AlienTeam)
			a = ToClient->pers.LastLife[i][1];

		if (ToClient->pers.LastLife[i][0] == (uchar) h
			&& ToClient->pers.LastLife[i][1] == (uchar) a)
			continue;

		if (abs(ToClient->pers.LastLife[i][0] - h) >= 5
			|| abs(ToClient->pers.LastLife[i][1] - a) >= 5
			|| !h)
		{
			ToClient->pers.LastLife[i][0] = (uchar) h;
			ToClient->pers.LastLife[i][1] = (uchar) a;

			//InvAddTrapCmdValue(i + (h << 7) + (a << (7 + 12)));		// MaxClients is 128, so need 7 bits for i

			if (a > 1023)
				a = 1023;

			assert(i < 64);

			if (Team == level.AlienTeam)
			{
				if (h > 1023)
					h = 1023;
				AddS = Inv_StringFromNum(i + (h << es_InvCS_Num), 3);
			}
			else
			{
				if (h > 255)
					h = 255;
				AddS = Inv_StringFromNum(i + ((h + (a << es_InvCS_Health)) << es_InvCS_Num), 4);
			}

			//AddS = va(" %d %d %d", i, h, a);

			j = strlen(AddS);
			if (slen +  j > sizeof(s))
				break;

			strcpy(s + slen, AddS);
			slen += j;
			++NbClient;
		}
	}

	if (NbClient > 0)
	{
		//InvChangeTrapCmdByte(0, (byte) NbClient);
		//InvSendTrapCmd(ClientNum);
		Inv_SendServerCommand(ClientNum, s);
	}
}


/*void InvSendTrapCmd(int ClientNum)
{
	printf("Send TrapCmd (size = %d) to Client : %d (buf = '%s')\n", InvGetTrapCmdLgt(), ClientNum, InvGetTrapCmdBuffer());
	fflush(stdout);
	trap_SendServerCommand(ClientNum, InvGetTrapCmdBuffer());
}*/


// ==============================================
// * Game death & respawn handling
// ==============================================

int NbMemberAlive(team_t Team)
{
	gclient_t	*cl;
	int i;
	int n = 0;


	for (i = 0; i < g_maxclients.integer; ++i)
	{
		cl = level.clients + i;

		if (cl->pers.connected != CON_CONNECTED)
			continue;

		//if (cl->sess.sessionTeam == Team && cl->ps.stats[STAT_HEALTH] > 0)
		if (cl->sess.sessionTeam == Team
			&& (cl->pers.NbLife > 0 || cl->pers.NbLife == -1))
		{
			++n;
		}
	}

	return n;
}



void InvasionSetDroppedGoal(gentity_t *Drop)
{
	VectorCopy(Drop->r.currentOrigin, ctf_blueflag.origin);
	ctf_blueflag.areanum = BotPointAreaNum(ctf_blueflag.origin);
	VectorCopy(Drop->r.mins, ctf_blueflag.mins);
	VectorCopy(Drop->r.maxs, ctf_blueflag.maxs);

	ctf_blueflag.entitynum = Drop->s.number;
	ctf_blueflag.number = ctf_blueflag.entitynum + 2;
	ctf_blueflag.flags = GFL_ITEM | GFL_DROPPED;
	ctf_blueflag.iteminfo = 0;

	if (Drop->s.pos.trType != TR_STATIONARY)
	{
		GetDroppedGoal = trap_Cvar_VariableValue("sv_fps") * 0.5f;
		if (GetDroppedGoal < 5)
			GetDroppedGoal = 5;

		InvasionGoal[0] = Drop;
	}
	else
	{
		gentity_t *te = G_TempEntity(InvasionGoal[0]->s.pos.trBase, EV_GLOBAL_TEAM_SOUND);
		te->s.eventParm = GTS_DROPPED_EGG;
		te->r.svFlags |= SVF_BROADCAST;
	}
}


void InvasionPlayerDie(gentity_t *self)
{
	if (g_gametype.integer == GT_INVASION || g_gametype.integer == GT_DESTROY)
	{
		gclient_t *client = self->client;

		if (client->pers.NbLife != -1)
		{
			--client->pers.NbLife;
			assert(client->pers.NbLife >= 0);
		}

		if (client->sess.sessionTeam == level.MarineTeam)
			memset(client->pers.LastLife, 0, sizeof(client->pers.LastLife));

		if (level.Period == e_Period_Playing &&
			InvasionNormalRespawn(client) == qfalse)	// Check if any marine alive
		{
			if (client->sess.sessionTeam == level.MarineTeam)
			{
				if (NbMemberAlive(level.MarineTeam) == 0)
					InvasionWin(level.AlienTeam, qfalse);
			}
			else
			{
				if (NbMemberAlive(level.AlienTeam) == 0)
					InvasionWin(level.MarineTeam, qfalse);
			}
		}
	}
}


qboolean InvasionRespawn(gentity_t *self)
{
	if (self->client == NULL)
		return qfalse;

	if ((g_gametype.integer == GT_INVASION || g_gametype.integer == GT_DESTROY) &&
		InvasionNormalRespawn(self->client) == qfalse)					// Handle Marine death
	{
		char *st;

		if (level.Period >= e_Period_Playing)
		{
			if (g_InvAnarchy.integer && level.Period == e_Period_Playing)
			{
				if (level.MarineTeam == TEAM_RED)
					st = "b";
				else
					st = "r";

				CopyToBodyQue(self);
				self->client->ps.persistant[PERS_CLASS] = self->client->pers.AlienRace;		// ToCheck : it's not very smart to do this here !
				self->client->ps.stats[STAT_HEALTH] = 1;		//Too: dirty .... :''(

				SetTeam(self, st, NULL, qtrue);

				InvasionInitAlien(self->s.clientNum);
			}
			else
				SetTeam(self, "s", NULL, qtrue);
		}
		else
		{
			if (level.MarineTeam == TEAM_RED)
				st = "r";
			else
				st = "b";

			SetTeam(self, st, NULL, qtrue);
		}

		return qtrue;
	}

	return qfalse;
}


qboolean InvasionNormalRespawn(gclient_t *client)
{
	if (client->pers.NbLife > 0 || client->pers.NbLife == -1)
		return qtrue;

	return qfalse;

	/*if (client->sess.sessionTeam == level.MarineTeam)
		return qfalse;

	return qtrue;*/
}


void InvasionGiveWeapon(int clientNum, weapon_t Weapon, int Ammo)
{
	gclient_t *client = level.clients + clientNum;
	gentity_t *ent = g_entities + clientNum;
	gitem_t *item = BG_FindItemForWeapon(Weapon);

	client->ps.stats[STAT_WEAPONS] |= (1 << Weapon);
	client->ps.ammo[Weapon] = Ammo;
	G_AddEvent(ent, EV_ITEM_PICKUP2, item - bg_itemlist);
}


void InvasionInitMarine(int clientNum, int weapon)
{
	gclient_t *client = level.clients + clientNum;

	InvasionGiveWeapon(clientNum, WP_MACHINEGUN, InvMarineWeaponAmmunitions[WP_MACHINEGUN]);

	client->ps.stats[STAT_ARMOR] = InvMarineArmorMax[client->pers.ArmorType] * client->ps.stats[STAT_MAX_HEALTH] / 100;
	client->ps.persistant[PERS_CLASS] = client->pers.ArmorType + (weapon << e_Class_MarineWeaponDec);

	switch (weapon)
	{
		case e_Selection_ChainGun:
			weapon = WP_INVCHAINGUN;
			break;

		case e_Selection_ShotGun:
			weapon = WP_SHOTGUN;
			break;

		case e_Selection_FlameThrower:
			weapon = WP_FLAMETHROWER;
			break;

		case e_Selection_Bfg:
			weapon = WP_BFG;
			break;

		case e_Selection_Sniper:
			weapon = WP_RAILGUN;
			break;
	}

	InvasionGiveWeapon(clientNum, WP_GAUNTLET, -1);
	InvasionGiveWeapon(clientNum, weapon, InvMarineWeaponAmmunitions[weapon]);
}


void InvasionInitAlien(int clientNum)
{
	gclient_t *client = level.clients + clientNum;
	gentity_t *ent = g_entities + clientNum;

	switch (client->pers.AlienRace)
	{
		case e_Selection_Trooper:
			InvasionGiveWeapon(clientNum, WP_GAUNTLET, -1);
			InvasionGiveWeapon(clientNum, WP_MACHINEGUN, 175);
			ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] * 2;
			client->ps.ammo[WP_GRENADE_LAUNCHER] = 2;
			break;

		case e_Selection_Bloomy:
			InvasionGiveWeapon(clientNum, WP_PLASMAGUN, 30);
			ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] * 1.5;
			break;

		case e_Selection_Rad:
			InvasionGiveWeapon(clientNum, WP_LIGHTNING, 50);
			break;

		case e_Selection_Xenomorph:
			InvasionGiveWeapon(clientNum, WP_GAUNTLET, -1);
			break;
	}

	client->ps.persistant[PERS_CLASS] = client->pers.AlienRace;
	client->RadiationTime = 0;
}


// ==============================================
// * Models selection
// ==============================================

char **InvasionCanUseModel(gclient_t *Client, team_t Team, char *Model)
{
	char **s = (Team != level.MarineTeam) ? AlienModel :
				((Client->sess.Gender == GENDER_FEMALE) ? FemaleMarineModel : MaleMarineModel);
	char work[MAX_QPATH];
	int i;

	strcpy(work, Model);
	i = 0;
	while (work[i])
	{
		if (work[i] == '\\' || work[i] == '/')
		{
			work[i] = 0;
			break;
		}
		++i;
	}

	while (*s)
	{
		if (!Q_stricmp(work, *s))
			return s;

		++s;
	}

	return NULL;
}

char *InvasionGetDefaultModel(gclient_t *Client, team_t Team)
{
	char **s = (Team == level.AlienTeam) ? AlienModel :
				((Client->sess.Gender == GENDER_FEMALE) ? FemaleMarineModel : MaleMarineModel);

	if (*s == NULL)
		return "";

	return *s;
}

void InvasionCheckModel(gclient_t *Client, team_t Team, char *Model)
{
	if (Team == level.MarineTeam)
	{
		if (Client->sess.Gender == GENDER_FEMALE)
			strcpy(Model, FemaleMarineModel[Client->pers.ArmorType]);
		else
			strcpy(Model, MaleMarineModel[Client->pers.ArmorType]);
	}
	else if (Team == level.AlienTeam)
	{
		strcpy(Model, AlienModel[Client->pers.AlienRace]);
	}
	else //if (InvasionCanUseModel(Client, Team, Model) == NULL)
		strcpy(Model, InvasionGetDefaultModel(Client, Team));
}


/*==================== EOF because of buggy VSS ===========*/