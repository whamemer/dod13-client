//
// dod_fontplat.cpp
//
// The font backend of freevgui for the platforms without GDI, see dod_fontplat.h. The sizes follow the rules of CreateFont:
// a positive tall is the height of the cell (the ascent and the descent), a negative one is the size of the em, 0 is 12.
// The weight from 600 is drawn by smearing the glyph by a pixel, the italic is a shear of its rows, as neither file has such a
// style. The underline, the strikeout, the rotation, the width and the symbol charset are not supported, nothing asks for them.
//
// A font never fails to be made and does not load its file when it is made. freevgui makes its own fonts (the default ones of its
// scheme) when the engine starts it, before the client runs and can give the loader; a font that could not be made there became the
// debugging font of freevgui with boxes and hex codes, and that one is taken by every later font of the same height, because it is
// compared by the height only. So the file is loaded with the first use of the font, and until it is there the font has the metrics
// of its size and draws nothing.
//

// waf builds every file of cl_dll, but Windows has GDI and the include directories of freevgui are set by CMake only, which
// defines DOD_FONTPLAT for this file, so for anything else it is empty
#ifdef DOD_FONTPLAT

#include "dod_fontplat.h"
#include "platform/common/font.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include "stb_truetype.h"

using namespace vgui;

namespace
{

const float ITALIC_SLANT = 0.2f;
const int BOLD_WEIGHT = 600;
const int DEFAULT_TALL = 12;

struct FontFile_t
{
	const char *pszPath;
	stbtt_fontinfo info;
};

// the first one is for everything, the second is for the face that the main menu draws with it
FontFile_t s_FontFiles[] =
{
	{ "gfx/fonts/tahoma.ttf" },
	{ "gfx/fonts/FiraSans-Regular.ttf" }
};

DoDLoadFontFileFn s_pfnLoad = NULL;
DoDFreeFontFileFn s_pfnFree = NULL;

// the file stays in memory as long as the game runs, every font of it uses the same data. A file that is not there is asked for
// again with the next use of a font
FontFile_t *LoadFontFile( FontFile_t &file )
{
	if( file.info.data )
		return &file;

	if( !s_pfnLoad )
		return NULL;

	int iLength = 0;
	unsigned char *pData = s_pfnLoad( file.pszPath, &iLength );

	if( !pData )
		return NULL;

	if( !stbtt_InitFont( &file.info, pData, stbtt_GetFontOffsetForIndex( pData, 0 ) ) )
	{
		if( s_pfnFree )
			s_pfnFree( pData );

		memset( &file.info, 0, sizeof( file.info ) );
		return NULL;
	}

	return &file;
}

FontFile_t *FindFontFile( const char *pszName )
{
	bool bFira = !strncmp( pszName, "Trebuchet", 9 );
	FontFile_t *pFile = LoadFontFile( s_FontFiles[bFira ? 1 : 0] );

	if( !pFile )
		pFile = LoadFontFile( s_FontFiles[bFira ? 0 : 1] );

	return pFile;
}

struct GlyphBox_t
{
	// the box of the ink of stb_truetype: x from the origin of the glyph to the right, y from the baseline down
	int rx0, ry0, rx1, ry1;

	// the same with the smear and the shear, this is what the glyph occupies in the end
	int x0, x1;

	int advance;
};

class FontPlat_Stb : public BaseFontPlat
{
public:
	FontPlat_Stb( const char *pszName, int newTall, int newWide, float newRotation, int newWeight, bool newItalic, bool newUnderline,
		bool newStrikeout, bool newSymbol );

	virtual ~FontPlat_Stb() override
	{
		delete[] name;
	}

	virtual bool equals( const char *newName, int newTall, int newWide, float newRotation, int newWeight, bool newItalic, bool newUnderline,
		bool newStrikeout, bool newSymbol ) override;
	virtual void getCharRGBA( int ch, int rgbaX, int rgbaY, int rgbaWide, int rgbaTall, unsigned char *rgba ) override;
	virtual void getCharABCwide( int ch, int &a, int &b, int &c ) override;

