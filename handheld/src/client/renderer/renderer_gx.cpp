#include "RenderBackend.h"

#if defined(__WII__)

#include <gccore.h>

#include <cassert>
#include <unordered_map>
#include <vector>

#include "platform/log.h"

namespace WiiRenderer {

using RenderBackend::TextureData;
using RenderBackend::TextureId;

namespace {

struct WiiTexture {
    GXTexObj texObj;
    std::vector<unsigned char> backing;
    bool initialized;
    WiiTexture() : initialized(false) {}
};

struct ModelMtxState {
    Mtx m;
};

struct ProjMtxState {
    Mtx44 m;
    int type;
};

std::unordered_map<TextureId, WiiTexture> s_textures;
Mtx s_modelMatrix;
Mtx44 s_projectionMatrix;
std::vector<ModelMtxState> s_modelStack;
std::vector<ProjMtxState> s_projectionStack;
int s_projectionType = GX_PERSPECTIVE;

const int kDefaultScissorWidth = 640;
const int kDefaultScissorHeight = 480;

void debugAssertUnsupported(bool condition, const char* message) {
#if !defined(NDEBUG)
    if (!condition) {
        LOGW("%s\n", message);
        assert(condition && "Unsupported render state request on Wii backend");
    }
#else
    (void)condition;
    (void)message;
#endif
}

bool mapBlendFactor(unsigned int factor, u8* outFactor) {
    if (!outFactor) {
        return false;
    }

    switch (factor) {
    case 0x0000: // GL_ZERO
        *outFactor = GX_BL_ZERO;
        return true;
    case 0x0001: // GL_ONE
        *outFactor = GX_BL_ONE;
        return true;
    case 0x0302: // GL_SRC_ALPHA
        *outFactor = GX_BL_SRCALPHA;
        return true;
    case 0x0303: // GL_ONE_MINUS_SRC_ALPHA
        *outFactor = GX_BL_INVSRCALPHA;
        return true;
    default:
        break;
    }

    return false;
}

void applyModelMatrix() {
    GX_LoadPosMtxImm(s_modelMatrix, GX_PNMTX0);
}

void applyProjectionMatrix() {
    GX_LoadProjectionMtx(s_projectionMatrix, s_projectionType);
}

WiiTexture* findTexture(TextureId id) {
    std::unordered_map<TextureId, WiiTexture>::iterator it = s_textures.find(id);
    if (it == s_textures.end()) {
        return 0;
    }
    return &it->second;
}

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

        GX_InitTexObj(&tex.texObj, &tex.backing[0], img.w, img.h, GX_TF_RGB565, GX_REPEAT, GX_REPEAT, GX_FALSE);
    } else {
        tex.backing.assign(img.data, img.data + img.numBytes);
        GX_InitTexObj(&tex.texObj, &tex.backing[0], img.w, img.h, GX_TF_RGBA8, GX_REPEAT, GX_REPEAT, GX_FALSE);
    }

    GX_InitTexObjLOD(&tex.texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE, GX_ANISO_1);

    tex.initialized = true;
}

} // namespace

void init() {
    GX_SetCullMode(GX_CULL_BACK);
    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);

    guMtxIdentity(s_modelMatrix);
    guMtx44Identity(s_projectionMatrix);
    s_modelStack.clear();
    s_projectionStack.clear();
    s_projectionType = GX_PERSPECTIVE;
    applyModelMatrix();
    applyProjectionMatrix();
}

void setPerspective(float fovYDeg, float aspect, float zNear, float zFar) {
    Mtx44 p;
    guPerspective(p, fovYDeg, aspect, zNear, zFar);
    guMtx44Copy(p, s_projectionMatrix);
    s_projectionType = GX_PERSPECTIVE;
    applyProjectionMatrix();
}

void setOrtho(float left, float right, float bottom, float top, float zNear, float zFar) {
    Mtx44 p;
    guOrtho(p, top, bottom, left, right, zNear, zFar);
    guMtx44Copy(p, s_projectionMatrix);
    s_projectionType = GX_ORTHOGRAPHIC;
    applyProjectionMatrix();
}

void pushModelMatrix() {
    ModelMtxState state;
    guMtxCopy(s_modelMatrix, state.m);
    s_modelStack.push_back(state);
}

void popModelMatrix() {
    if (s_modelStack.empty()) {
        return;
    }
    guMtxCopy(s_modelStack.back().m, s_modelMatrix);
    s_modelStack.pop_back();
    applyModelMatrix();
}

void loadModelIdentity() {
    guMtxIdentity(s_modelMatrix);
    applyModelMatrix();
}

void translateModel(float x, float y, float z) {
    guMtxTransApply(s_modelMatrix, s_modelMatrix, x, y, z);
    applyModelMatrix();
}

void scaleModel(float x, float y, float z) {
    Mtx scale;
    guMtxScale(scale, x, y, z);
    guMtxConcat(s_modelMatrix, scale, s_modelMatrix);
    applyModelMatrix();
}

