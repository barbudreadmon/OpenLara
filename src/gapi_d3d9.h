#ifndef H_GAPI_D3D9
#define H_GAPI_D3D9

#include "core.h"
#include <d3d9.h>

#define PROFILE_MARKER(title)
#define PROFILE_LABEL(id, name, label)
#define PROFILE_TIMING(time)

extern LPDIRECT3D9           D3D;
extern LPDIRECT3DDEVICE9     device;
extern D3DPRESENT_PARAMETERS d3dpp;

#ifdef _DEBUG
void D3DCHECK(HRESULT res) {
    if (res == D3D_OK) return;

    LOG("! ");
    switch (res) {
        case D3DERR_WRONGTEXTUREFORMAT        : LOG("D3DERR_WRONGTEXTUREFORMAT");        break;
        case D3DERR_UNSUPPORTEDCOLOROPERATION : LOG("D3DERR_UNSUPPORTEDCOLOROPERATION"); break;
        case D3DERR_UNSUPPORTEDCOLORARG       : LOG("D3DERR_UNSUPPORTEDCOLORARG");       break;
        case D3DERR_UNSUPPORTEDALPHAOPERATION : LOG("D3DERR_UNSUPPORTEDALPHAOPERATION"); break;
        case D3DERR_UNSUPPORTEDALPHAARG       : LOG("D3DERR_UNSUPPORTEDALPHAARG");       break;
        case D3DERR_TOOMANYOPERATIONS         : LOG("D3DERR_TOOMANYOPERATIONS");         break;
        case D3DERR_CONFLICTINGTEXTUREFILTER  : LOG("D3DERR_CONFLICTINGTEXTUREFILTER");  break;
        case D3DERR_UNSUPPORTEDFACTORVALUE    : LOG("D3DERR_UNSUPPORTEDFACTORVALUE");    break;
        case D3DERR_CONFLICTINGRENDERSTATE    : LOG("D3DERR_CONFLICTINGRENDERSTATE");    break;
        case D3DERR_UNSUPPORTEDTEXTUREFILTER  : LOG("D3DERR_UNSUPPORTEDTEXTUREFILTER");  break;
        case D3DERR_CONFLICTINGTEXTUREPALETTE : LOG("D3DERR_CONFLICTINGTEXTUREPALETTE"); break;
        case D3DERR_DRIVERINTERNALERROR       : LOG("D3DERR_DRIVERINTERNALERROR");       break;
        case D3DERR_NOTFOUND                  : LOG("D3DERR_NOTFOUND");                  break;
        case D3DERR_MOREDATA                  : LOG("D3DERR_MOREDATA");                  break;
        case D3DERR_DEVICELOST                : LOG("D3DERR_DEVICELOST");                break;
        case D3DERR_DEVICENOTRESET            : LOG("D3DERR_DEVICENOTRESET");            break;
        case D3DERR_NOTAVAILABLE              : LOG("D3DERR_NOTAVAILABLE");              break;
        case D3DERR_OUTOFVIDEOMEMORY          : LOG("D3DERR_OUTOFVIDEOMEMORY");          break;
        case D3DERR_INVALIDDEVICE             : LOG("D3DERR_INVALIDDEVICE");             break;
        case D3DERR_INVALIDCALL               : LOG("D3DERR_INVALIDCALL");               break;
        case D3DERR_DRIVERINVALIDCALL         : LOG("D3DERR_DRIVERINVALIDCALL");         break;
        case D3DERR_WASSTILLDRAWING           : LOG("D3DERR_WASSTILLDRAWING");           break;
        default                               : LOG("D3DERR_UNKNOWN");                   break;
    }
    LOG("\n");
    ASSERT(false);
}
#else
    #define D3DCHECK(res) res
#endif

