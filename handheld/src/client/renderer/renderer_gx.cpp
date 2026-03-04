#include "RenderBackend.h"

#if defined(__WII__)

#include <gccore.h>

#include <cassert>
#include <cstring>
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

struct ArrayBuffer {
    std::vector<unsigned char> data;
    bool dynamic;
    ArrayBuffer() : dynamic(false) {}
};

struct ModelMtxState {
    Mtx m;
};

struct ProjMtxState {
    Mtx44 m;
    int type;
};

std::unordered_map<TextureId, WiiTexture> s_textures;
std::unordered_map<unsigned int, ArrayBuffer> s_arrayBuffers;
unsigned int s_nextBufferId = 1;
unsigned int s_boundArrayBuffer = 0;
Mtx s_modelMatrix;
Mtx44 s_projectionMatrix;
std::vector<ModelMtxState> s_modelStack;
std::vector<ProjMtxState> s_projectionStack;
int s_projectionType = GX_PERSPECTIVE;

const int kDefaultScissorWidth = 640;
const int kDefaultScissorHeight = 480;

const unsigned int kGlTriangles = 0x0004;
const unsigned int kGlTriangleFan = 0x0006;

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

u8 mapPrimitiveMode(unsigned int mode) {
    switch (mode) {
    case kGlTriangles:
        return GX_TRIANGLES;
    case kGlTriangleFan:
        return GX_TRIANGLEFAN;
    default:
        LOGW("RenderBackend(Wii): unsupported primitive mode 0x%X, using triangles\n", mode);
        debugAssertUnsupported(false, "Unsupported primitive mode requested");
        return GX_TRIANGLES;
    }
}

ArrayBuffer* findBoundArrayBuffer() {
    std::unordered_map<unsigned int, ArrayBuffer>::iterator it = s_arrayBuffers.find(s_boundArrayBuffer);
    if (it == s_arrayBuffers.end()) {
        return 0;
    }
    return &it->second;
}

void emitVertex(const unsigned char* raw, int vertexStride, bool hasColor) {
    const float* pos = reinterpret_cast<const float*>(raw);
    const float* uv = reinterpret_cast<const float*>(raw + 3 * sizeof(float));

    GX_Position3f32(pos[0], pos[1], pos[2]);
    GX_TexCoord2f32(uv[0], uv[1]);

    if (hasColor) {
        const unsigned int packed = *reinterpret_cast<const unsigned int*>(raw + 5 * sizeof(float));
        const u8 r = static_cast<u8>(packed & 0xFFu);
        const u8 g = static_cast<u8>((packed >> 8) & 0xFFu);
        const u8 b = static_cast<u8>((packed >> 16) & 0xFFu);
        const u8 a = static_cast<u8>((packed >> 24) & 0xFFu);
        GX_Color4u8(r, g, b, a);
    } else {
        GX_Color4u8(255, 255, 255, 255);
    }

    (void)vertexStride;
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
    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

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

void submitTexturedMesh(int vertexCount, int vertexStride, unsigned int mode) {
    if (vertexCount <= 0 || vertexStride < static_cast<int>(5 * sizeof(float))) {
        return;
    }

    ArrayBuffer* bound = findBoundArrayBuffer();
    if (!bound || bound->data.empty()) {
        LOGW("RenderBackend(Wii): submitTexturedMesh called without uploaded array buffer (id=%u)\n", s_boundArrayBuffer);
        debugAssertUnsupported(false, "submitTexturedMesh requires a bound uploaded array buffer");
        return;
    }

    const size_t requiredBytes = static_cast<size_t>(vertexCount) * static_cast<size_t>(vertexStride);
    if (requiredBytes > bound->data.size()) {
        LOGW("RenderBackend(Wii): vertex submission exceeds uploaded buffer (%u > %u bytes)\n",
             static_cast<unsigned int>(requiredBytes),
             static_cast<unsigned int>(bound->data.size()));
        debugAssertUnsupported(false, "Vertex submission exceeded uploaded array buffer size");
        return;
    }

    const bool hasColor = vertexStride >= static_cast<int>(6 * sizeof(float));
    const u8 gxMode = mapPrimitiveMode(mode);
    const unsigned char* cursor = &bound->data[0];

    GX_Begin(gxMode, GX_VTXFMT0, static_cast<u16>(vertexCount));
    for (int i = 0; i < vertexCount; ++i) {
        emitVertex(cursor, vertexStride, hasColor);
        cursor += vertexStride;
    }
    GX_End();
}

void bindArrayBuffer(unsigned int bufferId) {
    s_boundArrayBuffer = bufferId;
}

void uploadArrayBuffer(const void* data, int bytes, bool dynamic) {
    if (s_boundArrayBuffer == 0) {
        LOGW("RenderBackend(Wii): uploadArrayBuffer called with no bound buffer\n");
        debugAssertUnsupported(false, "uploadArrayBuffer requires a bound buffer");
        return;
    }

    ArrayBuffer& buffer = s_arrayBuffers[s_boundArrayBuffer];
    buffer.dynamic = dynamic;
    if (!data || bytes <= 0) {
        buffer.data.clear();
        return;
    }

    buffer.data.resize(static_cast<size_t>(bytes));
    std::memcpy(&buffer.data[0], data, static_cast<size_t>(bytes));
}

void genBuffers(int count, unsigned int* outIds) {
    if (!outIds || count <= 0) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        const unsigned int id = s_nextBufferId++;
        s_arrayBuffers[id] = ArrayBuffer();
        outIds[i] = id;
    }
}

void deleteBuffers(int count, const unsigned int* ids) {
    if (!ids || count <= 0) {
        return;
    }

    for (int i = 0; i < count; ++i) {
        const unsigned int id = ids[i];
        s_arrayBuffers.erase(id);
        if (s_boundArrayBuffer == id) {
            s_boundArrayBuffer = 0;
        }
    }
}

} // namespace WiiRenderer

#endif
