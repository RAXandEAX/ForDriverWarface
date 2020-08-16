#pragma once
#include <Windows.h>
#include <string>
#pragma warning(disable : 4005)
#include <ntstatus.h>
#pragma warning(default : 4005)


inline NTSTATUS LastNtStatus()
{
	return *(NTSTATUS*)((unsigned char*)NtCurrentTeb() + (0x598 + 0x197 * sizeof(void*)));
}
inline NTSTATUS LastNtStatus(NTSTATUS status)
{
	return *(NTSTATUS*)((unsigned char*)NtCurrentTeb() + (0x598 + 0x197 * sizeof(void*))) = status;
}
class IUtilit
{
public:
	static std::wstring  GetParent(const std::wstring& path)
	{
		if (path.empty())
			return path;

		auto idx = path.rfind(L'\\');
		if (idx == path.npos)
			idx = path.rfind(L'/');

		if (idx != path.npos)
			return path.substr(0, idx);
		else
			return path;
	}
	static std::wstring  GetExeDirectory()
	{
		wchar_t imgName[MAX_PATH] = { 0 };
		GetModuleFileNameW(NULL, imgName, MAX_PATH);
		return GetParent(imgName);
	}
	static NTSTATUS      SetPriviledge(const LPCWSTR  name)
	{
		TOKEN_PRIVILEGES Priv, PrivOld;
		DWORD cbPriv = sizeof(PrivOld);
		HANDLE hToken;

		if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, FALSE, &hToken))
		{
			if (GetLastError() != ERROR_NO_TOKEN)
				return LastNtStatus();

			if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
				return LastNtStatus();
		}

		Priv.PrivilegeCount = 1;
		Priv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		LookupPrivilegeValueW(NULL, name, &Priv.Privileges[0].Luid);

		if (!AdjustTokenPrivileges(hToken, FALSE, &Priv, sizeof(Priv), &PrivOld, &cbPriv))
		{
			CloseHandle(hToken);
			return LastNtStatus();
		}

		if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
		{
			CloseHandle(hToken);
			return LastNtStatus();
		}

		return STATUS_SUCCESS;
	}
 

};

  


