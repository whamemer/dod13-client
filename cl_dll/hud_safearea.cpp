//
// hud_safearea.cpp
//
// see hud_safearea.h
//

#include "hud.h"
#include "cl_util.h"
#include "cl_dll.h"
#include "hud_safearea.h"

// the margin that the corners of a touch screen get with hud_safearea -1, as a part of the height of the screen: the radius of the
// corners of a phone is about 4% of its short side
#define SAFE_AUTO_MARGIN	0.03f

// the part of the screen that a margin may take at most
#define SAFE_MAX_PART		0.25f

static cvar_t *hud_safearea = NULL;
static cvar_t *touch_enable = NULL;

// left, top, right, bottom of the window as the parts of its size, the engine updates them every frame
static const float *s_pWindowInsets = NULL;

// the same in the units of the HUD
static int s_iInset[4] = { 0, 0, 0, 0 };

static int s_iAnchor = SAFE_NONE;

typedef void ( *FillFn_t )( int x, int y, int width, int height, int r, int g, int b, int a );
typedef void ( *SprDrawFn_t )( int frame, int x, int y, const wrect_t *prc );
typedef void ( *SprDrawGenericFn_t )( int frame, int x, int y, const wrect_t *prc, int blendsrc, int blenddst, int width, int height );
typedef void ( *ScissorFn_t )( int x, int y, int width, int height );
typedef int ( *DrawCharFn_t )( int x, int y, int number, int r, int g, int b );
typedef int ( *DrawStringFn_t )( int x, int y, const char *string );

static FillFn_t s_pfnFillRGBA = NULL;
static FillFn_t s_pfnFillRGBABlend = NULL;
static SprDrawFn_t s_pfnSPR_Draw = NULL;
static SprDrawFn_t s_pfnSPR_DrawHoles = NULL;
static SprDrawFn_t s_pfnSPR_DrawAdditive = NULL;
static SprDrawGenericFn_t s_pfnSPR_DrawGeneric = NULL;
static ScissorFn_t s_pfnSPR_EnableScissor = NULL;
static DrawCharFn_t s_pfnDrawCharacter = NULL;
static DrawStringFn_t s_pfnDrawConsoleString = NULL;

void HUD_SafeAreaOffset( int anchor, int &dx, int &dy )
{
	dx = 0;
	dy = 0;

	if( anchor & SAFE_LEFT )
		dx = s_iInset[0];
	else if( anchor & SAFE_RIGHT )
		dx = -s_iInset[2];

	if( anchor & SAFE_TOP )
		dy = s_iInset[1];
	else if( anchor & SAFE_BOTTOM )
		dy = -s_iInset[3];
}

static void CurrentOffset( int &dx, int &dy )
{
	HUD_SafeAreaOffset( s_iAnchor, dx, dy );
}

static void Hook_FillRGBA( int x, int y, int width, int height, int r, int g, int b, int a )
{
	int dx, dy;

	CurrentOffset( dx, dy );
	s_pfnFillRGBA( x + dx, y + dy, width, height, r, g, b, a );
}

static void Hook_FillRGBABlend( int x, int y, int width, int height, int r, int g, int b, int a )
{
	int dx, dy;

	CurrentOffset( dx, dy );
	s_pfnFillRGBABlend( x + dx, y + dy, width, height, r, g, b, a );
}

static void Hook_SPR_Draw( int frame, int x, int y, const wrect_t *prc )
{
	int dx, dy;

	CurrentOffset( dx, dy );
	s_pfnSPR_Draw( frame, x + dx, y + dy, prc );
}

static void Hook_SPR_DrawHoles( int frame, int x, int y, const wrect_t *prc )
{
	int dx, dy;

	CurrentOffset( dx, dy );
	s_pfnSPR_DrawHoles( frame, x + dx, y + dy, prc );
}

static void Hook_SPR_DrawAdditive( int frame, int x, int y, const wrect_t *prc )
{
	int dx, dy;

	CurrentOffset( dx, dy );
	s_pfnSPR_DrawAdditive( frame, x + dx, y + dy, prc );
}

static void Hook_SPR_DrawGeneric( int frame, int x, int y, const wrect_t *prc, int blendsrc, int blenddst, int width, int height )
{
	int dx, dy;

	CurrentOffset( dx, dy );
	s_pfnSPR_DrawGeneric( frame, x + dx, y + dy, prc, blendsrc, blenddst, width, height );
}

