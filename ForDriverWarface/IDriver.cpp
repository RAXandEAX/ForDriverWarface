#include "IDriver.h"

 

IDriver::IDriver()
{
	IUtilit::SetPriviledge(L"SeDebugPrivilege");
	IUtilit::SetPriviledge(L"SeLoadDriverPrivilege");
	HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");

	pNtLoadDriver = (NtLoadDriverFn)GetProcAddress(ntdll, "NtLoadDriver");
	pNtUnloadDriver = (NtUnloadDriverFn)GetProcAddress(ntdll, "NtUnloadDriver");
	pRtlInitUnicodeString = (RtlInitUnicodeStringFn)GetProcAddress(ntdll, "RtlInitUnicodeString");
}


NTSTATUS IDriver::DbgTestMsg(){
	if (hDriver == INVALID_HANDLE_VALUE)
		return STATUS_DEVICE_DOES_NOT_EXIST;
	DWORD bytes = 0;

	if (!DeviceIoControl(hDriver, IOCTL_PRINT_DEBUG_MESS, NULL, 0, nullptr, 0, &bytes, NULL)){
		printf_s("Error in IOCTL_PRINT_DEBUG_MESS!");
		return LastNtStatus();
	}
	return STATUS_SUCCESS;
}

NTSTATUS IDriver::Hide(const char * name) {
	if (hDriver == INVALID_HANDLE_VALUE)
		return STATUS_DEVICE_DOES_NOT_EXIST;
	DWORD bytes = 0;

	if (!DeviceIoControl(hDriver, IRP_HIDE_PROC, &name, 0, nullptr, 0, &bytes, NULL)) {
		return LastNtStatus();
	}
	return STATUS_SUCCESS;
}
NTSTATUS IDriver::ReadMem(DWORD pid, uint64_t base, uint64_t size, PVOID buffer)
{
	if (hDriver == INVALID_HANDLE_VALUE)
		return STATUS_DEVICE_DOES_NOT_EXIST;


	DWORD bytes = 0;
	MEMORY copyMem = { 0 };

	copyMem.pId = pid;
	copyMem.pAdress = base;
	copyMem.pBuffer = (ULONGLONG)buffer;
	copyMem.iSize = size;

	if (!DeviceIoControl(hDriver, IOCTL_READ_MEMORY, &copyMem, sizeof(copyMem), nullptr, 0, &bytes, NULL))
		return LastNtStatus();

	return STATUS_SUCCESS;
}

NTSTATUS IDriver::WriteMem(DWORD pid, uint64_t base, uint64_t size, PVOID buffer)
{
	if (hDriver == INVALID_HANDLE_VALUE)
		return STATUS_DEVICE_DOES_NOT_EXIST;
	DWORD bytes = 0;
	MEMORY copyMem = { 0 };

	copyMem.pId = pid;
	copyMem.pAdress = base;
	copyMem.pBuffer = (ULONGLONG)buffer;
	copyMem.iSize = size;

	if (!DeviceIoControl(hDriver, IOCTL_WRITE_MEMORY, &copyMem, sizeof(copyMem), nullptr, 0, &bytes, NULL))
		return LastNtStatus();

	return STATUS_SUCCESS;
}

NTSTATUS IDriver::Loaded(const std::wstring& path)
{
	if (hDriver != INVALID_HANDLE_VALUE)
		return STATUS_SUCCESS;

	hDriver = CreateFileW(DVR_DEVICE_FILE, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (hDriver != INVALID_HANDLE_VALUE)
		return loadStatus = STATUS_SUCCESS;

	return Reload(path);
}
NTSTATUS IDriver::Reload(std::wstring path)
{
	NTSTATUS status = STATUS_SUCCESS;

	Unload();
	status = loadStatus = LoadDriver(DVR_DEVICE_NAME, path);
	if (!NT_SUCCESS(status))
	{
		return LastNtStatus(status);
	}

	hDriver = CreateFileW(DVR_DEVICE_FILE, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (hDriver == INVALID_HANDLE_VALUE)
	{
		status = LastNtStatus();
		return status;
	}
	return status;
}
NTSTATUS IDriver::LoadDriver(const std::wstring& svcName, const std::wstring& path)
{
	UNICODE_STRING Ustr;

	if (!path.empty() && PrepareDriverRegEntry(svcName, path) != 0)
		return LastNtStatus();

	std::wstring regPath = L"\\registry\\machine\\SYSTEM\\CurrentControlSet\\Services\\" + svcName;
	pRtlInitUnicodeString(&Ustr, regPath.c_str());

	return pNtLoadDriver(&Ustr);
}
NTSTATUS IDriver::UnloadDriver(const std::wstring& svcName)
{
	UNICODE_STRING Ustr = { 0 };

	std::wstring regPath = L"\\registry\\machine\\SYSTEM\\CurrentControlSet\\Services\\" + svcName;
	pRtlInitUnicodeString(&Ustr, regPath.c_str());

	NTSTATUS status = pNtUnloadDriver(&Ustr);
	RegDeleteTreeW(HKEY_LOCAL_MACHINE, (L"SYSTEM\\CurrentControlSet\\Services\\" + svcName).c_str());
	return status;
}
LSTATUS  IDriver::PrepareDriverRegEntry(const std::wstring& svcName, const std::wstring& path)
{
	HKEY key1,
		key2;
	DWORD dwType = 1;
	LSTATUS status = 0;
	WCHAR wszLocalPath[MAX_PATH] = { 0 };

	swprintf_s(wszLocalPath, MAX_PATH, L"\\??\\%s", path.c_str());

	status = RegOpenKeyW(HKEY_LOCAL_MACHINE, L"system\\CurrentControlSet\\Services", &key1);
	if (status)
		return status;

	status = RegCreateKeyW(key1, svcName.c_str(), &key2);
	if (status)
	{
		RegCloseKey(key1);
		return status;
	}

	status = RegSetValueExW(key2, L"ImagePath", 0, REG_SZ, reinterpret_cast<const BYTE*>(wszLocalPath), static_cast<DWORD>(sizeof(WCHAR)* (wcslen(wszLocalPath) + 1)));

	if (status)
	{
		RegCloseKey(key2);
		RegCloseKey(key1);
		return status;
	}

	status = RegSetValueExW(key2, L"Type", 0, REG_DWORD, reinterpret_cast<const BYTE*>(&dwType), sizeof(dwType));
	if (status)
	{
		RegCloseKey(key2);
		RegCloseKey(key1);
		return status;
	}

	RegCloseKey(key2);
	RegCloseKey(key1);

	return status;
}
NTSTATUS IDriver::Unload()
{
	if (hDriver != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hDriver);
		hDriver = INVALID_HANDLE_VALUE;
	}
	return UnloadDriver(DVR_DEVICE_NAME);
}
IDriver::~IDriver()
{
	Unload();
}

IDriver& IDriver::Singleton()
{
	static IDriver pObject;
	return pObject;
}