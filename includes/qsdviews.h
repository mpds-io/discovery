// QsdViews.h : header file
// Ported to Windows 8.1 by Fulcrum.013 Mariupol, Ukraine 
// 17 september 2020
//
#define  _CRT_SECURE_NO_WARNINGS
#include <afxcview.h>

class CDiscoveryDoc;

/////////////////////////////////////////////////////////////////////////////
// CProfileListView view

class CProfileListView : public CListView
{
protected:
	CProfileListView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CProfileListView)

// Attributes
public:
	CImageList m_imageList;
	int m_iSort;
	int *m_piSortInd;

	BOOL m_bProcessedContextMenu;	// used to prevent context menu from being called twice

// Operations
public:
	CDiscoveryDoc* GetDocument();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProfileListView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CProfileListView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CProfileListView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnQsdStartSingle();
	afx_msg void OnUpdateQsdStartSingle(CCmdUI* pCmdUI);
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CProfileGraphView view

class CProfileGraphView : public CView
{
protected:
	CProfileGraphView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CProfileGraphView)

// Attributes
public:
	float m_afXRange[2], m_afYRange[2];

// Operations
public:
	CDiscoveryDoc* GetDocument();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProfileGraphView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CProfileGraphView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CProfileGraphView)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CQsdGraphView view

class CQsdGraphView : public CView
{
protected:
	CQsdGraphView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CQsdGraphView)

// Attributes
public:

// Operations
public:
	CDiscoveryDoc* GetDocument();

	void DrawSelection (CDC*, LPCRECT);	// draw XOR rectangle
	int DrawSelections (CDC*);			// draw XOR rectangles for all selected datapoints
	void DrawSlabRect (CDC*);			// draw XOR rectangle when in slab definition mode
	void MonitorSlabRange (CDC*, float[2]);	// write out current slab range in XOR mode
	BOOL GetSlabRange (float[2]);		// converts current slab rectangle into range values
	BOOL Is2DProjection (void);

private:
	int* m_piSort;		// indexes of the z-sorted datapoints
	MAT44 m_omat;		// current orientation matrix
	float m_fScale;		// factor view coordinates -> client coordinates
	CPoint m_ptShift;	// shift of center of rotation relative to center of drawing area
	BOOL m_bBalls;		// TRUE if datapoints are balls, FALSE if they are dots
	BOOL m_bGrid;		// draw grid on the back wall in 2D projection?
	int m_iTracking;	// > 0 indicates tracking while left mouse button down
	BOOL m_bTracking;	// TRUE while user is tracking with mouse button down
	CPoint m_ptLastMouse;	// for calculation of tracking amounts
	BOOL m_bSlabMode;	// TRUE when user is defining a slab along the vertical axis
	BOOL m_bSlabActive;	// TRUE after definition of a slab to indicate that points are clipped
	CRect m_rectSlab;	// stores the XOR rectangle when defining a slab
	CRect m_rectCube2D;	// stores the rectangle of the back wall when in 2D projection
	CRect m_rectDraw;	// drawing area
	int m_iVertAxis;	// identifies type and orientation of the vertical axis
//	int m_iSlabAxis;	// 1=x-, 2=y-, 3=z-axis is orthogonal to slab
//	float m_afSlabRange[2];	// minimum and maximum value of the slab in axis-units
	int UpdateViewCoords (float[8][3], float[6][3], float[4][3], float[6]);
	void Rotate (UINT, float[3]);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQsdGraphView)
	public:
	virtual void OnInitialUpdate();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CQsdGraphView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CQsdGraphView)
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnViewQsdAdjust();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnViewQsdProjXY();
	afx_msg void OnViewQsdProjXZ();
	afx_msg void OnViewQsdProjYZ();
	afx_msg void OnViewQsdDots();
	afx_msg void OnUpdateViewQsdDots(CCmdUI* pCmdUI);
	afx_msg void OnViewQsdBalls();
	afx_msg void OnUpdateViewQsdBalls(CCmdUI* pCmdUI);
	afx_msg void OnViewRotXY();
	afx_msg void OnUpdateViewRotXY(CCmdUI* pCmdUI);
	afx_msg void OnViewRotZ();
	afx_msg void OnUpdateViewRotZ(CCmdUI* pCmdUI);
	afx_msg void OnViewShift();
	afx_msg void OnUpdateViewShift(CCmdUI* pCmdUI);
	afx_msg void OnViewEnlarge();
	afx_msg void OnUpdateViewEnlarge(CCmdUI* pCmdUI);
	afx_msg void OnViewNoTrack();
	afx_msg void OnUpdateViewNoTrack(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnViewQsdGrid();
	afx_msg void OnUpdateViewQsdGrid(CCmdUI* pCmdUI);
	afx_msg void OnViewQsdProjClosest();
	afx_msg void OnViewQsdSlab();
	afx_msg void OnUpdateViewQsdSlab(CCmdUI* pCmdUI);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnViewQsdNoSlab();
	afx_msg void OnUpdateViewQsdNoSlab(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectInv();
	afx_msg void OnEditSelectAll();
	afx_msg void OnUpdateViewQsdProjXZ(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewQsdProjYZ(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewQsdProjClosest(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CQsdListView view

class CQsdListView : public CListView
{
protected:
	CQsdListView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CQsdListView)

// Attributes
public:
	CImageList m_imageList;
	int m_iSort;
	int *m_piSortInd;

	BOOL m_bProcessedContextMenu;	// used to prevent context menu from being called twice
	BOOL m_bNoItemChangedMsg;		// see comment in OnItemChanged()

// Operations
public:
	CDiscoveryDoc* GetDocument();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQsdListView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CQsdListView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CQsdListView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnEditSelectAll();
	afx_msg void OnEditSelectInv();
	afx_msg void OnFileSaveData();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CQsdLegendView view

class CQsdLegendView : public CEditView
{
protected:
	CQsdLegendView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CQsdLegendView)

// Attributes
public:

// Operations
public:
	CDiscoveryDoc* GetDocument();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQsdLegendView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CQsdLegendView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CQsdLegendView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CQsdHistogramView view

class CQsdHistogramView : public CView
{
protected:
	CQsdHistogramView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CQsdHistogramView)

// Attributes
public:
	float m_fBorderY;		// y-coordinate (0..100) of horizontal border line
	float m_fYMin, m_fYMax;	// y-coordinate of histogram bottom and top, rsp., usually 0 and 100, rsp.
	CRect m_rectHisto;		// saves the histogram rectangle for mouse message handlers
	CRect m_rectSplit;		// used when shifting horizontal border
	BOOL m_bSplitTracking;
	BOOL m_bZoom;

// Operations
public:
	CDiscoveryDoc* GetDocument();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQsdHistogramView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CQsdHistogramView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CQsdHistogramView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnViewQsdHistoFull();
	afx_msg void OnUpdateViewQsdHistoFull(CCmdUI* pCmdUI);
	afx_msg void OnViewQsdHistoZoom();
	afx_msg void OnUpdateViewQsdHistoZoom(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
