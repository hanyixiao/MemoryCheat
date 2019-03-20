#pragma once
#include <Windows.h>
#include <list>
#include <algorithm>

class CMemFinder
{
public:
	CMemFinder()
	{
		//获得一次内存的大小
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		m_dwPageSize = info.dwPageSize;
	}
	~CMemFinder()
	{
		//关闭句柄
		SafeCloseHandle();
	}
	//打开进程
	bool openProcess(DWORD dwProcessId)
	{
		//若果进程是打开的 关闭进成
		if (IsValidHandle())
		{
			SafeCloseHandle();
		}
		//打开
		m_hProcess = ::OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE |
			PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD |
			PROCESS_QUERY_INFORMATION,
			FALSE, dwProcessId);
		if (IsValidHandle()) {
			return true;
		}
		else {
			SafeCloseHandle();
			return false;
		}
	}
	/*初次扫描
	*1.打开进程
	*2.扫描范围内的进程
	*3.与目标值进行比较
	*/
	template< typename T>
	bool FindFirst(DWORD dwProcessId, DWORD dwBegin, DWORD dwEnd, T value)
	{
		m_arList.clear();
		return _FindFirst();
	}
private:
	//关闭进程句柄
	void SafeCloseHandle()
	{
		CloseHandle(m_hProcess);
		m_hProcess = INVALID_HANDLE_VALUE;
	}
	//进程是否合法
	bool IsValidHandle()
	{
		return (m_hProcess&&INVALID_HANDLE_VALUE != m_hProcess);
	}
	//真正的查找函数
	template <typename T>
	bool _FindFirst(DWORD dwProcessId, DWORD dwBegin, DWORD dwEnd, T value)
	{
		HANDLE hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | 
			PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
			false, dwProcessId);
		m_hProcess = hProcess;
		if (m_hProcess == NULL) {
			return false;
		}
		//目标长度
		const size_t len = sizeof(value);
		const void *pValue = &value;
	}
	DWORD m_dwPageSize;
	//目标进程句柄
	HANDLE m_hProcess{ INVALID_HANDLE_VALUE };
	//搜索的结果
	std::list<DWORD>m_arlist;
};