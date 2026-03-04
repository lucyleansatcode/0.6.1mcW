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

/* Matrix Helpers */

// Model matrix helpers operate on the backend-managed model stack and always
// affect subsequent draw calls immediately.
// - push*/pop* are balanced stack operations; pop on an empty stack is a no-op.
// - load*Identity resets the current matrix to identity.
// - translate/scale/rotate post-multiply the current model matrix.
// - Wii/GX and GL backends use the same logical ordering so callsites can share
//   transform code without platform branches.

void pushModelMatrix();
void popModelMatrix();
void loadModelIdentity();
void translateModel(float x, float y, float z);
void scaleModel(float x, float y, float z);
void rotateModel(float angleDeg, float x, float y, float z);

void pushProjectionMatrix();
void popProjectionMatrix();
void loadProjectionIdentity();

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
void setDepthWriteMask(bool writeMask);
void setDepthTestState(bool enabled);
void setCullState(bool enabled);
// Enables/disables color blending. When enabled, srcFactor/dstFactor are GL-era
// blend constants.
// Backends should map known factors to native APIs. Unsupported pairs must use
// a deterministic fallback (standard alpha blend: SRC_ALPHA, ONE_MINUS_SRC_ALPHA)
// and surface a debug warning/assert so regressions are visible.
void setBlendState(bool enabled, unsigned int srcFactor, unsigned int dstFactor);
void setBlendEnabled(bool enabled);
void setBlendFunc(unsigned int srcFactor, unsigned int dstFactor);
// Alpha test compatibility switch.
// - enabled=true: reject fully transparent fragments (alpha > 0 style cutout).
// - enabled=false: disable alpha rejection (all fragments pass).
// Backends that cannot represent the exact legacy GL state should implement the
// deterministic behavior above.
void setAlphaTestState(bool enabled);
void setTextureState(bool enabled);
void setFogState(bool enabled);
void setColor(float r, float g, float b, float a);
void setShadeModel(unsigned int mode);
// Enables/disables rectangular clipping for subsequent draws.
// Rectangle coordinates use the backend's window-space convention expected by
// current callsites. Disabling must restore a full-frame scissor region.
// Invalid dimensions are clamped to a disabled/full-frame outcome.
void setScissorState(bool enabled, int x, int y, int width, int height);

/* Geometry Submission */

void submitTexturedMesh(int vertexCount, int vertexStride, unsigned int mode = RB_TRIANGLES);

/* Buffers */

void bindArrayBuffer(unsigned int bufferId);
void uploadArrayBuffer(const void* data, int bytes, bool dynamic);
void genBuffers(int count, unsigned int* outIds);
void deleteBuffers(int count, const unsigned int* ids);

/* Shader Compatibility Layer (No-op on GX, kept for engine compatibility) */

void shaderCompatUseProgram(unsigned int program);
void shaderCompatSetUniformMat4(int location, const float* matrix4x4);

}

#endif