namespace GAPI {
    #include "shaders/d3d9/compose_sprite_vs.h"
    #include "shaders/d3d9/compose_sprite_ps.h"
    #include "shaders/d3d9/compose_flash_vs.h"
    #include "shaders/d3d9/compose_flash_ps.h"
    #include "shaders/d3d9/compose_room_vs.h"
    #include "shaders/d3d9/compose_room_ps.h"
    #include "shaders/d3d9/compose_entity_vs.h"
    #include "shaders/d3d9/compose_entity_ps.h"
    #include "shaders/d3d9/compose_mirror_vs.h"
    #include "shaders/d3d9/compose_mirror_ps.h"
    #include "shaders/d3d9/shadow_vs.h"
    #include "shaders/d3d9/shadow_ps.h"
    #include "shaders/d3d9/ambient_vs.h"
    #include "shaders/d3d9/ambient_ps.h"
    #include "shaders/d3d9/water_drop_vs.h"
    #include "shaders/d3d9/water_drop_ps.h"
    #include "shaders/d3d9/water_simulate_vs.h"
    #include "shaders/d3d9/water_simulate_ps.h"
    #include "shaders/d3d9/water_caustics_vs.h"
    #include "shaders/d3d9/water_caustics_ps.h"
    #include "shaders/d3d9/water_rays_vs.h"
    #include "shaders/d3d9/water_rays_ps.h"
    #include "shaders/d3d9/water_mask_vs.h"
    #include "shaders/d3d9/water_mask_ps.h"
    #include "shaders/d3d9/water_compose_vs.h"
    #include "shaders/d3d9/water_compose_ps.h"
    #include "shaders/d3d9/filter_vs.h"
    #include "shaders/d3d9/filter_ps.h"
    #include "shaders/d3d9/gui_vs.h"
    #include "shaders/d3d9/gui_ps.h"

    using namespace Core;

    typedef ::Vertex Vertex;

    int cullMode, blendMode;
    uint32 clearColor;

    LPDIRECT3DSURFACE9           defRT, defDS;
    LPDIRECT3DVERTEXDECLARATION9 vertexDecl;


    struct Texture;
    struct Mesh;

    struct Resource {
        Texture *texture;
        Mesh    *mesh;
    } resList[256];
    int resCount;

    void registerResource(Mesh *mesh) {
        resList[resCount].mesh    = mesh;
        resList[resCount].texture = NULL;
        resCount++;
    }

    void registerResource(Texture *texture) {
        resList[resCount].mesh    = NULL;
        resList[resCount].texture = texture;
        resCount++;
    }

    void unregisterResource(void *res) {
        for (int i = 0; i < resCount; i++)
            if (resList[i].mesh == res || resList[i].texture == res) {
                resList[i] = resList[--resCount];
                break;
            }
    }

// Shader
    enum {
        USAGE_VS,
        USAGE_PS,
    };

    static const struct Binding {
        int reg;
        int usage;
    } bindings[uMAX] = {
        {   0, USAGE_VS | USAGE_PS }, // uFlags
        {   0, USAGE_VS | USAGE_PS }, // uParam
        {   1, USAGE_VS | USAGE_PS }, // uTexParam
        {   2, USAGE_VS | USAGE_PS }, // uViewProj
        {   6, USAGE_VS | USAGE_PS }, // uBasis
        {  70, USAGE_VS | USAGE_PS }, // uLightProj
        {  74, USAGE_VS | USAGE_PS }, // uMaterial
        {  75, USAGE_VS | USAGE_PS }, // uAmbient
        {  81, USAGE_VS | USAGE_PS }, // uFogParams
        {  82, USAGE_VS | USAGE_PS }, // uViewPos
        {  83, USAGE_VS | USAGE_PS }, // uLightPos
        {  87, USAGE_VS | USAGE_PS }, // uLightColor
        {  91, USAGE_VS | USAGE_PS }, // uRoomSize
        {  92, USAGE_VS | USAGE_PS }, // uPosScale
        {  98, USAGE_VS | USAGE_PS }, // uContacts
    };

    struct Shader {
        LPDIRECT3DVERTEXSHADER9 VS;
        LPDIRECT3DPIXELSHADER9  PS;

        BOOL flags[16];

        Shader() : VS(NULL), PS(NULL) {}

