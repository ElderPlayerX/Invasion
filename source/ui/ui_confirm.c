// Copyright (C) 1999-2000 Id Software, Inc.
//
/*
=======================================================================

CONFIRMATION MENU

=======================================================================
*/


#include "ui_local.h"


#define ART_CONFIRM_FRAME	"menu/art/cut_frame"

#define ID_CONFIRM_NO		10
#define ID_CONFIRM_YES		11


typedef struct
{
	menuframework_s menu;

	menutext_s		no;
	menutext_s		yes;

	int				slashX;
	const char *	question;
	void			(*draw)(void);
	void			(*action)(qboolean result);
}
confirmMenu_t;


static confirmMenu_t	s_confirm;


/*
=================
ConfirmMenu_Event
=================
*/
static void ConfirmMenu_Event(void* ptr, int event)
{
	qboolean	result;

	if (event != QM_ACTIVATED)
	{
		return;
	}

	UI_PopMenu();

	if (((menucommon_s*)ptr)->id == ID_CONFIRM_NO)
	{
		result = qfalse;
	}
	else
	{
		result = qtrue;
	}

	if (s_confirm.action)
	{
		s_confirm.action(result);
	}
}


/*
=================
ConfirmMenu_Key
=================
*/
static sfxHandle_t ConfirmMenu_Key(int key)
{
	switch (key)
	{
	case K_KP_LEFTARROW:
	case K_LEFTARROW:
	case K_KP_RIGHTARROW:
	case K_RIGHTARROW:
		key = K_TAB;
		break;

	case 'n':
	case 'N':
		ConfirmMenu_Event(&s_confirm.no, QM_ACTIVATED);
		break;

	case 'y':
	case 'Y':
		ConfirmMenu_Event(&s_confirm.yes, QM_ACTIVATED);
		break;
	}

	return Menu_DefaultKey(&s_confirm.menu, key);
}


/*
=================
ConfirmMenu_Draw
=================
*/
static void ConfirmMenu_Draw(void)
{
	int Flags = UI_CENTER|UI_INVERSE;

	if (s_confirm.action == NULL)
	{
		int i,
			l = strlen(s_confirm.question);
		char Text[1024];

		Flags |= UI_SMALLFONT;
		UI_DrawProportionalString(320, 204 - 32, "Error :", Flags, color_red);

		for (i = l / 2; i >=0 ; --i)
		{
			if (s_confirm.question[i] == ' ' || s_confirm.question[i] == ':')
			{
				++i;
				Q_strncpyz(Text, s_confirm.question, i);
				Text[i] = 0;

				UI_DrawProportionalString(320, 204, Text, Flags, color_red);

				if (i < l)
					UI_DrawProportionalString(320, 204 + 18, s_confirm.question + i, Flags, color_red);

				break;
			}
		}

		if (i < 0)
			UI_DrawProportionalString(320, 204, s_confirm.question, Flags, color_red);
	}
	else
	{
		UI_DrawNamedPic(142, 118, 359, 256, ART_CONFIRM_FRAME);
		UI_DrawProportionalString(320, 204, s_confirm.question, Flags, color_red);
		UI_DrawProportionalString(s_confirm.slashX, 265, "/", UI_LEFT|UI_INVERSE, color_red);
	}

	Menu_Draw(&s_confirm.menu);

	if (s_confirm.draw)
	{
		s_confirm.draw();
	}
}


/*
=================
ConfirmMenu_Cache
=================
*/
void ConfirmMenu_Cache(void)
{
	trap_R_RegisterShaderNoMip(ART_CONFIRM_FRAME);
}


/*
=================
UI_ConfirmMenu
=================
*/
void UI_ConfirmMenu(const char *question, void (*draw)(void), void (*action)(qboolean result))
{
	uiClientState_t	cstate;
	int	n1, n2, n3;
	int	l1, l2, l3;

	// zero set all our globals
	memset(&s_confirm, 0, sizeof(s_confirm));

	ConfirmMenu_Cache();

	n1 = UI_ProportionalStringWidth("YES/NO");
	n2 = UI_ProportionalStringWidth("YES") + PROP_GAP_WIDTH;
	n3 = UI_ProportionalStringWidth("/")  + PROP_GAP_WIDTH;
	l1 = 320 - (n1 / 2);
	l2 = l1 + n2;
	l3 = l2 + n3;
	s_confirm.slashX = l2;

	s_confirm.question = question;
	s_confirm.draw = draw;
	s_confirm.action = action;

	s_confirm.menu.draw       = ConfirmMenu_Draw;
	s_confirm.menu.key        = ConfirmMenu_Key;
	s_confirm.menu.wrapAround = qtrue;

	trap_GetClientState(&cstate);
	if (cstate.connState >= CA_CONNECTED)
	{
		s_confirm.menu.fullscreen = qfalse;
	}
	else
	{
		s_confirm.menu.fullscreen = qtrue;
	}

	if (action != NULL)
	{
		s_confirm.yes.generic.type		= MTYPE_PTEXT;
		s_confirm.yes.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
		s_confirm.yes.generic.callback	= ConfirmMenu_Event;
		s_confirm.yes.generic.id		= ID_CONFIRM_YES;
		s_confirm.yes.generic.x			= l1;
		s_confirm.yes.generic.y			= 264;
		s_confirm.yes.string			= "YES";
		s_confirm.yes.color				= color_red;
		s_confirm.yes.style				= UI_LEFT;

		s_confirm.no.generic.type		= MTYPE_PTEXT;
		s_confirm.no.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
		s_confirm.no.generic.callback	= ConfirmMenu_Event;
		s_confirm.no.generic.id			= ID_CONFIRM_NO;
		s_confirm.no.generic.x		    = l3;
		s_confirm.no.generic.y		    = 264;
		s_confirm.no.string				= "NO";
		s_confirm.no.color			    = color_red;
		s_confirm.no.style			    = UI_LEFT;

		Menu_AddItem(&s_confirm.menu,	&s_confirm.yes);
		Menu_AddItem(&s_confirm.menu,	&s_confirm.no);
	}
	else
	{
		s_confirm.yes.generic.type		= MTYPE_PTEXT;
		s_confirm.yes.generic.flags	= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
		s_confirm.yes.generic.callback= ConfirmMenu_Event;
		s_confirm.yes.generic.id		= ID_CONFIRM_YES;
		s_confirm.yes.generic.x			= 320 - UI_ProportionalStringWidth("OK") / 2;
		s_confirm.yes.generic.y			= 264;
		s_confirm.yes.string				= "OK";
		s_confirm.yes.color				= color_red;
		s_confirm.yes.style				= UI_LEFT;

		Menu_AddItem(&s_confirm.menu,	&s_confirm.yes);
	}

	UI_PushMenu(&s_confirm.menu);

	Menu_SetCursorToItem(&s_confirm.menu, &s_confirm.no);
}




/*==================== EOF because of buggy VSS ===========*/