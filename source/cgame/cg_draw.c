// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"

#ifdef MISSIONPACK
#include "../ui/ui_shared.h"

// used for scoreboard
extern displayContextDef_t cgDC;
menuDef_t *menuScoreboard = NULL;
#else
int drawTeamOverlayModificationCount = -1;
#endif

int sortedTeamPlayers[TEAM_MAXOVERLAY];
int	numSortedTeamPlayers;

static char *IGH_Armor[e_Selection_MaxArmor] = {
	"- Kevlar - Light Armor",
	"- Titanium - Medium Armor",
	"- Composite - Heavy Armor",
	"- Magnetic - Regenerative Armor" };

static char *IGH_Weapon[e_Selection_MaxWeapon] = {
	"- Chaingun - fires bullets\n2nd fire: stable shot",
	"- Shotgun - fires pellets\n2nd fire: 3 shots burst mode",
	"- FlameThrower - fires flames\n2nd fire: short range",
	"- BFG 11k - fires plasma energy\n2nd fire: charge mode",
	"- Sniper Rifle - fires bullets\n2nd fire : toggle Zoom On/Off" };

static char *IGH_Race[e_Selection_MaxRace] = {
	"- Trooper -\nCan grab dead marine weapons\n2nd attack: 2 grenades",
	"- Bloomy -\nExploses lethally when die\n(except when killed by CrowBar)\nArmed with an Acid Gun",
	"- Rad -\nIrradiates marines through their armor\nCloser of marine, more lethal\nGood Immune against BFG",
	"- Xeno -\nQuick on legs & lethal claws\n2nd attack: big jumping attack\nWalk on Wall when crouched" };

static char *IGH_AlienGoal[GT_MAX_GAME_TYPE] = {
	NULL,	//FFA
	NULL,	//Tourney
	NULL,	//1 up
	NULL,	//TDM
	"-- CTF Goal --\nCapture and bring back the enemy Flag",
	"-- TTE Goal --\nDefend the Egg & kill all the Marines",
	"-- DAE Goal --\nProtect the Eggs & kill all the Marines",
	};

static char *IGH_MarineGoal[GT_MAX_GAME_TYPE] = {
	NULL,	//FFA
	NULL,	//Tourney
	NULL,	//1 up
	NULL,	//TDM
	"-- CTF Goal --\nCapture and bring back the enemy Flag",
	"-- TTE Goal --\nBring back the Egg & try to survive",
	"-- DAE Goal --\nSeek & Destroy all the Eggs",
	};


vec4_t RedTeamColor = { 1.0f, 0.5f, 0.1f, 1.0f },
		BlueTeamColor = { 0.1f, 0.5f, 1.0f, 1.0f };


char systemChat[256];
char teamChat1[256];
char teamChat2[256];

#ifdef MISSIONPACK

int CG_Text_Width(const char *text, float scale, int limit)
{
	int count,len;
	float out;
	glyphInfo_t *glyph;
	float useScale;
	// FIXME: see ui_main.c, same problem
	//	const unsigned char *s = text;
	const char *s = text;
	fontInfo_t *font = &cgDC.Assets.textFont;
	if (scale <= cg_smallFont.value)
	{
		font = &cgDC.Assets.smallFont;
	}
	else if (scale > cg_bigFont.value)
	{
		font = &cgDC.Assets.bigFont;
	}
	useScale = scale * font->glyphScale;
	out = 0;
	if (text)
	{
   	len = strlen(text);
		if (limit > 0 && len > limit)
		{
			len = limit;
		}
		count = 0;
		while (s && *s && count < len)
		{
			if (Q_IsColorString(s))
			{
				s += 2;
				continue;
			}
			else
			{
				glyph = &font->glyphs[(int)*s]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
				out += glyph->xSkip;
				s++;
				count++;
			}
   	}
	}
	return out * useScale;
}

int CG_Text_Height(const char *text, float scale, int limit)
{
	int len, count;
	float max;
	glyphInfo_t *glyph;
	float useScale;
	// TTimo: FIXME
	//	const unsigned char *s = text;
	const char *s = text;
	fontInfo_t *font = &cgDC.Assets.textFont;
	if (scale <= cg_smallFont.value)
	{
		font = &cgDC.Assets.smallFont;
	}
	else if (scale > cg_bigFont.value)
	{
		font = &cgDC.Assets.bigFont;
	}
	useScale = scale * font->glyphScale;
	max = 0;
	if (text)
	{
   	len = strlen(text);
		if (limit > 0 && len > limit)
		{
			len = limit;
		}
		count = 0;
		while (s && *s && count < len)
		{
			if (Q_IsColorString(s))
			{
				s += 2;
				continue;
			}
			else
			{
				glyph = &font->glyphs[(int)*s]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
	   		if (max < glyph->height)
				{
		   		max = glyph->height;
				}
				s++;
				count++;
			}
   	}
	}
	return max * useScale;
}

void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader)
{
  float w, h;
  w = width * scale;
  h = height * scale;
  CG_AdjustFrom640(&x, &y, &w, &h);
  trap_R_DrawStretchPic(x, y, w, h, s, t, s2, t2, hShader);
}

void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style)
{
	int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph;
	float useScale;
	fontInfo_t *font = &cgDC.Assets.textFont;
	if (scale <= cg_smallFont.value)
	{
		font = &cgDC.Assets.smallFont;
	}
	else if (scale > cg_bigFont.value)
	{
		font = &cgDC.Assets.bigFont;
	}
	useScale = scale * font->glyphScale;
	if (text)
	{
	// TTimo: FIXME
	//		const unsigned char *s = text;
		const char *s = text;
		trap_R_SetColor(color);
		memcpy(&newColor[0], &color[0], sizeof(vec4_t));
   	len = strlen(text);
		if (limit > 0 && len > limit)
		{
			len = limit;
		}
		count = 0;
		while (s && *s && count < len)
		{
			glyph = &font->glyphs[*s];
   	//int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
   	//float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
			if (Q_IsColorString(s))
			{
				memcpy(newColor, g_color_table[ColorIndex(*(s+1))], sizeof(newColor));
				newColor[3] = color[3];
				trap_R_SetColor(newColor);
				s += 2;
				continue;
			}
			else
			{
				float yadj = useScale * glyph->top;
				if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE)
				{
					int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;
					colorBlack[3] = newColor[3];
					trap_R_SetColor(colorBlack);
					CG_Text_PaintChar(x + ofs, y - yadj + ofs,
														glyph->imageWidth,
														glyph->imageHeight,
														useScale,
														glyph->s,
														glyph->t,
														glyph->s2,
														glyph->t2,
														glyph->glyph);
					colorBlack[3] = 1.0f;
					trap_R_SetColor(newColor);
				}
				CG_Text_PaintChar(x, y - yadj,
													glyph->imageWidth,
													glyph->imageHeight,
													useScale,
													glyph->s,
													glyph->t,
													glyph->s2,
													glyph->t2,
													glyph->glyph);
				// CG_DrawPic(x, y - yadj, scale * cgDC.Assets.textFont.glyphs[text[i]].imageWidth, scale * cgDC.Assets.textFont.glyphs[text[i]].imageHeight, cgDC.Assets.textFont.glyphs[text[i]].glyph);
				x += (glyph->xSkip * useScale) + adjust;
				s++;
				count++;
			}
   	}
		trap_R_SetColor(NULL);
	}
}


#endif

/*
==============
CG_DrawField

Draws large numbers for status bar and powerups
==============
*/
#ifndef MISSIONPACK
static void CG_DrawField(int x, int y, int width, int value, int Small, qboolean Shadow, float *Color)
{
	char	num[16], *ptr;
	int		l;
	int		frame;
	int w = CHAR_WIDTH, h = CHAR_HEIGHT;

	if (width < 1)
	{
		return;
	}

	if (Small)
	{
		w >>= Small;
		h >>= Small;
	}

	// draw number string
	if (width > 5)
	{
		width = 5;
	}

	switch (width)
	{
	case 1:
		value = value > 9 ? 9 : value;
		value = value < 0 ? 0 : value;
		break;
	case 2:
		value = value > 99 ? 99 : value;
		value = value < -9 ? -9 : value;
		break;
	case 3:
		value = value > 999 ? 999 : value;
		value = value < -99 ? -99 : value;
		break;
	case 4:
		value = value > 9999 ? 9999 : value;
		value = value < -999 ? -999 : value;
		break;
	}

	Com_sprintf(num, sizeof(num), "%i", value);
	l = strlen(num);
	if (l > width)
		l = width;
	x += 2 + (CHAR_WIDTH >> Small) * (width - l);

	if (!Shadow && Color != NULL)
		trap_R_SetColor(Color);

	ptr = num;
	while (*ptr && l)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		if (Shadow)
		{
			assert(Color != NULL);
			trap_R_SetColor(colorBlack);
			CG_DrawPic(x + 2, y + 2, w, h, cgs.media.numberShaders[frame]);
			trap_R_SetColor(Color);
		}

		CG_DrawPic(x, y, w, h, cgs.media.numberShaders[frame]);
		x += w;
		ptr++;
		l--;
	}

	if (Color != NULL)
		trap_R_SetColor(NULL);
}
#endif

/*
================
CG_Draw3DModel

================
*/
void CG_Draw3DModel(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles)
{
	refdef_t		refdef;
	refEntity_t		ent;

	if (!cg_draw3dIcons.integer || !cg_drawIcons.integer)
	{
		return;
	}

	CG_AdjustFrom640(&x, &y, &w, &h);

	memset(&refdef, 0, sizeof(refdef));

	memset(&ent, 0, sizeof(ent));
	AnglesToAxis(angles, ent.axis);
	VectorCopy(origin, ent.origin);
	ent.hModel = model;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW;		// no stencil shadows

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear(refdef.viewaxis);

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;

	trap_R_ClearScene();
	trap_R_AddRefEntityToScene(&ent);
	trap_R_RenderScene(&refdef);
}

/*
================
CG_DrawHead

Used for both the status bar and the scoreboard
================
*/
void CG_DrawHead(float x, float y, float w, float h, int clientNum, vec3_t headAngles)
{
	clipHandle_t	cm;
	clientInfo_t	*ci;
	float			len;
	vec3_t			origin;
	vec3_t			mins, maxs;

	ci = &cgs.clientinfo[ clientNum ];

	if (cg_draw3dIcons.integer)
	{
		cm = ci->headModel;
		if (!cm)
		{
			return;
		}

		// offset the origin y and z to center the head
		trap_R_ModelBounds(cm, mins, maxs);

		origin[2] = -0.5 * (mins[2] + maxs[2]);
		origin[1] = 0.5 * (mins[1] + maxs[1]);

		// calculate distance so the head nearly fills the box
		// assume heads are taller than wide
		len = 0.7 * (maxs[2] - mins[2]);
		origin[0] = len / 0.268;	// len / tan(fov/2)

		// allow per-model tweaking
		VectorAdd(origin, ci->headOffset, origin);

		CG_Draw3DModel(x, y, w, h, ci->headModel, ci->headSkin, origin, headAngles);
	}
	else if (cg_drawIcons.integer)
	{
		CG_DrawPic(x, y, w, h, ci->modelIcon);
	}

	// if they are deferred, draw a cross out
	if (ci->deferred)
	{
		CG_DrawPic(x, y, w, h, cgs.media.deferShader);
	}
}

/*
================
CG_DrawFlagModel

Used for both the status bar and the scoreboard
================
*/
void CG_DrawFlagModel(float x, float y, float w, float h, int team, qboolean force2D)
{
	float			len;
	vec3_t			origin, angles;
	vec3_t			mins, maxs;
	qhandle_t		handle;

	if (!force2D && cg_draw3dIcons.integer)
	{
		if (team == TEAM_RED)
		{
			handle = cgs.media.redFlagModel;
		}
		else if (team == TEAM_BLUE)
		{
			handle = cgs.media.blueFlagModel;
		}
		else if (team == TEAM_FREE)
		{
			handle = cgs.media.neutralFlagModel;
		}
		else
			return;

		VectorClear(angles);

		// offset the origin y and z to center the flag
		trap_R_ModelBounds(handle, mins, maxs);

		origin[2] = -0.5 * (mins[2] + maxs[2]);
		origin[1] = 0.5 * (mins[1] + maxs[1]);

		// calculate distance so the flag nearly fills the box
		// assume heads are taller than wide
		len = 0.5 * (maxs[2] - mins[2]);
		origin[0] = len / 0.268;	// len / tan(fov/2)

		angles[YAW] = 60 * sin(cg.time / 2000.0);

		CG_Draw3DModel(x, y, w, h, handle, 0, origin, angles);
	}
	else if (cg_drawIcons.integer)
	{
		qhandle_t icon = (team == TEAM_RED) ? cgs.media.RedFlagIcon: cgs.media.BlueFlagIcon;
		CG_DrawPic(x, y, w, h, icon);
	}
}

