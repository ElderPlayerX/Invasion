// Copyright (C) 1999-2000 Id Software, Inc.
//
//
// ui_team.c
//

#include "ui_local.h"

extern void InitMenuText(menutext_s *Menu, int type, unsigned flags, int x, int y, int id,
	void (*callback)(void *self, int event), char *string, float *color, int style);

#define TEAMMAIN_FRAME	"menu/art/cut_frame"

#define ID_JOINRED				100
#define ID_JOINREDFEMALE		104
#define ID_JOINBLUE				101
#define ID_JOINBLUEFEMALE		106
#define ID_JOINGAME				102
#define ID_SPECTATE				103
#define ID_JOINAUTO				107


typedef struct
{
	menuframework_s	menu;
	menubitmap_s	frame;
	menutext_s		joinred;
	menutext_s		joinredfemale;
	menutext_s		joinblue;
	menutext_s		joinbluefemale;
	menutext_s		joinauto;
	menutext_s		joingame;
	menutext_s		spectate;
}
teammain_t;

static teammain_t	s_teammain;

// bk001204 - unused
//static menuframework_s	s_teammain_menu;
//static menuaction_s		s_teammain_orders;
//static menuaction_s		s_teammain_voice;
//static menuaction_s		s_teammain_joinred;
//static menuaction_s		s_teammain_joinblue;
//static menuaction_s		s_teammain_joingame;
//static menuaction_s		s_teammain_spectate;

static char Join[2][32];


/*
===============
TeamMain_MenuEvent
===============
*/
static void TeamMain_MenuEvent(void* ptr, int event)
{
	char	info[MAX_INFO_STRING];
	int	gametype;

	trap_GetConfigString(CS_SERVERINFO, info, MAX_INFO_STRING);
	gametype = atoi(Info_ValueForKey(info,"g_gametype"));

	if (event != QM_ACTIVATED)
	{
		return;
	}

	switch (((menucommon_s*)ptr)->id)
	{
	case ID_JOINRED:
		trap_Cmd_ExecuteText(EXEC_APPEND, "cmd team red male\n");		//Too:
		UI_ForceMenuOff();
		break;

	case ID_JOINREDFEMALE:
		trap_Cmd_ExecuteText(EXEC_APPEND, "cmd team red female\n");
		UI_ForceMenuOff();
		break;

	case ID_JOINBLUE:
		trap_Cmd_ExecuteText(EXEC_APPEND, "cmd team blue male\n");		//Too:
		UI_ForceMenuOff();
		break;

	case ID_JOINAUTO:
		trap_Cmd_ExecuteText(EXEC_APPEND, "cmd team auto male\n");
		UI_ForceMenuOff();
		break;

	case ID_JOINBLUEFEMALE:
		trap_Cmd_ExecuteText(EXEC_APPEND, "cmd team blue female\n");
		UI_ForceMenuOff();
		break;

	case ID_JOINGAME:
		trap_Cmd_ExecuteText(EXEC_APPEND, "cmd team free\n");
		UI_ForceMenuOff();
		break;

	case ID_SPECTATE:
		trap_Cmd_ExecuteText(EXEC_APPEND, "cmd team spectator\n");
		UI_ForceMenuOff();
		break;
	}
}


