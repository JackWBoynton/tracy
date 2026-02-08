#ifndef __FONTS_HPP__
#define __FONTS_HPP__

struct ImFont;

namespace tracy
{

struct FontData
{
    ImFont* normal;
    ImFont* mono;
    ImFont* bold;
    ImFont* boldItalic;
    ImFont* italic;
};

extern FontData g_fonts;
extern float FontNormal, FontSmall, FontBig;

// Load Tracy's fonts into ImGui's main atlas as separate font entries.
// Can be called anytime - will trigger atlas rebuild if needed.
void LoadFonts( float scale );

// Check if fonts were added after atlas was built (needs texture re-upload)
bool FontsNeedRebuild();

// Call after backend has rebuilt the font texture
void FontsRebuilt();

// Reset font state
void FreeFonts();

}

#endif
