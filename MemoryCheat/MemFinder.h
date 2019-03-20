#pragma once
#include <Windows.h>
#include <list>
#include <algorithm>

class CMemFinder
{
public:
	CMemFinder()
	{
		//���һ���ڴ�Ĵ�С
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		m_dwPageSize = info.dwPageSize;
	}
	~CMemFinder()
	{
		//�رվ��
		SafeCloseHandle();
	}
	//�򿪽���
	bool openProcess(DWORD dwProcessId)
	{
		//���������Ǵ򿪵� �رս���
		if (IsValidHandle())
		{
			SafeCloseHandle();
		}
		//��
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
	/*����ɨ��
	*1.�򿪽���
	*2.ɨ�跶Χ�ڵĽ���
	*3.��Ŀ��ֵ���бȽ�
	*/
	template< typename T>
	bool FindFirst(DWORD dwProcessId, DWORD dwBegin, DWORD dwEnd, T value)
	{
		m_arList.clear();
		return _FindFirst();
	}
private:
	//�رս��̾��
	void SafeCloseHandle()
	{
		CloseHandle(m_hProcess);
		m_hProcess = INVALID_HANDLE_VALUE;
	}
	//�����Ƿ�Ϸ�
	bool IsValidHandle()
	{
		return (m_hProcess&&INVALID_HANDLE_VALUE != m_hProcess);
	}
	//�����Ĳ��Һ���
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
		//Ŀ�곤��
		const size_t len = sizeof(value);
		const void *pValue = &value;
	}
	DWORD m_dwPageSize;
	//Ŀ����̾��
	HANDLE m_hProcess{ INVALID_HANDLE_VALUE };
	//�����Ľ��
	std::list<DWORD>m_arlist;
};