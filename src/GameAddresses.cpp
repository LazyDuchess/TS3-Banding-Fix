#include "pch.h"
#include "GameAddresses.h"
#include "scan.h"
#include <Windows.h>
#include <Psapi.h>

std::map<std::string, char*> GameAddresses::Addresses;

char CreateScreenRenderTexturesLookup[] = { 0x53, 0x56, 0x57, 0xE8, 0x88, 0x7A, 0xF3, 0xFF, 0x8B, 0x7C, 0x24, 0x20, 0x85, 0xFF, 0x8B, 0xD8, 0x75, 0x05, 0xBF, 0x3D, 0x00, 0x00, 0x00 };
char CreateScreenRenderTexturesLookupMask[] = "xxxx????xx????xxxxxxxxx";

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

	return true;
}