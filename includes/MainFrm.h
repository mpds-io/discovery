// MainFrm.h : interface of the CMainFrame class
// Ported to Windows 8.1 by Fulcrum.013 Mariupol, Ukraine 
// 17 september 2020
//
/////////////////////////////////////////////////////////////////////////////
#define  _CRT_SECURE_NO_WARNINGS
class CMainFrame : public CFrameWnd
{
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
protected:
	CSplitterWnd m_wndSplitter;
	CSplitterWnd m_wndSplitter2;
	int m_iViewMode;
public:
	BOOL m_bMaximizeFrame;		// TRUE if main window is to be maximized

	//HANDLE m_hQsdProcess;		// process handle of QSD kernel

// Operations
public:
	BOOL SwitchToViewMode (int, BOOL =FALSE);
	void MaximizePane (void);
	void AdjustPaneSizes (void);
	BOOL UpdateMenu (int);		// insert view-specific pulldown menu after View menu
	BOOL UpdateToolbar (int);	// exchange some buttons specific for the new view
	BOOL SetQsdViews (int, int, int);	// change views in panes for QSD view mode

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void RecalcLayout(BOOL bNotify = TRUE);
	//}}AFX_VIRTUAL

	virtual void OnUpdateFrameTitle(BOOL bAddToTitle);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnViewQsd();
	afx_msg void OnUpdateViewQsd(CCmdUI* pCmdUI);
	afx_msg void OnViewReport();
	afx_msg void OnUpdateViewReport(CCmdUI* pCmdUI);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnQsdStart();
	afx_msg void OnUpdateQsdStart(CCmdUI* pCmdUI);
	afx_msg void OnViewChemistry();
	afx_msg void OnUpdateViewChemistry(CCmdUI* pCmdUI);
	afx_msg void OnViewQsdProfile();
	afx_msg void OnUpdateViewQsdProfile(CCmdUI* pCmdUI);
	afx_msg void OnViewQsdCube();
	afx_msg void OnUpdateViewQsdCube(CCmdUI* pCmdUI);
	afx_msg void OnUpdateIndicatorCount(CCmdUI* pCmdUI);
	afx_msg void OnUpdateIndicatorSel(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnQsdStartDirect();
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
