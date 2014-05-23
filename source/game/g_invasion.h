//
// Copyright (C) 2000 ManuTOO ! (quite impressive, isn't it ? :o)))
//

//#ifndef __G_INVASION_H
//#define __G_INVASION_H

extern vmCvar_t	g_InvSwapPeriod;
extern vmCvar_t	g_InvRoundTime;
extern vmCvar_t	g_InvWarmUp;
extern vmCvar_t	g_InvAnarchy;
extern vmCvar_t	g_InvEggHealth;
extern vmCvar_t	g_InvAutoMode;

void InvasionStart(int Restart);
void InvasionMain(void);
void InvasionFinishRound(qboolean Restart);
void InvasionSetMarineTeam(team_t Team);
void InvasionSendInfo(void);


void InvasionPlayerDie(gentity_t *self);
void InvasionWin(team_t Team, qboolean FlagLost);
qboolean InvasionNormalRespawn(gclient_t *client);		// return qfalse if this team members should go spectator or in opposite team on death
qboolean InvasionRespawn(gentity_t *self);				// return qtrue if respawn done, else qfalse (so Invasion don't handle this entities respawn)
void InvasionInitMarine(int clientNum, int weapon);
void InvasionInitAlien(int clientNum);
void InvasionGiveWeapon(int clientNum, weapon_t Weapon, int Ammo);
void InvasionSetDroppedGoal(gentity_t *Drop);


char **InvasionCanUseModel(gclient_t *Client, team_t Team, char *Model);
char *InvasionGetDefaultModel(gclient_t *Client, team_t Team);
void InvasionCheckModel(gclient_t *Client, team_t Team, char *Model);


void InvasionSetPeriod(EPeriod Period);

extern float InvArmorProtection[e_Selection_MaxArmor];
extern float InvArmorSpeed[e_Selection_MaxArmor];
extern float InvAlienSpeed[e_Selection_MaxRace];
extern int InvMarineArmorMax[e_Selection_MaxArmor];
extern int InvMarineWeaponAmmunitions[WP_NUM_WEAPONS];

extern char *MarineGoalClassName;
extern char *MarineGoalPickupName;
extern char *AlienGoalClassName;
extern char *AlienGoalPickupName;
extern gentity_t *InvasionGoal[4];

//void InvSendTrapCmd(int ClientNum);

void Inv_SendServerCommand(int n, char *str);

int Inv_GetGoalPosition(gentity_t *ent);
void Inv_SendRadar(gclient_t *Client);

// Destroy All Eggs
void Inv_SetDestroyableEgg(gentity_t *egg);
void Inv_EggDie(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath);

//#endif	// __G_INVASION_H



/*==================== EOF because of buggy VSS ===========*/