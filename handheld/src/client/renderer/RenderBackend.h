#ifndef NET_MINECRAFT_CLIENT_RENDERER_RENDERBACKEND_H
#define NET_MINECRAFT_CLIENT_RENDERER_RENDERBACKEND_H

#include "TextureData.h"

static const unsigned int RB_TRIANGLES = 0x0004;

namespace RenderBackend {

typedef unsigned int TextureId;

/* Core */

void init();

/* Projection */

void setPerspective(float fovYDeg, float aspect, float zNear, float zFar);
void setOrtho(float left, float right, float bottom, float top, float zNear, float zFar);

/* Texture */

void configureTextureSampling(bool useMipMap, bool blur, bool clamp);
TextureId genTexture();
void deleteTexture(TextureId id);
void bindTexture(TextureId id);
TextureId createTextureFromData(const TextureData& img);
void uploadTexture2D(const TextureData& img);
void updateTexture2D(int x, int y, int w, int h, const void* data, int format, int type);

/* Render State */

void setDepthState(bool enabled, bool writeMask);
void setCullState(bool enabled);
void setBlendState(bool enabled);              // Wii uses fixed blend factors internally
void setAlphaTestState(bool enabled);
void setScissorState(bool enabled, int x, int y, int width, int height);

/* Geometry Submission */

void submitTexturedMesh(int vertexCount, int vertexStride, unsigned int mode = RB_TRIANGLES);

/* Buffers */

void bindArrayBuffer(unsigned int bufferId);
void uploadArrayBuffer(const void* data, int bytes, bool dynamic);

/* Shader Compatibility Layer (No-op on GX, kept for engine compatibility) */

void shaderCompatUseProgram(unsigned int program);
void shaderCompatSetUniformMat4(int location, const float* matrix4x4);

}

#endif