// DiscoveryView.h : interface of the CDiscoveryView class
// Ported to Windows 8.1 by Fulcrum.013 Mariupol, Ukraine 
// 17 september 2020
//

// The following message tells the Report View that it needs rebuilding after
// a header drag and drop operation:
#define  _CRT_SECURE_NO_WARNINGS
#define WM_REPORT_COLUMNORDERCHANGED	(WM_USER+0x0100)

/////////////////////////////////////////////////////////////////////////////
// CReportHeaderCtrl window

class CReportHeaderCtrl : public CHeaderCtrl
{
// Construction
public:
	CReportHeaderCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReportHeaderCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CReportHeaderCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CReportHeaderCtrl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CReportCtrl window

class CReportCtrl : public CListCtrl
{
// Construction
public:
	CReportCtrl();

// Attributes
public:
	//int m_aiColInd[MAX_REPORT_COLUMNS];	// indexes of the columns (header drag & drop)
	int *m_piRowInd;					// indexes of the rows (can be sorted)

	int m_aiSort[MAX_REPORT_COLUMNS];	// stores sorting order for every column
	int m_iSel;					// index of selected column (-1: no selection)

	BOOL m_bProcessedContextMenu;	// TRUE when <this> handles context menu himself

	CHeaderCtrl* GetHeaderCtrl();
	int GetColumnCount();
	int GetHeaderItemIndex (int);	// index of header item at position x
	BOOL GetHeaderItemRect (int, LPRECT);	// rectangle of header item

// Operations
public:
	int SetSort (int, int);		// set sorting direction for a column

// Overrides
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReportCtrl)
	protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CReportCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CReportCtrl)
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryView

class CDiscoveryView : public CView
{
protected: // create from serialization only
	CDiscoveryView();
	DECLARE_DYNCREATE(CDiscoveryView)

public:
	//CDiscoveryView();

// Attributes
public:
	CReportCtrl m_Report;

	CDiscoveryDoc* GetDocument();

	int Build (void);			// init columns, then sort and build the list
	int Rebuild (void);			// sort and build the list
	int Sort (void);			// sorts the list
	int RegisterColumnMove (int, int);	// register column rearrangement after header drag and drop
	void RegisterColumnSelection (int);	// register that a new column has been selected
	void RegisterSortOrder (int, int);	// register new sort order for a specified column
	void RegisterColumnWidth (int, int);// register new column width after tracking of the divider

private:
	int m_iPrintFontHeight;		// font size when printing, in twips
	int m_nPrintLines;			// total number of lines to be printed
	int m_nPrintLinesPerPage;	// maximum number of lines per page

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDiscoveryView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDiscoveryView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CDiscoveryView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnEditSelectAll();
	afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectInv();
	afx_msg void OnUpdateEditSelectInv(CCmdUI* pCmdUI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveData();
	afx_msg void OnUpdateFileSaveData(CCmdUI* pCmdUI);
	//}}AFX_MSG
	afx_msg void OnReportHeaderCmd(UINT nID);
	afx_msg LRESULT OnReportColumnOrderChanged(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in DiscoveryView.cpp
inline CDiscoveryDoc* CDiscoveryView::GetDocument()
   { return (CDiscoveryDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////
