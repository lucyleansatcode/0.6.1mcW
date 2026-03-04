#ifndef NET_MINECRAFT_CLIENT_GUI__GUIRENDERCONTEXT_H__
#define NET_MINECRAFT_CLIENT_GUI__GUIRENDERCONTEXT_H__

namespace GuiRenderContext {

typedef unsigned int RenderId;
typedef unsigned int RenderState;

static const RenderState BlendSrcAlpha = 0x0302;
static const RenderState BlendOneMinusSrcAlpha = 0x0303;
static const RenderState ShadeFlat = 0x1D00;
static const RenderState ShadeSmooth = 0x1D01;

void setBlendState(bool enabled, RenderState srcFactor, RenderState dstFactor);
void setAlphaTestState(bool enabled);
void setDepthState(bool enabled, bool writeMask);
void setScissorState(bool enabled, int x, int y, int width, int height);

void setTexture2DState(bool enabled);
void setFogState(bool enabled);
void setColor(float r, float g, float b, float a);
void setShadeModel(RenderState mode);

void translate(float x, float y, float z);
void scale(float x, float y, float z);
void pushMatrix();
void popMatrix();

void genBuffers(int count, RenderId* outIds);
void deleteBuffers(int count, const RenderId* ids);

void restoreGuiDefaults();

}

#endif
