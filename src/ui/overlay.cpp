// overlay.cpp
#include "overlay.h"
#include "menu.h"
#include "core/offsets.h"
#include "core/syscall.h"
#include "core/process.h"
#include "sdk/client.h"
#include "features/aimbot.h"
#include "features/triggerbot.h"
#include "features/esp.h"
#include "features/radar.h"
#include <MinHook.h>
#include <d3d11.h>
#include <dxgi.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

// forward from imgui_impl_win32
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace overlay {

using PresentFn = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT);
using ResizeFn  = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

static PresentFn  g_orig_present = nullptr;
static ResizeFn   g_orig_resize  = nullptr;
static void*      g_present_addr = nullptr;
static void*      g_resize_addr  = nullptr;
static ID3D11Device*        g_device = nullptr;
static ID3D11DeviceContext* g_ctx    = nullptr;
static ID3D11RenderTargetView* g_rtv = nullptr;
static HWND                 g_wnd  = nullptr;
static WNDPROC              g_orig_wndproc = nullptr;
static bool                 g_imgui_init = false;

static void* get_present_target() {
    // create dummy swapchain to grab vtable
    DXGI_SWAP_CHAIN_DESC desc{};
    desc.BufferCount = 1;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.OutputWindow = GetForegroundWindow();
    desc.SampleDesc.Count = 1;
    desc.Windowed = TRUE;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    IDXGISwapChain* sc = nullptr;
    ID3D11Device* dev = nullptr;
    ID3D11DeviceContext* ctx = nullptr;
    D3D_FEATURE_LEVEL lvl;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        nullptr, 0, D3D11_SDK_VERSION, &desc, &sc, &dev, &lvl, &ctx);
    if (FAILED(hr) || !sc) return nullptr;
    void** vt = *reinterpret_cast<void***>(sc);
    void* present = vt[8];
    void* resize  = vt[13];
    sc->Release();
    if (dev) dev->Release();
    if (ctx) ctx->Release();
    g_present_addr = present;
    g_resize_addr = resize;
    return present;
}

static LRESULT CALLBACK wndproc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (g_imgui_init) ImGui_ImplWin32_WndProcHandler(h, m, w, l);
    if (m == WM_KEYDOWN && w == VK_INSERT) {
        ui::g_menu_open = !ui::g_menu_open;
        return 0;
    }
    return CallWindowProcW(g_orig_wndproc, h, m, w, l);
}

static void init_imgui(IDXGISwapChain* sc) {
    if (g_imgui_init) return;
    if (FAILED(sc->GetDevice(__uuidof(ID3D11Device), (void**)&g_device))) return;
    g_device->GetImmediateContext(&g_ctx);

    DXGI_SWAP_CHAIN_DESC desc; sc->GetDesc(&desc);
    g_wnd = desc.OutputWindow;

    // render target
    ID3D11Texture2D* bb = nullptr;
    sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&bb);
    if (bb) { g_device->CreateRenderTargetView(bb, nullptr, &g_rtv); bb->Release(); }

    ImGui::CreateContext();
    ImGui_ImplWin32_Init(g_wnd);
    ImGui_ImplDX11_Init(g_device, g_ctx);
    g_orig_wndproc = (WNDPROC)SetWindowLongPtrW(g_wnd, GWLP_WNDPROC, (LONG_PTR)wndproc);
    g_imgui_init = true;
}

static HRESULT WINAPI hk_resize(IDXGISwapChain* sc, UINT a, UINT b, UINT c, DXGI_FORMAT d, UINT e) {
    if (g_rtv) { g_rtv->Release(); g_rtv = nullptr; }
    HRESULT r = g_orig_resize(sc, a, b, c, d, e);
    if (g_imgui_init) {
        ID3D11Texture2D* bb = nullptr;
        if (SUCCEEDED(sc->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&bb)) && bb) {
            g_device->CreateRenderTargetView(bb, nullptr, &g_rtv);
            bb->Release();
        }
    }
    return r;
}

static HRESULT WINAPI hk_present(IDXGISwapChain* sc, UINT sync, UINT flags) {
    init_imgui(sc);

    // feature tick
    auto enemies = client::enemies();
    features::aimbot_tick(enemies);
    features::triggerbot_tick(enemies);

    if (g_imgui_init && g_rtv) {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        features::esp_draw(enemies, dl);
        features::radar_draw(enemies, dl);
        ui::menu_draw();

        ImGui::Render();
        g_ctx->OMSetRenderTargets(1, &g_rtv, nullptr);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

    return g_orig_present(sc, sync, flags);
}

bool init() {
    if (MH_Initialize() != MH_OK) return false;

    if (!get_present_target()) return false;
    MH_CreateHook(g_present_addr, &hk_present, reinterpret_cast<void**>(&g_orig_present));
    MH_CreateHook(g_resize_addr,  &hk_resize,  reinterpret_cast<void**>(&g_orig_resize));
    MH_EnableHook(MH_ALL_HOOKS);
    return true;
}

void shutdown() {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
    if (g_imgui_init) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        if (g_wnd && g_orig_wndproc)
            SetWindowLongPtrW(g_wnd, GWLP_WNDPROC, (LONG_PTR)g_orig_wndproc);
        g_imgui_init = false;
    }
    if (g_rtv)    { g_rtv->Release(); g_rtv = nullptr; }
    if (g_ctx)    { g_ctx->Release(); g_ctx = nullptr; }
    if (g_device) { g_device->Release(); g_device = nullptr; }
}

} // namespace overlay