void rotateModel(float angleDeg, float x, float y, float z) {
    Mtx rotate;
    guVector axis = { x, y, z };
    guMtxRotAxisDeg(rotate, &axis, angleDeg);
    guMtxConcat(s_modelMatrix, rotate, s_modelMatrix);
    applyModelMatrix();
}

void pushProjectionMatrix() {
    ProjMtxState state;
    guMtx44Copy(s_projectionMatrix, state.m);
    state.type = s_projectionType;
    s_projectionStack.push_back(state);
}

void popProjectionMatrix() {
    if (s_projectionStack.empty()) {
        return;
    }
    s_projectionType = s_projectionStack.back().type;
    guMtx44Copy(s_projectionStack.back().m, s_projectionMatrix);
    s_projectionStack.pop_back();
    applyProjectionMatrix();
}

void loadProjectionIdentity() {
    guMtx44Identity(s_projectionMatrix);
    s_projectionType = GX_ORTHOGRAPHIC;
    applyProjectionMatrix();
}

void registerTexture(TextureId id) {
    s_textures[id] = WiiTexture();
}

void deleteTexture(TextureId id) {
    s_textures.erase(id);
}

void bindTexture(TextureId id) {
    WiiTexture* tex = findTexture(id);
    if (!tex || !tex->initialized) {
        return;
    }

    GX_LoadTexObj(&tex->texObj, GX_TEXMAP0);
}

void uploadTexture2D(TextureId id, const TextureData& img) {
    WiiTexture* tex = findTexture(id);
    if (!tex) {
        return;
    }

    initTexObj(*tex, img);

    if (tex->initialized) {
        GX_LoadTexObj(&tex->texObj, GX_TEXMAP0);
    }
}

void configureTextureSampling(TextureId id, bool useMipMap, bool blur, bool clamp) {
    WiiTexture* tex = findTexture(id);
    if (!tex || !tex->initialized) {
        return;
    }

    u8 minFilter = blur ? GX_LINEAR : (useMipMap ? GX_LIN_MIP_LIN : GX_NEAR);
    u8 magFilter = blur ? GX_LINEAR : GX_NEAR;
    u8 wrapMode = clamp ? GX_CLAMP : GX_REPEAT;

    GX_InitTexObjFilterMode(&tex->texObj, minFilter, magFilter);
    GX_InitTexObjWrapMode(&tex->texObj, wrapMode, wrapMode);
}

void setDepthState(bool enabled, bool writeMask) {
    GX_SetZMode(enabled ? GX_TRUE : GX_FALSE, GX_LEQUAL, writeMask ? GX_TRUE : GX_FALSE);
}

void setCullState(bool enabled) {
    GX_SetCullMode(enabled ? GX_CULL_BACK : GX_CULL_NONE);
}

void setBlendState(bool enabled, unsigned int srcFactor, unsigned int dstFactor) {
    if (enabled) {
        u8 gxSrc = GX_BL_SRCALPHA;
        u8 gxDst = GX_BL_INVSRCALPHA;
        const bool srcSupported = mapBlendFactor(srcFactor, &gxSrc);
        const bool dstSupported = mapBlendFactor(dstFactor, &gxDst);

        if (!srcSupported || !dstSupported) {
            LOGW("RenderBackend(Wii): unsupported blend factors src=0x%X dst=0x%X, using SRC_ALPHA/ONE_MINUS_SRC_ALPHA fallback\n",
                 srcFactor,
                 dstFactor);
            debugAssertUnsupported(false, "Unsupported blend factor requested");
            gxSrc = GX_BL_SRCALPHA;
            gxDst = GX_BL_INVSRCALPHA;
        }

        GX_SetBlendMode(GX_BM_BLEND, gxSrc, gxDst, GX_LO_NOOP);
    } else {
        GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    }
}

void setAlphaTestState(bool enabled) {
    if (enabled) {
        GX_SetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_ALWAYS, 0);
    } else {
        GX_SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    }
}

void setScissorState(bool enabled, int x, int y, int width, int height) {
    if (!enabled) {
        GX_SetScissor(0, 0, kDefaultScissorWidth, kDefaultScissorHeight);
        return;
    }

    if (width <= 0 || height <= 0) {
        LOGW("RenderBackend(Wii): invalid scissor rectangle (%d, %d, %d, %d), disabling scissor\n",
             x,
             y,
             width,
             height);
        debugAssertUnsupported(false, "Invalid scissor rectangle requested");
        GX_SetScissor(0, 0, kDefaultScissorWidth, kDefaultScissorHeight);
        return;
    }

    if (x < 0 || y < 0) {
        LOGW("RenderBackend(Wii): negative scissor origin (%d, %d), clamping to screen space\n", x, y);
        debugAssertUnsupported(false, "Negative scissor origin requested");
    } else {
        GX_SetScissor(x, y, width, height);
        return;
    }

    const int clampedX = x < 0 ? 0 : x;
    const int clampedY = y < 0 ? 0 : y;
    GX_SetScissor(clampedX, clampedY, width, height);
}

} // namespace WiiRenderer

#endif
