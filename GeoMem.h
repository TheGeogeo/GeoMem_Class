#pragma once
#include <Windows.h>
#include <vector>
#include <TlHelp32.h>

class GeoMem
{
private:
	HANDLE m_handle;
	DWORD m_procId;

public:
	GeoMem();
	GeoMem(const wchar_t* procName);
	~GeoMem();

	HANDLE GetHandle();
	DWORD GetProcId();

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
};
