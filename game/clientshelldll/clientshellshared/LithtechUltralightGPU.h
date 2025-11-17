#pragma once
#include <Ultralight/platform/GPUDriver.h>
#include <Ultralight/Bitmap.h>
#include <unordered_map>
#include <d3d9.h>

struct CachedCommand {
    ultralight::CommandType type;
    uint32_t geometry_id;
    ultralight::GPUState state;
    uint32_t indices_count;
    uint32_t indices_offset;
};

class LithtechUltralightGPU : public ultralight::GPUDriver {
public:
    LithtechUltralightGPU(IDirect3DDevice9* pDevice);

    // ===== Synchronize =====
    void BeginSynchronize() override;
    void EndSynchronize() override;

    // ===== Texture =====
    uint32_t NextTextureId() override;
    void CreateTexture(uint32_t texture_id,
        ultralight::RefPtr<ultralight::Bitmap> bitmap) override;
    void UpdateTexture(uint32_t texture_id,
        ultralight::RefPtr<ultralight::Bitmap> bitmap) override;
    void DestroyTexture(uint32_t texture_id) override;

    // ===== Render Buffer =====
    uint32_t NextRenderBufferId() override;
    void CreateRenderBuffer(uint32_t render_buffer_id,
        const ultralight::RenderBuffer& buffer) override;
    void DestroyRenderBuffer(uint32_t render_buffer_id) override;
    void BindRenderBuffer(uint32_t render_buffer_id);

    // ===== Geometry =====
    uint32_t NextGeometryId() override;
    void CreateGeometry(uint32_t geometry_id,
        const ultralight::VertexBuffer& vertices,
        const ultralight::IndexBuffer& indices) override;
    void UpdateGeometry(uint32_t geometry_id,
        const ultralight::VertexBuffer& vertices,
        const ultralight::IndexBuffer& indices) override;
    void DestroyGeometry(uint32_t geometry_id) override;

    // ===== Command List =====
    void UpdateCommandList(const ultralight::CommandList& list) override;
    void CacheCommandList(const ultralight::CommandList& list);

    HTEXTURE CreateRenderTargetTexture(uint32_t w, uint32_t h);

    const std::vector<CachedCommand>& GetCommands() const { return command_queue_; }
    uint32_t GetFinalCompositeTextureId() const { return final_texture_id_; }

    // Our executor for DrawGeometry commands
    void ExecuteDrawGeometry(const ultralight::Command& command);
    void Render();

    struct LTQuad {
        LTPoly_GT4 poly;
    };

    struct LTGeometry {
        std::vector<LTQuad> quads;
    };

    struct LTTexture {
        HTEXTURE tex;
        uint32_t w, h;
    };

    struct LTRenderBuffer {
        uint32_t w, h;
    };

    std::unordered_map<uint32_t, LTTexture> textures_;
    std::unordered_map<uint32_t, LTGeometry> geometries_;
    std::unordered_map<uint32_t, LTRenderBuffer> renderbuffers_;
    std::vector<CachedCommand> command_queue_;

    uint32_t current_rb_ = 0;
    uint32_t final_texture_id_ = 0;

private:

    IDirect3DDevice9* m_pDevice;
};
