#include "LithtechUltralightGPU.h"
#include "iltclient.h"
#include "ilttexinterface.h"
#include "iltdrawprim.h"

extern ILTClient* g_pLTClient;
extern ILTTexInterface* g_pTexInterface;
extern ILTDrawPrim* g_pDrawPrim;

using GPUVertexQuad = ultralight::Vertex_2f_4ub_2f_2f_28f;
using IndexType = ultralight::IndexType;

LithtechUltralightGPU::LithtechUltralightGPU(IDirect3DDevice9* pDevice) {
    m_pDevice = pDevice;
}

/*===========================================================
    SYNCHRONIZE
===========================================================*/
void LithtechUltralightGPU::BeginSynchronize() {
    //
}

void LithtechUltralightGPU::EndSynchronize() {
    //
}

/*===========================================================
    TEXTURES
===========================================================*/
uint32_t LithtechUltralightGPU::NextTextureId() {
    static uint32_t next = 1;
    return next++;
}

HTEXTURE LithtechUltralightGPU::CreateRenderTargetTexture(uint32_t w, uint32_t h) {
    if (!g_pTexInterface || w == 0 || h == 0)
        return nullptr;

    // Cria textura vazia
    HTEXTURE tex = nullptr;

    // LithTech pede UM BUFFER ZERADO pra criar a textura
    std::vector<uint8_t> blank(w * h * 4, 0);

    LTRESULT r = g_pTexInterface->CreateTextureFromData(
        tex,
        TEXTURETYPE_ARGB8888,
        TEXTUREFLAG_32BITSYSCOPY,
        blank.data(),
        w,
        h,
        1
    );

    if (r != LT_OK || !tex) {
        OutputDebugStringA("[UL] Falha ao criar RTT LithTech.\n");
        return nullptr;
    }

    return tex;
}


void LithtechUltralightGPU::CreateTexture(uint32_t texture_id,
    ultralight::RefPtr<ultralight::Bitmap> bmp)
{
    uint32_t w = bmp->width();
    uint32_t h = bmp->height();
    if (!w || !h) return;

    if (!bmp || bmp->IsEmpty()) {
        // Isso é um RTT, não copie pixels.
        textures_[texture_id] = { CreateRenderTargetTexture(w, h), 0, 0 };
        return;
    }

    uint8_t* src = (uint8_t*)bmp->LockPixels();
    uint32_t pitch = bmp->row_bytes();

    if (src == nullptr) {
		return;
    }

    std::vector<uint8_t> final(w * h * 4);

    for (uint32_t y = 0; y < h; ++y) {
        memcpy(&final[y * w * 4], src + pitch * y, w * 4);
    }

    bmp->UnlockPixels();

    HTEXTURE hTex = nullptr;

    g_pTexInterface->CreateTextureFromData(
        hTex,
        TEXTURETYPE_ARGB8888,
        TEXTUREFLAG_32BITSYSCOPY,
        final.data(),
        w,
        h,
        1
    );

    textures_[texture_id] = { hTex, w, h };
}

void LithtechUltralightGPU::UpdateTexture(uint32_t tex, ultralight::RefPtr<ultralight::Bitmap> bmp) {
    DestroyTexture(tex);
    CreateTexture(tex, bmp);
}

void LithtechUltralightGPU::DestroyTexture(uint32_t texture_id) {
    auto it = textures_.find(texture_id);
    if (it == textures_.end()) return;

    g_pTexInterface->ReleaseTextureHandle(it->second.tex);
    textures_.erase(it);
}

/*===========================================================
    RENDER BUFFERS
===========================================================*/
uint32_t LithtechUltralightGPU::NextRenderBufferId() {
    static uint32_t next = 1;
    return next++;
}

void LithtechUltralightGPU::CreateRenderBuffer(uint32_t rb, const ultralight::RenderBuffer& desc) {
    renderbuffers_[rb] = { desc.width, desc.height };
}

void LithtechUltralightGPU::DestroyRenderBuffer(uint32_t rb) {
    renderbuffers_.erase(rb);
}

void LithtechUltralightGPU::BindRenderBuffer(uint32_t rb) {
    current_rb_ = rb;
}

/*===========================================================
    GEOMETRY (QUADS ONLY)
===========================================================*/
uint32_t LithtechUltralightGPU::NextGeometryId() {
    static uint32_t next = 1;
    return next++;
}

