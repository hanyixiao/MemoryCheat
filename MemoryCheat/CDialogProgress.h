#pragma once
#include <afxdialogex.h>
#include "afxwin.h"
class CDialogProgress :
	public CDialogEx
{
	DECLARE_DYNAMIC(CDialogProgress);
public:
	CDialogProgress(CWnd *pParent = NULL);
	~CDialogProgress();
	//�Ի�������
#ifdef AFX_DESIGN_TIME
	enum{IDD = IDD_DIALOG_PROGRESS_LIST};
#endif
protected:
	virtual void DoDataExchange(CDataExchange *pDx);
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	//32λ���̱�
	CListCtrl m_lst;
	//�����б��ͼ��
	CImageList m_imgList;
	//��ȡ32Ϊ���̱�
	BOOL GetProcessList();
public:
	//��ѡ��ѡ�еĵĽ��̵�ID
	static DWORD m_dwProcessId;
	//¼��ѡ��Ľ�����
	static CString m_strProcessName;

	//˫�������б��¼�
	afx_msg void OnNMDblclkList1(NMHDR *pNMHER, LRESULT *pResult);
};


