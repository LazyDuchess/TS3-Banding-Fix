#include "pch.h"
#include "Core.h"
#include "iostream"
#include "MinHook.h"
#include "GameAddresses.h"
#include "d3d9.h"

typedef bool(__stdcall* CREATESCREENRENDERTEXTURES)(void*, void*, void*, void*, void*, void*);
typedef IDirect3D9*(__stdcall* DIRECT3DCREATE9)(UINT);
typedef HRESULT(__stdcall* IDIRECT3D9CREATEDEVICE)(IDirect3D9*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice9**);
typedef HRESULT(__stdcall* IDIRECT3DDEVICE9CREATETEXTURE)(IDirect3DDevice9*, UINT, UINT, UINT, DWORD, D3DFORMAT, D3DPOOL, IDirect3DTexture9**, HANDLE*);

IDIRECT3DDEVICE9CREATETEXTURE fpIDirect3DDevice9CreateTexture = NULL;
DIRECT3DCREATE9 fpDirect3DCreate9 = NULL;
IDIRECT3D9CREATEDEVICE fpDirect3D9CreateDevice = NULL;
CREATESCREENRENDERTEXTURES fpCreateScreenRenderTextures = NULL;

bool textureHighDepth = false;

bool __stdcall DetourCreateScreenRenderTextures(void* unk1, void* unk2, void* unk3, void* unk4, void* unk5, void* unk6) {
	textureHighDepth = true;
	bool res = fpCreateScreenRenderTextures(unk1, unk2, unk3, unk4, unk5, unk6);
	textureHighDepth = false;
	return res;
}

// CREATETEXTURE FOR SCREEN SEEMS TO BE CALLED AT 00956FB6 IN TS3.EXE

HRESULT __stdcall DetourIDirect3DDevice9CreateTexture(IDirect3DDevice9* me, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) {
	if (textureHighDepth)
		Format = D3DFMT_A16B16G16R16F;
	return fpIDirect3DDevice9CreateTexture(me, Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT __stdcall DetourIDirect3D9CreateDevice(IDirect3D9* me, UINT adapter, D3DDEVTYPE deviceType, HWND hFocusWindow, DWORD behaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
	pPresentationParameters->BackBufferFormat = D3DFMT_A2R10G10B10;

	HRESULT res = fpDirect3D9CreateDevice(me, adapter, deviceType, hFocusWindow, behaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	void** vtable = *reinterpret_cast<void***>(*ppReturnedDeviceInterface);

	if (MH_CreateHook(vtable[23], &DetourIDirect3DDevice9CreateTexture,
		reinterpret_cast<LPVOID*>(&fpIDirect3DDevice9CreateTexture)) != MH_OK)
	{
		return res;
	}

	if (MH_EnableHook(vtable[23]) != MH_OK)
	{
		return res;
	}

	return res;
}

IDirect3D9* __stdcall DetourDirect3DCreate9(UINT sdkVersion) {
	IDirect3D9* device = fpDirect3DCreate9(sdkVersion);

	void** vtable = *(void***)device;

	if (MH_CreateHook(vtable[16], &DetourIDirect3D9CreateDevice,
		reinterpret_cast<LPVOID*>(&fpDirect3D9CreateDevice)) != MH_OK)
	{
		return device;
	}

	if (MH_EnableHook(vtable[16]) != MH_OK)
	{
		return device;
	}

	return device;
}

Core* Core::_instance = nullptr;

Core* Core::GetInstance() {
	return _instance;
}

bool Core::Create() {
	_instance = new Core();
	return _instance->Initialize();
}

bool Core::Initialize() {
	printf("Core initializing\n");

	if (!GameAddresses::Initialize())
		return false;

	if (MH_Initialize() != MH_OK) {
		return false;
	}

	HMODULE hD3D9 = GetModuleHandleA("d3d9.dll");
	if (!hD3D9) {
		return false;
	}

	void* pDirect3DCreate9 = GetProcAddress(hD3D9, "Direct3DCreate9");
	if (!pDirect3DCreate9) {
		return false;
	}

	if (MH_CreateHook(pDirect3DCreate9, &DetourDirect3DCreate9,
		reinterpret_cast<LPVOID*>(&fpDirect3DCreate9)) != MH_OK)
	{
		return false;
	}

	if (MH_EnableHook(pDirect3DCreate9) != MH_OK)
	{
		return false;
	}

	if (MH_CreateHook(GameAddresses::Addresses["CreateScreenRenderTextures"], &DetourCreateScreenRenderTextures,
		reinterpret_cast<LPVOID*>(&fpCreateScreenRenderTextures)) != MH_OK)
	{
		return false;
	}

	if (MH_EnableHook(GameAddresses::Addresses["CreateScreenRenderTextures"]) != MH_OK)
	{
		return false;
	}

	return true;
}