/*
===============
TeamMain_MenuInit
===============
*/
void TeamMain_MenuInit(int NbRed, int NbBlue)
{
	int	y;
	char	info[MAX_INFO_STRING];
	int	gametype;
	char	*s;

	trap_GetConfigString(CS_SERVERINFO, info, MAX_INFO_STRING);
	gametype = atoi(Info_ValueForKey(info,"g_gametype"));

	memset(&s_teammain, 0, sizeof(s_teammain));

	TeamMain_Cache();

	s_teammain.menu.wrapAround = qtrue;
	s_teammain.menu.fullscreen = qfalse;

	s_teammain.frame.generic.type   = MTYPE_BITMAP;
	s_teammain.frame.generic.flags	= QMF_INACTIVE;
	s_teammain.frame.generic.name   = TEAMMAIN_FRAME;
	s_teammain.frame.generic.x		= 142;
	s_teammain.frame.generic.y		= 118;
	s_teammain.frame.width			= 359;
	s_teammain.frame.height			= 256;

	y = 194;

	if (NbRed != -1)		//Too:
	{
		strcpy(Join[0], va("Marines - %d player(s)", NbRed));
		s = Join[0];
	}
	else
		s = "JOIN MARINES";// (as male)";

	InitMenuText(&s_teammain.joinred, MTYPE_PTEXT, QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS,
					320, y, ID_JOINRED, TeamMain_MenuEvent, s,
					colorRed, UI_CENTER|UI_SMALLFONT);

	/*{
		y += 20;
		InitMenuText(&s_teammain.joinredfemale, MTYPE_PTEXT, QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS,
						320, y, ID_JOINREDFEMALE, TeamMain_MenuEvent, "JOIN RED (as female)",
						colorRed, UI_CENTER|UI_SMALLFONT);
	}*/

	if (NbBlue != -1)		//Too:
	{
		strcpy(Join[1], va("Aliens - %d player(s)", NbBlue));
		s = Join[1];
	}
	else
		s = "JOIN ALIENS";// (as male)";

	y += 20;
	InitMenuText(&s_teammain.joinblue, MTYPE_PTEXT, QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS,
					320, y, ID_JOINBLUE, TeamMain_MenuEvent, s,
					colorRed, UI_CENTER|UI_SMALLFONT);

	y += 20;
	InitMenuText(&s_teammain.joinauto, MTYPE_PTEXT, QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS,
					320, y, ID_JOINAUTO, TeamMain_MenuEvent, "AUTO SELECT",
					colorRed, UI_CENTER|UI_SMALLFONT);

	/*{
		y += 20;
		InitMenuText(&s_teammain.joinbluefemale, MTYPE_PTEXT, QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS,
						320, y, ID_JOINBLUEFEMALE, TeamMain_MenuEvent, "JOIN BLUE (as female)",
						colorRed, UI_CENTER|UI_SMALLFONT);
	}*/

	y += 20;
	InitMenuText(&s_teammain.joingame, MTYPE_PTEXT, QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS,
					320, y, ID_JOINGAME, TeamMain_MenuEvent, "JOIN GAME",
					colorRed, UI_CENTER|UI_SMALLFONT);

	y += 20;
	InitMenuText(&s_teammain.spectate, MTYPE_PTEXT, QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS,
					320, y, ID_SPECTATE, TeamMain_MenuEvent, "SPECTATE",
					colorRed, UI_CENTER|UI_SMALLFONT);

	// set initial states
	switch (gametype)
	{
	case GT_SINGLE_PLAYER:
	case GT_FFA:
	case GT_TOURNAMENT:
		s_teammain.joinred.generic.flags  |= QMF_GRAYED;
		s_teammain.joinblue.generic.flags |= QMF_GRAYED;
		break;

	default:
	case GT_TEAM:
	case GT_CTF:
	case GT_INVASION:
	case GT_DESTROY:
		s_teammain.joingame.generic.flags |= QMF_GRAYED;
		break;
	}

	Menu_AddItem(&s_teammain.menu, (void*) &s_teammain.frame);
	Menu_AddItem(&s_teammain.menu, (void*) &s_teammain.joinred);
	Menu_AddItem(&s_teammain.menu, (void*) &s_teammain.joinblue);
	Menu_AddItem(&s_teammain.menu, (void*) &s_teammain.joinauto);
	Menu_AddItem(&s_teammain.menu, (void*) &s_teammain.joingame);
	Menu_AddItem(&s_teammain.menu, (void*) &s_teammain.spectate);
	/*{
		Menu_AddItem(&s_teammain.menu, (void*) &s_teammain.joinredfemale);
		Menu_AddItem(&s_teammain.menu, (void*) &s_teammain.joinbluefemale);
	}*/
}


/*
===============
TeamMain_Cache
===============
*/
void TeamMain_Cache(void)
{
	trap_R_RegisterShaderNoMip(TEAMMAIN_FRAME);
}


/*
===============
UI_TeamMainMenu
===============
*/
void UI_TeamMainMenu(int NbRed, int NbBlue)
{
	TeamMain_MenuInit(NbRed, NbBlue);
	UI_PushMenu (&s_teammain.menu);
}

/*==================== EOF because of buggy VSS ===========*/