/*
================
CG_DrawStatusBarHead

================
*/
#ifndef MISSIONPACK

static void CG_DrawStatusBarHead(float x)
{
	vec3_t		angles;
	float		size, stretch;
	float		frac;

	VectorClear(angles);

	if (cg.damageTime && cg.time - cg.damageTime < DAMAGE_TIME)
	{
		frac = (float)(cg.time - cg.damageTime) / DAMAGE_TIME;
		size = ICON_SIZE * 1.25 * (1.5 - frac * 0.5);

		stretch = size - ICON_SIZE * 1.25;
		// kick in the direction of damage
		x -= stretch * 0.5 + cg.damageX * stretch * 0.5;

		cg.headStartYaw = 180 + cg.damageX * 45;

		cg.headEndYaw = 180 + 20 * cos(crandom()*M_PI);
		cg.headEndPitch = 5 * cos(crandom()*M_PI);

		cg.headStartTime = cg.time;
		cg.headEndTime = cg.time + 100 + random() * 2000;
	}
	else
	{
		if (cg.time >= cg.headEndTime)
		{
			// select a new head angle
			cg.headStartYaw = cg.headEndYaw;
			cg.headStartPitch = cg.headEndPitch;
			cg.headStartTime = cg.headEndTime;
			cg.headEndTime = cg.time + 100 + random() * 2000;

			cg.headEndYaw = 180 + 20 * cos(crandom()*M_PI);
			cg.headEndPitch = 5 * cos(crandom()*M_PI);
		}

		size = ICON_SIZE * 1.25;
	}

	// if the server was frozen for a while we may have a bad head start time
	if (cg.headStartTime > cg.time)
	{
		cg.headStartTime = cg.time;
	}

	frac = (cg.time - cg.headStartTime) / (float)(cg.headEndTime - cg.headStartTime);
	frac = frac * frac * (3 - 2 * frac);
	angles[YAW] = cg.headStartYaw + (cg.headEndYaw - cg.headStartYaw) * frac;
	angles[PITCH] = cg.headStartPitch + (cg.headEndPitch - cg.headStartPitch) * frac;

	CG_DrawHead(x, 480 - size, size, size,
				cg.snap->ps.clientNum, angles);
}
#endif

/*
================
CG_DrawStatusBarFlag

================
*/
#ifndef MISSIONPACK
static void CG_DrawStatusBarFlag(float x, int team)
{
	CG_DrawFlagModel(x, 480 - ICON_SIZE, ICON_SIZE, ICON_SIZE, team, qfalse);
}
#endif

/*
================
CG_DrawTeamBackground

================
*/
void CG_DrawTeamBackground(int x, int y, int w, int h, float alpha, int team)
{
	vec4_t		hcolor;

	if (team == TEAM_RED)
		ColorCopy(hcolor, RedTeamColor);
	else if (team == TEAM_BLUE)
		ColorCopy(hcolor, BlueTeamColor);
	else
		return;

	hcolor[3] = alpha;

	trap_R_SetColor(hcolor);
	CG_DrawPic(x, y, w, h, cgs.media.teamStatusBar);
	trap_R_SetColor(NULL);
}

/*
================
CG_DrawStatusBar

================
*/
#ifndef MISSIONPACK
static void CG_DrawStatusBar(void)
{
	int			color;
	centity_t	*cent;
	playerState_t	*ps;
	int			value;
	vec3_t		angles;
	vec3_t		origin;
#ifdef MISSIONPACK
	qhandle_t	handle;
#endif
	static float colors[4][4] = {
//		{ 0.2, 1.0, 0.2, 1.0 }, { 1.0, 0.2, 0.2, 1.0 }, {0.5, 0.5, 0.5, 1}};
		{ 1.0f, 0.69f, 0.0f, 1.0f },		// normal
		{ 1.0f, 0.2f, 0.2f, 1.0f },		// low health
		{0.5f, 0.5f, 0.5f, 1.0f},			// weapon firing
		{ 1.0f, 1.0f, 1.0f, 1.0f }	};			// health > 100

	if (cg_drawStatus.integer == 0)
	{
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	VectorClear(angles);

	// draw any 3D icons first, so the changes back to 2D are minimized
	if (cent->currentState.weapon && cg_weapons[ cent->currentState.weapon ].ammoModel)
	{
		origin[0] = 70;
		origin[1] = 0;
		origin[2] = 0;
		angles[YAW] = 90 + 20 * sin(cg.time / 1000.0);
		CG_Draw3DModel(CHAR_WIDTH*3 + TEXT_ICON_SPACE, 432, ICON_SIZE, ICON_SIZE,
					   cg_weapons[ cent->currentState.weapon ].ammoModel, 0, origin, angles);
	}

	CG_DrawStatusBarHead(185 + CHAR_WIDTH*3 + TEXT_ICON_SPACE);

	if (cg.predictedPlayerState.powerups[PW_REDFLAG])
	{
		CG_DrawStatusBarFlag(185 + CHAR_WIDTH*3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_RED);
	}
	else if (cg.predictedPlayerState.powerups[PW_BLUEFLAG])
	{
		CG_DrawStatusBarFlag(185 + CHAR_WIDTH*3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_BLUE);
	}
	else if (cg.predictedPlayerState.powerups[PW_NEUTRALFLAG])
	{
		CG_DrawStatusBarFlag(185 + CHAR_WIDTH*3 + TEXT_ICON_SPACE + ICON_SIZE, TEAM_FREE);
	}

	if (ps->stats[ STAT_ARMOR ])
	{
		origin[0] = 90;
		origin[1] = 0;
		origin[2] = -10;
		angles[YAW] = (cg.time & 2047) * 360 / 2048.0f;
		CG_Draw3DModel(370 + CHAR_WIDTH*3 + TEXT_ICON_SPACE, 432, ICON_SIZE, ICON_SIZE,
					   cgs.media.armorModel, 0, origin, angles);
	}
#ifdef MISSIONPACK
	if (cgs.gametype == GT_HARVESTER)
	{
		origin[0] = 90;
		origin[1] = 0;
		origin[2] = -10;
		angles[YAW] = (cg.time & 2047) * 360 / 2048.0f;
		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE)
		{
			handle = cgs.media.redCubeModel;
		}
		else
		{
			handle = cgs.media.blueCubeModel;
		}
		CG_Draw3DModel(640 - (TEXT_ICON_SPACE + ICON_SIZE), 416, ICON_SIZE, ICON_SIZE, handle, 0, origin, angles);
	}
#endif
	//
	// ammo
	//
	if (cent->currentState.weapon)
	{
		int Mul = 0, less = 0;

		value = ps->ammo[cent->currentState.weapon];
		if (value > -1)
		{
			if (cg.snap->ps.persistant[PERS_TEAM] == cgs.InvasionInfo.MarineTeam)
			{
				int n = Inv_MarineCharger[cent->currentState.weapon];

				if (n && value)
				{
					Mul = value / n;
					value = value % n;
					if (!value)
					{
						value = n;
						--Mul;
					}

					less = (CHAR_WIDTH >> 2) * 3;
				}
			}

			if (Inv_IsFiring(cg.predictedPlayerState.weaponstate)
				&& cg.predictedPlayerState.weaponTime > 100)
			{
				// draw as dark grey when reloading
				color = 2;	// dark grey
			}
			else
			{
				if (value >= 0)
				{
					color = 0;	// green
				}
				else
				{
					color = 1;	// red
				}
			}

			CG_DrawField(0 + (CHAR_WIDTH >> 1) * 3 - less, 432 + (CHAR_HEIGHT >> 2), 3, value, 1, qtrue, colors[color]);

			if (ps->weapon == WP_BFG && ps->weaponstate == WEAPON_FIRING2)
			{
				if (ps->stats[STAT_WEAPON_SPECIAL] > 0)
					CG_DrawField((CHAR_WIDTH >> 1) * 3,
										432 + (CHAR_HEIGHT >> 2) + (CHAR_HEIGHT >> 3), 1,
										ps->stats[STAT_WEAPON_SPECIAL], 2, qtrue, colors[color]);
			}
			else if (Mul)
			{
				CG_DrawField((CHAR_WIDTH >> 1) * (3 + 3) - (CHAR_WIDTH >> 2) * 2 - 1,
								432 + (CHAR_HEIGHT >> 2) + (CHAR_HEIGHT >> 3), 1,
								Mul, 2, qtrue, colors[color]);


				CG_DrawStringExt((CHAR_WIDTH >> 1) * (3 + 3) - (CHAR_WIDTH >> 2) * 1,
										432 + (CHAR_HEIGHT >> 2) + (CHAR_HEIGHT >> 3),
										"x", colors[color], qfalse, qtrue,
										CHAR_WIDTH >> 2, CHAR_HEIGHT >> 2, 1);
			}

			// if we didn't draw a 3D icon, draw a 2D icon for ammo
			if (!cg_draw3dIcons.integer && cg_drawIcons.integer)
			{
				qhandle_t	icon;

				icon = cg_weapons[ cg.predictedPlayerState.weapon ].ammoIcon;
				if (icon)
				{
					CG_DrawPic(CHAR_WIDTH*3 + TEXT_ICON_SPACE, 432, ICON_SIZE, ICON_SIZE, icon);
				}
			}
		}
	}

	//
	// health
	//
	value = ps->stats[STAT_HEALTH];
	if (value > 100)
	{
		color = 3;							// white
	}
	else if (value > 50)
	{
		color = 0;							// green
	}
	else if (value > 25)
		color = (cg.time >> 8) & 1;	// flash
	else if (value > 0)
	{
		color = (cg.time >> 6) & 1;	//Too: flash quickly
	}
	else
		color = 1;

	// stretch the health up when taking damage
	CG_DrawField(185 + (CHAR_WIDTH >> 1) * 3, 432 + (CHAR_HEIGHT >> 2), 3, value, 1, qtrue, colors[color]);


	//
	// armor
	//
	value = ps->stats[STAT_ARMOR];
	if (value > 0)
	{
		CG_DrawField(370 + (CHAR_WIDTH >> 1) * 3, 432 + (CHAR_HEIGHT >> 2), 3, value, 1, qtrue, colors[0]);
		// if we didn't draw a 3D icon, draw a 2D icon for armor
		if (!cg_draw3dIcons.integer && cg_drawIcons.integer)
		{
			CG_DrawPic(370 + CHAR_WIDTH*3 + TEXT_ICON_SPACE, 432, ICON_SIZE, ICON_SIZE, cgs.media.armorIcon);
		}

	}
#ifdef MISSIONPACK
	//
	// cubes
	//
	if (cgs.gametype == GT_HARVESTER)
	{
		value = ps->generic1;
		if (value > 99)
		{
			value = 99;
		}
		CG_DrawField(640 - (CHAR_WIDTH*2 + TEXT_ICON_SPACE + ICON_SIZE), 432, 2, value, 0, qtrue, colors[0]);
		// if we didn't draw a 3D icon, draw a 2D icon for armor
		if (!cg_draw3dIcons.integer && cg_drawIcons.integer)
		{
			if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE)
			{
				handle = cgs.media.redCubeIcon;
			}
			else
			{
				handle = cgs.media.blueCubeIcon;
			}
			CG_DrawPic(640 - (TEXT_ICON_SPACE + ICON_SIZE), 432, ICON_SIZE, ICON_SIZE, handle);
		}
	}
#endif
}
#endif

/*
===========================================================================================

  UPPER RIGHT CORNER

===========================================================================================
*/

/*
================
Too: CG_DrawKills
================
*/
enum
{
	es_DK_Width = TINYCHAR_WIDTH + 2,
	es_DK_Height = TINYCHAR_HEIGHT + 2,
	es_DK_Egg = 64,
};