        void init(Core::Pass pass, int type, int *def, int defCount) {
            const BYTE *vSrc, *pSrc;
            switch (pass) {
                case Core::passCompose :
                    switch (type) {
                        case 0 : vSrc = COMPOSE_SPRITE_VS; pSrc = COMPOSE_SPRITE_PS; break;
                        case 1 : vSrc = COMPOSE_FLASH_VS;  pSrc = COMPOSE_FLASH_PS;  break;
                        case 2 : vSrc = COMPOSE_ROOM_VS;   pSrc = COMPOSE_ROOM_PS;   break;
                        case 3 : vSrc = COMPOSE_ENTITY_VS; pSrc = COMPOSE_ENTITY_PS; break;
                        case 4 : vSrc = COMPOSE_MIRROR_VS; pSrc = COMPOSE_MIRROR_PS; break;
                        default : ASSERT(false);
                    }
                    break;
                case Core::passShadow  : vSrc = SHADOW_VS;  pSrc = SHADOW_PS;  break;
                case Core::passAmbient : vSrc = AMBIENT_VS; pSrc = AMBIENT_PS; break;
                case Core::passWater   : 
                    switch (type) {
                        case 0 : vSrc = WATER_DROP_VS;     pSrc = WATER_DROP_PS;     break;
                        case 1 : vSrc = WATER_SIMULATE_VS; pSrc = WATER_SIMULATE_PS; break;
                        case 2 : vSrc = WATER_CAUSTICS_VS; pSrc = WATER_CAUSTICS_PS; break;
                        case 3 : vSrc = WATER_RAYS_VS;     pSrc = WATER_RAYS_PS;     break;
                        case 4 : vSrc = WATER_MASK_VS;     pSrc = WATER_MASK_PS;     break;
                        case 5 : vSrc = WATER_COMPOSE_VS;  pSrc = WATER_COMPOSE_PS;  break;
                        default : ASSERT(false);
                    }
                    break;
                case Core::passFilter  : vSrc = FILTER_VS;  pSrc = FILTER_PS;  break;
                case Core::passGUI     : vSrc = GUI_VS;     pSrc = GUI_PS;     break;
                default                : ASSERT(false); LOG("! wrong pass id\n"); return;
            }

            memset(flags, 0, sizeof(flags));
            flags[type] = TRUE;

            for (int i = 0; i < defCount; i++) {
                switch (def[i]) {
                    case SD_UNDERWATER   : flags[ 5] = TRUE; break;
                    case SD_ALPHA_TEST   : flags[ 6] = TRUE; break;
                    case SD_CLIP_PLANE   : flags[ 7] = TRUE; break;
                    case SD_OPT_AMBIENT  : flags[ 8] = TRUE; break;
                    case SD_OPT_SHADOW   : flags[ 9] = TRUE; break;
                    case SD_OPT_CONTACT  : flags[10] = TRUE; break;
                    case SD_OPT_CAUSTICS : flags[11] = TRUE; break;
                }
            }

            device->CreateVertexShader ((DWORD*)vSrc, &VS);
            device->CreatePixelShader  ((DWORD*)pSrc, &PS);
        }

        void deinit() {
            if (VS) VS->Release();
            if (PS) PS->Release();
        }

        void bind() {
            if (Core::active.shader != this) {
                Core::active.shader = this;
                device->SetVertexShader(VS);
                device->SetPixelShader(PS);

                device->SetVertexShaderConstantB (0, flags, COUNT(flags));
                device->SetPixelShaderConstantB  (0, flags, COUNT(flags));
            }
        }

        void setConstant(UniformType uType, const float *value, int vectors) {
            const Binding &b = bindings[uType];
            if (b.usage | USAGE_VS) device->SetVertexShaderConstantF (b.reg, value, vectors);
            if (b.usage | USAGE_PS) device->SetPixelShaderConstantF  (b.reg, value, vectors);
        }

        void setParam(UniformType uType, const vec4 &value, int count = 1) {
            setConstant(uType, (float*)&value, count);
        }

        void setParam(UniformType uType, const mat4 &value, int count = 1) {
            setConstant(uType, (float*)&value, count * 4);
        }

