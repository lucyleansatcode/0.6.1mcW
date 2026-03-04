#include "GuiRenderContext.h"

#include "../renderer/RenderBackend.h"
#include "../renderer/render_compat.h"

namespace GuiRenderContext {

void setBlendState(bool enabled, unsigned int srcFactor, unsigned int dstFactor) {
    RenderBackend::setBlendState(enabled, srcFactor, dstFactor);
}

void setAlphaTestState(bool enabled) {
    RenderBackend::setAlphaTestState(enabled);
}

void setDepthState(bool enabled, bool writeMask) {
    RenderBackend::setDepthState(enabled, writeMask);
}

void setScissorState(bool enabled, int x, int y, int width, int height) {
    RenderBackend::setScissorState(enabled, x, y, width, height);
}

void setTexture2DState(bool enabled) {
    if (enabled) glEnable2(GL_TEXTURE_2D); else glDisable2(GL_TEXTURE_2D);
}

void setFogState(bool enabled) {
    if (enabled) glEnable2(GL_FOG); else glDisable2(GL_FOG);
}

void setColor(float r, float g, float b, float a) {
    glColor4f2(r, g, b, a);
}

void setShadeModel(unsigned int mode) {
    glShadeModel2(mode);
}

void translate(float x, float y, float z) {
    glTranslatef2(x, y, z);
}

void scale(float x, float y, float z) {
    glScalef2(x, y, z);
}

void pushMatrix() {
    glPushMatrix2();
}

void popMatrix() {
    glPopMatrix2();
}

void genBuffers(int count, unsigned int* outIds) {
    glGenBuffers2(count, outIds);
}

void deleteBuffers(int count, const unsigned int* ids) {
    glDeleteBuffers(count, ids);
}

void restoreGuiDefaults() {
    setDepthState(true, true);
    setBlendState(false, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    setAlphaTestState(true);
    setTexture2DState(true);
    setShadeModel(GL_FLAT);
    setColor(1.0f, 1.0f, 1.0f, 1.0f);
    setScissorState(false, 0, 0, 0, 0);
}

}