static float CG_DrawKills(float y)
{
	int i, l1, l2, nl2, w1, y2;
	vec4_t Color, TeamColor;
	char Text[4];

	for (i = 0; i < MAX_KILLDRAWN; ++i)
	{
		Kill_t *Kill = cg.Kills + (cg.KillIndex + i) % MAX_KILLDRAWN;

		if (!Kill->Time || Kill->Time + 5000 < cg.time)
			continue;

		w1 = CG_DrawStrlen(Kill->Killer) * es_DK_Width;

		if (Kill->TeamKilled < 0)
		{
			nl2 = 0;
			l2 = es_DK_Egg - 4;
			l1 = l2 + w1 + es_DK_Width + 4;
			y2 = y + (es_DK_Egg >> 1);
		}
		else
		{
			nl2 = l2 = CG_DrawStrlen(Kill->Killed);
			//if (l2 < 8)
			//	l2 = 8;
			l2 *= es_DK_Width;
			nl2 *= es_DK_Width;
			l1 = l2 + w1 + es_DK_Width + 4;
			y2 = y;
		}

		Vector4Copy(colorWhite, Color);
		if (Kill->Time + 4500 < cg.time)
			Color[3] = 1.0f + (Kill->Time + 4500 - cg.time) / 500.0f;

		if (Kill->TeamKiller > 0)
		{
			if (Kill->TeamKiller == cgs.InvasionInfo.MarineTeam)
				ColorCopy(TeamColor, RedTeamColor);
			else if (Kill->TeamKiller == cgs.InvasionInfo.AlienTeam)
				ColorCopy(TeamColor, BlueTeamColor);
			TeamColor[3] = Color[3] * 0.66f;

			trap_R_SetColor(TeamColor);

			if (Kill->TeamKilled < 0)
				CG_DrawPic(640 - l1, y + (es_DK_Egg >> 1), w1, es_DK_Height + 1, cgs.media.teamStatusBar);
			else
				CG_DrawPic(640 - l1, y, w1, es_DK_Height + 1, cgs.media.teamStatusBar);
		}

		if (Kill->TeamKilled > 0)
		{
			if (Kill->TeamKilled == cgs.InvasionInfo.MarineTeam)
				ColorCopy(TeamColor, RedTeamColor);
			else if (Kill->TeamKilled == cgs.InvasionInfo.AlienTeam)
				ColorCopy(TeamColor, BlueTeamColor);
			TeamColor[3] = Color[3] * 0.66f;

			trap_R_SetColor(TeamColor);
			CG_DrawPic(640 - l2, y, nl2, es_DK_Height + 1, cgs.media.teamStatusBar);
		}

		trap_R_SetColor(Color);
		CG_DrawPic(640 - l2 - es_DK_Width - 2, y2, es_DK_Width, es_DK_Height, Kill->Weapon);
		trap_R_SetColor(NULL);

		CG_DrawStringExt(640 - l1, y2, Kill->Killer, Color, qfalse, qtrue, es_DK_Width, es_DK_Height, 0);

		if (Kill->TeamKilled < 0)
		{
			int x = 640 - es_DK_Egg;

			Color[0] = 1.0f;
			Color[1] = 0.1f;
			Color[2] = 0.1f;
			CG_DrawPic(x, y, es_DK_Egg, es_DK_Egg, cgs.media.EggZone);
		#ifdef MISSIONPACK
			Com_sprintf (Text, sizeof(Text), "%i", -Kill->TeamKilled);
			CG_Text_Paint(x + es_DK_Egg - (CHAR_WIDTH >> 1) - 4, y + es_DK_Egg - (CHAR_HEIGHT >> 1) - 4,
				1, Color, Text, 0, 0, 1);
		#else
			CG_DrawField(x + es_DK_Egg - (CHAR_WIDTH >> 1) - 4, y + es_DK_Egg - (CHAR_HEIGHT >> 1) - 4,
							1, -Kill->TeamKilled, 1, 1, Color);
		#endif
		}
		else
			CG_DrawStringExt(640 - l2, y, Kill->Killed, Color, qfalse, qtrue, es_DK_Width, es_DK_Height, 0);

		if (Kill->TeamKilled < 0)
			y += es_DK_Egg + 1;
		else
			y += es_DK_Height + 1;
	}

	return y;
}

/*
================
CG_DrawAttacker

================
*/
/*Too: remove that... not really useful, and too disturbing on the HUD
static float CG_DrawAttacker(float y)
{
	int			t;
	float		size;
	vec3_t		angles;
	const char	*info;
	const char	*name;
	int			clientNum;

	if (cg.predictedPlayerState.stats[STAT_HEALTH] <= 0)
	{
		return y;
	}

	if (!cg.attackerTime)
	{
		return y;
	}

	clientNum = cg.predictedPlayerState.persistant[PERS_ATTACKER];
	if (clientNum < 0 || clientNum >= MAX_CLIENTS || clientNum == cg.snap->ps.clientNum)
	{
		return y;
	}

	t = cg.time - cg.attackerTime;
	if (t > ATTACKER_HEAD_TIME)
	{
		cg.attackerTime = 0;
		return y;
	}

	size = ICON_SIZE * 1.25;

	angles[PITCH] = 0;
	angles[YAW] = 180;
	angles[ROLL] = 0;
	CG_DrawHead(640 - size, y, size, size, clientNum, angles);

	info = CG_ConfigString(CS_PLAYERS + clientNum);
	name = Info_ValueForKey(info, "n");
	y += size;
	CG_DrawBigString(640 - (Q_PrintStrlen(name) * BIGCHAR_WIDTH), y, name, 0.5);

	return y + BIGCHAR_HEIGHT + 2;
}*/

/*
==================
CG_DrawSnapshot
==================
*/
static float CG_DrawSnapshot(float y)
{
	char		*s;
	int			w;

	s = va("time:%i snap:%i cmd:%i", cg.snap->serverTime,
		cg.latestSnapshotNum, cgs.serverCommandSequence);
	w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;

	CG_DrawBigString(635 - w, y + 2, s, 1.0F);

	return y + BIGCHAR_HEIGHT + 4;
}

/*
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES	16
static float CG_DrawFPS(float y)
{
	char		*s;
	int			w;
	static int	previousTimes[FPS_FRAMES];
	static int	index;
	int		i, total;
	//int		fps;
	static	int	previous;
	int		t, frameTime;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if (index > FPS_FRAMES)
	{
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for (i = 0; i < FPS_FRAMES; i++)
		{
			total += previousTimes[i];
		}
		if (!total)
		{
			total = 1;
		}
		cg.fps = 1000.0f * FPS_FRAMES / total;

		if (cg_drawFPS.integer)
		{
			s = va("%ifps", (int) cg.fps);
			w = CG_DrawStrlen(s) * SMALLCHAR_WIDTH;//BIGCHAR_WIDTH;
			//CG_DrawBigString(635 - w, y + 2, s, 1.0F);
			CG_DrawSmallString(635 - w, y + 1, s, 1.0f);
		}
		else
			return y;
	}

	return y + SMALLCHAR_HEIGHT + 2;
	//return y + BIGCHAR_HEIGHT + 4;
}

/*
=================
CG_DrawTimer
=================
*/
static float CG_DrawTimer(float y)
{
	char		*s;
	int			w;
	int			mins, seconds, tens;
	int			msec;

	msec = cg.time - cgs.levelStartTime;

	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

	s = va("%i:%i%i", mins, tens, seconds);
	w = CG_DrawStrlen(s) * SMALLCHAR_WIDTH;// * BIGCHAR_WIDTH;

	//CG_DrawBigString(635 - w, y + 2, s, 1.0F);
	CG_DrawSmallString(635 - w, y + 1, s, 1.0f);

	return y + SMALLCHAR_HEIGHT + 2;
	//return y + BIGCHAR_HEIGHT + 4;
}


/*
=================
CG_DrawTeamOverlay
=================
*/

static float CG_DrawTeamOverlay(float y, qboolean right, qboolean upper, qboolean First)
{
	int x, w, h, xx;
	int i, j, len;
	const char *p;
	vec4_t		hcolor;
	int pwidth, lwidth;
	int plyrs;
	char st[16];
	clientInfo_t *ci;
	gitem_t	*item;
	int ret_y, count;
	int Team = cgs.clientinfo[cg.clientNum].team;

	if (!cg_drawTeamOverlay.integer)
	{
		return y;
	}

	if (Team == TEAM_SPECTATOR)
	{
		if (First ^ upper)
			Team = TEAM_BLUE;
		else
			Team = TEAM_RED;
	}
	else if (!First)
		return y;

	if (Team != TEAM_RED && Team != TEAM_BLUE)
	{
		return y; // Not on any team
	}

	plyrs = 0;

	// max player name width
	pwidth = 0;
	count = (numSortedTeamPlayers > 12) ? 12 : numSortedTeamPlayers;
	for (i = 0; i < count; i++)
	{
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if (ci->infoValid && ci->team == Team)
		{
			plyrs++;
			len = CG_DrawStrlen(ci->name);
			if (len > pwidth)
				pwidth = len;
		}
	}

	if (!plyrs)
		return y;

	if (pwidth > TEAM_OVERLAY_MAXNAME_WIDTH)
		pwidth = TEAM_OVERLAY_MAXNAME_WIDTH;

	// max location name width
	lwidth = 0;
	/*{
		for (i = 1; i < MAX_LOCATIONS; i++)
		{
			p = CG_ConfigString(CS_LOCATIONS + i);
			if (p && *p)
			{
				len = CG_DrawStrlen(p);
				if (len > lwidth)
					lwidth = len;
			}
		}
	}*/

	if (lwidth > TEAM_OVERLAY_MAXLOCATION_WIDTH)
		lwidth = TEAM_OVERLAY_MAXLOCATION_WIDTH;

	w = (pwidth + lwidth + 4 + 7) * TINYCHAR_WIDTH;

	if (right)
		x = 640 - w;
	else
		x = 0;

	h = plyrs * TINYCHAR_HEIGHT;

	if (upper)
	{
		ret_y = y + h;
	}
	else
	{
		y -= h;
		ret_y = y;
	}

	if (Team == TEAM_RED)
	{
		ColorCopy(hcolor, RedTeamColor);
		hcolor[3] = 0.33f;
	}
	else if (Team == TEAM_BLUE)
	{
		ColorCopy(hcolor, BlueTeamColor);
		hcolor[3] = 0.33f;
	}

	trap_R_SetColor(hcolor);
	CG_DrawPic(x, y, w, h, cgs.media.teamStatusBar);
	trap_R_SetColor(NULL);

	for (i = 0; i < count; i++)
	{
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if (ci->infoValid && ci->team == Team)
		{
			hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0f;

			xx = x + TINYCHAR_WIDTH;

			CG_DrawStringExt(xx, y,
				ci->name, hcolor, qfalse, qfalse,
				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, TEAM_OVERLAY_MAXNAME_WIDTH);

			if (lwidth)
			{
				p = CG_ConfigString(CS_LOCATIONS + ci->location);
				if (!p || !*p)
					p = "unknown";
				len = CG_DrawStrlen(p);
				if (len > lwidth)
					len = lwidth;

//				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth +
//					((lwidth/2 - len/2) * TINYCHAR_WIDTH);
				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth;
				CG_DrawStringExt(xx, y,
					p, hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
					TEAM_OVERLAY_MAXLOCATION_WIDTH);
			}

			CG_GetColorForHealth(ci->health, ci->armor, hcolor);

			Com_sprintf (st, sizeof(st), "%3i %3i", ci->health,	ci->armor);

			xx = x + TINYCHAR_WIDTH * 3 +
				TINYCHAR_WIDTH * pwidth + TINYCHAR_WIDTH * lwidth;

			CG_DrawStringExt(xx, y,
				st, hcolor, qfalse, qfalse,
				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);

			// draw weapon icon
			xx += TINYCHAR_WIDTH * 3;

			if (cg_weapons[ci->curWeapon].weaponIcon)
			{
				CG_DrawPic(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
					cg_weapons[ci->curWeapon].weaponIcon);
			}
			else
			{
				CG_DrawPic(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
					cgs.media.deferShader);
			}

			// Draw powerup icons
			if (right)
			{
				xx = x;
			}
			else
			{
				xx = x + w - TINYCHAR_WIDTH;
			}
			for (j = 0; j <= PW_NUM_POWERUPS; j++)
			{
				if (ci->powerups & (1 << j))
				{
					item = BG_FindItemForPowerup(j);

					if (item != NULL)
					{
						CG_DrawPic(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
						trap_R_RegisterShader(item->icon));
						if (right)
						{
							xx -= TINYCHAR_WIDTH;
						}
						else
						{
							xx += TINYCHAR_WIDTH;
						}
					}
				}
			}

			y += TINYCHAR_HEIGHT;
		}
	}

	return ret_y;
//#endif
}


/*
=====================
CG_DrawUpperRight

=====================
*/
static void CG_DrawUpperRight(void)
{
	float	y;

	y = 0;

	if (cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 1)
	{
		y = CG_DrawTeamOverlay(y, qtrue, qtrue, qtrue);
		y = CG_DrawTeamOverlay(y, qtrue, qtrue, qfalse);
	}

	if (cg_drawSnapshot.integer)
	{
		y = CG_DrawSnapshot(y);
	}
	//if (cg_drawFPS.integer)
	{
		y = CG_DrawFPS(y);
	}
	if (cg_drawTimer.integer)
	{
		y = CG_DrawTimer(y);
	}

	y = CG_DrawKills(y);

	//if (cg_drawAttacker.integer)
		//y = CG_DrawAttacker(y);

}

/*
===========================================================================================

  LOWER RIGHT CORNER

===========================================================================================
*/

/*
=======================
//Too: CG_DrawLowArmor
=======================
*/
static float CG_DrawLowArmor(float y)
{
	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR
		|| cg.snap->ps.stats[STAT_ARMOR] >= 75
		|| cg.snap->ps.stats[STAT_ARMOR] <= 0)
	{
		return y;
	}

	y -= 64;
	CG_DrawPic(640 - (64 + 8), y, 64, 64, cgs.media.LowArmor);

	return y;
}

