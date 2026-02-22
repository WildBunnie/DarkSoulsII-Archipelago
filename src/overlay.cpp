#include "overlay.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "kiero.h"
#include "imgui.h"
#include "imgui_impl_win32.h"

// TODO dont duplicate this from ds2.h
#ifdef _M_X64
    #define DS2_64 1
    #define DS2_32 0
#elif defined(_M_IX86)
    #define DS2_64 0
    #define DS2_32 1
#endif

#if DS2_64
    #include <d3d11.h>
    #include "imgui_impl_dx11.h"
#elif DS2_32
    #include <d3d9.h>
    #include "imgui_impl_dx9.h"
#endif

overlay_render_fn render_overlay = 0;

static WNDPROC oWndProc = NULL;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK hkWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);

    ImGuiIO& io = ImGui::GetIO();

    // block game input when imgui wants the mouse or keyboard
    switch (uMsg)
    {
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_MOUSEWHEEL:
            if (io.WantCaptureMouse) return true;
            break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_CHAR:
            if (io.WantCaptureKeyboard || io.WantTextInput) return true;
            break;
    }

    return ::CallWindowProc(oWndProc, hwnd, uMsg, wParam, lParam);
}

#if DS2_64

typedef long(__stdcall* Present)(IDXGISwapChain*, UINT, UINT);
static Present oPresent = 0;

typedef HRESULT(__stdcall* ResizeBuffers)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);
static ResizeBuffers oResizeBuffers = 0;

static ID3D11Device* gDevice = nullptr;
static ID3D11DeviceContext* gContext = nullptr;
static ID3D11RenderTargetView* gMainRenderTargetView = nullptr;

long __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	static bool init = false;

    if (!init)
    {
        DXGI_SWAP_CHAIN_DESC desc;
        pSwapChain->GetDesc(&desc);

        pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&gDevice);
        gDevice->GetImmediateContext(&gContext);

        ID3D11Texture2D* pBackBuffer = nullptr;
        pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
        gDevice->CreateRenderTargetView(pBackBuffer, NULL, &gMainRenderTargetView);
        pBackBuffer->Release();

        oWndProc = (WNDPROC)SetWindowLongPtr(
            desc.OutputWindow,
            GWLP_WNDPROC,
            reinterpret_cast<LONG_PTR>(hkWindowProc)
        );

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(desc.OutputWindow);
        ImGui_ImplDX11_Init(gDevice, gContext);

        init = true;
    }

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	render_overlay();

	ImGui::EndFrame();
	ImGui::Render();
	gContext->OMSetRenderTargets(1, &gMainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	return oPresent(pSwapChain, SyncInterval, Flags);
}

HRESULT __stdcall hkResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    if (gMainRenderTargetView) {
        gMainRenderTargetView->Release();
        gMainRenderTargetView = nullptr;
    }

    ImGui_ImplDX11_InvalidateDeviceObjects();

    HRESULT hr = oResizeBuffers(
        pSwapChain,
        BufferCount,
        Width,
        Height,
        NewFormat,
        SwapChainFlags
    );

    ID3D11Texture2D* pBackBuffer = nullptr;
    pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
    gDevice->CreateRenderTargetView(pBackBuffer, NULL, &gMainRenderTargetView);
    pBackBuffer->Release();

    ImGui_ImplDX11_CreateDeviceObjects();

    return hr;
}

#elif DS2_32

typedef long(__stdcall* Reset)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
static Reset oReset = 0;

typedef long(__stdcall* EndScene)(LPDIRECT3DDEVICE9);
static EndScene oEndScene = 0;

long __stdcall hkReset(LPDIRECT3DDEVICE9 pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	long result = oReset(pDevice, pPresentationParameters);
	ImGui_ImplDX9_CreateDeviceObjects();

	return result;
}

long __stdcall hkEndScene(LPDIRECT3DDEVICE9 pDevice)
{
	static bool init = false;

	if (!init) {
		D3DDEVICE_CREATION_PARAMETERS params;
		pDevice->GetCreationParameters(&params);

		oWndProc = (WNDPROC)::SetWindowLongPtr(
		  (HWND)params.hFocusWindow,
		  GWLP_WNDPROC,
		  (LONG)hkWindowProc
        );

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(params.hFocusWindow);
		ImGui_ImplDX9_Init(pDevice);

		init = true;
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	render_overlay();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return oEndScene(pDevice);
}

#endif // DS2_32

int init_overlay(overlay_render_fn fn)
{
    if (!fn) return 0;
    render_overlay = fn;

    if (kiero::init(kiero::RenderType::Auto) != kiero::Status::Success) return 0;

    // TODO check the error properly instead of always returning 0
#if DS2_32
    if (kiero::getRenderType() == kiero::RenderType::D3D9) {
        if (kiero::bind(16, (void**)&oReset, hkReset) != kiero::Status::Success) return 0;
        if (kiero::bind(42, (void**)&oEndScene, hkEndScene) != kiero::Status::Success) return 0;
    }
#elif DS2_64
    if (kiero::getRenderType() == kiero::RenderType::D3D11) {
        if (kiero::bind(8, (void**)&oPresent, hkPresent) != kiero::Status::Success) return 0;
        if (kiero::bind(13, (void**)&oResizeBuffers, hkResizeBuffers) != kiero::Status::Success) return 0;
    }
#endif
    else {
        assert(0 && "invalid kiero render type");
    }

    return 1;
}

void shutdown_overlay()
{
    kiero::shutdown();
}