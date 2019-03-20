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
	//对话框数据
#ifdef AFX_DESIGN_TIME
	enum{IDD = IDD_DIALOG_PROGRESS_LIST};
#endif
protected:
	virtual void DoDataExchange(CDataExchange *pDx);
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	//32位进程表
	CListCtrl m_lst;
	//进程列表的图标
	CImageList m_imgList;
	//获取32为进程表
	BOOL GetProcessList();
public:
	//当选中选中的的进程的ID
	static DWORD m_dwProcessId;
	//录拖选择的进程名
	static CString m_strProcessName;

	//双击进程列表事件
	afx_msg void OnNMDblclkList1(NMHDR *pNMHER, LRESULT *pResult);
};


