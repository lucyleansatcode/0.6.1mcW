#include "RenderBackend.h"

#if defined(__WII__)

#include <gccore.h>
#include <malloc.h>
#include <unordered_map>
#include <vector>

namespace RenderBackend {
namespace WiiRenderer {

namespace {
struct WiiTexture {
    GXTexObj texObj;
    std::vector<unsigned char> backing;
    bool initialized;

    WiiTexture() : initialized(false) {}
};

std::unordered_map<TextureId, WiiTexture> s_textures;

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
            const unsigned short r = static_cast<unsigned short>(src[i * 4 + 0] >> 3);
            const unsigned short g = static_cast<unsigned short>(src[i * 4 + 1] >> 2);
            const unsigned short b = static_cast<unsigned short>(src[i * 4 + 2] >> 3);
            out[i] = static_cast<unsigned short>((r << 11) | (g << 5) | b);
        }

        GX_InitTexObj(&tex.texObj, &tex.backing[0], img.w, img.h, GX_TF_RGB565, GX_REPEAT, GX_REPEAT, GX_FALSE);
    } else {
        tex.backing.assign(img.data, img.data + img.numBytes);
        GX_InitTexObj(&tex.texObj, &tex.backing[0], img.w, img.h, GX_TF_RGBA8, GX_REPEAT, GX_REPEAT, GX_FALSE);
    }

    GX_InitTexObjLOD(&tex.texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);
    tex.initialized = true;
}
}

void registerTexture(TextureId id) {
    s_textures[id] = WiiTexture();
}

void deleteTexture(TextureId id) {
    s_textures.erase(id);
}

void bindTexture(TextureId id) {
    std::unordered_map<TextureId, WiiTexture>::iterator it = s_textures.find(id);
    if (it == s_textures.end() || !it->second.initialized) {
        return;
    }
    GX_LoadTexObj(&it->second.texObj, GX_TEXMAP0);
}

void configureTextureSampling(TextureId id, bool useMipMap, bool blur, bool clamp) {
    std::unordered_map<TextureId, WiiTexture>::iterator it = s_textures.find(id);
    if (it == s_textures.end() || !it->second.initialized) {
        return;
    }

    const u8 minFilter = blur ? GX_LINEAR : (useMipMap ? GX_LIN_MIP_LIN : GX_NEAR);
    const u8 magFilter = blur ? GX_LINEAR : GX_NEAR;
    const u8 wrapMode = clamp ? GX_CLAMP : GX_REPEAT;

    GX_InitTexObjFilterMode(&it->second.texObj, minFilter, magFilter);
    GX_InitTexObjWrapMode(&it->second.texObj, wrapMode, wrapMode);
}

void uploadTexture2D(TextureId id, const TextureData& img) {
    std::unordered_map<TextureId, WiiTexture>::iterator it = s_textures.find(id);
    if (it == s_textures.end()) {
        return;
    }

    initTexObj(it->second, img);
    if (it->second.initialized) {
        GX_LoadTexObj(&it->second.texObj, GX_TEXMAP0);
    }
}

} // namespace WiiRenderer
} // namespace RenderBackend

#endif