void LithtechUltralightGPU::CreateGeometry(uint32_t id,
    const ultralight::VertexBuffer& vtx,
    const ultralight::IndexBuffer& idx)
{
    LTGeometry geom;

    if (vtx.format != ultralight::VertexBufferFormat::_2f_4ub_2f_2f_28f) {
        geometries_[id] = geom;
        return;
    }

    uint32_t vstride = sizeof(GPUVertexQuad);
    uint32_t vcount = vtx.size / vstride;

    auto verts = (GPUVertexQuad*)vtx.data;
    auto indices = (uint32_t*)idx.data;

    uint32_t icount = idx.size / sizeof(uint32_t);

    for (uint32_t i = 0; i + 5 < icount; i += 6) {
        LTQuad q{};

        for (int j = 0; j < 4; j++) {
            uint32_t vi = indices[i + j];
            if (vi >= vcount) continue;

            const auto& v = verts[vi];

            q.poly.verts[j].x = v.pos[0];
            q.poly.verts[j].y = v.pos[1];
            q.poly.verts[j].z = 0;

            q.poly.verts[j].u = v.tex[0];
            q.poly.verts[j].v = v.tex[1];

            q.poly.verts[j].rgba =
                (v.color[0] << 0) |
                (v.color[1] << 8) |
                (v.color[2] << 16) |
                (v.color[3] << 24);
        }

        geom.quads.push_back(q);
    }

    geometries_[id] = std::move(geom);
}

void LithtechUltralightGPU::UpdateGeometry(uint32_t id,
    const ultralight::VertexBuffer& v,
    const ultralight::IndexBuffer& i)
{
    CreateGeometry(id, v, i);
}

void LithtechUltralightGPU::DestroyGeometry(uint32_t id) {
    geometries_.erase(id);
}

/*===========================================================
    EXECUTE DRAW GEOMETRY (core)
===========================================================*/
void LithtechUltralightGPU::ExecuteDrawGeometry(const ultralight::Command& cmd)
{
    auto gIt = geometries_.find(cmd.geometry_id);
    if (gIt == geometries_.end())
        return;

    auto tIt = textures_.find(cmd.gpu_state.texture_1_id);
    if (tIt == textures_.end())
        return;

    auto& geom = gIt->second;
    auto& tex = tIt->second;

    const auto& m = cmd.gpu_state.transform;

    g_pDrawPrim->SetTexture(tex.tex);

    for (auto& q : geom.quads) {
        LTPoly_GT4 p = q.poly;

        // Apply transform
        for (int i = 0; i < 4; i++) {
            float x = p.verts[i].x;
            float y = p.verts[i].y;

            p.verts[i].x = m.data[0] * x + m.data[4] * y + m.data[12];
            p.verts[i].y = m.data[1] * x + m.data[5] * y + m.data[13];
        }

        g_pDrawPrim->DrawPrim(&p);
    }
}

void LithtechUltralightGPU::Render()
{
    // Obter textura final do UL
    uint32_t texId = this->GetFinalCompositeTextureId();
    auto it = this->textures_.find(texId);
    if (it == this->textures_.end())
        return;

    const auto& tex = it->second;

    // Desenhar overlay 2D Ultralight na tela
    LTPoly_GT4 quad;

    g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
    g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ);
    g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
    g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
    g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
    g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
    g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

    g_pDrawPrim->SetTexture(tex.tex);

    float x = 100.0f;
    float y = 100.0f;
    float w = tex.w;
    float h = tex.h;

    g_pDrawPrim->SetXYWH(&quad, x, y, w, h);
    g_pDrawPrim->SetUVWH(&quad, 0.f, 0.f, 1.f, 1.f);
    g_pDrawPrim->SetRGBA(&quad, 0xFFFFFFFF);

    g_pDrawPrim->DrawPrim(&quad);
}


void LithtechUltralightGPU::UpdateCommandList(const ultralight::CommandList& cmd)
{
    for (uint32_t i = 0; i < cmd.size; i++) {
        const auto& command = cmd.commands[i];

        switch (command.command_type) {

        case ultralight::CommandType::DrawGeometry:
            ExecuteDrawGeometry(command);
            break;

        case ultralight::CommandType::ClearRenderBuffer:
            // LithTech já usa sua própria limpeza
            break;
        }
    }

	this->CacheCommandList(cmd);
}


void LithtechUltralightGPU::CacheCommandList(const ultralight::CommandList& list) {
    command_queue_.clear();

    for (uint32_t i = 0; i < list.size; ++i) {
        const auto& cmd = list.commands[i];

        CachedCommand c;
        c.type = cmd.command_type;
        c.geometry_id = cmd.geometry_id;
        c.state = cmd.gpu_state;
        c.indices_count = cmd.indices_count;
        c.indices_offset = cmd.indices_offset;

        if (cmd.command_type == ultralight::CommandType::DrawGeometry) {
            final_texture_id_ = cmd.gpu_state.texture_1_id;
        }

        command_queue_.push_back(c);
    }
}
