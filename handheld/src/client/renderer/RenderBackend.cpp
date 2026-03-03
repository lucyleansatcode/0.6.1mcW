#include "RenderBackend.h"

#include <unordered_map>

#if !defined(WII) && !defined(__WII__)
#include "renderer_gx.h"
#endif

#if defined(__WII__)
#include <gccore.h>
#include <vector>
#endif

namespace RenderBackend {

#if defined(__WII__)
namespace {
TextureId s_nextTextureId = 1;
TextureId s_boundTexture = 0;
}

namespace WiiRenderer {
void registerTexture(TextureId id);
void deleteTexture(TextureId id);
void bindTexture(TextureId id);
void configureTextureSampling(TextureId id, bool useMipMap, bool blur, bool clamp);
void uploadTexture2D(TextureId id, const TextureData& img);
}
#else
namespace {
std::unordered_map<TextureId, unsigned int> s_glTextures;
TextureId s_nextTextureId = 1;
}
#endif

void init() {
#if defined(__WII__)
    // GX init is expected to happen in the platform layer before rendering.
    GX_SetCullMode(GX_CULL_BACK);
    GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
#else
    glInit();
#endif
}

void setPerspective(float fovYDeg, float aspect, float zNear, float zFar) {
#if defined(__WII__)
    Mtx44 p;
    guPerspective(p, fovYDeg, aspect, zNear, zFar);
    GX_LoadProjectionMtx(p, GX_PERSPECTIVE);
#else
    gluPerspective(fovYDeg, aspect, zNear, zFar);
#endif
}

void setOrtho(float left, float right, float bottom, float top, float zNear, float zFar) {
#if defined(__WII__)
    Mtx44 p;
    guOrtho(p, top, bottom, left, right, zNear, zFar);
    GX_LoadProjectionMtx(p, GX_ORTHOGRAPHIC);
#else
    glOrthof(left, right, bottom, top, zNear, zFar);
#endif
}

void configureTextureSampling(bool useMipMap, bool blur, bool clamp) {
#if defined(__WII__)
    WiiRenderer::configureTextureSampling(s_boundTexture, useMipMap, blur, clamp);
#else
    glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, blur ? GL_LINEAR : (useMipMap ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST));
    glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, blur ? GL_LINEAR : GL_NEAREST);
    glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
    glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
#endif
}

TextureId genTexture() {
#if defined(__WII__)
    const TextureId id = s_nextTextureId++;
    WiiRenderer::registerTexture(id);
    return id;
#else
    unsigned int glId = 0;
    glGenTextures(1, &glId);
    const TextureId id = s_nextTextureId++;
    s_glTextures[id] = glId;
    return id;
#endif
}

void deleteTexture(TextureId id) {
#if defined(__WII__)
    WiiRenderer::deleteTexture(id);
    if (s_boundTexture == id) {
        s_boundTexture = 0;
    }
#else
    std::unordered_map<TextureId, unsigned int>::iterator it = s_glTextures.find(id);
    if (it == s_glTextures.end()) {
        return;
    }

    unsigned int glId = it->second;
    glDeleteTextures(1, &glId);
    s_glTextures.erase(it);
#endif
}

void bindTexture(TextureId id) {
#if defined(__WII__)
    s_boundTexture = id;
    WiiRenderer::bindTexture(id);
#else
    std::unordered_map<TextureId, unsigned int>::iterator it = s_glTextures.find(id);
    const unsigned int glId = it == s_glTextures.end() ? 0 : it->second;
    glBindTexture2(GL_TEXTURE_2D, glId);
#endif
}

TextureId createTextureFromData(const TextureData& img) {
    const TextureId id = genTexture();
    bindTexture(id);
    uploadTexture2D(img);
    return id;
}

