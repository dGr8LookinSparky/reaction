//-----------------------------------------------------------------------------
//
// $Id$
//
//-----------------------------------------------------------------------------
//
// $Log$
// Revision 1.46  2002/06/16 20:06:13  jbravo
// Reindented all the source files with "indent -kr -ut -i8 -l120 -lc120 -sob -bad -bap"
//
// Revision 1.45  2002/06/16 19:12:52  jbravo
// Removed the MISSIONPACK ifdefs and missionpack only code.
//
// Revision 1.44  2002/06/03 05:23:47  niceass
// spectator changes
//
// Revision 1.43  2002/06/03 00:48:32  niceass
// match scoreboard changes
//
// Revision 1.42  2002/05/19 18:32:11  jbravo
// Made new cvars for regular xhair colors.
//
// Revision 1.41  2002/05/18 03:55:35  niceass
// many misc. changes
//
// Revision 1.40  2002/05/13 05:24:54  jbravo
// the ssg color cvars now also control normal xhair color
//
// Revision 1.39  2002/05/01 21:14:59  jbravo
// Misc fixes
//
// Revision 1.38  2002/05/01 03:27:17  niceass
// centerprint fix + prettier
//
// Revision 1.37  2002/04/29 06:10:48  niceass
// better centerprint
//
// Revision 1.36  2002/04/29 01:24:34  niceass
// frag counter added
//
// Revision 1.35  2002/04/23 00:21:44  jbravo
// Cleanups of the new model code.  Removed the spectator bar for zcam modes.
//
// Revision 1.34  2002/04/06 21:43:58  makro
// New surfaceparm system
//
// Revision 1.33  2002/03/31 03:31:24  jbravo
// Compiler warning cleanups
//
// Revision 1.32  2002/03/30 02:29:43  jbravo
// Lots of spectator code updates. Removed debugshit, added some color.
//
// Revision 1.31  2002/03/24 22:50:52  niceass
// misc. 2d screen stuff changed
//
// Revision 1.30  2002/03/23 05:17:42  jbravo
// Major cleanup of game -> cgame communication with LCA vars.
//
// Revision 1.29  2002/03/21 15:02:05  jbravo
// More teamname cleanups and fix for fraglines.
//
// Revision 1.28  2002/03/17 21:32:23  jbravo
// Fixed the dynamic teamnames system up a bit to reduce traffic
//
// Revision 1.27  2002/03/17 13:18:14  jbravo
// Replace the fraglimit on the HUD with the clients score
//
// Revision 1.26  2002/03/17 00:40:23  jbravo
// Adding variable team names. g_RQ3_team1name and g_RQ3_team2name. Fixed
// Slicers fraglimit check.
//
// Revision 1.25  2002/03/04 20:50:59  jbravo
// No floating scores over dead bodies, triangles disabled, and no viewing
// names of enemys just of teammates.
//
// Revision 1.24  2002/03/04 19:28:21  jbravo
// Fixing follownames up as suggested in the forums.
//
// Revision 1.23  2002/03/02 14:54:24  jbravo
// Added the skin and model names to the name of the player thats being
// followed, as in AQ
//
// Revision 1.22  2002/02/28 05:41:54  blaze
// weapons stats on client side
//
// Revision 1.21  2002/02/25 19:41:53  jbravo
// Fixed the use ESC and join menu to join teams when dead players are
// spectating in TP mode.
// Tuned the autorespawn system a bit.  Now dead ppl. are dead for a very
// small time before they are made into spectators.
//
// Revision 1.20  2002/02/23 18:07:46  slicer
// Changed Sniper code and Cam code
//
// Revision 1.19  2002/01/11 20:20:57  jbravo
// Adding TP to main branch
//
// Revision 1.18  2002/01/11 19:48:29  jbravo
// Formatted the source in non DOS format.
//
// Revision 1.17  2001/12/31 16:28:41  jbravo
// I made a Booboo with the Log tag.
//
//
//-----------------------------------------------------------------------------
// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"

// JBravo: warning fix
void CG_DrawWeaponStats(void);

int drawTeamOverlayModificationCount = -1;
int sortedTeamPlayers[TEAM_MAXOVERLAY];
int numSortedTeamPlayers;
char systemChat[256];
char teamChat1[256];
char teamChat2[256];

