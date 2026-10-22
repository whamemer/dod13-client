//
// hud_safearea.h
//
// The safe area of the screen: a phone has a cutout, rounded corners and bezels that eat the parts of the HUD that sit in the corners.
// The engine reports the part of the window that is safe (the cutout) and the client adds a margin for the corners, then the elements of
// the HUD that are attached to an edge of the screen are moved from it by that much.
//
// The drawing functions of gEngfuncs are replaced by ones that add the offset of the current anchor to the position, so the code of the
// HUD does not change: CHud::Redraw sets the anchor of the element before its Draw (CHudBase::m_iSafeAnchor), and a part of the element
// that sits at another edge sets its own with CSafeAnchor. Outside the HUD (the anchor is SAFE_NONE) the functions only pass the call on.
// With no cutout and no margin (a desktop) the offsets are zero.
//

#ifndef HUD_SAFEAREA_H
#define HUD_SAFEAREA_H

// the edges that a drawing is attached to
#define SAFE_NONE	0
#define SAFE_LEFT	( 1 << 0 )
#define SAFE_RIGHT	( 1 << 1 )
#define SAFE_TOP	( 1 << 2 )
#define SAFE_BOTTOM	( 1 << 3 )

// hooks the drawing functions of gEngfuncs and registers hud_safearea
void HUD_SafeAreaInit( void );

// once per frame: the insets in the units of the HUD (the pixels of ScreenWidth and ScreenHeight)
void HUD_SafeAreaThink( void );

// sets the anchor of the next drawings, returns the previous one
int HUD_SafeAnchor( int anchor );

// how far the anchor moves a drawing: to the right and down
void HUD_SafeAreaOffset( int anchor, int &dx, int &dy );

// the scope of an anchor
class CSafeAnchor
{
public:
	CSafeAnchor( int anchor ) : m_iOld( HUD_SafeAnchor( anchor ) ) {}
	~CSafeAnchor() { HUD_SafeAnchor( m_iOld ); }

private:
	int m_iOld;
};

#endif // HUD_SAFEAREA_H
