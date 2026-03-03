#include <gccore.h>
#include <ogc/lwp_watchdog.h>
#include <stdio.h>
#include <string.h>

#include "client/renderer/RenderBackend.h"
#include "client/renderer/TextureData.h"

namespace {
static void* g_fifo = NULL;
static GXRModeObj* g_rmode = NULL;
static void* g_frameBuffers[2] = {NULL, NULL};
static int g_fbIndex = 0;

void initVideo() {
    VIDEO_Init();
    g_rmode = VIDEO_GetPreferredMode(NULL);

    g_frameBuffers[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(g_rmode));
    g_frameBuffers[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(g_rmode));

    VIDEO_Configure(g_rmode);
    VIDEO_SetNextFramebuffer(g_frameBuffers[g_fbIndex]);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (g_rmode->viTVMode & VI_NON_INTERLACE) {
        VIDEO_WaitVSync();
    }

    g_fifo = memalign(32, DEFAULT_FIFO_SIZE);
    memset(g_fifo, 0, DEFAULT_FIFO_SIZE);

    GX_Init(g_fifo, DEFAULT_FIFO_SIZE);
    GX_SetCopyClear((GXColor){0x20, 0x20, 0x40, 0xFF}, 0x00FFFFFF);
    GX_SetViewport(0.0f, 0.0f, (f32)g_rmode->fbWidth, (f32)g_rmode->efbHeight, 0.0f, 1.0f);
    GX_SetScissor(0, 0, g_rmode->fbWidth, g_rmode->efbHeight);
    GX_SetDispCopyYScale((f32)g_rmode->xfbHeight / (f32)g_rmode->efbHeight);
    GX_SetDispCopySrc(0, 0, g_rmode->fbWidth, g_rmode->efbHeight);
    GX_SetDispCopyDst(g_rmode->fbWidth, g_rmode->xfbHeight);
    GX_SetCopyFilter(g_rmode->aa, g_rmode->sample_pattern, GX_TRUE, g_rmode->vfilter);
    GX_SetFieldMode(g_rmode->field_rendering, ((g_rmode->viHeight == 2 * g_rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));

    GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);
    GX_SetDispCopyGamma(GX_GM_1_0);

    Mtx view;
    guMtxIdentity(view);
    GX_LoadPosMtxImm(view, GX_PNMTX0);

    Mtx44 projection;
    guOrtho(projection, 1.0f, -1.0f, -1.3333f, 1.3333f, 0.0f, 1.0f);
    GX_LoadProjectionMtx(projection, GX_ORTHOGRAPHIC);

    GX_ClearVtxDesc();
    GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
    GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

    GX_SetNumTexGens(1);
    GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GX_SetNumChans(0);
    GX_SetNumTevStages(1);
    GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
    GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
    GX_SetCullMode(GX_CULL_NONE);
    GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
}

void presentFrame() {
    GX_DrawDone();
    GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GX_CopyDisp(g_frameBuffers[g_fbIndex], GX_TRUE);
    VIDEO_SetNextFramebuffer(g_frameBuffers[g_fbIndex]);
    VIDEO_Flush();
    VIDEO_WaitVSync();
    g_fbIndex ^= 1;
}

RenderBackend::TextureId createDebugTexture() {
    static unsigned char pixels[] = {
        255, 0, 0, 255,    0, 255, 0, 255,
        0, 0, 255, 255,    255, 255, 0, 255,
    };

    TextureData tex;
    tex.data = pixels;
    tex.w = 2;
    tex.h = 2;
    tex.numBytes = sizeof(pixels);
    tex.format = TEXF_UNCOMPRESSED_8888;
    tex.transparent = false;

    const RenderBackend::TextureId id = RenderBackend::createTextureFromData(tex);
    RenderBackend::configureTextureSampling(false, false, false);
    return id;
}

void drawTexturedQuad(RenderBackend::TextureId texId) {
    RenderBackend::bindTexture(texId);

    GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
    GX_Position3f32(-0.75f, -0.75f, 0.0f);
    GX_TexCoord2f32(0.0f, 1.0f);

    GX_Position3f32(0.75f, -0.75f, 0.0f);
    GX_TexCoord2f32(1.0f, 1.0f);

    GX_Position3f32(0.75f, 0.75f, 0.0f);
    GX_TexCoord2f32(1.0f, 0.0f);

    GX_Position3f32(-0.75f, 0.75f, 0.0f);
    GX_TexCoord2f32(0.0f, 0.0f);
}
} // namespace

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    initVideo();
    CON_Init(g_frameBuffers[0], 20, 20, g_rmode->fbWidth, g_rmode->xfbHeight, g_rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    printf("Wii bootstrap: GX initialized.\\n");

    RenderBackend::init();
    const RenderBackend::TextureId texId = createDebugTexture();

    while (true) {
        GX_InvVtxCache();
        GX_InvalidateTexAll();

        GX_SetCopyClear((GXColor){0x10, 0x10, 0x30, 0xFF}, 0x00FFFFFF);
        drawTexturedQuad(texId);
        presentFrame();
    }

    return 0;
}