        void setParam(UniformType uType, const Basis &value, int count = 1) {
            setConstant(uType, (float*)&value, count * 2);
        }
    };

// Texture
    struct Texture {
        LPDIRECT3DTEXTURE9     tex2D;
        LPDIRECT3DCUBETEXTURE9 texCube;

        int       width, height, origWidth, origHeight;
        TexFormat fmt;
        uint32    opt;

        Texture(int width, int height, uint32 opt) : tex2D(NULL), texCube(NULL), width(width), height(height), origWidth(width), origHeight(height), fmt(FMT_RGBA), opt(opt) {}

        void init(void *data) {
            ASSERT((opt & OPT_PROXY) == 0);

            bool isDepth  = fmt == FMT_DEPTH || fmt == FMT_SHADOW;
            bool mipmaps  = (opt & OPT_MIPMAPS) != 0;
            bool cube     = (opt & OPT_CUBEMAP) != 0;
            bool isTarget = (opt & OPT_TARGET)  != 0 && !isDepth;

            static const struct FormatDesc {
                int       bpp;
                D3DFORMAT format;
            } formats[FMT_MAX] = {
                {   8, D3DFMT_L8       },
                {  32, D3DFMT_A8R8G8B8 },
                {  16, D3DFMT_R5G6B5   },
                {  16, D3DFMT_A1R5G5B5 },
                {  64, D3DFMT_G32R32F  },
                {  32, D3DFMT_G16R16F  },
                {  16, D3DFMT_D16      },
                {  16, D3DFMT_D24X8    },
            };
            
            FormatDesc desc = formats[fmt];

            uint32 usage = 0;
            if (mipmaps)  usage |= D3DUSAGE_AUTOGENMIPMAP;
            if (isDepth)  usage |= D3DUSAGE_DEPTHSTENCIL;
            if (isTarget) usage |= D3DUSAGE_RENDERTARGET;

            D3DPOOL pool = (isTarget || isDepth) ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;

            if (cube) {
                D3DCHECK(device->CreateCubeTexture(width, 1, usage, desc.format, pool, &texCube, NULL));
            } else {
                D3DCHECK(device->CreateTexture(width, height, 1, usage, desc.format, pool, &tex2D, NULL));
                if (data && !isTarget) {
                    D3DLOCKED_RECT rect;
                    D3DCHECK(tex2D->LockRect(0, &rect, NULL, 0));
                    if (width != origWidth || height != origHeight) {
                        memset(rect.pBits, 0, width * height * (desc.bpp / 8));
                        uint8 *dst = (uint8*)rect.pBits;
                        uint8 *src = (uint8*)data;
                        for (int y = 0; y < origHeight; y++) {
                            memcpy(dst, src, origWidth * (desc.bpp / 8));
                            src += origWidth * (desc.bpp / 8);
                            dst += width * (desc.bpp / 8);
                        }
                    } else {
                        memcpy(rect.pBits, data, width * height * (desc.bpp / 8));
                    }
                    D3DCHECK(tex2D->UnlockRect(0));
                }
            }

            if (pool != D3DPOOL_MANAGED)
                registerResource(this);
        }

        void deinit() {
            unregisterResource(this);
            if (tex2D)   tex2D->Release();
            if (texCube) texCube->Release();
        }

        void generateMipMap() {
            if (texCube) texCube->GenerateMipSubLevels();
            if (tex2D)   tex2D->GenerateMipSubLevels();
        }

        void update(void *data) {
            D3DLOCKED_RECT rect;
            D3DCHECK(tex2D->LockRect(0, &rect, NULL, 0));
            memcpy(rect.pBits, data, width * height * 4);
            D3DCHECK(tex2D->UnlockRect(0));
        }

