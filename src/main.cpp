// main.cpp — entry point, init thread, cleanup
#include <windows.h>
#include <thread>
#include "core/crypt.h"
#include "core/offsets.h"
#include "core/schema.h"
#include "sdk/client.h"
#include "ui/overlay.h"
#include "config/config.h"

static HMODULE g_self = nullptr;
static HMODULE g_client = nullptr;
static std::atomic<bool> g_running{ false };
static std::thread g_init_thread;

static void init_thread() {
    // wait for client.dll to load
    while (!(g_client = GetModuleHandleW(ENC_W(L"client.dll")))) {
        if (!g_running) return;
        Sleep(200);
    }

    // resolve offsets (pattern scan + schema, with fallbacks)
    if (!off::init(g_client)) {
        // per spec: log which offset failed, degrade, do not crash
        // offsets that failed will be marked and skipped in features
    }

    // schema resolution for class/field offsets
    schema::init();

    // resolve interfaces (CGameEntitySystem, CEngineClient, etc.)
    client::init();

    // install present hook + imgui
    overlay::init();

    // load config
    config::load(ENC("cs2ext.cfg"));
}

static void shutdown() {
    g_running = false;
    overlay::shutdown();
    config::save(ENC("cs2ext.cfg"));
    if (g_init_thread.joinable()) g_init_thread.join();
}

BOOL APIENTRY DllMain(HMODULE hmod, DWORD reason, LPVOID) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        g_self = hmod;
        DisableThreadLibraryCalls(hmod);
        g_running = true;
        g_init_thread = std::thread(init_thread);
        break;
    case DLL_PROCESS_DETACH:
        shutdown();
        break;
    }
    return TRUE;
}