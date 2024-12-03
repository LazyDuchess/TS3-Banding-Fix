#include "pch.h"
#include "Core.h"
#include "iostream"
#include "MinHook.h"
#include "GameAddresses.h"
#include "d3d9.h"
#include "scan.h"

bool textureHighDepth = false;
D3DFORMAT textureFormat = D3DFMT_A16B16G16R16F;
D3DFORMAT backBufferFormat = D3DFMT_A2R10G10B10;

void __stdcall FixPresentParameters(D3DPRESENT_PARAMETERS* pPresentParameters) {
	pPresentParameters->BackBufferFormat = backBufferFormat;
	pPresentParameters->Flags
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

	DetourCreateTextureReturn = GameAddresses::Addresses["CreateTexture"] + 5;
	MakeJMP((BYTE*)GameAddresses::Addresses["CreateTexture"], (DWORD)DetourCreateTexture, 5);

	DetourCreateDeviceReturn = GameAddresses::Addresses["CreateDevice"] + 5;
	MakeJMP((BYTE*)GameAddresses::Addresses["CreateDevice"], (DWORD)DetourCreateDevice, 5);

	if (MH_Initialize() != MH_OK) {
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