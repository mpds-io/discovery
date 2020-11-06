// MainFrm.cpp : implementation of the CMainFrame class
// Created: 11.01.1999  -  Updated: 23.03.2001
// Ported to Windows 8.1 by Fulcrum.013 Mariupol, Ukraine 
// 17 september 2020

#define  _CRT_SECURE_NO_WARNINGS 
#include "stdafx.h"
#include "Discovery.h"

#include "MainFrm.h"
#include "DiscoveryDoc.h"
#include "DiscoveryView.h"
#include "ChemView.h"
#include "QsdViews.h"

#include "QsdRunDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_VIEW_QSD, OnViewQsd)
	ON_UPDATE_COMMAND_UI(ID_VIEW_QSD, OnUpdateViewQsd)
	ON_COMMAND(ID_VIEW_REPORT, OnViewReport)
	ON_UPDATE_COMMAND_UI(ID_VIEW_REPORT, OnUpdateViewReport)
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_COMMAND(ID_QSD_START, OnQsdStart)
	ON_UPDATE_COMMAND_UI(ID_QSD_START, OnUpdateQsdStart)
	ON_COMMAND(ID_VIEW_CHEMISTRY, OnViewChemistry)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CHEMISTRY, OnUpdateViewChemistry)
	ON_COMMAND(ID_VIEW_QSD_PROFILE, OnViewQsdProfile)
	ON_UPDATE_COMMAND_UI(ID_VIEW_QSD_PROFILE, OnUpdateViewQsdProfile)
	ON_COMMAND(ID_VIEW_QSD_CUBE, OnViewQsdCube)
	ON_UPDATE_COMMAND_UI(ID_VIEW_QSD_CUBE, OnUpdateViewQsdCube)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_COUNT, OnUpdateIndicatorCount)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_SEL, OnUpdateIndicatorSel)
	//}}AFX_MSG_MAP
	// Global help commands
	ON_COMMAND(ID_QSD_START_DIRECT, OnQsdStartDirect)
	ON_COMMAND(ID_HELP_FINDER, CFrameWnd::OnHelpFinder)
	ON_COMMAND(ID_HELP, CFrameWnd::OnHelp)
	ON_COMMAND(ID_CONTEXT_HELP, CFrameWnd::OnContextHelp)
	ON_COMMAND(ID_DEFAULT_HELP, CFrameWnd::OnHelpFinder)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_COUNT,		// total number of QSD Discovery Space datapoints
	ID_INDICATOR_SEL		// number of currently selected datapoints
/*	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,	*/
};

static const char s_szApplication[]="Application",
				  s_szQsd[]="Qsd";

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	m_bMaximizeFrame = FALSE;
	m_iViewMode		 = VIEW_REPORT;	// default view when app starts
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	if (!m_wndToolBar.Create(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	// Normally the two panes that show the number of (selected) datapoints of
	// the QSD Discovery Space would aquire half of the statusbar's width.
	// Here we fix the widths of the two panes at the right end of the statusbar.
	UINT uid, ustyle;
	int iwidth;
	m_wndStatusBar.GetPaneInfo(1, uid, ustyle, iwidth);
	m_wndStatusBar.SetPaneInfo(1, uid, ustyle, 100);
	m_wndStatusBar.GetPaneInfo(2, uid, ustyle, iwidth);
	m_wndStatusBar.SetPaneInfo(2, uid, ustyle, 80);

	// TODO: Remove this if you don't want tool tips or a resizeable toolbar
	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

	// TODO: Delete these three lines if you don't want the toolbar to
	//  be dockable
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);

	// Retrieve latest toolbar position from registry
	LoadBarState(s_szApplication);

	return 0;
}

BOOL CMainFrame::OnCreateClient( LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	TRACE("Entering CMainFrame::OnCreateClient()\n");

	// Create a static splitter window made up of two columns.
	// The first (left) column contains the main graphics, whereas the right one
	// contains a list or a Results Report. The right pane is not used (and thus
	// hidden) in Report View Mode.
	VERIFY( m_wndSplitter.CreateStatic(this, 1, 2) );

	// Get main frame rectangle to determine widths of the two horizontal panes.
	// Since Report View is the first view when starting the application, the first
	// pane containing the list control will have full width.
	CRect rectClient;
	GetClientRect(&rectClient);
	CSize sizePane(::GetSystemMetrics(SM_CXSCREEN), rectClient.Height());

	// Create default view (report view of type CDiscoveryView) in left column
	VERIFY( m_wndSplitter.CreateView(0, 0, pContext->m_pNewViewClass, sizePane, pContext) );

	// The right pane will be splitted in two rows. These two panes will only be
	// used for QSD, and here initialized with simple edit views.
	VERIFY( m_wndSplitter2.CreateStatic(&m_wndSplitter,	// the main splitter as parent
										2,				// number of rows
										1,				// number of columns
										WS_CHILD | WS_VISIBLE,
										m_wndSplitter.IdFromRowCol(0,1)) );

	VERIFY( m_wndSplitter2.CreateView(0, 0, RUNTIME_CLASS(CEditView),
									  CSize(0,rectClient.Height()), pContext) );
	VERIFY( m_wndSplitter2.CreateView(1, 0, RUNTIME_CLASS(CEditView),
									  CSize(0,0), pContext) );
	//VERIFY( m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CEditView), CSize(0,0), pContext) );

	// Let the first pane be the default pane
	SetActiveView((CView*)m_wndSplitter.GetPane(0,0));

	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	// Retrieve latest size and position of frame window from registry.
	// If not available (first execution after installation), show frame window
	// with 800 x 600 pixels wide and high, rsp., in the upper left corner of
	// the screen.
	WINDOWPLACEMENT wp;
	wp.length			= sizeof(WINDOWPLACEMENT);
	wp.flags			= 0;
	wp.showCmd			= SW_SHOWNORMAL;
	wp.ptMinPosition	= CPoint(-1,-1);
	wp.ptMaxPosition	= CPoint(-1,-1);
	wp.rcNormalPosition	= CRect(0,0,800,600);

	CString strMainWindow = AfxGetApp()->GetProfileString(s_szApplication,"MainWindow");
	if ( !strMainWindow.IsEmpty() )
	{
		if (sscanf(strMainWindow, "%u,%u,%d,%d,%d,%d,%d,%d,%d,%d",
			&wp.flags, &wp.showCmd,
			&wp.ptMinPosition.x, &wp.ptMinPosition.y,
			&wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
			&wp.rcNormalPosition.left, &wp.rcNormalPosition.top,
			&wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom) == 10)
		{
			cs.cx		= wp.rcNormalPosition.right  - wp.rcNormalPosition.left;
			cs.cy		= wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
			cs.x		= wp.rcNormalPosition.left;
			cs.y		= wp.rcNormalPosition.top;
			if ((wp.showCmd & SW_SHOWMAXIMIZED) == SW_SHOWMAXIMIZED)
				m_bMaximizeFrame = TRUE;
		}
	};

	return CFrameWnd::PreCreateWindow(cs);
}

