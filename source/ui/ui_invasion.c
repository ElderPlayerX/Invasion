// Copyright (C) 1999-2000 Id Software, Inc.
//
/*
=======================================================================

Invasion MENU

=======================================================================
*/


#include "ui_local.h"

void InvasionNextMenu(void);
void InvasionSendMarineSelection(void);
void InvasionSendAlienSelection(void);


vmCvar_t	Inv_Conf_MarineWeapon;
vmCvar_t	Inv_Conf_MarineArmor;
vmCvar_t	Inv_Conf_AlienRace;


#define INV_MENU_VERTICAL_SPACING	28
#define INV_MAX_SELECT_TIME			15


enum
{
	e_Id_Default			=	10,
	e_Id_AllDefault,
	e_Id_ChainGun			= 	15,
	e_Id_ShotGun,
	e_Id_Flame,
	e_Id_Bfg,
	e_Id_Kevlar,
	e_Id_Titanium,
	e_Id_Composite,
	e_Id_Magnetic,
	e_Id_Trooper,
	e_Id_Bloomy,
	e_Id_Rad,
	e_Id_Xenomorph,
};

typedef struct
{
	menuframework_s menu;

	menutext_s Banner1;
	menutext_s Banner2;
	menutext_s Banner3;
}
AutoDefault_t;

menutext_s Default;
menutext_s AllDefault;

typedef struct
{
	menuframework_s menu;

	menutext_s Banner;
	menutext_s ChainGun;
	menutext_s ShotGun;
	menutext_s Flame;
	menutext_s Bfg;
}
MarineWeapon_t;

typedef struct
{
	menuframework_s menu;

	menutext_s Banner;
	menutext_s Kevlar;
	menutext_s Titanium;
	menutext_s Composite;
	menutext_s Magnetic;
}
MarineArmor_t;

typedef struct
{
	menuframework_s menu;

	menutext_s Banner;
	menutext_s Trooper;
	menutext_s Bloomy;
	menutext_s Rad;
	menutext_s Xenomorph;
}
AlienRace_t;


typedef enum
{
	e_Menu_None					=	0,
	e_Menu_MarineWeapon,
	e_Menu_MarineArmor,
	e_Menu_MarineEnd,
	e_Menu_AlienRace,
}
EInvMenu;

static char MenuBuffer[sizeof(menuframework_s) + sizeof(menutext_s) * 10];
static EInvMenu InvCurrentMenu;
static int InvMenuStartTime;
static int FirstId, NbId;


// ====================================
// *** Common stuff
// ====================================

void InitMenuText(menutext_s *Menu, int type, unsigned flags, int x, int y, int id,
			void (*callback)(void *self, int event), char *string, float *color, int style)
{
	Menu->generic.type		= type;
	Menu->generic.flags		= flags;
	Menu->generic.x			= x;
	Menu->generic.y			= y;
	Menu->generic.id			= id;
	Menu->generic.callback	= callback;
	Menu->string				= string;
	Menu->color					= color;
	Menu->style					= style;
}



void InitDefault(int x, int y, int Num, void (*callback)(void *self, int event))
{
	static char Str[2][32];

	memset(&Default, 0, sizeof(Default));
	memset(&AllDefault, 0, sizeof(AllDefault));

	strcpy(Str[0], va("%d - Last Selection", Num));
	strcpy(Str[1], va("%d - Last For All", Num + 1));

	NbId = Num - 1;

	y += INV_MENU_VERTICAL_SPACING;
	InitMenuText(&Default, MTYPE_PTEXT, QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS,
					x, y, e_Id_Default, callback, Str[0],
					color_white, UI_LEFT | UI_SMALLFONT | UI_DROPSHADOW);

	y += INV_MENU_VERTICAL_SPACING;
	InitMenuText(&AllDefault, MTYPE_PTEXT, QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS,
					x, y, e_Id_AllDefault, callback, Str[1],
					color_white, UI_LEFT | UI_SMALLFONT | UI_DROPSHADOW);
}


