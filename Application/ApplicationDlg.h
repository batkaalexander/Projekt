
// ApplicationDlg.h : header file
//

#pragma once

#include "LogDlg.h"
#include <GdiPlus.h>
#include <stdlib.h>
#include <vector>
#include <thread>
#include <atomic>

class CStaticImage : public CStatic
{
public:
	// Overridables (for owner draw only)
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;
};

class CStaticHistogram : public CStatic
{
	void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) override;
};

// CApplicationDlg dialog
class CApplicationDlg : public CDialogEx
{
	// Construction
public:
	enum
	{
		WM_DRAW_IMAGE = (WM_USER + 1),
		WM_DRAW_HISTOGRAM,
		WM_SET_BITMAP,
		WM_ROTATE_IMAGE
	};

	CApplicationDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_APPLICATION_DIALOG };
#endif

protected:
	void DoDataExchange(CDataExchange* pDX) override;	// DDX/DDV support

	void OnOK() override {}
	void OnCancel() override {}


	// Implementation
protected:
	// rotation with clock rotation

	HICON m_hIcon;

	bool m_bHistRed = false;
	bool m_bHistBlue = false;
	bool m_bHistGreen = false;
	bool m_bHistJas = false;
	std::vector<int> m_uHistRed;
	std::vector<int> m_uHistBlue;
	std::vector<int> m_uHistGreen;
	std::vector<int> m_uHistJas;
	std::atomic<std::thread::id> m_thread_id;
	std::atomic<int> num_m_thread = std::thread::hardware_concurrency();
	// 0 none, 1 right90, 2 left90, 3 right45, 4 left45 
	int m_rightRot=0;
	// actual rotation in degrees
	int m_rotState = 0;

	void OpenImage(CString fName);
	void RotateImage();
	// Generated message map functions
	BOOL OnInitDialog() override;
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnFileOpen();
	afx_msg void OnUpdateFileOpen(CCmdUI *pCmdUI);
	afx_msg void OnFileClose();
	afx_msg void OnUpdateFileClose(CCmdUI *pCmdUI);
	afx_msg LRESULT OnKickIdle(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg void OnSizing(UINT nSide, LPRECT lpRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnDrawImage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDrawHistogram(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSetBitmap(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRotateBitmap(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
protected:
	CListCtrl m_ctrlFileList;
	CStaticImage m_ctrlImage;
	CStaticHistogram m_ctrlHistogram;

	CPoint m_ptFileList;
	CPoint m_ptHistogram;
	CPoint m_ptImage;

	CString m_csDirectory;

	CLogDlg m_ctrlLog;

	Gdiplus::Bitmap *m_pBitmap;
	Gdiplus::Bitmap *m_pBitmapBackUp;
	DWORD m_nMaxThreads;
public:
	afx_msg void OnLvnItemchangedFileList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLogOpen();
	afx_msg void OnUpdateLogOpen(CCmdUI *pCmdUI);
	afx_msg void OnLogClear();
	afx_msg void OnUpdateLogClear(CCmdUI *pCmdUI);
	afx_msg void OnHistogramRed();
	afx_msg void OnUpdateHistogramRed(CCmdUI *pCmdUI);
	afx_msg void OnHistogramBlue();
	afx_msg void OnUpdateHistogramBlue(CCmdUI *pCmdUI);
	afx_msg void OnHistogramGreen();
	afx_msg void OnUpdateHistogramGreen(CCmdUI *pCmdUI);
	afx_msg void OnHistogramJas();
	afx_msg void OnUpdateHistogramJas(CCmdUI *pCmdUI);
	afx_msg void On1();
	afx_msg void OnUpdate1(CCmdUI *pCmdUI);
	afx_msg void OnAuto();
	afx_msg void OnUpdateAuto(CCmdUI *pCmdUI);
	afx_msg void On2();
	afx_msg void OnUpdate2(CCmdUI *pCmdUI);
	afx_msg void On4();
	afx_msg void OnUpdate4(CCmdUI *pCmdUI);
	afx_msg void On8();
	afx_msg void OnUpdate8(CCmdUI *pCmdUI);
	afx_msg void On16();
	afx_msg void OnUpdate16(CCmdUI *pCmdUI);
	afx_msg void OnRotateleft45();
	afx_msg void OnUpdateRotateleft45(CCmdUI *pCmdUI);
	afx_msg void OnRotateleft90();
	afx_msg void OnUpdateRotateleft90(CCmdUI *pCmdUI);
	afx_msg void OnRotateright45();
	afx_msg void OnUpdateRotateright45(CCmdUI *pCmdUI);
	afx_msg void OnRotateright90();
	afx_msg void OnUpdateRotateright90(CCmdUI *pCmdUI);
	afx_msg void OnEfectReset();
	afx_msg void OnUpdateEfectReset(CCmdUI *pCmdUI);
};