// Adjust size and position of report view after toolbar has moved
void CMainFrame::RecalcLayout(BOOL bNotify) 
{
	// TODO: Add your specialized code here and/or call the base class
	TRACE("Entering CMainFrame::RecalcLayout()\n");
	
	CFrameWnd::RecalcLayout(bNotify);

	// Compare CMainFrame::OnSize()
	if (m_iViewMode == VIEW_REPORT || m_iViewMode == VIEW_CHEMISTRY)
	{
		if ( ::IsWindow(m_wndSplitter.GetSafeHwnd()) )
		{
			if ( GetActiveView() )
			{
				MaximizePane();
			}
		}
	}
}

// Document title disappears from the frame window's title bar after single or
// prediction run. Reason is that GetActiveView() returns NULL (don't know why)
// and thus OnUpdateFrameTitle() cannot access the document's <m_strTitle>.
void CMainFrame::OnUpdateFrameTitle(BOOL bAddToTitle)
{
	CView* pView	= GetActiveView();
	CDocument* pDoc	= NULL;
	if (pView != NULL)
		pDoc = pView->GetDocument();

	//TRACE("CMainFrame::OnUpdateFrameTitle() - pView=%X, pDoc=%X\n", pView, pDoc);

	if (pDoc != NULL)
		CFrameWnd::OnUpdateFrameTitle(bAddToTitle);
};


/////////////////////////////////////////////////////////////////////////////
// CMainFrame operations

// Central function to switch to the view mode given in <iNewMode>.
// ....
// If <bWizard> is TRUE, open the wizard to select run mode for QSD.
// Returns TRUE if switched to new view, FALSE if view <iNewMode> has still been active.

BOOL CMainFrame::SwitchToViewMode (int iNewMode, BOOL bWizard /*=FALSE*/)
{
	ASSERT( iNewMode == VIEW_REPORT		|| iNewMode == VIEW_QSD			||
			iNewMode == VIEW_CHEMISTRY	|| iNewMode == VIEW_STATISTICS);

	if (m_iViewMode == iNewMode)
		return FALSE;

	CView* pOldView = GetActiveView();
	pOldView->ShowWindow(SW_HIDE);

	CCreateContext context;
	context.m_pCurrentDoc = GetActiveDocument();

	if (iNewMode == VIEW_QSD)
	{
		m_wndSplitter.DeleteView(0,0);

		VERIFY( m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CProfileGraphView),
										 CSize(200,100), &context) );

		// Insert QSD menu between View and Help menu and exchange some toolbar buttons
		UpdateMenu(VIEW_QSD);
		UpdateToolbar(VIEW_QSD);

		m_wndSplitter.SetColumnInfo(0, 400, 50);	// TEST
		m_wndSplitter.SetActivePane(0,0);
		m_wndSplitter.RecalcLayout();

		RecalcLayout();

		if ( bWizard )
			PostMessage(WM_COMMAND, ID_QSD_START);
	}
	else if (iNewMode == VIEW_REPORT || iNewMode == VIEW_CHEMISTRY)
	{
		// Get splitter window's size to expand first pane to full size
		CRect rectClient;
		GetClientRect(&rectClient);
		CSize sizePane(::GetSystemMetrics(SM_CXSCREEN), rectClient.Height());

		m_wndSplitter.DeleteView(0, 0);

		if (iNewMode == VIEW_REPORT)
		{
			VERIFY( m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CDiscoveryView),
											 sizePane, &context) );
		}
		else if (iNewMode == VIEW_CHEMISTRY)
		{
			VERIFY( m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CChemView),
											 sizePane, &context) );
		};

		UpdateMenu(iNewMode);
		UpdateToolbar(iNewMode);

		m_wndSplitter.SetColumnInfo(1, 0, 0);
		m_wndSplitter.SetActivePane(0, 0);
		RecalcLayout();

		// Expand the only view (the list control in pane 0) to the splitter window's size
		MaximizePane();
	};

	// This is to call the new view's OnInitialUpdate() method
	InitialUpdateFrame(NULL, TRUE);

	m_iViewMode = iNewMode;

	return TRUE;
};

// Enlarge view in first pane to splitter window's size
void CMainFrame::MaximizePane (void)
{
	CRect rectClient, rectSplitter;
	GetClientRect(&rectClient);
	m_wndSplitter.GetWindowRect(&rectSplitter);
	rectClient.bottom = rectClient.top  + rectSplitter.Height();
	rectClient.right  = rectClient.left + rectSplitter.Width();

	CView* pView = (CView*)m_wndSplitter.GetPane(0,0);
	pView->MoveWindow(&rectClient);
};

// Adjust the sizes of the three panes in QSD view. The graphics pane (left pane)
// should be half as wide as the application window's client area. The upper right
// pane should show approximately 20 rows in the table.
void CMainFrame::AdjustPaneSizes (void)
{
	CRect rectClient, rectItem;
	int iheight;
	GetClientRect(&rectClient);
	CListView* pListView = (CListView*)m_wndSplitter2.GetPane(0,0);
	ASSERT_KINDOF(CListView, pListView);
	if ( pListView->GetListCtrl().GetItemRect(0, &rectItem, LVIR_LABEL) )
		iheight = rectItem.Height() * 20;
	else
		iheight = 200;
	m_wndSplitter2.SetRowInfo(0, iheight, 50);
	m_wndSplitter2.RecalcLayout();
	m_wndSplitter.SetColumnInfo(0, rectClient.Width()/2, 50);
	m_wndSplitter.RecalcLayout();
};

// Add a pulldown menu after the View menu, which contains specific functions
// for the view type <iNewMode>.
BOOL CMainFrame::UpdateMenu (int iNewMode)
{
	// First check if there is a pulldown menu from the previous view mode.
	// (When the app starts, there are only File, Edit, View, and Help.)
	// If there are more, remove all between View and Help.
	// Note: Since CMenu::DeleteMenu() and CMenu::InsertMenu() do not work properly,
	// I access the HMENU directly.
	HMENU hMenu = ::GetMenu(GetSafeHwnd());
	int nItems	= ::GetMenuItemCount(hMenu);
	ASSERT( nItems == 4 || nItems == 5 );
	if (nItems == 5)
	{
#ifdef _DEBUG
		if ( !::DeleteMenu(hMenu, 3, MF_BYPOSITION) )
		{
			LPVOID lpMsgBuf;
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL);

			// Display the string.
			::MessageBox( NULL, (LPTSTR)lpMsgBuf, "GetLastError", MB_OK|MB_ICONINFORMATION );

			// Free the buffer.
			LocalFree( lpMsgBuf );
		}
#else
		::DeleteMenu(hMenu, 3, MF_BYPOSITION);
#endif
	};

	// Now load the new menu from the resources and insert it after the View menu
	int iid;
	CString strText;
	if		(iNewMode == VIEW_REPORT)
	{
		iid		= IDR_REPORT;
		strText	= "&Report";
	}
	else if (iNewMode == VIEW_QSD)
	{
		iid		= IDR_QSD;
		strText	= "&QSD";
	}
	else if (iNewMode == VIEW_CHEMISTRY)
	{
		iid		= IDR_CHEMISTRY;
		strText	= "&Chemistry";
	}
	else if (iNewMode == VIEW_STATISTICS)
	{
		iid		= IDR_STATISTICS;
		strText	= "&Statistics";
	}
	else
	{
		TRACE("CMainFrame::UpdateMenu() - illegal view mode for <iNewMode>\n");
		return FALSE;
	};

	// Note: Using CMenu objects rather than HMENU directly works on NT 4.0 but
	// crashes on Win95, so I use HMENU's here, too.
	//CMenu NewMenu;
	//VERIFY( NewMenu.LoadMenu(iid) );
	//CMenu *pSubMenu = NewMenu.GetSubMenu(0);
	//ASSERT( pSubMenu != NULL );
	HMENU hNewMenu = ::LoadMenu(AfxGetInstanceHandle(), MAKEINTRESOURCE(iid));
	ASSERT( ::IsMenu(hNewMenu) );
	HMENU hSubMenu = ::GetSubMenu(hNewMenu, 0);
	ASSERT( ::IsMenu(hSubMenu) );

	// Now insert the new pulldown menu between View and Help menu
	::InsertMenu(hMenu, 3, MF_BYPOSITION | MF_POPUP, (UINT)hSubMenu/*pSubMenu->Detach()*/, strText);

	// Redraw menu bar
	DrawMenuBar();

	return TRUE;
};

