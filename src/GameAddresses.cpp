#include "pch.h"
#include "GameAddresses.h"
#include "scan.h"
#include <Windows.h>
#include <Psapi.h>

std::map<std::string, char*> GameAddresses::Addresses;

char CreateScreenRenderTexturesLookup[] = { 0x53, 0x56, 0x57, 0xE8, 0x88, 0x7A, 0xF3, 0xFF, 0x8B, 0x7C, 0x24, 0x20, 0x85, 0xFF, 0x8B, 0xD8, 0x75, 0x05, 0xBF, 0x3D, 0x00, 0x00, 0x00 };
char CreateScreenRenderTexturesLookupMask[] = "xxxx????xx????xxxxxxxxx";

char CreateTextureLookup[] = { 0xFF, 0xD0, 0x89, 0x45, 0xDC, 0xC7, 0x45, 0xFC, 0xFF, 0xFF, 0xFF, 0xFF, 0xEB, 0x1B, 0xB8, 0x01, 0x00, 0x00, 0x00, 0xC3, 0x8B, 0x65, 0xE8, 0xB8, 0x05, 0x40, 0x00, 0x80 };
char CreateTextureLookupMask[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxx";

char CreateDeviceLookup[] = { 0x8B, 0x41, 0x40, 0xFF, 0xD0, 0x85, 0xC0, 0x0F, 0x8C, 0xB3, 0x02, 0x00, 0x00, 0x8B, 0x45, 0x08, 0x83, 0x45, 0x44, 0x01, 0x8B, 0x08, 0x8D, 0x94, 0x24, 0x50, 0x01 };
char CreateDeviceLookupMask[] = "xxxxxxxxx????xxxxxxxxxxxxxx";

bool GameAddresses::RegisterAddress(char* name, char* address) {
	if (address != nullptr) {
		printf("GameAddresses: Registering %s pointing to %p\n", name, address);
		Addresses[name] = address;
		return true;
	}
	printf("GameAddresses: Failed to find address for %s\n", name);
	return false;
}

bool GameAddresses::Initialize() {
	HMODULE module = GetModuleHandleA(NULL);
	char* modBase = (char*)module;
	HANDLE proc = GetCurrentProcess();
	MODULEINFO modInfo;
	GetModuleInformation(proc, module, &modInfo, sizeof(MODULEINFO));
	int size = modInfo.SizeOfImage;

	if (!RegisterAddress("CreateScreenRenderTextures", ScanInternal(CreateScreenRenderTexturesLookup, CreateScreenRenderTexturesLookupMask, modBase, size))) return false;
	if (!RegisterAddress("CreateTexture", ScanInternal(CreateTextureLookup, CreateTextureLookupMask, modBase, size))) return false;
	if (!RegisterAddress("CreateDevice", ScanInternal(CreateDeviceLookup, CreateDeviceLookupMask, modBase, size))) return false;

	return true;
}