#include <imgui.h>
#include <imgui_internal.h>
#include <math.h>
#include <misc/freetype/imgui_freetype.h>

#include "Fonts.hpp"
#include "profiler/IconsFontAwesome6.h"
#include "profiler/TracyEmbed.hpp"

#include "data/FontFixed.hpp"
#include "data/FontIcons.hpp"
#include "data/FontNormal.hpp"
#include "data/FontBold.hpp"
#include "data/FontBoldItalic.hpp"
#include "data/FontItalic.hpp"
#include "data/FontEmoji.hpp"

namespace tracy
{

FontData g_fonts = {};
float FontNormal = 0, FontSmall = 0, FontBig = 0;
static bool s_fontsLoaded = false;
static bool s_needsRebuild = false;

bool FontsNeedRebuild()
{
    return s_needsRebuild;
}

void FontsRebuilt()
{
    s_needsRebuild = false;
}

void LoadFonts( float scale )
{
    if( s_fontsLoaded )
        return;

    auto* atlas = ImGui::GetIO().Fonts;

    // If atlas is already built, we need to trigger a rebuild after adding fonts
    bool wasBuilt = atlas->IsBuilt();

    ImFontConfig configBasic;
    configBasic.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_LightHinting;
    configBasic.FontDataOwnedByAtlas = false;
    ImFontConfig configMerge;
    configMerge.MergeMode = true;
    configMerge.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_LightHinting;
    configMerge.FontDataOwnedByAtlas = false;
    ImFontConfig configFixed;
    configFixed.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_LightHinting;
    configFixed.GlyphExtraAdvanceX = -1;
    configFixed.FontDataOwnedByAtlas = false;

    auto fontFixed = Unembed( FontFixed );
    auto fontIcons = Unembed( FontIcons );
    auto fontNormal = Unembed( FontNormal );
    auto fontBold = Unembed( FontBold );
    auto fontBoldItalic = Unembed( FontBoldItalic );
    auto fontItalic = Unembed( FontItalic );
    auto fontEmoji = Unembed( FontEmoji );

    // Add Tracy fonts as separate entries in the main atlas
    g_fonts.normal = atlas->AddFontFromMemoryTTF( (void*)fontNormal->data(), fontNormal->size(), round( 15.0f * scale ), &configBasic );
    atlas->AddFontFromMemoryTTF( (void*)fontIcons->data(), fontIcons->size(), round( 14.0f * scale ), &configMerge );
    atlas->AddFontFromMemoryTTF( (void*)fontEmoji->data(), fontEmoji->size(), round( 14.0f * scale ), &configMerge );

    g_fonts.mono = atlas->AddFontFromMemoryTTF( (void*)fontFixed->data(), fontFixed->size(), round( 15.0f * scale ), &configFixed );
    atlas->AddFontFromMemoryTTF( (void*)fontIcons->data(), fontIcons->size(), round( 14.0f * scale ), &configMerge );
    atlas->AddFontFromMemoryTTF( (void*)fontEmoji->data(), fontEmoji->size(), round( 14.0f * scale ), &configMerge );

    g_fonts.bold = atlas->AddFontFromMemoryTTF( (void*)fontBold->data(), fontBold->size(), round( 15.0f * scale ), &configBasic );
    atlas->AddFontFromMemoryTTF( (void*)fontIcons->data(), fontIcons->size(), round( 14.0f * scale ), &configMerge );
    atlas->AddFontFromMemoryTTF( (void*)fontEmoji->data(), fontEmoji->size(), round( 14.0f * scale ), &configMerge );

    g_fonts.boldItalic = atlas->AddFontFromMemoryTTF( (void*)fontBoldItalic->data(), fontBoldItalic->size(), round( 15.0f * scale ), &configBasic );
    atlas->AddFontFromMemoryTTF( (void*)fontIcons->data(), fontIcons->size(), round( 14.0f * scale ), &configMerge );
    atlas->AddFontFromMemoryTTF( (void*)fontEmoji->data(), fontEmoji->size(), round( 14.0f * scale ), &configMerge );

    g_fonts.italic = atlas->AddFontFromMemoryTTF( (void*)fontItalic->data(), fontItalic->size(), round( 15.0f * scale ), &configBasic );
    atlas->AddFontFromMemoryTTF( (void*)fontIcons->data(), fontIcons->size(), round( 14.0f * scale ), &configMerge );
    atlas->AddFontFromMemoryTTF( (void*)fontEmoji->data(), fontEmoji->size(), round( 14.0f * scale ), &configMerge );

    FontNormal = round( scale * 15.f );
    FontSmall = round( scale * 15 * 2.f / 3.f );
    FontBig = round( scale * 15 * 1.4f );

    s_fontsLoaded = true;

    if( wasBuilt )
    {
        // Atlas was already built, need to rebuild it
        atlas->Build();
        s_needsRebuild = true;
    }
}

void FreeFonts()
{
    g_fonts = {};
    s_fontsLoaded = false;
    s_needsRebuild = false;
}

}
