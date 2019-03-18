#include "stdafx.h"
#include "CDialogProgress.h"
#include "MemoryCheat.h"
#include "afxdialogex.h"

#include "TlHelp32.h"

namespace hh 
{
	//�ж�ϵͳ�Ƿ�Ϊ64λ��
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
	//�ж��Ƿ�Ϊ32λ����
	BOOL Is32BitProgress(DWORD dwProgressId)
	{
		//�������64λϵͳ,�϶�Ϊ32λ����
		if (!hh::isOs64()){
			return true;
		}
		if (dwProgressId == 0 || dwProgressId == 4)
		{
			return false;
		}
		//�ж�32λ����
		typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);
		static LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)
			GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
		//��32λϵͳ�����̿϶���32λ
		if (!fnIsWow64Process) {
			return true;
		}
		//�ж��Ƿ�Ϊ64λϵͳ�µ�32λ����
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
	//��ý��̵�����·��
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