// Exchange some buttons and/or comboboxes for the new view that is identified
// by <iNewMode>.
// Returns TRUE if successful, otherwise FALSE.
BOOL CMainFrame::UpdateToolbar (int iNewMode)
{
	CToolBarCtrl& tb = m_wndToolBar.GetToolBarCtrl();

	// Exchange the command ID for the options button
	int ind = tb.CommandToIndex((m_iViewMode == VIEW_REPORT)	? ID_REPORT_OPTIONS :
								(m_iViewMode == VIEW_CHEMISTRY)	? ID_CHEM_OPTIONS   :
								(m_iViewMode == VIEW_QSD)		? ID_QSD_OPTIONS    :
																  ID_STAT_OPTIONS);
	if (ind > 0)
	{
		tb.SetCmdID(ind, (iNewMode == VIEW_REPORT)		? ID_REPORT_OPTIONS :
						 (iNewMode == VIEW_CHEMISTRY)	? ID_CHEM_OPTIONS	:
						 (iNewMode == VIEW_QSD)			? ID_QSD_OPTIONS	: ID_STAT_OPTIONS);
	};

	// Remove other controls right from the Options button...
	// ....

	return TRUE;
};

// Defines which panes become visible in QSD view mode.
//   iGraph=1 : Graphical representation of the profiles in left pane,
//   iGraph=2 : Graphical representation of the Discovery Space;
//   iText=1  : List of profiles in upper right pane,
//   iText=2  : List of system properties and their coordinate triplets (Discovery Space),
//   iText=3  : Results Report View for multiple runs,
//   iText=4  : Results Report View for single run;
//   iGraph2=1: Legend for property/operation indexes (taken from RESULTS.DAT),
//   iGraph2=2: Histogram representation,
//   iGraph2=0: Hide the histogram pane.

BOOL CMainFrame::SetQsdViews (int iGraph, int iText, int iGraph2)
{
	BOOL bChange;
	// Change view in graphics pane only if different from current view and if wanted.
	// (<iGraph> == 0 leaves the current view unchanged.)
	CView* pView = (CView*)m_wndSplitter.GetPane(0,0);
	bChange		 = ((iGraph >= 1 && iGraph <= 2) &&
					(pView->IsKindOf(RUNTIME_CLASS(CProfileGraphView)) && iGraph != 1 ||
					 pView->IsKindOf(RUNTIME_CLASS(CQsdGraphView))     && iGraph != 2));
	if ( bChange )
	{
		CCreateContext context;
		context.m_pCurrentDoc = pView->GetDocument();

		// Hide the old view and destroy it from the pane.
		// For a view, the <m_bAutoDelete> member is TRUE, so that the view object
		// will be deleted from within CView::OnNcDestroy().
		CRect rectView;
		pView->GetWindowRect(&rectView);
		ScreenToClient(&rectView);
		pView->ShowWindow(SW_HIDE);
		pView->DestroyWindow();

		// Create and check the new view
		CView* pNewView;
		if (iGraph == 1)
		{
			m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CProfileGraphView),
				CSize(rectView.Width(), rectView.Height()), &context);
			pNewView = (CView*)m_wndSplitter.GetPane(0,0);
			ASSERT_KINDOF( CProfileGraphView, pNewView );
		}
		else	// iGraph == 2
		{
			m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CQsdGraphView),
				CSize(rectView.Width(), rectView.Height()), &context);
			pNewView = (CView*)m_wndSplitter.GetPane(0,0);
			ASSERT_KINDOF( CQsdGraphView, pNewView );
		};

		pNewView->SetWindowPos(0, rectView.left, rectView.top, 0, 0,
			SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);
		pNewView->OnInitialUpdate();
	};

	// Change view in list pane only if different from current view and if wanted.
	// (<iText> == 0 leaves the current view unchanged.)
	pView	= (CView*)m_wndSplitter2.GetPane(0,0);
	bChange	= ((iText >= 1 && iText <= 4) &&
			   (pView->IsKindOf(RUNTIME_CLASS(CProfileListView)) && iText != 1 ||
				pView->IsKindOf(RUNTIME_CLASS(CQsdListView))     && iText != 2 ||
				pView->IsKindOf(RUNTIME_CLASS(CEditView))        && iText <  3));
	if ( bChange )
	{
		CCreateContext context;
		context.m_pCurrentDoc = pView->GetDocument();

		CRect rectView;
		pView->GetWindowRect(&rectView);
		ScreenToClient(&rectView);
		pView->ShowWindow(SW_HIDE);
		pView->DestroyWindow();

		CView* pNewView;
		if (iText == 1)
		{
			m_wndSplitter2.CreateView(0, 0, RUNTIME_CLASS(CProfileListView),
				CSize(rectView.Width(), rectView.Height()), &context);
			pNewView = (CView*)m_wndSplitter2.GetPane(0,0);
			ASSERT_KINDOF( CProfileListView, pNewView );
		}
		else if (iText == 2)
		{
			m_wndSplitter2.CreateView(0, 0, RUNTIME_CLASS(CQsdListView),
				CSize(rectView.Width(), rectView.Height()), &context);
			pNewView = (CView*)m_wndSplitter2.GetPane(0,0);
			ASSERT_KINDOF( CQsdListView, pNewView );
		}
		else	// iText == 3 || iText == 4
		{
			m_wndSplitter2.CreateView(0, 0, RUNTIME_CLASS(CEditView),
				CSize(rectView.Width(), rectView.Height()), &context);
			pNewView = (CView*)m_wndSplitter2.GetPane(0,0);
			ASSERT_KINDOF( CEditView, pNewView );
		};

		pNewView->SetWindowPos(0, rectView.left, rectView.top, 0, 0,
			SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);
		pNewView->OnInitialUpdate();
	};

	// Dito for auxiliary pane in the lower right pane.
	pView	= (CView*)m_wndSplitter2.GetPane(1,0);
	bChange	= (	(iGraph2 == 1 && !pView->IsKindOf(RUNTIME_CLASS(CQsdLegendView))) ||
				(iGraph2 == 2 && !pView->IsKindOf(RUNTIME_CLASS(CQsdHistogramView))) );