/*
==============
CG_DrawField

Draws large numbers for status bar and powerups
==============
*/
static void CG_DrawField(int x, int y, int width, int value)
{
	char num[16], *ptr;
	int l;
	int frame;

	if (width < 1) {
		return;
	}
	// draw number string
	if (width > 5) {
		width = 5;
	}

	switch (width) {
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
	x += 2 + CHAR_WIDTH * (width - l);

	ptr = num;
	while (*ptr && l) {
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr - '0';

		CG_DrawPic(x, y, CHAR_WIDTH, CHAR_HEIGHT, cgs.media.numberShaders[frame]);
		x += CHAR_WIDTH;
		ptr++;
		l--;
	}
}

/*
================
CG_Draw3DModel

================
*/
void CG_Draw3DModel(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles)
{
	refdef_t refdef;
	refEntity_t ent;

	if (!cg_draw3dIcons.integer || !cg_drawIcons.integer) {
		return;
	}

	CG_AdjustFrom640(&x, &y, &w, &h);

	memset(&refdef, 0, sizeof(refdef));

	memset(&ent, 0, sizeof(ent));
	AnglesToAxis(angles, ent.axis);
	VectorCopy(origin, ent.origin);
	ent.hModel = model;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW;	// no stencil shadows

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
	clipHandle_t cm;
	clientInfo_t *ci;
	float len;
	vec3_t origin;
	vec3_t mins, maxs;

	ci = &cgs.clientinfo[clientNum];

	if (cg_draw3dIcons.integer) {
		cm = ci->headModel;
		if (!cm) {
			return;
		}
		// offset the origin y and z to center the head
		trap_R_ModelBounds(cm, mins, maxs);

		origin[2] = -0.5 * (mins[2] + maxs[2]);
		origin[1] = 0.5 * (mins[1] + maxs[1]);

		// calculate distance so the head nearly fills the box
		// assume heads are taller than wide
		len = 0.7 * (maxs[2] - mins[2]);
		origin[0] = len / 0.268;	// len / tan( fov/2 )

		// allow per-model tweaking
		VectorAdd(origin, ci->headOffset, origin);

		CG_Draw3DModel(x, y, w, h, ci->headModel, ci->headSkin, origin, headAngles);
	} else if (cg_drawIcons.integer) {
		CG_DrawPic(x, y, w, h, ci->modelIcon);
	}
	// if they are deferred, draw a cross out
	if (ci->deferred) {
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
	qhandle_t cm;
	float len;
	vec3_t origin, angles;
	vec3_t mins, maxs;
	qhandle_t handle;

	if (!force2D && cg_draw3dIcons.integer) {

		VectorClear(angles);

		cm = cgs.media.redFlagModel;

		// offset the origin y and z to center the flag
		trap_R_ModelBounds(cm, mins, maxs);

		origin[2] = -0.5 * (mins[2] + maxs[2]);
		origin[1] = 0.5 * (mins[1] + maxs[1]);

		// calculate distance so the flag nearly fills the box
		// assume heads are taller than wide
		len = 0.5 * (maxs[2] - mins[2]);
		origin[0] = len / 0.268;	// len / tan( fov/2 )

		angles[YAW] = 60 * sin(cg.time / 2000.0);;

		if (team == TEAM_RED) {
			handle = cgs.media.redFlagModel;
		} else if (team == TEAM_BLUE) {
			handle = cgs.media.blueFlagModel;
		} else if (team == TEAM_FREE) {
			handle = cgs.media.neutralFlagModel;
		} else {
			return;
		}
		CG_Draw3DModel(x, y, w, h, handle, 0, origin, angles);
	} else if (cg_drawIcons.integer) {
		gitem_t *item;

		if (team == TEAM_RED) {
			item = BG_FindItemForPowerup(PW_REDFLAG);
		} else if (team == TEAM_BLUE) {
			item = BG_FindItemForPowerup(PW_BLUEFLAG);
		} else if (team == TEAM_FREE) {
			item = BG_FindItemForPowerup(PW_NEUTRALFLAG);
		} else {
			return;
		}
		if (item) {
			CG_DrawPic(x, y, w, h, cg_items[ITEM_INDEX(item)].icon);
		}
	}
}

/*
================
CG_DrawTeamBackground

================
*/
void CG_DrawTeamBackground(int x, int y, int w, int h, float alpha, int team)
{
	vec4_t hcolor;

	hcolor[3] = alpha;
	if (team == TEAM_RED) {
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
	} else if (team == TEAM_BLUE) {
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 1;
	} else {
		return;
	}
	trap_R_SetColor(hcolor);
	CG_DrawPic(x, y, w, h, cgs.media.teamStatusBar);
	trap_R_SetColor(NULL);
}

/*
================
CG_DrawStatusBar

================
*/
static void CG_DrawStatusBar(void)
{
	int color;
	int style;
	centity_t *cent;
	playerState_t *ps;
	int value;
	vec4_t hcolor;
	qhandle_t hicon;
	qhandle_t icon;
	int i;

	static float colors[4][4] = {
		{0.0f, 1.0f, 0.0f, 1.0f},	// full green
		{0.6f, 0.6f, 0.6f, 1.0f},	// firing
		{0.8f, 0.8f, 0.0f, 1.0f},	// not maximum
		{0.8f, 0.0f, 0.0f, 1.0f}
	};			// out of ammo

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	//Draw health
	value = ps->stats[STAT_HEALTH];
	style = UI_LEFT | UI_DROPSHADOW;

	if (value <= 25)
		style |= UI_PULSE;

	//Elder: Need bandaging?
	if ((ps->stats[STAT_RQ3] & RQ3_BANDAGE_NEED) == RQ3_BANDAGE_NEED)
		hicon = cgs.media.rq3_healthicon2;
	else
		hicon = cgs.media.rq3_healthicon;

	//Elder: dynamic health color ramps
	//Blends from green to yellow to red algebraically
	//100 - Green, 50 - Yellow, 25 - Red, 0 - Faded Red
	//Note: These formulas are clamped from 0.0 to 1.0 algebraically
	hcolor[0] = (value > 50) * (-0.02 * value + 2.0) + (value <= 50) * 1;
	hcolor[1] = (value > 25 && value <= 50) * (0.04 * value - 1.0) + (value > 50) * 1;
	hcolor[2] = 0;
	hcolor[3] = (value <= 25) * (0.01 * value + 0.75) + (value > 25) * 1;

	/* Elder: Old clamp routine for reference -- more efficient since less stack usage?
	   for (i = 0; i < 4; i++) {
	   if (hcolor[i] > 1.0) {
	   CG_Printf ("Over one on %i\n",i);
	   hcolor[i] = 1.0;
	   }
	   else if (hcolor[i] < 0.0) {
	   CG_Printf ("Below zero on %i\n",i);
	   hcolor[i] = 0.0;
	   }
	   } */

	CG_DrawPic(8, 440, SMICON_SIZE, SMICON_SIZE, hicon);
	//CG_DrawStringExt(44, 444, va("%d", value), hcolor, qtrue, qtrue, 24, 24, 3);
	UI_DrawProportionalString(44, 444, va("%d", value), style, hcolor);

	//Elder: Draw weapon ammo and clips
	style = UI_LEFT | UI_DROPSHADOW;
/*
	icon = cg_weapons[ cg.predictedPlayerState.weapon ].weaponIcon;

	origin[0] = 40;
	origin[1] = 0;
	origin[2] = -30;

	if (icon)
		CG_DrawPic(152, 440, SMICON_SIZE, SMICON_SIZE, icon);
*/

	icon = cg_weapons[cg.predictedPlayerState.weapon].ammoIcon;
	//Don't draw ammo icon if holding grenade or knife
	if (icon && cg.predictedPlayerState.weapon != WP_KNIFE && cg.predictedPlayerState.weapon != WP_GRENADE)
		CG_DrawPic(252, 440, SMICON_SIZE, SMICON_SIZE, icon);

	if (cent->currentState.weapon) {
		value = ps->ammo[cent->currentState.weapon];

		// Select colour
		if (cg.predictedPlayerState.weaponstate == WEAPON_FIRING && cg.predictedPlayerState.weaponTime > 100)
			color = 1;
		else if (ps->ammo[cent->currentState.weapon] == 0)
			color = 3;
		else if (ps->ammo[cent->currentState.weapon] < ClipAmountForAmmo(cent->currentState.weapon))
			color = 2;
		else
			color = 0;

		if (value >= 0)
			UI_DrawProportionalString(188, 444, va("%d", value), style, colors[color]);

		//UI_DrawProportionalString(188, 444, "/"), style, colors[0]);

		value = ps->stats[STAT_CLIPS];
		if (value > -1 &&
		    cg.predictedPlayerState.weapon != WP_KNIFE && cg.predictedPlayerState.weapon != WP_GRENADE)
			UI_DrawProportionalString(288, 444, va("%d", value), style, colors[0]);
	}
	// Elder: temporary
	//if (cg.snap->ps.stats[STAT_RELOADTIME] > 0)
	//UI_DrawProportionalString( 10, 400, va("%i", cg.snap->ps.stats[STAT_RELOADTIME]), style, colors[2]);

	//Elder: draw grenades, if any, on the side
	if (cg.snap->ps.ammo[WP_GRENADE] > 0) {
		icon = cg_weapons[WP_GRENADE].weaponIcon;
		if (icon)
			CG_DrawPic(640 - SMICON_SIZE, 360, SMICON_SIZE, SMICON_SIZE, icon);
		UI_DrawProportionalString(580, 364, va("%d", cg.snap->ps.ammo[WP_GRENADE]), style, colors[0]);
	}
	//Elder: draw a special weapon, if any, on the side
	for (i = 1; i < MAX_WEAPONS; i++) {
		if (i == WP_KNIFE || i == WP_PISTOL || i == WP_GRENADE || i == WP_AKIMBO)
			continue;

		if ((1 << i) == ((1 << i) & cg.snap->ps.stats[STAT_WEAPONS]))
			break;
	}

	if (i < MAX_WEAPONS) {
		icon = cg_weapons[i].weaponIcon;
		if (icon)
			CG_DrawPic(640 - SMICON_SIZE, 400, SMICON_SIZE, SMICON_SIZE, icon);
	}
}

/*
===========================================================================================

  UPPER RIGHT CORNER

===========================================================================================
*/

/*
================
CG_DrawAttacker

================
*/
static float CG_DrawAttacker(float y)
{
	int t;
	float size;
	vec3_t angles;
	const char *info;
	const char *name;
	int clientNum;

	if (cg.predictedPlayerState.stats[STAT_HEALTH] <= 0) {
		return y;
	}

	if (!cg.attackerTime) {
		return y;
	}

	clientNum = cg.predictedPlayerState.persistant[PERS_ATTACKER];
	if (clientNum < 0 || clientNum >= MAX_CLIENTS || clientNum == cg.snap->ps.clientNum) {
		return y;
	}

	t = cg.time - cg.attackerTime;
	if (t > ATTACKER_HEAD_TIME) {
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
}

/*
==================
CG_DrawSnapshot
==================
*/
static float CG_DrawSnapshot(float y)
{
	char *s;
	int w;

	s = va("time:%i snap:%i cmd:%i", cg.snap->serverTime, cg.latestSnapshotNum, cgs.serverCommandSequence);
	w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;

	CG_DrawBigString(635 - w, y + 2, s, 1.0F);

	return y + BIGCHAR_HEIGHT + 4;
}

static float CG_DrawScore(float y)
{
	char *s;
	int w, x;
	float Color[4];

	y += 4;

	s = va("%i", cg.snap->ps.persistant[PERS_SCORE]);
	w = CG_DrawStrlen(s) * SMALLCHAR_WIDTH;
	x = w;

	MAKERGBA(Color, 0.0f, 0.0f, 0.0f, 0.4f);
	CG_FillRect(631 - x - 3, y - 1, w + 6, SMALLCHAR_HEIGHT + 6, Color);

	MAKERGBA(Color, 0.0f, 0.0f, 0.0f, 1.0f);
	CG_DrawCleanRect(631 - x - 3, y - 1, w + 6, SMALLCHAR_HEIGHT + 6, 1, Color);

	CG_DrawSmallString(631 - x, y + 2, s, 1.0F);

	if (cgs.gametype >= GT_TEAM) {
		// blue team
		s = va("%i", cgs.scores2);
		w = CG_DrawStrlen(s) * SMALLCHAR_WIDTH;
		x += w + 10;

		MAKERGBA(Color, 0.0f, 0.0f, 1.0f, 0.4f);
		CG_FillRect(631 - x - 3, y - 1, w + 6, SMALLCHAR_HEIGHT + 6, Color);

		MAKERGBA(Color, 0.0f, 0.0f, 0.0f, 1.0f);
		CG_DrawCleanRect(631 - x - 3, y - 1, w + 6, SMALLCHAR_HEIGHT + 6, 1, Color);

		CG_DrawSmallString(631 - x, y + 2, s, 1.0f);

		// red team
		s = va("%i", cgs.scores1);
		w = CG_DrawStrlen(s) * SMALLCHAR_WIDTH;
		x += w + 10;

		MAKERGBA(Color, 1.0f, 0.0f, 0.0f, 0.4f);
		CG_FillRect(631 - x - 3, y - 1, w + 6, SMALLCHAR_HEIGHT + 6, Color);

		MAKERGBA(Color, 0.0f, 0.0f, 0.0f, 1.0f);
		CG_DrawCleanRect(631 - x - 3, y - 1, w + 6, SMALLCHAR_HEIGHT + 6, 1, Color);

		CG_DrawSmallString(631 - x, y + 2, s, 1.0f);
	}

	return y + SMALLCHAR_HEIGHT + 4;
}

/*
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES	4
static float CG_DrawFPS(float y)
{
	char *s;
	int w;
	static int previousTimes[FPS_FRAMES];
	static int index;
	int i, total;
	int fps;
	static int previous;
	int t, frameTime;
	float Color[4];

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;

	y += 4;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if (index > FPS_FRAMES) {
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for (i = 0; i < FPS_FRAMES; i++) {
			total += previousTimes[i];
		}
		if (!total) {
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;

		s = va("%ifps", fps);
		w = CG_DrawStrlen(s) * SMALLCHAR_WIDTH;

		MAKERGBA(Color, 0.0f, 0.0f, 0.0f, 0.4f);
		CG_FillRect(631 - w - 3, y - 1, w + 6, SMALLCHAR_HEIGHT + 6, Color);

		MAKERGBA(Color, 0.0f, 0.0f, 0.0f, 1.0f);
		CG_DrawCleanRect(631 - w - 3, y - 1, w + 6, SMALLCHAR_HEIGHT + 6, 1, Color);

		CG_DrawSmallString(631 - w, y + 2, s, 1.0F);
	}

	return y + SMALLCHAR_HEIGHT + 4;
}

/*
=================
CG_DrawTimer
=================
*/
static float CG_DrawTimer(float y)
{
	char *s;
	int w;
	int mins, seconds, tens;
	int msec;

	msec = cg.time - cgs.levelStartTime;

	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

	s = va("%i:%i%i", mins, tens, seconds);
	w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;

	CG_DrawBigString(635 - w, y + 2, s, 1.0F);

	return y + BIGCHAR_HEIGHT + 4;
}

/*
=================
CG_DrawTeamOverlay
=================
*/

static float CG_DrawTeamOverlay(float y, qboolean right, qboolean upper)
{
	int x, w, h, xx;
	int i, j, len;
	const char *p;
	vec4_t hcolor;
	int pwidth, lwidth;
	int plyrs;
	char st[16];
	clientInfo_t *ci;
	gitem_t *item;
	int ret_y, count;

	if (!cg_drawTeamOverlay.integer) {
		return y;
	}

	if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_RED && cg.snap->ps.persistant[PERS_TEAM] != TEAM_BLUE) {
		return y;	// Not on any team
	}

	plyrs = 0;

	// max player name width
	pwidth = 0;
	count = (numSortedTeamPlayers > 8) ? 8 : numSortedTeamPlayers;
	for (i = 0; i < count; i++) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if (ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {
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
	for (i = 1; i < MAX_LOCATIONS; i++) {
		p = CG_ConfigString(CS_LOCATIONS + i);
		if (p && *p) {
			len = CG_DrawStrlen(p);
			if (len > lwidth)
				lwidth = len;
		}
	}

	if (lwidth > TEAM_OVERLAY_MAXLOCATION_WIDTH)
		lwidth = TEAM_OVERLAY_MAXLOCATION_WIDTH;

	w = (pwidth + lwidth + 4 + 7) * TINYCHAR_WIDTH;

	if (right)
		x = 640 - w;
	else
		x = 0;

	h = plyrs * TINYCHAR_HEIGHT;

	if (upper) {
		ret_y = y + h;
	} else {
		y -= h;
		ret_y = y;
	}

	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED) {
		hcolor[0] = 1.0f;
		hcolor[1] = 0.0f;
		hcolor[2] = 0.0f;
		hcolor[3] = 0.33f;
	} else {		// if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
		hcolor[0] = 0.0f;
		hcolor[1] = 0.0f;
		hcolor[2] = 1.0f;
		hcolor[3] = 0.33f;
	}
	trap_R_SetColor(hcolor);
	CG_DrawPic(x, y, w, h, cgs.media.teamStatusBar);
	trap_R_SetColor(NULL);

	for (i = 0; i < count; i++) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if (ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {

			hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0;

			xx = x + TINYCHAR_WIDTH;

			CG_DrawStringExt(xx, y,
					 ci->name, hcolor, qfalse, qfalse,
					 TINYCHAR_WIDTH, TINYCHAR_HEIGHT, TEAM_OVERLAY_MAXNAME_WIDTH);

			if (lwidth) {
				p = CG_ConfigString(CS_LOCATIONS + ci->location);
				if (!p || !*p)
					p = "unknown";
				len = CG_DrawStrlen(p);
				if (len > lwidth)
					len = lwidth;

//                              xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth + 
//                                      ((lwidth/2 - len/2) * TINYCHAR_WIDTH);
				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth;
				CG_DrawStringExt(xx, y,
						 p, hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
						 TEAM_OVERLAY_MAXLOCATION_WIDTH);
			}

			CG_GetColorForHealth(ci->health, ci->armor, hcolor);

			Com_sprintf(st, sizeof(st), "%3i %3i", ci->health, ci->armor);

			xx = x + TINYCHAR_WIDTH * 3 + TINYCHAR_WIDTH * pwidth + TINYCHAR_WIDTH * lwidth;

			CG_DrawStringExt(xx, y, st, hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);

			// draw weapon icon
			xx += TINYCHAR_WIDTH * 3;

			if (cg_weapons[ci->curWeapon].weaponIcon) {
				CG_DrawPic(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
					   cg_weapons[ci->curWeapon].weaponIcon);
			} else {
				CG_DrawPic(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, cgs.media.deferShader);
			}

			// Draw powerup icons
			if (right) {
				xx = x;
			} else {
				xx = x + w - TINYCHAR_WIDTH;
			}
			for (j = 0; j <= PW_NUM_POWERUPS; j++) {
				if (ci->powerups & (1 << j)) {

					item = BG_FindItemForPowerup(j);

					if (item) {
						CG_DrawPic(xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
							   trap_R_RegisterShader(item->icon));
						if (right) {
							xx -= TINYCHAR_WIDTH;
						} else {
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
	float y;

	y = 0;

	if (cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 1) {
		y = CG_DrawTeamOverlay(y, qtrue, qtrue);
	}
	if (cg_drawSnapshot.integer) {
		y = CG_DrawSnapshot(y);
	}
	y = CG_DrawScore(y);
	if (cg_drawFPS.integer) {
		y = CG_DrawFPS(y);
	}
	if (cg_drawTimer.integer) {
		y = CG_DrawTimer(y);
	}
	if (cg_drawAttacker.integer) {
		y = CG_DrawAttacker(y);
	}

}

/*
===========================================================================================

  LOWER RIGHT CORNER

===========================================================================================
*/

/*
================
CG_DrawPowerups
================
*/
static float CG_DrawPowerups(float y)
{
	int sorted[MAX_POWERUPS];
	int sortedTime[MAX_POWERUPS];
	int i, j, k;
	int active;
	playerState_t *ps;
	int t;
	gitem_t *item;
	int x;
	int color;
	float size;
	float f;
	static float colors[2][4] = {
		{0.2f, 1.0f, 0.2f, 1.0f},
		{1.0f, 0.2f, 0.2f, 1.0f}
	};

	ps = &cg.snap->ps;

	if (ps->stats[STAT_HEALTH] <= 0) {
		return y;
	}
	// sort the list by time remaining
	active = 0;
	for (i = 0; i < MAX_POWERUPS; i++) {
		if (!ps->powerups[i]) {
			continue;
		}
		t = ps->powerups[i] - cg.time;
		// ZOID--don't draw if the power up has unlimited time (999 seconds)
		// This is true of the CTF flags
		if (t < 0 || t > 999000) {
			continue;
		}
		// insert into the list
		for (j = 0; j < active; j++) {
			if (sortedTime[j] >= t) {
				for (k = active - 1; k >= j; k--) {
					sorted[k + 1] = sorted[k];
					sortedTime[k + 1] = sortedTime[k];
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
	for (i = 0; i < active; i++) {
		item = BG_FindItemForPowerup(sorted[i]);

		if (item) {

			color = 1;

			y -= ICON_SIZE;

			trap_R_SetColor(colors[color]);
			CG_DrawField(x, y, 2, sortedTime[i] / 1000);

			t = ps->powerups[sorted[i]];
			if (t - cg.time >= POWERUP_BLINKS * POWERUP_BLINK_TIME) {
				trap_R_SetColor(NULL);
			} else {
				vec4_t modulate;

				f = (float) (t - cg.time) / POWERUP_BLINK_TIME;
				f -= (int) f;
				modulate[0] = modulate[1] = modulate[2] = modulate[3] = f;
				trap_R_SetColor(modulate);
			}

			if (cg.powerupActive == sorted[i] && cg.time - cg.powerupTime < PULSE_TIME) {
				f = 1.0 - (((float) cg.time - cg.powerupTime) / PULSE_TIME);
				size = ICON_SIZE * (1.0 + (PULSE_SCALE - 1.0) * f);
			} else {
				size = ICON_SIZE;
			}

			CG_DrawPic(640 - size, y + ICON_SIZE / 2 - size / 2,
				   size, size, trap_R_RegisterShader(item->icon));
		}
	}
	trap_R_SetColor(NULL);

	return y;
}

/*
=====================
CG_DrawLowerRight

=====================
*/
static void CG_DrawLowerRight(void)
{
	float y;

	//y = 480 - ICON_SIZE;
	y = 320 + SMICON_SIZE;

	if (cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 2) {
		y = CG_DrawTeamOverlay(y, qtrue, qfalse);
	}
	// NiceAss: Taken out. Kinda ugly. I'll probably recode this later....
//      y = CG_DrawScores( y );
	y = CG_DrawPowerups(y);
}

/*
===================
CG_DrawPickupItem
===================
*/
static int CG_DrawPickupItem(int y)
{
	int value;
	float *fadeColor;

	if (cg.snap->ps.stats[STAT_HEALTH] <= 0) {
		return y;
	}

	y -= ICON_SIZE;

	value = cg.itemPickup;
	if (value) {
		fadeColor = CG_FadeColor(cg.itemPickupTime, 3000);
		if (fadeColor) {
			CG_RegisterItemVisuals(value);
			trap_R_SetColor(fadeColor);
			CG_DrawPic(8, y, ICON_SIZE, ICON_SIZE, cg_items[value].icon);
			CG_DrawBigString(ICON_SIZE + 16, y + (ICON_SIZE / 2 - BIGCHAR_HEIGHT / 2),
					 bg_itemlist[value].pickup_name, fadeColor[0]);
			trap_R_SetColor(NULL);
		}
	}

	return y;
}

/*
=====================
CG_DrawLowerLeft

=====================
*/
static void CG_DrawLowerLeft(void)
{
	float y;

	y = 480 - ICON_SIZE;

	if (cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 3) {
		y = CG_DrawTeamOverlay(y, qfalse, qfalse);
	}

	y = CG_DrawPickupItem(y);
}

//===========================================================================================

/*
=================
CG_DrawTeamInfo
=================
*/
static void CG_DrawTeamInfo(void)
{
	int w, h;
	int i, len;
	vec4_t hcolor;
	int chatHeight;

#define CHATLOC_Y 420		// bottom end
#define CHATLOC_X 0

	if (cg_teamChatHeight.integer < TEAMCHAT_HEIGHT)
		chatHeight = cg_teamChatHeight.integer;
	else
		chatHeight = TEAMCHAT_HEIGHT;
	if (chatHeight <= 0)
		return;		// disabled

	if (cgs.teamLastChatPos != cgs.teamChatPos) {
		if (cg.time - cgs.teamChatMsgTimes[cgs.teamLastChatPos % chatHeight] > cg_teamChatTime.integer) {
			cgs.teamLastChatPos++;
		}

		h = (cgs.teamChatPos - cgs.teamLastChatPos) * TINYCHAR_HEIGHT;

		w = 0;

		for (i = cgs.teamLastChatPos; i < cgs.teamChatPos; i++) {
			len = CG_DrawStrlen(cgs.teamChatMsgs[i % chatHeight]);
			if (len > w)
				w = len;
		}
		w *= TINYCHAR_WIDTH;
		w += TINYCHAR_WIDTH * 2;

		if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED) {
			hcolor[0] = 1.0f;
			hcolor[1] = 0.0f;
			hcolor[2] = 0.0f;
			hcolor[3] = 0.33f;
		} else if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE) {
			hcolor[0] = 0.0f;
			hcolor[1] = 0.0f;
			hcolor[2] = 1.0f;
			hcolor[3] = 0.33f;
		} else {
			hcolor[0] = 0.0f;
			hcolor[1] = 1.0f;
			hcolor[2] = 0.0f;
			hcolor[3] = 0.33f;
		}

		trap_R_SetColor(hcolor);
		CG_DrawPic(CHATLOC_X, CHATLOC_Y - h, 640, h, cgs.media.teamStatusBar);
		trap_R_SetColor(NULL);

		hcolor[0] = hcolor[1] = hcolor[2] = 1.0f;
		hcolor[3] = 1.0f;

		for (i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i--) {
			CG_DrawStringExt(CHATLOC_X + TINYCHAR_WIDTH,
					 CHATLOC_Y - (cgs.teamChatPos - i) * TINYCHAR_HEIGHT,
					 cgs.teamChatMsgs[i % chatHeight], hcolor, qfalse, qfalse,
					 TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0);
		}
	}
}

/*
===================
CG_DrawHoldableItem
===================
*/
static void CG_DrawHoldableItem(void)
{
	int value;

	value = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
	if (value) {
		CG_RegisterItemVisuals(value);
		//CG_DrawPic( 640-ICON_SIZE, (SCREEN_HEIGHT-ICON_SIZE)/2, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
		CG_DrawPic(640 - SMICON_SIZE, 440, SMICON_SIZE, SMICON_SIZE, cg_items[value].icon);
	}

}

/*
===================
CG_DrawReward
===================
*/
static void CG_DrawReward(void)
{
	float *color;
	int i, count;
	float x, y;
	char buf[32];

	if (!cg_drawRewards.integer) {
		return;
	}

	color = CG_FadeColor(cg.rewardTime, REWARD_TIME);
	if (!color) {
		if (cg.rewardStack > 0) {
			for (i = 0; i < cg.rewardStack; i++) {
				cg.rewardSound[i] = cg.rewardSound[i + 1];
				cg.rewardShader[i] = cg.rewardShader[i + 1];
				cg.rewardCount[i] = cg.rewardCount[i + 1];
			}
			cg.rewardTime = cg.time;
			cg.rewardStack--;
			color = CG_FadeColor(cg.rewardTime, REWARD_TIME);
			trap_S_StartLocalSound(cg.rewardSound[0], CHAN_ANNOUNCER);
		} else {
			return;
		}
	}

	trap_R_SetColor(color);

	/*
	   count = cg.rewardCount[0]/10;                                // number of big rewards to draw

	   if (count) {
	   y = 4;
	   x = 320 - count * ICON_SIZE;
	   for ( i = 0 ; i < count ; i++ ) {
	   CG_DrawPic( x, y, (ICON_SIZE*2)-4, (ICON_SIZE*2)-4, cg.rewardShader[0] );
	   x += (ICON_SIZE*2);
	   }
	   }

	   count = cg.rewardCount[0] - count*10;                // number of small rewards to draw
	 */

	if (cg.rewardCount[0] >= 10) {
		y = 56;
		x = 320 - ICON_SIZE / 2;
		CG_DrawPic(x, y, ICON_SIZE - 4, ICON_SIZE - 4, cg.rewardShader[0]);
		Com_sprintf(buf, sizeof(buf), "%d", cg.rewardCount[0]);
		x = (SCREEN_WIDTH - SMALLCHAR_WIDTH * CG_DrawStrlen(buf)) / 2;
		CG_DrawStringExt(x, y + ICON_SIZE, buf, color, qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0);
	} else {

		count = cg.rewardCount[0];

		y = 56;
		x = 320 - count * ICON_SIZE / 2;
		for (i = 0; i < count; i++) {
			CG_DrawPic(x, y, ICON_SIZE - 4, ICON_SIZE - 4, cg.rewardShader[0]);
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

typedef struct {
	int frameSamples[LAG_SAMPLES];
	int frameCount;
	int snapshotFlags[LAG_SAMPLES];
	int snapshotSamples[LAG_SAMPLES];
	int snapshotCount;
} lagometer_t;

lagometer_t lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo(void)
{
	int offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[lagometer.frameCount & (LAG_SAMPLES - 1)] = offset;
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
void CG_AddLagometerSnapshotInfo(snapshot_t * snap)
{
	// dropped packet
	if (!snap) {
		lagometer.snapshotSamples[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = -1;
		lagometer.snapshotCount++;
		return;
	}
	// add this snapshot's info
	lagometer.snapshotSamples[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = snap->ping;
	lagometer.snapshotFlags[lagometer.snapshotCount & (LAG_SAMPLES - 1)] = snap->snapFlags;
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
	float x, y;
	int cmdNum;
	usercmd_t cmd;
	const char *s;
	int w;			// bk010215 - FIXME char message[1024];

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap_GetUserCmd(cmdNum, &cmd);
	if (cmd.serverTime <= cg.snap->ps.commandTime || cmd.serverTime > cg.time) {	// special check for map_restart // bk 0102165 - FIXME
		return;
	}
	// also add text in center of screen
	s = "Connection Interrupted";	// bk 010215 - FIXME
	w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
	CG_DrawBigString(320 - w / 2, 100, s, 1.0F);

	// blink the icon
	if ((cg.time >> 9) & 1) {
		return;
	}
	//Elder: changed position
	x = 0;
	y = 0;
	//x = 640 - 48;
	//y = 480 - 48;

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
	int a, x, y, i;
	float v;
	float ax, ay, aw, ah, mid, range;
	int color;
	float vscale;

	if (!cg_lagometer.integer || cgs.localServer) {
		CG_DrawDisconnect();
		return;
	}
	//
	// draw the graph
	//
	//Elder: changed position
	x = 0;
	y = 0;

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
	for (a = 0; a < aw; a++) {
		i = (lagometer.frameCount - 1 - a) & (LAG_SAMPLES - 1);
		v = lagometer.frameSamples[i];
		v *= vscale;
		if (v > 0) {
			if (color != 1) {
				color = 1;
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
			}
			if (v > range) {
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		} else if (v < 0) {
			if (color != 2) {
				color = 2;
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_BLUE)]);
			}
			v = -v;
			if (v > range) {
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for (a = 0; a < aw; a++) {
		i = (lagometer.snapshotCount - 1 - a) & (LAG_SAMPLES - 1);
		v = lagometer.snapshotSamples[i];
		if (v > 0) {
			if (lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED) {
				if (color != 5) {
					color = 5;	// YELLOW for rate delay
					trap_R_SetColor(g_color_table[ColorIndex(COLOR_YELLOW)]);
				}
			} else {
				if (color != 3) {
					color = 3;
					trap_R_SetColor(g_color_table[ColorIndex(COLOR_GREEN)]);
				}
			}
			v = v * vscale;
			if (v > range) {
				v = range;
			}
			trap_R_DrawStretchPic(ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader);
		} else if (v < 0) {
			if (color != 4) {
				color = 4;	// RED for dropped snapshots
				trap_R_SetColor(g_color_table[ColorIndex(COLOR_RED)]);
			}
			trap_R_DrawStretchPic(ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0,
					      cgs.media.whiteShader);
		}
	}

	trap_R_SetColor(NULL);

	if (cg_nopredict.integer || cg_synchronousClients.integer) {
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
	char *s;
	int length = 0;

	Q_strncpyz(cg.centerPrint, str, sizeof(cg.centerPrint));

	cg.centerPrintTime = cg.time;
	cg.centerPrintY = y;
	cg.centerPrintCharWidth = charWidth;
	cg.centerPrintMaxLen = 0;

	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while (*s) {
		if (*s == '\n') {
			cg.centerPrintLines++;
			if (length > cg.centerPrintMaxLen)
				cg.centerPrintMaxLen = length;
			length = 0;
		} else {
			length++;
		}
		s++;
	}
	if (cg.centerPrintMaxLen == 0)
		cg.centerPrintMaxLen = CG_DrawStrlen(str);

	// Last character a linefeed
	if (*(s - 1) == '\n')
		cg.centerPrintLines--;
}

/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString(void)
{
	char *start;
	int l;
	int x, y, w;
	float *color, color2[4];
	int windowHeight;

	if (!cg.centerPrintTime) {
		return;
	}

	color = CG_FadeColor(cg.centerPrintTime, 1000 * cg_centertime.value);
	if (!color) {
		return;
	}

	trap_R_SetColor(color);

	start = cg.centerPrint;

	windowHeight = cg.centerPrintLines * (int) (cg.centerPrintCharWidth * 1.5);

	y = cg.centerPrintY - windowHeight / 2;

	if (cg.centerPrint[0] != '\n') {
		MAKERGBA(color2, 0.0f, 0.0f, 0.0f, 0.4f * color[3]);
		CG_FillRect(320 - (cg.centerPrintMaxLen * cg.centerPrintCharWidth) * 0.5f - 3, y - 3,
			    cg.centerPrintCharWidth * cg.centerPrintMaxLen + 6, windowHeight + 6, color2);
		MAKERGBA(color2, 0.0f, 0.0f, 0.0f, 1.0f * color[3]);
		CG_DrawCleanRect(320 - (cg.centerPrintMaxLen * cg.centerPrintCharWidth) * 0.5f - 3, y - 3,
				 cg.centerPrintCharWidth * cg.centerPrintMaxLen + 6, windowHeight + 6, 1, color2);
	}

	while (1) {
		char linebuffer[1024];

		for (l = 0; l < 50; l++) {
			if (!start[l] || start[l] == '\n') {
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

		w = cg.centerPrintCharWidth * CG_DrawStrlen(linebuffer);

		x = (SCREEN_WIDTH - w) / 2;

		CG_DrawStringExt(x, y, linebuffer, color, qfalse, qtrue,
				 cg.centerPrintCharWidth, (int) (cg.centerPrintCharWidth * 1.5), 0);

		y += cg.centerPrintCharWidth * 1.5;
		while (*start && (*start != '\n')) {
			start++;
		}
		if (!*start) {
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
	float w, h;
	qhandle_t hShader;
	float f;
	float x, y;
	int ca, i;
	vec4_t crosshairColor;

	if (!cg_drawCrosshair.integer) {
		return;
	}
	//Slicer: Adding Crosshair to FOLLOW SPECS
	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR && !(cg.snap->ps.pm_flags & PMF_FOLLOW)) {
		return;
	}

	if (cg.renderingThirdPerson) {
		return;
	}
	// set color based on health
	if (cg_crosshairHealth.integer) {
		vec4_t hcolor;

		CG_ColorForHealth(hcolor);
		trap_R_SetColor(hcolor);
	} else {
		trap_R_SetColor(NULL);
	}

	w = h = cg_crosshairSize.value;

	// pulse the size of the crosshair when picking up items
	f = cg.time - cg.itemPickupBlendTime;
	if (f > 0 && f < ITEM_BLOB_TIME) {
		f /= ITEM_BLOB_TIME;
		w *= (1 + f);
		h *= (1 + f);
	}

	//Elder: Sniper crosshairs - lots of hardcoded values :/
	//if ( cg.snap->ps.weapon==WP_SSG3000 && cg.zoomLevel > 0 && cg.zoomLevel < 4) {
	// some pile of crap
	// using SSG and zoomed in
	if (cg.snap->ps.weapon == WP_SSG3000 && ((cg.zoomLevel & RQ3_ZOOM_LOW) || (cg.zoomLevel & RQ3_ZOOM_MED))) {
		if ((cg.zoomFirstReturn == -1 || cg.snap->ps.weaponTime < ZOOM_TIME) &&
		    cg.snap->ps.stats[STAT_RELOADTIME] <= 0) {
			//Makro - wasn't initialized, caused a warning in MSVC
			int zoomMag = 0;

			x = SCREEN_WIDTH / 2;
			y = SCREEN_HEIGHT / 2;

			//derive zoom level
			if ((cg.zoomLevel & RQ3_ZOOM_LOW) == RQ3_ZOOM_LOW &&
			    (cg.zoomLevel & RQ3_ZOOM_MED) == RQ3_ZOOM_MED) {
				zoomMag = 2;
			} else if ((cg.zoomLevel & RQ3_ZOOM_LOW) == RQ3_ZOOM_LOW) {
				zoomMag = 0;
			} else if ((cg.zoomLevel & RQ3_ZOOM_MED) == RQ3_ZOOM_MED) {
				zoomMag = 1;
			} else {
				//Shouldn't need to be here
				CG_Error("CG_DrawCrosshair: received no zoom value\n");
			}

			//Elder: Setup crosshair colours
			crosshairColor[0] = cg_RQ3_ssgColorR.value;
			crosshairColor[1] = cg_RQ3_ssgColorG.value;
			crosshairColor[2] = cg_RQ3_ssgColorB.value;
			crosshairColor[3] = cg_RQ3_ssgColorA.value;

			//Clamp
			for (i = 0; i < 4; i++) {
				if (crosshairColor[i] > 1.0f)
					crosshairColor[i] = 1.0f;
				else if (crosshairColor[i] < 0)
					crosshairColor[i] = 0;
			}

			trap_R_SetColor(crosshairColor);
			//I can probably scale the zoom with the screen width -/+ keys
			//But I'll do it later.
			CG_DrawPic(x - 128, y - 128, 256, 256, cgs.media.ssgCrosshair[zoomMag]);

			trap_R_SetColor(NULL);
			return;
		}
	}
	//else {
	x = cg_crosshairX.integer;
	y = cg_crosshairY.integer;
	CG_AdjustFrom640(&x, &y, &w, &h);

	ca = cg_drawCrosshair.integer;
	if (ca < 0) {
		ca = 0;
	}
	hShader = cgs.media.crosshairShader[ca % NUM_CROSSHAIRS];
	//}
	crosshairColor[0] = cg_RQ3_crosshairColorR.value;
	crosshairColor[1] = cg_RQ3_crosshairColorG.value;
	crosshairColor[2] = cg_RQ3_crosshairColorB.value;
	crosshairColor[3] = cg_RQ3_crosshairColorA.value;
	for (i = 0; i < 4; i++) {
		if (crosshairColor[i] > 1.0f)
			crosshairColor[i] = 1.0f;
		else if (crosshairColor[i] < 0)
			crosshairColor[i] = 0;
	}

	trap_R_SetColor(crosshairColor);
	trap_R_DrawStretchPic(x + cg.refdef.x + 0.5 * (cg.refdef.width - w),
			      y + cg.refdef.y + 0.5 * (cg.refdef.height - h), w, h, 0, 0, 1, 1, hShader);
	trap_R_SetColor(NULL);
}

/*
=================
CG_ScanForCrosshairEntity
=================
*/
static void CG_ScanForCrosshairEntity(void)
{
	trace_t trace;
	vec3_t start, end;
	int content;

	VectorCopy(cg.refdef.vieworg, start);
	VectorMA(start, 131072, cg.refdef.viewaxis[0], end);

	CG_Trace(&trace, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum, CONTENTS_SOLID | CONTENTS_BODY);
	if (trace.entityNum >= MAX_CLIENTS) {
		return;
	}
	// if the player is in fog, don't show it
	content = trap_CM_PointContents(trace.endpos, 0);
	if (content & CONTENTS_FOG) {
		return;
	}
	// if the player is invisible, don't show it
	if (cg_entities[trace.entityNum].currentState.powerups & (1 << PW_INVIS)) {
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
	float *color;
	char *name;
	float w;

	if (!cg_drawCrosshair.integer) {
		return;
	}
	if (!cg_drawCrosshairNames.integer) {
		return;
	}
	if (cg.renderingThirdPerson) {
		return;
	}
// JBravo: no names for zcam users.
	if (cg.snap->ps.stats[STAT_RQ3] & RQ3_ZCAM) {
		return;
	}
	// scan the known entities to see if the crosshair is sighted on one
	CG_ScanForCrosshairEntity();

	// draw the name of the player being looked at
	color = CG_FadeColor(cg.crosshairClientTime, 1000);
	if (!color) {
		trap_R_SetColor(NULL);
		return;
	}
// JBravo: Lets not show player names of opponents
	if (cgs.clientinfo[cg.clientNum].team != cgs.clientinfo[cg.crosshairClientNum].team && cgs.gametype >= GT_TEAM) {
		return;
	}

	name = cgs.clientinfo[cg.crosshairClientNum].name;
	w = CG_DrawStrlen(name) * BIGCHAR_WIDTH;
	CG_DrawBigString(320 - w / 2, 170, name, color[3] * 0.5f);
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
	float Color[4];

	MAKERGBA(Color, 0.0f, 0.0f, 0.0f, 0.4f);

	if (cgs.gametype >= GT_TEAM) {
		if (cg.snap->ps.persistant[PERS_SAVEDTEAM] == TEAM_RED) {
			MAKERGBA(Color, 0.7f, 0.0f, 0.0f, 0.3f);
		}

		if (cg.snap->ps.persistant[PERS_SAVEDTEAM] == TEAM_BLUE) {
			MAKERGBA(Color, 0.0f, 0.0f, 0.7f, 0.3f);
		}
	}

	CG_FillRect(0, 420, 640, 60, Color);

	MAKERGBA(Color, 0.0f, 0.0f, 0.0f, 1.0f);
	CG_DrawCleanLine(0, 420, 640, 1, Color);

	CG_DrawBigString(320 - 10 * 8, 425, "Spectating", 1.0F);

	if (cg.snap->ps.persistant[PERS_SAVEDTEAM] == TEAM_RED || cg.snap->ps.persistant[PERS_SAVEDTEAM] == TEAM_BLUE)
		return;

	if (cgs.gametype == GT_TOURNAMENT) {
		CG_DrawBigString(320 - 15 * 8, 455, "Waiting to play...", 1.0F);
	} else if (cgs.gametype >= GT_TEAM) {
		CG_DrawBigString(320 - 19 * 8, 455, "Join a team to play", 1.0F);
	}
}

/*
=================
CG_DrawVote
=================
*/
static void CG_DrawVote(void)
{
	char *s;
	int sec;

	if (!cgs.voteTime) {
		return;
	}
	// play a talk beep whenever it is modified
	if (cgs.voteModified) {
		cgs.voteModified = qfalse;
		trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
	}

	sec = (VOTE_TIME - (cg.time - cgs.voteTime)) / 1000;
	if (sec < 0) {
		sec = 0;
	}
	s = va("VOTE(%i):%s yes:%i no:%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo);
	CG_DrawSmallString(0, 58, s, 1.0F);
}

/*
=================
CG_DrawTeamVote
=================
*/
static void CG_DrawTeamVote(void)
{
	char *s;
	int sec, cs_offset;

	if (cgs.clientinfo->team == TEAM_RED)
		cs_offset = 0;
	else if (cgs.clientinfo->team == TEAM_BLUE)
		cs_offset = 1;
	else
		return;

	if (!cgs.teamVoteTime[cs_offset]) {
		return;
	}
	// play a talk beep whenever it is modified
	if (cgs.teamVoteModified[cs_offset]) {
		cgs.teamVoteModified[cs_offset] = qfalse;
		trap_S_StartLocalSound(cgs.media.talkSound, CHAN_LOCAL_SOUND);
	}

	sec = (VOTE_TIME - (cg.time - cgs.teamVoteTime[cs_offset])) / 1000;
	if (sec < 0) {
		sec = 0;
	}
	s = va("TEAMVOTE(%i):%s yes:%i no:%i", sec, cgs.teamVoteString[cs_offset],
	       cgs.teamVoteYes[cs_offset], cgs.teamVoteNo[cs_offset]);
	CG_DrawSmallString(0, 90, s, 1.0F);
}

static qboolean CG_DrawScoreboard()
{
	return CG_DrawOldScoreboard();
}

/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission(void)
{
	if (cgs.gametype == GT_SINGLE_PLAYER) {
		CG_DrawCenterString();
		return;
	}
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
	float x;
	vec4_t color;
	const char *name;
	int team;
	char combinedName[512];

	if (!(cg.snap->ps.pm_flags & PMF_FOLLOW)) {
		return qfalse;
	}
	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;

// JBravo: if gametype >= team, append teamname to his name.
	if (cgs.gametype >= GT_TEAM) {
		team = cgs.clientinfo[cg.snap->ps.clientNum].team;
		if (team == TEAM_RED) {
			Com_sprintf(combinedName, sizeof(combinedName), "%sFollowing%s %s%s/%s%s", S_COLOR_RED,
				    S_COLOR_WHITE, cgs.clientinfo[cg.snap->ps.clientNum].name, S_COLOR_RED,
				    S_COLOR_MAGENTA, cg_RQ3_team1name.string);
		} else {
			Com_sprintf(combinedName, sizeof(combinedName), "%sFollowing%s %s%s/%s%s", S_COLOR_RED,
				    S_COLOR_WHITE, cgs.clientinfo[cg.snap->ps.clientNum].name, S_COLOR_RED,
				    S_COLOR_MAGENTA, cg_RQ3_team2name.string);
		}
		x = 0.5 * (640 - BIGCHAR_WIDTH * CG_DrawStrlen(combinedName));
		CG_DrawStringExt(x, 372, combinedName, color, qfalse, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0);
	} else {
		CG_DrawBigString(320 - 9 * 8, 24, "following", 1.0F);
		name = cgs.clientinfo[cg.snap->ps.clientNum].name;
		x = 0.5 * (640 - GIANT_WIDTH * CG_DrawStrlen(combinedName));
		CG_DrawStringExt(x, 40, name, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0);
	}

	return qtrue;
}

/*
=================
CG_DrawAmmoWarning
=================
*/
static void CG_DrawAmmoWarning(void)
{
	const char *s;
	int w;

	if (cg_drawAmmoWarning.integer == 0) {
		return;
	}

	if (!cg.lowAmmoWarning) {
		return;
	}

	if (cg.lowAmmoWarning == 2) {
		s = "OUT OF AMMO";
	} else {
		s = "LOW AMMO WARNING";
	}
	w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
	//Elder: commented out for now
	//CG_DrawBigString(320 - w / 2, 64, s, 1.0F);
}

/*
=================
CG_DrawWarmup
=================
*/
static void CG_DrawWarmup(void)
{
	int w;
	int sec;
	int i;
	float scale;
	clientInfo_t *ci1, *ci2;
	int cw;
	const char *s;

	sec = cg.warmup;
	if (!sec) {
		return;
	}

	if (sec < 0) {
		s = "Waiting for players";
		w = CG_DrawStrlen(s) * BIGCHAR_WIDTH;
		CG_DrawBigString(320 - w / 2, 24, s, 1.0F);
		cg.warmupCount = 0;
		return;
	}

	if (cgs.gametype == GT_TOURNAMENT) {
		// find the two active players
		ci1 = NULL;
		ci2 = NULL;
		for (i = 0; i < cgs.maxclients; i++) {
			if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_FREE) {
				if (!ci1) {
					ci1 = &cgs.clientinfo[i];
				} else {
					ci2 = &cgs.clientinfo[i];
				}
			}
		}

		if (ci1 && ci2) {
			s = va("%s vs %s", ci1->name, ci2->name);
			w = CG_DrawStrlen(s);
			if (w > 640 / GIANT_WIDTH) {
				cw = 640 / w;
			} else {
				cw = GIANT_WIDTH;
			}
			CG_DrawStringExt(320 - w * cw / 2, 20, s, colorWhite, qfalse, qtrue, cw, (int) (cw * 1.5f), 0);
		}
	} else {
		if (cgs.gametype == GT_FFA) {
			s = "Free For All";
		} else if (cgs.gametype == GT_TEAM) {
			s = "Team Deathmatch";
// JBravo adding teamplay
		} else if (cgs.gametype == GT_TEAMPLAY) {
			s = "RQ3 Teamplay";
		} else if (cgs.gametype == GT_CTF) {
			s = "Capture the Flag";
		} else {
			s = "";
		}
		w = CG_DrawStrlen(s);
		if (w > 640 / GIANT_WIDTH) {
			cw = 640 / w;
		} else {
			cw = GIANT_WIDTH;
		}
		CG_DrawStringExt(320 - w * cw / 2, 25, s, colorWhite, qfalse, qtrue, cw, (int) (cw * 1.1f), 0);
	}

	sec = (sec - cg.time) / 1000;
	if (sec < 0) {
		cg.warmup = 0;
		sec = 0;
	}
	s = va("Starts in: %i", sec + 1);
	if (sec != cg.warmupCount) {
		cg.warmupCount = sec;
		switch (sec) {
		case 0:
			if (cg_RQ3_anouncer.integer == 1)
				trap_S_StartLocalSound(cgs.media.count1Sound, CHAN_ANNOUNCER);
			break;
		case 1:
			if (cg_RQ3_anouncer.integer == 1)
				trap_S_StartLocalSound(cgs.media.count2Sound, CHAN_ANNOUNCER);
			break;
		case 2:
			if (cg_RQ3_anouncer.integer == 1)
				trap_S_StartLocalSound(cgs.media.count3Sound, CHAN_ANNOUNCER);
			break;
		default:
			break;
		}
	}
	scale = 0.45f;
	switch (cg.warmupCount) {
	case 0:
		cw = 28;
		scale = 0.54f;
		break;
	case 1:
		cw = 24;
		scale = 0.51f;
		break;
	case 2:
		cw = 20;
		scale = 0.48f;
		break;
	default:
		cw = 16;
		scale = 0.45f;
		break;
	}

	w = CG_DrawStrlen(s);
	CG_DrawStringExt(320 - w * cw / 2, 70, s, colorWhite, qfalse, qtrue, cw, (int) (cw * 1.5), 0);
}

//==================================================================================
/*
=================
CG_Draw2D
=================
*/
static void CG_Draw2D(void)
{
	// if we are taking a levelshot for the menu, don't draw anything
	if (cg.levelShot) {
		return;
	}

	if (cg_draw2D.integer == 0) {
		return;
	}

	if (cg.snap->ps.pm_type == PM_INTERMISSION) {
		CG_DrawIntermission();
		return;
	}

/*
	if (cg.cameraMode) {
		return;
	}
*/
	//Slicer: Adding HUD for follow spectating
	if ((cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR && !(cg.snap->ps.pm_flags & PMF_FOLLOW)) ||
	    cg.predictedPlayerState.pm_type == PM_SPECTATOR) {
		//if (cg.snap->ps.persistant[PERS_SAVEDTEAM] != TEAM_RED && cg.snap->ps.persistant[PERS_SAVEDTEAM] != TEAM_BLUE)
		// cg.predictedPlayerState.pm_type == PM_SPECTATOR
// JBravo: no spectator bar for zcam modes.
		if (!(cg.snap->ps.stats[STAT_RQ3] & RQ3_ZCAM)) {
			CG_DrawSpectator();
		}
		CG_DrawCrosshair();
		CG_DrawCrosshairNames();
	} else {
		// don't draw any status if dead or the scoreboard is being explicitly shown
		// if ( !cg.showScores && cg.snap->ps.stats[STAT_HEALTH] > 0 ) {
		if (cg.snap->ps.stats[STAT_HEALTH] > 0) {
			CG_DrawStatusBar();
			CG_DrawAmmoWarning();
			CG_DrawCrosshair();
			CG_DrawCrosshairNames();
			CG_DrawWeaponSelect();
			CG_DrawHoldableItem();
			CG_DrawReward();
		}

		if (cgs.gametype >= GT_TEAM) {
			CG_DrawTeamInfo();
		}
	}

	CG_DrawVote();
	CG_DrawTeamVote();
	CG_DrawLagometer();
	CG_DrawUpperRight();
	CG_DrawLowerRight();
	CG_DrawLowerLeft();

	if (!CG_DrawFollow()) {
		CG_DrawWarmup();
	}
	// don't draw center string if scoreboard is up
	cg.scoreBoardShowing = CG_DrawScoreboard();
	if (!cg.scoreBoardShowing) {
		CG_DrawCenterString();
	}
	if (cg.showWStats)
		CG_DrawWeaponStats();
}

static void CG_DrawTourneyScoreboard()
{
	CG_DrawOldTourneyScoreboard();
}

/*
=====================
CG_DrawDamageBlend

Elder: Does a fullscreen alpha blend like Quake 2 when hurt
=====================
*/
#define MAX_DAMAGE_ALPHA	0.75
#define MAX_BLEND_TIME		1500
static void CG_DrawDamageBlend()
{
	float dmg;
	vec4_t damageColor;

	//CG_Printf("CG_DrawDamageBlend: trueDamage (%i)\n", cg.rq3_trueDamage);

	//Leave if no true damage, disabled, or ragepro
	if (!cg_RQ3_painblend.integer)
		return;

	if (!cg.rq3_trueDamage || cgs.glconfig.hardwareType == GLHW_RAGEPRO)
		return;

	//Clamp blend time
	if (cg.rq3_blendTime > MAX_BLEND_TIME)
		cg.rq3_blendTime = MAX_BLEND_TIME;

	//Reset if we've gone past our blendTime
	if (cg.time - cg.damageTime > cg.rq3_blendTime) {
		//cg.rq3_trueDamage = 0;
		cg.rq3_blendTime = 0;
		return;
	}

	VectorCopy(colorRed, damageColor);
	dmg = cg.rq3_trueDamage;

	//clamp at 100 health
	if (dmg > 100)
		dmg = 100;

	damageColor[3] = MAX_DAMAGE_ALPHA * (dmg / 100.0) * (1.0 - (cg.time - cg.damageTime) / cg.rq3_blendTime);

	if (damageColor[3] > MAX_DAMAGE_ALPHA)
		damageColor[3] = MAX_DAMAGE_ALPHA;
	else if (damageColor[3] < 0)
		damageColor[3] = 0;

	CG_FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, damageColor);
}

/*
=====================
CG_DrawIRBlend

Elder: Small red tint
Note: This sucks - causes 10fps drop on my system so don't use it
=====================
// JBravo: apparently not used.
*/
/*
static void CG_DrawIRBlend()
{
	vec4_t irColor;

	if (bg_itemlist[cg.snap->ps.stats[STAT_HOLDABLE_ITEM]].giTag == HI_BANDOLIER
		&& cg.rq3_irvision)
	{
		irColor[0] = 0;
		irColor[1] = 1.0f;
		irColor[2] = 0;
		irColor[3] = 0.1f;
		CG_FillRect(0,0,SCREEN_WIDTH, SCREEN_HEIGHT, irColor);
	}
}
*/

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive(stereoFrame_t stereoView)
{
	float separation;
	vec3_t baseOrg;

	// optionally draw the info screen instead
	if (!cg.snap) {
		CG_DrawInformation();
		return;
	}
	// optionally draw the tournement scoreboard instead
	if (cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR && (cg.snap->ps.pm_flags & PMF_SCOREBOARD)) {
		CG_DrawTourneyScoreboard();
		return;
	}

	switch (stereoView) {
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
	}

	// clear around the rendered view if sized down
	CG_TileClear();

	// offset vieworg appropriately if we're doing stereo separation
	VectorCopy(cg.refdef.vieworg, baseOrg);
	if (separation != 0) {
		VectorMA(cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg);
	}
	// draw 3D view
	trap_R_RenderScene(&cg.refdef);

	// restore original viewpoint if running stereo
	if (separation != 0) {
		VectorCopy(baseOrg, cg.refdef.vieworg);
	}
	// Elder: draw damage blend
	CG_DrawDamageBlend();
	//CG_DrawIRBlend();

	// draw status bar and other floating elements
	CG_Draw2D();
}