// the clipping rectangle of the sprites that follow
static void Hook_SPR_EnableScissor( int x, int y, int width, int height )
{
	int dx, dy;

	CurrentOffset( dx, dy );
	s_pfnSPR_EnableScissor( x + dx, y + dy, width, height );
}

// returns the width of the character, the position is not in it
static int Hook_DrawCharacter( int x, int y, int number, int r, int g, int b )
{
	int dx, dy;

	CurrentOffset( dx, dy );
	return s_pfnDrawCharacter( x + dx, y + dy, number, r, g, b );
}

// returns the position after the string: in the coordinates of the caller, without the offset
static int Hook_DrawConsoleString( int x, int y, const char *string )
{
	int dx, dy;

	CurrentOffset( dx, dy );
	return s_pfnDrawConsoleString( x + dx, y + dy, string ) - dx;
}

void HUD_SafeAreaInit( void )
{
	// once: the table of the engine is copied to the client at the start of the client
	if( s_pfnFillRGBA )
		return;

	hud_safearea = gEngfuncs.pfnRegisterVariable( "hud_safearea", "-1", FCVAR_ARCHIVE );

	s_pfnFillRGBA = gEngfuncs.pfnFillRGBA;
	s_pfnFillRGBABlend = gEngfuncs.pfnFillRGBABlend;
	s_pfnSPR_Draw = gEngfuncs.pfnSPR_Draw;
	s_pfnSPR_DrawHoles = gEngfuncs.pfnSPR_DrawHoles;
	s_pfnSPR_DrawAdditive = gEngfuncs.pfnSPR_DrawAdditive;
	s_pfnSPR_DrawGeneric = gEngfuncs.pfnSPR_DrawGeneric;
	s_pfnSPR_EnableScissor = gEngfuncs.pfnSPR_EnableScissor;
	s_pfnDrawCharacter = gEngfuncs.pfnDrawCharacter;
	s_pfnDrawConsoleString = gEngfuncs.pfnDrawConsoleString;

	gEngfuncs.pfnFillRGBA = Hook_FillRGBA;
	gEngfuncs.pfnFillRGBABlend = Hook_FillRGBABlend;
	gEngfuncs.pfnSPR_Draw = Hook_SPR_Draw;
	gEngfuncs.pfnSPR_DrawHoles = Hook_SPR_DrawHoles;
	gEngfuncs.pfnSPR_DrawAdditive = Hook_SPR_DrawAdditive;
	gEngfuncs.pfnSPR_DrawGeneric = Hook_SPR_DrawGeneric;
	gEngfuncs.pfnSPR_EnableScissor = Hook_SPR_EnableScissor;
	gEngfuncs.pfnDrawCharacter = Hook_DrawCharacter;
	gEngfuncs.pfnDrawConsoleString = Hook_DrawConsoleString;
}

int HUD_SafeAnchor( int anchor )
{
	int old = s_iAnchor;

	s_iAnchor = anchor;

	return old;
}

void HUD_SafeAreaThink( void )
{
	if( !hud_safearea )
		return;

	if( !touch_enable )
		touch_enable = gEngfuncs.pfnGetCvarPointer( "touch_enable" );

	// the safe area of the window, only the engine of Xash3D FWGS has it
	if( !s_pWindowInsets && gMobileEngfuncs && gMobileEngfuncs->pfnGetNativeObject )
		s_pWindowInsets = (const float *)gMobileEngfuncs->pfnGetNativeObject( "WindowInsets" );

	// the corners: a margin from every edge, as a part of the height of the screen. -1 is automatic: a touch screen has rounded
	// corners, a monitor has none
	float flMargin = hud_safearea->value;

	if( flMargin < 0.0f )
		flMargin = ( touch_enable && touch_enable->value ) ? SAFE_AUTO_MARGIN : 0.0f;

	int iMargin = (int)( flMargin * ScreenHeight + 0.5f );

	for( int i = 0; i < 4; i++ )
	{
		int iSize = ( i & 1 ) ? ScreenHeight : ScreenWidth;
		float flPart = s_pWindowInsets ? s_pWindowInsets[i] : 0.0f;

		// not a number, or a nonsense
		if( !( flPart > 0.0f ) )
			flPart = 0.0f;

		int iInset = (int)( flPart * iSize + 0.5f );

		// the cutout and the corners are the same edge, so the bigger one is enough
		if( iInset < iMargin )
			iInset = iMargin;

		if( iInset > (int)( iSize * SAFE_MAX_PART ) )
			iInset = (int)( iSize * SAFE_MAX_PART );

		s_iInset[i] = iInset;
	}
}
