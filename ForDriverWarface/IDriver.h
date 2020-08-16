#pragma once
#include  "IUtilit.h"
#include <winternl.h>
#include "..\DriverRead\Define.h"
 

typedef void (NTAPI* RtlInitUnicodeStringFn)(PUNICODE_STRING, PCWSTR);
typedef NTSTATUS(NTAPI* NtLoadDriverFn)(IN PUNICODE_STRING);
typedef NTSTATUS(NTAPI* NtUnloadDriverFn)(IN PUNICODE_STRING);
  
class IDriver
{
public:
	IDriver();
	~IDriver();
	static IDriver&  Singleton();

	NTSTATUS  DbgTestMsg();
	NTSTATUS Hide(const char* name);

	NTSTATUS Loaded(const std::wstring& path = IUtilit::GetExeDirectory() + L"\\drv.sys");
	NTSTATUS Reload(std::wstring path);
	NTSTATUS Unload();

	NTSTATUS WriteMem(DWORD pid, uint64_t base, uint64_t size, PVOID buffer);
	NTSTATUS ReadMem(DWORD pid, uint64_t base, uint64_t size, PVOID buffer);
	
	inline bool     IsLoaded()  const { return hDriver != INVALID_HANDLE_VALUE; }
	inline NTSTATUS GetStatus() const { return loadStatus; }
private:
	LSTATUS  PrepareDriverRegEntry(const std::wstring& svcName, const std::wstring& path);
	NTSTATUS LoadDriver(const std::wstring& svcName, const std::wstring& path);
	NTSTATUS UnloadDriver(const std::wstring& svcName);
private:
	IDriver(const IDriver&) = delete;
	IDriver& operator = (const IDriver&) = delete;

	HANDLE   hDriver    = INVALID_HANDLE_VALUE;
	NTSTATUS loadStatus = STATUS_NOT_FOUND;

     RtlInitUnicodeStringFn pRtlInitUnicodeString = nullptr;
     NtLoadDriverFn pNtLoadDriver = nullptr;
     NtUnloadDriverFn pNtUnloadDriver = nullptr;
};

inline IDriver& Driver() { 
	return IDriver::Singleton(); 
}

#define Log(x, ...)   printf_s(x,__VA_ARGS__)
