#include "GuiRenderContext.h"

#include "../renderer/RenderBackend.h"

namespace GuiRenderContext {

void setBlendState(bool enabled, RenderState srcFactor, RenderState dstFactor) {
    RenderBackend::setBlendState(enabled, srcFactor, dstFactor);
}

void setAlphaTestState(bool enabled) {
    RenderBackend::setAlphaTestState(enabled);
}

void setDepthState(bool enabled, bool writeMask) {
    RenderBackend::setDepthState(enabled, writeMask);
}

void setCullState(bool enabled) {
    RenderBackend::setCullState(enabled);
}

void setScissorState(bool enabled, int x, int y, int width, int height) {
    RenderBackend::setScissorState(enabled, x, y, width, height);
}

void setTexture2DState(bool enabled) {
    RenderBackend::setTextureState(enabled);
}

void setFogState(bool enabled) {
    RenderBackend::setFogState(enabled);
}

void setColor(float r, float g, float b, float a) {
    RenderBackend::setColor(r, g, b, a);
}

void setShadeModel(RenderState mode) {
    RenderBackend::setShadeModel(mode);
}

void translate(float x, float y, float z) {
    RenderBackend::translateModel(x, y, z);
}

void scale(float x, float y, float z) {
    RenderBackend::scaleModel(x, y, z);
}

void pushMatrix() {
    RenderBackend::pushModelMatrix();
}

void popMatrix() {
    RenderBackend::popModelMatrix();
}

void genBuffers(int count, RenderId* outIds) {
    RenderBackend::genBuffers(count, outIds);
}

void deleteBuffers(int count, const RenderId* ids) {
    RenderBackend::deleteBuffers(count, ids);
}

void restoreGuiDefaults() {
    setDepthState(true, true);
    setBlendState(false, GuiRenderContext::BlendSrcAlpha, GuiRenderContext::BlendOneMinusSrcAlpha);
    setAlphaTestState(true);
    setTexture2DState(true);
    setShadeModel(GuiRenderContext::ShadeFlat);
    setColor(1.0f, 1.0f, 1.0f, 1.0f);
    setScissorState(false, 0, 0, 0, 0);
}

}
