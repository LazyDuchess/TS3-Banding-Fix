#include "pch.h"
#include "Core.h"
#include "iostream"
#include "MinHook.h"
#include "GameAddresses.h"
#include "d3d9.h"
#include "scan.h"

bool wire = false;

enum D3DVTABLE_INDEX {
    iQueryInterface,
    iAddRef,
    iRelease,
    iTestCooperativeLevel,
    iGetAvailableTextureMem,
    iEvictManagedResources,
    iGetDirect3D,
    iGetDeviceCaps,
    iGetDisplayMode,
    iGetCreationParameters,
    iSetCursorProperties,
    iSetCursorPosition,
    iShowCursor,
    iCreateAdditionalSwapChain,
    iGetSwapChain,
    iGetNumberOfSwapChains,
    iReset,
    iPresent,
    iGetBackBuffer,
    iGetRasterStatus,
    iSetDialogBoxMode,
    iSetGammaRamp,
    iGetGammaRamp,
    iCreateTexture,
    iCreateVolumeTexture,
    iCreateCubeTexture,
    iCreateVertexBuffer,
    iCreateIndexBuffer,
    iCreateRenderTarget,
    iCreateDepthStencilSurface,
    iUpdateSurface,
    iUpdateTexture,
    iGetRenderTargetData,
    iGetFrontBufferData,
    iStretchRect,
    iColorFill,
    iCreateOffscreenPlainSurface,
    iSetRenderTarget,
    iGetRenderTarget,
    iSetDepthStencilSurface,
    iGetDepthStencilSurface,
    iBeginScene,
    iEndScene,
    iClear,
    iSetTransform,
    iGetTransform,
    iMultiplyTransform,
    iSetViewport,
    iGetViewport,
    iSetMaterial,
    iGetMaterial,
    iSetLight,
    iGetLight,
    iLightEnable,
    iGetLightEnable,
    iSetClipPlane,
    iGetClipPlane,
    iSetRenderState,
    iGetRenderState,
    iCreateStateBlock,
    iBeginStateBlock,
    iEndStateBlock,
    iSetClipStatus,
    iGetClipStatus,
    iGetTexture,
    iSetTexture,
    iGetTextureStageState,
    iSetTextureStageState,
    iGetSamplerState,
    iSetSamplerState,
    iValidateDevice,
    iSetPaletteEntries,
    iGetPaletteEntries,
    iSetCurrentTexturePalette,
    iGetCurrentTexturePalette,
    iSetScissorRect,
    iGetScissorRect,
    iSetSoftwareVertexProcessing,
    iGetSoftwareVertexProcessing,
    iSetNPatchMode,
    iGetNPatchMode,
    iDrawPrimitive,
    iDrawIndexedPrimitive,
    iDrawPrimitiveUP,
    iDrawIndexedPrimitiveUP,
    iProcessVertices,
    iCreateVertexDeclaration,
    iSetVertexDeclaration,
    iGetVertexDeclaration,
    iSetFVF,
    iGetFVF,
    iCreateVertexShader,
    iSetVertexShader,
    iGetVertexShader,
    iSetVertexShaderConstantF,
    iGetVertexShaderConstantF,
    iSetVertexShaderConstantI,
    iGetVertexShaderConstantI,
    iSetVertexShaderConstantB,
    iGetVertexShaderConstantB,
    iSetStreamSource,
    iGetStreamSource,
    iSetStreamSourceFreq,
    iGetStreamSourceFreq,
    iSetIndices,
    iGetIndices,
    iCreatePixelShader,
    iSetPixelShader,
    iGetPixelShader,
    iSetPixelShaderConstantF,
    iGetPixelShaderConstantF,
    iSetPixelShaderConstantI,
    iGetPixelShaderConstantI,
    iSetPixelShaderConstantB,
    iGetPixelShaderConstantB,
    iDrawRectPatch,
    iDrawTriPatch,
    iDeletePatch,
    iCreateQuery,
    iSetConvolutionMonoKernel,
    iComposeRects,
    iPresentEx,
    iGetGPUThreadPriority,
    iSetGPUThreadPriority,
    iWaitForVBlank,
    iCheckResourceResidency,
    iSetMaximumFrameLatency,
    iGetMaximumFrameLatency,
    iCheckDeviceState,
    iCreateRenderTargetEx,
    iCreateOffscreenPlainSurfaceEx,
    iCreateDepthStencilSurfaceEx,
    iResetEx,
    iGetDisplayModeEx
};