        void bind(int sampler) {
            if (opt & OPT_PROXY) return;
            ASSERT(tex2D || texCube);

            if (Core::active.textures[sampler] != this) {
                Core::active.textures[sampler] = this;
                if (tex2D) {
                    device->SetTexture(sampler, tex2D);
                    if (opt & OPT_VERTEX) {
                        device->SetTexture(D3DVERTEXTEXTURESAMPLER0 + sampler, tex2D);
                    }
                } else if (texCube)
                    device->SetTexture(sampler, texCube);

                bool filter  = (Core::settings.detail.filter > Core::Settings::LOW)    && !(opt & OPT_NEAREST);
                bool mipmaps = (Core::settings.detail.filter > Core::Settings::MEDIUM) &&  (opt & OPT_MIPMAPS);
                bool aniso   = filter && mipmaps && (Core::support.maxAniso > 0);

                device->SetSamplerState(sampler, D3DSAMP_ADDRESSU, (opt & OPT_REPEAT) ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);
                device->SetSamplerState(sampler, D3DSAMP_ADDRESSV, (opt & OPT_REPEAT) ? D3DTADDRESS_WRAP : D3DTADDRESS_CLAMP);

                if (aniso) {
                    device->SetSamplerState(sampler, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
                    device->SetSamplerState(sampler, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
                    device->SetSamplerState(sampler, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
                    device->SetSamplerState(sampler, D3DSAMP_MAXANISOTROPY, support.maxAniso);
                } else {
                    device->SetSamplerState(sampler, D3DSAMP_MINFILTER, filter ? D3DTEXF_LINEAR : D3DTEXF_POINT);
                    device->SetSamplerState(sampler, D3DSAMP_MAGFILTER, filter ? D3DTEXF_LINEAR : D3DTEXF_POINT);
                    device->SetSamplerState(sampler, D3DSAMP_MIPFILTER, mipmaps ? (filter ? D3DTEXF_LINEAR : D3DTEXF_POINT) : D3DTEXF_NONE);
                    device->SetSamplerState(sampler, D3DSAMP_MAXANISOTROPY, 1);
                }
            }
        }

        void unbind(int sampler) {
            if (Core::active.textures[sampler]) {
                Core::active.textures[sampler] = NULL;
                device->SetTexture(sampler, NULL);
            }
        }

        void setFilterQuality(int value) {}
    };

// Mesh
    struct Mesh {
        LPDIRECT3DINDEXBUFFER9  IB;
        LPDIRECT3DVERTEXBUFFER9 VB;

        int  iCount;
        int  vCount;
        bool dynamic;

        Mesh(bool dynamic) : IB(NULL), VB(NULL), dynamic(dynamic) {}

        void init(Index *indices, int iCount, ::Vertex *vertices, int vCount, int aCount) {
            this->iCount = iCount;
            this->vCount = vCount;
            ASSERT(sizeof(GAPI::Vertex) == sizeof(::Vertex));

            uint32 usage = D3DUSAGE_WRITEONLY | (dynamic ? D3DUSAGE_DYNAMIC : 0);
            D3DPOOL pool = dynamic ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;

            D3DCHECK(device->CreateIndexBuffer  (iCount * sizeof(Index),  usage, D3DFMT_INDEX16, pool, &IB, NULL));
            D3DCHECK(device->CreateVertexBuffer (vCount * sizeof(Vertex), usage, D3DFMT_UNKNOWN, pool, &VB, NULL));

            update(indices, iCount, vertices, vCount);

            if (pool != D3DPOOL_MANAGED)
                registerResource(this);
        }

        void deinit() {
            unregisterResource(this);
            IB->Release();
            VB->Release();
        }

        void update(Index *indices, int iCount, ::Vertex *vertices, int vCount) {
            ASSERT(sizeof(GAPI::Vertex) == sizeof(::Vertex));

            void* ptr;
            int size;

            if (indices && iCount) {
                size = iCount * sizeof(indices[0]);
                D3DCHECK(IB->Lock(0, 0, &ptr, dynamic ? D3DLOCK_DISCARD : 0));
                memcpy(ptr, indices, size);
                D3DCHECK(IB->Unlock());
            }

            if (vertices && vCount) {
                size = vCount * sizeof(vertices[0]);
                D3DCHECK(VB->Lock(0, 0, &ptr, dynamic ? D3DLOCK_DISCARD: 0));
                memcpy(ptr, vertices, size);
                D3DCHECK(VB->Unlock());
            }
        }

        void bind(const MeshRange &range) const {
            device->SetIndices(IB);
            device->SetStreamSource(0, VB, 0, sizeof(Vertex));
        }

        void initNextRange(MeshRange &range, int &aIndex) const {
            range.aIndex = -1;
        }
    };

//    GLuint FBO, defaultFBO;
    struct RenderTargetCache {
        int count;
        struct Item {
            LPDIRECT3DSURFACE9 surface;
            int     width;
            int     height;
        } items[MAX_RENDER_BUFFERS];
    } rtCache[2];

    void init() {
        memset(rtCache, 0, sizeof(rtCache));
        
        D3DADAPTER_IDENTIFIER9 adapterInfo;
        D3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &adapterInfo);
        LOG("Vendor   : %s\n", adapterInfo.Description);
        LOG("Renderer : Direct3D 9.0c\n");

        support.maxAniso       = 8;
        support.maxVectors     = 16;
        support.shaderBinary   = false;
        support.VAO            = false; // SHADOW_COLOR
        support.depthTexture   = false; // SHADOW_DEPTH
        support.shadowSampler  = false;
        support.discardFrame   = false;
        support.texNPOT        = true;
        support.texRG          = false;
        support.texBorder      = false;
        support.colorFloat     = true;
        support.colorHalf      = true;
        support.texFloatLinear = true;
        support.texFloat       = true;
        support.texHalfLinear  = true;
        support.texHalf        = true;
        support.clipDist       = false;

        #ifdef PROFILE
            support.profMarker = false;
            support.profTiming = false;
        #endif

        const D3DVERTEXELEMENT9 VERTEX_DECL[] = {
            {0, OFFSETOF(Vertex, coord),    D3DDECLTYPE_SHORT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0}, // aCoord
            {0, OFFSETOF(Vertex, normal),   D3DDECLTYPE_SHORT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0}, // aNormal
            {0, OFFSETOF(Vertex, texCoord), D3DDECLTYPE_SHORT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0}, // aTexCoord
            {0, OFFSETOF(Vertex, color),    D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0}, // aColor
            {0, OFFSETOF(Vertex, light),    D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    1}, // aLight
            D3DDECL_END()
        };

        device->CreateVertexDeclaration(VERTEX_DECL, &vertexDecl);

        defRT = defDS = NULL;
    }

    void deinit() {
        vertexDecl->Release();
        if (defRT) defRT->Release();
        if (defDS) defDS->Release();
    }

    void resetDevice() {
    // release dummy RTs
        for (int i = 0; i < 2; i++) {
            RenderTargetCache &cache = rtCache[i];
            for (int j = 0; j < cache.count; j++)
                cache.items[j].surface->Release();
            cache.count = 0;
        }

    // release texture RTs
        int tmpCount = resCount;
        Resource tmpList[256];
        memcpy(tmpList, resList, sizeof(Resource) * tmpCount);

        for (int i = 0; i < tmpCount; i++) {
            Resource &res = tmpList[i];
            if (res.mesh)
                res.mesh->deinit();
            else
                res.texture->deinit();
        }

        if (defRT) defRT->Release();
        if (defDS) defDS->Release();

        D3DCHECK(device->Reset(&d3dpp));

        device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &defRT);
        device->GetDepthStencilSurface(&defDS);

    // reinit texture RTs
        for (int i = 0; i < tmpCount; i++) {
            Resource &res = tmpList[i];
            if (res.mesh)
                res.mesh->init(NULL, res.mesh->iCount, NULL, res.mesh->vCount, 0);
            else
                res.texture->init(NULL);
        }
    }

