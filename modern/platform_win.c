/* platform_win.c - Win32 + Direct3D 11 Implementierung der Plattform-Schicht. */
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <mmsystem.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "platform.h"
#include "gfx.h"

/* ---------------------------------------------------------------------- */
/* Zustand                                                                */
/* ---------------------------------------------------------------------- */
static HWND  g_hwnd;
static int   g_quit;
static int   g_bbw, g_bbh;          /* Backbuffer-Groesse (Pixel)         */
static plat_config g_cfg;

static ID3D11Device        *g_dev;
static ID3D11DeviceContext *g_ctx;
static IDXGISwapChain      *g_swap;
static ID3D11RenderTargetView *g_rtv;
static ID3D11Texture2D     *g_tex;   /* 320x200 dynamisch                 */
static ID3D11ShaderResourceView *g_srv;
static ID3D11SamplerState  *g_samp;
static ID3D11VertexShader  *g_vs;
static ID3D11PixelShader   *g_ps;
static ID3D11InputLayout   *g_il;
static ID3D11Buffer        *g_vbo;
static ID3D11Buffer        *g_cbuf;

typedef struct { float x, y, u, v; } vtx;

typedef struct { float texSize[2]; float pad[2];
                 float scan, glow, vig, time; } ps_consts;

static void audio_init(void);
static void audio_shutdown(void);

/* ---- Tastatur-Eventqueue --------------------------------------------- */
typedef struct { int sc; int ch; } keyev;
#define KQ_MAX 64
static keyev g_kq[KQ_MAX];
static int   g_kq_head, g_kq_tail;

static void kq_push(int sc, int ch) {
    int nt = (g_kq_tail + 1) % KQ_MAX;
    if (nt == g_kq_head) return;         /* voll -> aeltestes verwerfen unnoetig */
    g_kq[g_kq_tail].sc = sc;
    g_kq[g_kq_tail].ch = ch;
    g_kq_tail = nt;
}
static int kq_pop(keyev *e) {
    if (g_kq_head == g_kq_tail) return 0;
    *e = g_kq[g_kq_head];
    g_kq_head = (g_kq_head + 1) % KQ_MAX;
    return 1;
}

/* VK -> (BIOS-Scancode, ASCII). ch==0 kennzeichnet Nicht-Zeichen-Tasten. */
static void map_vk(WPARAM vk, int *sc, int *ch) {
    *sc = 0; *ch = 0;
    switch (vk) {
        case VK_UP:     *sc = 72; return;
        case VK_DOWN:   *sc = 80; return;
        case VK_LEFT:   *sc = 75; return;
        case VK_RIGHT:  *sc = 77; return;
        case VK_HOME:   *sc = 71; return;
        case VK_PRIOR:  *sc = 73; return;   /* PgUp */
        case VK_END:    *sc = 79; return;
        case VK_NEXT:   *sc = 81; return;   /* PgDn */
        case VK_SPACE:  *sc = 57; *ch = ' '; return;
        case VK_ESCAPE: *sc = 1;  *ch = 27; return;
        case VK_RETURN: *sc = 28; *ch = 13; return;
        case VK_BACK:   *sc = 14; *ch = 8;  return;
    }
    if (vk >= '0' && vk <= '9') { *ch = (int)vk; return; }
    if (vk >= 'A' && vk <= 'Z') { *ch = (int)vk; return; }   /* Grossbuchstaben */
}

/* ---------------------------------------------------------------------- */
/* Fensterprozedur                                                        */
/* ---------------------------------------------------------------------- */
static LRESULT CALLBACK wndproc(HWND h, UINT m, WPARAM w, LPARAM l) {
    switch (m) {
        case WM_CLOSE:
        case WM_DESTROY:
            g_quit = 1;
            PostQuitMessage(0);
            return 0;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            int sc, ch;
            map_vk(w, &sc, &ch);
            if (sc || ch) kq_push(sc, ch);
            /* Alt+F4 weiterhin zulassen */
            break;
        }
    }
    return DefWindowProc(h, m, w, l);
}

