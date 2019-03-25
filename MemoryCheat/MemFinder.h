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
	/*
	*在findFirst的基础上继续寻找
	*/
	template< typename T>
	bool FindNext(T value)
	{
		return _FindNext(value);
	}
	//读内存
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
	//写内存
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
	//调用回调函数
	bool RemoteCall(unsigned char code[], size_t len,
		unsigned char para[], size_t paraLen)
	{
		if (!IsValidHandle()) {

		}
		//申请代码内存
		PVOID mFuncAddr = ::VirtualAllocEx(m_hProcess, NULL, len,
			MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (nullptr == mFuncAddr) {
			return false;
		}
		//申请函数参数内存
		PVOID ParamAddr = ::VirtualAllocEx(m_hProcess, NULL, len,
			MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		if (nullptr == ParamAddr) {
			return false;
		}

		//写入代码，和参数
		this->Write((DWORD)mFuncAddr, code, len);
		this->Write((DWORD)ParamAddr, para, paraLen);

		// 创建远程线程
		DWORD dwThreadId;
		HANDLE hThread = ::CreateRemoteThread(m_hProcess, NULL, 0,
			(LPTHREAD_START_ROUTINE)mFuncAddr, ParamAddr, 0, &dwThreadId);

		//等待执行完成
		if (hThread&&hThread != INVALID_HANDLE_VALUE) {
			WaitForSingleObject(hThread,1000);
			//等待指定对象处于信号状态或超1s
		}
		

		//释放内存
		VirtualFreeEx(hThread, mFuncAddr, len, MEM_RELEASE);
		VirtualFreeEx(hThread, ParamAddr, paraLen, MEM_RELEASE);
		return true;
	}
	//设置初次的扫描函数
	virtual void SetCallbackFirst(bool(_stdcall *pGoonFirst)(void *pArgs,
		size_t nAddrCount, size_t index), void *pArgs)
	{
		m_pGoonFirst = pGoonFirst;
		m_pArgsFirst = pArgs;
	}
	//设置初次扫描之后的回调函数
	virtual void SetCallbackNext(bool(_stdcall *pGoonFirst)(void *pArgs,
		size_t nAddrCount, size_t index), void *pArgs)
	{
		m_pGoonNext = pGoonFirst;
		m_pArgsNext = pArgs;
	}
	//获得结果
	virtual const std::list<DWORD> &GetResult() const
	{
		return m_arlist;
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
	template< typename T>
	bool _FindNext(T value)
	{
		//目标值的长度
		const size_t len = sizeof(value);
		const void *pValue = &value;

		//已经储存的地址数量
		size_t cnt = m_arlist.size();
		size_t index = 0;
		std::list<DWORD> dwTemp;
		for (auto addr : m_arlist) {
			//如果有回调函数，并且调用函数返回FALSE，则退出查找
			if (this->m_pGoonNext && !this->m_pGoonNext(this->m_pArgsNext, cnt, index++))
			{
				return false;
			}
			//处理消息
			WaitForIdle();

			//读取内容
			T tReadValue;
			if (!::ReadProcessMemory(m_hProcess, (LPCVOID)addr, &tReadValue, len, NULL))
			{
				continue;//没有读取成功
			}
			//和目标进行对比
			if (0 == memcmp(pValue, &tReadValue, len)) {
				//值相等保存
				dwTemp.push_back(addr);
			}
		}
		//保存本次结果
		m_arlist = dwTemp;
		return !m_arlist.empty();
	}
	//看看有没有消息，有就取出来，并且分发下去、
	void WaitForIdle()
	{
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {//hWnd,参数==nULL 则获取所有句柄的消息
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
	DWORD m_dwPageSize;
	//目标进程句柄
	HANDLE m_hProcess{ INVALID_HANDLE_VALUE };
	//搜索的结果
	std::list<DWORD>m_arlist;
	//回调函数
	typedef bool(_stdcall *PFUN_CALLBACK)(void *pArgs, size_t nPageCount, size_t index);
	//首次扫描 回调函数
	PFUN_CALLBACK m_pGoonFirst{ nullptr };
	//下次扫描 回调函数
	PFUN_CALLBACK m_pGoonNext{ nullptr };
	// 回调函数的参数
	void *m_pArgsFirst{ nullptr };
	void *m_pArgsNext{ nullptr };
};