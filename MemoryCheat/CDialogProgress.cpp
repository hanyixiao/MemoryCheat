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

DWORD CDialogProgress::m_dwProcessId = 0;
CString CDialogProgress::m_strProcessName = TEXT("");
//CDialog�Ի���
IMPLEMENT_DYNAMIC(CDialogProgress, CDialogEx);//MFc�ؼ���������ʱ���ʶ��


CDialogProgress::CDialogProgress(CWnd *pParent)
	: CDialogEx(IDD_DIALOG_PROGRESS_LIST,pParent)
{

}


CDialogProgress::~CDialogProgress()
{
}

void CDialogProgress::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_lst);
}
BEGIN_MESSAGE_MAP(CDialogProgress, CDialog)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CDialogProgress::OnNMDlbclkList1)
END_MESSAGE_MAP()

//CDialog��Ϣ����


//���32λϵͳ�����б�
BOOL CDialogProgress::GetProcessList()
{
	//ͨ�����̿��գ���������
	//�ж��Ƿ�Ϊ32λ����
	//����Ϊ32λ��������뵽m_lst�б���
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		return(FALSE);
	}
	//��õ�һ������
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32)) {
		CloseHandle(hProcessSnap);
		return(FALSE);
	}
	do {
		//�ж��Ƿ�Ϊ32λ����
		if (hh::Is32BitProgress(pe32.th32ProcessID)) {
			int indexIco = -1;
			{
				HICON hIco = 0;
				if (hh::GetProcessIco(pe32.th32ProcessID, hIco)) {
					//��������ͼ�꣬�����б�
					indexIco = m_imgList.Add(hIco);
				}
			}
			//��������
			{
				//������
				int index = m_lst.InsertItem(m_lst.GetItemCount(),
					pe32.szExeFile, indexIco);
				//����ID
				CString s;
				s.Format(TEXT("%d"), pe32.th32ProcessID);
				m_lst.SetItemText(index, 1, s);
				//���ø�������
				m_lst.SetItemData(index, (DWORD_PTR)pe32.th32ProcessID);

			}
		}
		else {
			//����32λ����ʲô������
		}
	}
	// �����һ������
	while (Process32Next(hProcessSnap, &pe32));
	
	//�رտ��վ��
	CloseHandle(hProcessSnap);
	return(TRUE);
}

BOOL CDialogProgress::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// �ؼ�
	{
		LONG lStyle = GetWindowLong(m_lst.m_hWnd, GWL_STYLE);
		lStyle &= ~LVS_TYPEMASK;
		lStyle |= LVS_REPORT;
		SetWindowLong(m_lst.GetSafeHwnd(), GWL_STYLE, lStyle);
		DWORD dwStyle = m_lst.GetExtendedStyle();
		dwStyle |= LVS_EX_FULLROWSELECT; //ѡ���� ��������
		dwStyle |= LVS_EX_GRIDLINES; //������
		m_lst.SetExtendedStyle(dwStyle);
		// ������,�����ô�С
		{
			CRect rc;
			m_lst.GetClientRect(rc);
			m_lst.InsertColumn(0, _T("������"), LVCFMT_LEFT, rc.Width() / 2);
			m_lst.InsertColumn(1, _T("����ID"), LVCFMT_LEFT, rc.Width() / 2);
		}
		// ���ÿؼ�������ͼ���б�,�����ſ�����ÿ�еĿ�ͷ��ʾͼ��
		m_imgList.Create(16, 16, ILC_COLOR32, 1, 1);
		m_lst.SetImageList(&m_imgList, LVSIL_SMALL);
	}
	//�г�����
	GetProcessList();
	return TRUE;  // return TRUE unless you set the focus to a control
	// �쳣: OCX ����ҳӦ���� FALSE
}


void CDialogProgress::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = pNMItemActivate->iItem;
	// �����ѡ�е���,���ûش��������ڵı���,ʹ�����ڿ���֪����ǰѡ��Ľ�����Ϣ
	if (nItem >= 0) {
		m_dwProcessId = (DWORD)m_lst.GetItemData(nItem);
		m_strProcessName = m_lst.GetItemText(nItem, 0);
		OnOK();
	}
	*pResult = 0;
}
