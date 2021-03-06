
// ApplicationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Application.h"
#include "ApplicationDlg.h"
#include "afxdialogex.h"
#include <utility>
#include <tuple>
#include "Utils.h"
#include <omp.h>
#include <thread>
#include <functional>
#define _USE_MATH_DEFINES
#include <math.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef MIN_SIZE
#define MIN_SIZE 300
#endif

void CStaticImage::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	GetParent()->SendMessage(CApplicationDlg::WM_DRAW_IMAGE, (WPARAM)lpDrawItemStruct);
}

void CStaticHistogram::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	GetParent()->SendMessage(CApplicationDlg::WM_DRAW_HISTOGRAM, (WPARAM)lpDrawItemStruct);
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {}

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override    // DDX/DDV support
	{
		CDialogEx::DoDataExchange(pDX);
	}

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};


BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


namespace
{
	typedef BOOL(WINAPI *LPFN_GLPI)(
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION,
		PDWORD);


	// Helper function to count set bits in the processor mask.
	DWORD CountSetBits(ULONG_PTR bitMask)
	{
		DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
		DWORD bitSetCount = 0;
		ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
		DWORD i;

		for (i = 0; i <= LSHIFT; ++i)
		{
			bitSetCount += ((bitMask & bitTest) ? 1 : 0);
			bitTest /= 2;
		}

		return bitSetCount;
	}

	DWORD CountMaxThreads()
	{
		LPFN_GLPI glpi;
		BOOL done = FALSE;
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
		DWORD returnLength = 0;
		DWORD logicalProcessorCount = 0;
		DWORD numaNodeCount = 0;
		DWORD processorCoreCount = 0;
		DWORD processorL1CacheCount = 0;
		DWORD processorL2CacheCount = 0;
		DWORD processorL3CacheCount = 0;
		DWORD processorPackageCount = 0;
		DWORD byteOffset = 0;
		PCACHE_DESCRIPTOR Cache;

		glpi = (LPFN_GLPI)GetProcAddress(
			GetModuleHandle(TEXT("kernel32")),
			"GetLogicalProcessorInformation");
		if (NULL == glpi)
		{
			TRACE(TEXT("\nGetLogicalProcessorInformation is not supported.\n"));
			return (1);
		}

		while (!done)
		{
			DWORD rc = glpi(buffer, &returnLength);

			if (FALSE == rc)
			{
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					if (buffer)
						free(buffer);

					buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(
						returnLength);

					if (NULL == buffer)
					{
						TRACE(TEXT("\nError: Allocation failure\n"));
						return (2);
					}
				}
				else
				{
					TRACE(TEXT("\nError %d\n"), GetLastError());
					return (3);
				}
			}
			else
			{
				done = TRUE;
			}
		}

		ptr = buffer;

		while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength)
		{
			switch (ptr->Relationship)
			{
			case RelationNumaNode:
				// Non-NUMA systems report a single record of this type.
				numaNodeCount++;
				break;

			case RelationProcessorCore:
				processorCoreCount++;

				// A hyperthreaded core supplies more than one logical processor.
				logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
				break;

			case RelationCache:
				// Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
				Cache = &ptr->Cache;
				if (Cache->Level == 1)
				{
					processorL1CacheCount++;
				}
				else if (Cache->Level == 2)
				{
					processorL2CacheCount++;
				}
				else if (Cache->Level == 3)
				{
					processorL3CacheCount++;
				}
				break;

			case RelationProcessorPackage:
				// Logical processors share a physical package.
				processorPackageCount++;
				break;

			default:
				TRACE(TEXT("\nError: Unsupported LOGICAL_PROCESSOR_RELATIONSHIP value.\n"));
				break;
			}
			byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
			ptr++;
		}

		TRACE(TEXT("\nGetLogicalProcessorInformation results:\n"));
		TRACE(TEXT("Number of NUMA nodes: %d\n"), numaNodeCount);
		TRACE(TEXT("Number of physical processor packages: %d\n"), processorPackageCount);
		TRACE(TEXT("Number of processor cores: %d\n"), processorCoreCount);
		TRACE(TEXT("Number of logical processors: %d\n"), logicalProcessorCount);
		TRACE(TEXT("Number of processor L1/L2/L3 caches: %d/%d/%d\n"), processorL1CacheCount, processorL2CacheCount, processorL3CacheCount);

		free(buffer);
		TRACE(_T("OPENMP - %i/%i\n"), omp_get_num_procs(), omp_get_max_threads());
		return logicalProcessorCount;
	}
}

