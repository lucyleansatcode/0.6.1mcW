#ifndef NET_MINECRAFT_CLIENT_RENDERER_RENDER_COMPAT_H
#define NET_MINECRAFT_CLIENT_RENDERER_RENDER_COMPAT_H

// Legacy GL compatibility helpers kept for non-Wii targets.
// Wii builds use the GX RenderBackend path and do not compile render_compat.cpp.
#if !defined(WII) && !defined(__WII__)

#include <GLES/gl.h>

void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);
void glInit();
void anGenBuffers(GLsizei n, GLuint* buffers);

void drawArrayVT(int bufferId, int vertices, int vertexSize = 24, unsigned int mode = GL_TRIANGLES);
void drawArrayVT_NoState(int bufferId, int vertices, int vertexSize = 24);
void drawArrayVTC(int bufferId, int vertices, int vertexSize = 24);
void drawArrayVTC_NoState(int bufferId, int vertices, int vertexSize = 24);

void MultiplyMatrices4by4OpenGL_FLOAT(float *result, float *matrix1, float *matrix2);
void MultiplyMatrixByVector4by4OpenGL_FLOAT(float *resultvector, const float *matrix, const float *pvector);
int glhInvertMatrixf2(float *m, float *out);
int glhUnProjectf(float winx, float winy, float winz, float *modelview, float *projection, int *viewport, float *objectCoordinate);

#endif

#endif
