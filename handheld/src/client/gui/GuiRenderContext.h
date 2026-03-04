#ifndef NET_MINECRAFT_CLIENT_GUI__GUIRENDERCONTEXT_H__
#define NET_MINECRAFT_CLIENT_GUI__GUIRENDERCONTEXT_H__

namespace GuiRenderContext {

void setBlendState(bool enabled, unsigned int srcFactor, unsigned int dstFactor);
void setAlphaTestState(bool enabled);
void setDepthState(bool enabled, bool writeMask);
void setScissorState(bool enabled, int x, int y, int width, int height);

void setTexture2DState(bool enabled);
void setFogState(bool enabled);
void setColor(float r, float g, float b, float a);
void setShadeModel(unsigned int mode);

void translate(float x, float y, float z);
void scale(float x, float y, float z);
void pushMatrix();
void popMatrix();

void genBuffers(int count, unsigned int* outIds);
void deleteBuffers(int count, const unsigned int* ids);

void restoreGuiDefaults();

}

#endif