/* ---------------------------------------------------------------------- */
/* Shader-Quellen                                                         */
/* ---------------------------------------------------------------------- */
static const char *HLSL =
"struct VSIn { float2 pos:POSITION; float2 uv:TEXCOORD0; };\n"
"struct VSOut{ float4 pos:SV_POSITION; float2 uv:TEXCOORD0; };\n"
"VSOut vs_main(VSIn i){ VSOut o; o.pos=float4(i.pos,0,1); o.uv=i.uv; return o; }\n"
"cbuffer C:register(b0){ float2 texSize; float2 pad; float scan; float glow; float vig; float t; };\n"
"Texture2D tex:register(t0); SamplerState smp:register(s0);\n"
"float4 ps_main(VSOut i):SV_Target{\n"
"  float3 col = tex.Sample(smp,i.uv).rgb;\n"
"  if(glow>0){ float2 px=1.0/texSize;\n"
"    float3 g = tex.Sample(smp,i.uv+float2(px.x,0)).rgb + tex.Sample(smp,i.uv-float2(px.x,0)).rgb\n"
"             + tex.Sample(smp,i.uv+float2(0,px.y)).rgb + tex.Sample(smp,i.uv-float2(0,px.y)).rgb;\n"
"    col += g*0.25*glow; }\n"
"  if(scan>0){ float ln=i.uv.y*texSize.y; float s=0.5+0.5*cos(ln*6.2831853);\n"
"    col *= 1.0 - scan*(1.0-s); }\n"
"  if(vig>0){ float2 d=i.uv-0.5; float v=1.0-dot(d,d)*vig*2.0; col*=saturate(v); }\n"
"  return float4(saturate(col),1);\n"
"}\n";

static ID3DBlob *compile(const char *entry, const char *target) {
    ID3DBlob *code = NULL, *err = NULL;
    HRESULT hr = D3DCompile(HLSL, strlen(HLSL), NULL, NULL, NULL,
                            entry, target, 0, 0, &code, &err);
    if (FAILED(hr)) {
        if (err) { OutputDebugStringA((char *)ID3D10Blob_GetBufferPointer(err));
                   ID3D10Blob_Release(err); }
        return NULL;
    }
    if (err) ID3D10Blob_Release(err);
    return code;
}

/* Fitting-Rechteck (320x200) in den Backbuffer einpassen, NDC-Quad fuellen. */
static void build_quad(void) {
    float A = g_cfg.aspect43 ? (4.0f / 3.0f) : (320.0f / 200.0f);
    float rw, rh, nx, ny;
    vtx q[6];
    D3D11_MAPPED_SUBRESOURCE ms;

    rw = (float)g_bbw; rh = rw / A;
    if (rh > g_bbh) { rh = (float)g_bbh; rw = rh * A; }
    if (g_cfg.integer_scale && !g_cfg.aspect43) {
        int s = g_bbw / 320; int s2 = g_bbh / 200;
        if (s2 < s) s = s2;
        if (s < 1) s = 1;
        rw = 320.0f * s; rh = 200.0f * s;
    }
    /* Bildanteil an der Backbuffer-Breite/-Hoehe = NDC-Halbbreite/-hoehe. */
    nx = rw / (float)g_bbw;
    ny = rh / (float)g_bbh;

    /* Zwei Dreiecke, y in NDC nach oben positiv -> v invertiert. */
    q[0] = (vtx){-nx,  ny, 0, 0};
    q[1] = (vtx){ nx,  ny, 1, 0};
    q[2] = (vtx){ nx, -ny, 1, 1};
    q[3] = (vtx){-nx,  ny, 0, 0};
    q[4] = (vtx){ nx, -ny, 1, 1};
    q[5] = (vtx){-nx, -ny, 0, 1};

    if (SUCCEEDED(ID3D11DeviceContext_Map(g_ctx, (ID3D11Resource *)g_vbo, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &ms))) {
        memcpy(ms.pData, q, sizeof(q));
        ID3D11DeviceContext_Unmap(g_ctx, (ID3D11Resource *)g_vbo, 0);
    }
}