//	bChange	= ((iGraph2 >= 1 && iGraph2 <= 2) &&
//			   (pView->IsKindOf(RUNTIME_CLASS(CQsdLegendView))	  && iGraph2 != 1 ||
//			    pView->IsKindOf(RUNTIME_CLASS(CQsdHistogramView)) && iGraph2 != 2));
	if ( bChange )
	{
		CCreateContext context;
		context.m_pCurrentDoc = pView->GetDocument();

		CRect rectView;
		pView->GetWindowRect(&rectView);
		ScreenToClient(&rectView);
		pView->ShowWindow(SW_HIDE);
		pView->DestroyWindow();

		CView* pNewView;
		if (iGraph2 == 1)
		{
			m_wndSplitter2.CreateView(1, 0, RUNTIME_CLASS(CQsdLegendView),
				CSize(rectView.Width(), rectView.Height()), &context);
			pNewView = (CView*)m_wndSplitter2.GetPane(1,0);
			ASSERT_KINDOF( CQsdLegendView, pNewView );
		}
		else	// iGraph2 == 2
		{
			m_wndSplitter2.CreateView(1, 0, RUNTIME_CLASS(CQsdHistogramView),
				CSize(rectView.Width(), rectView.Height()), &context);
			pNewView = (CView*)m_wndSplitter2.GetPane(1,0);
			ASSERT_KINDOF( CQsdHistogramView, pNewView );
		};

		pNewView->SetWindowPos(0, rectView.left, rectView.top, 0, 0,
			SWP_SHOWWINDOW | SWP_NOZORDER | SWP_NOSIZE);
		pNewView->OnInitialUpdate();
	};

	RecalcLayout();

	return TRUE;
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnSize(UINT nType, int cx, int cy) 
{
	TRACE("Entering CMainFrame::OnSize() - cx=%d, cy=%d\n", cx, cy);

	CFrameWnd::OnSize(nType, cx, cy);

	// Enlarge the only useful pane in Report View Mode to the client area's size.
	// This hides the splitter bar.
	if (m_iViewMode == VIEW_REPORT || m_iViewMode == VIEW_CHEMISTRY)
	{
		// Since WM_SIZE is sent multiple times when the frame window is being created
		// when starting the application, check if there is already a valid view.
		if ( ::IsWindow(m_wndSplitter.GetSafeHwnd()) )
		{
			if ( GetActiveView() )
			{
				MaximizePane();
			}
		}
	}
}