	virtual int getTall() override
	{
		Prepare();

		return height;
	}

	virtual int getWide() override
	{
		Prepare();

		return maxWide;
	}

	virtual void drawSetTextFont( SurfacePlat * ) override
	{
	}

private:
	bool Prepare();
	void getBox( int ch, GlyphBox_t &box );
	int shearFor( int y ) const;

	FontFile_t *file;
	char *name;
	int tall, wide, weight;
	float rotation;
	bool italic, underline, strikeout, symbol;

	float scale;
	bool bold;
	int ascent, height, maxWide;
};

FontPlat_Stb::FontPlat_Stb( const char *pszName, int newTall, int newWide, float newRotation, int newWeight, bool newItalic,
	bool newUnderline, bool newStrikeout, bool newSymbol ) :
	file( NULL ),
	tall( newTall ), wide( newWide ), weight( newWeight ),
	rotation( newRotation ),
	italic( newItalic ), underline( newUnderline ), strikeout( newStrikeout ), symbol( newSymbol ),
	scale( 0.0f )
{
	name = new char[strlen( pszName ) + 1];
	strcpy( name, pszName );

	bold = weight >= BOLD_WEIGHT;

	// until the file is there: the cell is what the size says (the em is a bit smaller than the cell), the glyphs are half of it
	height = tall < 0 ? (int)( -tall * 1.2f + 0.5f ) : ( tall ? tall : DEFAULT_TALL );
	ascent = height * 4 / 5;
	maxWide = height;
}

// loads the file with the first use and makes the real metrics of the font, false while there is no file
bool FontPlat_Stb::Prepare()
{
	if( file )
		return true;

	file = FindFontFile( name );

	if( !file )
		return false;

	int iAscent, iDescent, iLineGap;

	stbtt_GetFontVMetrics( &file->info, &iAscent, &iDescent, &iLineGap );

	if( tall < 0 )
	{
		scale = stbtt_ScaleForMappingEmToPixels( &file->info, (float)-tall );
		ascent = (int)floorf( iAscent * scale + 0.5f );
		height = ascent + (int)floorf( -iDescent * scale + 0.5f );
	}
	else
	{
		int iCell = tall ? tall : DEFAULT_TALL;

		scale = stbtt_ScaleForPixelHeight( &file->info, (float)iCell );
		ascent = (int)floorf( iAscent * scale + 0.5f );
		height = iCell;
	}

	int x0, y0, x1, y1;

	stbtt_GetFontBoundingBox( &file->info, &x0, &y0, &x1, &y1 );

	maxWide = (int)ceilf( ( x1 - x0 ) * scale ) + ( bold ? 1 : 0 ) + ( italic ? (int)ceilf( height * ITALIC_SLANT ) : 0 );

	return true;
}

// freevgui does not tell the fonts that it reuses from the fonts that it has made: a font that takes an old one gets no number of
// its own (-1) and shares the cache of the glyphs of the surface with every other font that did, so their text is drawn with the
// glyphs of whichever of them was drawn first. A font that is never taken always has a number of its own.
bool FontPlat_Stb::equals( const char *newName, int newTall, int newWide, float newRotation, int newWeight, bool newItalic, bool newUnderline,
	bool newStrikeout, bool newSymbol )
{
	return false;
}

// how far to the right the row y (from the baseline down, the pixel row that starts there) goes with the italic
int FontPlat_Stb::shearFor( int y ) const
{
	return (int)floorf( -( y + 0.5f ) * ITALIC_SLANT + 0.5f );
}

void FontPlat_Stb::getBox( int ch, GlyphBox_t &box )
{
	// the characters of the strings are signed
	if( ch < 0 )
		ch &= 0xFF;

	int iAdvance, iLeftBearing;

	stbtt_GetCodepointHMetrics( &file->info, ch, &iAdvance, &iLeftBearing );
	stbtt_GetCodepointBitmapBox( &file->info, ch, scale, scale, &box.rx0, &box.ry0, &box.rx1, &box.ry1 );

	box.advance = (int)floorf( iAdvance * scale + 0.5f );
	box.x0 = box.rx0;
	box.x1 = box.rx1;

	if( box.rx1 <= box.rx0 || box.ry1 <= box.ry0 )
		return;

	if( bold )
	{
		box.x1++;
		box.advance++;
	}

	if( italic )
	{
		// the shear shrinks with the row going down: the top row goes the farthest to the right, the bottom one the farthest to the left
		box.x0 = box.rx0 + shearFor( box.ry1 - 1 );
		box.x1 = box.rx1 + shearFor( box.ry0 ) + ( bold ? 1 : 0 );
	}
}

void FontPlat_Stb::getCharABCwide( int ch, int &a, int &b, int &c )
{
	// no file yet: the glyphs are not drawn, they only have a width
	if( !Prepare() )
	{
		a = c = 0;
		b = height / 2 ? height / 2 : 1;
		return;
	}

	GlyphBox_t box;

	getBox( ch, box );

	b = box.x1 - box.x0;

	if( b <= 0 )
	{
		// the space and the like: the advance goes to the right bearing
		a = b = 0;
		c = box.advance;
		return;
	}

	a = box.x0;
	c = box.advance - a - b;
}

// white, the coverage is the alpha: the surface paints it with the color of the text
void FontPlat_Stb::getCharRGBA( int ch, int rgbaX, int rgbaY, int rgbaWide, int rgbaTall, unsigned char *rgba )
{
	if( !rgba || !Prepare() )
		return;

	GlyphBox_t box;

	getBox( ch, box );

	int w = box.rx1 - box.rx0;
	int h = box.ry1 - box.ry0;

	if( w <= 0 || h <= 0 )
		return;

	if( ch < 0 )
		ch &= 0xFF;

	unsigned char *pCoverage = (unsigned char *)malloc( w * h );

	if( !pCoverage )
		return;

	memset( pCoverage, 0, w * h );
	stbtt_MakeCodepointBitmap( &file->info, pCoverage, w, h, w, scale, scale, ch );

	for( int y = 0; y < h; y++ )
	{
		int cellY = ascent + box.ry0 + y;

		// the glyph stays inside the cell, and the cell inside the buffer
		if( cellY < 0 || cellY >= height || rgbaY + cellY >= rgbaTall )
			continue;

		int shear = italic ? shearFor( box.ry0 + y ) : 0;

		for( int x = 0; x < w; x++ )
		{
			unsigned char coverage = pCoverage[y * w + x];

			if( !coverage )
				continue;

			// the box of the glyph is at the origin of the cell, the surface adds the left bearing itself
			int cellX = box.rx0 + x + shear - box.x0;

			for( int i = 0; i < ( bold ? 2 : 1 ); i++ )
			{
				int px = rgbaX + cellX + i;
				int py = rgbaY + cellY;

				if( cellX + i < 0 || px >= rgbaWide || py >= rgbaTall )
					continue;

				unsigned char *dst = &rgba[( py * rgbaWide + px ) * 4];

				dst[0] = dst[1] = dst[2] = 255;

				if( dst[3] < coverage )
					dst[3] = coverage;
			}
		}
	}

	free( pCoverage );
}

}

void DoD_SetFontFileFuncs( DoDLoadFontFileFn pfnLoad, DoDFreeFontFileFn pfnFree )
{
	s_pfnLoad = pfnLoad;
	s_pfnFree = pfnFree;
}

BaseFontPlat *vgui::FontPlat_CreateSystem( const char *name, int tall, int wide, float rotation, int weight, bool italic, bool underline,
	bool strikeout, bool symbol )
{
	// never null, see the top of the file: the debugging font of freevgui is what a null becomes
	return new FontPlat_Stb( name ? name : "", tall, wide, rotation, weight, italic, underline, strikeout, symbol );
}

#endif // DOD_FONTPLAT