typedef IDirect3D9* (__stdcall* DIRECT3DCREATE9)(UINT);
typedef HRESULT(__stdcall* IDIRECT3D9CREATEDEVICE)(IDirect3D9*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice9**);
typedef HRESULT(__stdcall* IDIRECT3DDEVICE9SETRENDERSTATE)(IDirect3DDevice9*, D3DRENDERSTATETYPE, DWORD);

DIRECT3DCREATE9 fpDirect3DCreate9 = NULL;
IDIRECT3D9CREATEDEVICE fpDirect3D9CreateDevice = NULL;
IDIRECT3DDEVICE9SETRENDERSTATE fpDirect3DDevice9SetRenderState = NULL;

HRESULT __stdcall DetourIDirect3DDevice9SetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) {
    if ((GetAsyncKeyState(VK_SUBTRACT) & 0x8001) == 0x8001) {
        wire = !wire;
    }
    if (wire)
        fpDirect3DDevice9SetRenderState(device, D3DRS_FILLMODE, D3DFILL_WIREFRAME);
    else
        fpDirect3DDevice9SetRenderState(device, D3DRS_FILLMODE, D3DFILL_SOLID);
    return fpDirect3DDevice9SetRenderState(device, State, Value);
}

HRESULT __stdcall DetourIDirect3D9CreateDevice(IDirect3D9* me, UINT adapter, D3DDEVTYPE deviceType, HWND hFocusWindow, DWORD behaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
	pPresentationParameters->BackBufferFormat = D3DFMT_A2R10G10B10;
	HRESULT res = fpDirect3D9CreateDevice(me, adapter, deviceType, hFocusWindow, behaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
	void** vtable = *reinterpret_cast<void***>(*ppReturnedDeviceInterface);
	if (MH_CreateHook(vtable[D3DVTABLE_INDEX::iSetRenderState], &DetourIDirect3DDevice9SetRenderState,
		reinterpret_cast<LPVOID*>(&fpDirect3DDevice9SetRenderState)) != MH_OK)
	{
		return res;
	}
	if (MH_EnableHook(vtable[D3DVTABLE_INDEX::iSetRenderState]) != MH_OK)
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

bool textureHighDepth = false;
D3DFORMAT textureFormat = D3DFMT_A16B16G16R16F;
D3DFORMAT backBufferFormat = D3DFMT_A2R10G10B10;

void __stdcall FixPresentParameters(D3DPRESENT_PARAMETERS* pPresentParameters) {
	pPresentParameters->BackBufferFormat = backBufferFormat;
}

char* DetourCreateDeviceReturn = NULL;

void __declspec(naked) DetourCreateDevice() {
	__asm {
		mov eax, [ecx+0x40]

		// D3DPRESENT_PARAMETERS*
		mov ecx, [esp+0x14]

		// save registers
		push edi
		push esp
		push ebp
		push eax

		push ecx
		call FixPresentParameters

		//restore registers
		pop eax
		pop ebp
		pop esp
		pop edi

		call eax
		jmp DetourCreateDeviceReturn
	}
}

char* DetourCreateTextureReturn = NULL;

void __declspec(naked) DetourCreateTexture() {
	__asm {
		mov cl, textureHighDepth
		test cl, cl
		je exitLabel
		mov ecx, textureFormat
		mov [esp+0x14], ecx
		exitLabel:
		call eax
		mov [ebp-0x24], eax
		jmp DetourCreateTextureReturn
	}
}

typedef bool(__stdcall* CREATESCREENRENDERTEXTURES)(void*, void*, void*, void*, void*, void*);

CREATESCREENRENDERTEXTURES fpCreateScreenRenderTextures = NULL;

bool __stdcall DetourCreateScreenRenderTextures(void* unk1, void* unk2, void* unk3, void* unk4, void* unk5, void* unk6) {
	textureHighDepth = true;
	bool res = fpCreateScreenRenderTextures(unk1, unk2, unk3, unk4, unk5, unk6);
	textureHighDepth = false;
	return res;
}

// CREATETEXTURE CALLED AT 00956FB6 IN TS3.EXE
// CREATEDEVICE CALLED AT 0094ed90 IN TS3.EXE

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

	DetourCreateTextureReturn = GameAddresses::Addresses["CreateTexture"] + 5;
	MakeJMP((BYTE*)GameAddresses::Addresses["CreateTexture"], (DWORD)DetourCreateTexture, 5);

	DetourCreateDeviceReturn = GameAddresses::Addresses["CreateDevice"] + 5;
	MakeJMP((BYTE*)GameAddresses::Addresses["CreateDevice"], (DWORD)DetourCreateDevice, 5);

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