void Marine_Event(void *ptr, int notification)
{
	int id = ((menucommon_s*)ptr)->id;

	if (notification != QM_ACTIVATED)
		return;

	switch (id)
	{
		case e_Id_ChainGun:
		case e_Id_ShotGun:
		case e_Id_Flame:
		case e_Id_Bfg:
			trap_Cvar_SetValue("Inv_Conf_MarineWeapon", id - e_Id_ChainGun);
			InvasionNextMenu();
			break;

		case e_Id_Kevlar:
		case e_Id_Titanium:
		case e_Id_Composite:
		case e_Id_Magnetic:
			trap_Cvar_SetValue("Inv_Conf_MarineArmor", id - e_Id_Kevlar);
			InvasionNextMenu();
			break;

		case e_Id_Default:
			InvasionNextMenu();
			break;

		case e_Id_AllDefault:
			UI_PopMenu();
			InvasionSendMarineSelection();
			break;
	}
}


void Alien_Event(void *ptr, int notification)
{
	int id = ((menucommon_s*)ptr)->id;

	if (notification != QM_ACTIVATED)
		return;

	switch (id)
	{
		case e_Id_Trooper:
		case e_Id_Bloomy:
		case e_Id_Rad:
		case e_Id_Xenomorph:
			trap_Cvar_SetValue("Inv_Conf_AlienRace", id - e_Id_Trooper);
			InvasionNextMenu();
			break;

		case e_Id_Default:
			InvasionNextMenu();
			break;

		case e_Id_AllDefault:
			UI_PopMenu();
			InvasionSendAlienSelection();
			break;
	}
}


sfxHandle_t InvasionSelection_Key(int key)
{
	if (key >= '1' && key < '1' + NbId + 2)
	{
		menucommon_s Dummy;
		memset(&Dummy, 0, sizeof(Dummy));

		if (key == '1' + NbId)
			Dummy.id = e_Id_Default;
		else if (key == '1' + NbId + 1)
			Dummy.id = e_Id_AllDefault;
		else
			Dummy.id = FirstId + key - '1';

		if (InvCurrentMenu < e_Menu_MarineEnd)
			Marine_Event(&Dummy, QM_ACTIVATED);
		else
			Alien_Event(&Dummy, QM_ACTIVATED);

		return menu_in_sound;
	}

	switch (key)
	{
		case K_MOUSE2:
		case K_ESCAPE:
			if (InvCurrentMenu < e_Menu_MarineEnd)
				Marine_Event(&AllDefault, QM_ACTIVATED);
			else
				Alien_Event(&AllDefault, QM_ACTIVATED);
			return menu_in_sound;

		default:
			return Menu_DefaultKey(uis.activemenu, key);
	}

	return menu_in_sound;
}


// ====================================
// *** Auto Default
// ====================================

void AutoDefaultDrawMenu(void)
{
	Menu_Draw(uis.activemenu);

	if (uis.realtime - InvMenuStartTime >= (INV_MAX_SELECT_TIME + 2) * 1000)
		UI_PopMenu();
}

void AutoDefault_Init(void)
{
	int y;
	int x = 320 - BIGCHAR_WIDTH * 8;
	AutoDefault_t *s_AutoDefault = (AutoDefault_t *) MenuBuffer;

	memset(s_AutoDefault, 0, sizeof(AutoDefault_t));
	s_AutoDefault->menu.fullscreen = qfalse;

	InvCurrentMenu = e_Menu_None;

	y = 192;
	InitMenuText(&s_AutoDefault->Banner1, MTYPE_BTEXT, QMF_CENTER_JUSTIFY,
					320, y, 0, NULL, "LAST",
					color_white, UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW);

	y += (int) (INV_MENU_VERTICAL_SPACING * 1.5);
	InitMenuText(&s_AutoDefault->Banner2, MTYPE_BTEXT, QMF_CENTER_JUSTIFY,
					320, y, 0, NULL, "SELECTION",
					color_white, UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW);

	y += (int) (INV_MENU_VERTICAL_SPACING * 1.5);
	InitMenuText(&s_AutoDefault->Banner3, MTYPE_BTEXT, QMF_CENTER_JUSTIFY,
					320, y, 0, NULL, "APPLIED",
					color_white, UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW);

	Menu_AddItem(&s_AutoDefault->menu, &s_AutoDefault->Banner1);
	Menu_AddItem(&s_AutoDefault->menu, &s_AutoDefault->Banner2);
	Menu_AddItem(&s_AutoDefault->menu, &s_AutoDefault->Banner3);
	s_AutoDefault->menu.draw = AutoDefaultDrawMenu;

	UI_PushMenu(&s_AutoDefault->menu);
}


