#include "RenderBackend.h"

#include <cassert>
#include <unordered_map>

#include "platform/log.h"

#if !defined(WII) && !defined(__WII__)
#include "render_compat.h"
#endif

namespace RenderBackend {

#if defined(__WII__)
namespace {
TextureId s_boundTexture = 0;
TextureId s_nextTextureId = 1;
bool s_blendEnabled = false;
unsigned int s_blendSrcFactor = 0x0302; // GL_SRC_ALPHA
unsigned int s_blendDstFactor = 0x0303; // GL_ONE_MINUS_SRC_ALPHA
}

namespace WiiRenderer {
void init();
void setPerspective(float fovYDeg, float aspect, float zNear, float zFar);
void setOrtho(float left, float right, float bottom, float top, float zNear, float zFar);
void pushModelMatrix();
void popModelMatrix();
void loadModelIdentity();
void translateModel(float x, float y, float z);
void scaleModel(float x, float y, float z);
void rotateModel(float angleDeg, float x, float y, float z);
void pushProjectionMatrix();
void popProjectionMatrix();
void loadProjectionIdentity();
void registerTexture(TextureId id);
void deleteTexture(TextureId id);
void bindTexture(TextureId id);
void configureTextureSampling(TextureId id, bool useMipMap, bool blur, bool clamp);
void uploadTexture2D(TextureId id, const TextureData& img);
void setDepthState(bool enabled, bool writeMask);
void setCullState(bool enabled);
void setBlendState(bool enabled, unsigned int srcFactor, unsigned int dstFactor);
void setAlphaTestState(bool enabled);
void setScissorState(bool enabled, int x, int y, int width, int height);
}
#else
namespace {
std::unordered_map<TextureId, unsigned int> s_glTextures;
TextureId s_nextTextureId = 1;
}
#endif

void init() {
#if defined(__WII__)
    WiiRenderer::init();
#endif
}

void setPerspective(float fovYDeg, float aspect, float zNear, float zFar) {
#if defined(__WII__)
    WiiRenderer::setPerspective(fovYDeg, aspect, zNear, zFar);
#else
    gluPerspective(fovYDeg, aspect, zNear, zFar);
#endif
}

void setOrtho(float left, float right, float bottom, float top, float zNear, float zFar) {
#if defined(__WII__)
    WiiRenderer::setOrtho(left, right, bottom, top, zNear, zFar);
#else
    glOrthof(left, right, bottom, top, zNear, zFar);
#endif
}

void pushModelMatrix() {
#if defined(__WII__)
    WiiRenderer::pushModelMatrix();
#else
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
#endif
}

void popModelMatrix() {
#if defined(__WII__)
    WiiRenderer::popModelMatrix();
#else
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
#endif
}

void loadModelIdentity() {
#if defined(__WII__)
    WiiRenderer::loadModelIdentity();
#else
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#endif
}

void translateModel(float x, float y, float z) {
#if defined(__WII__)
    WiiRenderer::translateModel(x, y, z);
#else
    glMatrixMode(GL_MODELVIEW);
    glTranslatef(x, y, z);
#endif
}

void scaleModel(float x, float y, float z) {
#if defined(__WII__)
    WiiRenderer::scaleModel(x, y, z);
#else
    glMatrixMode(GL_MODELVIEW);
    glScalef(x, y, z);
#endif
}

void rotateModel(float angleDeg, float x, float y, float z) {
#if defined(__WII__)
    WiiRenderer::rotateModel(angleDeg, x, y, z);
#else
    glMatrixMode(GL_MODELVIEW);
    glRotatef(angleDeg, x, y, z);
#endif
}

void pushProjectionMatrix() {
#if defined(__WII__)
    WiiRenderer::pushProjectionMatrix();
#else
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
#endif
}

void popProjectionMatrix() {
#if defined(__WII__)
    WiiRenderer::popProjectionMatrix();
#else
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
#endif
}

void loadProjectionIdentity() {
#if defined(__WII__)
    WiiRenderer::loadProjectionIdentity();
#else
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
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
    WiiRenderer::setDepthState(enabled, writeMask);
#else
    if (enabled) glEnable2(GL_DEPTH_TEST); else glDisable2(GL_DEPTH_TEST);
    glDepthMask(writeMask ? GL_TRUE : GL_FALSE);
#endif
}

void setDepthWriteMask(bool writeMask) {
#if defined(__WII__)
    LOGW("RenderBackend(Wii): setDepthWriteMask(%d) is not independently supported; use setDepthState for deterministic behavior\n",
         writeMask ? 1 : 0);
#if !defined(NDEBUG)
    assert(false && "Unsupported independent depth write mask request on Wii backend");
#endif
#else
    glDepthMask(writeMask ? GL_TRUE : GL_FALSE);
#endif
}

void setDepthTestState(bool enabled) {
#if defined(__WII__)
    LOGW("RenderBackend(Wii): setDepthTestState(%d) is not independently supported; use setDepthState for deterministic behavior\n",
         enabled ? 1 : 0);
#if !defined(NDEBUG)
    assert(false && "Unsupported independent depth test request on Wii backend");
#endif
#else
    if (enabled) glEnable2(GL_DEPTH_TEST); else glDisable2(GL_DEPTH_TEST);
#endif
}

void setCullState(bool enabled) {
#if defined(__WII__)
    WiiRenderer::setCullState(enabled);
#else
    if (enabled) glEnable2(GL_CULL_FACE); else glDisable2(GL_CULL_FACE);
#endif
}

void setBlendState(bool enabled, unsigned int srcFactor, unsigned int dstFactor) {
#if defined(__WII__)
    s_blendEnabled = enabled;
    s_blendSrcFactor = srcFactor;
    s_blendDstFactor = dstFactor;
    WiiRenderer::setBlendState(enabled, srcFactor, dstFactor);
#else
    if (enabled) {
        glEnable2(GL_BLEND);
        glBlendFunc2(srcFactor, dstFactor);
    } else {
        glDisable2(GL_BLEND);
    }
#endif
}

void setBlendEnabled(bool enabled) {
#if defined(__WII__)
    s_blendEnabled = enabled;
    WiiRenderer::setBlendState(s_blendEnabled, s_blendSrcFactor, s_blendDstFactor);
#else
    if (enabled) glEnable2(GL_BLEND); else glDisable2(GL_BLEND);
#endif
}

void setBlendFunc(unsigned int srcFactor, unsigned int dstFactor) {
#if defined(__WII__)
    s_blendSrcFactor = srcFactor;
    s_blendDstFactor = dstFactor;
    WiiRenderer::setBlendState(s_blendEnabled, s_blendSrcFactor, s_blendDstFactor);
#else
    glBlendFunc2(srcFactor, dstFactor);
#endif
}


void setAlphaTestState(bool enabled) {
#if defined(__WII__)
    WiiRenderer::setAlphaTestState(enabled);
#else
    if (enabled) glEnable2(GL_ALPHA_TEST); else glDisable2(GL_ALPHA_TEST);
#endif
}

void setTextureState(bool enabled) {
#if defined(__WII__)
    (void)enabled;
#else
    if (enabled) glEnable2(GL_TEXTURE_2D); else glDisable2(GL_TEXTURE_2D);
#endif
}

void setFogState(bool enabled) {
#if defined(__WII__)
    (void)enabled;
#else
    if (enabled) glEnable2(GL_FOG); else glDisable2(GL_FOG);
#endif
}

void setColor(float r, float g, float b, float a) {
#if defined(__WII__)
    (void)r; (void)g; (void)b; (void)a;
#else
    glColor4f2(r, g, b, a);
#endif
}

void setShadeModel(unsigned int mode) {
#if defined(__WII__)
    (void)mode;
#else
    glShadeModel2(mode);
#endif
}

void setScissorState(bool enabled, int x, int y, int width, int height) {
#if defined(__WII__)
    WiiRenderer::setScissorState(enabled, x, y, width, height);
#else
    if (enabled) {
        glEnable2(GL_SCISSOR_TEST);
        glScissor(x, y, width, height);
    } else {
        glDisable2(GL_SCISSOR_TEST);
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

void genBuffers(int count, unsigned int* outIds) {
#if defined(__WII__)
    (void)count;
    (void)outIds;
#else
    glGenBuffers2(count, outIds);
#endif
}

void deleteBuffers(int count, const unsigned int* ids) {
#if defined(__WII__)
    (void)count;
    (void)ids;
#else
    glDeleteBuffers(count, ids);
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