/*
=======================
//Too: CG_DrawRadiation
=======================
*/
static float CG_DrawRadiation(float y)
{
	float Alpha;

	if (cg.RadiationLevel <= 0)
		return y;

	if (cg.RadiationLevel > 20)
		cg.RadiationLevel = 20;

	while (cg.time - cg.RadiationTime > 500)
	{
		cg.RadiationTime += 100;

		if (!(--cg.RadiationLevel))
			return y;
	}

	Alpha = cg.RadiationLevel * (1 / 16.0);
	if (Alpha > 1.0)
		Alpha = 1.0f;

	if (Alpha > 0)
	{
		vec4_t hcolor;

		y -= 64;

		hcolor[0] = hcolor[1] = hcolor[2] = 1;
		hcolor[3] = Alpha;

		trap_R_SetColor(hcolor);
		CG_DrawPic(640 - (64 + 8), y, 64, 64, cgs.media.Radiation);
		trap_R_SetColor(NULL);
	}

	return y;
}

/*
=================
CG_DrawScores

Draw the small two score display
=================
*/
#ifndef MISSIONPACK
static float CG_DrawScores(float y)
{
	const char	*s;
	int			s1, s2, score;
	int			x, w;
	int			v;
	vec4_t		color;
	float		y1;
	gitem_t		*item;

	s1 = cgs.scores1;
	s2 = cgs.scores2;

	y -=  BIGCHAR_HEIGHT + 8;

	y1 = y;

	// draw from the right side to left
	if (cgs.gametype >= GT_TEAM)
	{
		x = 640;

		color[0] = 0;
		color[1] = 0;
		color[2] = 1;
		color[3] = 0.33f;
		s = va("%2i", s2);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH + 8;
		x -= w;
		CG_FillRect(x, y-4,  w, BIGCHAR_HEIGHT+8, color);
		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE)
		{
			CG_DrawPic(x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader);
		}
		CG_DrawBigString(x + 4, y, s, 1.0F);

		if (cgs.gametype == GT_CTF)
		{
			// Display flag status
			item = BG_FindItemForPowerup(PW_BLUEFLAG);

			if (item)
			{
				y1 = y - BIGCHAR_HEIGHT - 8;
				if (cgs.blueflag >= 0 && cgs.blueflag <= 2)
				{
					CG_DrawPic(x, y1-4, w, BIGCHAR_HEIGHT+8, cgs.media.blueFlagShader[cgs.blueflag]);
				}
			}
		}

		color[0] = 1;
		color[1] = 0;
		color[2] = 0;
		color[3] = 0.33f;
		s = va("%2i", s1);
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH + 8;
		x -= w;
		CG_FillRect(x, y-4,  w, BIGCHAR_HEIGHT+8, color);
		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED)
		{
			CG_DrawPic(x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader);
		}
		CG_DrawBigString(x + 4, y, s, 1.0F);

		if (cgs.gametype == GT_CTF)
		{
			// Display flag status
			item = BG_FindItemForPowerup(PW_REDFLAG);

			if (item)
			{
				y1 = y - BIGCHAR_HEIGHT - 8;
				if (cgs.redflag >= 0 && cgs.redflag <= 2)
				{
					CG_DrawPic(x, y1-4, w, BIGCHAR_HEIGHT+8, cgs.media.redFlagShader[cgs.redflag]);
				}
			}
		}

#ifdef MISSIONPACK
		if (cgs.gametype == GT_1FCTF)
		{
			// Display flag status
			item = BG_FindItemForPowerup(PW_NEUTRALFLAG);

			if (item)
			{
				y1 = y - BIGCHAR_HEIGHT - 8;
				if (cgs.flagStatus >= 0 && cgs.flagStatus <= 3)
				{
					CG_DrawPic(x, y1-4, w, BIGCHAR_HEIGHT+8, cgs.media.flagShader[cgs.flagStatus]);
				}
			}
		}
#endif
		if (cgs.gametype >= GT_CTF)
		{
			v = cgs.capturelimit;
		}
		else
		{
			v = cgs.fraglimit;
		}
		if (v)
		{
			s = va("%2i", v);
			w = CG_DrawStrlen(s) * BIGCHAR_WIDTH + 8;
			x -= w;
			CG_DrawBigString(x + 4, y, s, 1.0F);
		}

	}
	else
	{
		qboolean	spectator;

		x = 640;
		score = cg.snap->ps.persistant[PERS_SCORE];
		spectator = (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR);

		// always show your score in the second box if not in first place
		if (s1 != score)
		{
			s2 = score;
		}
		if (s2 != SCORE_NOT_PRESENT)
		{
			s = va("%2i", s2);
			w = CG_DrawStrlen(s) * BIGCHAR_WIDTH + 8;
			x -= w;
			if (!spectator && score == s2 && score != s1)
			{
				color[0] = 1;
				color[1] = 0;
				color[2] = 0;
				color[3] = 0.33f;
				CG_FillRect(x, y-4,  w, BIGCHAR_HEIGHT+8, color);
				CG_DrawPic(x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader);
			}
			else
			{
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;
				CG_FillRect(x, y-4,  w, BIGCHAR_HEIGHT+8, color);
			}
			CG_DrawBigString(x + 4, y, s, 1.0F);
		}

		// first place
		if (s1 != SCORE_NOT_PRESENT)
		{
			s = va("%2i", s1);
			w = CG_DrawStrlen(s) * BIGCHAR_WIDTH + 8;
			x -= w;
			if (!spectator && score == s1)
			{
				color[0] = 0;
				color[1] = 0;
				color[2] = 1;
				color[3] = 0.33f;
				CG_FillRect(x, y-4,  w, BIGCHAR_HEIGHT+8, color);
				CG_DrawPic(x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader);
			}
			else
			{
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;
				CG_FillRect(x, y-4,  w, BIGCHAR_HEIGHT+8, color);
			}
			CG_DrawBigString(x + 4, y, s, 1.0F);
		}

		if (cgs.fraglimit)
		{
			s = va("%2i", cgs.fraglimit);
			w = CG_DrawStrlen(s) * BIGCHAR_WIDTH + 8;
			x -= w;
			CG_DrawBigString(x + 4, y, s, 1.0F);
		}

	}

	return y1 - 8;
}
#endif

/*
================
CG_DrawPowerups
================
*/
#ifndef MISSIONPACK
static float CG_DrawPowerups(float y)
{
	int		sorted[MAX_POWERUPS];
	int		sortedTime[MAX_POWERUPS];
	int		i, j, k;
	int		active;
	playerState_t	*ps;
	int		t;
	gitem_t	*item;
	int		x;
	int		color;
	float	size;
	float	f;
	static float colors[2][4] =
	{
		{ 0.2f, 1.0f, 0.2f, 1.0f }
		, { 1.0f, 0.2f, 0.2f, 1.0f }
		};

	ps = &cg.snap->ps;

	if (ps->stats[STAT_HEALTH] <= 0)
	{
		return y;
	}

	// sort the list by time remaining
	active = 0;
	for (i = 1; i < MAX_POWERUPS; i++)
	{
		if (!ps->powerups[i] || i == PW_SPECIAL)
		{
			continue;
		}
		t = ps->powerups[i] - cg.time;
		// ZOID--don't draw if the power up has unlimited time (999 seconds)
		// This is true of the CTF flags
		if (t < 0 || t > 999000)
		{
			continue;
		}

		// insert into the list
		for (j = 0; j < active; j++)
		{
			if (sortedTime[j] >= t)
			{
				for (k = active - 1; k >= j; k--)
				{
					sorted[k+1] = sorted[k];
					sortedTime[k+1] = sortedTime[k];
				}
				break;
			}
		}
		sorted[j] = i;
		sortedTime[j] = t;
		active++;
	}

	// draw the icons and timers
	x = 640 - ICON_SIZE - CHAR_WIDTH * 2;
	for (i = 0; i < active; i++)
	{
		item = BG_FindItemForPowerup(sorted[i]);

		if (item)
		{
			color = 1;

			y -= ICON_SIZE;

			//trap_R_SetColor(colors[color]);
			CG_DrawField(x, y, 2, sortedTime[ i ] / 1000, 0, qtrue, colors[color]);

			t = ps->powerups[ sorted[i] ];
			if (t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME)
			{
			  trap_R_SetColor(NULL);
			}
			else
			{
				vec4_t	modulate;

				f = (float)(t - cg.time) / POWERUP_BLINK_TIME;
				f -= (int)f;
				modulate[0] = modulate[1] = modulate[2] = modulate[3] = f;
				trap_R_SetColor(modulate);
			}

			if (cg.powerupActive == sorted[i] &&
			cg.time - cg.powerupTime < PULSE_TIME)
			{
				f = 1.0 - (((float)cg.time - cg.powerupTime) / PULSE_TIME);
				size = ICON_SIZE * (1.0 + (PULSE_SCALE - 1.0) * f);
			}
			else
			{
				size = ICON_SIZE;
			}

			CG_DrawPic(640 - size, y + ICON_SIZE / 2 - size / 2,
			size, size, trap_R_RegisterShader(item->icon));
		}
	}
	trap_R_SetColor(NULL);

	return y;
}
#endif

/*
=====================
CG_DrawLowerRight

=====================
*/

#ifndef MISSIONPACK
static void CG_DrawLowerRight(void)
{
	float	y;

	y = 480 - ICON_SIZE;

	if (cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 2)
	{
		y = CG_DrawTeamOverlay(y, qtrue, qfalse, qtrue);
		y = CG_DrawTeamOverlay(y, qtrue, qfalse, qfalse);
	}

	if (cgs.gametype == GT_CTF)
		y = CG_DrawScores(y);

	y = CG_DrawPowerups(y);
	y = CG_DrawLowArmor(y);
	y = CG_DrawRadiation(y);
}
#endif

/*
===================
CG_DrawPickupItem
===================
*/
#ifndef MISSIONPACK
static int CG_DrawPickupItem(int y)
{
	int		value;
	float	*fadeColor;

	if (cg.snap->ps.stats[STAT_HEALTH] <= 0)
	{
		return y;
	}

	y -= ICON_SIZE;

	value = cg.itemPickup;
	if (value)
	{
		fadeColor = CG_FadeColor(cg.itemPickupTime, 3000);
		if (fadeColor)
		{
			CG_RegisterItemVisuals(value);
			trap_R_SetColor(fadeColor);
			CG_DrawPic(8, y, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon);
			CG_DrawBigString(ICON_SIZE + 16, y + (ICON_SIZE/2 - BIGCHAR_HEIGHT/2), bg_itemlist[ value ].pickup_name, fadeColor[0]);
			trap_R_SetColor(NULL);
		}
	}

	return y;
}
#endif


/*
==========================
//Too: CG_DrawGameTimeLeft
==========================
*/
static void CG_DrawGameTimeLeft(void)
{
	int			w;
	int			sec;
	int			cw;
	const char	*s;
	const float *setColor;

	sec = cg.GameTimeLeft;
	if (!sec)
		return;

	sec = (sec - cg.time + 1000) / 1000;
	if (sec < 0)
	{
		cgs.InvasionInfo.Period = e_Period_WaitRestart;
		sec = 0;
	}

	s = va("%d:%02d", sec / 60, sec % 60);

	if (sec != cg.GameTimeLeftCount && cgs.InvasionInfo.Period == e_Period_Playing)
	{
		sfxHandle_t *sfx = NULL;

		cg.GameTimeLeftCount = sec;

		switch (sec)
		{
		case 1:
			sfx = &cgs.media.count1Sound;
			break;
		case 2:
			sfx = &cgs.media.count2Sound;
			break;
		case 3:
			sfx = &cgs.media.count3Sound;
			break;
		default:
			break;
		}

		if (sfx != NULL)
			trap_S_StartLocalSound(*sfx, CHAN_ANNOUNCER);
	}

	w = CG_DrawStrlen(s);
	cw = 16;

	if (sec < 10)
		setColor = colorRed;
	else
		setColor = colorGreen;

	CG_DrawStringExt(640 - (w * cw + cw/2), 480 - (int) (cw * 2.25), s, setColor,
			qfalse, qtrue, cw, (int)(cw * 1.5), 0);
}



/*
==================
//Too: CG_DrawRadar
==================
*/

enum
{
	es_RadarX = 64 + 8,
	es_RadarY = 64 + 16,
	es_RadarRadius = 64,
	es_GoalSize = 4,
	es_GoalSizeO = 6,
};