void InvasionDrawMenu(void)
{
	Menu_Draw(uis.activemenu);

	if (uis.realtime - InvMenuStartTime >= INV_MAX_SELECT_TIME * 1000)
	{
		if (InvCurrentMenu < e_Menu_MarineEnd)
			Marine_Event(&AllDefault.generic, QM_ACTIVATED);
		else
			Alien_Event(&AllDefault.generic, QM_ACTIVATED);

		AutoDefault_Init();

		//trap_Cmd_ExecuteText(EXEC_APPEND, "cp \"" S_COLOR_WHITE "DEFAULT SELECTION APPLIED\n\"");
	}
}


// ====================================
// *** Marine Weapon
// ====================================


void MarineWeapon_Init(void)
{
	int y;
	int x = 320 - BIGCHAR_WIDTH * 8;
	MarineWeapon_t *s_MarineWeapon = (MarineWeapon_t *) MenuBuffer;

	memset(s_MarineWeapon, 0, sizeof(MarineWeapon_t));
	s_MarineWeapon->menu.fullscreen = qfalse;
	s_MarineWeapon->menu.key = InvasionSelection_Key;

	InvCurrentMenu = e_Menu_MarineWeapon;
	FirstId = e_Id_ChainGun;

	y = 160;
	InitMenuText(&s_MarineWeapon->Banner, MTYPE_BTEXT, QMF_CENTER_JUSTIFY,
					320, y, 0, NULL, "SELECT WEAPON",
					color_white, UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW);

	y += (int) (INV_MENU_VERTICAL_SPACING * 1.5);
	InitMenuText(&s_MarineWeapon->ChainGun, MTYPE_PTEXT, QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS,
					x, y, e_Id_ChainGun, Marine_Event, "1 - Chain Gun",
					color_red, UI_LEFT | UI_SMALLFONT | UI_DROPSHADOW);

	y += INV_MENU_VERTICAL_SPACING;
	InitMenuText(&s_MarineWeapon->ShotGun, MTYPE_PTEXT, QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS,
					x, y, e_Id_ShotGun, Marine_Event, "2 - ShotGun",
					color_red, UI_LEFT | UI_SMALLFONT | UI_DROPSHADOW);

	y += INV_MENU_VERTICAL_SPACING;
	InitMenuText(&s_MarineWeapon->Flame, MTYPE_PTEXT, QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS,
					x, y, e_Id_Flame, Marine_Event, "3 - FlameThrower",
					color_red, UI_LEFT | UI_SMALLFONT | UI_DROPSHADOW);

	y += INV_MENU_VERTICAL_SPACING;
	InitMenuText(&s_MarineWeapon->Bfg, MTYPE_PTEXT, QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS,
					x, y, e_Id_Bfg, Marine_Event, "4 - Bfg 11k",
					color_red, UI_LEFT | UI_SMALLFONT | UI_DROPSHADOW);

	InitDefault(x, y, 5, Marine_Event);

	Menu_AddItem(&s_MarineWeapon->menu, &s_MarineWeapon->Banner);
	Menu_AddItem(&s_MarineWeapon->menu, &s_MarineWeapon->ChainGun);
	Menu_AddItem(&s_MarineWeapon->menu, &s_MarineWeapon->ShotGun);
	Menu_AddItem(&s_MarineWeapon->menu, &s_MarineWeapon->Flame);
	Menu_AddItem(&s_MarineWeapon->menu, &s_MarineWeapon->Bfg);
	Menu_AddItem(&s_MarineWeapon->menu, &Default);
	Menu_AddItem(&s_MarineWeapon->menu, &AllDefault);
	s_MarineWeapon->menu.draw = InvasionDrawMenu;

	UI_PushMenu(&s_MarineWeapon->menu);
}