// CApplicationDlg dialog


CApplicationDlg::CApplicationDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_APPLICATION_DIALOG, pParent)
	, m_pBitmap(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_nMaxThreads = CountMaxThreads();
}

void CApplicationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILE_LIST, m_ctrlFileList);
	DDX_Control(pDX, IDC_IMAGE, m_ctrlImage);
	DDX_Control(pDX, IDC_HISTOGRAM, m_ctrlHistogram);
}

BEGIN_MESSAGE_MAP(CApplicationDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPEN, OnUpdateFileOpen)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, OnUpdateFileClose)
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_MESSAGE(WM_DRAW_IMAGE, OnDrawImage)
	ON_MESSAGE(WM_DRAW_HISTOGRAM, OnDrawHistogram)
	ON_MESSAGE(WM_SET_BITMAP, OnSetBitmap)
	ON_MESSAGE(WM_ROTATE_IMAGE, OnRotateBitmap)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_FILE_LIST, OnLvnItemchangedFileList)
	ON_COMMAND(ID_LOG_OPEN, OnLogOpen)
	ON_UPDATE_COMMAND_UI(ID_LOG_OPEN, OnUpdateLogOpen)
	ON_COMMAND(ID_LOG_CLEAR, OnLogClear)
	ON_UPDATE_COMMAND_UI(ID_LOG_CLEAR, OnUpdateLogClear)
	ON_WM_DESTROY()
	ON_COMMAND(ID_HISTOGRAM_RED, &CApplicationDlg::OnHistogramRed)
	ON_UPDATE_COMMAND_UI(ID_HISTOGRAM_RED, &CApplicationDlg::OnUpdateHistogramRed)
	ON_COMMAND(ID_HISTOGRAM_BLUE, &CApplicationDlg::OnHistogramBlue)
	ON_UPDATE_COMMAND_UI(ID_HISTOGRAM_BLUE, &CApplicationDlg::OnUpdateHistogramBlue)
	ON_COMMAND(ID_HISTOGRAM_GREEN, &CApplicationDlg::OnHistogramGreen)
	ON_UPDATE_COMMAND_UI(ID_HISTOGRAM_GREEN, &CApplicationDlg::OnUpdateHistogramGreen)
	ON_COMMAND(ID_HISTOGRAM_JAS, &CApplicationDlg::OnHistogramJas)
	ON_UPDATE_COMMAND_UI(ID_HISTOGRAM_JAS, &CApplicationDlg::OnUpdateHistogramJas)
	ON_COMMAND(THREADS_1, &CApplicationDlg::On1)
	ON_UPDATE_COMMAND_UI(THREADS_1, &CApplicationDlg::OnUpdate1)
	ON_COMMAND(THREADS_AUTO, &CApplicationDlg::OnAuto)
	ON_UPDATE_COMMAND_UI(THREADS_AUTO, &CApplicationDlg::OnUpdateAuto)
	ON_COMMAND(THREADS_2, &CApplicationDlg::On2)
	ON_UPDATE_COMMAND_UI(THREADS_2, &CApplicationDlg::OnUpdate2)
	ON_COMMAND(THREADS_4, &CApplicationDlg::On4)
	ON_UPDATE_COMMAND_UI(THREADS_4, &CApplicationDlg::OnUpdate4)
	ON_COMMAND(THREADS_8, &CApplicationDlg::On8)
	ON_UPDATE_COMMAND_UI(THREADS_8, &CApplicationDlg::OnUpdate8)
	ON_COMMAND(THREADS_16, &CApplicationDlg::On16)
	ON_UPDATE_COMMAND_UI(THREADS_16, &CApplicationDlg::OnUpdate16)
	ON_COMMAND(ID_ROTATELEFT_45, &CApplicationDlg::OnRotateleft45)
	ON_UPDATE_COMMAND_UI(ID_ROTATELEFT_45, &CApplicationDlg::OnUpdateRotateleft45)
	ON_COMMAND(ID_ROTATELEFT_90, &CApplicationDlg::OnRotateleft90)
	ON_UPDATE_COMMAND_UI(ID_ROTATELEFT_90, &CApplicationDlg::OnUpdateRotateleft90)
	ON_COMMAND(ID_ROTATERIGHT_45, &CApplicationDlg::OnRotateright45)
	ON_UPDATE_COMMAND_UI(ID_ROTATERIGHT_45, &CApplicationDlg::OnUpdateRotateright45)
	ON_COMMAND(ID_ROTATERIGHT_90, &CApplicationDlg::OnRotateright90)
	ON_UPDATE_COMMAND_UI(ID_ROTATERIGHT_90, &CApplicationDlg::OnUpdateRotateright90)
	ON_COMMAND(ID_EFECT_RESET, &CApplicationDlg::OnEfectReset)
	ON_UPDATE_COMMAND_UI(ID_EFECT_RESET, &CApplicationDlg::OnUpdateEfectReset)