static void CG_DrawRadar(void)
{
	int g, up, angle, radius;
	int u, v, Size;
	centity_t *cent;
	float ang, Yaw;
	const char *s = cg.Radar;
	qhandle_t Shader;
	vec4_t Quat;
	int Current = (cg.predictedPlayerState.stats[STAT_SPEC1] << 16)
			+ (cg.predictedPlayerState.stats[STAT_SPEC2] & 65535);

	if (cg.NightVision)
		return;

	if (Current)
	{
		vec3_t Matrix[3];

		AngleVectors(cg.predictedPlayerState.viewangles, Matrix[0], Matrix[1], Matrix[2]);
		Inv_GetQuatFromStat(Current, Quat, NULL);
		Inv_QuatMultiply(Quat, Matrix);

		Matrix[0][2] = 0;
		if (!VectorNormalize(Matrix[0]))
		{
			Yaw = 0;
		}
		else
		{
			Yaw = acos(Matrix[0][0]);
			if (Matrix[0][1] > 0)
				Yaw = -Yaw;

			Yaw *= 180 / M_PI;
		}
	}
	else
		Yaw = cg.refdefViewAngles[YAW];

	CG_DrawPic(es_RadarX - es_RadarRadius, es_RadarX - es_RadarRadius, es_RadarRadius * 2, es_RadarRadius * 2, cgs.media.InvRadar);
	cent = &cg_entities[cg.snap->ps.clientNum];

	while (s[0])
	{
		vec4_t MarineColor,// = { 1.0f, 0.5f, 0.0f, 1.0f },
				AlienColor;// = { 0.0f, 0.5f, 1.0f, 1.0f };
		int Team;
		int n = Inv_NumFromString(s, 3);
		s += 3;

		ColorCopy(MarineColor, RedTeamColor);
		ColorCopy(AlienColor, BlueTeamColor);

		assert(s - cg.Radar < sizeof(cg.Radar));

		Team = (n >> (es_InvRadar_Radius + es_InvRadar_Angle + es_InvRadar_Height)) & ((1 << es_InvRadar_Team) - 1);
		up = (n >> (es_InvRadar_Radius + es_InvRadar_Angle)) & ((1 << es_InvRadar_Height) - 1);
		angle = (n >> es_InvRadar_Radius) & ((1 << es_InvRadar_Angle) - 1);
		radius = n & ((1 << es_InvRadar_Radius) - 1);

		assert(up);
		if (!up)
			continue;

		angle = angle * (360.0 / 63.0);

		angle = (Yaw - angle);
		ang = angle * (M_PI * 2.0 / 360.0) - M_PI / 2;

		u = cos(ang) * (es_RadarRadius - 1) * radius / 256.0f;
		v = sin(ang) * (es_RadarRadius - 1) * radius / 256.0f;

		switch (up)
		{
			case 1:
				Size = es_GoalSizeO;
				Shader = cgs.media.InvRadarDotUp;
				break;
			case 2:
				Size = es_GoalSizeO;
				Shader = cgs.media.InvRadarDotDown;
				break;
			default:
				Size = es_GoalSize;
				Shader = cgs.media.InvRadarDot;
				break;
		}

		if (!Team)
			trap_R_SetColor(MarineColor);
		else
			trap_R_SetColor(AlienColor);

		CG_DrawPic(es_RadarX + u - (Size >> 1), es_RadarX + v - (Size >> 1),
								Size, Size, Shader);

		trap_R_SetColor(NULL);
	}

	s = NULL;

	while (1)
	{
		if (s == NULL)
		{
			g = cg.snap->ps.stats[STAT_GOAL_RADAR];
			s = cg.RadarGoal;
		}
		else
		{
			vec3_t Goal;
			int i;

			if (!s[0])
				break;

			for (i = 0; i < 3; ++i)
			{
				Goal[i] = (Inv_NumFromString(s, 3) << 14) >> 14;
				s += 3;
			}

			g  = Inv_RelativePosition(Goal, cg.predictedPlayerState.origin);
		}

		up = (g >> 14) & 3;

		if (!up)
			continue;

		angle = (g >> 8) & 63;
		radius = g & 255;

		angle = angle * (360.0 / 63.0);

		angle = (Yaw - angle);
		ang = angle * (M_PI * 2.0 / 360.0) - M_PI / 2;

		u = cos(ang) * (es_RadarRadius - 1) * radius / 256.0f;
		v = sin(ang) * (es_RadarRadius - 1) * radius / 256.0f;

		switch (up)
		{
			case 1:
				Size = es_GoalSizeO;
				Shader = cgs.media.InvGoalUp;
				break;
			case 2:
				Size = es_GoalSizeO;
				Shader = cgs.media.InvGoalDown;
				break;
			default:
				Size = es_GoalSize;
				Shader = cgs.media.InvGoal;
				break;
		}

		CG_DrawPic(es_RadarX + u - (Size >> 1), es_RadarX + v - (Size >> 1),
								Size, Size, Shader);
	}
}


/*
==================
//Too: CG_DrawInGameHelp
==================
*/

char *IGH_GetNextLine(char *str, int *OffSet)
{
	int i = 0;
	static char buf[128];

	if (str[0] == 0)
		return NULL;

	while (str[i] != '\n' && str[i] != 0 && i < sizeof(buf) - 1)
	{
		buf[i] = str[i];
		++i;
	}

	buf[i] = 0;

	if (str[i] == '\n')
		++i;
	*OffSet += i;

	return buf;
}

static void CG_DrawInGameHelp(void)
{
	vec4_t hcolor;
	int x = 8,
		y = 240;
	char var[MAX_TOKEN_CHARS];
	char *str[5] = { NULL, NULL, NULL, NULL, NULL },
		*prt;
	clientInfo_t *ci = cgs.clientinfo + cg.snap->ps.clientNum;
	int j;

	if (!Inv_cg_DrawInGameHelp.integer)
		return;

	trap_Cvar_VariableStringBuffer("inv_IGM", var, sizeof(var));
	if (atoi(var))
	{
		cg.InGameHelpStartTime = cg.InGameHelpEndTime = 0;
		return;
	}

	if (cg_paused.integer)
		return;

	if (cg.time > cg.InGameHelpEndTime || cg.time < cg.InGameHelpStartTime)
	{
		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
		{
			if (cg.InGameHelpStartTime == 0 && !(cg_paused.integer))
			{
				cg.InGameHelpStartTime = cg.time + 0.5 * 1000;
				cg.InGameHelpEndTime = cg.InGameHelpStartTime + 13 * 1000;
			}
		}

		return;
	}

	if (cg.snap->ps.persistant[PERS_TEAM] == cgs.InvasionInfo.MarineTeam)
	{
		int Armor = ci->Class & e_Class_MarineArmorMask;
		int Weapon = (ci->Class >> e_Class_MarineWeaponDec) & e_Class_MarineWeaponMask;

		str[0] = IGH_Armor[Armor];
		str[1] = IGH_Weapon[Weapon];
		str[2] = IGH_MarineGoal[cgs.gametype];
	}
	else if (cg.snap->ps.persistant[PERS_TEAM] == cgs.InvasionInfo.AlienTeam)
	{
		int Race = ci->Class & e_Class_AlienRaceMask;

		str[0] = IGH_Race[Race];
		str[1] = IGH_AlienGoal[cgs.gametype];
	}
	else
	{
		str[0] = "-- Spectator --\nFire1 to follow next player\nFire2 to follow previous player\nJump to exit follow mode\nPrev/Next Weapon to change follow mode";
	}

	if (cg.time > cg.InGameHelpEndTime - 2 * 1000)
		hcolor[3] = (cg.InGameHelpEndTime - cg.time) * (0.75 / 2000.0);
	else if (cg.time < cg.InGameHelpStartTime + 0.5 * 1000)
		hcolor[3] = (cg.time - cg.InGameHelpStartTime) * (0.75 / 500.0);
	else
		hcolor[3] = 0.75;

	hcolor[0] = 0.125;
	hcolor[1] = 0.125;
	hcolor[2] = 0.125;

	CG_FillRect(x - 4, y - 4, 320, SMALLCHAR_HEIGHT * 9 + 8, hcolor);

	hcolor[0] = 1;
	hcolor[1] = 0.5;
	hcolor[2] = 0.25;

	CG_DrawSmallStringColor(x, y, "--- In-Game Help ---", hcolor);

	if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR)
	{
		y += SMALLCHAR_HEIGHT;
		CG_DrawSmallStringColor(x, y, "You selected:", hcolor);
	}

	j = 0;
	while (str[j] != NULL)
	{
		int i = 0;

		if (str[j + 1] == NULL)
			y += SMALLCHAR_HEIGHT;

		while ((prt = IGH_GetNextLine(str[j] + i, &i)) != NULL)
		{
			y += SMALLCHAR_HEIGHT;
			CG_DrawSmallStringColor(x, y, prt, hcolor);
		}

		++j;
	}
}

/*
=====================
CG_DrawLowerLeft

=====================
*/
#ifndef MISSIONPACK
static void CG_DrawLowerLeft(void)
{
	float	y;

	y = 480 - ICON_SIZE;

	if (cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 3)
	{
		y = CG_DrawTeamOverlay(y, qfalse, qfalse, qtrue);
		y = CG_DrawTeamOverlay(y, qfalse, qfalse, qfalse);
	}

	y = CG_DrawPickupItem(y);
}
#endif


//===========================================================================================

/*
=================
CG_DrawTeamInfo
=================
*/
#ifndef MISSIONPACK
static void CG_DrawTeamInfo(void)
{
	int w, h;
	int i, len;
	vec4_t		hcolor;
	int		chatHeight;

#define CHATLOC_Y 420 // bottom end
#define CHATLOC_X 0

	if (cg_teamChatHeight.integer < TEAMCHAT_HEIGHT)
		chatHeight = cg_teamChatHeight.integer;
	else
		chatHeight = TEAMCHAT_HEIGHT;
	if (chatHeight <= 0)
		return; // disabled

	if (cgs.teamLastChatPos != cgs.teamChatPos)
	{
		if (cg.time - cgs.teamChatMsgTimes[cgs.teamLastChatPos % chatHeight] > cg_teamChatTime.integer)
		{
			CG_Printf("%s\n", cgs.teamChatMsgs[cgs.teamLastChatPos % chatHeight]);
			cgs.teamLastChatPos++;
		}

		h = (cgs.teamChatPos - cgs.teamLastChatPos) * TINYCHAR_HEIGHT;

		w = 0;

		for (i = cgs.teamLastChatPos; i < cgs.teamChatPos; i++)
		{
			len = CG_DrawStrlen(cgs.teamChatMsgs[i % chatHeight]);
			if (len > w)
				w = len;
		}
		w *= TINYCHAR_WIDTH;
		w += TINYCHAR_WIDTH * 2;

		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED)
		{
			ColorCopy(hcolor, RedTeamColor);
			hcolor[3] = 0.6f;//0.33f;
		}
		else if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE)
		{
			ColorCopy(hcolor, BlueTeamColor);
			hcolor[3] = 0.6f;//0.33f;
		}
		else
		{
			hcolor[0] = 0;
			hcolor[1] = 0.5f;
			hcolor[2] = 0;
			hcolor[3] = 0.6f;//0.33f;
		}

		trap_R_SetColor(hcolor);
		CG_DrawPic(CHATLOC_X, CHATLOC_Y - h, 640, h, cgs.media.teamStatusBar);
		trap_R_SetColor(NULL);

		hcolor[0] = hcolor[1] = hcolor[2] = 1.0f;
		hcolor[3] = 1.0f;

		for (i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i--)
		{
			CG_DrawStringExt(CHATLOC_X + TINYCHAR_WIDTH,
				CHATLOC_Y - (cgs.teamChatPos - i)*TINYCHAR_HEIGHT,
				cgs.teamChatMsgs[i % chatHeight], hcolor, qfalse, qfalse,
				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);
		}
	}
}
#endif

/*
===================
CG_DrawHoldableItem
===================
*/
#ifndef MISSIONPACK
static void CG_DrawHoldableItem(void)
{
	int		value;

	value = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
	if (value)
	{
		CG_RegisterItemVisuals(value);
		CG_DrawPic(640-ICON_SIZE, (SCREEN_HEIGHT-ICON_SIZE)/2, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon);
	}

}
#endif

#ifdef MISSIONPACK
/*
===================
CG_DrawPersistantPowerup
===================
*/
/*
static void CG_DrawPersistantPowerup(void)
{
	int		value;

	value = cg.snap->ps.stats[STAT_PERSISTANT_POWERUP];
	if (value)
	{
		CG_RegisterItemVisuals(value);
		CG_DrawPic(640-ICON_SIZE, (SCREEN_HEIGHT-ICON_SIZE)/2 - ICON_SIZE, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon);
	}
}
*/

#endif


