#include "RenderBackend.h"

#if defined(__WII__)

#include <gccore.h>
#include <malloc.h>
#include <unordered_map>
#include <vector>

namespace RenderBackend {

namespace {

struct WiiTexture {
    GXTexObj texObj;
    std::vector<unsigned char> backing;
    bool initialized;
    WiiTexture() : initialized(false) {}
};

std::unordered_map<TextureId, WiiTexture> s_textures;
TextureId s_nextTextureId = 1;

void initTexObj(WiiTexture& tex, const TextureData& img) {
    if (!img.data || img.w <= 0 || img.h <= 0) {
        tex.initialized = false;
        return;
    }

    const bool useRgb565 = !img.transparent;

    if (useRgb565) {
        const int pixelCount = img.w * img.h;
        tex.backing.resize(pixelCount * 2);
        unsigned short* out = reinterpret_cast<unsigned short*>(&tex.backing[0]);
        const unsigned char* src = img.data;

        for (int i = 0; i < pixelCount; ++i) {
            unsigned short r = src[i * 4 + 0] >> 3;
            unsigned short g = src[i * 4 + 1] >> 2;
            unsigned short b = src[i * 4 + 2] >> 3;
            out[i] = (r << 11) | (g << 5) | b;
        }

        GX_InitTexObj(&tex.texObj, &tex.backing[0],
                      img.w, img.h,
                      GX_TF_RGB565,
                      GX_REPEAT, GX_REPEAT,
                      GX_FALSE);
    } else {
        tex.backing.assign(img.data, img.data + img.numBytes);
        GX_InitTexObj(&tex.texObj, &tex.backing[0],
                      img.w, img.h,
                      GX_TF_RGBA8,
                      GX_REPEAT, GX_REPEAT,
                      GX_FALSE);
    }

    GX_InitTexObjLOD(&tex.texObj,
                     GX_NEAR, GX_NEAR,
                     0.0f, 0.0f, 0.0f,
                     GX_FALSE, GX_FALSE,
                     GX_ANISO_1);

    tex.initialized = true;
}

} // anonymous

/* Texture API */

TextureId genTexture() {
    TextureId id = s_nextTextureId++;
    s_textures[id] = WiiTexture();
    return id;
}

void deleteTexture(TextureId id) {
    s_textures.erase(id);
}

void bindTexture(TextureId id) {
    auto it = s_textures.find(id);
    if (it == s_textures.end() || !it->second.initialized)
        return;

    GX_LoadTexObj(&it->second.texObj, GX_TEXMAP0);
}

TextureId createTextureFromData(const TextureData& img) {
    TextureId id = genTexture();
    uploadTexture2D(img);
    return id;
}

void uploadTexture2D(const TextureData& img) {
    TextureId id = s_nextTextureId - 1; // last generated
    auto it = s_textures.find(id);
    if (it == s_textures.end())
        return;

    initTexObj(it->second, img);

    if (it->second.initialized)
        GX_LoadTexObj(&it->second.texObj, GX_TEXMAP0);
}

void configureTextureSampling(bool useMipMap, bool blur, bool clamp) {
    auto it = s_textures.find(s_nextTextureId - 1);
    if (it == s_textures.end() || !it->second.initialized)
        return;

    u8 minFilter = blur ? GX_LINEAR : (useMipMap ? GX_LIN_MIP_LIN : GX_NEAR);
    u8 magFilter = blur ? GX_LINEAR : GX_NEAR;
    u8 wrapMode = clamp ? GX_CLAMP : GX_REPEAT;

    GX_InitTexObjFilterMode(&it->second.texObj, minFilter, magFilter);
    GX_InitTexObjWrapMode(&it->second.texObj, wrapMode, wrapMode);
}

/* Render State */

void setBlendState(bool enabled) {
    if (enabled) {
        GX_SetBlendMode(GX_BM_BLEND,
                        GX_BL_SRCALPHA,
                        GX_BL_INVSRCALPHA,
                        GX_LO_CLEAR);
    } else {
        GX_SetBlendMode(GX_BM_NONE,
                        GX_BL_ONE,
                        GX_BL_ZERO,
                        GX_LO_CLEAR);
    }
}

void setDepthState(bool enabled, bool writeMask) {
    GX_SetZMode(enabled ? GX_TRUE : GX_FALSE,
                GX_LEQUAL,
                writeMask ? GX_TRUE : GX_FALSE);
}

void setCullState(bool enabled) {
    GX_SetCullMode(enabled ? GX_CULL_BACK : GX_CULL_NONE);
}

void setAlphaTestState(bool enabled) {
    if (enabled)
        GX_SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
    else
        GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

void setScissorState(bool enabled, int x, int y, int w, int h) {
    if (enabled)
        GX_SetScissor(x, y, w, h);
    else
        GX_SetScissor(0, 0, 640, 480);
}

/* Stubbed (implement later properly) */

void init() {}
void setPerspective(float, float, float, float) {}
void setOrtho(float, float, float, float, float, float) {}
void updateTexture2D(int, int, int, int, const void*, int, int) {}
void submitTexturedMesh(int, int, unsigned int) {}
void bindArrayBuffer(unsigned int) {}
void uploadArrayBuffer(const void*, int, bool) {}
void shaderCompatUseProgram(unsigned int) {}
void shaderCompatSetUniformMat4(int, const float*) {}

} // namespace RenderBackend

#endif