END_MESSAGE_MAP()


void CApplicationDlg::OnDestroy()
{
	m_ctrlLog.DestroyWindow();
	Default();

	if (m_pBitmap != nullptr)
	{
		delete m_pBitmap;
		m_pBitmap = nullptr;
	}
}

namespace
{
	void DrawHistogram(CDC *&DC, CRect &rc, std::vector<int> &hist, COLORREF clr)
	{
		double scaleY = 0;
		double scaleX = 0;
		int vyska = rc.bottom - rc.top;
		int sirka = rc.right - rc.left;
		for (int i = 0; i < hist.size(); i++)
		{
			if (scaleY < hist[i])
			{
				scaleY = hist[i];
			}
			if (hist[i] != 0)
			{
				scaleX++;
			}
		}
		scaleY = vyska / scaleY;
		scaleX = sirka / scaleX;
		for (int i = 0; i < hist.size(); i++)
		{

			DC->FillSolidRect((int)(rc.left + (i)*scaleX), (int)(rc.bottom - (hist[i])*scaleY), 1, (int)((hist[i])*scaleY), clr);
		}
		return;
	}
}

LRESULT CApplicationDlg::OnDrawHistogram(WPARAM wParam, LPARAM lParam)
{

	LPDRAWITEMSTRUCT lpDI = (LPDRAWITEMSTRUCT)wParam;

	CDC * pDC = CDC::FromHandle(lpDI->hDC);

	pDC->FillSolidRect(&(lpDI->rcItem), RGB(255, 255, 255));
	CRect rct = lpDI->rcItem;
	if (m_bHistRed) DrawHistogram(pDC, rct, m_uHistRed, RGB(255, 0, 0));
	if (m_bHistGreen) DrawHistogram(pDC, rct, m_uHistBlue, RGB(0, 255, 0));
	if (m_bHistBlue) DrawHistogram(pDC, rct, m_uHistGreen, RGB(0, 0, 255));
	if (m_bHistJas) DrawHistogram(pDC, rct, m_uHistJas, RGB(0, 0, 0));
	CBrush brBlack(RGB(0, 0, 0));
	pDC->FrameRect(&(lpDI->rcItem), &brBlack);
	return S_OK;
}

LRESULT CApplicationDlg::OnDrawImage(WPARAM wParam, LPARAM lParam)
{
	LPDRAWITEMSTRUCT lpDI = (LPDRAWITEMSTRUCT)wParam;

	CDC * pDC = CDC::FromHandle(lpDI->hDC);

	if (m_pBitmap == nullptr)
	{
		pDC->FillSolidRect(&(lpDI->rcItem), RGB(255, 255, 255));
	}
	else
	{
		// Fit bitmap into client area
		double dWtoH = (double)m_pBitmap->GetWidth() / (double)m_pBitmap->GetHeight();

		CRect rct(lpDI->rcItem);
		rct.DeflateRect(1, 1, 1, 1);

		UINT nHeight = rct.Height();
		UINT nWidth = (UINT)(dWtoH * nHeight);

		if (nWidth > (UINT)rct.Width())
		{
			nWidth = rct.Width();
			nHeight = (UINT)(nWidth / dWtoH);
			_ASSERTE(nHeight <= (UINT)rct.Height());
		}

		if (nHeight < (UINT)rct.Height())
		{
			UINT nBanner = (rct.Height() - nHeight) / 2;
			pDC->FillSolidRect(rct.left, rct.top, rct.Width(), nBanner, RGB(255, 255, 255));
			pDC->FillSolidRect(rct.left, rct.bottom - nBanner - 2, rct.Width(), nBanner + 2, RGB(255, 255, 255));
		}

		if (nWidth < (UINT)rct.Width())
		{
			UINT nBanner = (rct.Width() - nWidth) / 2;
			pDC->FillSolidRect(rct.left, rct.top, nBanner, rct.Height(), RGB(255, 255, 255));
			pDC->FillSolidRect(rct.right - nBanner - 2, rct.top, nBanner + 2, rct.Height(), RGB(255, 255, 255));
		}

		Gdiplus::Graphics gr(lpDI->hDC);
		Gdiplus::Rect destRect(rct.left + (rct.Width() - nWidth) / 2, rct.top + (rct.Height() - nHeight) / 2, nWidth, nHeight);
		gr.DrawImage(m_pBitmap, destRect);
	}

	CBrush brBlack(RGB(0, 0, 0));
	pDC->FrameRect(&(lpDI->rcItem), &brBlack);

	return S_OK;
}