// ====================================
// *** Marine Armor
// ====================================

void MarineArmor_Init(void)
{
	int y;
	int x = 320 - BIGCHAR_WIDTH * 8;
	MarineArmor_t *s_MarineArmor = (MarineArmor_t *) MenuBuffer;

	memset(s_MarineArmor, 0, sizeof(MarineArmor_t));
	s_MarineArmor->menu.fullscreen = qfalse;
	s_MarineArmor->menu.key = InvasionSelection_Key;

	InvCurrentMenu = e_Menu_MarineArmor;
	FirstId = e_Id_Kevlar;

	y = 160;
	InitMenuText(&s_MarineArmor->Banner, MTYPE_BTEXT, QMF_CENTER_JUSTIFY,
					320, y, 0, NULL, "SELECT ARMOR",
					color_white, UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW);

	y += (int) (INV_MENU_VERTICAL_SPACING * 1.5);
	InitMenuText(&s_MarineArmor->Kevlar, MTYPE_PTEXT, QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS,
					x, y, e_Id_Kevlar, Marine_Event, "1 - Kevlar (300)",
					color_red, UI_LEFT | UI_SMALLFONT | UI_DROPSHADOW);

	y += INV_MENU_VERTICAL_SPACING;
	InitMenuText(&s_MarineArmor->Titanium, MTYPE_PTEXT, QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS,
					x, y, e_Id_Titanium, Marine_Event, "2 - Titanium (500)",
					color_red, UI_LEFT | UI_SMALLFONT | UI_DROPSHADOW);

	y += INV_MENU_VERTICAL_SPACING;
	InitMenuText(&s_MarineArmor->Composite, MTYPE_PTEXT, QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS,
					x, y, e_Id_Composite, Marine_Event, "3 - Composite (750)",
					color_red, UI_LEFT | UI_SMALLFONT | UI_DROPSHADOW);

	y += INV_MENU_VERTICAL_SPACING;
	InitMenuText(&s_MarineArmor->Magnetic, MTYPE_PTEXT, QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS,
					x, y, e_Id_Magnetic, Marine_Event, "4 - Magnetic (200+)",
					color_red, UI_LEFT | UI_SMALLFONT | UI_DROPSHADOW);

	InitDefault(x, y, 5, Marine_Event);

	Menu_AddItem(&s_MarineArmor->menu, &s_MarineArmor->Banner);
	Menu_AddItem(&s_MarineArmor->menu, &s_MarineArmor->Kevlar);
	Menu_AddItem(&s_MarineArmor->menu, &s_MarineArmor->Titanium);
	Menu_AddItem(&s_MarineArmor->menu, &s_MarineArmor->Composite);
	Menu_AddItem(&s_MarineArmor->menu, &s_MarineArmor->Magnetic);
	Menu_AddItem(&s_MarineArmor->menu, &Default);
	Menu_AddItem(&s_MarineArmor->menu, &AllDefault);
	s_MarineArmor->menu.draw = InvasionDrawMenu;

	UI_PushMenu(&s_MarineArmor->menu);
}


// ====================================
// *** Alien Race
// ====================================

