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

	template <class typeClass>
	typeClass ReadEx(uintptr_t address);

	template <class typeClass>
	bool WriteEx(uintptr_t address, typeClass value);

	DWORD GetProcIdByName(const wchar_t* procName);
	uintptr_t GetModuleAddress(const wchar_t* modName);
	uintptr_t FindAddresMultiPointer(uintptr_t address, std::vector<unsigned int> offsets);
};
