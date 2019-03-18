#include "stdafx.h"
#include "CDialogProgress.h"
#include "MemoryCheat.h"
#include "afxdialogex.h"

#include "TlHelp32.h"

namespace hh 
{
	//判断系统是否为64位的
	BOOL isOs64()
	{
		SYSTEM_INFO si;
		GetNativeSystemInfo(&si);
		if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64
			|| si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
		{
			return true;
		}
		else {
			return false;
		}
	}
	//判断是否为32位进程
	BOOL Is32BitProgress(DWORD dwProgressId)
	{
		//如果不是64位系统,肯定为32位进程
		if (!hh::isOs64()){
			return true;
		}
		if (dwProgressId == 0 || dwProgressId == 4)
		{
			return false;
		}
		//判断32位进程
		typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
		static LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)
			GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
		//是32位系统，进程肯定是32位
		if (!fnIsWow64Process) {
			return true;
		}
		//判断是否为64位系统下的32位进程
		HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | 
			PROCESS_VM_READ, FALSE, dwProgressId);
		if (!h || h == INVALID_HANDLE_VALUE) {
			return FALSE;
		}
		BOOL b32 = 0;
		BOOL b = fnIsWow64Process(h, &b32);
		CloseHandle(h);
		return b32;
	}
	//获得进程的完整路径
	BOOL GetFullPathProcess(DWORD dwProgressId, TCHAR szBuffer[], DWORD dwMaxLen)
	{
		HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION |
			PROCESS_VM_READ, FALSE, dwProgressId);
		if (h || h != INVALID_HANDLE_VALUE) {
			DWORD dwLen = dwMaxLen;
			BOOL b = ::QueryFullProcessImageName(h, 0, szBuffer, (PDWORD)&dwLen);
			if (b) {
				szBuffer[dwLen] = _T('\0');
			}
			CloseHandle(h);
			return b;
		}
		return false;
	}

	BOOL GetProcessIco(DWORD dwProcessId, HICON &hIco)
	{
		TCHAR szPath[MAX_PATH] = { 0 };
		if (hh::GetFullPathProcess(dwProcessId, szPath, sizeof(szPath))) {
			SHFILEINFO info = { 0 };
			DWORD_PTR dwRet = ::SHGetFileInfo(szPath, 0, &info, sizeof(info), SHGFI_ICON | SHGFI_LARGEICON);
			if (dwRet == 0) {
				hIco = 0;
			}
			else {
				hIco = info.hIcon;
			}
		}
		return hIco != 0;
	}

}

CDialogProgress::CDialogProgress(CWnd *pParent)
	: CDialogEx(IDD_DIALOG_PROGRESS_LIST,pParent)
{

}


CDialogProgress::~CDialogProgress()
{
}
