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
	/*
	*��findFirst�Ļ����ϼ���Ѱ��
	*/
	template< typename T>
	bool FindNext(T value)
	{
		return _FindNext(value);
	}
	//���ڴ�
	template< typename T>
	bool Read(DWORD dwAddr, T &val)
	{
		if (::ReadProcessMemory(m_hProcess, (LPCVOID)dwAddr, &val, sizeof(val), NULL)) {
			return true;
		}
		return false;
	}
	bool Read(DWORD dwAddr, unsigned char val[], size_t len)
	{
		if (::ReadProcessMemory(m_hProcess, (LPCVOID)dwAddr, &val[0], len, NULL)) {
			return true;
		}
		return false;
	}
	//д�ڴ�
	template< typename T>
	bool Write(DWORD dwAddr, T val)
	{
		if (::WriteProcessMemory(m_hProcess, (LPVOID)dwAddr, &val, sizeof(val), NULL)) {
			return true;
		}
		return false;
	}
	bool Write(DWORD dwAddr, unsigned char code[], size_t len)
	{
		if (::WriteProcessMemory(m_hProcess, (LPVOID)dwAddr, &code[0], len, NULL)) {
			return true;
		}
		return false;
	}
	//���ûص�����
	bool RemoteCall(unsigned char code[], size_t len,
		unsigned char para[], size_t paraLen)
	{
		if (!IsValidHandle()) {

		}
		//��������ڴ�
		PVOID mFuncAddr = ::VirtualAllocEx(m_hProcess, NULL, len,
			MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (nullptr == mFuncAddr) {
			return false;
		}
		//���뺯�������ڴ�
		PVOID ParamAddr = ::VirtualAllocEx(m_hProcess, NULL, len,
			MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (nullptr == ParamAddr) {
			return false;
		}

		//д����룬�Ͳ���
		this->Write((DWORD)mFuncAddr, code, len);
		this->Write((DWORD)ParamAddr, para, paraLen);

		// ����Զ���߳�
		DWORD dwThreadId;
		HANDLE hThread = ::CreateRemoteThread(m_hProcess, NULL, 0,
			(LPTHREAD_START_ROUTINE)mFuncAddr, ParamAddr, 0, &dwThreadId);

		//�ȴ�ִ�����
		if (hThread&&hThread != INVALID_HANDLE_VALUE) {
			WaitForSingleObject(hThread,1000);
			//�ȴ�ָ���������ź�״̬��1s
		}
		

		//�ͷ��ڴ�
		VirtualFreeEx(hThread, mFuncAddr, len, MEM_RELEASE);
		VirtualFreeEx(hThread, ParamAddr, paraLen, MEM_RELEASE);
		return true;
	}
	//���ó��ε�ɨ�躯��
	virtual void SetCallbackFirst(bool(_stdcall *pGoonFirst)(void *pArgs,
		size_t nAddrCount, size_t index), void *pArgs)
	{
		m_pGoonFirst = pGoonFirst;
		m_pArgsFirst = pArgs;
	}
	//���ó���ɨ��֮��Ļص�����
	virtual void SetCallbackNext(bool(_stdcall *pGoonFirst)(void *pArgs,
		size_t nAddrCount, size_t index), void *pArgs)
	{
		m_pGoonNext = pGoonFirst;
		m_pArgsNext = pArgs;
	}
	//��ý��
	virtual const std::list<DWORD> &GetResult() const
	{
		return m_arlist;
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
	template< typename T>
	bool _FindNext(T value)
	{
		//Ŀ��ֵ�ĳ���
		const size_t len = sizeof(value);
		const void *pValue = &value;

		//�Ѿ�����ĵ�ַ����
		size_t cnt = m_arlist.size();
		size_t index = 0;
		std::list<DWORD> dwTemp;
		for (auto addr : m_arlist) {
			//����лص����������ҵ��ú�������FALSE�����˳�����
			if (this->m_pGoonNext && !this->m_pGoonNext(this->m_pArgsNext, cnt, index++))
			{
				return false;
			}
			//������Ϣ
			WaitForIdle();

			//��ȡ����
			T tReadValue;
			if (!::ReadProcessMemory(m_hProcess, (LPCVOID)addr, &tReadValue, len, NULL))
			{
				continue;//û�ж�ȡ�ɹ�
			}
			//��Ŀ����жԱ�
			if (0 == memcmp(pValue, &tReadValue, len)) {
				//ֵ��ȱ���
				dwTemp.push_back(addr);
			}
		}
		//���汾�ν��
		m_arlist = dwTemp;
		return !m_arlist.empty();
	}
	//������û����Ϣ���о�ȡ���������ҷַ���ȥ��
	void WaitForIdle()
	{
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {//hWnd,����==nULL ���ȡ���о������Ϣ
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
	DWORD m_dwPageSize;
	//Ŀ����̾��
	HANDLE m_hProcess{ INVALID_HANDLE_VALUE };
	//�����Ľ��
	std::list<DWORD>m_arlist;
	//�ص�����
	typedef bool(_stdcall *PFUN_CALLBACK)(void *pArgs, size_t nPageCount, size_t index);
	//�״�ɨ�� �ص�����
	PFUN_CALLBACK m_pGoonFirst{ nullptr };
	//�´�ɨ�� �ص�����
	PFUN_CALLBACK m_pGoonNext{ nullptr };
	// �ص������Ĳ���
	void *m_pArgsFirst{ nullptr };
	void *m_pArgsNext{ nullptr };
};