    mat4 ortho(float l, float r, float b, float t, float znear, float zfar) {
        return mat4(mat4::PROJ_ZERO_POS, l, r, b, t, znear, zfar);
    }

    mat4 perspective(float fov, float aspect, float znear, float zfar) {
        return mat4(mat4::PROJ_ZERO_POS, fov, aspect, znear, zfar);
    }

    bool beginFrame() {
        switch (device->TestCooperativeLevel()) {
            case D3D_OK            :
                break;
            case D3DERR_DEVICELOST :
                return false;
            case D3DERR_DEVICENOTRESET : 
                switch (device->Reset(&d3dpp)) {
                    case D3D_OK            : break;
                    default                : return false;
                }
            case D3DERR_DRIVERINTERNALERROR :
                Core::quit();
                return false;
        }

        if (defRT == NULL) device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &defRT);
        if (defDS == NULL) device->GetDepthStencilSurface(&defDS);

        device->BeginScene();
        return true;
    }

    void endFrame() {
        device->EndScene();
    }

    void resetState() {
        device->SetVertexDeclaration(vertexDecl);
        device->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
        device->SetRenderState(D3DRS_LIGHTING, FALSE);
        device->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    }

    int cacheRenderTarget(bool depth, int width, int height) {
        RenderTargetCache &cache = rtCache[depth];

        for (int i = 0; i < cache.count; i++)
            if (cache.items[i].width == width && cache.items[i].height == height)
                return i;

        ASSERT(cache.count < MAX_RENDER_BUFFERS);

        RenderTargetCache::Item &item = cache.items[cache.count];
        item.width  = width;
        item.height = height;

        if (depth)
            device->CreateDepthStencilSurface(width, height, D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, true, &item.surface, NULL);
        else
            device->CreateRenderTarget(width, height, D3DFMT_R5G6B5, D3DMULTISAMPLE_NONE, 0, false, &item.surface, NULL);

        return cache.count++;
    }