void AlienRace_Init(void)
{
	int y;
	int x = 320 - BIGCHAR_WIDTH * 8;
	AlienRace_t *s_AlienRace = (AlienRace_t *) MenuBuffer;

	memset(s_AlienRace, 0, sizeof(AlienRace_t));
	s_AlienRace->menu.fullscreen = qfalse;
	s_AlienRace->menu.key = InvasionSelection_Key;

	InvCurrentMenu = e_Menu_AlienRace;
	FirstId = e_Id_Trooper;

	y = 160;
	InitMenuText(&s_AlienRace->Banner, MTYPE_BTEXT, QMF_CENTER_JUSTIFY,
					320, y, 0, NULL, "SELECT RACE",
					color_white, UI_CENTER | UI_SMALLFONT | UI_DROPSHADOW);

	y += (int) (INV_MENU_VERTICAL_SPACING * 1.5);
	InitMenuText(&s_AlienRace->Trooper, MTYPE_PTEXT, QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS,
					x, y, e_Id_Trooper, Alien_Event, "1 - Trooper",
					color_red, UI_LEFT | UI_SMALLFONT | UI_DROPSHADOW);

	y += INV_MENU_VERTICAL_SPACING;
	InitMenuText(&s_AlienRace->Bloomy, MTYPE_PTEXT, QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS,
					x, y, e_Id_Bloomy, Alien_Event, "2 - Bloomy",
					color_red, UI_LEFT | UI_SMALLFONT | UI_DROPSHADOW);

	y += INV_MENU_VERTICAL_SPACING;
	InitMenuText(&s_AlienRace->Rad, MTYPE_PTEXT, QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS,
					x, y, e_Id_Rad, Alien_Event, "3 - Rad",
					color_red, UI_LEFT | UI_SMALLFONT | UI_DROPSHADOW);

	y += INV_MENU_VERTICAL_SPACING;
	InitMenuText(&s_AlienRace->Xenomorph, MTYPE_PTEXT, QMF_LEFT_JUSTIFY | QMF_PULSEIFFOCUS,
					x, y, e_Id_Xenomorph, Alien_Event, "4 - Xenomorph",
					color_red, UI_LEFT | UI_SMALLFONT | UI_DROPSHADOW);

	InitDefault(x, y, 5, Alien_Event);

	Menu_AddItem(&s_AlienRace->menu, &s_AlienRace->Banner);
	Menu_AddItem(&s_AlienRace->menu, &s_AlienRace->Trooper);
	Menu_AddItem(&s_AlienRace->menu, &s_AlienRace->Bloomy);
	Menu_AddItem(&s_AlienRace->menu, &s_AlienRace->Rad);
	Menu_AddItem(&s_AlienRace->menu, &s_AlienRace->Xenomorph);
	Menu_AddItem(&s_AlienRace->menu, &Default);
	Menu_AddItem(&s_AlienRace->menu, &AllDefault);
	s_AlienRace->menu.draw = InvasionDrawMenu;

	UI_PushMenu(&s_AlienRace->menu);
}


// ====================================
// *** Menu Launch
// ====================================

void Inv_UI_MarineSelection_f(void)
{
	InvMenuStartTime = uis.realtime;
	MarineWeapon_Init();
}


void Inv_UI_AlienSelection_f(void)
{
	InvMenuStartTime = uis.realtime;
	AlienRace_Init();
}


void InvasionNextMenu(void)
{
	UI_PopMenu();

	switch (InvCurrentMenu)
	{
		case e_Menu_MarineWeapon:
			MarineArmor_Init();
			break;

		case e_Menu_MarineArmor:
			InvasionSendMarineSelection();
			break;

		case e_Menu_AlienRace:
			InvasionSendAlienSelection();
			break;
	}
}


void InvasionSendMarineSelection(void)
{
	trap_Cvar_Update(&Inv_Conf_MarineWeapon);
	trap_Cvar_Update(&Inv_Conf_MarineArmor);
	trap_Cmd_ExecuteText(EXEC_APPEND, va("InvMarineSelected %i %i\n",
			Inv_Conf_MarineWeapon.integer, Inv_Conf_MarineArmor.integer));

	trap_Cmd_ExecuteText(EXEC_APPEND, "Inv_IGH\n");
}


void InvasionSendAlienSelection(void)
{
	trap_Cvar_Update(&Inv_Conf_AlienRace);
	trap_Cmd_ExecuteText(EXEC_APPEND, va("InvAlienSelected %i\n",
			Inv_Conf_AlienRace.integer));

	trap_Cmd_ExecuteText(EXEC_APPEND, "Inv_IGH\n");
}



/*==================== EOF because of buggy VSS ===========*/