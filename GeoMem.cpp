#include "GeoMem.h"

GeoMem::GeoMem() {
	this->m_handle = NULL;
}

/// <summary>
/// Init handle and procId, you can get It with Getter.
/// </summary>
/// <param name="procName">use L macro</param>
GeoMem::GeoMem(const wchar_t* procName) {
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

template <class typeClass>
typeClass GeoMem::ReadEx(uintptr_t address) {
	typeClass ret;
	ReadProcessMemory(this->m_handle, (LPCVOID)address, &ret, sizeof(ret), NULL);
	return ret;
}

template <class typeClass>
bool GeoMem::WriteEx(uintptr_t address, typeClass value) {
	try
	{
		WriteProcessMemory(this->m_handle, (LPVOID)address, &value, sizeof(value), NULL);
		return true;
	}
	catch (const std::exception&)
	{
		return false;
	}
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