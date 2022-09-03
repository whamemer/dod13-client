/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "cvardef.h"
#include "dod_shared.h"

char weaponnames[64][64];

cvar_t *pcvar_classlimitcvars[28];

char classlimitcvarNames[27][32];

char *sPlayerModelFiles[6];

char *s_USVoiceFiles[28];
char *s_BRITVoiceFiles[28];
char *s_GERVoiceFiles[29];

int iVoiceToHandSignal[29];
char *s_VoiceCommands[29][4];
char *s_HandSignalSubtitles[25][3];
char *sHandSignals[30];

DodClassInfo_t g_ParaClassInfo[21];
DodClassInfo_t g_ClassInfo[28];

struct p_wpninfo_s P_WpnInfo[41];

char *sHelmetModels[7];

char *szMapMarkerIcons[15];