/*
===================
CG_DrawReward
===================
*/
static void CG_DrawReward(void)
{
	float	*color;
	int		i, count;
	float	x, y;
	char	buf[32];

	if (!cg_drawRewards.integer)
	{
		return;
	}

	color = CG_FadeColor(cg.rewardTime, REWARD_TIME);
	if (!color)
	{
		if (cg.rewardStack > 0)
		{
			for (i = 0; i < cg.rewardStack; i++)
			{
				cg.rewardSound[i] = cg.rewardSound[i+1];
				cg.rewardShader[i] = cg.rewardShader[i+1];
				cg.rewardCount[i] = cg.rewardCount[i+1];
			}
			cg.rewardTime = cg.time;
			cg.rewardStack--;
			color = CG_FadeColor(cg.rewardTime, REWARD_TIME);
			trap_S_StartLocalSound(cg.rewardSound[0], CHAN_ANNOUNCER);
		}
		else
		{
			return;
		}
	}

	trap_R_SetColor(color);

	/*
	count = cg.rewardCount[0]/10;				// number of big rewards to draw

	if (count)
	{
		y = 4;
		x = 320 - count * ICON_SIZE;
		for (i = 0; i < count; i++)
		{
			CG_DrawPic(x, y, (ICON_SIZE*2)-4, (ICON_SIZE*2)-4, cg.rewardShader[0]);
			x += (ICON_SIZE*2);
		}
	}

	count = cg.rewardCount[0] - count*10;		// number of small rewards to draw
	*/

	if (cg.rewardCount[0] >= 10)
	{
		y = 56;
		x = 320 - ICON_SIZE/2;
		CG_DrawPic(x, y, ICON_SIZE-4, ICON_SIZE-4, cg.rewardShader[0]);
		Com_sprintf(buf, sizeof(buf), "%d", cg.rewardCount[0]);
		x = (SCREEN_WIDTH - SMALLCHAR_WIDTH * CG_DrawStrlen(buf)) / 2;
		CG_DrawStringExt(x, y+ICON_SIZE, buf, color, qfalse, qtrue,
								SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0);
	}
	else
	{

		count = cg.rewardCount[0];

		y = 56;
		x = 320 - count * ICON_SIZE/2;
		for (i = 0; i < count; i++)
		{
			CG_DrawPic(x, y, ICON_SIZE-4, ICON_SIZE-4, cg.rewardShader[0]);
			x += ICON_SIZE;
		}
	}
	trap_R_SetColor(NULL);
}


/*
===============================================================================

LAGOMETER

===============================================================================
*/

#define	LAG_SAMPLES		128


typedef struct
{
	int		frameSamples[LAG_SAMPLES];
	int		frameCount;
	int		snapshotFlags[LAG_SAMPLES];
	int		snapshotSamples[LAG_SAMPLES];
	int		snapshotCount;
}
lagometer_t;

lagometer_t		lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo(void)
{
	int			offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[ lagometer.frameCount & (LAG_SAMPLES - 1) ] = offset;
	lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void CG_AddLagometerSnapshotInfo(snapshot_t *snap)
{
	// dropped packet
	if (!snap)
	{
		lagometer.snapshotSamples[ lagometer.snapshotCount & (LAG_SAMPLES - 1) ] = -1;
		lagometer.snapshotCount++;
		return;
	}

	// add this snapshot's info
	lagometer.snapshotSamples[ lagometer.snapshotCount & (LAG_SAMPLES - 1) ] = snap->ping;
	lagometer.snapshotFlags[ lagometer.snapshotCount & (LAG_SAMPLES - 1) ] = snap->snapFlags;
	lagometer.snapshotCount++;
}

/*
==============
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect(void)
{
	float		x, y;
	int			cmdNum;
	usercmd_t	cmd;
	const char		*s;
	int			w;

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap_GetUserCmd(cmdNum, &cmd);
	if (cmd.serverTime <= cg.snap->ps.commandTime
		|| cmd.serverTime > cg.time) {	// special check for map_restart
		return;
	}

	// also add text in center of screen
	s = "Connection Interrupted";
	w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
	CG_DrawBigString(320 - w/2, 100, s, 1.0F);

	// blink the icon
	if ((cg.time >> 9) & 1)
	{
		return;
	}

	x = 640 - 48;
	y = 480 - 48;

	CG_DrawPic(x, y, 48, 48, trap_R_RegisterShader("gfx/2d/net.tga"));
}


#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300

/*
==============
CG_DrawLagometer
==============
*/
static void CG_DrawLagometer(void)
{
	int		a, x, y, i;
	float	v;
	float	ax, ay, aw, ah, mid, range;
	int		color;
	float	vscale;

	if (!cg_lagometer.integer || cgs.localServer)
	{
		CG_DrawDisconnect();
		return;
	}

	//
	// draw the graph
	//
#ifdef MISSIONPACK
	x = 640 - 48;
	y = 480 - 144;
#else
	x = 640 - 48;
	y = 480 - 48;
#endif

	trap_R_SetColor(NULL);
	CG_DrawPic(x, y, 48, 48, cgs.media.lagometerShader);

	ax = x;
	ay = y;
	aw = 48;
	ah = 48;
	CG_AdjustFrom640(&ax, &ay, &aw, &ah);

	color = -1;
	range = ah / 3;
	mid = ay + range;

	vscale = range / MAX_LAGOMETER_RANGE;

	// draw the frame interpoalte / extrapolate graph
	for (a = 0; a < aw; a++)
	{
		i = (lagometer.frameCount - 1 - a) & (LAG_SAMPLES - 1);
		v = lagometer.frameSamples[i];
		v *= vscale;
		if (v > 0)
		{
			if (color != 1)
			{
				color = 1;
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
			}
			if (v > range)
			{
				v = range;
			}
			trap_R_DrawStretchPic (ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		}
		else if (v < 0)
		{
			if (color != 2)
			{
				color = 2;
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_BLUE)]);
			}
			v = -v;
			if (v > range)
			{
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for (a = 0; a < aw; a++)
	{
		i = (lagometer.snapshotCount - 1 - a) & (LAG_SAMPLES - 1);
		v = lagometer.snapshotSamples[i];
		if (v > 0)
		{
			if (lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED)
			{
				if (color != 5)
				{
					color = 5;	// YELLOW for rate delay
					trap_R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
				}
			}
			else
			{
				if (color != 3)
				{
					color = 3;
					trap_R_SetColor(g_color_table[ColorIndex(COLOR_GREEN)]);
				}
			}
			v = v * vscale;
			if (v > range)
			{
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		}
		else if (v < 0)
		{
			if (color != 4)
			{
				color = 4;		// RED for dropped snapshots
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_RED)]);
			}
			trap_R_DrawStretchPic(ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader);
		}
	}

	trap_R_SetColor(NULL);

	if (cg_nopredict.integer || cg_synchronousClients.integer)
	{
		CG_DrawBigString(ax, ay, "snc", 1.0);
	}

	CG_DrawDisconnect();
}



/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_CenterPrint(const char *str, int y, int charWidth)
{
	char	*s;

	Q_strncpyz(cg.centerPrint, str, sizeof(cg.centerPrint));

	cg.centerPrintTime = cg.time;
	cg.centerPrintY = y;
	cg.centerPrintCharWidth = charWidth;

	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while(*s)
	{
		if (*s == '\n')
			cg.centerPrintLines++;
		s++;
	}
}


/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_RightPrint(const char *str, int y, int charWidth)
{
	char	*s;

	Q_strncpyz(cg.RightPrint, str, sizeof(cg.RightPrint));

	cg.RightPrintTime = cg.time;
	cg.RightPrintY = y;
	cg.RightPrintCharWidth = charWidth;

	// count the number of lines for centering
	cg.RightPrintLines = 1;
	s = cg.RightPrint;
	while(*s)
	{
		if (*s == '\n')
			cg.RightPrintLines++;
		s++;
	}
}


