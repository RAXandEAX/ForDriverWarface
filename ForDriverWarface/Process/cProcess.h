#define _CRT_SECURE_NO_WARNINGS
#include "..\IDriver.h"
class cProcess
{
public:
	cProcess();
	virtual ~cProcess();

	bool Attach(const std::string&);
	bool IsEmpry()  const { return  (this->m_dwProcessId == 0); }

	DWORD GetProcId() const {	return this->m_dwProcessId;}

	static cProcess&  Singleton();

	template <class cData>
	cData Read(uintptr_t dwAddress)
	{
		cData cRead;
		Driver().ReadMem(this->m_dwProcessId, (uintptr_t)dwAddress, sizeof(cData), &cRead);
		return cRead;
	}
	template <class cData>
	void Write(DWORD dwAddress, cData write) {
		Driver().WriteMem(this->m_dwProcessId, (uint64_t)dwAddress, sizeof(cData), &write);
	}
	bool ReadMemory(uintptr_t offset, size_t size, PVOID* bufer)
	{
		PVOID readBuf = NULL;
		if (!NT_SUCCESS(Driver().ReadMem(this->m_dwProcessId, (uint64_t)offset, size, &readBuf)) && readBuf == NULL)
			return false;

		*bufer = readBuf;
		return true;
	}

private:
	DWORD m_dwProcessId;
	DWORD GetProcessIdByName(const std::string&);
private:
	cProcess(const cProcess&) = delete;
	cProcess& operator = (const cProcess&) = delete;
};
inline cProcess& Process() 
{
	return cProcess::Singleton();
}


