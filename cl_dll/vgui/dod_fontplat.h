//
// dod_fontplat.h
//
// The system fonts of freevgui outside of Windows. freevgui rasterizes them through GDI on Windows only, platform/posix/font.cpp has no
// backend and every font made by name becomes the debugging font with boxes and hex codes instead of letters. dod_fontplat.cpp is such a
// backend: it draws the text with the TrueType files that the extras of the engine carry (the main menu uses them too). It is not
// compiled on Windows, and a freevgui that is a shared library keeps its own stub.
//
// The file is compiled apart from the code of the client because the headers of freevgui and the original ones of VGUI cannot be
// included together, so the engine is reached through two functions that the client gives.
//

#ifndef DOD_FONTPLAT_H
#define DOD_FONTPLAT_H

// loads a file of the virtual file system of the engine, returns NULL when there is no such file
typedef unsigned char *( *DoDLoadFontFileFn )( const char *pszPath, int *pLength );
typedef void ( *DoDFreeFontFileFn )( void *pData );

// has to be called before the first font is made
void DoD_SetFontFileFuncs( DoDLoadFontFileFn pfnLoad, DoDFreeFontFileFn pfnFree );

#endif // DOD_FONTPLAT_H
