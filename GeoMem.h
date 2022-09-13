#pragma once
#include <Windows.h>
#include <vector>
#include <TlHelp32.h>

class GeoMem
{
private:
	HANDLE m_handle;
	DWORD m_procId;
	const wchar_t* m_procName;

public:
	GeoMem();
	GeoMem(const wchar_t* procName);
	~GeoMem();

	HANDLE GetHandle();
	DWORD GetProcId();
	const wchar_t* GetProcName();

	template <typename typeClass>
	typeClass ReadEx(uintptr_t address) {
		typeClass ret;
		ReadProcessMemory(this->m_handle, (LPCVOID)address, &ret, sizeof(ret), NULL);
		return ret;
	}

	template <typename typeClass>
	bool WriteEx(uintptr_t address, typeClass value) {
		return WriteProcessMemory(this->m_handle, (LPVOID)address, &value, sizeof(value), NULL);
	}

	DWORD GetProcIdByName(const wchar_t* procName);
	uintptr_t GetModuleAddress(const wchar_t* modName);
	uintptr_t FindAddresMultiPointer(uintptr_t address, std::vector<unsigned int> offsets);

	MODULEENTRY32 GetModuleByName(const wchar_t* moduleName);

	void* PatternScan(char* base, size_t size, const char* pattern, const char* mask);
	void* PatternScanEx(uintptr_t begin, uintptr_t end, const char* pattern, const char* mask);
	void* PatternScanExModule(const wchar_t* moduleName, const char* pattern, const char* mask);

	void PlaceJMP(BYTE* Address, DWORD jumpTo, DWORD length = 5);
};