    void bindTarget(Texture *target, int face) {
        if (!target) { // may be a null
            D3DCHECK(device->SetRenderTarget(0, defRT));
            D3DCHECK(device->SetDepthStencilSurface(defDS));
        } else {
            ASSERT(target->opt & OPT_TARGET);

            LPDIRECT3DSURFACE9 surface;

            bool depth = target->fmt == FMT_DEPTH || target->fmt == FMT_SHADOW;

            if (target->tex2D) {
                D3DCHECK(target->tex2D->GetSurfaceLevel(0, &surface));
            } else if (target->texCube)
                D3DCHECK(target->texCube->GetCubeMapSurface(D3DCUBEMAP_FACES(D3DCUBEMAP_FACE_POSITIVE_X + face), 0, &surface));

            int rtIndex = cacheRenderTarget(!depth, target->width, target->height);

            if (depth) {
                D3DCHECK(device->SetRenderTarget(0, rtCache[false].items[rtIndex].surface));
                D3DCHECK(device->SetDepthStencilSurface(surface));
            } else {
                D3DCHECK(device->SetRenderTarget(0, surface));
                D3DCHECK(device->SetDepthStencilSurface(rtCache[true].items[rtIndex].surface));
            }

            surface->Release();
        }

        Core::active.viewport = Viewport(0, 0, 0, 0); // forcing viewport reset
    }

    void discardTarget(bool color, bool depth) {}

    void copyTarget(Texture *dst, int xOffset, int yOffset, int x, int y, int width, int height) {
        ASSERT(dst && dst->tex2D);

        LPDIRECT3DSURFACE9 surface;
        dst->tex2D->GetSurfaceLevel(0, &surface);

        RECT srcRect = { x, y, x + width, y + height },
             dstRect = { xOffset, yOffset, xOffset + width, yOffset + height };

        device->StretchRect(defRT, &srcRect, surface, &dstRect, D3DTEXF_POINT);

        surface->Release();
    }

    void setVSync(bool enable) {
        d3dpp.PresentationInterval = enable ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
        GAPI::resetDevice();
    }

    void waitVBlank() {}

    void clear(bool color, bool depth) {
        uint32 flags = (color ? D3DCLEAR_TARGET : 0) | (depth ? D3DCLEAR_ZBUFFER : 0);
        device->Clear(0, NULL, flags, clearColor, 1.0f, 0);
    }

