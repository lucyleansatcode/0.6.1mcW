#ifndef NET_MINECRAFT_CLIENT_RENDERER_RENDERBACKEND_H
#define NET_MINECRAFT_CLIENT_RENDERER_RENDERBACKEND_H

#include "gles.h"
#include "TextureData.h"

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

void submitTexturedMesh(int vertexCount, int vertexStride, unsigned int mode = GL_TRIANGLES);

void bindArrayBuffer(unsigned int bufferId);
void uploadArrayBuffer(const void* data, int bytes, bool dynamic);

void shaderCompatUseProgram(unsigned int program);
void shaderCompatSetUniformMat4(int location, const float* matrix4x4);

}

#endif
