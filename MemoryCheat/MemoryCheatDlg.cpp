
// MemoryCheatDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "MemoryCheat.h"
#include "MemoryCheatDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//回调函数
static bool *g_pGoon = nullptr;

//进度范围表（0-rage）
static const int range = 100;

//首次扫描函数
bool __stdcall FirstSearchRoutine(void *pArgs, size_t nPageCount, size_t index)
{
	CProgressCtrl *p = (CProgressCtrl *)pArgs;
	if (nPageCount == 0) {
		return *g_pGoon;
	}
	p->SetPos(static_cast<int>(index / (nPageCount / float(range))));
	return g_pGoon;
}

//下次扫描函数
bool __stdcall NextSearchRoutine(void *pArgs, size_t addrCount, size_t index)
{
	CProgressCtrl *p = (CProgressCtrl *)pArgs;
	if (addrCount == 0) {
		return *g_pGoon;
	}

	p->SetPos(static_cast<int>(index / (addrCount / float(range))));
	return *g_pGoon;

}

// CMemoryCheatDlg 对话框


//
//CMemoryCheatDlg::CMemoryCheatDlg(CWnd* pParent /*=nullptr*/)
//	: CDialogEx(IDD_MEMORYCHEAT_DIALOG, pParent)
//{
//	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
//}

//构造函数
CMemoryCheatDlg::CMemoryCheatDlg(CWnd *pParent)
	:CDialogEx(IDD_MEMORYCHEAT_DIALOG, pParent)
	, m_strSearchValue(_T(""))
	, m_strValueType(_T(""))
	, m_strLimitBegin(TEXT("0x00000000"))
	, m_strLimitEnd(TEXT("0xffffffff"))
	, m_strDesEdit(TEXT(""))
	, m_strValueEdit(TEXT(""))
	, m_strValueTypeEdit(TEXT(""))
	, m_strAddressEdit(TEXT("")) 
{
	m_hIcon = AfxGetApp()->LoadIconW(IDR_MAINFRAME);
	g_pGoon = &m_bGoon;
}

void CMemoryCheatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_ADDRESS_TEMP, m_lstAddressTemp);
	DDX_Control(pDX, IDC_LIST_ADDRESS_TARGET, m_lstAddressTarget);
	DDX_Text(pDX, IDC_EDIT_SEARCH_VALUE, m_strSearchValue);
	DDX_CBString(pDX, IDC_COMBO_VALUE_TYPE, m_strValueType);
	DDX_Control(pDX, IDC_COMBO_VALUE_TYPE,m_cbbValueType);
	DDX_Text(pDX, IDC_EDIT_LIMIT_START, m_strLimitBegin);
	DDX_Text(pDX, IDC_EDIT_LIMIT_END, m_strLimitEnd);
	DDX_Control(pDX, IDC_PROGRESS_SEARCH, m_pProcess);
	DDX_Text(pDX, IDC_EDIT_DES, m_strDesEdit);
	DDX_Text(pDX, IDC_EDIT_VALUE, m_strValueEdit);
	DDX_CBString(pDX, IDC_COMBO_VALUE_TYPE2, m_strValueTypeEdit);
	DDX_Control(pDX, IDC_COMBO_VALUE_TYPE2, m_cbbValueTypeEdit);
	DDX_Text(pDX, IDC_EDIT_ADDRESS, m_strAddressEdit);
}

BEGIN_MESSAGE_MAP(CMemoryCheatDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_PROCESS, &CMemoryCheatDlg::OnBnClickedButtonProgress)
	ON_BN_CLICKED(IDC_BUTTON_FIRST, &CMemoryCheatDlg::OnBnClickedButtonFirst)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, &CMemoryCheatDlg::OnBnClickedButtonNext)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CMemoryCheatDlg::OnBnClickedButtonStop)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_ADDRESS_TEMP, &CMemoryCheatDlg::OnNMDblclkListAddressTemp)
	ON_NOTIFY(NM_CLICK, IDC_LIST_ADDRESS_TARGET, &CMemoryCheatDlg::OnNMClickListAddressTarget)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CMemoryCheatDlg::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DEL, &CMemoryCheatDlg::OnBnClickedButtonDel)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CMemoryCheatDlg::OnBnClickedButtonSave)
	ON_CBN_SELCHANGE(IDC_COMBO_VALUE_TYPE2, &CMemoryCheatDlg::OnCbnSelchangeComboValueTypeEdit)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_PLANT_INJECT, &CMemoryCheatDlg::OnBnClickedButtonPlantInject)
END_MESSAGE_MAP()


// CMemoryCheatDlg 消息处理程序

BOOL CMemoryCheatDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//界面初始化
	{
		//临时数据列表
		{

		}

	}
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMemoryCheatDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMemoryCheatDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMemoryCheatDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CMemoryCheatDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
}


void CMemoryCheatDlg::OnBnClickedButton6()
{
	// TODO: 在此添加控件通知处理程序代码
}
