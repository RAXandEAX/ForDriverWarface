#include "cProcess.h"
#include <tlhelp32.h>
#include <iostream>

cProcess::cProcess():
	m_dwProcessId(NULL)
{
}
bool	   cProcess::Attach(const std::string& szProcessName) {
	if (szProcessName.empty())
		return false;

	this->m_dwProcessId = GetProcessIdByName(szProcessName);
	if (!m_dwProcessId)
		return false;

	return true;
}
DWORD      cProcess::GetProcessIdByName(const std::string& szProcessName)
{
	HANDLE hSnap;
	DWORD pId = 0;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	try {
		hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (!hSnap) {
			throw std::exception("Could not create process snapshot");
		}

		if (!Process32First(hSnap, &pe32)) {
			throw std::exception("Enumerating processes failed");
		}

		do {
			if (std::string(pe32.szExeFile) == szProcessName) {
				pId = pe32.th32ProcessID;
				break;
			}
		} while (Process32Next(hSnap, &pe32));

		CloseHandle(hSnap);

		if(pId != 0)
			std::cout << "  = > Process[" << szProcessName << "]: " << pId << std::endl;

		return pId;
	}
	catch (std::exception e) {
		CloseHandle(hSnap);
		throw;
	}
}

cProcess::~cProcess()
{
	m_dwProcessId = NULL;
}
cProcess& cProcess::Singleton()
{
	static cProcess pObject;
	return pObject;
}