// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "g_Invasion.h"

#include "botlib.h"		//bot lib interface
#include "be_aas.h"
#include "be_ea.h"
#include "be_ai_char.h"
#include "be_ai_chat.h"
#include "be_ai_gen.h"
#include "be_ai_goal.h"
#include "be_ai_move.h"
#include "be_ai_weap.h"
#include "ai_main.h"


/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
extern bot_state_t *botstates[MAX_CLIENTS];


void G_WriteClientSessionData(gclient_t *client)
{
	const char	*s;
	const char	*var;

	if (!client->sess.TeamLeader[0])
		strcpy(client->sess.TeamLeader, "0123");

	s = va("%i %i %i %i %i %i %i %i %i %i %i %i %i %i %s",
		client->sess.sessionTeam,
		client->sess.spectatorTime,
		client->sess.spectatorState,
		client->sess.spectatorClient,
		client->sess.wins,
		client->sess.losses,
		client->sess.Score,
		client->sess.BaseTeam,
		client->sess.Gender,
		client->sess.EnterTime,
		client->sess.Played,
		client->sess.cs,
		client->sess.inactivityTime - level.time,
		client->sess.teamLeader,
		client->sess.TeamLeader
		);

	var = va("session%i", client - level.clients);

	trap_Cvar_Set(var, s);
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData(gclient_t *client)
{
	char	s[MAX_STRING_CHARS];
	const char	*var;

	// bk001205 - format
	int teamLeader;
	int spectatorState;
	int sessionTeam;
	int baseTeam;

	var = va("session%i", client - level.clients);
	trap_Cvar_VariableStringBuffer(var, s, sizeof(s));

	sscanf(s, "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %s",
		&sessionTeam,                 // bk010221 - format
		&client->sess.spectatorTime,
		&spectatorState,              // bk010221 - format
		&client->sess.spectatorClient,
		&client->sess.wins,
		&client->sess.losses,
		&client->sess.Score,
		&baseTeam,
		&client->sess.Gender,
		&client->sess.EnterTime,
		&client->sess.Played,
		&client->sess.cs,
		&client->sess.inactivityTime,
		&teamLeader,                   // bk010221 - format
		client->sess.TeamLeader
		);

	// bk001205 - format issues
	client->sess.sessionTeam = (team_t)sessionTeam;
	client->sess.BaseTeam = (team_t) baseTeam;
	client->sess.spectatorState = (spectatorState_t)spectatorState;
	client->sess.teamLeader = (qboolean)teamLeader;

	client->sess.inactivityTime += level.time;

	if (level.newGame == qfalse)
		client->ps.persistant[PERS_SCORE] = client->sess.Score;
	else
		client->sess.Score = 0;
}


/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData(gclient_t *client, char *userinfo)
{
	clientSession_t	*sess;
	const char		*value;

	sess = &client->sess;

	// initial team determination
	if (g_gametype.integer >= GT_TEAM)
	{
		if (g_teamAutoJoin.integer)
		{
			sess->sessionTeam = PickTeam(-1);
			BroadcastTeamChange(client, -1);
		}
		else
		{
			// always spawn as spectator in team games
			sess->sessionTeam = TEAM_SPECTATOR;
		}
	}
	else
	{
		value = Info_ValueForKey(userinfo, "team");
		if (value[0] == 's')
		{
			// a willing spectator, not a waiting-in-line
			sess->sessionTeam = TEAM_SPECTATOR;
		}
		else
		{
			switch (g_gametype.integer)
			{
			default:
			case GT_FFA:
			case GT_SINGLE_PLAYER:
				if (g_maxGameClients.integer > 0 &&
					level.numNonSpectatorClients >= g_maxGameClients.integer)
					{
					sess->sessionTeam = TEAM_SPECTATOR;
				}
				else
				{
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			case GT_TOURNAMENT:
				// if the game is full, go into a waiting mode
				if (level.numNonSpectatorClients >= 2)
				{
					sess->sessionTeam = TEAM_SPECTATOR;
				}
				else
				{
					sess->sessionTeam = TEAM_FREE;
				}
				break;
			}
		}
	}

	sess->BaseTeam = sess->sessionTeam;
	sess->Gender = GENDER_NEUTER;
	sess->spectatorState = SPECTATOR_FREE;
	sess->spectatorTime = level.time;
	client->sess.EnterTime = 0;
	client->sess.Played = 0;
	strcpy(client->sess.TeamLeader, "0123");
	sess->inactivityTime = level.time + g_inactivity.integer * 1000;
	sess->cs = 1;

	G_WriteClientSessionData(client);
}


/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession(void)
{
	char	s[MAX_STRING_CHARS];
	int			gt;

	trap_Cvar_VariableStringBuffer("session", s, sizeof(s));

	sscanf(s, "%i", &gt);
	//gt = atoi(s);

	// if the gametype changed since the last session, don't use any
	// client sessions
	if (g_gametype.integer != gt)
	{
		level.newSession = qtrue;
		G_Printf("Gametype changed, clearing session data.\n");

		InvasionSetMarineTeam(TEAM_RED);
	}
	else
	{
		sscanf(s, "%i %i %i %i %i %i %i %i",
			&gt,
			&level.teamScores[TEAM_RED],
			&level.teamScores[TEAM_BLUE],
			&level.GameStartTime,
			&level.InternalRestart,
			&level.MarineTeam,
			&level.RoundNum,
			&level.Period);

		//InvasionSetMarineTeam(level.MarineTeam);
		InvasionSetMarineTeam(TEAM_RED);
	}
}

/*
==================
G_WriteSessionData

==================
*/
void G_WriteSessionData(void)
{
	int		i;
	const char	*s;

	s = va("%i %i %i %i %i %i %i %i",
		g_gametype.integer,
		level.teamScores[TEAM_RED],
		level.teamScores[TEAM_BLUE],
		level.GameStartTime,
		level.InternalRestart,
		level.MarineTeam,
		level.RoundNum,
		level.Period);

	trap_Cvar_Set("session", s);

	for (i = 0; i < level.maxclients; i++)
	{
		if (level.clients[i].pers.connected == CON_CONNECTED)
		{
			G_WriteClientSessionData(&level.clients[i]);
		}
	}
}

/*==================== EOF because of buggy VSS ===========*/