void CMainFrame::OnClose() 
{
	// Save toolbar state and size and position of frame window in registry
	SaveBarState(s_szApplication);

	char szHelp[512];
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	if ( GetWindowPlacement(&wp) )
	{
		wp.flags = 0;
		if ( IsZoomed() )
			wp.flags |= WPF_RESTORETOMAXIMIZED;
		sprintf(szHelp, "%u,%u,%d,%d,%d,%d,%d,%d,%d,%d",
			wp.flags, wp.showCmd,
			wp.ptMinPosition.x, wp.ptMinPosition.y,
			wp.ptMaxPosition.x, wp.ptMaxPosition.y,
			wp.rcNormalPosition.left,  wp.rcNormalPosition.top,
			wp.rcNormalPosition.right, wp.rcNormalPosition.bottom);
		AfxGetApp()->WriteProfileString(s_szApplication,"MainWindow",szHelp);
	};
	
	CFrameWnd::OnClose();
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame commands

// Change to QSD view
void CMainFrame::OnViewQsd() 
{
	SwitchToViewMode(VIEW_QSD);
}

void CMainFrame::OnUpdateViewQsd(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(m_iViewMode == VIEW_QSD);
}

// Change to Chemistry view
void CMainFrame::OnViewChemistry() 
{
//	SwitchToViewMode(VIEW_CHEMISTRY);
}

void CMainFrame::OnUpdateViewChemistry(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(FALSE);
//	pCmdUI->SetRadio(m_iViewMode == VIEW_CHEMISTRY);
}

void CMainFrame::OnViewReport() 
{
//	SwitchToViewMode(VIEW_REPORT);
}

void CMainFrame::OnUpdateViewReport(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(FALSE);
//	pCmdUI->SetRadio(m_iViewMode == VIEW_REPORT);
}

void CMainFrame::OnQsdStart() 
{
	CDiscoveryApp* pApp = (CDiscoveryApp*)AfxGetApp();

	CDiscoveryDoc* pDoc = (CDiscoveryDoc*)GetActiveDocument();
	ASSERT_VALID(pDoc);

	// Switch to QSD view mode if not yet active
	if (m_iViewMode != VIEW_QSD)
		SwitchToViewMode(VIEW_QSD);

	// Open the wizard to start either a QSD single run or multiple runs.
	// Preset that run type the user has selected for the latest run.
	CQsdRunSheet Sheet(IDS_RUN_QSD_SHEET_TITLE);
	Sheet.m_SelPage.m_iRunType	= pApp->GetProfileInt(s_szQsd, "RunType", 0);
	Sheet.m_SelPage.m_iLevel	= pApp->GetProfileInt(s_szQsd, "Level", 0);

	// Settings for the Configuration page
	Sheet.m_ConfPage.m_iNum		= pApp->GetProfileInt(s_szQsd, "NumBestResults", 20);	// TO BE CHECKED
	Sheet.m_ConfPage.m_iMax		= pApp->GetProfileInt(s_szQsd, "MaxNeighbours", 50);	// TO BE CHECKED
	Sheet.m_ConfPage.m_bError	= (BOOL)pApp->GetProfileInt(s_szQsd, "NeighbourError", 1);	// TO BE CHECKED
	Sheet.m_ConfPage.m_iQuality	= pApp->GetProfileInt(s_szQsd, "MinQuality", 95);
//	Sheet.m_ConfPage.m_iLevel	= pApp->GetProfileInt(s_szQsd, "Level", 0);

	// In any case, set the Select run page to be the first page of the wizard
	Sheet.AddPage(&Sheet.m_SelPage);

	// Dependent if a single or a multiple run is pre-set, set the single run or
	// multiple run page, rsp., as second page.
	// Note: There are different multiple runs now: "2-step scan" is what we previously
	// called "Multiple Run"; "fast scan" and "precise scan" have been added.
	pDoc->m_iRunType = Sheet.m_SelPage.m_iRunType;
	if (pDoc->m_iRunType == 0)		// "Search" = multiple run
	{
		Sheet.AddPage(&Sheet.m_MultPage);
	}
	else	// m_iRunType == 1 || m_iRunType == 2 (single run or prediction)
	{
		Sheet.AddPage(&Sheet.m_SinglePage);
		Sheet.m_SinglePage.m_bPredict = (pDoc->m_iRunType == 2);
	};

	// Set the configuration page as third and last page
	Sheet.AddPage(&Sheet.m_ConfPage);

	// No Help button
	Sheet.m_SelPage.m_psp.dwFlags		&= ~PSP_HASHELP;
	Sheet.m_SinglePage.m_psp.dwFlags	&= ~PSP_HASHELP;
	Sheet.m_MultPage.m_psp.dwFlags		&= ~PSP_HASHELP;
	Sheet.m_ConfPage.m_psp.dwFlags		&= ~PSP_HASHELP;
	Sheet.m_psh.dwFlags					&= ~PSH_HASHELP;

	// Make the default property sheet a wizard. In this case, CPropertySheet::DoModal()
	// will return ID_WIZFINISH when the user clicks the Finish button on the last page
	// rather than IDOK.
	Sheet.SetWizardMode();
	if (Sheet.DoModal() == ID_WIZFINISH)
	{
		pApp->WriteProfileInt(s_szQsd, "RunType", Sheet.m_SelPage.m_iRunType);
		pApp->WriteProfileInt(s_szQsd, "NumBestResults", Sheet.m_ConfPage.m_iNum);
		pApp->WriteProfileInt(s_szQsd, "MaxNeighbours", Sheet.m_ConfPage.m_iMax);
		pApp->WriteProfileInt(s_szQsd, "NeighbourError", (int)Sheet.m_ConfPage.m_bError);
		pApp->WriteProfileInt(s_szQsd, "MinQuality", Sheet.m_ConfPage.m_iQuality);
		pApp->WriteProfileInt(s_szQsd, "Level", Sheet.m_SelPage.m_iLevel);

		// Check if main input file for the QSD kernel is already available
		// (the records are stored in CDiscoveryDoc::m_QsdRecords). If not,
		// create them from the column that has been selected in the Report View.
		if (pDoc->m_QsdRecords.GetSize() <= 0)
		{
			pDoc->CreateQsdRecords();
		};

		pDoc->m_iRunType = Sheet.m_SelPage.m_iRunType;
		BOOL bMultiple	 = (pDoc->m_iRunType == 0);

		// Write configuration file QSD.CFG. This file has the format:
		//   PropertyFilename=PROPERTY.DAT
		//   CompoundFilename=QSDINP.DAT
		//   DiscoverySpaceFilename=QSDOUT.DAT
		//   Profilename=PROFILES.DAT				[use another name for single run]
		//   NumBestResults=20
		//   WriteDiscoverySpace=0					[-1 for single run]
		//   MaxNeighbours=50
		//   NeighbourError=0
		//   AutoRun=2								[1 for multiple run]
		//   Operators=1,2,3,4,5					[multiple run only]
		//   Properties=1,2,3,4,5,6					[multiple run only]
		//   SingleTask=01-01 / 02-03 / 04-02		[single run only]
		//   Level=1								[1=system level, 2=compound level]

		// The configuration file is simply "QSD.CFG" and is placed in the same
		// directory as QSD.EXE. The same is for the actual input file.
		CString strLine;
		CStdioFile f;
		CFileException e;
		CString strCfgPath	= pApp->m_strQsdExePath;	// ends with backslash
		strCfgPath		   += "QSD.CFG";
		CString strInpPath  = pApp->m_strQsdExePath;
		strInpPath		   += "QSDINP.DAT";
		if ( !f.Open(strCfgPath, CFile::modeWrite | CFile::modeCreate | CFile::typeText, &e) )
		{
			ReportFileError(&e);
			return;
		};

		// Path of the file with the elemental properties
		strLine  = "PropertyFilename=";
		strLine	+= pApp->m_strQsdPropertyFileName;
		f.WriteString(strLine);

		// Path of the file with the compound properties, here simply "QSDINP.DAT"
		strLine  = "\nCompoundFilename=";
		strLine += pApp->m_strQsdExePath;
		strLine += "QSDINP.DAT";
		f.WriteString(strLine);

		// Path of the file with the resulting discovery space, here simply "QSDOUT.DAT"
		strLine  = "\nDiscoverySpaceFilename=";
		strLine += pApp->m_strQsdExePath;
		strLine += "QSDOUT.DAT";
		f.WriteString(strLine);

		// Path to the file with the resulting profiles, here simply "PROFILES.DAT"
		strLine  = "\nProfilename=";
		strLine += pApp->m_strQsdExePath;
		strLine += "PROFILES.DAT";
		f.WriteString(strLine);

		// Refresh rate of writing output data into QSDOUT.DAT; must be -1 for a single run.
		strLine  = "\nWriteDiscoverySpace=";
		strLine	+= (bMultiple ? "0" : "-1");
		f.WriteString(strLine);

		// Following settings from the configuration page of the QSD Start property sheet
		strLine.Format("\nNumBestResults=%d\nMaxNeighbours=%d\nMinimumQuality=%d",
					   Sheet.m_ConfPage.m_iNum, Sheet.m_ConfPage.m_iMax,
					   Sheet.m_ConfPage.m_iQuality);
/*		strLine.Format("\nNumBestResults=%d\nMaxNeighbours=%d\nNeighbourError=%d"
					   "\nMinimumQuality=%d",
					   Sheet.m_ConfPage.m_iNum, Sheet.m_ConfPage.m_iMax,
					   (int)Sheet.m_ConfPage.m_bError, Sheet.m_ConfPage.m_iQuality);	*/
		f.WriteString(strLine);

		// "AutoRun" means "single run" (2) or "multiple runs" (1).
		// Single run with code 2 forces QSD.EXE to write out PROFILES.DAT.
		// Otherwise (code 0) profiles would not be written.
		strLine.Format("\nAutoRun=%d",	(pDoc->m_iRunType == 0) ? 1 :	// multiple run
										(pDoc->m_iRunType == 1) ? 2 :	// single run
																  3);	// prediction
//		strLine.Format("\nAutoRun=%d",	(pDoc->m_iRunType == 0) ? 4 :	// fast scan
//										(pDoc->m_iRunType == 1) ? 5 :	// precise scan
//										(pDoc->m_iRunType == 2) ? 1 :	// 2-step scan
//										(pDoc->m_iRunType == 3) ? 2 :	// single run with profiles
//																  3);	// prediction
		//strLine.Format("\nAutoRun=%d", bMultiple ? 1 : 2);
		f.WriteString(strLine);

		// "System Level" or "Compound Level"?
		strLine.Format("\nLevel=%d", Sheet.m_SelPage.m_iLevel+1);
		f.WriteString(strLine);

		// For multiple runs, we must write the indexes of the selected
		// properties and operators.
		char szHelp[64];
		register int i;
		if ( bMultiple )
		{
			ASSERT( Sheet.m_MultPage.m_nSelProps > 0     && Sheet.m_MultPage.m_nSelOps > 0 &&
					Sheet.m_MultPage.m_pSelProps != NULL && Sheet.m_MultPage.m_pSelOps != NULL );

			strLine = "\nOperators=";
			for (i=0; i < Sheet.m_MultPage.m_nSelOps; i++)
			{
				if (i > 0)
					strLine += ",";
				_itoa(Sheet.m_MultPage.m_pSelOps[i]+1, szHelp, 10);
				strLine += szHelp;
			};
			f.WriteString(strLine);

			strLine  = "\nProperties=";
			for (i=0; i < Sheet.m_MultPage.m_nSelProps; i++)
			{
				if (i > 0)
					strLine += ",";
				_itoa(Sheet.m_MultPage.m_pSelProps[i]+1, szHelp, 10);
				strLine += szHelp;
			};
			strLine += "\n";			// this is the last line
			f.WriteString(strLine);

			// Note: The indexes that have been allocated in CQsdMultipleRunPage::OnKillActive()
			// will be deleted in the destructor of CQsdMultipleRunPage.
		}

		// For a single run, we write three pairs of indexes (property and operation
		// index) for each of the axes x, y, and z. For x- and y-axis 0 is the first
		// valid property or the first valid operation, rsp., but for z-axis the first
		// option is "<none>" indicating "no property" for the z-axis, rsp.
		else
		{
			strLine.Format("\nSingleTask=%2.2d-%2.2d / %2.2d-%2.2d / %2.2d-%2.2d\n",
						   Sheet.m_SinglePage.m_iPropX+1, Sheet.m_SinglePage.m_iOpX+1,
						   Sheet.m_SinglePage.m_iPropY+1, Sheet.m_SinglePage.m_iOpY+1,
						   Sheet.m_SinglePage.m_iPropZ,   Sheet.m_SinglePage.m_iOpZ);
			f.WriteString(strLine);
		};

		f.Close();

		// So far for the configuration file.
		// If we make a prediction run, we will now read the compound data
		// from the file <Sheet.m_SinglePage.m_strFilename> and add them to
		// <CDiscoveryDoc.m_QsdRecords>
		if (pDoc->m_iRunType == 2)
		{
			ASSERT( !Sheet.m_SinglePage.m_strFilename.IsEmpty() );

			int irtc = pDoc->ImportQsdRecords(Sheet.m_SinglePage.m_strFilename,
											  Sheet.m_SelPage.m_iLevel+1);
			TRACE("CMainFrame::OnQsdStart() - CDiscoveryDoc::ImportQsdRecords() returns %d P-records\n",
				irtc);
			if (irtc == 0)
			{
				AfxMessageBox("Warning! No compounds (left) with properties to be predicted!");
			}
			else if (irtc < 0)	// TO BE CHECKED
			{
			}
		}
		else
		{
			// If the user made a prediction run before, there may be compounds
			// to be predicted in the QSD record array. These must be removed here.
			// (CDiscoveryDoc::ImportQsdRecords() removes these records before
			//  inserting the compounds to be predicted.)
			int irtc = pDoc->RemoveQsdRecords(1);
			
			TRACE("CMainFrame::OnQsdStart() - %d compound(s) to be predicted removed\n", irtc);
		};
		
		// Now write the actual input file with the compound property data.
		// This file consists of the records stored in m_QsdRecords[] and has the format:
		//   - line #0: "QSD",
		//   - line #1: Number of records in m_QsdRecords[],
		//   - line #2: Record #0 of m_QsdRecords[],
		//   - line #3: Record #1, etc.

		if ( !f.Open(strInpPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText, &e) )
		{
			ReportFileError(&e);
			return;
		};

		// Begin with "QSD" and the number of records
		int nRecords = pDoc->m_QsdRecords.GetSize();
		strLine.Format("QSD\n%d\n", nRecords);
		f.WriteString(strLine);

		// And now write the <nRecords> records for the residual <nRecords> lines
		for (i=0; i < nRecords; i++)
		{
			f.WriteString(pDoc->m_QsdRecords[i].Format(strLine));
		};

		f.Close();

		// Performing multiple QSD runs is normally a lengthy operation.
		// Since it makes no sense for the user to continue work with the current
		// document during that time, we handle this process in a modal dialog box.
		CRect rectClient;
		if ( bMultiple )
		{
			CQsdProgressDlg dlg;
			dlg.m_nRuns  = Sheet.m_MultPage.m_nSelProps * Sheet.m_MultPage.m_nSelOps;
			dlg.m_nBests = Sheet.m_ConfPage.m_iNum;

			// Returning IDOK signals that the dialog timer registered a normal
			// end of the working thread, while IDCANCEL is caused by either a
			// user break or by an error code from QSD.EXE.
			if (dlg.DoModal() == IDCANCEL)
			{
				// The "user break" message is displayed in <CQsdProgressDlg>
				//AfxMessageBox(IDS_QSD_RUN_CANCELED);
				return;
			};

			// Now read the results from the previous QSD run.
			// Note: In fast-scan mode, there is no profile.
			
			// The file "PROFILES.DAT" contains the "profiles" for the best runs, e.g.:
			//   "QSD PROFILE";1;2;3;4;5;6;7;8;9;10;...
			//   "01-01 / 01-02 / 02-02 ";5787(0);5156(4);...
			//   "01-01 / 01-02 / 02-01 ";5719(0);5193(1);...
			CString strFile;
			strFile  = pApp->m_strQsdExePath;
			strFile += "PROFILES.DAT";
			if (pDoc->m_iRunType == 0)
			{
				if (pDoc->ReadQsdProfiles(strFile, TRUE) < 0)
				{
					AfxMessageBox(IDS_QSD_CANNOT_READ_PROFILES);
					pDoc->m_iQsdSelProfile = -1;
					return;
				}
			}
			else
			{
				pDoc->m_QsdProfiles.RemoveAll();
				pDoc->m_iQsdSelProfile = -1;
			};

			// Read the lines of "RESULT.DAT". This will be the QSD Results Report.
			strFile  = pApp->m_strQsdExePath;
			strFile += "RESULTS.DAT";
			if ( !pDoc->ReadQsdResults(strFile) )
			{
				AfxMessageBox(IDS_QSD_CANNOT_READ_RESULTS);
				return;
			};

			// Select profile graphics in the left pane, the list of profiles in the
			// upper right pane. The results will get into the lower right pane.
			SetQsdViews(1, 1, 1);

			// Adust the sizes of the three panes. The graphics pane should have
			// half of the application client area's width.
			AfxGetMainWnd()->GetClientRect(&rectClient);
			m_wndSplitter2.SetRowInfo(0, 200, 50);
			m_wndSplitter2.RecalcLayout();
			m_wndSplitter.SetColumnInfo(0, rectClient.Width()/2, 50);
			m_wndSplitter.RecalcLayout();

			m_wndSplitter.SetActivePane(0,0);

			pDoc->UpdateAllViews(NULL);
			m_wndSplitter.RedrawWindow();
		}
		else	// bMultiple == FALSE
		{
			// Compare CQsdProgressDlg::OnInitDialog() how to call a working thread
			// for the QSD kernel QSD.EXE. Since we have no possibility to watch the
			// progress of the kernel for a single run, we simply wait for it to
			// finish and show the hourglass cursor.
			CWaitCursor curWait;
			char szExe[_MAX_PATH], szDir[_MAX_PATH], *cp;
			strcpy(szDir, pApp->m_strQsdExePath);
			strcpy(szExe, szDir);
			strcat(szExe, "QSD.EXE");
			if ((cp = strrchr(szDir, '\\')) != NULL)
				*cp = '\0';

			PROCESS_INFORMATION ProcessInfo;
			STARTUPINFO StartupInfo;
			memset(&StartupInfo, 0, sizeof(STARTUPINFO));
			StartupInfo.cb			= sizeof(STARTUPINFO);
			StartupInfo.dwFlags		= STARTF_USESHOWWINDOW;
			StartupInfo.wShowWindow	= SW_HIDE;

			BOOL bStat = CreateProcess(NULL, szExe, 0, 0, TRUE, 0, 0, szDir,
									   &StartupInfo, &ProcessInfo);

			// The difference from CQsdProgressDlg::OnInitDialog() is that we
			// wait here until the thread has terminated.
			// Compare CodeGuru at: http://www.codeguru.com/misc/process.shtml,
			// by: Joerg Koenig, Joerg.Koenig@rhein-neckar.de.
			if ( bStat )
			{
				DWORD dwExitCode = 0;
				HANDLE hProcess	 = (HANDLE)ProcessInfo.hProcess;
				WaitForSingleObject(hProcess, INFINITE);
				GetExitCodeProcess(hProcess, &dwExitCode);
				CloseHandle(hProcess);
				CloseHandle(ProcessInfo.hThread);

				TRACE("CMainFrame::OnQsdStart() - single run finished with exit code %u\n",
					dwExitCode);

				// An exit code of 1 indicates that QSD.EXE encountered an error.
				// Then we display the file "QSD.ERR" in a message box.
				if (dwExitCode == 1)
				{
					ReportQsdExeError();
					return;
				}
			}
			else
			{
				AfxMessageBox("Fatal Error:\nCannot execute QSD kernel for single run!");
				return;
			};

			// Read the single profile from the file "PROFILES.DAT"
			CString strProfile;
			strProfile  = pApp->m_strQsdExePath;
			strProfile += "PROFILES.DAT";
			if ( !pDoc->ReadQsdProfiles(strProfile, FALSE) )
			{
				AfxMessageBox(IDS_QSD_CANNOT_READ_PROFILES);
				pDoc->m_iQsdSelProfile = -1;
				return;
			};

			pDoc->m_iQsdSelProfile = 0;

			// Now read the results of the single run from file "QSDOUT.DAT"
			CString strOut;
			strOut  = pApp->m_strQsdExePath;
			strOut += "QSDOUT.DAT";
			if ( !pDoc->ReadQsdDiscoverySpace(strOut) )
			{
				AfxMessageBox(IDS_QSD_CANNOT_READ_DISCOVERYSPACE);
				return;
			};

			// Document filename in pplication window title bar disappears after
			// Single Run (don't know why).
			CString strTitle = pDoc->GetTitle();
			pDoc->SetTitle(strTitle);

			// Select Discovery Space graphics (the cube) in the left pane,
			// the coordinate list in the upper right pane, and the histogram
			// view of the profile in the lower right pane.
			SetQsdViews(2, 2, 2);

			// Adust the sizes of the three panes
			AdjustPaneSizes();

			m_wndSplitter.SetActivePane(0,0);
			ASSERT( GetActiveView() != NULL );

			pDoc->UpdateAllViews(NULL);
			m_wndSplitter.RedrawWindow();
		}
	}
}

void CMainFrame::OnUpdateQsdStart(CCmdUI* pCmdUI) 
{
	// The Start command is not available while a QSD process is running
	CDiscoveryDoc* pDoc = (CDiscoveryDoc*)GetActiveDocument();
	DWORD dwExitCode;
	GetExitCodeProcess(pDoc->m_hQsdProcess, &dwExitCode);
	pCmdUI->Enable(dwExitCode != STILL_ACTIVE);
}

// This is an indirect command that is not available from a menu. It is sent from
// CDiscoveryView::OnQsdStartSingle() for the selected profile whose index is
// stored in <CDiscoveryDoc::m_nQsdSelProfile>.
void CMainFrame::OnQsdStartDirect() 
{
	CDiscoveryApp* pApp = (CDiscoveryApp*)AfxGetApp();
	CDiscoveryDoc* pDoc = (CDiscoveryDoc*)GetActiveDocument();
	ASSERT_VALID( pDoc );
	ASSERT( pDoc->m_QsdRecords.GetSize() > 0 );	// must have been generated or loaded at that time
	ASSERT( pDoc->m_iQsdSelProfile >= 0 && pDoc->m_iQsdSelProfile < pDoc->m_QsdProfiles.GetSize() );

	// Write configuration file QSD.CFG. Compare CMainFrame::OnQsdStart().
	CString strLine;
	CStdioFile f;
	CFileException e;
	CString strCfgPath  = pApp->m_strQsdExePath;
	strCfgPath		   += "QSD.CFG";
	CString strInpPath	= pApp->m_strQsdExePath;
	strInpPath		   += "QSDINP.DAT";
	if ( !f.Open(strCfgPath, CFile::modeWrite | CFile::modeCreate | CFile::typeText, &e) )
	{
		ReportFileError(&e);
		return;
	};

	// Path of the file with the elemental properties
	strLine  = "PropertyFilename=";
	strLine	+= pApp->m_strQsdPropertyFileName;
	f.WriteString(strLine);

	// Path of the file with the compound properties, here simply "QSDINP.DAT"
	strLine  = "\nCompoundFilename=";
	strLine += pApp->m_strQsdExePath;
	strLine += "QSDINP.DAT";
	f.WriteString(strLine);

	// Path of the file with the resulting discovery space, here simply "QSDOUT.DAT"
	strLine  = "\nDiscoverySpaceFilename=";
	strLine += pApp->m_strQsdExePath;
	strLine += "QSDOUT.DAT";
	f.WriteString(strLine);

	// Path to the file with the resulting profiles, here simply "PROFILES.DAT"
	strLine  = "\nProfilename=";
	strLine += pApp->m_strQsdExePath;
	strLine += "PROFILES.DAT";
	f.WriteString(strLine);

	// Refresh rate of writing output data into QSDOUT.DAT; must be -1 for a single run.
	strLine  = "\nWriteDiscoverySpace=";
	strLine	+= "-1";
	f.WriteString(strLine);

	// Configurational settings direct from the registry
	strLine.Format("\nNumBestResults=%d\nMaxNeighbours=%d",
					pApp->GetProfileInt(s_szQsd, "NumBestResults", 20),
					pApp->GetProfileInt(s_szQsd, "MaxNeighbours", 50));
/*	strLine.Format("\nNumBestResults=%d\nMaxNeighbours=%d\nNeighbourError=%d",
					pApp->GetProfileInt(s_szQsd, "NumBestResults", 20),
					pApp->GetProfileInt(s_szQsd, "MaxNeighbours", 50),
					pApp->GetProfileInt(s_szQsd, "NeighbourError", 1));	*/
	f.WriteString(strLine);

	// "AutoRun=0" means "single run" but without writing PROFILES.DAT
	strLine = "\nAutoRun=0";
	f.WriteString(strLine);

	// We retrieve the setting "System Level" or "Compound Level" from the
	// registry, where it has been stored from <CQsdRunSheet>.
	// (In Registry stored as 0 or 1; for QSD.CFG 1 or 2, rsp.)
	strLine.Format("\nLevel=%d", pApp->GetProfileInt(s_szQsd, "Level", 0)+1);
	f.WriteString(strLine);

	// Get the selected profile and write the indexes of the three pairs of
	// property/operator indexes.
	CQsdProfile* pProfile = &pDoc->m_QsdProfiles[pDoc->m_iQsdSelProfile];
	strLine.Format("\nSingleTask=%2.2d-%2.2d / %2.2d-%2.2d / %2.2d-%2.2d\n",
				   pProfile->m_ip1, pProfile->m_io1,
				   pProfile->m_ip2, pProfile->m_io2,
				   pProfile->m_ip3, pProfile->m_io3);
	f.WriteString(strLine);

	f.Close();

	// Now write the actual input file with the compound property data
	if ( !f.Open(strInpPath, CFile::modeCreate | CFile::modeWrite | CFile::typeText, &e) )
	{
		ReportFileError(&e);
		return;
	};

	// Begin with "QSD" and the number of records
	int nRecords = pDoc->m_QsdRecords.GetSize();
	strLine.Format("QSD\n%d\n", nRecords);
	f.WriteString(strLine);

	// And now write the <nRecords> records for the residual <nRecords> lines
	for (register int i=0; i < nRecords; i++)
	{
		f.WriteString(pDoc->m_QsdRecords[i].Format(strLine));
	};

	f.Close();

	// Now run the QSD kernel and wait for its termination
	CWaitCursor curWait;
	char szExe[_MAX_PATH], szDir[_MAX_PATH], *cp;
	strcpy(szDir, pApp->m_strQsdExePath);
	strcpy(szExe, szDir);
	strcat(szExe, "QSD.EXE");
	if ((cp = strrchr(szDir, '\\')) != NULL)
		*cp = '\0';

	PROCESS_INFORMATION ProcessInfo;
	STARTUPINFO StartupInfo;
	memset(&StartupInfo, 0, sizeof(STARTUPINFO));
	StartupInfo.cb			= sizeof(STARTUPINFO);
	StartupInfo.dwFlags		= STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow	= SW_HIDE;

	BOOL bStat = CreateProcess(NULL, szExe, 0, 0, TRUE, 0, 0, szDir,
							   &StartupInfo, &ProcessInfo);

	if ( bStat )
	{
		DWORD dwExitCode = 0;
		HANDLE hProcess	 = (HANDLE)ProcessInfo.hProcess;
		WaitForSingleObject(hProcess, INFINITE);
		GetExitCodeProcess(hProcess, &dwExitCode);
		CloseHandle(hProcess);
		CloseHandle(ProcessInfo.hThread);

		TRACE("CMainFrame::OnQsdStartDirect() - single run finished with exit code %u\n",
			dwExitCode);

		if (dwExitCode == 1)
		{
			ReportQsdExeError();
			return;
		}
	}
	else
	{
		AfxMessageBox("Fatal Error:\nCannot execute QSD kernel for single run!");
		return;
	};

	// Update the current profile from file "PROFILES.DAT".
	// (Not more necessary, since number of neighbours does not change between
	// multiple runs and single run.)
//	CString strProfile;
//	strProfile  = pApp->m_strQsdExePath;
//	strProfile += "PROFILES.DAT";
//	VERIFY( pDoc->UpdateQsdProfile(strProfile, pDoc->m_iQsdSelProfile) >= 0 );

	// Now read the results of the single run from file "QSDOUT.DAT"
	CString strOut;
	strOut  = pApp->m_strQsdExePath;
	strOut += "QSDOUT.DAT";
	if ( !pDoc->ReadQsdDiscoverySpace(strOut) )
	{
		AfxMessageBox(IDS_QSD_CANNOT_READ_DISCOVERYSPACE);
		return;
	};

	// Switch to Discovery Space graphics and list and adjust pane sizes
	SetQsdViews(2, 2, 2);
	AdjustPaneSizes();
	m_wndSplitter.SetActivePane(0,0);	// make one view the active view
										// for CFrameWnd::GetActiveDocument()
	pDoc->UpdateAllViews(NULL);
	m_wndSplitter.RedrawWindow();
}

// Switch to QSD profile view
void CMainFrame::OnViewQsdProfile() 
{
	// Select profile graphics in the left pane, the list of profiles
	// in the (upper) right pane. Try to hide the small lower right pane
	// which can display the histogram of the neighbour percentages.
	SetQsdViews(1, 1, 1);

	// Adust the sizes of the two/three panes
	AdjustPaneSizes();

	// Don't forget to activate one of the new views (default: the graphics pane
	// in the left part). Otherwise CFrameWnd::GetActiveDocument() fails, since
	// it accesses the active view's <m_pDocument> member.
	m_wndSplitter.SetActivePane(0,0);

	CDiscoveryDoc* pDoc = (CDiscoveryDoc*)GetActiveDocument();
	ASSERT_VALID(pDoc);

	pDoc->UpdateAllViews(NULL);
	m_wndSplitter.RedrawWindow();
}

void CMainFrame::OnUpdateViewQsdProfile(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	BOOL bSel	 = FALSE;
	if (m_iViewMode == VIEW_QSD)
	{
		bEnable = TRUE;
		bSel	= m_wndSplitter.GetPane(0,0)->IsKindOf(RUNTIME_CLASS(CProfileGraphView));
	};
	pCmdUI->Enable(bEnable);
	pCmdUI->SetRadio(bSel);
}

void CMainFrame::OnViewQsdCube() 
{
	SetQsdViews(2, 2, 2);

	// Adust the sizes of the three panes
	AdjustPaneSizes();

	m_wndSplitter.SetActivePane(0,0);	// see above

	CDiscoveryDoc* pDoc = (CDiscoveryDoc*)GetActiveDocument();
	ASSERT_VALID(pDoc);

	pDoc->UpdateAllViews(NULL);
	m_wndSplitter.RedrawWindow();
}

void CMainFrame::OnUpdateViewQsdCube(CCmdUI* pCmdUI) 
{
	BOOL bEnable = FALSE;
	BOOL bSel	 = FALSE;
	if (m_iViewMode == VIEW_QSD)
	{
		bEnable = TRUE;
		bSel	= m_wndSplitter.GetPane(0,0)->IsKindOf(RUNTIME_CLASS(CQsdGraphView));
	};
	pCmdUI->Enable(bEnable);
	pCmdUI->SetRadio(bSel);
}

// These two functions are called by the framework to update the total number of
// datapoints and the current number of selected datapoints, rsp., in the statusbar.
void CMainFrame::OnUpdateIndicatorCount(CCmdUI* pCmdUI) 
{
	CString strInfo;
	CDiscoveryDoc* pDoc = (CDiscoveryDoc*)GetActiveDocument();
	ASSERT_VALID( pDoc );
	if (pDoc != NULL)
		strInfo.Format("%d datapoint(s)", pDoc->m_QsdRecords.GetSize());
	else
		strInfo = "No datapoints";
	pCmdUI->SetText(strInfo);
	pCmdUI->Enable(pDoc != NULL);
}

void CMainFrame::OnUpdateIndicatorSel(CCmdUI* pCmdUI) 
{
	CString strInfo;
	CDiscoveryDoc* pDoc = (CDiscoveryDoc*)GetActiveDocument();
	ASSERT_VALID( pDoc );
	if (pDoc != NULL)
	{
		for (register int i=0, n=pDoc->m_QsdRecords.GetSize(), nSel=0; i < n; i++)
		{
			if ( pDoc->m_QsdRecords[i].m_bSel )
				nSel++;
		};
		strInfo.Format("%d selected", nSel);
	}
	else
		strInfo = "Nothing selected";
	pCmdUI->SetText(strInfo);
	pCmdUI->Enable(pDoc != NULL);
}