    void setClearColor(const vec4 &color) {
        clearColor = (int(color.x * 255) << 16) |
                     (int(color.y * 255) << 8 ) |
                     (int(color.z * 255)      ) |
                     (int(color.w * 255) << 24);
    }

    void setViewport(const Viewport &vp) {
        D3DVIEWPORT9 dv;
        dv.X      = vp.x;
        dv.Y      = vp.y;
        dv.Width  = vp.width;
        dv.Height = vp.height;
        dv.MinZ   = 0.0f;
        dv.MaxZ   = 1.0f;

        RECT ds;
        ds.left   = vp.x;
        ds.top    = vp.y;
        ds.right  = vp.x + vp.width;
        ds.bottom = vp.y + vp.height;

        device->SetViewport(&dv);
        device->SetScissorRect(&ds);
    }

    void setDepthTest(bool enable) {
        device->SetRenderState(D3DRS_ZENABLE, enable);
    }

    void setDepthWrite(bool enable) {
        device->SetRenderState(D3DRS_ZWRITEENABLE, enable);
    }

    void setColorWrite(bool r, bool g, bool b, bool a) {
        device->SetRenderState(D3DRS_COLORWRITEENABLE,
            (r ? D3DCOLORWRITEENABLE_RED   : 0) |
            (g ? D3DCOLORWRITEENABLE_GREEN : 0) |
            (b ? D3DCOLORWRITEENABLE_BLUE  : 0) |
            (a ? D3DCOLORWRITEENABLE_ALPHA : 0));
    }

    void setAlphaTest(bool enable) {}

    void setCullMode(int rsMask) {
        cullMode = rsMask;
        switch (rsMask) {
            case RS_CULL_BACK  : device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);  break;
            case RS_CULL_FRONT : device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW); break;
            default            : device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        }
    }

    void setBlendMode(int rsMask) {
        blendMode = rsMask;
        switch (rsMask) {
            case RS_BLEND_ALPHA   : 
                device->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
                device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
                break;
            case RS_BLEND_ADD     :
                device->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);
                device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
                break;
            case RS_BLEND_MULT    :
                device->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_DESTCOLOR);
                device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
                break;
            case RS_BLEND_PREMULT :
                device->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);
                device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
                break;
            default               :
                device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
                return;
        }
        device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    }

    void setViewProj(const mat4 &mView, const mat4 &mProj) {}

    void updateLights(vec4 *lightPos, vec4 *lightColor, int count) {
        if (active.shader) {
            active.shader->setParam(uLightColor, lightColor[0], count);
            active.shader->setParam(uLightPos,   lightPos[0],   count);
        }
    }

    void DIP(Mesh *mesh, const MeshRange &range) {
        device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, range.vStart, 0, mesh->vCount, range.iStart, range.iCount / 3);
    }

    vec4 copyPixel(int x, int y) {
        GAPI::Texture *t = Core::active.target;
        ASSERT(t && t->tex2D);

        LPDIRECT3DSURFACE9 surface, texSurface;
        D3DCHECK(t->tex2D->GetSurfaceLevel(0, &texSurface));
        D3DCHECK(device->CreateOffscreenPlainSurface(t->width, t->height, D3DFMT_A8R8G8B8, D3DPOOL_SYSTEMMEM, &surface, NULL));
        D3DCHECK(device->GetRenderTargetData(texSurface, surface));

        RECT r = { x, y, x + 1, y + 1 };
        D3DLOCKED_RECT rect;
        surface->LockRect(&rect, &r, D3DLOCK_READONLY);
        ubyte4 c = *((ubyte4*)rect.pBits);
        surface->UnlockRect();

        texSurface->Release();
        surface->Release();

        return vec4(float(c.z), float(c.y), float(c.x), float(c.w)) * (1.0f / 255.0f);
    }

    void initPSO(PSO *pso) {
        ASSERT(pso);
        ASSERT(pso && pso->data == NULL);
        pso->data = &pso;
    }

    void deinitPSO(PSO *pso) {
        ASSERT(pso);
        ASSERT(pso->data != NULL);
        pso->data = NULL;
    }

    void bindPSO(const PSO *pso) {
        //
    }
}

#endif