void CApplicationDlg::OnSizing(UINT nSide, LPRECT lpRect)
{
	if ((lpRect->right - lpRect->left) < MIN_SIZE)
	{
		switch (nSide)
		{
		case WMSZ_LEFT:
		case WMSZ_BOTTOMLEFT:
		case WMSZ_TOPLEFT:
			lpRect->left = lpRect->right - MIN_SIZE;
		default:
			lpRect->right = lpRect->left + MIN_SIZE;
			break;
		}
	}

	if ((lpRect->bottom - lpRect->top) < MIN_SIZE)
	{
		switch (nSide)
		{
		case WMSZ_TOP:
		case WMSZ_TOPRIGHT:
		case WMSZ_TOPLEFT:
			lpRect->top = lpRect->bottom - MIN_SIZE;
		default:
			lpRect->bottom = lpRect->top + MIN_SIZE;
			break;
		}
	}

	__super::OnSizing(nSide, lpRect);
}

void CApplicationDlg::OnSize(UINT nType, int cx, int cy)
{
	Default();

	if (!::IsWindow(m_ctrlFileList.m_hWnd) || !::IsWindow(m_ctrlImage.m_hWnd))
		return;

	m_ctrlFileList.SetWindowPos(nullptr, -1, -1, m_ptFileList.x, cy - m_ptFileList.y, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
	m_ctrlFileList.Invalidate();


	m_ctrlImage.SetWindowPos(nullptr, -1, -1, cx - m_ptImage.x, cy - m_ptImage.y, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
	m_ctrlImage.Invalidate();

	CRect rct;
	GetClientRect(&rct);

	m_ctrlHistogram.SetWindowPos(nullptr, rct.left + m_ptHistogram.x, rct.bottom - m_ptHistogram.y, -1, -1, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	m_ctrlHistogram.Invalidate();
}

void CApplicationDlg::OnClose()
{
	EndDialog(0);
}

BOOL CApplicationDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CRect rct;
	m_ctrlFileList.GetClientRect(&rct);
	m_ctrlFileList.InsertColumn(0, _T("Filename"), 0, rct.Width());

	CRect rctClient;
	GetClientRect(&rctClient);
	m_ctrlFileList.GetWindowRect(&rct);
	m_ptFileList.y = rctClient.Height() - rct.Height();
	m_ptFileList.x = rct.Width();

	m_ctrlImage.GetWindowRect(&rct);
	m_ptImage.x = rctClient.Width() - rct.Width();
	m_ptImage.y = rctClient.Height() - rct.Height();

	m_ctrlHistogram.GetWindowRect(&rct);
	ScreenToClient(&rct);
	m_ptHistogram.x = rct.left - rctClient.left;
	m_ptHistogram.y = rctClient.bottom - rct.top;

	m_ctrlLog.Create(IDD_LOG_DIALOG, this);
	m_ctrlLog.ShowWindow(SW_HIDE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CApplicationDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CApplicationDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CApplicationDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CApplicationDlg::OnFileOpen()
{
	CFileDialog dlg(true, nullptr, nullptr
		, OFN_ALLOWMULTISELECT | OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON | OFN_PATHMUSTEXIST
		, _T("JPEG Files (*.jpg;*.jpeg)|*.jpg;*.jpeg | Bitmap Files (*.bmp)|*.bmp|PNG Files (*.png)|*.png||")
		, this);
	CString cs;
	const int maxFiles = 100;
	const int buffSize = maxFiles * (MAX_PATH + 1) + 1;

	dlg.GetOFN().lpstrFile = cs.GetBuffer(buffSize);
	dlg.GetOFN().nMaxFile = buffSize;

	if (dlg.DoModal() == IDOK)
	{
		m_ctrlFileList.DeleteAllItems();

		if (m_pBitmap != nullptr)
		{
			delete m_pBitmap;
			m_pBitmap = nullptr;
		}

		m_ctrlImage.Invalidate();
		m_ctrlHistogram.Invalidate();

		cs.ReleaseBuffer();

		std::vector<CString> names;

		std::tie(m_csDirectory, names) = Utils::ParseFiles(cs);

		for (int i = 0; i < (int)names.size(); ++i)
		{
			m_ctrlFileList.InsertItem(i, names[i]);
		}
	}
	else
	{
		cs.ReleaseBuffer();
	}

}


void CApplicationDlg::OnUpdateFileOpen(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


void CApplicationDlg::OnFileClose()
{
	m_ctrlFileList.DeleteAllItems();
}


void CApplicationDlg::OnUpdateFileClose(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_ctrlFileList.GetItemCount() > 0);
}

LRESULT CApplicationDlg::OnKickIdle(WPARAM wParam, LPARAM lParam)
{
	CMenu* pMainMenu = GetMenu();
	CCmdUI cmdUI;
	for (UINT n = 0; n < (UINT)pMainMenu->GetMenuItemCount(); ++n)
	{
		CMenu* pSubMenu = pMainMenu->GetSubMenu(n);
		cmdUI.m_nIndexMax = pSubMenu->GetMenuItemCount();
		for (UINT i = 0; i < cmdUI.m_nIndexMax; ++i)
		{
			cmdUI.m_nIndex = i;
			cmdUI.m_nID = pSubMenu->GetMenuItemID(i);
			cmdUI.m_pMenu = pSubMenu;
			cmdUI.DoUpdate(this, FALSE);
		}
	}
	return TRUE;
}

namespace
{
	void LoadAndCalc(CString fileName, Gdiplus::Bitmap *&bitmp, std::vector<int> &histr, std::vector<int> &histg, std::vector<int> &histb, std::vector<int> &histj, const int tn, std::function<bool()> fn);
	void ProcessAndRotate(Gdiplus::Bitmap *&bitmp, int right, int rotState, const int tn, std::function<bool()> fn);
}

void CApplicationDlg::OpenImage(CString fName)
{
	Gdiplus::Bitmap *bmp = m_pBitmap;
	std::vector<int> lhistr;
	std::vector<int> lhistg;
	std::vector<int> lhistb;
	std::vector<int> lhistj;
	std::thread::id thisThread = std::this_thread::get_id();
	m_thread_id = thisThread;
	LoadAndCalc(fName, bmp, lhistr, lhistg, lhistb, lhistj, num_m_thread, [this, thisThread]() {return m_thread_id != thisThread; });
	if (thisThread == m_thread_id)
	{
		std::tuple<std::thread::id, Gdiplus::Bitmap*, std::vector<int>&, std::vector<int>&, std::vector<int>&, std::vector<int>&> obj(thisThread, bmp, lhistr, lhistg, lhistb, lhistj);
		SendMessage(WM_SET_BITMAP, (WPARAM)&obj);
	}
	else
	{
		delete bmp;
	}
}

void CApplicationDlg::RotateImage()
{
	if (m_pBitmap != nullptr && m_rightRot != 0)
	{
		Gdiplus::Bitmap *bmp = m_pBitmapBackUp;
		switch (m_rightRot)
		{
		case 1:
			m_rotState += 90;
			break;
		case 2:
			m_rotState -= 90;
			break;
		case 3:
			m_rotState += 45;
			break;
		case 4:
			m_rotState -= 45;
			break;
		};

		if (m_rotState < 0)
		{
			m_rotState = 360 + m_rotState;
		}
		else
		{
			m_rotState = m_rotState % 360;
		}
		std::thread::id thisThread = std::this_thread::get_id();
		m_thread_id = thisThread;
		ProcessAndRotate(bmp, m_rightRot, m_rotState, num_m_thread, [this, thisThread]() {return m_thread_id != thisThread; });
		if (thisThread == m_thread_id)
		{
			std::tuple<Gdiplus::Bitmap*, int, int> obj(bmp, m_rightRot, m_rotState);
			SendMessage(WM_ROTATE_IMAGE, (WPARAM)&obj);
		}
	}
}

void CApplicationDlg::OnLvnItemchangedFileList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	if (m_pBitmap != nullptr)
	{
		delete m_pBitmap;
		m_pBitmap = nullptr;
	}

	CString csFileName;
	POSITION pos = m_ctrlFileList.GetFirstSelectedItemPosition();
	if (pos)
		csFileName = m_csDirectory + m_ctrlFileList.GetItemText(m_ctrlFileList.GetNextSelectedItem(pos), 0);

	if (!csFileName.IsEmpty())
	{
		std::thread thread(&CApplicationDlg::OpenImage, this, csFileName);
		thread.detach();
	}
	else
	{
		m_uHistRed.clear();
		m_uHistGreen.clear();
		m_uHistBlue.clear();
		m_uHistJas.clear();
	}

	m_ctrlImage.Invalidate();
	m_ctrlHistogram.Invalidate();

	*pResult = 0;
}

void CApplicationDlg::OnLogOpen()
{
	m_ctrlLog.ShowWindow(SW_SHOW);
}


void CApplicationDlg::OnUpdateLogOpen(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(::IsWindow(m_ctrlLog.m_hWnd) && !m_ctrlLog.IsWindowVisible());
}


void CApplicationDlg::OnLogClear()
{
	m_ctrlLog.SendMessage(CLogDlg::WM_TEXT);
}


void CApplicationDlg::OnUpdateLogClear(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(::IsWindow(m_ctrlLog.m_hWnd) && m_ctrlLog.IsWindowVisible());
}

void CApplicationDlg::OnHistogramRed()
{
	// TODO: Add your command handler code here
	m_bHistRed = !m_bHistRed;
	Invalidate();
}


void CApplicationDlg::OnUpdateHistogramRed(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (m_bHistRed)pCmdUI->SetCheck(1);
	else pCmdUI->SetCheck(0);
}


void CApplicationDlg::OnHistogramBlue()
{
	// TODO: Add your command handler code here
	m_bHistBlue = !m_bHistBlue;
	Invalidate();
}


void CApplicationDlg::OnUpdateHistogramBlue(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (m_bHistBlue)pCmdUI->SetCheck(1);
	else pCmdUI->SetCheck(0);
}


void CApplicationDlg::OnHistogramGreen()
{
	// TODO: Add your command handler code here
	m_bHistGreen = !m_bHistGreen;
	Invalidate();
}


void CApplicationDlg::OnUpdateHistogramGreen(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (m_bHistGreen)pCmdUI->SetCheck(1);
	else pCmdUI->SetCheck(0);
}


void CApplicationDlg::OnHistogramJas()
{
	// TODO: Add your command handler code here
	m_bHistJas = !m_bHistJas;
	Invalidate();
}


void CApplicationDlg::OnUpdateHistogramJas(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (m_bHistJas)pCmdUI->SetCheck(1);
	else pCmdUI->SetCheck(0);
}

LRESULT CApplicationDlg::OnRotateBitmap(WPARAM wParam, LPARAM lParam)
{
	auto ptuple = (std::tuple<Gdiplus::Bitmap*, int, int> *)(wParam);
	m_pBitmap = std::get<0>(*ptuple);
	m_rightRot = std::get<1>(*ptuple);
	m_rotState = std::get<2>(*ptuple);
	m_rightRot = 0;
	m_ctrlImage.Invalidate();
	return 0;
}

LRESULT CApplicationDlg::OnSetBitmap(WPARAM wParam, LPARAM lParam)
{
	auto ptuple = (std::tuple<std::thread::id, Gdiplus::Bitmap*, std::vector<int>&, std::vector<int>&, std::vector<int>&, std::vector<int>&> *)(wParam);
	if (std::get<0>(*ptuple) == m_thread_id)
	{
		m_pBitmap = std::get<1>(*ptuple);
		m_pBitmapBackUp = std::get<1>(*ptuple);
		m_uHistRed = std::move(std::get<2>(*ptuple));
		m_uHistGreen = std::move(std::get<3>(*ptuple));
		m_uHistBlue = std::move(std::get<4>(*ptuple));
		m_uHistJas = std::move(std::get<5>(*ptuple));
		m_rightRot = 0;
		m_rotState = 0;
		m_ctrlImage.Invalidate();
		m_ctrlHistogram.Invalidate();
	}
	return 0;
}


void CApplicationDlg::On1()
{
	// TODO: Add your command handler code here
	num_m_thread = 1;
	Invalidate();
}


void CApplicationDlg::OnUpdate1(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (num_m_thread == 1)pCmdUI->SetCheck(1);
	else pCmdUI->SetCheck(0);
}


void CApplicationDlg::OnAuto()
{
	// TODO: Add your command handler code here
	num_m_thread = std::thread::hardware_concurrency();
	Invalidate();
}


void CApplicationDlg::OnUpdateAuto(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (num_m_thread == std::thread::hardware_concurrency())pCmdUI->SetCheck(1);
	else pCmdUI->SetCheck(0);
}


void CApplicationDlg::On2()
{
	// TODO: Add your command handler code here
	num_m_thread = 2;
	Invalidate();
}


void CApplicationDlg::OnUpdate2(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (num_m_thread == 2)pCmdUI->SetCheck(1);
	else pCmdUI->SetCheck(0);
}


void CApplicationDlg::On4()
{
	// TODO: Add your command handler code here
	num_m_thread = 4;
	Invalidate();
}


void CApplicationDlg::OnUpdate4(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (num_m_thread == 4)pCmdUI->SetCheck(1);
	else pCmdUI->SetCheck(0);
}


void CApplicationDlg::On8()
{
	// TODO: Add your command handler code here
	num_m_thread = 8;
	Invalidate();
}


void CApplicationDlg::OnUpdate8(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (num_m_thread == 8)pCmdUI->SetCheck(1);
	else pCmdUI->SetCheck(0);
}


void CApplicationDlg::On16()
{
	// TODO: Add your command handler code here
	num_m_thread = 16;
	Invalidate();
}


void CApplicationDlg::OnUpdate16(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
	if (num_m_thread == 16)pCmdUI->SetCheck(1);
	else pCmdUI->SetCheck(0);
}

namespace
{
	void LoadAndCalc(CString fileName, Gdiplus::Bitmap *&bitmp, std::vector<int> &histr, std::vector<int> &histg, std::vector<int> &histb, std::vector<int> &histj, const int tn, std::function<bool()> fn)
	{
		bitmp = Gdiplus::Bitmap::FromFile(fileName);
		if (bitmp == NULL)
			return;
		Gdiplus::BitmapData* bmpData = new Gdiplus::BitmapData();
		Gdiplus::Rect rectangle(0, 0, bitmp->GetWidth(), bitmp->GetHeight());
		bitmp->LockBits(&rectangle, Gdiplus::ImageLockModeRead, PixelFormat32bppRGB, bmpData);
		histr.clear();
		histg.clear();
		histb.clear();
		histj.clear();;
		histr.assign(256, 0);
		histg.assign(256, 0);
		histb.assign(256, 0);
		histj.assign(256, 0);
		Utils::Threading(histr, histg, histb, histj, bitmp->GetWidth(), bitmp->GetHeight(), bmpData->Scan0, bmpData->Stride, tn, fn);
		bitmp->UnlockBits(bmpData);
		delete bmpData;
		return;
	}

	void ProcessAndRotate(Gdiplus::Bitmap *&bitmp, int right, int rotState, const int tn, std::function<bool()> fn)
	{
		if (right != 0)
		{
			Gdiplus::BitmapData* bmpData = new Gdiplus::BitmapData();
			int height = bitmp->GetHeight();
			int width = bitmp->GetWidth();
			float radians = (2 * 3.1416f*rotState) / 360;
			float cosine = (float)cos(radians);
			float sine = (float)sin(radians);

			float Point1x = (-height*sine);
			float Point1y = (height*cosine);
			float Point2x = (width*cosine - height*sine);
			float Point2y = (height*cosine + width*sine);
			float Point3x = (width*cosine);
			float Point3y = (width*sine);

			int DestBitmapWidth;
			int DestBitmapHeight;
			float minx;
			float miny;
			float maxx;
			float maxy;
			if (rotState != 135 && rotState != 225)
			{
				minx = min(0, min(Point1x, min(Point2x, Point3x)));
				miny = min(0, min(Point1y, min(Point2y, Point3y)));
				maxx = max(Point1x, max(Point2x, Point3x));
				maxy = max(Point1y, max(Point2y, Point3y));

				DestBitmapWidth = (int)ceil(fabs(maxx) - minx);
				DestBitmapHeight = (int)ceil(fabs(maxy) - miny);
			}
			else if (rotState == 135)
			{
				minx = min(0, min(Point1x, min(Point2x, Point3x)));
				miny = min(0, min(Point1y, min(Point2y, Point3y)));
				maxx = max(Point1x, max(Point2x, Point3x));
				maxy = max(Point1y, max(Point2y, Point3y));

				float minxx = min(0, min(Point1x, min(-Point2x, -Point3x)));
				float minyx = min(0, min(Point1y, min(Point2y, Point3y)));
				float maxxx = max(Point1x, max(Point2x, -Point3x));
				float maxyx = max(Point1y, max(Point2y, Point3y));

				DestBitmapWidth = (int)ceil(fabs(maxxx) - minxx);
				DestBitmapHeight = (int)ceil(fabs(maxyx) - minyx);
			}
			else if (rotState == 225)
			{
				minx = min(0, min(Point1x, min(Point2x, Point3x)));
				miny = min(0, min(Point1y, min(Point2y, Point3y)));
				maxx = max(Point1x, max(Point2x, Point3x));
				maxy = max(Point1y, max(Point2y, Point3y));

				float minxx = min(0, min(Point1x, min(Point2x, Point3x)));
				float minyx = min(0, min(Point1y, min(-Point2y, Point3y)));
				float maxxx = max(Point1x, max(Point2x, Point3x));
				float maxyx = max(Point1y, max(Point2y, Point3y));

				DestBitmapWidth = (int)ceil(fabs(maxxx) - minxx);
				DestBitmapHeight = (int)ceil(fabs(maxyx) - minyx);
			}

			Gdiplus::Rect rectangle(0, 0, bitmp->GetWidth(), bitmp->GetHeight());
			Gdiplus::Rect rectangleC(0, 0, DestBitmapWidth, DestBitmapHeight);
			Gdiplus::Bitmap bitmpC(DestBitmapWidth, DestBitmapHeight, PixelFormat32bppRGB);
			Gdiplus::BitmapData* bmpDataC = new Gdiplus::BitmapData();
			bitmp->LockBits(&rectangle, Gdiplus::ImageLockModeRead, PixelFormat32bppRGB, bmpData);
			bitmpC.LockBits(&rectangleC, Gdiplus::ImageLockModeWrite, PixelFormat32bppRGB, bmpDataC);

			Utils::Rotate(bmpData->Scan0, bmpDataC->Scan0, bmpData->Stride, bmpDataC->Stride, height, width, rotState, tn, fn);

			bitmpC.UnlockBits(bmpDataC);
			bitmp->UnlockBits(bmpData);
			bitmp = bitmpC.Clone(rectangleC, PixelFormat32bppRGB);
			delete bmpDataC;
			delete bmpData;
		}
	}
}

void CApplicationDlg::OnRotateleft45()
{
	// TODO: Add your command handler code here
	m_rightRot = 4;
	RotateImage();
	Invalidate();
}


void CApplicationDlg::OnUpdateRotateleft45(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

void CApplicationDlg::OnRotateleft90()
{
	// TODO: Add your command handler code here
	m_rightRot = 2;
	RotateImage();
	Invalidate();
}

void CApplicationDlg::OnUpdateRotateleft90(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}

void CApplicationDlg::OnRotateright45()
{
	// TODO: Add your command handler code here
	m_rightRot = 3;
	RotateImage();
	Invalidate();
}


void CApplicationDlg::OnUpdateRotateright45(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}


void CApplicationDlg::OnRotateright90()
{
	// TODO: Add your command handler code here
	m_rightRot = 1;
	RotateImage();
	Invalidate();
}


void CApplicationDlg::OnUpdateRotateright90(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}


void CApplicationDlg::OnEfectReset()
{
	// TODO: Add your command handler code here
	m_pBitmap = m_pBitmapBackUp;
	m_rightRot = 0;
	m_rotState = 0;
	Invalidate();
}


void CApplicationDlg::OnUpdateEfectReset(CCmdUI *pCmdUI)
{
	// TODO: Add your command update UI handler code here
}