/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString(void)
{
	char	*start;
	int		l;
	int		x, y, w, h;
	float	*color;

	if (!cg.centerPrintTime)
	{
		return;
	}

	color = CG_FadeColor(cg.centerPrintTime, 1000 * cg_centertime.value);
	if (!color)
	{
		cg.centerPrintTime = 0;
		return;
	}

	trap_R_SetColor(color);

	start = cg.centerPrint;

	y = cg.centerPrintY - cg.centerPrintLines * BIGCHAR_HEIGHT / 2;

	while (1)
	{
		char linebuffer[1024];

		for (l = 0; l < 50; l++)
		{
			if (!start[l] || start[l] == '\n')
			{
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

#ifdef MISSIONPACK
		w = CG_Text_Width(linebuffer, 0.5, 0);
		h = CG_Text_Height(linebuffer, 0.5, 0);
		x = (SCREEN_WIDTH - w) / 2;
		CG_Text_Paint(x, y + h, 0.5, color, linebuffer, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
		y += h + 6;
#else
		w = cg.centerPrintCharWidth * CG_DrawStrlen(linebuffer);

		x = (SCREEN_WIDTH - w) / 2;

		CG_DrawStringExt(x, y, linebuffer, color, qfalse, qtrue,
			cg.centerPrintCharWidth, (int)(cg.centerPrintCharWidth * 1.5), 0);

		y += cg.centerPrintCharWidth * 1.5;
#endif
		while (*start && (*start != '\n'))
		{
			start++;
		}
		if (!*start)
		{
			break;
		}
		start++;
	}

	trap_R_SetColor(NULL);
}


/*
===================
CG_DrawRightString
===================
*/
static void CG_DrawRightString(void)
{
	char	*start;
	int		l;
	int		x, y, w, h;
	float	*color;

	if (!cg.RightPrintTime)
	{
		return;
	}

	color = CG_FadeColor(cg.RightPrintTime, 1000 * cg_centertime.value);
	if (!color)
	{
		cg.RightPrintTime = 0;
		return;
	}

	trap_R_SetColor(color);

	start = cg.RightPrint;

	y = cg.RightPrintY - cg.RightPrintLines * BIGCHAR_HEIGHT / 2;

	while (1)
	{
		char linebuffer[1024];

		for (l = 0; l < 50; l++)
		{
			if (!start[l] || start[l] == '\n')
			{
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

#ifdef MISSIONPACK
		w = CG_Text_Width(linebuffer, 0.5, 0);
		h = CG_Text_Height(linebuffer, 0.5, 0);
		x = (SCREEN_WIDTH - w);
		CG_Text_Paint(x, y + h, 0.5, color, linebuffer, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
		y += h + 6;
#else
		w = cg.RightPrintCharWidth * CG_DrawStrlen(linebuffer);

		x = (SCREEN_WIDTH - w);

		CG_DrawStringExt(x, y, linebuffer, color, qfalse, qtrue,
			cg.RightPrintCharWidth, (int)(cg.RightPrintCharWidth * 1.5), 0);

		y += cg.RightPrintCharWidth * 1.5;
#endif
		while (*start && (*start != '\n'))
		{
			start++;
		}
		if (!*start)
		{
			break;
		}
		start++;
	}

	trap_R_SetColor(NULL);
}



/*
================================================================================

CROSSHAIR

================================================================================
*/


/*
=================
CG_DrawCrosshair
=================
*/
static void CG_DrawCrosshair(void)
{
	float		w, h;
	qhandle_t	hShader;
	float		f;
	float		x, y;
	int			ca;

	if (!cg_drawCrosshair.integer)
	{
		return;
	}

	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR && Inv_cg_drawCrosshairSpectator.integer == 0)
	{
		return;
	}

	if (cg.renderingThirdPerson)
	{
		return;
	}

	// set color based on health
	if (cg_crosshairHealth.integer)
	{
		vec4_t		hcolor;

		CG_ColorForHealth(hcolor);
		trap_R_SetColor(hcolor);
	}
	else
	{
		trap_R_SetColor(NULL);
	}

	if (cg.NightVision >= 2 && !cg.Zoom
		&& cg.snap->ps.persistant[PERS_TEAM] == cgs.InvasionInfo.MarineTeam)
	{
		w = h = 64;
	}
	else
		w = h = cg_crosshairSize.value;

	// pulse the size of the crosshair when picking up items
	f = cg.time - cg.itemPickupBlendTime;
	if (f > 0 && f < ITEM_BLOB_TIME)
	{
		f /= ITEM_BLOB_TIME;
		w *= (1 + f);
		h *= (1 + f);
	}

	if (cg.NightVision >= 2 && !cg.Zoom
		&& cg.snap->ps.persistant[PERS_TEAM] == cgs.InvasionInfo.MarineTeam)
	{
		x = y = 0;
	}
	else
	{
		x = cg_crosshairX.integer;
		y = cg_crosshairY.integer;
	}

	CG_AdjustFrom640(&x, &y, &w, &h);

	ca = cg_drawCrosshair.integer;
	if (ca < 0)
		ca = 0;

	if (cg.NightVision >= 2 && !cg.Zoom
		&& cg.snap->ps.persistant[PERS_TEAM] == cgs.InvasionInfo.MarineTeam)
	{
		hShader = cgs.media.crosshairShader[9];
	}
	else
		hShader = cgs.media.crosshairShader[ca % NUM_CROSSHAIRS];

	trap_R_DrawStretchPic(x + cg.refdef.x + 0.5 * (cg.refdef.width - w),
		y + cg.refdef.y + 0.5 * (cg.refdef.height - h),
		w, h, 0, 0, 1, 1, hShader);
}



/*
=================
CG_ScanForCrosshairEntity
=================
*/
static void CG_ScanForCrosshairEntity(void)
{
	trace_t		trace;
	vec3_t		start, end;
	int			content;

	VectorCopy(cg.refdef.vieworg, start);
	VectorMA(start, 131072, cg.refdef.viewaxis[0], end);

	CG_Trace(&trace, start, vec3_origin, vec3_origin, end,
		cg.snap->ps.clientNum, CONTENTS_SOLID|CONTENTS_BODY);
	if (trace.entityNum >= MAX_CLIENTS)
	{
		return;
	}

	// if the player is in fog, don't show it
	content = trap_CM_PointContents(trace.endpos, 0);
	if (content & CONTENTS_FOG)
	{
		return;
	}

	// if the player is invisible, don't show it
	if (cg_entities[ trace.entityNum ].currentState.powerups & (1 << PW_INVIS))
	{
		return;
	}

	// update the fade timer
	cg.crosshairClientNum = trace.entityNum;
	cg.crosshairClientTime = cg.time;
}


/*
=====================
CG_DrawCrosshairNames
=====================
*/
static void CG_DrawCrosshairNames(void)
{
	float		*color;
	char		*name;
	float		w;

	if (!cg_drawCrosshair.integer
		|| !cg_drawCrosshairNames.integer
		|| cg.renderingThirdPerson)
	{
		return;
	}

	// scan the known entities to see if the crosshair is sighted on one
	CG_ScanForCrosshairEntity();

	// draw the name of the player being looked at
	color = CG_FadeColor(cg.crosshairClientTime, 1000);
	if (!color)
	{
		cg.crosshairClientTime = 0;
		trap_R_SetColor(NULL);
		return;
	}

	if (cg.NightVision && !cg.Zoom)
	{
		if (cg.NightVision >= 2 && cg.snap->ps.persistant[PERS_TEAM] == cgs.InvasionInfo.MarineTeam)
		{
			centity_t *cent = cg_entities + cg.crosshairClientNum;
			float Dist, Speed = VectorLength(cent->currentState.pos.trDelta);
			vec3_t Delta;

			if (cg.NightVision == 2)
			{
				color[0] = 0.3f;
				color[1] = 1.0f;
				color[2] = 0.3f;
				color[3] *= 0.5f;
			}
			else
			{
				color[0] = 0.03f;
				color[1] = 0.1f;
				color[2] = 0.03f;
				color[3] *= 0.75f;
			}

			VectorSubtract(cg.predictedPlayerState.origin, cent->currentState.pos.trBase, Delta);
			Dist = VectorLength(Delta);

			Speed *= (60 * 60 / 1000.0f) * 0.9f * 3.7f / 100;
			name = va("%02dkmh", (int) Speed);
			CG_DrawStringExt(320 + 8, 240 - 28, name, color, qtrue, qfalse, SMALLCHAR_WIDTH, 12, 0);

			Dist *= 0.9f * 3.7f / 100;
			name = va("%03dm", (int) Dist);
			CG_DrawStringExt(320 + 3, 240 + 29 - 12, name, color, qtrue, qfalse, SMALLCHAR_WIDTH, 12, 0);
		}

		return;
	}

	name = cgs.clientinfo[cg.crosshairClientNum].name;
#ifdef MISSIONPACK
	color[3] *= 0.5;
	w = CG_Text_Width(name, 0.3f, 0);
	CG_Text_Paint(320 - w / 2, 190, 0.3f, color, name, 0, 0, ITEM_TEXTSTYLE_SHADOWED);
#else
	w = CG_DrawStrlen(name) * SMALLCHAR_WIDTH;
	CG_DrawSmallString(320 - w / 2, 200, name, color[3] * 0.5f);
#endif

	if (cgs.clientinfo[cg.crosshairClientNum].team == cgs.InvasionInfo.AlienTeam &&
		cg.snap->ps.persistant[PERS_TEAM] == cgs.InvasionInfo.MarineTeam)
		return;

	//Too: print life & armor
	{
		if (cgs.clientinfo[cg.crosshairClientNum].health || cgs.clientinfo[cg.crosshairClientNum].armor)
		{
			if (cg.snap->ps.persistant[PERS_TEAM] == cgs.InvasionInfo.AlienTeam)
			{
				name = va("%d", cgs.clientinfo[cg.crosshairClientNum].health);
				w = CG_DrawStrlen(name) * SMALLCHAR_WIDTH;
				CG_DrawSmallString(320 - w / 2, 200 - SMALLCHAR_HEIGHT, name, color[3] * 0.5);
			}
			else
			{
				name = va("%d/%d", cgs.clientinfo[cg.crosshairClientNum].health, cgs.clientinfo[cg.crosshairClientNum].armor);
				w = CG_DrawStrlen(name) * SMALLCHAR_WIDTH;
				CG_DrawSmallString(320 - w / 2, 200 - SMALLCHAR_HEIGHT, name, color[3] * 0.5);
			}
		}
	}

	trap_R_SetColor(NULL);
}


//==============================================================================

/*
=================
CG_DrawSpectator
=================
*/
static void CG_DrawSpectator(void)
{
	CG_DrawBigString(320 - 9 * 8, 440, "SPECTATOR", 1.0F);
	if (cgs.gametype == GT_TOURNAMENT)
	{
		CG_DrawBigString(320 - 15 * 8, 460, "waiting to play", 1.0F);
	}
	else if (cgs.gametype >= GT_TEAM)
	{
		clientInfo_t *ci = &cgs.clientinfo[cg.snap->ps.clientNum];

		if (ci->team == ci->BaseTeam)
			CG_DrawBigString(320 - 39 * 8, 460, "press ESC and use the JOIN menu to play", 1.0F);
		else
			CG_DrawBigString(320 - 30 * 8, 460, "Ghost ! Wait next round starts", 1.0F);
	}
}

/*
=================
CG_DrawVote
=================
*/
static void CG_DrawVote(void)
{
	char	*s;
	int		sec;

	if (!cgs.voteTime)
	{
		return;
	}

	// play a talk beep whenever it is modified
	if (cgs.voteModified)
	{
		cgs.voteModified = qfalse;
		trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
	}

	sec = (VOTE_TIME - (cg.time - cgs.voteTime)) / 1000;
	if (sec < 0)
	{
		sec = 0;
	}
#ifdef MISSIONPACK
	s = va("VOTE(%i):%s yes:%i no:%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo);
	CG_DrawSmallString(0, 58, s, 1.0F);
	s = "or press ESC then click Vote";
	CG_DrawSmallString(0, 58 + SMALLCHAR_HEIGHT + 2, s, 1.0F);
#else
	s = va("VOTE(%i):%s yes:%i no:%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo);
	CG_DrawSmallString(0, 58, s, 1.0F);
#endif
}

/*
=================
CG_DrawTeamVote
=================
*/
static void CG_DrawTeamVote(void)
{
	char	*s;
	int		sec, cs_offset;

	if (cgs.clientinfo->team == TEAM_RED)
		cs_offset = 0;
	else if (cgs.clientinfo->team == TEAM_BLUE)
		cs_offset = 1;
	else
		return;

	if (!cgs.teamVoteTime[cs_offset])
	{
		return;
	}

	// play a talk beep whenever it is modified
	if (cgs.teamVoteModified[cs_offset])
	{
		cgs.teamVoteModified[cs_offset] = qfalse;
		trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
	}

	sec = (VOTE_TIME - (cg.time - cgs.teamVoteTime[cs_offset])) / 1000;
	if (sec < 0)
	{
		sec = 0;
	}
	s = va("TEAMVOTE(%i):%s yes:%i no:%i", sec, cgs.teamVoteString[cs_offset],
							cgs.teamVoteYes[cs_offset], cgs.teamVoteNo[cs_offset]);
	CG_DrawSmallString(0, 90, s, 1.0F);
}


static qboolean CG_DrawScoreboard()
{
#ifdef MISSIONPACK
	static qboolean firstTime = qtrue;
	float fade, *fadeColor;

	if (menuScoreboard)
	{
		menuScoreboard->window.flags &= ~WINDOW_FORCED;
	}
	if (cg_paused.integer)
	{
		cg.deferredPlayerLoading = 0;
		firstTime = qtrue;
		return qfalse;
	}

	// should never happen in Team Arena
	if (cgs.gametype == GT_SINGLE_PLAYER && cg.predictedPlayerState.pm_type == PM_INTERMISSION)
	{
		cg.deferredPlayerLoading = 0;
		firstTime = qtrue;
		return qfalse;
	}

	// don't draw scoreboard during death while warmup up
	if (cg.warmup && !cg.showScores && cg.predictedPlayerState.pm_type != PM_INTERMISSION)
	{
		return qfalse;
	}

	if (cg.showScores || cg.predictedPlayerState.pm_type == PM_DEAD || cg.predictedPlayerState.pm_type == PM_INTERMISSION)
	{
		fade = 1.0f;
		fadeColor = colorWhite;
	}
	else
	{
		fadeColor = CG_FadeColor(cg.scoreFadeTime, FADE_TIME);
		if (!fadeColor)
		{
			// next time scoreboard comes up, don't print killer
			cg.deferredPlayerLoading = 0;
			cg.killerName[0] = 0;
			firstTime = qtrue;
			return qfalse;
		}
		fade = *fadeColor;
	}


	if (menuScoreboard == NULL)
	{
		if (cgs.gametype >= GT_TEAM)
		{
			menuScoreboard = Menus_FindByName("teamscore_menu");
		}
		else
		{
			menuScoreboard = Menus_FindByName("score_menu");
		}
	}

	if (menuScoreboard)
	{
		if (firstTime)
		{
			CG_SetScoreSelection(menuScoreboard);
			firstTime = qfalse;
		}
		Menu_Paint(menuScoreboard, qtrue);
	}

	// load any models that have been deferred
	if (++cg.deferredPlayerLoading > 10)
	{
		CG_LoadDeferredPlayers();
	}

	return qtrue;
#else
	return CG_DrawOldScoreboard();
#endif
}

/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission(void)
{
//	int key;
#ifdef MISSIONPACK
	//if (cg_singlePlayer.integer)
	//{
	//	CG_DrawCenterString();
	//	return;
	//}
#else
	if (cgs.gametype == GT_SINGLE_PLAYER)
	{
		CG_DrawCenterString();
		return;
	}
#endif
	cg.scoreFadeTime = cg.time;
	cg.scoreBoardShowing = CG_DrawScoreboard();
}

/*
=================
CG_DrawFollow
=================
*/
static qboolean CG_DrawFollow(void)
{
	float		x, y;
	vec4_t		color;
	const char	*name;

	if (cg.RespawnTime > cg.time
		&& (!cg.scoreBoardShowing || !(cg.snap->ps.pm_flags & PMF_FOLLOW)))
	{
		int s = (cg.RespawnTime - cg.time + 999) / 1000;
		char *str;

		str = va("Next Respawn Wave in" S_COLOR_GREEN " %d" S_COLOR_WHITE " second", s);

		if (s > 1)
			strcat(str, "s");

		x = 320 - BIGCHAR_WIDTH * CG_DrawStrlen(str) * 0.5f;
		y = 480 - BIGCHAR_HEIGHT - 16;
		if (cg.snap->ps.pm_flags & PMF_FOLLOW)
			y -= 128;

		CG_DrawBigString(x, y, str, 1.0f);
	}

	if (!(cg.snap->ps.pm_flags & PMF_FOLLOW))
	{
		return qfalse;
	}

	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;

	CG_DrawBigString(320 - 9 * 8, 24, "following", 1.0f);

	name = cgs.clientinfo[ cg.snap->ps.clientNum ].name;

	x = 0.5f * (640 - BIGCHAR_WIDTH * CG_DrawStrlen(name));
	CG_DrawStringExt(x, 40, name, color, qfalse, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT * 1.5f, 0);

	return qtrue;
}



/*
=================
CG_DrawAmmoWarning
=================
*/
/*static void CG_DrawAmmoWarning(void)		//Too:
{
	const char	*s;
	int			w;

	if (cg_drawAmmoWarning.integer == 0)
	{
		return;
	}

	if (!cg.lowAmmoWarning)
	{
		return;
	}

	if (cg.lowAmmoWarning == 2)
	{
		s = "OUT OF AMMO";
	}
	else
	{
		s = "LOW AMMO WARNING";
	}
	w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
	CG_DrawBigString(320 - w / 2, 64, s, 1.0F);
} */


#ifdef MISSIONPACK
/*
=================
CG_DrawProxWarning
=================
*/
static void CG_DrawProxWarning(void)
{
	char s [32];
	int			w;
  static int proxTime;
  static int proxCounter;
  static int proxTick;

	if (!(cg.snap->ps.eFlags & EF_TICKING))
	{
    proxTime = 0;
		return;
	}

  if (proxTime == 0)
{
    proxTime = cg.time + 5000;
    proxCounter = 5;
    proxTick = 0;
  }

  if (cg.time > proxTime)
{
    proxTick = proxCounter--;
    proxTime = cg.time + 1000;
  }

  if (proxTick != 0)
{
    Com_sprintf(s, sizeof(s), "INTERNAL COMBUSTION IN: %i", proxTick);
  }
else
{
    Com_sprintf(s, sizeof(s), "YOU HAVE BEEN MINED");
  }

	w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
	CG_DrawBigStringColor(320 - w / 2, 64 + BIGCHAR_HEIGHT, s, g_color_table[ColorIndex(COLOR_RED)]);
}
#endif


/*
=================
CG_DrawWarmup
=================
*/
static void CG_DrawWarmup(void)
{
	int			w;
	int			sec;
	int			i;
	float scale;
	clientInfo_t	*ci1, *ci2;
	int			cw;
	const char	*s;

	sec = cg.warmup;
	if (!sec)
	{
		return;
	}

	if (sec < 0)
	{
		s = "Waiting for players";
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
		CG_DrawBigString(320 - w / 2, 24, s, 1.0F);
		cg.warmupCount = 0;
		cgs.InvasionInfo.Period = e_Period_WaitPlayers;
		return;
	}

	cgs.InvasionInfo.Period = e_Period_Selection;

	if (cgs.gametype == GT_TOURNAMENT)
	{
		// find the two active players
		ci1 = NULL;
		ci2 = NULL;
		for (i = 0; i < cgs.maxclients; i++)
		{
			if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_FREE)
			{
				if (!ci1)
				{
					ci1 = &cgs.clientinfo[i];
				}
				else
				{
					ci2 = &cgs.clientinfo[i];
				}
			}
		}

		if (ci1 && ci2)
		{
			s = va("%s vs %s", ci1->name, ci2->name);
#ifdef MISSIONPACK
			w = CG_Text_Width(s, 0.6f, 0);
			CG_Text_Paint(320 - w / 2, 60, 0.6f, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
#else
			w = CG_DrawStrlen(s);
			if (w > 640 / GIANT_WIDTH)
			{
				cw = 640 / w;
			}
			else
			{
				cw = GIANT_WIDTH;
			}
			CG_DrawStringExt(320 - w * cw/2, 20,s, colorWhite,
					qfalse, qtrue, cw, (int)(cw * 1.5f), 0);
#endif
		}
	}
	else
	{
		if (cgs.gametype == GT_FFA)
		{
			s = "Free For All";
		}
		else if (cgs.gametype == GT_TEAM)
		{
			s = "Team Deathmatch";
		}
		else if (cgs.gametype == GT_CTF)
		{
			s = "Capture the Flag";
/*#ifdef MISSIONPACK
		}
		else if (cgs.gametype == GT_1FCTF)
		{
			s = "One Flag CTF";
		}
		else if (cgs.gametype == GT_OBELISK)
		{
			s = "Overload";
		}
		else if (cgs.gametype == GT_HARVESTER)
		{
			s = "Harvester";
#endif*/
		}
		else //if (cgs.gametype == GT_INVASION)
		{
			s = "Invasion b"INVASION_VERSION;
		}
#ifdef MISSIONPACK
		w = CG_Text_Width(s, 0.6f, 0);
		CG_Text_Paint(320 - w / 2, 90, 0.6f, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
#else
		w = CG_DrawStrlen(s);
		if (w > 640 / GIANT_WIDTH)
		{
			cw = 640 / w;
		}
		else
		{
			cw = GIANT_WIDTH;
		}
		CG_DrawStringExt(320 - w * cw/2, 25,s, colorWhite,
				qfalse, qtrue, cw, (int)(cw * 1.1), 0);
#endif
	}

	sec -= cg.time;

	if (sec < 0 && (cgs.gametype == GT_INVASION || cgs.gametype == GT_DESTROY))
	{
		cg.warmup = 0;

		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
			CG_CenterPrint("", 20, GIANTCHAR_WIDTH*2);
		else
		{
			trap_S_StartLocalSound(cgs.media.countFightSound, CHAN_ANNOUNCER);
			CG_CenterPrint("FIGHT!", 50, GIANTCHAR_WIDTH*1.0f);
		}

		return;
	}

	sec /= 1000;

	if (sec < 0)
	{
		sec = 0;
		cg.warmup = 0;
	}

	s = va("Starts in: %i", sec + 1);
	if (sec != cg.warmupCount)
	{
		cg.warmupCount = sec;
		switch (sec)
		{
		case 0:
			trap_S_StartLocalSound(cgs.media.count1Sound, CHAN_ANNOUNCER);
			break;
		case 1:
			trap_S_StartLocalSound(cgs.media.count2Sound, CHAN_ANNOUNCER);
			break;
		case 2:
			trap_S_StartLocalSound(cgs.media.count3Sound, CHAN_ANNOUNCER);
			break;
		default:
			break;
		}
	}
	scale = 0.45f;
	switch (cg.warmupCount)
	{
	case 0:
		cw = 25;
		scale = 0.54f;
		break;
	case 1:
		cw = 22;
		scale = 0.51f;
		break;
	case 2:
		cw = 19;
		scale = 0.48f;
		break;
	default:
		cw = 16;
		scale = 0.45f;
		break;
	}

#ifdef MISSIONPACK
		w = CG_Text_Width(s, scale, 0);
		CG_Text_Paint(320 - w / 2, 125, scale, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
#else
	w = CG_DrawStrlen(s);
	CG_DrawStringExt(320 - w * cw/2, 70, s, colorWhite,
			qfalse, qtrue, cw, (int)(cw * 1.5f), 0);
#endif
}

//==================================================================================
#ifdef MISSIONPACK
/*
=================
CG_DrawTimedMenus
=================
*/
void CG_DrawTimedMenus()
{
	if (cg.voiceTime)
	{
		int t = cg.time - cg.voiceTime;
		if (t > 2500)
		{
			Menus_CloseByName("voiceMenu");
			trap_Cvar_Set("cl_conXOffset", "0");
			cg.voiceTime = 0;
		}
	}
}
#endif
/*
=================
CG_Draw2D
=================
*/
static void CG_Draw2D(void)
{
#ifdef MISSIONPACK
	if (cgs.orderPending && cg.time > cgs.orderTime)
	{
		CG_CheckOrderPending();
	}
#endif
	// if we are taking a levelshot for the menu, don't draw anything
	if (cg.levelShot)
	{
		return;
	}

	if (cg.Zoom)
	{
		CG_DrawPic(0, 0, 640, 480, cgs.media.ZoomBlack);
		CG_DrawUpperRight();
		CG_DrawCrosshair();
		return;
	}

	if (cg.NightVision || cg.VisionTime + 250 > cg.time)
	{
		if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR)
		{
			int n;
			vec4_t hcolor[4] = { { 0.1f, 1.0f, 0.1f, 1.0f },
										{ 1.0f, 0.1f, 0.1f, 1.0f },
										{ 0.1f, 1.0f, 0.1f, 1.0f },
										{ 1.0f, 1.0f, 1.0f, 0.8f } };

			if (cg.NightVision == 1)
				n = 2;
			else if (cg.snap->ps.persistant[PERS_TEAM] == cgs.InvasionInfo.MarineTeam)
				n = 0;
			else
				n = 1;

			trap_R_SetColor(hcolor[n]);

			if (cg.VisionTime + 250 > cg.time)
			{
				int dy = sqrt((cg.time - cg.VisionTime) / 250.0f) * (0.5f * 480);
				int old = cg.NightVision - 1;

				if (cg.NightVision)
					CG_DrawPic(0, 240 - dy, 640, dy * 2.0f, cgs.media.NightVision[cg.NightVision-1]);

				if (old < 0)
					old = 3;

				dy = 240 - dy;

				if (old > 0)
				{
					if (old == 1)
						n = 2;
					else if (cg.snap->ps.persistant[PERS_TEAM] == cgs.InvasionInfo.MarineTeam)
						n = 0;
					else
						n = 1;

					trap_R_SetColor(hcolor[n]);
					CG_DrawPic(0, 0, 640, dy, cgs.media.NightVision[old - 1]);
					CG_DrawPic(0, 480 - dy, 640, dy, cgs.media.NightVision[old - 1]);
				}

				trap_R_SetColor(hcolor[3]);
				CG_DrawPic(0, dy - 4, 640, 8, cgs.media.VisionSwapFx);
				CG_DrawPic(0, 480 - (dy + 4), 640, 8, cgs.media.VisionSwapFx);
				trap_R_SetColor(NULL);
			}
			else if (cg.NightVision)
			{
				CG_DrawPic(0, 0, 640, 480, cgs.media.NightVision[cg.NightVision-1]);
				trap_R_SetColor(NULL);
			}

			//CG_DrawUpperRight();
			//CG_DrawCrosshair();
			//return;
		}
	}

	if (cg_draw2D.integer == 0)
	{
		return;
	}

	if (cg.snap->ps.pm_type == PM_INTERMISSION)
	{
		CG_DrawIntermission();
		return;
	}

/*
	if (cg.cameraMode) {
		return;
	}
*/

	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR)
	{
		CG_DrawSpectator();
		CG_DrawCrosshair();
		CG_DrawCrosshairNames();
	}
	else
	{
		// don't draw any status if dead or the scoreboard is being explicitly shown
		if (!cg.showScores && cg.snap->ps.stats[STAT_HEALTH] > 0)
		{

#ifdef MISSIONPACK
			if (cg_drawStatus.integer)
			{
				Menu_PaintAll();
				CG_DrawTimedMenus();
			}
#else
			CG_DrawStatusBar();
#endif

			//CG_DrawAmmoWarning();

#ifdef MISSIONPACK
			CG_DrawProxWarning();
#endif
			CG_DrawCrosshair();
			CG_DrawCrosshairNames();
			CG_DrawWeaponSelect();

#ifndef MISSIONPACK
			CG_DrawHoldableItem();
#else
			//CG_DrawPersistantPowerup();
#endif
			CG_DrawReward();
		}
	}

	CG_DrawVote();
	CG_DrawTeamVote();

	CG_DrawLagometer();

#ifdef MISSIONPACK
	if (!cg_paused.integer)
	{
		CG_DrawUpperRight();
	}
#else
	CG_DrawUpperRight();
#endif

#ifndef MISSIONPACK
	CG_DrawLowerRight();
	CG_DrawLowerLeft();
#endif

	if (cgs.gametype >= GT_TEAM)
	{
#ifndef MISSIONPACK
		CG_DrawTeamInfo();
#endif
	}

	CG_DrawGameTimeLeft();

	CG_DrawRadar();

	CG_DrawInGameHelp();

	// don't draw center string if scoreboard is up
	cg.scoreBoardShowing = CG_DrawScoreboard();
	if (!cg.scoreBoardShowing)
	{
		CG_DrawCenterString();
		CG_DrawRightString();
	}

	if (!CG_DrawFollow())
	{
		CG_DrawWarmup();
	}
}


static void CG_DrawTourneyScoreboard()
{
#ifdef MISSIONPACK
#else
	CG_DrawOldTourneyScoreboard();
#endif
}

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive(stereoFrame_t stereoView)
{
	float		separation;
	vec3_t		baseOrg;

	// optionally draw the info screen instead
	if (!cg.snap)
	{
		CG_DrawInformation();
		return;
	}

	// optionally draw the tournement scoreboard instead
	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR &&
		(cg.snap->ps.pm_flags & PMF_SCOREBOARD))
		{
		CG_DrawTourneyScoreboard();
		return;
	}

	switch (stereoView)
	{
	case STEREO_CENTER:
		separation = 0;
		break;
	case STEREO_LEFT:
		separation = -cg_stereoSeparation.value / 2;
		break;
	case STEREO_RIGHT:
		separation = cg_stereoSeparation.value / 2;
		break;
	default:
		separation = 0;
		CG_Error("CG_DrawActive: Undefined stereoView");
		break;
	}


	// clear around the rendered view if sized down
	CG_TileClear();

	// offset vieworg appropriately if we're doing stereo separation
	VectorCopy(cg.refdef.vieworg, baseOrg);
	if (separation != 0)
	{
		VectorMA(cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg);
	}

	// draw 3D view
	/*if (cg.NightVision)
	{
		cg.refdef.rdflags |= RDF_NOWORLDMODEL;
		trap_R_RenderScene(&cg.refdef);
		//trap_Cvar_Set("r_drawentities", "0");
	}
	else*/
		trap_R_RenderScene(&cg.refdef);

	// restore original viewpoint if running stereo
	if (separation != 0)
	{
		VectorCopy(baseOrg, cg.refdef.vieworg);
	}

	// draw status bar and other floating elements
 	CG_Draw2D();
}



/*==================== EOF because of buggy VSS ===========*/