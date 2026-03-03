#ifndef NET_MINECRAFT_CLIENT_RENDERER_RENDERBACKEND_H
#define NET_MINECRAFT_CLIENT_RENDERER_RENDERBACKEND_H

#include "TextureData.h"

#if !defined(WII) && !defined(__WII__)
#include "gles.h"
#else
static const unsigned int RB_TRIANGLES = 0x0004;
#endif

namespace RenderBackend {

void init();

void setPerspective(float fovYDeg, float aspect, float zNear, float zFar);
void setOrtho(float left, float right, float bottom, float top, float zNear, float zFar);

void configureTextureSampling(bool useMipMap, bool blur, bool clamp);
void uploadTexture2D(const TextureData& img);
void updateTexture2D(int x, int y, int w, int h, const void* data, int format, int type);

void setDepthState(bool enabled, bool writeMask);
void setCullState(bool enabled);
void setBlendState(bool enabled, unsigned int srcFactor, unsigned int dstFactor);

void submitTexturedMesh(int vertexCount, int vertexStride, unsigned int mode =
#if defined(WII) || defined(__WII__)
    RB_TRIANGLES
#else
    GL_TRIANGLES
#endif
);

void bindArrayBuffer(unsigned int bufferId);
void uploadArrayBuffer(const void* data, int bytes, bool dynamic);

void shaderCompatUseProgram(unsigned int program);
void shaderCompatSetUniformMat4(int location, const float* matrix4x4);

}

#endif