void uploadTexture2D(const TextureData& img) {
#if defined(__WII__)
    WiiRenderer::uploadTexture2D(s_boundTexture, img);
#else
    const GLint mode = img.transparent ? GL_RGBA : GL_RGB;
    if (img.format == TEXF_UNCOMPRESSED_565)
        glTexImage2D2(GL_TEXTURE_2D, 0, mode, img.w, img.h, 0, mode, GL_UNSIGNED_SHORT_5_6_5, img.data);
    else if (img.format == TEXF_UNCOMPRESSED_4444)
        glTexImage2D2(GL_TEXTURE_2D, 0, mode, img.w, img.h, 0, mode, GL_UNSIGNED_SHORT_4_4_4_4, img.data);
    else if (img.format == TEXF_UNCOMPRESSED_5551)
        glTexImage2D2(GL_TEXTURE_2D, 0, mode, img.w, img.h, 0, mode, GL_UNSIGNED_SHORT_5_5_5_1, img.data);
    else
        glTexImage2D2(GL_TEXTURE_2D, 0, mode, img.w, img.h, 0, mode, GL_UNSIGNED_BYTE, img.data);
#endif
}

void updateTexture2D(int x, int y, int w, int h, const void* data, int format, int type) {
#if defined(__WII__)
    (void)x; (void)y; (void)w; (void)h; (void)data; (void)format; (void)type;
    // The GX upload path currently uses full texture object updates.
#else
    glTexSubImage2D2(GL_TEXTURE_2D, 0, x, y, w, h, format, type, data);
#endif
}

void setDepthState(bool enabled, bool writeMask) {
#if defined(__WII__)
    GX_SetZMode(enabled ? GX_TRUE : GX_FALSE, GX_LEQUAL, writeMask ? GX_TRUE : GX_FALSE);
#else
    if (enabled) glEnable2(GL_DEPTH_TEST); else glDisable2(GL_DEPTH_TEST);
    glDepthMask(writeMask ? GL_TRUE : GL_FALSE);
#endif
}

void setCullState(bool enabled) {
#if defined(__WII__)
    GX_SetCullMode(enabled ? GX_CULL_BACK : GX_CULL_NONE);
#else
    if (enabled) glEnable2(GL_CULL_FACE); else glDisable2(GL_CULL_FACE);
#endif
}

void setBlendState(bool enabled, unsigned int srcFactor, unsigned int dstFactor) {
#if defined(__WII__)
    if (enabled) GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    else GX_SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    (void)srcFactor;
    (void)dstFactor;
#else
    if (enabled) {
        glEnable2(GL_BLEND);
        glBlendFunc2(srcFactor, dstFactor);
    } else {
        glDisable2(GL_BLEND);
    }
#endif
}

void submitTexturedMesh(int vertexCount, int vertexStride, unsigned int mode) {
#if defined(__WII__)
    (void)vertexCount;
    (void)vertexStride;
    (void)mode;
    // GX submission is handled by the fixed-function TEV+FIFO code path.
#else
    glTexCoordPointer2(2, GL_FLOAT, vertexStride, (GLvoid*)(3 * 4));
    glEnableClientState2(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer2(3, GL_FLOAT, vertexStride, 0);
    glEnableClientState2(GL_VERTEX_ARRAY);
    glDrawArrays2(mode, 0, vertexCount);
    glDisableClientState2(GL_VERTEX_ARRAY);
    glDisableClientState2(GL_TEXTURE_COORD_ARRAY);
#endif
}

void bindArrayBuffer(unsigned int bufferId) {
#if defined(__WII__)
    (void)bufferId;
#else
    glBindBuffer2(GL_ARRAY_BUFFER, bufferId);
#endif
}

void uploadArrayBuffer(const void* data, int bytes, bool dynamic) {
#if defined(__WII__)
    (void)data;
    (void)bytes;
    (void)dynamic;
#else
    glBufferData2(GL_ARRAY_BUFFER, bytes, data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
#endif
}

void shaderCompatUseProgram(unsigned int program) {
#if defined(__WII__)
    (void)program;
#else
    (void)program;
#endif
}

void shaderCompatSetUniformMat4(int location, const float* matrix4x4) {
    (void)location;
    (void)matrix4x4;
}

}