/* ---------------------------------------------------------------------- */
int plat_init(const plat_config *cfg) {
    WNDCLASSA wc; DWORD style; RECT rc;
    int scrw, scrh, winw, winh, x, y;
    DXGI_SWAP_CHAIN_DESC sd;
    ID3D11Texture2D *back = NULL;
    D3D11_TEXTURE2D_DESC td;
    D3D11_SAMPLER_DESC sadesc;
    D3D11_BUFFER_DESC bd;
    D3D11_INPUT_ELEMENT_DESC ied[2];
    ID3DBlob *vsb, *psb;
    HRESULT hr;

    g_cfg = *cfg;
    g_quit = 0;
    g_kq_head = g_kq_tail = 0;      /* Input-Queue bei (Re-)Init leeren */
    SetProcessDPIAware();

    scrw = GetSystemMetrics(SM_CXSCREEN);
    scrh = GetSystemMetrics(SM_CYSCREEN);

    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = wndproc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "SpaceflightWnd";
    RegisterClassA(&wc);

    if (g_cfg.fullscreen) {
        winw = scrw; winh = scrh;
        style = WS_POPUP;
        x = 0; y = 0;
        g_bbw = (g_cfg.width  > 0) ? g_cfg.width  : scrw;
        g_bbh = (g_cfg.height > 0) ? g_cfg.height : scrh;
    } else {
        g_bbw = (g_cfg.width  > 0) ? g_cfg.width  : 1280;
        g_bbh = (g_cfg.height > 0) ? g_cfg.height : 800;
        winw = g_bbw; winh = g_bbh;
        style = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;
        rc.left = 0; rc.top = 0; rc.right = winw; rc.bottom = winh;
        AdjustWindowRect(&rc, style, FALSE);
        winw = rc.right - rc.left; winh = rc.bottom - rc.top;
        x = (scrw - winw) / 2; y = (scrh - winh) / 2;
    }
    /* Im randlosen Vollbild ist der Backbuffer = Fenster = Bildschirm. */
    if (g_cfg.fullscreen) { g_bbw = winw; g_bbh = winh; }

    g_hwnd = CreateWindowA("SpaceflightWnd", "Spaceflight (1989) - Direct3D 11",
                           style, x, y, winw, winh, NULL, NULL, wc.hInstance, NULL);
    if (!g_hwnd) return 1;

    memset(&sd, 0, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width  = g_bbw;
    sd.BufferDesc.Height = g_bbh;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;                       /* randloses "Fake-Fullscreen" */
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    hr = D3D11CreateDeviceAndSwapChain(
        NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION,
        &sd, &g_swap, &g_dev, NULL, &g_ctx);
    if (FAILED(hr)) return 2;

    IDXGISwapChain_GetBuffer(g_swap, 0, &IID_ID3D11Texture2D, (void **)&back);
    ID3D11Device_CreateRenderTargetView(g_dev, (ID3D11Resource *)back, NULL, &g_rtv);
    ID3D11Texture2D_Release(back);

    /* 320x200 dynamische Textur */
    memset(&td, 0, sizeof(td));
    td.Width = FB_W; td.Height = FB_H; td.MipLevels = 1; td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DYNAMIC;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    td.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    if (FAILED(ID3D11Device_CreateTexture2D(g_dev, &td, NULL, &g_tex))) return 3;
    ID3D11Device_CreateShaderResourceView(g_dev, (ID3D11Resource *)g_tex, NULL, &g_srv);

    /* Shader */
    vsb = compile("vs_main", "vs_4_0");
    psb = compile("ps_main", "ps_4_0");
    if (!vsb || !psb) return 4;
    ID3D11Device_CreateVertexShader(g_dev, ID3D10Blob_GetBufferPointer(vsb),
        ID3D10Blob_GetBufferSize(vsb), NULL, &g_vs);
    ID3D11Device_CreatePixelShader(g_dev, ID3D10Blob_GetBufferPointer(psb),
        ID3D10Blob_GetBufferSize(psb), NULL, &g_ps);

    ied[0].SemanticName = "POSITION"; ied[0].SemanticIndex = 0;
    ied[0].Format = DXGI_FORMAT_R32G32_FLOAT; ied[0].InputSlot = 0;
    ied[0].AlignedByteOffset = 0; ied[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    ied[0].InstanceDataStepRate = 0;
    ied[1].SemanticName = "TEXCOORD"; ied[1].SemanticIndex = 0;
    ied[1].Format = DXGI_FORMAT_R32G32_FLOAT; ied[1].InputSlot = 0;
    ied[1].AlignedByteOffset = 8; ied[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    ied[1].InstanceDataStepRate = 0;
    ID3D11Device_CreateInputLayout(g_dev, ied, 2, ID3D10Blob_GetBufferPointer(vsb),
        ID3D10Blob_GetBufferSize(vsb), &g_il);
    ID3D10Blob_Release(vsb); ID3D10Blob_Release(psb);

    /* Vertexpuffer (6 Vertices, dynamisch) */
    memset(&bd, 0, sizeof(bd));
    bd.ByteWidth = sizeof(vtx) * 6;
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ID3D11Device_CreateBuffer(g_dev, &bd, NULL, &g_vbo);

    /* Konstantenpuffer */
    memset(&bd, 0, sizeof(bd));
    bd.ByteWidth = sizeof(ps_consts);
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ID3D11Device_CreateBuffer(g_dev, &bd, NULL, &g_cbuf);

    /* Sampler: Point (knackige Pixel) + Clamp */
    memset(&sadesc, 0, sizeof(sadesc));
    sadesc.Filter = g_cfg.integer_scale ? D3D11_FILTER_MIN_MAG_MIP_POINT
                                        : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sadesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sadesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sadesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    ID3D11Device_CreateSamplerState(g_dev, &sadesc, &g_samp);

    build_quad();
    audio_init();

    ShowWindow(g_hwnd, SW_SHOW);
    SetForegroundWindow(g_hwnd);
    SetFocus(g_hwnd);
    return 0;
}

void plat_shutdown(void) {
    audio_shutdown();
    if (g_ctx) ID3D11DeviceContext_ClearState(g_ctx);
    #define REL(p) do{ if(p){ IUnknown_Release((IUnknown*)(p)); (p)=NULL; } }while(0)
    REL(g_cbuf); REL(g_vbo); REL(g_il); REL(g_ps); REL(g_vs);
    REL(g_samp); REL(g_srv); REL(g_tex); REL(g_rtv); REL(g_swap);
    REL(g_ctx); REL(g_dev);
    #undef REL
    if (g_hwnd) { DestroyWindow(g_hwnd); g_hwnd = NULL; }
}

int plat_pump(void) {
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return !g_quit;
}

void plat_present(void) {
    D3D11_MAPPED_SUBRESOURCE ms;
    D3D11_VIEWPORT vp;
    ps_consts pc;
    UINT stride = sizeof(vtx), offset = 0;
    float clear[4] = {0, 0, 0, 1};
    int x, y;

    /* g_fb -> RGBA in die Textur */
    if (SUCCEEDED(ID3D11DeviceContext_Map(g_ctx, (ID3D11Resource *)g_tex, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &ms))) {
        for (y = 0; y < FB_H; y++) {
            uint8_t *dst = (uint8_t *)ms.pData + y * ms.RowPitch;
            uint8_t *src = g_fb[y];
            for (x = 0; x < FB_W; x++) {
                rgba_t c = g_palette[src[x] & 3];
                dst[x * 4 + 0] = c.r; dst[x * 4 + 1] = c.g;
                dst[x * 4 + 2] = c.b; dst[x * 4 + 3] = 255;
            }
        }
        ID3D11DeviceContext_Unmap(g_ctx, (ID3D11Resource *)g_tex, 0);
    }

    vp.TopLeftX = 0; vp.TopLeftY = 0;
    vp.Width = (float)g_bbw; vp.Height = (float)g_bbh;
    vp.MinDepth = 0; vp.MaxDepth = 1;
    ID3D11DeviceContext_RSSetViewports(g_ctx, 1, &vp);
    ID3D11DeviceContext_OMSetRenderTargets(g_ctx, 1, &g_rtv, NULL);
    ID3D11DeviceContext_ClearRenderTargetView(g_ctx, g_rtv, clear);

    pc.texSize[0] = FB_W; pc.texSize[1] = FB_H;
    pc.pad[0] = pc.pad[1] = 0;
    pc.scan = g_cfg.fx_scanline; pc.glow = g_cfg.fx_glow;
    pc.vig = g_cfg.fx_vignette; pc.time = (float)plat_time();
    if (SUCCEEDED(ID3D11DeviceContext_Map(g_ctx, (ID3D11Resource *)g_cbuf, 0,
            D3D11_MAP_WRITE_DISCARD, 0, &ms))) {
        memcpy(ms.pData, &pc, sizeof(pc));
        ID3D11DeviceContext_Unmap(g_ctx, (ID3D11Resource *)g_cbuf, 0);
    }

    ID3D11DeviceContext_IASetInputLayout(g_ctx, g_il);
    ID3D11DeviceContext_IASetVertexBuffers(g_ctx, 0, 1, &g_vbo, &stride, &offset);
    ID3D11DeviceContext_IASetPrimitiveTopology(g_ctx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ID3D11DeviceContext_VSSetShader(g_ctx, g_vs, NULL, 0);
    ID3D11DeviceContext_PSSetShader(g_ctx, g_ps, NULL, 0);
    ID3D11DeviceContext_PSSetConstantBuffers(g_ctx, 0, 1, &g_cbuf);
    ID3D11DeviceContext_PSSetShaderResources(g_ctx, 0, 1, &g_srv);
    ID3D11DeviceContext_PSSetSamplers(g_ctx, 0, 1, &g_samp);
    ID3D11DeviceContext_Draw(g_ctx, 6, 0);

    IDXGISwapChain_Present(g_swap, g_cfg.vsync ? 1 : 0, 0);
}

/* ---- Timing ---------------------------------------------------------- */
double plat_time(void) {
    static LARGE_INTEGER freq;
    LARGE_INTEGER now;
    if (!freq.QuadPart) QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&now);
    return (double)now.QuadPart / (double)freq.QuadPart;
}
void plat_sleep_ms(double ms) {
    if (ms <= 0) return;
    Sleep((DWORD)(ms + 0.5));
}

/* ---- Tastatur -------------------------------------------------------- */
int plat_keypressed(void) {
    plat_pump();
    return g_kq_head != g_kq_tail;
}
int plat_readkey_char(void) {
    keyev e;
    while (!kq_pop(&e)) { if (!plat_pump()) return 0; plat_sleep_ms(1); }
    return e.ch;
}
int plat_readkey_scancode(void) {
    keyev e;
    while (!kq_pop(&e)) { if (!plat_pump()) return 1; plat_sleep_ms(1); }
    return e.sc;
}
int plat_getkey(void) {
    keyev e;
    while (!kq_pop(&e)) { if (!plat_pump()) return 27; plat_sleep_ms(1); }
    if (e.ch) return e.ch;
    return 256 + e.sc;
}
void plat_clear_input(void) {
    plat_pump();
    g_kq_head = g_kq_tail = 0;
}
void plat_inject_key(int sc, int ch) { kq_push(sc, ch); }

/* ---- Audio: PC-Speaker-Ersatz (Rechteckton via WinMM waveOut) -------- */
#define SND_RATE   22050
#define SND_BUFLEN 512
#define SND_NBUF   4
#define SND_AMP    46

#define SND_HOLD 0.05               /* Mindest-Tondauer (s) -> kurze Blips hoerbar */

static HWAVEOUT g_wo;
static WAVEHDR  g_wh[SND_NBUF];
static unsigned char g_wbuf[SND_NBUF][SND_BUFLEN];
static HANDLE   g_snd_thread;
static volatile int g_snd_run;
static volatile int g_snd_freq;     /* 0 = Stille */
static volatile double g_snd_hold_until;  /* Ton laeuft bis zu diesem Zeitpunkt */
static double   g_snd_phase;        /* nur vom Audio-Thread benutzt */

static void fill_buffer(unsigned char *buf) {
    int freq = g_snd_freq, i;
    if (freq <= 0 || plat_time() >= g_snd_hold_until) {
        memset(buf, 128, SND_BUFLEN);   /* Stille (8-bit unsigned Mitte) */
        g_snd_phase = 0;
        return;
    }
    {
        double inc = (double)freq / (double)SND_RATE;   /* Zyklen pro Sample */
        for (i = 0; i < SND_BUFLEN; i++) {
            g_snd_phase += inc;
            if (g_snd_phase >= 1.0) g_snd_phase -= 1.0;
            buf[i] = (g_snd_phase < 0.5) ? (128 + SND_AMP) : (128 - SND_AMP);
        }
    }
}

static DWORD WINAPI snd_thread(LPVOID param) {
    (void)param;
    while (g_snd_run) {
        int i;
        for (i = 0; i < SND_NBUF; i++) {
            if (g_wh[i].dwFlags & WHDR_DONE) {
                waveOutUnprepareHeader(g_wo, &g_wh[i], sizeof(WAVEHDR));
                fill_buffer((unsigned char *)g_wh[i].lpData);
                g_wh[i].dwFlags = 0;
                g_wh[i].dwBufferLength = SND_BUFLEN;
                waveOutPrepareHeader(g_wo, &g_wh[i], sizeof(WAVEHDR));
                waveOutWrite(g_wo, &g_wh[i], sizeof(WAVEHDR));
            }
        }
        Sleep(4);
    }
    return 0;
}

static void audio_init(void) {
    WAVEFORMATEX wfx;
    int i;
    memset(&wfx, 0, sizeof(wfx));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = SND_RATE;
    wfx.wBitsPerSample = 8;
    wfx.nBlockAlign = 1;
    wfx.nAvgBytesPerSec = SND_RATE;
    if (waveOutOpen(&g_wo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        g_wo = NULL;
        return;
    }
    for (i = 0; i < SND_NBUF; i++) {
        memset(&g_wh[i], 0, sizeof(WAVEHDR));
        fill_buffer(g_wbuf[i]);            /* Start: Stille */
        g_wh[i].lpData = (LPSTR)g_wbuf[i];
        g_wh[i].dwBufferLength = SND_BUFLEN;
        waveOutPrepareHeader(g_wo, &g_wh[i], sizeof(WAVEHDR));
        waveOutWrite(g_wo, &g_wh[i], sizeof(WAVEHDR));
    }
    g_snd_run = 1;
    g_snd_thread = CreateThread(NULL, 0, snd_thread, NULL, 0, NULL);
}

static void audio_shutdown(void) {
    int i;
    if (!g_wo) return;
    g_snd_run = 0;
    if (g_snd_thread) {
        WaitForSingleObject(g_snd_thread, 1000);
        CloseHandle(g_snd_thread);
        g_snd_thread = NULL;
    }
    waveOutReset(g_wo);
    for (i = 0; i < SND_NBUF; i++)
        if (g_wh[i].dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(g_wo, &g_wh[i], sizeof(WAVEHDR));
    waveOutClose(g_wo);
    g_wo = NULL;
}

void plat_sound(int freq) {
    if (freq > 0) {
        g_snd_freq = freq;
        g_snd_hold_until = plat_time() + SND_HOLD;   /* garantierte Mindestdauer */
    }
}
/* NoSound() beendet den Ton nicht abrupt - die Mindestdauer laeuft aus, damit
   die vielen Sound()+NoSound()-Paare des Originals ueberhaupt hoerbar werden. */
void plat_nosound(void) { }

/* ---- Font aus GDI ---------------------------------------------------- */
#define GLYPH 16
static uint16_t g_font[96][GLYPH];
static int g_fminx = 0, g_fmaxx = 15, g_fminy = 0, g_fmaxy = 15;

void plat_font_metrics(int *minx, int *maxx, int *miny, int *maxy) {
    *minx = g_fminx; *maxx = g_fmaxx; *miny = g_fminy; *maxy = g_fmaxy;
}

const uint16_t (*plat_make_font(void))[GLYPH] {
    HDC memdc; HBITMAP bmp, oldbmp; HFONT font, oldfont;
    BITMAPINFO bi; void *bits = NULL;
    int c, x, y;

    memdc = CreateCompatibleDC(NULL);
    memset(&bi, 0, sizeof(bi));
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = GLYPH;
    bi.bmiHeader.biHeight = -GLYPH;        /* top-down */
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    bmp = CreateDIBSection(memdc, &bi, DIB_RGB_COLORS, &bits, NULL, 0);
    oldbmp = (HBITMAP)SelectObject(memdc, bmp);

    /* 15px hoch in 16er-Zelle, damit Ober-/Unterlaengen passen. */
    font = CreateFontA(15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                       NONANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");
    oldfont = (HFONT)SelectObject(memdc, font);
    SetTextColor(memdc, RGB(255, 255, 255));
    SetBkColor(memdc, RGB(0, 0, 0));
    SetBkMode(memdc, OPAQUE);

    for (c = 0; c < 96; c++) {
        char ch = (char)(c + 32);
        RECT r = {0, 0, GLYPH, GLYPH};
        uint32_t *px = (uint32_t *)bits;
        FillRect(memdc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
        TextOutA(memdc, 1, 0, &ch, 1);     /* 1px Rand links */
        GdiFlush();
        for (y = 0; y < GLYPH; y++) {
            uint16_t row = 0;
            for (x = 0; x < GLYPH; x++) {
                uint32_t p = px[y * GLYPH + x];
                if ((p & 0xFF) > 100) row |= (0x8000 >> x);
            }
            g_font[c][y] = row;
        }
    }

    SelectObject(memdc, oldfont); DeleteObject(font);
    SelectObject(memdc, oldbmp);  DeleteObject(bmp);
    DeleteDC(memdc);

    /* Ink-Box ueber alle Glyphen (ausser Space) bestimmen. */
    g_fminx = 15; g_fmaxx = 0; g_fminy = 15; g_fmaxy = 0;
    for (c = 1; c < 96; c++) {
        for (y = 0; y < GLYPH; y++) {
            uint16_t rb = g_font[c][y];
            if (!rb) continue;
            if (y < g_fminy) g_fminy = y;
            if (y > g_fmaxy) g_fmaxy = y;
            for (x = 0; x < GLYPH; x++)
                if (rb & (0x8000 >> x)) {
                    if (x < g_fminx) g_fminx = x;
                    if (x > g_fmaxx) g_fmaxx = x;
                }
        }
    }
    if (g_fmaxx < g_fminx) { g_fminx = 0; g_fmaxx = 15; }
    if (g_fmaxy < g_fminy) { g_fminy = 0; g_fmaxy = 15; }
    return g_font;
}
