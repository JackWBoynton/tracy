#ifndef __TRACYTEXTURE_HPP__
#define __TRACYTEXTURE_HPP__

#include <functional>
#include <imgui.h>

// Global zigzag texture initialized by InitTexture()
extern ImTextureID zigzagTex;

namespace tracy
{

// Texture backend callbacks for custom backends (Vulkan, Metal, D3D, etc.)
// Set these before calling InitTexture() to use a custom backend.
// If not set, falls back to built-in OpenGL3 implementation (if TRACY_BACKEND_OPENGL3 defined).
struct TextureCallbacks
{
    ImTextureID (*MakeTexture)( bool zigzag ) = nullptr;
    void (*FreeTexture)( ImTextureID tex, void(*runOnMainThread)(const std::function<void()>&, bool) ) = nullptr;
    void (*UpdateTexture)( ImTextureID tex, const char* data, int w, int h ) = nullptr;
    void (*UpdateTextureRGBA)( ImTextureID tex, void* data, int w, int h ) = nullptr;
    void (*UpdateTextureRGBAMips)( ImTextureID tex, void** data, int* w, int* h, size_t mips ) = nullptr;
};

// Set custom texture callbacks for non-OpenGL backends.
// Call this before InitTexture().
void SetTextureCallbacks( const TextureCallbacks& callbacks );

void InitTexture();
ImTextureID MakeTexture( bool zigzag = false );
void FreeTexture( ImTextureID tex, void(*runOnMainThread)(const std::function<void()>&, bool) );
void UpdateTexture( ImTextureID tex, const char* data, int w, int h );
void UpdateTextureRGBA( ImTextureID tex, void* data, int w, int h );
void UpdateTextureRGBAMips( ImTextureID tex, void** data, int* w, int* h, size_t mips );

}

#endif
