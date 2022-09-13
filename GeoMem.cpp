#include "GeoMem.h"

/// <summary>
/// If you init with basic instance use : GetProcIdByName
/// </summary>
GeoMem::GeoMem() {
	this->m_handle = NULL;
	this->m_procId = NULL;
}

/// <summary>
/// Init handle and procId, you can get It with Getter.
/// </summary>
/// <param name="procName">use L macro</param>
GeoMem::GeoMem(const wchar_t* procName) {
	this->m_procName = procName;
	GeoMem::GetProcIdByName(procName);
}

GeoMem::~GeoMem() {
	CloseHandle(this->m_handle);
}

HANDLE GeoMem::GetHandle() {
	return this->m_handle;
}

DWORD GeoMem::GetProcId() {
	return this->m_procId;
}

const wchar_t* GeoMem::GetProcName() {
	return this->m_procName;
}

DWORD GeoMem::GetProcIdByName(const wchar_t* procName) {
	HANDLE hProcessId = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);

	do
	{
		if (!_wcsicmp(pEntry.szExeFile, procName))
		{
			this->m_procId = pEntry.th32ProcessID;
			CloseHandle(hProcessId);
			this->m_handle = OpenProcess(PROCESS_ALL_ACCESS, false, this->m_procId);
		}
	} while (Process32Next(hProcessId, &pEntry));
	return this->m_procId;
}

uintptr_t GeoMem::GetModuleAddress(const wchar_t* modName) {
	HANDLE hModule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, this->m_procId);
	MODULEENTRY32 mEntry;
	mEntry.dwSize = sizeof(mEntry);

	do
	{
		if (!_wcsicmp(mEntry.szModule, modName))
		{
			CloseHandle(hModule);
			return (DWORD)mEntry.hModule;
		}
	} while (Module32Next(hModule, &mEntry));
	return 0;
}

uintptr_t GeoMem::FindAddresMultiPointer(uintptr_t address, std::vector<unsigned int> offsets) {
	for (unsigned int i = 0; i < offsets.size(); i++)
	{
		ReadProcessMemory(this->m_handle, (BYTE*)address, &address, sizeof(address), NULL);
		address += offsets[i];
	}
	return address;
}

void* GeoMem::PatternScan(char* base, size_t size, const char* pattern, const char* mask)
{
	size_t patternLength = strlen(mask);

	for (unsigned int i = 0; i < size - patternLength; i++)
	{
		bool found = true;
		for (unsigned int j = 0; j < patternLength; j++)
		{
			if (mask[j] != '?' && pattern[j] != *(base + i + j))
			{
				found = false;
				break;
			}
		}
		if (found)
		{
			return (void*)(base + i);
		}
	}
	return nullptr;
}

void* GeoMem::PatternScanEx(uintptr_t begin, uintptr_t end, const char* pattern, const char* mask)
{
	uintptr_t currentChunk = begin;
	SIZE_T bytesRead;

	while (currentChunk < end)
	{
		char buffer[4096];

		DWORD oldprotect;
		VirtualProtectEx(this->m_handle, (void*)currentChunk, sizeof(buffer), PAGE_EXECUTE_READWRITE, &oldprotect);
		ReadProcessMemory(this->m_handle, (void*)currentChunk, &buffer, sizeof(buffer), &bytesRead);
		VirtualProtectEx(this->m_handle, (void*)currentChunk, sizeof(buffer), oldprotect, &oldprotect);

		if (bytesRead == 0)
		{
			return nullptr;
		}

		void* internalAddress = PatternScan((char*)&buffer, bytesRead, pattern, mask);

		if (internalAddress != nullptr)
		{
			//calculate from internal to external
			uintptr_t offsetFromBuffer = (uintptr_t)internalAddress - (uintptr_t)&buffer;
			return (void*)(currentChunk + offsetFromBuffer);
		}
		else
		{
			//advance to next chunk
			currentChunk = currentChunk + bytesRead;
		}
	}
	return nullptr;
}

void* GeoMem::PatternScanExModule(const wchar_t* moduleName, const char* pattern, const char* mask)
{
	MODULEENTRY32 modEntry = GeoMem::GetModuleByName(moduleName);

	if (!modEntry.th32ModuleID)
	{
		return nullptr;
	}

	uintptr_t begin = (uintptr_t)modEntry.modBaseAddr;
	uintptr_t end = begin + modEntry.modBaseSize;
	return PatternScanEx(begin, end, pattern, mask);
}

MODULEENTRY32 GeoMem::GetModuleByName(const wchar_t* moduleName)
{
	MODULEENTRY32 modEntry = { 0 };

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, this->m_procId);

	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 curr = { 0 };

		curr.dwSize = sizeof(MODULEENTRY32);
		if (Module32First(hSnapshot, &curr))
		{
			do
			{
				if (!wcscmp(curr.szModule, moduleName))
				{
					modEntry = curr;
					break;
				}
			} while (Module32Next(hSnapshot, &curr));
		}
		CloseHandle(hSnapshot);
	}
	return modEntry;
}

void GeoMem::PlaceJMP(BYTE* Address, DWORD jumpTo, DWORD length = 5)
{
	DWORD dwOldProtect, dwBkup, dwRelAddr;

	VirtualProtect(Address, length, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	dwRelAddr = (DWORD)(jumpTo - (DWORD)Address) - 5;

	*Address = 0xE9;

	*((DWORD*)(Address + 0x1)) = dwRelAddr;

	for (DWORD x = 0x5; x < length; x++)
		*(Address + x) = 0x90;

	VirtualProtect(Address, length, dwOldProtect, &dwBkup);
}