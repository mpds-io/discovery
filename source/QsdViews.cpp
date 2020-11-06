// QsdViews.cpp : implementation file
// (C) 1999-2001 Klaus Brandenburg - Last Update: 27.03.2001
// Ported to Windows 8.1 by Fulcrum.013 Mariupol, Ukraine 
// 17 september 2020

#define  _CRT_SECURE_NO_WARNINGS 
#include "stdafx.h"
#include "discovery.h"
#include "QsdViews.h"
#include "DiscoveryDoc.h"
#include "QsdRunDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const char s_szApplication[]="Application",
				  s_szQsd[]="Qsd";

/////////////////////////////////////////////////////////////////////////////
// CProfileListView

IMPLEMENT_DYNCREATE(CProfileListView, CListView)

CProfileListView::CProfileListView()
{
	m_iSort = 0;	// 0=no sorting, n=sort for column #<n-1> ascending, -n=descending
	m_piSortInd = NULL;

	m_bProcessedContextMenu = FALSE;
}

CProfileListView::~CProfileListView()
{
	if (m_piSortInd != NULL)
		delete m_piSortInd;
}


BEGIN_MESSAGE_MAP(CProfileListView, CListView)
	//{{AFX_MSG_MAP(CProfileListView)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_COMMAND(ID_QSD_START_SINGLE, OnQsdStartSingle)
	ON_UPDATE_COMMAND_UI(ID_QSD_START_SINGLE, OnUpdateQsdStartSingle)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProfileListView drawing

void CProfileListView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CProfileListView diagnostics

#ifdef _DEBUG
void CProfileListView::AssertValid() const
{
	CListView::AssertValid();
}

void CProfileListView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

CDiscoveryDoc* CProfileListView::GetDocument()
{
	ASSERT_KINDOF( CDiscoveryDoc, m_pDocument );
	return (CDiscoveryDoc*)m_pDocument;
};

/////////////////////////////////////////////////////////////////////////////
// CProfileListView message handlers

int CProfileListView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	lpCreateStruct->style |= LVS_REPORT;
	lpCreateStruct->style |= LVS_SHOWSELALWAYS;
	lpCreateStruct->style |= LVS_SINGLESEL;

	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	return 0;
}

void CProfileListView::OnInitialUpdate() 
{
	CWinApp* pApp = AfxGetApp();

	// Create the small icon list to identify the profile graph colors
	m_imageList.Create(16, 16,	// horizontal and vertical bitmap size
						0,		// regular, that means no additional transparent bitmaps
						0,		// initial size of the internal bitmap array
						8);		// amount to increase array when necessary
	m_imageList.Add(pApp->LoadIcon(IDI_RED));
	m_imageList.Add(pApp->LoadIcon(IDI_BLUE));
	m_imageList.Add(pApp->LoadIcon(IDI_GREEN));
	m_imageList.Add(pApp->LoadIcon(IDI_YELLOW));
	m_imageList.Add(pApp->LoadIcon(IDI_PURPLE));
	m_imageList.Add(pApp->LoadIcon(IDI_CYAN));
	m_imageList.Add(pApp->LoadIcon(IDI_LIGHT_RED));
	m_imageList.Add(pApp->LoadIcon(IDI_LIGHT_BLUE));

	CListCtrl& list	= GetListCtrl();
	DWORD dwExStyle	= list.SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE);
	list.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwExStyle | LVS_EX_FULLROWSELECT);

	CListView::OnInitialUpdate();
}

// Callback function used by qsort() from within OnUpdate(), which uses the
// following static variables to access the profile definitions etc.:
static UINT s_uSort;					// identifies the column to be sorted for
static BOOL s_bDesc;					// descending (1) or ascending (0) order
static CQsdProfileArray *s_pProfiles;	// points to the array with the profiles

static int _SortProfiles (const void* elem1, const void* elem2)
{
	int i1			 = *(int*)elem1;
	int i2			 = *(int*)elem2;
	CQsdProfile* pp1 = &(*s_pProfiles)[i1];
	CQsdProfile* pp2 = &(*s_pProfiles)[i2];
	int isort		 = 0;
	int iv;
	if (s_uSort == 1)
	{
		isort		 = ((pp1->m_ip1 > pp2->m_ip1) ? 1 : (pp1->m_ip1 < pp2->m_ip1) ? -1 : 0);
		if (isort != 0)
			isort	 = ((pp1->m_io1 > pp2->m_io1) ? 1 : (pp1->m_io1 < pp2->m_io1) ? -1 : 0);
		if (isort != 0)
			isort	 = ((pp1->m_ip2 > pp2->m_ip2) ? 1 : (pp1->m_ip2 < pp2->m_ip2) ? -1 : 0);
		if (isort != 0)
			isort	 = ((pp1->m_io2 > pp2->m_io2) ? 1 : (pp1->m_io2 < pp2->m_io2) ? -1 : 0);
		if (isort != 0)
			isort	 = ((pp1->m_ip3 > pp2->m_ip3) ? 1 : (pp1->m_ip3 < pp2->m_ip3) ? -1 : 0);
		if (isort != 0)
			isort	 = ((pp1->m_io3 > pp2->m_io3) ? 1 : (pp1->m_io3 < pp2->m_io3) ? -1 : 0);
	}
	else if (s_uSort >= 2)
	{
		iv			 = s_uSort - 2;
		ASSERT( iv < pp1->m_nMax && iv < pp2->m_nMax );
		isort		 = ((pp1->m_pfValues[iv] > pp2->m_pfValues[iv]) ?  1 :
						(pp1->m_pfValues[iv] < pp2->m_pfValues[iv]) ? -1 : 0);
	};
	if ( s_bDesc )
		isort		*= -1;
	return isort;
};

// lHint == 0: Delete old columns, sort, insert columns, insert items.
// lHint == HINT_SORTLIST: Delete "old" items, then sort and re-insert items.
void CProfileListView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CDiscoveryDoc* pDoc = GetDocument();

	CListCtrl& list = GetListCtrl();

	// Delete all profile items from the list. When OnUpdate() has only been
	// called to reflect the new sort order, the items will be re-inserted.
	list.DeleteAllItems();

	// If no hint is given, we build or re-build the list from scratch and remove
	// all columns as far as they exist.
	if (lHint == 0)
	{
		while ( list.DeleteColumn(0) );
	};

	// Number of profiles (rows) to go into the list
	int np = pDoc->m_QsdProfiles.GetSize();
	register int ir, ip;

	// The items in the list can be sorted by changing the rank order of the
	// indexes of the profiles. <m_iSort>==0 means no sorting, +1 means sorting
	// in ascending order for column #0 (the profile descriptor), -1 for
	// descending order, +2 and -2 for column #1, etc.
	if (m_piSortInd)
		delete m_piSortInd;
	if (np > 0)
	{
		m_piSortInd = new int[np];
		for (ir=0; ir < np; ir++)
			m_piSortInd[ir] = ir;
		if (m_iSort != 0)
		{
			s_uSort		= abs(m_iSort);
			s_bDesc		= (m_iSort < 0);
			s_pProfiles	= &pDoc->m_QsdProfiles;
			qsort(m_piSortInd, np, sizeof(m_piSortInd[0]), _SortProfiles);
		}
	}
	else
		m_piSortInd = NULL;

	// If there are no profiles yet (may be this view has been switched on after
	// a single run without prior multiple runs), return immediately.
	if (np == 0)
		return;

	// Number of columns without the first column with the profile descriptor
	int nc = pDoc->m_QsdProfiles[0].m_nVal;
	register int ic;

	// The following initialization are not necessary after re-sorting, etc.
	if (lHint == 0)
	{
		// Set the image list, which uses color icons to identify the profile graphs.
		list.SetImageList(&m_imageList, LVSIL_SMALL);

		// The first column has the three index pairs of the properties and
		// operations used.
		int iwid = list.GetStringWidth("01-01 / 01-01 / 01-01  ") + 20;	// add 20 for icon
		list.InsertColumn(0, "Profile", LVCFMT_LEFT, iwid, 0);

		// The second column contains the best value for each profile
		iwid = list.GetStringWidth("123456789 (98.76) ");
		list.InsertColumn(1, "1", LVCFMT_LEFT, iwid, 1);

		// The residual columns contain values with decreasing values.
		// Their number depends on the "Maximum neighbours" setting.
		// These columns will have zero pixel width when initialized,
		// except the latest column.
		char szHelp[64];
		for (ic=1; ic < nc; ic++)
		{
			iwid = ((ic == nc-1) ? list.GetStringWidth("123456789 (98.76) ") : 0);
			list.InsertColumn(ic+1, _itoa(ic+1, szHelp, 10), LVCFMT_LEFT, iwid, ic+1);
		}
	};

	CString strItem;
	float fnRec	= (float)pDoc->m_QsdRecords.GetSize();
	int ni		= m_imageList.GetImageCount();
	ASSERT( ni > 0 );
	for (ir=0; ir < np; ir++)
	{
		// <ip> is the profile index, <ir> is the row index.
		// This means: row number <ir> will get profile number <ip>.
		ip = m_piSortInd[ir];
		CQsdProfile* pp = &pDoc->m_QsdProfiles[ip];
		strItem.Format("%2.2d-%2.2d / %2.2d-%2.2d / %2.2d-%2.2d",
					   pp->m_ip1, pp->m_io1, pp->m_ip2, pp->m_io2, pp->m_ip3, pp->m_io3);

		// Profile <ip> uses image no. (color index) <ip> MOD <image count>.
		list.InsertItem(ir, strItem, ip % ni);
		for (ic=0; ic < nc; ic++)
		{
			strItem.Format("%6.0f (%5.2f)", pp->m_pfValues[ic],
						   pp->m_pfValues[ic]*100.f/fnRec);
			list.SetItemText(ir, ic+1, strItem);
		}
	}
}

void CProfileListView::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Register click on header item to sort for this column or to revert the sort order.
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int iCol = pNMListView->iSubItem;
	if (abs(m_iSort)-1 == iCol)	// m_iSort==1 for column #0, 2 for column #1, etc.
		m_iSort *= -1;
	else
		m_iSort = iCol + 1;

	OnUpdate(this, HINT_SORTLIST, NULL);
	
	*pResult = 0;
}

void CProfileListView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Start a single run with the double-clicked profile
	OnQsdStartSingle();

	*pResult = 0;
}

void CProfileListView::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TRACE("Entering CProfileListView::OnRclick()\n");

	// See comments in CReportCtrl::OnNotify()
	GetParentFrame()->ActivateFrame();

	CMenu Menu, *pSubMenu;
	Menu.LoadMenu(IDR_PROFILE_LIST);
	pSubMenu = Menu.GetSubMenu(0);

	// See comments in CReportCtrl::OnContextMenu()
	m_bProcessedContextMenu = TRUE;

	CPoint ptScreen(GetMessagePos());
	UINT uCmd = GetPopupMenuCmd(pSubMenu, ptScreen.x, ptScreen.y, AfxGetMainWnd());
	if (uCmd != 0)
		AfxGetMainWnd()->PostMessage(WM_COMMAND, uCmd);
	
	*pResult = 0;
}

void CProfileListView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if ( !m_bProcessedContextMenu )
	{
		TRACE("CProfileListView::OnContextMenu() - processing WM_CONTEXTMENU\n");

		// See comments in CReportCtrl::OnNotify()
		GetParentFrame()->ActivateFrame();

		// When context menu has been requested by the keyboard, <point> contains
		// the screen coordinates (-1,-1). In that case we put the context menu
		// into the upper left corner of the view's client area.
		if (point.x < 0 && point.y < 0)
		{
			point.x = point.y = 0;
			ClientToScreen(&point);
		};

		CMenu Menu, *pSubMenu;
		Menu.LoadMenu(IDR_PROFILE_LIST);
		pSubMenu  = Menu.GetSubMenu(0);
	
		UINT uCmd = GetPopupMenuCmd(pSubMenu, point.x, point.y, AfxGetMainWnd());
		if (uCmd != 0)
			AfxGetMainWnd()->PostMessage(WM_COMMAND, uCmd);
	};

	m_bProcessedContextMenu = FALSE;
}

// When the command ID_QSD_START is called from the profile list, the pre-settings
// for the QSD Run Wizard will be the single run with the property and operation indexes
// of the currently selected profile.
void CProfileListView::OnQsdStartSingle() 
{
	CDiscoveryDoc* pDoc	= GetDocument();
	CListCtrl& list		= GetListCtrl();
	for (register int i=0, n=list.GetItemCount(); i < n; i++)
	{
		if (list.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
			break;
	};
	if (i == n)
		return;

	// Since the list can be sorted, identify the index of the profile
	int ir = i;
	int ip = m_piSortInd[ir];
	pDoc->m_iQsdSelProfile = ip;

	// The user can now still change the "Number of neighbours".
	// (This dialog has retired. We keep the number of neighbours as used for multiple runs.)
//	CQsdConfSingleDlg dlg;
//	CDiscoveryApp* pApp = (CDiscoveryApp*)AfxGetApp();
//	dlg.m_uMax	 = pApp->GetProfileInt(s_szQsd, "MaxNeighbours", 50);	// TO BE CHECKED
//	dlg.m_bError = (BOOL)pApp->GetProfileInt(s_szQsd, "NeighbourError", 1);	// TO BE CHECKED
//	if (dlg.DoModal() == IDOK)
//	{
//		pDoc->m_nQsdMaxNeighbours/*Single*/ = dlg.m_uMax;
//		pApp->WriteProfileInt(s_szQsd, "MaxNeighbours", (int)dlg.m_uMax);

		// The index of the selected profile has been stored in
		// <CDiscoveryDoc::m_iQsdSelProfile>. Now delegate further processing
		// to the frame window.
		AfxGetMainWnd()->PostMessage(WM_COMMAND, ID_QSD_START_DIRECT);
//	}
}

void CProfileListView::OnUpdateQsdStartSingle(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetListCtrl().GetSelectedCount() > 0);
}

// Copies the whole list (not only the selected line) to the clipboard,
// e.g. for pasting into Microsoft Excel.
// Subitems will be separated by tabstops, lines will be separated by CRLF.
void CProfileListView::OnEditCopy() 
{
	CStringList Strings;
	CString strLine, strItem;
	CListCtrl& list		 = GetListCtrl();
	CDiscoveryDoc* pDoc	 = GetDocument();
	CHeaderCtrl* pHeader = (CHeaderCtrl*)list.GetDlgItem(0);
	int nColumns		 = pHeader->GetItemCount();
	int nRows			 = list.GetItemCount();
	int iSize			 = 0;							// counts text size in bytes
	float fnRecords		 = (float)pDoc->m_QsdRecords.GetSize();	// to get percentages
	for (register int irow=0; irow < nRows; irow++)
	{
		int iProfile	 = m_piSortInd[irow];
		CQsdProfile* pp	 = &pDoc->m_QsdProfiles[iProfile];
		strLine			 = list.GetItemText(irow, 0);
		for (register int icol=1; icol < nColumns; icol++)
		{
			strLine		+= "\t";
			strItem.Format("%5.2f", pp->m_pfValues[icol-1]*100.0f/fnRecords);
			strLine		+= strItem;
		};
		strLine			+= "\r\n";
		iSize			+= strLine.GetLength();
		Strings.AddTail(strLine);
	};
	
	HANDLE hBuffer	= GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, iSize+2);	// plus '\0'
	LPSTR pBuffer	= (LPSTR)GlobalLock(hBuffer);
	LPSTR cp		= pBuffer;
	POSITION pos1, pos2;
	for (pos1=Strings.GetHeadPosition(); (pos2=pos1) != NULL; )
	{
		strcpy(cp, Strings.GetNext(pos1));
		cp += Strings.GetAt(pos2).GetLength();
	};
	*cp = '\0';
	GlobalUnlock(hBuffer);

	if ( list.OpenClipboard() )
	{
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hBuffer);
		CloseClipboard();
	}
}


/////////////////////////////////////////////////////////////////////////////
// CProfileGraphView

IMPLEMENT_DYNCREATE(CProfileGraphView, CView)

CProfileGraphView::CProfileGraphView()
{
	m_afXRange[0] = 0.0f;
	m_afXRange[1] = 0.0f;
	m_afYRange[0] = 0.0f;
	m_afYRange[1] = 0.0f;
}

CProfileGraphView::~CProfileGraphView()
{
}


BEGIN_MESSAGE_MAP(CProfileGraphView, CView)
	//{{AFX_MSG_MAP(CProfileGraphView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProfileGraphView drawing

void CProfileGraphView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
}

void CProfileGraphView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CDiscoveryDoc* pDoc = GetDocument();

	// The x- and y-range of the profile graph may be zoomed and shifted
	// afterwards, but when initialized, we adjust the graphics to fit all
	// points of all profiles within the drawing rectangle.
	
	int np   = pDoc->m_QsdProfiles.GetSize();
	int nv	 = ((np > 0) ? pDoc->m_QsdProfiles[0].m_nMax : 0);
	//int nv   = pDoc->m_nQsdMaxNeighbours/*Multiple*/;
	float fn = (float)pDoc->m_QsdRecords.GetSize();

	// Examine the x- and y-range of the profile points. The x-range depends on
	// the "Maximum neighbours" setting, the y-range on the lowest and highest
	// value of goodness in percent.
	float afXRange[2], afYRange[2];
	afXRange[0] = 0.0f;				// trivial
	afXRange[1] = (float)nv;
	if (np > 0)
	{
		ASSERT( fn >= 1.0f );

		afYRange[0] = 100.0f;
		afYRange[1] =   0.0f;
		for (register int ip=0; ip < np; ip++)
		{
			CQsdProfile* pp = &pDoc->m_QsdProfiles[ip];
			for (register int iv=0; iv < pp->m_nMax; iv++)
			{
				float fv = pp->m_pfValues[iv] * 100.0f / fn;	// get percent value
				if (fv < afYRange[0])
					afYRange[0] = fv;
				if (fv > afYRange[1])
					afYRange[1] = fv;
			}
		}
	}
	else
	{
		afYRange[0] =   0.0f;	// empty profile graph goes from 0 to 100 percent
		afYRange[1] = 100.0f;
	};

	// Store the x- and y-ranges for later drawing by OnDraw()
	m_afXRange[0] = afXRange[0];
	m_afXRange[1] = afXRange[1];
	m_afYRange[0] = afYRange[0];
	m_afYRange[1] = afYRange[1];

	TRACE("CProfileGraphView::OnUpdate() - m_afXRange=[%2.0f..%2.0f], m_afYRange=[%5.3f..%5.3f]\n",
		m_afXRange[0], m_afXRange[1], m_afYRange[0], m_afYRange[1]);
}

void CProfileGraphView::OnDraw(CDC* pDC)
{
	static COLORREF s_acrProfile[8] = {
		RGB(255,0,0), RGB(0,0,255), RGB(0,255,0), RGB(255,255,0), RGB(255,0,255),
		RGB(0,255,255), RGB(128,0,0), RGB(0,0,128)	};

	CDiscoveryDoc* pDoc = GetDocument();

	int iOldDC = pDC->SaveDC();
	ASSERT( iOldDC != 0 );

	// Pixels per millimeter in x- and y-direction
	float fpx	 = (float)pDC->GetDeviceCaps(HORZRES) / (float)pDC->GetDeviceCaps(HORZSIZE);
	float fpy	 = (float)pDC->GetDeviceCaps(VERTRES) / (float)pDC->GetDeviceCaps(VERTSIZE);

	// Calculate the full length of an axis ticker.
	int iTickLen = (int)ROUND(3.0f * (fpx + fpy) * 0.5f);	// 3 millimeters

	// Select a font with suitable size for the axis labelling. This will also
	// used to determine the distance from the edges of the drawing area,
	// including space needed to draw the tickers at the axes and the descriptions
	// of the axes at the left and bottom side.
	CFont ScaleFont, *pOldFont;
	ScaleFont.CreatePointFont(90, "Arial", pDC);
	pOldFont = pDC->SelectObject(&ScaleFont);
	CSize sizeBorder = pDC->GetTextExtent("100");
	int iTextHeight	 = sizeBorder.cy;
	sizeBorder.cx   += iTickLen;
	sizeBorder.cy   += iTickLen;

	// Determine the rectangle where the profile graphs will be drawn inside.
	CRect rectClient;
	GetClientRect(&rectClient);
	CRect rectGraph(rectClient.left+sizeBorder.cx+iTextHeight,
					rectClient.top+sizeBorder.cy,
					rectClient.right-sizeBorder.cx,
					rectClient.bottom-sizeBorder.cy-iTextHeight);

	CPen AxisPen(PS_SOLID, 1, RGB(0,0,0));
	CPen GridPen(pDC->IsPrinting() ? PS_DASH : PS_DOT, 1, RGB(192,192,192));

	// Calculate scaling factors for both axes, that means how many pixels for
	// one x- or y-unit, rsp.
	float fXScale, fYScale, fXRange, fYRange;
	fXRange = m_afXRange[1] - m_afXRange[0];
	if (fXRange >= 1.0f)
		fXScale = (float)(rectGraph.Width()-1) / fXRange;
	else
		fXScale = 0.0f;
	fYRange = m_afYRange[1] - m_afYRange[0];
	ASSERT( fYRange > 0.0f );
	fYScale = (float)(rectGraph.Height()-1) / fYRange;

	// Draw the x- and y-axis at both sides of the rectangle.
	pDC->SelectObject(&AxisPen);
	pDC->Rectangle(&rectGraph);

	// Draw scalings and labels on x-axes (bottom and top of the rectangle).
	// The 0, 5, 10, ... tickers will have full ticker length and will be labelled,
	// whereas the other tickers have only half length and no labels.
	pDC->SetTextColor(RGB(0,0,0));
	for (float fx=m_afXRange[0]; fx <= m_afXRange[1]; fx += 1.0f)
	{
		int ifx		= (int)ROUND(fx);
		BOOL bFull	= ((ifx % 5) == 0);		// fx == 0, 5, 10, ...?
		int ix		= (int)ROUND(fx*fXScale) + rectGraph.left;
		pDC->SelectObject(&AxisPen);
		pDC->MoveTo(ix, rectGraph.bottom);
		pDC->LineTo(ix, rectGraph.bottom + (bFull ? iTickLen : iTickLen/2));
		pDC->MoveTo(ix, rectGraph.top);
		pDC->LineTo(ix, rectGraph.top - (bFull ? iTickLen : iTickLen/2));

		if ( bFull && ix > rectGraph.left && ix < rectGraph.right-1)
		{
			pDC->SelectObject(&GridPen);
			pDC->MoveTo(ix, rectGraph.top+1);
			pDC->LineTo(ix, rectGraph.bottom-2);
		};

		if ( bFull )
		{
			char szHelp[32];
			sprintf (szHelp, "%1.0f", fx);
			pDC->SetTextAlign(TA_TOP | TA_CENTER);
			pDC->TextOut(ix, rectGraph.bottom + iTickLen, szHelp, strlen(szHelp));
			pDC->SetTextAlign(TA_BOTTOM | TA_CENTER);
			pDC->TextOut(ix, rectGraph.top - iTickLen, szHelp, strlen(szHelp));
		}
	};

	// Tickers and labels on the y-axis. Only the tickers for 0, 5, 10, ... percent
	// will have full ticker length and labels.
	for (float fy=m_afYRange[0]; fy <= m_afYRange[1]; fy += 1.0f)
	{
		int ify		= (int)ROUND(fy);
		BOOL bFull	= ((ify % 5) == 0);
		int iy		= -1*(int)ROUND((fy-m_afYRange[0])*fYScale) + rectGraph.bottom - 1;
		pDC->SelectObject(&AxisPen);
		pDC->MoveTo(rectGraph.left - (bFull ? iTickLen : iTickLen/2), iy);
		pDC->LineTo(rectGraph.left, iy);
		pDC->MoveTo(rectGraph.right, iy);
		pDC->LineTo(rectGraph.right + (bFull ? iTickLen : iTickLen/2), iy);

		if ( bFull && iy > rectGraph.top && iy < rectGraph.bottom-1)
		{
			pDC->SelectObject(&GridPen);
			pDC->MoveTo(rectGraph.left+1, iy);
			pDC->LineTo(rectGraph.right-2, iy);
		};

		if ( bFull )
		{
			char szHelp[32];
			sprintf (szHelp, "%1.0f", fy);
			pDC->SetTextAlign(TA_BASELINE | TA_RIGHT);
			pDC->TextOut(rectGraph.left - iTickLen, iy, szHelp, strlen(szHelp));
			pDC->SetTextAlign(TA_BASELINE | TA_LEFT);
			pDC->TextOut(rectGraph.right + iTickLen, iy, szHelp, strlen(szHelp));
		}
	};

	// Now we draw the profile graphs. The points will be connected by lines.
	int npp	= pDoc->m_QsdProfiles.GetSize();
	int npt = ((npp > 0) ? pDoc->m_QsdProfiles[0].m_nMax : 0);
	//int npt = pDoc->m_nQsdMaxNeighbours/*Multiple*/;
	if (npt > 0 && npp > 0)
	{
		POINT *ppt = new POINT[npt];
		float fn   = (float)pDoc->m_QsdRecords.GetSize();
		ASSERT( fn >= 1.0f );
		for (register int ip=0; ip < npp; ip++)
		{
			CQsdProfile* pp	= &pDoc->m_QsdProfiles[ip];
			ASSERT( npt == pp->m_nMax );
			for (register int iv=0; iv < npt; iv++)
			{
				int ipx   = (int)ROUND((float)(iv + 1) * fXScale) + rectGraph.left;
				float fy  = pp->m_pfValues[iv] * 100.0f / fn;	// get percent value
				int ipy   = -1 * (int)ROUND((fy - m_afYRange[0]) * fYScale) + rectGraph.bottom - 1;
				ppt[iv].x = ipx;
				ppt[iv].y = ipy;
			}

			CPen ProfilePen(PS_SOLID, 1, s_acrProfile[ip % (sizeof(s_acrProfile)/sizeof(s_acrProfile[0]))]);
			pDC->SelectObject(&ProfilePen);
			pDC->Polyline(ppt, npt);
		}

		delete ppt;
	};

	// Finally we label the x-axis with "Neighbours" and the y-axis with "Separation %"
	LOGFONT lf;
	CFont YAxisFont;
	ScaleFont.GetLogFont(&lf);
	lf.lfOrientation = lf.lfEscapement = 900;
	YAxisFont.CreateFontIndirect(&lf);
	pDC->SetTextAlign(TA_BOTTOM | TA_CENTER);
	pDC->TextOut((rectGraph.left+rectGraph.right)/2, rectClient.bottom-2, "Neighbours");
	pDC->SelectObject(pOldFont);
	pOldFont = pDC->SelectObject(&YAxisFont);
	pDC->SetTextAlign(TA_LEFT | TA_TOP);
	pDC->TextOut(rectClient.left+2, (rectGraph.top+rectGraph.bottom)/2, "Separation %");

	pDC->SelectObject(pOldFont);

	VERIFY( pDC->RestoreDC(iOldDC) );
}

/////////////////////////////////////////////////////////////////////////////
// CProfileGraphView diagnostics

#ifdef _DEBUG
void CProfileGraphView::AssertValid() const
{
	CView::AssertValid();
}

void CProfileGraphView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

CDiscoveryDoc* CProfileGraphView::GetDocument()
{
	ASSERT_KINDOF( CDiscoveryDoc, m_pDocument );
	return (CDiscoveryDoc*)m_pDocument;
};

/////////////////////////////////////////////////////////////////////////////
// CProfileGraphView message handlers


/////////////////////////////////////////////////////////////////////////////
// CQsdGraphView

IMPLEMENT_DYNCREATE(CQsdGraphView, CView)

CQsdGraphView::CQsdGraphView()
{
	ResetMatrix(m_omat);
	m_fScale = 0.0f;
	m_ptShift.x = m_ptShift.y = 0;
	m_piSort = NULL;
	m_bBalls = FALSE;
	m_iTracking = 0;
	m_bTracking = FALSE;
	m_bSlabMode	= FALSE;
	m_iVertAxis	= 0;
	m_bSlabActive = FALSE;
//	m_iSlabAxis = 0;
//	m_afSlabRange[0] = m_afSlabRange[1] = FErr;
	m_rectCube2D.SetRectEmpty();
	m_rectSlab.SetRectEmpty();
}

CQsdGraphView::~CQsdGraphView()
{
	if (m_piSort != NULL)
		delete m_piSort;
}

// Calculate view coordinates from current orientation.
// Save the extreme values in <afMinMax>:
//   afMinMax[0]=lowest x-, [1]=lowest y-, [2]=lowest z-value,
//           [3]=highest x-, [4]=highest y-, [5]=highest z-value.
// <afAxes> will contain the view coordinates of the origin (afAxes[0]) and the
// terminal points of x- [1], y- [2], and z-axis [3].
//   afCorners[0]: origin				(0,0,0)
//            [1]: end of the x-axis	(1,0,0)
//            [2]: end of the y-axis	(0,1,0)
//            [3]: end of the z-axis	(0,0,1)
//            [4]:						(1,1,0)
//            [5]:                      (1,0,1)
//            [6]:						(0,1,1)
//            [7]:						(1,1,1)
// afNormals[] contains the normal vectors of the six walls:
//   afNormals[0]: xy wall at z=0
//            [1]: xz wall at y=0
//            [2]: yz wall at x=0
//            [3]: xy wall at z=1
//            [4]: xz wall at y=1
//            [5]: yz wall at x=1
// afAxesLabels[] contains the positions for the origin's and the axes' labels.
// These are slightly different from the exact positions of the corresponding corners.
// Returns the number of processed coordinate triplets.

int CQsdGraphView::UpdateViewCoords (float afCorners[8][3], float afNormals[6][3],
									 float afAxesLabels[4][3], float afMinMax[6])
{
	CDiscoveryDoc* pDoc = GetDocument();

	float v1[4], v2[4], x, y, z;
	afMinMax[0]=afMinMax[1]=afMinMax[2]= 999.0f;
	afMinMax[3]=afMinMax[4]=afMinMax[5]=-999.0f;
	for (register int i=0, n=pDoc->m_QsdRecords.GetSize(); i < n; i++)
	{
		CQsdRecord *pq = &pDoc->m_QsdRecords[i];

		if ( pq->m_bNoCoords )		// coordinates may be "undefined"
			continue;

		// Make the points relative to the center of rotation at 0.5/0.5/0.5.
		v1[0] = pq->m_afNorm[0] - 0.5f;
		v1[1] = pq->m_afNorm[1] - 0.5f;
		v1[2] = pq->m_afNorm[2] - 0.5f;
		v1[3] = 1.0f;
		MulVector4(v1, v2, m_omat);
		x	  = v2[0] /*+ 0.5f*/;
		y	  = v2[1] /*+ 0.5f*/;
		z	  = v2[2] /*+ 0.5f*/;
		pq->m_afView[0] = x;
		pq->m_afView[1] = y;
		pq->m_afView[2] = z;

		if (x < afMinMax[0])
			afMinMax[0] = x;
		if (x > afMinMax[3])
			afMinMax[3] = x;
		if (y < afMinMax[1])
			afMinMax[1] = y;
		if (y > afMinMax[4])
			afMinMax[4] = y;
		if (z < afMinMax[2])
			afMinMax[2] = z;
		if (z > afMinMax[5])
			afMinMax[5] = z;
	};

	// Now construct and rotate the eight corner points
	for (i=0; i < 8; i++)
	{
		if		(i == 0)	// origin (0,0,0)
		{
			v1[0] = v1[1] = v1[2] = -0.5f;
		}
		else if (i == 1)	// x-axis terminal point (1,0,0)
		{
			v1[0] = 0.5f;
			v1[1] = v1[2] = -0.5f;
		}
		else if (i == 2)	// y-axis terminal point (0,1,0)
		{
			v1[0] = v1[2] = -0.5f;
			v1[1] = 0.5f;
		}
		else if (i == 3)	// z-axis terminal point (0,0,1)
		{
			v1[0] = v1[1] = -0.5f;
			v1[2] = 0.5f;
		}
		else if (i == 4)	// (1,1,0)
		{
			v1[0] = v1[1] = 0.5f;
			v1[2] = -0.5f;
		}
		else if (i == 5)	// (1,0,1)
		{
			v1[0] = v1[2] = 0.5f;
			v1[1] = -0.5f;
		}
		else if (i == 6)	// (0,1,1)
		{
			v1[0] = -0.5f;
			v1[1] = v1[2] = 0.5f;
		}
		else if (i == 7)	// (1,1,1)
		{
			v1[0] = v1[1] = v1[2] = 0.5f;
		};
		v1[3] = 1.0f;

		MulVector4(v1, v2, m_omat);
		afCorners[i][0] = x = v2[0];
		afCorners[i][1] = y = v2[1];
		afCorners[i][2] = z = v2[2];

		if (x < afMinMax[0])
			afMinMax[0] = x;
		if (x > afMinMax[3])
			afMinMax[3] = x;
		if (y < afMinMax[1])
			afMinMax[1] = y;
		if (y > afMinMax[4])
			afMinMax[4] = y;
		if (z < afMinMax[2])
			afMinMax[2] = z;
		if (z > afMinMax[5])
			afMinMax[5] = z;
	};

	// Construct and rotate the normal vectors of the six faces
	for (i=0; i < 6; i++)
	{
		if		(i == 0)	// xy wall at z=0
		{
			// In default projection, this wall is the rear wall of the cube
			// and the normal vector points into -z direction.
			v1[0] = v1[1] = 0.0f;	
			v1[2] = -1.0f;
		}
		else if (i == 1)	// xz wall at y=0
		{
			// This is the floor
			v1[0] = v1[2] = 0.0f;
			v1[1] = -1.0f;
		}
		else if (i == 2)	// yz wall at x=0
		{
			// This is the left wall
			v1[0] = -1.0f;
			v1[1] = v1[2] = 0.0f;
		}
		else if (i == 3)	// xy wall at z=1
		{
			// This is the front wall
			v1[0] = v1[1] = 0.0f;
			v1[2] = 1.0f;
		}
		else if (i == 4)	// xz wall at y=1
		{
			// This is the ceiling
			v1[0] = v1[2] = 0.0f;
			v1[1] = 1.0f;
		}
		else if (i == 5)	// yz wall at x=1
		{
			// This is the right wall
			v1[0] = 1.0f;
			v1[1] = v1[2] = 0.0f;
		};

		MulVector4(v1, v2, m_omat);
		afNormals[i][0] = v2[0];
		afNormals[i][1] = v2[1];
		afNormals[i][2] = v2[2];
	};

	// Finally set and rotate the origin's and axes' label positions
	for (i=0; i < 4; i++)
	{
		if		(i == 0)	// origin (-0.05,0,-0.05)
		{
			v1[0] = v1[2] = -0.55f;
			v1[1] = -0.5f;
		}
		else if (i == 1)	// x-axis label (1.05,0,0)
		{
			v1[0] = 0.55f;
			v1[1] = v1[2] = -0.5f;
		}
		else if (i == 2)	// y-axis label (0,1.05,0)
		{
			v1[0] = v1[2] = -0.5f;
			v1[1] = 0.55f;
		}
		else if (i == 3)	// z-axis label (0,0,1.05)
		{
			v1[0] = v1[1] = -0.5f;
			v1[2] = 0.55f;
		};
		v1[3] = 1.0f;

		MulVector4(v1, v2, m_omat);
		afAxesLabels[i][0] = x = v2[0];
		afAxesLabels[i][1] = y = v2[1];
		afAxesLabels[i][2] = z = v2[2];

		if (x < afMinMax[0])
			afMinMax[0] = x;
		if (x > afMinMax[3])
			afMinMax[3] = x;
		if (y < afMinMax[1])
			afMinMax[1] = y;
		if (y > afMinMax[4])
			afMinMax[4] = y;
		if (z < afMinMax[2])
			afMinMax[2] = z;
		if (z > afMinMax[5])
			afMinMax[5] = z;
	};

	return n;
};

// Applies a rotation along selected axes to the current orientation matrix.
// <flags> specifies the bitmask for the x- (bit 0), y- (bit 1), and z-axis (bit 2),
// <afIncr> the corresponding rotational increments in degrees.

void CQsdGraphView::Rotate (UINT flags, float afIncr[3])
{
	MAT44 m1, m2, m3;
	if ((flags & 7) == 0)
		return;
	ResetMatrix(m1);

	// Perform rotation along x-, y-, and z-axis (in this order) - as far as selected
	for (register int i=0, im=1; i < 3; i++, im <<= 1)
	{
		if ((flags & im) != 0)
		{
			GetRotationMatrix(i, afIncr[i], m2);
			MulMatrix(m1, m2, m3);
			memcpy(m1, m3, sizeof(MAT44));
		}
	};

	// Now concat the orientation matrix with the rotational matrix
	MulMatrix(m_omat, m1, m3);
	memcpy(m_omat, m3, sizeof(MAT44));

	// The view coordinates of the Discovery Space should be updated with a call
	// to UpdateViewCoords(), which is normally called before drawing.
};

BEGIN_MESSAGE_MAP(CQsdGraphView, CView)
	//{{AFX_MSG_MAP(CQsdGraphView)
	ON_WM_SETCURSOR()
	ON_WM_KEYDOWN()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_VIEW_QSD_ADJUST, OnViewQsdAdjust)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_VIEW_QSD_PROJ_XY, OnViewQsdProjXY)
	ON_COMMAND(ID_VIEW_QSD_PROJ_XZ, OnViewQsdProjXZ)
	ON_COMMAND(ID_VIEW_QSD_PROJ_YZ, OnViewQsdProjYZ)
	ON_COMMAND(ID_VIEW_QSD_DOTS, OnViewQsdDots)
	ON_UPDATE_COMMAND_UI(ID_VIEW_QSD_DOTS, OnUpdateViewQsdDots)
	ON_COMMAND(ID_VIEW_QSD_BALLS, OnViewQsdBalls)
	ON_UPDATE_COMMAND_UI(ID_VIEW_QSD_BALLS, OnUpdateViewQsdBalls)
	ON_COMMAND(ID_VIEW_ROTXY, OnViewRotXY)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ROTXY, OnUpdateViewRotXY)
	ON_COMMAND(ID_VIEW_ROTZ, OnViewRotZ)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ROTZ, OnUpdateViewRotZ)
	ON_COMMAND(ID_VIEW_SHIFT, OnViewShift)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SHIFT, OnUpdateViewShift)
	ON_COMMAND(ID_VIEW_ENLARGE, OnViewEnlarge)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ENLARGE, OnUpdateViewEnlarge)
	ON_COMMAND(ID_VIEW_NOTRACK, OnViewNoTrack)
	ON_UPDATE_COMMAND_UI(ID_VIEW_NOTRACK, OnUpdateViewNoTrack)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_VIEW_QSD_GRID, OnViewQsdGrid)
	ON_UPDATE_COMMAND_UI(ID_VIEW_QSD_GRID, OnUpdateViewQsdGrid)
	ON_COMMAND(ID_VIEW_QSD_PROJ_CLOSEST, OnViewQsdProjClosest)
	ON_COMMAND(ID_VIEW_QSD_SLAB, OnViewQsdSlab)
	ON_UPDATE_COMMAND_UI(ID_VIEW_QSD_SLAB, OnUpdateViewQsdSlab)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_VIEW_QSD_NO_SLAB, OnViewQsdNoSlab)
	ON_UPDATE_COMMAND_UI(ID_VIEW_QSD_NO_SLAB, OnUpdateViewQsdNoSlab)
	ON_COMMAND(ID_EDIT_SELECT_INV, OnEditSelectInv)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_UPDATE_COMMAND_UI(ID_VIEW_QSD_PROJ_XZ, OnUpdateViewQsdProjXZ)
	ON_UPDATE_COMMAND_UI(ID_VIEW_QSD_PROJ_YZ, OnUpdateViewQsdProjYZ)
	ON_UPDATE_COMMAND_UI(ID_VIEW_QSD_PROJ_CLOSEST, OnUpdateViewQsdProjClosest)
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_PRINT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQsdGraphView drawing

void CQsdGraphView::OnInitialUpdate() 
{
	// Reset the orientation matrix. The x-axis will then point to the right,
	// the y-axis upward, and the z-axis towards the viewer.
	ResetMatrix(m_omat);

	// Calculate the scaling factor to convert the 3D view coordinates to
	// 2D client coordinates. The minimum and maximum x- and y-coordinates of
	// the Discovery Space in default orientation reaches from -0.5 to +0.5.
	// Thus the factor is simply given by the smallest extent of the client area.
	CRect rectDraw;
	GetClientRect(&rectDraw);
	float fXScale = (float)rectDraw.Width();
	float fYScale = (float)rectDraw.Height();
	float fScale  = ((fXScale < fYScale) ? fXScale : fYScale);
	m_fScale	  = fScale * 0.75f;	// let some margin around the cube

	// Load some options for graphical representation from registry
	m_bBalls	  = (BOOL)AfxGetApp()->GetProfileInt(s_szQsd, "Balls", 0);
	m_bGrid		  = (BOOL)AfxGetApp()->GetProfileInt(s_szQsd, "Grid", 1);

	CView::OnInitialUpdate();
}

void CQsdGraphView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
//	TRACE("CQsdGraphView::OnUpdate() called with lHint = %d\n", (int)lHint);

	CDiscoveryDoc* pDoc	= GetDocument();

	// OnUpdate() is called with the HINT_DRAWSELECTION parameter, when the
	// selection state of an item in the list control of CQsdListView has changed.
	if (lHint == HINT_DRAWSELECTION)
	{
		//TRACE("CQsdGraphView::OnUpdate() - HINT_DRAWSELECTION, index=%d\n", *(int*)pHint);

		CClientDC dc(this);
		int ind = *(int*)pHint;
		DrawSelection(&dc, &pDoc->m_QsdRecords[ind].m_rectView);
	}

	// User shifted border line in histogram view (in prediction mode)
	else if (lHint == HINT_HISTOCHANGE && pDoc->m_iRunType == 4)
	{
		CView::OnUpdate(pSender, lHint, pHint);
	}
}

// Suppress WM_ERASEBKGND message to avoid flickering when double buffering
BOOL CQsdGraphView::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

// Callback function for the sorting of the datapoints for increasing z-view coordinates
static CQsdRecordArray* s_pQsdRecords=NULL;

static int _SortZ (const void* elem1, const void* elem2)
{
	CQsdRecord *p1 = &(*s_pQsdRecords)[*(int*)elem1];
	CQsdRecord *p2 = &(*s_pQsdRecords)[*(int*)elem2];
	return ((p1->m_afView[2] < p2->m_afView[2]) ? -1 :
			(p1->m_afView[2] > p2->m_afView[2]) ?  1 : 0);
};

void CQsdGraphView::OnDraw(CDC* pOutDC)
{
	CDiscoveryDoc* pDoc   = GetDocument();
	
	if (pDoc->m_QsdProfiles.GetSize() <= 0 || pDoc->m_iQsdSelProfile < 0)
		return;

	CQsdProfile* pProfile = &pDoc->m_QsdProfiles[pDoc->m_iQsdSelProfile];

	// Has Discovery Space three dimensions (usually), or only two or one?
	// If it has two or less dimensions, no properties are defined for the
	// z- (and y-) axis and all z- (and y-)values are zero. Then the "cube"
	// must be drawn in XY-projection only and cannot be rotated.
	int nDim = pDoc->GetQsdDimensionCount();

	// Update the view coordinates of all datapoints and retrieve the smallest
	// and highest x,y,z values in view coordinates (afMinMax[]) as the eight
	// points of the corners (afCorners[]):
	//   afCorners[0]: origin				(0,0,0)
	//            [1]: end of the x-axis	(1,0,0)
	//            [2]: end of the y-axis	(0,1,0)
	//            [3]: end of the z-axis	(0,0,1)
	//            [4]:						(1,1,0)
	//            [5]:                      (1,0,1)
	//            [6]:						(0,1,1)
	//            [7]:						(1,1,1)
	// afNormals[] contains the normal vectors of the six walls:
	//   afNormals[0]: xy wall at z=0
	//            [1]: xz wall at y=0
	//            [2]: yz wall at x=0
	//            [3]: xy wall at z=1
	//            [4]: xz wall at y=1
	//            [5]: yz wall at x=1
	// afAxesLabels[] contains the positions for the origin's and the axes'
	// labels. These are slightly different from the exact positions of the
	// corresponding corners.

	float afMinMax[6], afCorners[8][3], afNormals[6][3], afAxesLabels[4][3];
	UpdateViewCoords(afCorners, afNormals, afAxesLabels, afMinMax);

	// Calculate position and scaling factor from current client size.
	// Horizontal and vertical extent of the cube shall not exceed 2/3 of the
	// width and height, rsp., of the drawing area.

	CRect rectDraw, rectClient;
	float fScale, fPrintFac;
	GetClientRect(&rectClient);
	BOOL bPrint = pOutDC->IsPrinting();
	if ( bPrint )
	{
		// For printing (or print preview), we use the drawing area that has
		// been defined in the <CPrintInfo> parameter of the OnPrint() function.
		// We correct the scaling factor by the ratio between printer and
		// screen resolution.
		rectDraw	= m_rectDraw;
		float fx	= (float)rectDraw.Width();
		float fy	= (float)rectDraw.Height();
		float ffakx	= fx / (float)rectClient.Width();
		float ffaky	= fy / (float)rectClient.Height();
		fPrintFac	= ((ffakx < ffaky) ? ffakx : ffaky);
		fScale		= m_fScale * fPrintFac;
	}
	else
	{
		rectDraw	= rectClient;
		fScale		= m_fScale;
		fPrintFac	= 1.0f;
	};

	CSize sizeClient(rectDraw.Width(), rectDraw.Height());

	// Discovery Space can be shifted, <m_ptShift> describes the shift relative
	// to the center of the drawing area.
	CPoint ptCenter(rectDraw.Width()/2, rectDraw.Height()/2);
	ptCenter += m_ptShift;

	// When automatic adjustment is active, calculate scaling for each drawing
	// TO BE CHECKED

	//float fXScale = (float)(sizeClient.cx) / (afMinMax[3] - afMinMax[0]) * 2.0f / 3.0f;
	//float fYScale = (float)(sizeClient.cy) / (afMinMax[4] - afMinMax[1]) * 2.0f / 3.0f;
	//float fScale  = ((fXScale < fYScale) ? fXScale : fYScale);
	//float fScale = m_fScale;

	//TRACE("CQsdGraphView::OnDraw() - fXScale=%5.3f, fYScale=%5.3f\n", fXScale, fYScale);

	int iSaveDC = pOutDC->SaveDC();
	ASSERT( iSaveDC != 0 );

	// At first we determine which of the six walls of the cube are back walls and
	// which are front walls. A back wall has a normal vector pointing with a
	// negative z-value. If the normal vector's z-value is zero, the wall is treated
	// as open (the area would be invisible).
	// The front-most wall will get a thicker border than the other walls to help
	// the user where is the back and where is the rear of the cube. The front-most
	// wall is that wall with the biggest (positive) value of the normal vector's
	// z-coordinate. Exception: The cube is (approximately) in a 2D projection.
	BOOL abBack[6];
	float fFrontZ  = -1.0f;
	int iFrontWall = -1;				// index of the front-most wall
	for (register int i=0; i < 6; i++)
	{
		float fz  = afNormals[i][2];
		abBack[i] = (fz < 0.0f);
		if (fz > fFrontZ)
		{
			fFrontZ	   = fz;
			iFrontWall = i;
		};
	};
	if (fFrontZ >= 0.9999f)				// no thick border in 2D projection
		iFrontWall = -1;

	// For screen output, we use double buffering.
	// For printing and print preview, we directly draw to <pOutDC>.
	CDC dcBmp, *pDC;
	CBitmap Bmp, *pOldBmp = nullptr;
	if ( !bPrint )
	{
		dcBmp.CreateCompatibleDC(pOutDC);
		Bmp.CreateCompatibleBitmap(pOutDC, rectDraw.Width(), rectDraw.Height());
		pOldBmp = dcBmp.SelectObject(&Bmp);
		pDC		= &dcBmp;
	}
	else
	{
		pDC		= pOutDC;
	};

	// When we draw balls rather than dots, drawing may become a little bit lengthy,
	// so we show the hourglass cursor - but not when tracking is active.
	if ( m_bBalls && !m_bTracking )
		BeginWaitCursor();

	// Since we use double buffering and suppress the WM_ERASEBKGND message,
	// we must erase the background here.
	pDC->PatBlt(rectDraw.left, rectDraw.top, rectDraw.Width(), rectDraw.Height(),
				WHITENESS);

	// Check if we are in 2D projection
	BOOL b2D = Is2DProjection();

	//TRACE("CQsdGraphView::OnDraw() - b2D=%s\n", b2D ? "TRUE" : "FALSE");

	// Now we draw the back walls (three in clinographic projection, otherwise
	// two or only one back wall).
	CPen BackWallPen(PS_SOLID, 1, RGB(0,0,0));
	CPen FrontWallPen(PS_SOLID, 2, RGB(0,0,0));
	CBrush BackWallBrush(RGB(224,224,224));
	CPen* pOldPen	  = pDC->SelectObject(&BackWallPen);
	CBrush* pOldBrush = pDC->SelectObject(&BackWallBrush);
	for (i=0; i < 6; i++)
	{
		if ( abBack[i] )
		{
			// Collect the four corners of the wall number i
			float afWall[4][3];
			POINT ptWall[4];
			int i1, i2, i3, i4;
			if		(i == 0)
			{
				i1 = 0;		// indexes to afCorners[]
				i2 = 1;
				i3 = 4;
				i4 = 2;
			}
			else if (i == 1)
			{
				i1 = 0;
				i2 = 1;
				i3 = 5;
				i4 = 3;
			}
			else if (i == 2)
			{
				i1 = 0;
				i2 = 3;
				i3 = 6;
				i4 = 2;
			}
			else if (i == 3)
			{
				i1 = 3;
				i2 = 5;
				i3 = 7;
				i4 = 6;
			}
			else if (i == 4)
			{
				i1 = 2;
				i2 = 4;
				i3 = 7;
				i4 = 6;
			}
			else if (i == 5)
			{
				i1 = 1;
				i2 = 5;
				i3 = 7;
				i4 = 4;
			};

			for (register int kk=0; kk < 3; kk++)
			{
				afWall[0][kk] = afCorners[i1][kk];
				afWall[1][kk] = afCorners[i2][kk];
				afWall[2][kk] = afCorners[i3][kk];
				afWall[3][kk] = afCorners[i4][kk];
			};

			for (kk=0; kk < 4; kk++)
			{
				ptWall[kk].x  = ptCenter.x + afWall[kk][0]*fScale;
				ptWall[kk].y  = ptCenter.y - afWall[kk][1]*fScale;
			};

			// Draw the wall as four vertex polygon
			pDC->Polygon(ptWall, 4);
		}
	};

	// Labelling, grid, ticks, etc.

	CFont LabelFont, *pOldFont;
	LabelFont.CreatePointFont((int)ROUND(85.0f/*90.0f*/*fPrintFac), "MPDS"/*"Arial"*/);
	pOldFont = pDC->SelectObject(&LabelFont);
	pDC->SetTextColor(RGB(0,0,0));
	pDC->SetBkMode(TRANSPARENT);

	// Now label the origin (0) and the three main axes' ends (x,y,z) -
	// if not in 2D projection.
	if ( !b2D )
	{
		CPoint ptLabel;
		char szLabel[4];
		szLabel[1] = '\0';
		pDC->SetTextAlign(TA_CENTER | TA_BOTTOM);
		for (i=0; i < 4; i++)
		{
			ptLabel.x  = ptCenter.x + afAxesLabels[i][0]*fScale;
			ptLabel.y  = ptCenter.y - afAxesLabels[i][1]*fScale;
			szLabel[0] = ((i > 0) ? 'x'+(char)(i-1) : '0');
			pDC->TextOut(ptLabel.x, ptLabel.y, szLabel, 1);
		}
	}

	// If we are in 2D projection, we draw the grid and the ticks on the two axes
	// in the 2D plane. For each of the three axes, we check, if it points right,
	// left, top, or bottom.
	else	// if ( b2D )
	{
		float afVectors[3][3];
		for (i=0; i < 3; i++)
		{
			afVectors[0][i] = afCorners[1][i] - afCorners[0][i];	// x-axis
			afVectors[1][i] = afCorners[2][i] - afCorners[0][i];	// y-axis
			afVectors[2][i] = afCorners[3][i] - afCorners[0][i];	// z-axis
		};

		//TRACE("x-axis=(%6.4f,%6.4f,%6.4f), y-axis=(%6.4f,%6.4f,%6.4f), z-axis=(%6.4f,%6.4f,%6.4f)\n",
		//	afVectors[0][0], afVectors[0][1], afVectors[0][2],
		//	afVectors[1][0], afVectors[1][1], afVectors[1][2],
		//	afVectors[2][0], afVectors[2][1], afVectors[2][2]);

		// aiAxes[0] stores +1 if the x-axis points toward right, -1 if toward left
		// (+2, -2, +3, -3 for y- and z-axis, rsp.).
		// aiAxes[1] stores +1 if the x-axis points to the top, -1 if to the bottom, etc.
		int aiAxes[2];
		aiAxes[0]=aiAxes[1]=-1;
		if (fabs(fabs(afVectors[0][0])-1.0f) <= 1e-4)
			aiAxes[0] = ((afVectors[0][0] > 0.0f) ? 1 : -1);
		if (fabs(fabs(afVectors[0][1])-1.0f) <= 1e-4)
			aiAxes[1] = ((afVectors[0][1] > 0.0f) ? 1 : -1);
		if (fabs(fabs(afVectors[1][0])-1.0f) <= 1e-4)
			aiAxes[0] = ((afVectors[1][0] > 0.0f) ? 2 : -2);
		if (fabs(fabs(afVectors[1][1])-1.0f) <= 1e-4)
			aiAxes[1] = ((afVectors[1][1] > 0.0f) ? 2 : -2);
		if (fabs(fabs(afVectors[2][0])-1.0f) <= 1e-4)
			aiAxes[0] = ((afVectors[2][0] > 0.0f) ? 3 : -3);
		if (fabs(fabs(afVectors[2][1])-1.0f) <= 1e-4)
			aiAxes[1] = ((afVectors[2][1] > 0.0f) ? 3 : -3);

		//TRACE("aiAxes[]=(%d,%d)\n", aiAxes[0], aiAxes[1]);

		// Determine the rectangle of the corners of the square that is the
		// back wall and will bear the grid.
		int iaxis1 = abs(aiAxes[0]);						// 1=x-, 2=y-, 3=z-axis
		int iaxis2 = abs(aiAxes[1]);
		CPoint pt0, pt1, pt2;
		pt0.x = ptCenter.x + afCorners[0][0]*fScale;		// DC position of origin
		pt0.y = ptCenter.y - afCorners[0][1]*fScale;
		pt1.x = ptCenter.x + afCorners[iaxis1][0]*fScale;	// end of horizontal axis
		pt1.y = ptCenter.y - afCorners[iaxis1][1]*fScale;
		pt2.x = ptCenter.x + afCorners[iaxis2][0]*fScale;	// end of vertical axis
		pt2.y = ptCenter.y - afCorners[iaxis2][1]*fScale;

		// Save the square's dimensions and the identification of the vertical
		// axis for slab definition mode.
		m_rectCube2D.SetRect(pt0.x, pt0.y, pt1.x, pt2.y);
		//m_rectCube2D.NormalizeRect();
		m_iVertAxis	  = aiAxes[1];

		// Test
		//pDC->FillSolidRect(pt0.x-4, pt0.y-4, 8, 8, RGB(255,0,0));
		//pDC->FillSolidRect(pt1.x-4, pt1.y-4, 8, 8, RGB(0,255,0));
		//pDC->FillSolidRect(pt2.x-4, pt2.y-4, 8, 8, RGB(0,0,255));

		// Ranges and stepwidths for horizontal and vertical axis
		int id1		  = pt1.x - pt0.x;					// pixels on horizontal axis
		int id2		  = pt2.y - pt0.y;					// pixels on vertical axis
		float fRange1 = pDoc->m_afQsdScalings[iaxis1-1];// value range on horizontal axis
		float fRange2= pDoc->m_afQsdScalings[iaxis2-1];	// value range on vertical axis
		/**TEST**/
		if (fRange1 < 0.001f)							// range values may be zero for...
			fRange1 = 1.0f;								// ...one-dimensional discovery space
		if (fRange2 < 0.001f)
			fRange2 = 1.0f;
		ASSERT( fRange1 > 0.0f && fRange2 > 0.0f );
		float fScale1 = (float)id1 / fRange1;			// pixels per unit on horizontal axis
		float fScale2 = (float)id2 / fRange2;			// dito for vertical axis
		float flog	  = floor(log10(fRange1)) - 1.0f;
		float fStep1  = pow(10.0f, flog);				// pow(float,float);
		flog		  = floor(log10(fRange2)) - 1.0f;
		float fStep2  = pow(10.0f, (int)flog);

		//TRACE("fRange1=%6.4f, fRange2=%6.4f, fStep1=%6.4f, fStep2=%6.4f\n",
		//	fRange1, fRange2, fStep1, fStep2);

		int imax1 = 10/*5*/;	// relative stepwidth for ticks to be labelled...
		int imax2 = 5;			// ...and for the grid lines
		int ix, iy, im, ilen;
		float fx, fy, fxx, fyy;
		char szHelp[64];

		// Draw the grid - if wanted
		if ( m_bGrid )
		{
			CPen GridPen(PS_DOT, 1, RGB(128,128,128));
			pDC->SelectObject(&GridPen);
			fxx = fStep1 * (float)imax1;
			fyy = fStep2 * (float)imax2;
			for (fx=fxx; fx < fRange1; fx += fxx)
			{
				ix = pt0.x + fx*fScale1;
				pDC->MoveTo(ix, pt0.y);
				pDC->LineTo(ix, pt2.y);
			};
			for (fy=fyy; fy < fRange2; fy += fyy)
			{
				iy = pt0.y + fy*fScale2;
				pDC->MoveTo(pt0.x, iy);
				pDC->LineTo(pt1.x, iy);
			};
			pDC->SelectObject(pOldPen);
		};

		// Draw ticks and labels on horizontal and vertical axis
		for (im=1, fx=fStep1; fx < fRange1; im++, fx += fStep1)
		{
			BOOL bLong = ((im % imax1) == 0);
			ilen = (bLong ? 5 : 2);
			ix	 = pt0.x + fx*fScale1;
			if ( bLong )
			{
				sprintf(szHelp, "%3.2f", fx);
				TrimFloat(szHelp);				// "123.00" -> "123", "0.40" -> "0.4"
			};
			if (fScale2 > 0.0f)
			{
				pDC->MoveTo(ix, pt0.y-ilen);	// don'let ticks grow inside the wall
				pDC->LineTo(ix, pt0.y);
				if ( bLong )
				{
					pDC->SetTextAlign(TA_BOTTOM | TA_CENTER);
					pDC->TextOut(ix, pt0.y-ilen, szHelp, strlen(szHelp));
				}
			}
			else
			{
				pDC->MoveTo(ix, pt0.y);
				pDC->LineTo(ix, pt0.y+ilen);
				if ( bLong )
				{
					pDC->SetTextAlign(TA_TOP | TA_CENTER);
					pDC->TextOut(ix, pt0.y+ilen, szHelp, strlen(szHelp));
				}
			}
		};

		// Label for the horizontal axis somewhat behind the end of that axis
		szHelp[0] = (char)(iaxis1 - 1) + 'x';
		szHelp[1] = '\0';
		pDC->TextOut(pt0.x+fRange1*1.02f*fScale1,
					 (fScale2 > 0.0f) ? pt0.y-5 : pt0.y+5,
					 szHelp, strlen(szHelp));

		// On the vertical axis, unlike the horizontal axis, we start with zero
		for (im=0, fy=0.0f; fy < fRange2; im++, fy += fStep2)
		{
			BOOL bLong = ((im % imax2) == 0);
			ilen = (bLong ? 5 : 2);
			iy	 = pt0.y + fy*fScale2;
			if ( bLong )
			{
				sprintf(szHelp, "%3.2f", fy);
				TrimFloat(szHelp);
			};
			if (fScale1 > 0.0f)
			{
				pDC->MoveTo(pt0.x-ilen, iy);
				pDC->LineTo(pt0.x     , iy);
				if ( bLong )
				{
					pDC->SetTextAlign(TA_RIGHT | TA_BASELINE);
					pDC->TextOut(pt0.x-ilen, iy, szHelp, strlen(szHelp));
				}
			}
			else
			{
				pDC->MoveTo(pt0.x     , iy);
				pDC->LineTo(pt0.x+ilen, iy);
				if ( bLong )
				{
					pDC->SetTextAlign(TA_LEFT | TA_BASELINE);
					pDC->TextOut(pt0.x+ilen, iy, szHelp, strlen(szHelp));
				}
			}
		};

		// Label for the vertical axis
		szHelp[0] = (char)(iaxis2 - 1) + 'x';
		szHelp[1] = '\0';
		pDC->TextOut((fScale1 > 0.0f) ? pt0.x-5 : pt0.x+5,
					 pt0.y+fRange2*1.02f*fScale2,
					 szHelp, strlen(szHelp));
	};

	// Sort the datapoints for increasing z-view coordinates,
	// and draw them from back to the front.
	if (m_piSort != NULL)
		delete m_piSort;
	int np = pDoc->m_QsdRecords.GetSize();
	register int ip;
	if (np > 0)
	{
		m_piSort = new int[np];
		for (ip=0; ip < np; ip++)
			m_piSort[ip] = ip;
		s_pQsdRecords = &pDoc->m_QsdRecords;
		qsort(m_piSort, np, sizeof(int), _SortZ);

		int nGrey=0, nNotGrey=0;

		for (ip=0; ip < np; ip++)
		{
			CPoint pt;
			CQsdRecord* pq = &pDoc->m_QsdRecords[m_piSort[ip]];

			// Reset the 2D rectangle that is used to find the datapoint on mouse-
			// click, because the datapoint may not be drawn.
			::SetRectEmpty(&pq->m_rectView);

			// The datapoint may have undefined coordinates
			if ( pq->m_bNoCoords )
				continue;

			// If a slab has been defined, perform clipping
			if ( m_bSlabActive )
			{
				if (pDoc->m_iQsdSlabAxis == 1)
				{
					if (pq->m_afXyz[0] < pDoc->m_afQsdSlabRange[0] || 
						pq->m_afXyz[0] > pDoc->m_afQsdSlabRange[1])
						continue;
				}
				else if (pDoc->m_iQsdSlabAxis == 2)
				{
					if (pq->m_afXyz[1] < pDoc->m_afQsdSlabRange[0] || 
						pq->m_afXyz[1] > pDoc->m_afQsdSlabRange[1])
						continue;
				}
				else if (pDoc->m_iQsdSlabAxis == 3)
				{
					if (pq->m_afXyz[2] < pDoc->m_afQsdSlabRange[0] || 
						pq->m_afXyz[2] > pDoc->m_afQsdSlabRange[1])
						continue;
				}
			};

			// Make a simple 3D to 2D transformation (parallel projection)
			// by simply ignoring the z-coordinate of the datapoint.
			pt.x = ptCenter.x + pq->m_afView[0]*fScale;
			pt.y = ptCenter.y - pq->m_afView[1]*fScale;

			// Get the main color of the datapoint depending on the compound property.
			// In prediction mode, the color may be grey instead.
			COLORREF crMain;
			if (pq->m_bPredict && pDoc->m_iRunType == 4 &&
				(int)pq->m_fProb <= pDoc->m_iProbFilter)
			{
				crMain = RGB(128,128,128);
				nGrey++;
			}
			else
			{
				crMain = pq->GetColor();
				nNotGrey++;
			};

			// Get the device coordinates of the surrounding rectangle and store
			// these in <CQsdRecord::m_rectView> for WM_LBUTTONDOWN.
			int iRadius = (int)ROUND(2.0f*fPrintFac);	// TO BE CHECKED (radius)
			CRect rectPoint;

			// Now draw the ball or the dot, rsp.
			if ( m_bBalls )
			{
				CBrush brBall, *pOldBrush2;
				brBall.CreateSolidBrush(crMain);
				pOldBrush2 = pDC->SelectObject(&brBall);
				pDC->SelectStockObject(BLACK_PEN);
				rectPoint.SetRect(pt.x-iRadius, pt.y-iRadius, pt.x+iRadius+1, pt.y+iRadius+1);
				pDC->Ellipse(&rectPoint);
				pDC->SelectObject(pOldBrush2);
			}
			else
			{
				pDC->SetPixel(pt, crMain);
				rectPoint.SetRect(pt.x-1, pt.y-1, pt.x+2, pt.y+2);
			};

			::CopyRect(&pq->m_rectView, &rectPoint);
		
		};		// END for (ip..np)

		TRACE("CQsdGraphView::OnDraw() - nGrey = %d, nNotGrey = %d\n", nGrey, nNotGrey);
	}
	else
		m_piSort = NULL;

	// Finally draw the edges of the walls that are not back walls
	pDC->SelectStockObject(HOLLOW_BRUSH);
	for (i=0; i < 6; i++)
	{
		if ( !abBack[i] )
		{
			float afWall[4][3];
			POINT ptWall[4];
			int i1, i2, i3, i4;
			if		(i == 0)
			{
				i1 = 0;
				i2 = 1;
				i3 = 4;
				i4 = 2;
			}
			else if (i == 1)
			{
				i1 = 0;
				i2 = 1;
				i3 = 5;
				i4 = 3;
			}
			else if (i == 2)
			{
				i1 = 0;
				i2 = 3;
				i3 = 6;
				i4 = 2;
			}
			else if (i == 3)
			{
				i1 = 3;
				i2 = 5;
				i3 = 7;
				i4 = 6;
			}
			else if (i == 4)
			{
				i1 = 2;
				i2 = 4;
				i3 = 7;
				i4 = 6;
			}
			else if (i == 5)
			{
				i1 = 1;
				i2 = 5;
				i3 = 7;
				i4 = 4;
			};

			for (register int kk=0; kk < 3; kk++)
			{
				afWall[0][kk] = afCorners[i1][kk];
				afWall[1][kk] = afCorners[i2][kk];
				afWall[2][kk] = afCorners[i3][kk];
				afWall[3][kk] = afCorners[i4][kk];
			};

			for (kk=0; kk < 4; kk++)
			{
				ptWall[kk].x  = ptCenter.x + afWall[kk][0]*fScale;
				ptWall[kk].y  = ptCenter.y - afWall[kk][1]*fScale;
			};

			// The front-most wall will get a thicker border than the others
			pDC->SelectObject((i == iFrontWall) ? &FrontWallPen : &BackWallPen);

			// Draw the wall as four vertex polygon
			pDC->Polygon(ptWall, 4);
		}
	};

	// Draw the legend of the elemental properties, i.e. for the x-, y- and z-axis.
	// Note: Representation may be two- or one-dimensional. In these cases, the
	// property and operation indexes are zero.
	CStringArray astr;
	CString strOp;
	int nprop = ((pProfile->m_ip3 > 0) ? 3 : 2);
	astr.SetSize(nprop);
	astr[0].Format("x: %s / %s", pDoc->m_astrQsdProperties[pProfile->m_ip1-1],
								 GetQsdOperationString(strOp, pProfile->m_io1));
	if (pProfile->m_ip2 > 0)
		astr[1].Format("y: %s / %s", pDoc->m_astrQsdProperties[pProfile->m_ip2-1],
									 GetQsdOperationString(strOp, pProfile->m_io2));
	else
		astr[1] = "y: <none>";
	if (pProfile->m_ip3 > 0)
		astr[2].Format("z: %s / %s", pDoc->m_astrQsdProperties[pProfile->m_ip3-1],
									 GetQsdOperationString(strOp, pProfile->m_io3));

	CSize sizeText(0,0);
	for (i=0; i < nprop; i++)
	{
		CSize sizt = pDC->GetTextExtent(astr[i]);
		if (sizt.cx > sizeText.cx)
			sizeText.cx = sizt.cx;
		sizeText.cy = sizt.cy;
	};

	// Maximum width of legend: Half of the drawing area's total width, since we
	// may need equivalent space on the right side for the system property legend.
	int iHalfWidth = rectClient.Width() / 2 - 2;
	if (sizeText.cx > iHalfWidth)
		sizeText.cx = iHalfWidth;
	sizeText.cx		= (int)ROUND((float)sizeText.cx*fPrintFac);

	CRect rectText(rectDraw.left+2, rectDraw.bottom-2-nprop*sizeText.cy,
				   rectDraw.left+sizeText.cx+6, rectDraw.bottom-1);
	pDC->SelectStockObject(BLACK_PEN);
	pDC->SelectStockObject(WHITE_BRUSH);
	pDC->Rectangle(&rectText);

	CRect rectLine;
	pDC->SetTextAlign(TA_LEFT | TA_TOP);
	rectLine.left	= rectText.left + 2;
	rectLine.right	= rectText.right;
	rectLine.top	= rectDraw.bottom-1-nprop*sizeText.cy;
	rectLine.bottom	= rectLine.top + sizeText.cy;
	pDC->DrawText(astr[0], rectLine, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
	rectLine.top	= rectDraw.bottom-1-(nprop-1)*sizeText.cy;
	rectLine.bottom	= rectLine.top + sizeText.cy;
	pDC->DrawText(astr[1], rectLine, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
	if (nprop == 3)
	{
		rectLine.top	= rectDraw.bottom-1-sizeText.cy;
		rectLine.bottom	= rectLine.top + sizeText.cy;
		pDC->DrawText(astr[2], rectLine, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
	};

	// Draw the legend with the system properties into the lower right corner.
	// For that, which of the maximum MAX_SYSPROP_QSD system properties have been
	// defined in the current set of QSD records. The indexes and identification
	// strings will be stored in aiValues[] and astr[], rsp.
	int aiValues[MAX_SYSPROP_QSD], aiCounts[MAX_SYSPROP_QSD];
	int nValues = 0;	
	memset(aiCounts, 0, sizeof(aiCounts));
	int nGrey	= 0;		// counts grey balls in prediction mode
	for (ip=0; ip < np; ip++)
	{
		CQsdRecord* pq	= &pDoc->m_QsdRecords[ip];
		BOOL bGrey		= (pq->m_bPredict && (int)pq->m_fProb <= pDoc->m_iProbFilter);
		if ( bGrey )
		{
			nGrey++;
		}
		else
		{
			int iValue = pq->GetPropertyIndex();
			if (iValue < 0)		// -1 = undefined property
				continue;
			ASSERT( iValue >= 0 && iValue < MAX_SYSPROP_QSD );
			aiCounts[iValue]++;
		}
	};
	for (nValues=0, ip=0; ip < MAX_SYSPROP_QSD; ip++)
	{
		if (aiCounts[ip] > 0)
			aiValues[nValues++] = ip;
	};
	if (nGrey > 0 && nValues < MAX_SYSPROP_QSD)
		aiValues[nValues++] = -1;	// property "?"

	astr.RemoveAll();
	astr.SetSize(nValues, MAX_SYSPROP_QSD);
	CSize sizeMax(0,0);
	register int ileg, nleg=pDoc->m_astrQsdLegend.GetSize();
	for (ip=0; ip < nValues; ip++)
	{
		if (aiValues[ip] >= 0)
		{
			int ival = aiValues[ip];
			for (ileg=0; ileg < nleg; ileg++)
			{
				CString& strLeg = pDoc->m_astrQsdLegend[ileg];
				if (atoi(strLeg) == ival)
					astr[ip] = strLeg;
			}
			//astr[ip].Format("%d", aiValues[ip]);		// TO BE CHECKED
		}
		else
			astr[ip] = "?";
		CSize sizt = pDC->GetTextExtent(astr[ip]);
		if (sizt.cx > sizeMax.cx)
			sizeMax.cx = sizt.cx;
		if (sizt.cy > sizeMax.cy)
			sizeMax.cy = sizt.cy;
	};
	int irad = (int)ROUND(12.0f*fPrintFac);
	if (sizeMax.cy < irad)		// minimum height caused by the ball symbol
		sizeMax.cy = irad;
	sizeMax.cx += irad+4;		// additional width needed for the ball symbol

	if (sizeMax.cx > iHalfWidth)
		sizeMax.cx = iHalfWidth;
	sizeMax.cx		= (int)ROUND((float)sizeMax.cx*fPrintFac);

	CRect rectLegend(rectDraw.right-sizeMax.cx-2, rectDraw.bottom-nValues*sizeMax.cy-2,
					 rectDraw.right-2, rectDraw.bottom-2);
	pDC->Rectangle(&rectLegend);

	rectLine.left	= rectLegend.left+irad+2;
	rectLine.right	= rectLegend.right;

	int iy;
	int iyofs = (sizeMax.cy - irad) / 2;	// offset for ball if text is higher
	pDC->SelectStockObject(BLACK_PEN);
	pDC->SetTextAlign(TA_TOP | TA_LEFT);
	pDC->SetTextColor(RGB(0,0,0));
	for (ip=0, iy=rectLegend.top; ip < nValues; ip++, iy += sizeMax.cy)
	{
		COLORREF crMain;
		if (aiValues[ip] >= 0)
			crMain = CQsdRecord::GetColor(aiValues[ip]);
		else
			crMain = RGB(128,128,128);
		CBrush brBall(crMain);
		CBrush* pOldBrush2	= pDC->SelectObject(&brBall);
		pDC->Ellipse(rectLegend.left, iy+iyofs, rectLegend.left+irad, iy+iyofs+irad);
		pDC->SelectObject(pOldBrush2);
		rectLine.top		= iy;
		rectLine.bottom		= iy + sizeMax.cy;
		pDC->DrawText(astr[ip], rectLine, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
		//pDC->TextOut(rectLegend.left+irad+2, iy, astr[ip]);
	};

	// If slab has been defined, print its range at the top of the client area
	if ( m_bSlabActive )
	{
		CString strSlab;
		strSlab.Format("Slab: %c = %5.2f .. %5.2f", (char)(pDoc->m_iQsdSlabAxis-1+'x'),
					   pDoc->m_afQsdSlabRange[0], pDoc->m_afQsdSlabRange[1]);
		pDC->TextOut(100, 0, strSlab);
	};

	// The end of drawing is near, so restore pens etc.
	pDC->SelectObject(pOldFont);
	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);

	if ( m_bBalls && !m_bTracking )
		EndWaitCursor();

	// Now the bitmap is ready and can be copied to the output device.
	if ( !bPrint )
	{
		pOutDC->BitBlt(rectDraw.left, rectDraw.top, rectDraw.right, rectDraw.bottom,
					   &dcBmp, 0, 0, SRCCOPY);
		dcBmp.SelectObject(pOldBmp);
		Bmp.DeleteObject();

		// If a slab has been defined, draw a warning symbol to indicate that
		// datapoints are clipped (not displayed) that are outside the slab.
		if ( m_bSlabActive )
		{
			CDC dcSlab;
			CBitmap bmpSlab;
			bmpSlab.LoadBitmap(IDB_SLABWARN);
			dcSlab.CreateCompatibleDC(pOutDC);
			CBitmap *pOldBmp2 = dcSlab.SelectObject(&bmpSlab);
			pOutDC->BitBlt(rectDraw.left, rectDraw.top, 40, 40, &dcSlab, 0, 0, SRCCOPY);
			dcSlab.SelectObject(pOldBmp2);
			bmpSlab.DeleteObject();
		};

		// Finally draw the selections directly to screen rather than to bitmap
		DrawSelections(pOutDC);
	};

	VERIFY( pOutDC->RestoreDC(iSaveDC) );
}

// Draw rectangle <lpRect> in XOR drawing mode into device context <pDC>
void CQsdGraphView::DrawSelection (CDC* pDC, LPCRECT lpRect)
{
	CPen RectPen(PS_SOLID, 0, RGB(255,255,0));
	CPen *pOldPen	  = pDC->SelectObject(&RectPen);
	CBrush *pOldBrush = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
	int iOldROP2 = pDC->SetROP2(R2_XORPEN);
	pDC->Rectangle(lpRect);
	pDC->SelectObject(pOldPen);
	pDC->SelectObject(pOldBrush);
	pDC->SetROP2(iOldROP2);
};

// Draw the rectangles of all selected CQsdRecord objects in XOR mode
int CQsdGraphView::DrawSelections (CDC* pDC)
{
	int nSel = 0;
	CDiscoveryDoc* pDoc = GetDocument();
	for (register int iq=0, nq=pDoc->m_QsdRecords.GetSize(); iq < nq; iq++)
	{
		CQsdRecord *pq = &pDoc->m_QsdRecords[iq];
		if ( pq->m_bSel )
		{
			DrawSelection(pDC, &pq->m_rectView);
			nSel++;
		}
	};
	return nSel;
};

// Draws the slab rectangle in XOR mode
void CQsdGraphView::DrawSlabRect (CDC* pDC)
{
	int iOldROP2		= pDC->SetROP2(R2_XORPEN);
	CPen* pOldPen		= (CPen*)pDC->SelectStockObject(WHITE_PEN);
	CBrush* pOldBrush	= (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
	CRect rect(m_rectSlab);
	rect.NormalizeRect();		// we need a normalized copy for drawing
	pDC->Rectangle(&rect);
	pDC->SelectObject(pOldBrush);
	pDC->SelectObject(pOldPen);
	pDC->SetROP2(iOldROP2);
};

// Draws the current settings of the slab range <afRange> at top of the client area
void CQsdGraphView::MonitorSlabRange (CDC* pDC, float afRange[2])
{
	CFont Font;
	Font.CreatePointFont(90, "Arial", pDC);
	int iOldBkMode		= pDC->SetBkMode(OPAQUE);
	COLORREF crOldBk	= pDC->SetBkColor(RGB(255,255,255));
	COLORREF crOldText	= pDC->SetTextColor(RGB(0,0,0));
	CFont* pOldFont		= pDC->SelectObject(&Font);
	CString strText;
	strText.Format("Defining slab. Current range: %5.2f .. %5.2f    ",
				   afRange[0], afRange[1]);
	pDC->TextOut(0, 0, strText);
	pDC->SelectObject(pOldFont);
	pDC->SetTextColor(crOldText);
	pDC->SetBkColor(crOldBk);
	pDC->SetBkMode(iOldBkMode);
};

// Calculates the data range along the vertical axis from the current size of
// the slab rectangle.
// TRUE if range values are defined, FALSE if slab rectangle is not defined.
BOOL CQsdGraphView::GetSlabRange (float afRange[2])
{
	if ( m_rectSlab.IsRectNull() )
		return FALSE;

	CRect rectRange(m_rectSlab);
	rectRange.NormalizeRect();
	CDiscoveryDoc* pDoc = GetDocument();

	float fScale		= pDoc->m_afQsdScalings[abs(m_iVertAxis)-1] /
							(float)m_rectCube2D.Height();
	afRange[0]			= (float)(m_rectSlab.top    - m_rectCube2D.top) * fScale;
	afRange[1]			= (float)(m_rectSlab.bottom - m_rectCube2D.top) * fScale;
	if (afRange[1] < afRange[0])
	{
		float ftemp		= afRange[1];
		afRange[1]		= afRange[0];
		afRange[0]		= ftemp;
	};
	return TRUE;
};

/////////////////////////////////////////////////////////////////////////////
// CQsdGraphView printing

BOOL CQsdGraphView::OnPreparePrinting(CPrintInfo* pInfo) 
{
	return DoPreparePrinting(pInfo);
}

void CQsdGraphView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	// Register the drawing area for the central drawing function OnDraw()
	m_rectDraw = pInfo->m_rectDraw;

	OnDraw(pDC);
	
	//CView::OnPrint(pDC, pInfo);
}

BOOL CQsdGraphView::Is2DProjection (void)
{
	// Check if we are in 2D projection. For that, we take the three rows of
	// the orientation matrix (3x3 submatrix) as vectors of the axes x,y,z.
	// For each of these vectors, two components must be 0.0 and one 1.0.
	// To check this, we look for the sum of the absolute values, which must
	// be 1.0. In clinographic projections the sum is greater than 1.0 (whereas
	// the sum of the squares is 1.0 in any case).
	BOOL b2D = ((fabs(m_omat[0][0]) + fabs(m_omat[0][1]) + fabs(m_omat[0][2]) <= 1.0001f) &&
				(fabs(m_omat[1][0]) + fabs(m_omat[1][1]) + fabs(m_omat[1][2]) <= 1.0001f) &&
				(fabs(m_omat[2][0]) + fabs(m_omat[2][1]) + fabs(m_omat[2][2]) <= 1.0001f));
	return b2D;
};

/////////////////////////////////////////////////////////////////////////////
// CQsdGraphView diagnostics

#ifdef _DEBUG
void CQsdGraphView::AssertValid() const
{
	CView::AssertValid();
}

void CQsdGraphView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

CDiscoveryDoc* CQsdGraphView::GetDocument()
{
	ASSERT_KINDOF( CDiscoveryDoc, m_pDocument );
	return (CDiscoveryDoc*)m_pDocument;
};

/////////////////////////////////////////////////////////////////////////////
// CQsdGraphView message handlers

BOOL CQsdGraphView::PreTranslateMessage(MSG* pMsg) 
{
	// Since WM_KEYDOWN does not receive a keystroke on the Escape key, we must
	// handle this key here in PreTranslateMessage().
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
	{
		TRACE("CQsdGraphView::PreTranslateMessage() called with VK_ESCAPE\n");

		if ( !m_bTracking )
			m_iTracking = 0;
		
		if ( m_bSlabMode )
		{
			m_bSlabMode = FALSE;
			m_rectSlab.SetRectEmpty();	// to avoid confusion in OnMouseMove()
		};

		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	};
	
	return CView::PreTranslateMessage(pMsg);
}

void CQsdGraphView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// Process special keystrokes....
	float afIncr[3], fFactor;
	if		((GetKeyState(VK_SHIFT)   & 0xFF00) != 0)	// five times faster with Shift key
		fFactor = 5.0f;
	else if ((GetKeyState(VK_CONTROL) & 0xFF00) != 0)	// five times slower with Control key
		fFactor = 0.2f;
	else
		fFactor = 1.0f;
	fFactor *= (float)nRepCnt;	// keystroke repetition counter
	afIncr[0]=afIncr[1]=afIncr[2]=0.0f;

	switch (nChar)
	{
	case VK_LEFT:				// cursor left  -> (-)rotation along y-axis
	case VK_RIGHT:				// cursor right -> (+)rotation along y-axis
		// Do not rotate if we have only two or less dimensions in Discovery Space
		if (GetDocument()->GetQsdDimensionCount() == 3)
		{
			afIncr[1] = ((nChar == VK_RIGHT) ? -1.0f*fFactor : fFactor);
			Rotate(2, afIncr);
		};
		break;
	case VK_UP:					// cursor up/down -> rotation along x-axis
	case VK_DOWN:
		afIncr[0] = ((nChar == VK_DOWN) ? -1.0f*fFactor : fFactor);
		Rotate(1, afIncr);
		break;
	case VK_PRIOR:				// page up/down -> rotation along z-axis
	case VK_NEXT:
		afIncr[2] = ((nChar == VK_PRIOR) ? -1.0f*fFactor : fFactor);
		Rotate(4, afIncr);
		break;
	case VK_NUMPAD4:			// cursor on numeric pad -> shift (default: 5 pixels)
		m_ptShift.x -= (int)fFactor*5;
		break;
	case VK_NUMPAD6:
		m_ptShift.x += (int)fFactor*5;
		break;
	case VK_NUMPAD8:
		m_ptShift.y -= (int)fFactor*5;
		break;
	case VK_NUMPAD2:
		m_ptShift.y += (int)fFactor*5;
		break;
	case VK_HOME:				// enlarge scaling factor (default: 1.1-fold)
		m_fScale *= (1.0f + fFactor*0.1f);
		break;
	case VK_END:				// decrease scaling factor
		m_fScale /= (1.0f + fFactor*0.1f);
		break;
	default:
		CView::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	};

	// Redraw the view with updated rotation matrix, scaling factor, or position
	RedrawWindow();
}

BOOL CQsdGraphView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// Set special cursor when application is in tracking mode. This cursor
	// uses a special symbol indicating that tracking starts when the user
	// presses down the left mouse button (tracking).
	if (m_iTracking > 0)
	{
		HCURSOR hCursor = AfxGetApp()->LoadCursor((m_iTracking == 1) ? IDC_TRACKROTXY :
												  (m_iTracking == 2) ? IDC_TRACKROTZ  :
												  (m_iTracking == 3) ? IDC_TRACKSHIFT :
																	   IDC_TRACKZOOM);
		::SetCursor(hCursor);
		return FALSE;
	}
	else if ( m_bSlabMode )
	{
		::SetCursor(AfxGetApp()->LoadCursor(IDC_SLABMODE));
		return TRUE;
	}
	
	return CView::OnSetCursor(pWnd, nHitTest, message);
}

void CQsdGraphView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// When application is in tracking mode, track the Discovery Space (rotate,
	// shift, or zoom) unless the user releases the left mouse button.
	// Otherwise use this message to select the datapoint at the current mouse position.
	if (m_iTracking > 0)
	{
		// Set mouse capture to avoid loosing WM_MOUSEMOVE messages when the
		// cursor gets out of this view.
		SetCapture();
		m_bTracking = TRUE;

		// Save current mouse position. This will be used by the WM_MOUSEMOVE
		// handler to calculate the amount of rotation, shift, or zoom.
		m_ptLastMouse = point;

		// Now set the cursor symbol according to the type of tracking. Since the
		// mouse is captured while tracking, this symbol will not be overwritten
		// by the WM_SETCURSOR handler.
		HCURSOR hCursor = AfxGetApp()->LoadCursor((m_iTracking == 1) ? IDC_TRACKROTXY_A :
												  (m_iTracking == 2) ? IDC_TRACKROTZ_A  :
												  (m_iTracking == 3) ? IDC_TRACKSHIFT_A :
																	   IDC_TRACKZOOM_A);
		::SetCursor(hCursor);
	}

	// When in slab definition mode, define a rectangle with the top at the
	// current mouse position and left and right end at the left and right end
	// of the 2D-projected cube. The bottom will follow the mouse movements
	// unless the user releases the left mouse button.
	else if ( m_bSlabMode )
	{
		SetCapture();

		m_rectSlab.top		= point.y;
		m_rectSlab.bottom	= point.y + 1;
		m_rectSlab.left		= m_rectCube2D.left;
		m_rectSlab.right	= m_rectCube2D.right;

		CClientDC dc(this);
		DrawSlabRect(&dc);

		// Prepare display of current slab range at top of the client area
		float afRange[2];
		GetSlabRange(afRange);
		MonitorSlabRange(&dc, afRange);
	}

	// This will handle selection of datapoints
	else
	{
		CDiscoveryDoc* pDoc = GetDocument();

		// If SHIFT key is down, use extended selection
		BOOL bExtSel = ((nFlags & MK_SHIFT) != 0);

		// See if there is an object at position <point>. To find the object
		// move through the sorted list of datapoints from front to the rear.
		int ind		= -1;
		int nPoints	= pDoc->m_QsdRecords.GetSize();
		ASSERT( m_piSort != NULL );
		for (register int ip=nPoints-1; ip >= 0 && ind == -1; ip--)
		{
			if ( ::PtInRect(&pDoc->m_QsdRecords[m_piSort[ip]].m_rectView, point) )
				ind = m_piSort[ip];
		};

		if (ind >= 0)
		{
			TRACE("CQsdGraphView::OnLButtonDown() - object at (%d,%d) has index %d\n",
				point.x, point.y, ind);

			// Check if the datapoint <ind> has still been selected
			CQsdRecord* pq	= &pDoc->m_QsdRecords[ind];
			BOOL bSel		= pq->m_bSel;

			// When using extended selection, invert selection of clicked object
			if ( bExtSel )
			{
				CClientDC dc(this);
				DrawSelection(&dc, &pq->m_rectView);
				pq->m_bSel	= (bSel == FALSE);

				// Invert selection in list control of <CQsdListView>
				pDoc->UpdateAllViews(this, HINT_SELECTOBJECT, (CObject*)&ind);
			}

			// When using single selection:
			// If object has not yet been selected, reset all previous selections
			// and select only the clicked object.
			// If the object has just been selected, do not change previous selections.
			else
			{
				if ( !bSel )
				{
					CClientDC dc(this);
					DrawSelections(&dc);
					for (ip=0; ip < nPoints; ip++)
						pDoc->m_QsdRecords[ip].m_bSel = FALSE;
					pq->m_bSel = TRUE;
					DrawSelection(&dc, &pq->m_rectView);
					pDoc->UpdateAllViews(this, HINT_SELECTALL);
				}
			}
		}

		// If there is NO object at position <point>, let the user open a rectangle
		// and select all objects inside that rectangle.
		else
		{
			// In single selection mode remove all previous selections
			if ( !bExtSel )
			{
				CClientDC dc(this);
				DrawSelections(&dc);
				for (ip=0; ip < nPoints; ip++)
					pDoc->m_QsdRecords[ip].m_bSel = FALSE;
			};

			CRectTracker Tracker(CRect(point, CSize(0,0)), CRectTracker::dottedLine);
			Tracker.m_sizeMin = CSize(0,0);
			if ( Tracker.TrackRubberBand(this, point, TRUE) )
			{
				CRect rectSel;
				Tracker.GetTrueRect(&rectSel);

				TRACE("CQsdGraphView::OnLButtonDown() - CTrackerRect result: (%d,%d,%d,%d)\n",
					rectSel.left, rectSel.top, rectSel.right, rectSel.bottom);

				// In single selection mode, select all datapoints that are
				// completely inside the rectangle <rectSel>.
				// In extended selection mode, invert the selections of the datapoints
				// that are completely inside the rectangle <rectSel>.
				CClientDC dc(this);
				for (ip=0; ip < nPoints; ip++)
				{
					CQsdRecord* pq = &pDoc->m_QsdRecords[ip];
					if ( RectInRect(&pq->m_rectView, &rectSel, TRUE) )
					{
						if ( bExtSel )
							pq->m_bSel = (pq->m_bSel == FALSE);
						else
							pq->m_bSel = TRUE;
						DrawSelection(&dc, &pq->m_rectView);
					}
				}
			};

			pDoc->UpdateAllViews(this, HINT_SELECTALL);
		}
	};
	
	CView::OnLButtonDown(nFlags, point);
}

void CQsdGraphView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// If the user is tracking (rotating, shifting, or zooming) the Discovery Space,
	// end this tracking and release mouse capture. The mouse cursor will automatically
	// be restored by a WM_SETCURSOR message.
	if ( m_bTracking )
	{
		::ReleaseCapture();
		m_bTracking = FALSE;
	}

	// When in slab definition mode, register the slab rectangle and calculate
	// the range along the vertical axis, that means the depth of the slab.
	else if ( m_bSlabMode )
	{
		::ReleaseCapture();
		
		CClientDC dc(this);
		DrawSlabRect(&dc);		// clear the slab rectangle

		float afRange[2];
		GetSlabRange(afRange);

		CDiscoveryDoc *pDoc		  = GetDocument();
		pDoc->m_afQsdSlabRange[0] = afRange[0];
		pDoc->m_afQsdSlabRange[1] = afRange[1];

		TRACE("CQsdGraphView::OnLButtonUp() - Slab range is (%5.4f..%5.4f)\n",
			afRange[0], afRange[1]);

		// Clear the last settings for the current slab range
		MonitorSlabRange(&dc, afRange);

		// After the slab definition is complete, we bring the cube into an
		// orientation where the slab axis points towards the viewer. A special
		// symbol in the upper left corner will warn us that only the datapoints
		// within the slab will be displayed from now on. To cancel the clipping,
		// the user must double-click on that symbol.
		m_bSlabMode			 = FALSE;
		m_rectSlab.SetRectEmpty();	// to avoid confusion in OnMouseMove()
		m_bSlabActive		 = TRUE;

		pDoc->m_iQsdSlabAxis = abs(m_iVertAxis);
		if (pDoc->m_iQsdSlabAxis == 3)
			OnViewQsdProjXY();
		else if (pDoc->m_iQsdSlabAxis == 2)
			OnViewQsdProjXZ();
		else
			OnViewQsdProjXY();

		// List of datapoints must be refreshed, since the datapoints outside the
		// current slab do not appear in the table.
		pDoc->UpdateAllViews(this);
	};
	
	CView::OnLButtonUp(nFlags, point);
}

void CQsdGraphView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// If the user is tracking the Discovery Space, use the difference between
	// current and latest mouse position to calculate the amount of rotation,
	// shift, or changing in enlargement factor, rsp., then redraw the cube.
	if ( m_bTracking )
	{
		CDiscoveryDoc* pDoc = GetDocument();
		CSize sizeOffset	= point - m_ptLastMouse;
		m_ptLastMouse		= point;
		
		float afIncr[3];
		if (m_iTracking == 1)
		{
			if (sizeOffset.cx == 0 && sizeOffset.cy == 0)
				return;
			// Vertical mouse increments rotate along x-axis, horizontal ones along y-axis
			afIncr[0] = sizeOffset.cy * -0.25f;	// TO BE CHECKED (factor)
			afIncr[1] = sizeOffset.cx * -0.25f;
			afIncr[2] = 0.0f;
			Rotate(3, afIncr);
		}
		else if (m_iTracking == 2)
		{
			if (sizeOffset.cy == 0)
				return;
			// Vertical mouse increments rotate along z-axis
			afIncr[0] = afIncr[1] = 0.0f;
			afIncr[2] = sizeOffset.cy * 0.25f;	// TO BE CHECKED (factor)
			Rotate(4, afIncr);
		}
		else if (m_iTracking == 3)
		{
			if (sizeOffset.cx == 0 && sizeOffset.cy == 0)
				return;
			m_ptShift += sizeOffset;
		}
		else if (m_iTracking == 4)
		{
			if (sizeOffset.cy == 0)
				return;
			// Moving mouse to the top increases the enlargement factor
			m_fScale += (float)sizeOffset.cy * -1.0f;	// TO BE CHECKED (factor)
		};

		RedrawWindow();
	}

	// When in slab definition mode, register vertical mouse movements and
	// update the bottom of the slab rectangle, which is drawn in XOR mode.
	else if ( m_bSlabMode )
	{
		if ( m_rectSlab.IsRectNull() )
			return;

		CRect rectNew(m_rectSlab);
		rectNew.bottom = point.y + 1;
		if (rectNew.bottom == m_rectSlab.bottom)
			return;

		float afRange[2];
		CClientDC dc(this);
		DrawSlabRect(&dc);		// clear the old rectangle
		GetSlabRange(afRange);
		MonitorSlabRange(&dc, afRange);	// clear the old slab range printed at top of the client area

		m_rectSlab = rectNew;
		DrawSlabRect(&dc);		// draw rectangle at new position
		GetSlabRange(afRange);
		MonitorSlabRange(&dc, afRange);	// print the new slab range
	}
	
	//CView::OnMouseMove(nFlags, point);
}

void CQsdGraphView::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// Undefine the slab, if the user double-clicks into the slab warning symbol
	if ( m_bSlabActive && point.x > 0 && point.x < 40 && point.y > 0 && point.y < 40)
	{
		OnViewQsdNoSlab();
	}
	
	//CView::OnLButtonDblClk(nFlags, point);
}

void CQsdGraphView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// See comments in CReportCtrl::OnNotify()
	GetParentFrame()->ActivateFrame();

	// When context menu has been requested by the keyboard, <point> contains
	// the screen coordinates (-1,-1). In that case we put the context menu
	// into the upper left corner of the view's client area.
	if (point.x < 0 && point.y < 0)
	{
		point.x = point.y = 0;
		ClientToScreen(&point);
	};

	CMenu Menu, *pSubMenu;
	Menu.LoadMenu(IDR_QSD_GRAPH);
	pSubMenu  = Menu.GetSubMenu(0);
	
	UINT uCmd = GetPopupMenuCmd(pSubMenu, point.x, point.y, AfxGetMainWnd());
	if (uCmd != 0)
		AfxGetMainWnd()->PostMessage(WM_COMMAND, uCmd);
}

// Select all datapoints
void CQsdGraphView::OnEditSelectAll() 
{
	CClientDC dc(this);
	CDiscoveryDoc* pDoc = GetDocument();
	DrawSelections(&dc);
	for (register int iq=0, nq=pDoc->m_QsdRecords.GetSize(); iq < nq; iq++)
		pDoc->m_QsdRecords[iq].m_bSel = TRUE;
	DrawSelections(&dc);
	pDoc->UpdateAllViews(this, HINT_SELECTALL);
}

// Select all datapoints that have not been marked and vice versa
void CQsdGraphView::OnEditSelectInv() 
{
	CClientDC dc(this);
	CDiscoveryDoc* pDoc = GetDocument();
	DrawSelections(&dc);
	for (register int iq=0, nq=pDoc->m_QsdRecords.GetSize(); iq < nq; iq++)
	{
		CQsdRecord* pq = &pDoc->m_QsdRecords[iq];
		pq->m_bSel = (pq->m_bSel == FALSE);
	};
	DrawSelections(&dc);
	pDoc->UpdateAllViews(this, HINT_SELECTALL);
}

// Reset any shift increments and recalculate the scaling factor, so that cube
// fits into the drawing rectangle.
void CQsdGraphView::OnViewQsdAdjust() 
{
	CRect rectDraw;
	GetClientRect(&rectDraw);
	float fXScale = (float)rectDraw.Width();
	float fYScale = (float)rectDraw.Height();
	float fScale  = ((fXScale < fYScale) ? fXScale : fYScale);
	m_fScale	  = fScale * 0.75f;	// leave some margin for the cube
	m_ptShift.x	  = m_ptShift.y = 0;

	RedrawWindow();
}

// View towards the XY plane, that means, let the z-axis point toward the viewer.
void CQsdGraphView::OnViewQsdProjXY() 
{
	ResetMatrix(m_omat);
	RedrawWindow();
}

void CQsdGraphView::OnViewQsdProjXZ() 
{
	float afRot[3];
	afRot[0]=90.0f;	afRot[1]=afRot[2]=0.0f;
	ResetMatrix(m_omat);
	Rotate(1, afRot);
	RedrawWindow();
}

void CQsdGraphView::OnViewQsdProjYZ() 
{
	float afRot[3];
	afRot[1]=90.0f;	afRot[0]=afRot[2]=0.0f;
	ResetMatrix(m_omat);
	Rotate(2, afRot);
	RedrawWindow();
}

// 2D projections other than XY only when Discovery Space is really 3-dimensional
void CQsdGraphView::OnUpdateViewQsdProjXZ(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetQsdDimensionCount() == 3);
}

void CQsdGraphView::OnUpdateViewQsdProjYZ(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetQsdDimensionCount() == 3);
}

// Set the 2D projection which is closest to the current orientation
void CQsdGraphView::OnViewQsdProjClosest() 
{
	// Examine which of the six cube normals has the greatest z-value. This will
	// be the plane to be projected into the screen plane.
	float afMinMax[6], afCorners[8][3], afNormals[6][3], afAxesLabels[4][3];
	UpdateViewCoords(afCorners, afNormals, afAxesLabels, afMinMax);
	float fzmax = -999.0f;
	int izmax	= -1;
	for (register int inormal=0; inormal < 6; inormal++)
	{
		if (afNormals[inormal][2] > fzmax)
		{
			izmax = inormal;
			fzmax = afNormals[inormal][2];
		}
	};

	// Note: Since the z-axis of the view-coordinate system points towards the viewer,
	// we are looking on the corresponding back plane of a plane in 2D projection.
	// So we check for normals 3, 4, and 5 instead of 0, 1, 2.
	float afRot[3];
	if (izmax == 3)			// xy-plane
	{
		OnViewQsdProjXY();
		return;
	}
	else if (izmax == 4)	// xz-plane
	{
		OnViewQsdProjXZ();
		return;
	}
	else if (izmax == 5)	// yz-plane
	{
		OnViewQsdProjYZ();
		return;
	};

	ResetMatrix(m_omat);
	if (izmax == 0)			// xy-plane, but other side
	{
		afRot[0] = afRot[2] = 0.0f;
		afRot[1] = 180.0f;
		Rotate(2, afRot);
	}
	else if (izmax == 1)
	{
		afRot[0] = -90.0f;
		afRot[1] = afRot[2] = 0.0f;
		Rotate(1, afRot);
	}
	else if (izmax == 2)
	{
		afRot[1] = -90.0f;
		afRot[0] = afRot[2] = 0.0f;
		Rotate(2, afRot);
	};

	RedrawWindow();

/*	MAT44 m1;
	float f, fmax;
	register int i, j, imax;
	memset(m1, 0, sizeof(m1));
	m1[3][3] = 1.0f;
	for (j=0; j < 3; j++)
	{
		for (i=0, fmax=-1.0f, imax=-1; i < 3; i++)
		{
			f = fabs(m_omat[j][i]);
			if (f > fmax)
			{
				fmax = f;
				imax = i;
			}
		};
		m1[j][imax] = ((m_omat[j][imax] > 0.0f) ? 1.0f : -1.0f);
	}
	memcpy(m_omat, m1, sizeof(m_omat));
	RedrawWindow();	*/
}

void CQsdGraphView::OnUpdateViewQsdProjClosest(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetQsdDimensionCount() == 3);
}

// Define a slab along the vertical axis
void CQsdGraphView::OnViewQsdSlab() 
{
	if ( m_bSlabMode )
	{
		m_bSlabMode = FALSE;
		m_rectSlab.SetRectEmpty();	// to avoid confusion in OnMouseMove()
		return;
	};

	// Tracking mode must be disabled before defining the slab.
	if (m_iTracking > 0)
		m_iTracking = 0;

	// To define a slab, the Discovery Space must be in a 2D projection. The user
	// will be requested to bring the cube into the closest 2D projection.
	BOOL b2D = Is2DProjection();
	if ( !b2D )
	{
		if (AfxMessageBox(IDS_NEED_2D_PROJ_FOR_SLAB, MB_YESNO) == IDNO)
			return;
		OnViewQsdProjClosest();
	};
	ASSERT( Is2DProjection() );

	// Set the slab definition mode. The definition of the slab will start when
	// the user presses down the left mouse button.
	m_bSlabMode = TRUE;
}

void CQsdGraphView::OnUpdateViewQsdSlab(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetQsdDimensionCount() == 3);
	pCmdUI->SetCheck(m_bSlabMode);
}

// Undefine the slab and prevent datapoints from being clipped
void CQsdGraphView::OnViewQsdNoSlab() 
{
	if ( m_bSlabActive )
	{
		m_bSlabActive = FALSE;
		CDiscoveryDoc* pDoc = GetDocument();
		pDoc->m_afQsdSlabRange[0] = pDoc->m_afQsdSlabRange[1] = FErr;
		pDoc->m_iQsdSlabAxis = 0;
		RedrawWindow();
		pDoc->UpdateAllViews(this);	// list must also be refreshed
	}
}

void CQsdGraphView::OnUpdateViewQsdNoSlab(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_bSlabActive && GetDocument()->GetQsdDimensionCount() == 3);
}

// Command handlers for switching between dot and ball representation of datapoints
void CQsdGraphView::OnViewQsdDots() 
{
	if ( m_bBalls )
	{
		m_bBalls = FALSE;
		AfxGetApp()->WriteProfileInt(s_szQsd, "Balls", 0);
		RedrawWindow();
	}
}

void CQsdGraphView::OnUpdateViewQsdDots(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(m_bBalls == FALSE);
}

void CQsdGraphView::OnViewQsdBalls() 
{
	if ( !m_bBalls )
	{
		m_bBalls = TRUE;
		AfxGetApp()->WriteProfileInt(s_szQsd, "Balls", 1);
		RedrawWindow();
	}
}

void CQsdGraphView::OnUpdateViewQsdBalls(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(m_bBalls == TRUE);
}

// Enable or disable grid on back wall when in 2D projection mode
void CQsdGraphView::OnViewQsdGrid() 
{
	m_bGrid = (m_bGrid == FALSE);
	AfxGetApp()->WriteProfileInt(s_szQsd, "Grid", (int)m_bGrid);
	RedrawWindow();
}

void CQsdGraphView::OnUpdateViewQsdGrid(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bGrid);
}

// Command handlers to enable (but not start) or disable tracking modes
void CQsdGraphView::OnViewRotXY() 
{
	// <m_iTracking> == 1 indicates that WM_SETCURSOR shall set a special cursor
	// (arrow plus a symbol indicating the tracking mode), and that WM_LBUTTONDOWN
	// will not select a datapoint but start continuous rotation along x- and y-axis
	// while user is moving the mouse with left button down.
	m_iTracking = 1;
}

void CQsdGraphView::OnUpdateViewRotXY(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetDocument()->GetQsdDimensionCount() == 3);
	pCmdUI->SetRadio(m_iTracking == 1);
}

void CQsdGraphView::OnViewRotZ() 
{
	m_iTracking = 2;
}

void CQsdGraphView::OnUpdateViewRotZ(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(m_iTracking == 2);
}

void CQsdGraphView::OnViewShift() 
{
	m_iTracking = 3;
}

void CQsdGraphView::OnUpdateViewShift(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(m_iTracking == 3);
}

void CQsdGraphView::OnViewEnlarge() 
{
	m_iTracking = 4;
}

void CQsdGraphView::OnUpdateViewEnlarge(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(m_iTracking == 4);
}

void CQsdGraphView::OnViewNoTrack() 
{
	m_iTracking = 0;
}

void CQsdGraphView::OnUpdateViewNoTrack(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(m_iTracking == 0);
}


/////////////////////////////////////////////////////////////////////////////
// CQsdListView

IMPLEMENT_DYNCREATE(CQsdListView, CListView)

CQsdListView::CQsdListView()
{
	m_iSort = 0;	// 0=no sorting, n=sort for column #<n-1> ascending, -n=descending
	m_piSortInd = NULL;

	m_bProcessedContextMenu = FALSE;
	m_bNoItemChangedMsg		= FALSE;
}

CQsdListView::~CQsdListView()
{
	if (m_piSortInd != NULL)
		delete m_piSortInd;
}


BEGIN_MESSAGE_MAP(CQsdListView, CListView)
	//{{AFX_MSG_MAP(CQsdListView)
	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemchanged)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_COMMAND(ID_EDIT_SELECT_INV, OnEditSelectInv)
	ON_COMMAND(ID_FILE_SAVE_DATA, OnFileSaveData)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQsdListView drawing

void CQsdListView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CQsdListView diagnostics

#ifdef _DEBUG
void CQsdListView::AssertValid() const
{
	CListView::AssertValid();
}

void CQsdListView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}
#endif //_DEBUG

CDiscoveryDoc* CQsdListView::GetDocument()
{
	ASSERT_KINDOF( CDiscoveryDoc, m_pDocument );
	return (CDiscoveryDoc*)m_pDocument;
};

/////////////////////////////////////////////////////////////////////////////
// CQsdListView message handlers

int CQsdListView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	// This list uses multiple selection (list control default)
	lpCreateStruct->style |= LVS_REPORT;
	lpCreateStruct->style |= LVS_SHOWSELALWAYS;

	if (CListView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	return 0;
}

void CQsdListView::OnInitialUpdate() 
{
	// Create the image list. (Compare CProfileListView::OnInitialUpdate().)
	// Here we create small bitmaps (12 x 12 pixels) rather than load icons
	// from the resource.
	m_imageList.Create(12, 12, 0, MAX_COLOR_QSD, 8);

	CClientDC dc(this);
	CDC dcBmp;
	CBitmap Bmp, *pOldBmp;
	dcBmp.CreateCompatibleDC(&dc);
	Bmp.CreateCompatibleBitmap(&dc, 12, 12);
	pOldBmp			  = dcBmp.SelectObject(&Bmp);
	CPen* pOldPen	  = (CPen*)dcBmp.SelectStockObject(BLACK_PEN);
	CBrush* pOldBrush = (CBrush*)dcBmp.SelectStockObject(WHITE_BRUSH);

	for (register int ii=0; ii < MAX_COLOR_QSD; ii++)
	{
		CBrush brush(CQsdRecord::GetColor(ii));
		dcBmp.SelectObject(&Bmp);
		dcBmp.SelectObject(&brush);
		dcBmp.PatBlt(0, 0, 12, 12, WHITENESS);
		dcBmp.Ellipse(0, 0, 12, 12);
		dcBmp.SelectObject(pOldBrush);
		dcBmp.SelectObject(pOldBmp);
		m_imageList.Add(&Bmp, (CBitmap*)NULL);
	};

	CListCtrl& list	= GetListCtrl();
	DWORD dwExStyle	= list.SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE);
	list.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwExStyle | LVS_EX_FULLROWSELECT);

	list.SetImageList(&m_imageList, LVSIL_SMALL);

	CListView::OnInitialUpdate();
}

// Callback function used by qsort() from within OnUpdate(), which uses the
// static variables <s_uSort> and <s_bDesc> (see above) and the following pointer
// to access the compound properties and coordinate triplets, etc.:
static CQsdRecordArray *s_pRecords;	// points to the array with the QSD records

static int _SortRecords (const void* elem1, const void* elem2)
{
	int i1			= *(int*)elem1;
	int i2			= *(int*)elem2;
	CQsdRecord* pp1	= &(*s_pRecords)[i1];
	CQsdRecord* pp2 = &(*s_pRecords)[i2];
	int isort		= 0;
	// When we sort for x, we also sort for y and z in 2nd and 3rd preference.
	// For y and z this works similar.
	float x1		= pp1->m_afXyz[0];
	float x2		= pp2->m_afXyz[0];
	float y1		= pp1->m_afXyz[1];
	float y2		= pp2->m_afXyz[1];
	float z1		= pp1->m_afXyz[2];
	float z2		= pp2->m_afXyz[2];
	if (s_uSort == 1)		// sort for the element symbols alphabetically...
	{
		for (register int ie=0; ie < MAX_ELEM_QSD && isort == 0; ie++)
		{
			isort	= (	(pp1->m_abElemNo[ie] > pp2->m_abElemNo[ie]) ?  1 :
						(pp1->m_abElemNo[ie] < pp2->m_abElemNo[ie]) ? -1 : 0);
		};					// ...and for stoichiometric indexes if not unique
		for (ie=0; ie < MAX_ELEM_QSD && isort == 0; ie++)
		{
			isort	= (	(pp1->m_afStoich[ie] > pp2->m_afStoich[ie]) ?  1 :
						(pp1->m_afStoich[ie] < pp2->m_afStoich[ie]) ? -1 : 0);
		};
	}
	else if (s_uSort == 2)	// sort for system properties
	{
		isort		= (	(pp1->m_fProp > pp2->m_fProp) ?  1 :
						(pp1->m_fProp < pp2->m_fProp) ? -1 : 0);
	}
	else if (s_uSort == 3)	// sort for x-coordinates
	{
		isort		= (	(x1 > x2) ? 1 : (x1 < x2) ? -1 :
						(y1 > y2) ? 1 : (y1 < y2) ? -1 :
						(z1 > z2) ? 1 : (z1 < z2) ? -1 : 0);
	}
	else if (s_uSort == 4)	// sort for y-coordinates
	{
		isort		= (	(y1 > y2) ? 1 : (y1 < y2) ? -1 :
						(z1 > z2) ? 1 : (z1 < z2) ? -1 :
						(x1 > x2) ? 1 : (x1 < x2) ? -1 : 0);
	}
	else if (s_uSort == 5)	// sort for z-coordinates
	{
		isort		= (	(z1 > z2) ? 1 : (z1 < z2) ? -1 :
						(x1 > x2) ? 1 : (x1 < x2) ? -1 :
						(y1 > y2) ? 1 : (y1 < y2) ? -1 : 0);
	}
	else if (s_uSort == 6)	// sort for probability
	{
		isort		= (	(pp1->m_fProb > pp2->m_fProb) ?  1 :
						(pp1->m_fProb < pp2->m_fProb) ? -1 : 0);
	}
	if ( s_bDesc )
		isort	   *= -1;
	return isort;
};

// lHint == 0: Delete old columns, sort, insert columns, insert items.
// lHint == HINT_SORTLIST: Delete "old" items, then sort and re-insert items.
// lHint == HINT_SELECTOBJECT: The selection state of the QSD record number
//                             <*(int*)pHint> has changed.
// lHint == HINT_SELECTALL: The selection states of all items need updating.
// Compare CProfileListView::OnUpdate().

void CQsdListView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	CDiscoveryDoc* pDoc	= GetDocument();
	CListCtrl& list		= GetListCtrl();

	// HINT_HISTOCHANGE takes no influence in list
	if (lHint == HINT_HISTOCHANGE)
		return;

	// Change of selection of one or more items?
	if (lHint == HINT_SELECTOBJECT)
	{
		TRACE("CQsdListView::OnUpdate() called with lHint == HINT_SELECTOBJECT\n");

		// <ind> is the index in the array CDiscoveryDoc::m_QsdRecords[];
		// <iItem> is the index of the item in the list control.
		LV_FINDINFO lvfi;
		int ind			= *(int*)pHint;
		lvfi.flags		= LVFI_PARAM;
		lvfi.psz		= NULL;
		lvfi.lParam		= ind;
		int iItem		= list.FindItem(&lvfi);
		CQsdRecord* pq	= &pDoc->m_QsdRecords[ind];
		m_bNoItemChangedMsg	= TRUE;		// see CQsdListView::OnItemChanged()
		list.SetItemState(iItem, pq->m_bSel ? LVIS_SELECTED : 0, LVIS_SELECTED);
		list.EnsureVisible(iItem, FALSE);
		m_bNoItemChangedMsg = FALSE;
		return;
	};

	if (lHint == HINT_SELECTALL)
	{
		TRACE("CQsdListView::OnUpdate() called with lHint == HINT_SELECTALL\n");

		CWaitCursor curWait;
		BOOL bFirst = TRUE;
		list.SetRedraw(FALSE);
		m_bNoItemChangedMsg	= TRUE;
		for (register int i=0, imax=list.GetItemCount(); i < imax; i++)
		{
			// Invert state of selection only if necessary
			UINT uState	= list.GetItemState(i, LVIS_SELECTED);
			int iNo		= (int)list.GetItemData(i);	// index of the QSD record
			BOOL bSel	= pDoc->m_QsdRecords[iNo].m_bSel;
			if (uState == LVIS_SELECTED && !bSel )
			{
				list.SetItemState(i, 0, LVIS_SELECTED);
			}
			else if (uState == 0 && bSel )
			{
				list.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
				if ( bFirst )
				{
					list.EnsureVisible(i, FALSE);
					bFirst = FALSE;
				}
			}
		};

		m_bNoItemChangedMsg = FALSE;
		list.SetRedraw(TRUE);
		return;
	};

	CWaitCursor curWait;
	
	list.DeleteAllItems();

	if (lHint == 0)		// delete all previous columns when (re-)building (new) list
	{
		while ( list.DeleteColumn(0) );
	};

	// Get the number of QSD records (nq), that means the number of rows to go into
	// the list. This number may be less than what <m_QsdRecords.GetSize()> returns,
	// if a slab has been defined.
	int ntotal = pDoc->m_QsdRecords.GetSize(), itotal;
	int nq	   = 0;
	register int iq, ir/*, ic*/;

	// The items in the list can be sorted by changing the rank order of the
	// indexes of the profiles. <m_iSort>==0 means no sorting, +1 means sorting
	// in ascending order for column #0, -1 for descending order, +2 and -2
	// for column #1, etc.
	if (m_piSortInd)
		delete m_piSortInd;
	if (ntotal > 0)
	{
		m_piSortInd = new int[ntotal];
		int iaxis	= pDoc->m_iQsdSlabAxis;	// 0=no slab defined, 1..3 slab axis=x..z
		for (iq=0, itotal=0; itotal < ntotal; itotal++)
		{
			if (iaxis >=1 && iaxis <= 3)
			{
				CQsdRecord* pq = &pDoc->m_QsdRecords[itotal];
				if (pq->m_afXyz[iaxis-1] >= pDoc->m_afQsdSlabRange[0] &&
					pq->m_afXyz[iaxis-1] <= pDoc->m_afQsdSlabRange[1])
					m_piSortInd[iq++] = itotal;
			}
			else
				m_piSortInd[iq++] = itotal;
		};

		nq = iq;

		if (m_iSort != 0)
		{
			s_uSort		= abs(m_iSort);
			s_bDesc		= (m_iSort < 0);
			s_pRecords	= &pDoc->m_QsdRecords;
			qsort(m_piSortInd, nq, sizeof(m_piSortInd[0]), _SortRecords);
		}
	}
	else
		m_piSortInd = NULL;

	// Columns need not (must not) be (re-)inserted after sort order has changed
	if (lHint == 0)
	{
		// Set the image list to identify the colors in the cube
		//list.SetImageList(&m_imageList, LVSIL_SMALL);

		// The first column describes the chemical system, that means a list
		// of element symbols
		int iwid = list.GetStringWidth("XX;XX;XX;XX") + 20;	// add 20 for icon
		list.InsertColumn(0, "System", LVCFMT_LEFT, iwid, 0);

		// The second column contains the property value
		iwid = list.GetStringWidth("1234567890");
		list.InsertColumn(1, "Property", LVCFMT_LEFT, iwid, 1);

		// The third, fourth, and fifth column contain the x-, y-, and z-coordinate
		iwid = list.GetStringWidth("12345678901234");
		list.InsertColumn(2, "x", LVCFMT_LEFT, iwid, 2);
		list.InsertColumn(3, "y", LVCFMT_LEFT, iwid, 3);
		list.InsertColumn(4, "z", LVCFMT_LEFT, iwid, 4);

		// The sixth column contains the probability
		iwid = list.GetStringWidth("123456789012");
		list.InsertColumn(5, "Probability-%", LVCFMT_LEFT, iwid, 5);
	};

	list.SetItemCount(nq);
	list.SetRedraw(FALSE);

	// We do not add all items' and subitems' strings here, since that may last
	// some seconds for some thousand items. Instead we make use of text callback.
	// The text is then requested whenever a portion of the list must be drawn.
	// See CQsdListView::OnGetDispinfo() below.
	LV_ITEM lvi;
	lvi.mask		= LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_IMAGE;
	lvi.stateMask	= 0;
	lvi.state		= 0;
	lvi.pszText		= LPSTR_TEXTCALLBACK;
	//lvi.iImage	= 0;

	// Add the items and subitems into the list in sorted order
	//CString strItem;
	for (ir=0; ir < nq; ir++)
	{
		// <iq> is the index to CDiscoveryDoc::m_QsdRecords[],
		// <ir> the index of the row where the record will go.
		iq = m_piSortInd[ir];
		CQsdRecord* pRecord = &pDoc->m_QsdRecords[iq];

		lvi.iItem	= ir;
		lvi.iSubItem= 0;
		lvi.lParam	= (LPARAM)m_piSortInd[ir];
		lvi.iImage	= pRecord->GetPropertyIndex();
		list.InsertItem(&lvi);

//		pRecord->FormatComp(strItem);

		// Get the image index associated with that compound, and insert the item.
		// TO BE CHECKED
//		int iImage = 0;
//		list.InsertItem(ir, strItem /*, iImage*/);

		// Format the property into the second column
//		strItem.Format("%1.0f", pRecord->m_fProp);		// TO BE CHECKED
//		list.SetItemText(ir, 1, strItem);

		// Format the x-, y-, and z-coordinates for the 3rd, 4th, and 5th column
//		for (ic=0; ic < 3; ic++)
//		{
//			strItem.Format("%5.4f", pRecord->m_afXyz[ic]);
//			list.SetItemText(ir, ic+2, strItem);
//		}
	};

	list.SetRedraw(TRUE);
}

// Callback function that requests the text for an item or subitem of the list
void CQsdListView::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo	= (LV_DISPINFO*)pNMHDR;
	int iItem				= pDispInfo->item.iItem;		// row number
	int iSubItem			= pDispInfo->item.iSubItem;		// column
	LPARAM lParam			= pDispInfo->item.lParam;		// index of the QSD record

	CString strItem;
	CDiscoveryDoc* pDoc		= GetDocument();
	CQsdRecord* pRecord		= &pDoc->m_QsdRecords[lParam];

	if (iSubItem == 0)
	{
		// Format the composition for the first column
		pRecord->FormatComp(strItem);
	}
	else if (iSubItem == 1)
	{
		// Format the property into the second column
		if (pRecord->m_fProp >= 0.0f)
			strItem.Format("%1.0f", pRecord->m_fProp);		// TO BE CHECKED
		else
			strItem = "";
	}
	else if (iSubItem >= 2 && iSubItem <= 4)
	{
		// Format the x-, y-, or z-coordinate for the 3rd, 4th, or 5th column, rsp.
		// or leave it blank if they are undefined.
		if ( !pRecord->m_bNoCoords )
			strItem.Format("%5.4f", pRecord->m_afXyz[iSubItem-2]);
		else
			strItem = "";
	}
	else if (iSubItem == 5)
	{
		// Format the probability in percent into the sixth column
		if (pRecord->m_fProb >= 0.0f)
			strItem.Format("%1.0f", pRecord->m_fProb);		// TO BE CHECKED
		else if ( !pRecord->m_bPredict )
			strItem = "E";
		else
			strItem = "";
	}
	else
		strItem = "";

	strncpy(pDispInfo->item.pszText, strItem, pDispInfo->item.cchTextMax-1);
	
	*pResult = 0;
}

void CQsdListView::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Register click on header item to sort for this column or to revert the sort order.
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int iCol = pNMListView->iSubItem;
	if (abs(m_iSort)-1 == iCol)	// m_iSort==1 for column #0, 2 for column #1, etc.
		m_iSort *= -1;
	else
		m_iSort = iCol + 1;

	OnUpdate(this, HINT_SORTLIST, NULL);
	
	*pResult = 0;
}

void CQsdListView::OnItemchanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// This notification handler will be called e.g. when the user clicks an item.
	// But it will also be called after a call to CListCtrl::SetItemState() (usually
	// after the user has clicked an object in the CQsdGraphView drawing).
	// In the second case, the flag <m_bNoItemChangedMsg> is set by
	// CQsdListView::OnUpdate()and the following code will only be executed.
	if ( !m_bNoItemChangedMsg )
	{
		// Check only change of selection
		if (pNMListView->uChanged == LVIF_STATE &&
			(pNMListView->uNewState & LVIS_SELECTED) != (pNMListView->uOldState & LVIS_SELECTED))
		{
			CDiscoveryDoc* pDoc = GetDocument();
			int ind				= (int)GetListCtrl().GetItemData(pNMListView->iItem);
			BOOL bSel			= pDoc->m_QsdRecords[ind].m_bSel;
			pDoc->m_QsdRecords[ind].m_bSel = (bSel == FALSE);
			pDoc->UpdateAllViews(this, HINT_DRAWSELECTION, (CObject*)&ind);
		}
	};
	
	*pResult = 0;
}

void CQsdListView::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CQsdListView::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// Compare CProfileListView::OnRclick()
	GetParentFrame()->ActivateFrame();

	CMenu Menu, *pSubMenu;
	Menu.LoadMenu(IDR_QSD_LIST);
	pSubMenu = Menu.GetSubMenu(0);
	m_bProcessedContextMenu = TRUE;

	CPoint ptScreen(GetMessagePos());
	UINT uCmd = GetPopupMenuCmd(pSubMenu, ptScreen.x, ptScreen.y, AfxGetMainWnd());
	if (uCmd != 0)
		AfxGetMainWnd()->PostMessage(WM_COMMAND, uCmd);
	
	*pResult = 0;
}

void CQsdListView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// Compare CProfileListView::OnContextMenu()
	if ( !m_bProcessedContextMenu )
	{
		GetParentFrame()->ActivateFrame();

		if (point.x < 0 && point.y < 0)
		{
			point.x = point.y = 0;
			ClientToScreen(&point);
		};

		CMenu Menu, *pSubMenu;
		Menu.LoadMenu(IDR_QSD_LIST);
		pSubMenu  = Menu.GetSubMenu(0);
	
		UINT uCmd = GetPopupMenuCmd(pSubMenu, point.x, point.y, AfxGetMainWnd());
		if (uCmd != 0)
			AfxGetMainWnd()->PostMessage(WM_COMMAND, uCmd);
	};

	m_bProcessedContextMenu = FALSE;
}

// Copy the selected lines to the clipboard.
// Subitems will be separated by tabstops, lines will be separated by CRLF.
void CQsdListView::OnEditCopy() 
{
	CWaitCursor curWait;	// may become lengthy with some thousand compounds
	CListCtrl& list = GetListCtrl();
	CHeaderCtrl* pHeader = (CHeaderCtrl*)list.GetDlgItem(0);
	CStringList Strings;
	int iSize	 = 0;
	int nColumns = pHeader->GetItemCount();
	//int nColumns = 5;		// fixed number of columns
	for (register int ind=-1; (ind=list.GetNextItem(ind, LVNI_SELECTED)) != -1; )
	{
		CString strLine;
		for (register int icol=0; icol < nColumns; icol++)
		{
			if (icol > 0)
				strLine += "\t";
			strLine += list.GetItemText(ind, icol);
		};
		strLine += "\r\n";
		iSize += strLine.GetLength();
		Strings.AddTail(strLine);
	};
	
	HANDLE hBuffer	= GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, iSize+2);	// plus '\0'
	LPSTR pBuffer	= (LPSTR)GlobalLock(hBuffer);
	LPSTR cp		= pBuffer;
	POSITION pos1, pos2;
	for (pos1=Strings.GetHeadPosition(); (pos2=pos1) != NULL; )
	{
		strcpy(cp, Strings.GetNext(pos1));
		cp += Strings.GetAt(pos2).GetLength();
	};
	*cp = '\0';
	GlobalUnlock(hBuffer);

	if ( list.OpenClipboard() )
	{
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hBuffer);
		CloseClipboard();
	}
}

void CQsdListView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetListCtrl().GetSelectedCount() > 0);
}

// Select all items in the list control
void CQsdListView::OnEditSelectAll() 
{
	CDiscoveryDoc* pDoc = GetDocument();
	CListCtrl& list		= GetListCtrl();
	for (register int i=0, imax=list.GetItemCount(); i < imax; i++)
		list.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
}

// Clear the selections of marked items and select all items that are not marked
void CQsdListView::OnEditSelectInv() 
{
	CDiscoveryDoc* pDoc	= GetDocument();
	CListCtrl& list		= GetListCtrl();
	for (register int i=0, imax=list.GetItemCount(); i < imax; i++)
	{
		UINT iState = list.GetItemState(i, LVIS_SELECTED);
		list.SetItemState(i, iState ? 0 : LVIS_SELECTED, LVIS_SELECTED);
	}
}

// Save the whole contents of the list as CSV file
void CQsdListView::OnFileSaveData() 
{
	CString strFilter	= "CSV files (*.CSV)|*.CSV||";
	CString strFileName = GetDocument()->GetPathName();
	SetFileExtension(strFileName, "CSV");
	CFileDialog dlg(FALSE, NULL, strFileName,
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
	if (dlg.DoModal() == IDOK)
	{
		// Append extension ".CSV" if no extension is given, but not if the
		// file name ends with a dot.
		CString strPathName = dlg.GetPathName();
		if (strPathName.Right(1) != ".")
			strPathName += ".CSV";

		CStdioFile file;
		CListCtrl& list = GetListCtrl();
		if ( file.Open(strPathName, CFile::modeCreate | CFile::modeWrite | CFile::typeText) )
		{
			CWaitCursor curWait;
			CString strLine, strItem;
			int nRows = list.GetItemCount();
			int nCols = 5;		// fixed column count

			// First format the columns' headers
			for (register int icol=0; icol < nCols; icol++)
			{
				char szTitle[256];
				LV_COLUMN lvc;
				lvc.mask		= LVCF_TEXT;
				lvc.pszText		= szTitle;
				lvc.cchTextMax	= sizeof(szTitle);
				list.GetColumn(icol, &lvc);
				if (icol > 0)
					strLine += ";";
				strLine += szTitle;
			};
			strLine += "\n";
			file.WriteString(strLine);

			// Now write the <nRows> rows of the list control
			for (register int irow=0; irow < nRows; irow++)
			{
				strLine.Empty();
				for (icol=0; icol < nCols; icol++)
				{
					if (icol > 0)
						strLine += ";";
					strItem = list.GetItemText(irow, icol);
					if ( !strItem.IsEmpty() )
					{
						// Include item text in "" if that contains a semicolon
						BOOL bSem = (strItem.Find(';') >= 0 || strItem.Find(',') >= 0);
						if ( bSem )
							strLine += "\"";
						strLine += strItem;
						if ( bSem )
							strLine += "\"";
					}
				};
				strLine += "\n";
				file.WriteString(strLine);
			}
		}
		else	// file open error
		{
			CString strMsg;
			AfxFormatString1(strMsg, IDS_CANNOT_CREATE_CSV_FILE_XXX, strPathName);
			AfxMessageBox(strMsg);
		}
	}
}


/////////////////////////////////////////////////////////////////////////////
// CQsdLegendView

IMPLEMENT_DYNCREATE(CQsdLegendView, CEditView)

CQsdLegendView::CQsdLegendView()
{
}

CQsdLegendView::~CQsdLegendView()
{
}


BEGIN_MESSAGE_MAP(CQsdLegendView, CEditView)
	//{{AFX_MSG_MAP(CQsdLegendView)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQsdLegendView drawing

void CQsdLegendView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CQsdLegendView diagnostics

#ifdef _DEBUG
void CQsdLegendView::AssertValid() const
{
	CEditView::AssertValid();
}

void CQsdLegendView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}
#endif //_DEBUG

CDiscoveryDoc* CQsdLegendView::GetDocument()
{
	ASSERT_KINDOF( CDiscoveryDoc, m_pDocument );
	return (CDiscoveryDoc*)m_pDocument;
};

/////////////////////////////////////////////////////////////////////////////
// CQsdLegendView message handlers

BOOL CQsdLegendView::PreCreateWindow(CREATESTRUCT& cs) 
{
	//cs.style |= ES_READONLY;

	return CEditView::PreCreateWindow(cs);
}

int CQsdLegendView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CEditView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	HFONT hFont = (HFONT)::GetStockObject(ANSI_FIXED_FONT);
	SendMessage(WM_SETFONT, (WPARAM)hFont); 
		
	return 0;
}

void CQsdLegendView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	SetWindowText(GetDocument()->m_QsdResultsReport);
}


/////////////////////////////////////////////////////////////////////////////
// CQsdHistogramView

IMPLEMENT_DYNCREATE(CQsdHistogramView, CView)

CQsdHistogramView::CQsdHistogramView()
{
	m_fYMin			 = 0.0f;
	m_fYMax			 = 100.0f;
	m_fBorderY		 = 0.0f;
	m_bSplitTracking = FALSE;
	m_bZoom			 = FALSE;
}

CQsdHistogramView::~CQsdHistogramView()
{
}


BEGIN_MESSAGE_MAP(CQsdHistogramView, CView)
	//{{AFX_MSG_MAP(CQsdHistogramView)
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_VIEW_QSD_HISTO_FULL, OnViewQsdHistoFull)
	ON_UPDATE_COMMAND_UI(ID_VIEW_QSD_HISTO_FULL, OnUpdateViewQsdHistoFull)
	ON_COMMAND(ID_VIEW_QSD_HISTO_ZOOM, OnViewQsdHistoZoom)
	ON_UPDATE_COMMAND_UI(ID_VIEW_QSD_HISTO_ZOOM, OnUpdateViewQsdHistoZoom)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQsdHistogramView drawing

// Avoid flickering when double buffering
BOOL CQsdHistogramView::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

void CQsdHistogramView::OnDraw(CDC* pOutDC)
{
	CDiscoveryDoc* pDoc = GetDocument();

	int iSavedDC = pOutDC->SaveDC();

	// We use double buffering for flicker-free display when the histogram columns
	// change from pale to bold or vice versa when the user shifts the horizontal
	// line up and down.
	CRect rectDraw;
	CDC dcBmp, *pDC;
	CBitmap Bmp, *pOldBmp = nullptr;
	BOOL bPrint = pOutDC->IsPrinting();
	if ( !bPrint )
	{
		GetClientRect(&rectDraw);
		dcBmp.CreateCompatibleDC(pOutDC);
		Bmp.CreateCompatibleBitmap(pOutDC, rectDraw.Width(), rectDraw.Height());
		pOldBmp = dcBmp.SelectObject(&Bmp);
		pDC		= &dcBmp;
	}
	else
	{
		// rectDraw = ...
		pDC		= pOutDC;
	};

	// Since we do not make use of WM_ERASEBKGND, we must erase the background here
	pDC->PatBlt(rectDraw.left, rectDraw.top, rectDraw.Width(), rectDraw.Height(),
				WHITENESS);

	// The height of a column is relative to the number of experimental
	// compounds (not all of <CDocument.m_QsdRecords>).
	int ncnt = pDoc->GetQsdRecordCount((pDoc->m_iRunType == 4) ? 1 : 0);
	float fn = (float)ncnt;

	pDoc->m_iProbFilter = 100;

	char szHelp[256];
	if (pDoc->m_iQsdSelProfile >= 0 && ncnt > 0)
	{
		CQsdProfile* pq = &pDoc->m_QsdProfiles[pDoc->m_iQsdSelProfile];

		// <fpx> and <fpy> are used to convert millimeters to pixels
		float fpx = (float)pDC->GetDeviceCaps(HORZRES) / (float)pDC->GetDeviceCaps(HORZSIZE);
		float fpy = (float)pDC->GetDeviceCaps(VERTRES) / (float)pDC->GetDeviceCaps(VERTSIZE);

		// Draw a rectangle and leave 10 mm margin on every side
		CRect rectHisto(rectDraw);
		rectHisto.DeflateRect(fpx*10.0f, fpy*10.0f);
		pDC->SelectStockObject(HOLLOW_BRUSH);
		pDC->SelectStockObject(BLACK_PEN);
		pDC->Rectangle(rectHisto.left, rectHisto.top, rectHisto.right+1, rectHisto.bottom+1);

		// Get the number of neighbours, that is the number of classes/bars
		register int ix;
		int nx = pq->GetValueCount();
		ASSERT( nx > 0 );

		// The height of a column is relative to the number of experimental
		// compounds (not all of <CDocument.m_QsdRecords>).
		//float fn = (float)pDoc->GetQsdRecordCount((pDoc->m_iRunType == 4) ? 1 : 0);	// see above
		ASSERT( fn > 0.0f );

		// If we are in zoom mode, the y-axis goes from the minimum to the maximum
		// percent value, otherwise (full mode) from 0 through 100 percent.
		float fMin, fMax;
		if ( m_bZoom )
		{
			fMin =  999.0f;
			fMax = -999.0f;
			for (ix=0; ix < nx; ix++)
			{
				float fyc = pq->m_pfValues[ix] * 100.0f / fn;
				float fye = pq->m_pfErrors[ix] * 100.0f / fn;
				if (fyc + fye > fMax)
					fMax = fyc + fye;
				if (fyc < fMin)
					fMin = fyc;
			};

			// Difference between minimum and maximum should be at least 1 percent
			fMin = floor(fMin);
			fMax = ceil(fMax);
			if (fMin == fMax)
				fMax += 1.0f;
		}
		else
		{
			fMin =   0.0f;
			fMax = 100.0f;
		};

		// Store the minimum and maximum percent value, so that OnMouseMove() can
		// calculate the percentage from the current mouse position.
		m_fYMin = fMin;
		m_fYMax = fMax;
		ASSERT( fMax - fMin >= 0.001f );

		// Scale the y-axis from <fMin> to <fMax> percent, the x-axis depending
		// on the number of values in the profile.
		CSize sizeHisto(rectHisto.Width(), rectHisto.Height());
		float fyy = sizeHisto.cy / (fMax - fMin);
		float fxx = sizeHisto.cx / (float)nx;

		// Draw tickers on the y-axes (both left and right side of the rectangle)
		int iTickLen = fpx * 2.0f;	// 10-tickers are 2 mm long, the others 1 mm
		CFont ScaleFont, *pOldFont;
		ScaleFont.CreatePointFont(80, "Arial", pOutDC);
		pOldFont = pDC->SelectObject(&ScaleFont);
		pDC->SetTextColor(RGB(0,0,0));
		pDC->SetBkMode(TRANSPARENT);
		int iystep	= ((sizeHisto.cy >= 200) ? 1 : (sizeHisto.cy >= 100) ? 2 : 5);
		int iystart	= (int)fMin;
		int iyend	= (int)fMax;
		for (register int iy=iystart; iy <= iyend; iy += iystep)
		{
			BOOL bLong	= ((iy % 10) == 0);
			float fy	= ((float)iy - fMin) * fyy;
			int iypos	= rectHisto.bottom - (int)ROUND(fy);
			pDC->MoveTo(rectHisto.left, iypos);
			pDC->LineTo(rectHisto.left - (bLong ? iTickLen : iTickLen/2), iypos);
			pDC->MoveTo(rectHisto.right, iypos);
			pDC->LineTo(rectHisto.right + (bLong ? iTickLen : iTickLen/2) , iypos);
			if ( bLong )
			{
				// Meaning of y-axis left and right beneath y=50
				if (iy == 50)
				{
					sprintf(szHelp, "%% %d", iy);
					pDC->SetTextAlign(TA_BASELINE | TA_RIGHT);
					pDC->TextOut(rectHisto.left-iTickLen, iypos, szHelp, strlen(szHelp));
					sprintf(szHelp, "%d %%", iy);
					pDC->SetTextAlign(TA_BASELINE | TA_LEFT);
					pDC->TextOut(rectHisto.right+iTickLen, iypos, szHelp, strlen(szHelp));
				}
				else
				{
					_itoa(iy, szHelp, 10);
					pDC->SetTextAlign(TA_BASELINE | TA_RIGHT);
					pDC->TextOut(rectHisto.left-iTickLen, iypos, szHelp, strlen(szHelp));
					pDC->SetTextAlign(TA_BASELINE | TA_LEFT);
					pDC->TextOut(rectHisto.right+iTickLen, iypos, szHelp, strlen(szHelp));
				}
			}
		};

		// Draw tickers and labels on the x-axes as well as the bars
		for (ix=1; ix <= nx; ix++)
		{
			// <ixpos1> is the left, <ixpos2> the right edge of a histogram column,
			// <ixpos> the center, where the ticker is placed.
			int ixpos1 = rectHisto.left + (int)ROUND((float)(ix-1)*fxx);
			int ixpos2 = rectHisto.left + (int)ROUND((float)ix*fxx);
			int ixpos  = (ixpos1 + ixpos2) / 2;
			BOOL bLong = ((ix % 5) == 0);
			pDC->MoveTo(ixpos, rectHisto.bottom);
			pDC->LineTo(ixpos, rectHisto.bottom + (bLong ? iTickLen : iTickLen/2));
			pDC->MoveTo(ixpos, rectHisto.top);
			pDC->LineTo(ixpos, rectHisto.top - (bLong ? iTickLen : iTickLen/2));
			if ( bLong )
			{
				_itoa(ix, szHelp, 10);
				pDC->SetTextAlign(TA_TOP | TA_CENTER);
				pDC->TextOut(ixpos, rectHisto.bottom+iTickLen, szHelp, strlen(szHelp));
				pDC->SetTextAlign(TA_BOTTOM | TA_CENTER);
				pDC->TextOut(ixpos, rectHisto.top-iTickLen, szHelp, strlen(szHelp));
			};

			// Calculate percent value and error value
			float fyc = pq->m_pfValues[ix-1] * 100.0f / fn;
			float fye = pq->m_pfErrors[ix-1] * 100.0f / fn;
			float fy  = (fyc - fMin) * fyy;

			// Assuming that the column's heights are steadily decreasing from
			// the left to the right, we store the index of the last column
			// with <fyc> less than <m_fBorderY>.
			if (pDoc->m_iRunType == 4 && pDoc->m_iProbFilter == 100 && fyc < m_fBorderY)
			{
				pDoc->m_iProbFilter = ix - 1;
			};

			CRect rectCol;
			rectCol.left   = ixpos1;
			rectCol.right  = ixpos2;
			rectCol.bottom = rectHisto.bottom + 1;
			rectCol.top	   = rectHisto.bottom - (int)ROUND(fy);

			// The column's color depends on its height relative to the
			// horizontal border line.
			CBrush brCol, *pOldBrush;
			brCol.CreateSolidBrush((fyc > m_fBorderY) ? RGB(128,255,0) : RGB(0,128,0));
			pOldBrush = pDC->SelectObject(&brCol);
			pDC->Rectangle(&rectCol);
			pDC->SelectObject(pOldBrush);

			// If an error has been defined, add a grey rectangle atop of the column
			int ihe = (int)ROUND(fye*fyy);
			if (ihe >= 1)
			{
				CRect rectErr;
				CBrush brErr;
				brErr.CreateSolidBrush(RGB(192,192,192));
				pOldBrush = pDC->SelectObject(&brErr);
				rectErr.left	= rectCol.left;
				rectErr.right	= rectCol.right;
				rectErr.bottom	= rectCol.top;
				rectErr.top		= rectErr.bottom - ihe;
				pDC->Rectangle(&rectErr);
				pDC->SelectObject(pOldBrush);
			}
		};

		TRACE("CQsdHistogramView::OnDraw() - pDoc->m_iProbFilter set to %d\n",
			pDoc->m_iProbFilter);

		// Label the x-axis with "Neighbours" when run in Single Mode, or
		// "Probability" when run in Prediction Mode.
		TEXTMETRIC tm;
		pDC->GetTextMetrics(&tm);
		pDC->SetTextAlign(TA_CENTER | TA_TOP | TA_NOUPDATECP);
		strcpy(szHelp, (pDoc->m_iRunType == 4) ? "Probability-%" : "Neighbours");
		pDC->TextOut((rectHisto.left+rectHisto.right)/2,
					 rectHisto.bottom+iTickLen+tm.tmHeight,
					 szHelp, strlen(szHelp));

		// Draw the horizontal border line and initialize the rectangles <m_rectSplit>
		// and <m_rectHisto> for WM_SETCURSOR, WM_MOUSEMOVE, etc.
		CPen BorderPen(PS_SOLID, 1, RGB(255,0,0));
		float fy = (m_fBorderY - fMin) * fyy;
		int iyb	 = rectHisto.bottom - (int)ROUND(fy);
		pDC->SelectObject(&BorderPen);
		pDC->MoveTo(rectHisto.left, iyb);
		pDC->LineTo(rectHisto.right, iyb);

		// Position the percentage so that it is not hidden by columns,
		// that means the more right the lower the percentage is.
		char szPercent[16];
		sprintf(szPercent, "%4.1f", m_fBorderY);
		for (ix=0; ix < nx; ix++)
		{
			float fv = pq->m_pfValues[ix] * 100.0f / fn;
			if (fv < m_fBorderY)
				break;
		};
		pDC->SetTextAlign(TA_BOTTOM | TA_LEFT);
		pDC->SetTextColor(RGB(255,0,0,));
		pDC->TextOut(rectHisto.left+(int)ROUND((float)(ix+1)*fxx), iyb,
					 szPercent, strlen(szPercent));

		pDC->SelectObject(pOldFont);

		// Save rectangle of the horizontal border line's area
		m_rectSplit.left   = rectHisto.left;
		m_rectSplit.right  = rectHisto.right;
		m_rectSplit.top    = iyb - 2;
		m_rectSplit.bottom = iyb + 3;

		// The histogram rectangle's dimensions will be used to calculate the
		// new percentage value when the border line is shifted with the mouse.
		m_rectHisto		   = rectHisto;

	}		// END if (pDoc->m_iQsdSelProfile >= 0 && ncnt > 0)....
	else
	{
		m_rectHisto.SetRectEmpty();
		m_rectSplit.SetRectEmpty();
	};

	// Now the bitmap is ready and can be copied to the output device.
	if ( !bPrint )
	{
		pOutDC->BitBlt(rectDraw.left, rectDraw.top, rectDraw.right, rectDraw.bottom,
					   &dcBmp, 0, 0, SRCCOPY);
		dcBmp.SelectObject(pOldBmp);
		Bmp.DeleteObject();
	};

	VERIFY( pOutDC->RestoreDC(iSavedDC) );
}

/////////////////////////////////////////////////////////////////////////////
// CQsdHistogramView diagnostics

#ifdef _DEBUG
void CQsdHistogramView::AssertValid() const
{
	CView::AssertValid();
}

void CQsdHistogramView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

CDiscoveryDoc* CQsdHistogramView::GetDocument()
{
	ASSERT_KINDOF( CDiscoveryDoc, m_pDocument );
	return (CDiscoveryDoc*)m_pDocument;
};

/////////////////////////////////////////////////////////////////////////////
// CQsdHistogramView message handlers

void CQsdHistogramView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	// Set the horizontal border line to the top of the first column, which
	// has the maximum percentage value.
	CDiscoveryDoc* pDoc = GetDocument();
	if (pDoc->m_iQsdSelProfile >= 0)
	{
		int np = pDoc->GetQsdRecordCount((pDoc->m_iRunType == 4) ? 1 : 0);
		if (np > 0)
		{
			float fv   = pDoc->m_QsdProfiles[pDoc->m_iQsdSelProfile].m_pfValues[0];
			m_fBorderY = fv * 100.0f / (float)np;
		}
		else
			m_fBorderY = 0.0f;
	}
}

BOOL CQsdHistogramView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	// Set mouse cursor indicating that the horizontal border line can be shifted
	POINT ptMouse;
	::GetCursorPos(&ptMouse);
	ScreenToClient(&ptMouse);
	if ( !m_rectSplit.IsRectEmpty() && m_rectSplit.PtInRect(ptMouse) )
	{
		::SetCursor(AfxGetApp()->LoadCursor(AFX_IDC_VSPLITBAR));
		return FALSE;
	}
	
	return CView::OnSetCursor(pWnd, nHitTest, message);
}

void CQsdHistogramView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// When mouse is within rectangle of splitter, capture and set shifting mode.
	if ( !m_rectSplit.IsRectEmpty() && m_rectSplit.PtInRect(point) )
	{
		m_bSplitTracking = TRUE;
		SetCapture();
	};
	
	CView::OnLButtonDown(nFlags, point);
}

void CQsdHistogramView::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// Release mouse capture and end shifting mode
	if ( m_bSplitTracking )
	{
		::ReleaseCapture();
		m_bSplitTracking = FALSE;

		// When in prediction mode, redraw Discovery Space cube (grey balls)
		GetDocument()->UpdateAllViews(this, HINT_HISTOCHANGE);
	};
	
	CView::OnLButtonUp(nFlags, point);
}

void CQsdHistogramView::OnMouseMove(UINT nFlags, CPoint point) 
{
	// Calculate new position of horizontal border line when in shifting mode
	// and redraw histogram.
	if ( m_bSplitTracking )
	{
		ASSERT( !m_rectHisto.IsRectNull() );
		float fRel = m_fYMin + (float)(m_rectHisto.bottom - point.y) *
								(m_fYMax - m_fYMin) / (float)m_rectHisto.Height();
		if (fRel < m_fYMin)
			fRel = m_fYMin;
		else if (fRel > m_fYMax)
			fRel = m_fYMax;
/*		float fRel = (float)(m_rectHisto.bottom - point.y) * 100.0f /
					 (float)m_rectHisto.Height();
		if (fRel < 0.0f)
			fRel = 0.0f;
		else if (fRel > 100.0f)
			fRel = 100.0f;	*/
		m_fBorderY = fRel;
		RedrawWindow();
	};
	
	CView::OnMouseMove(nFlags, point);
}

void CQsdHistogramView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// Cursor key up/down: Increment/Decrement <m_fBorderY> for 1 percent.
	// Page up/down: Increment/Decrement <m_fBorderY> for 10 percent.
	// Cursor key right/left: Set <m_fBorderY> to the top of the right/left
	//                        neighbour column.
	if (nChar == VK_UP || nChar == VK_DOWN || nChar == VK_PRIOR || nChar == VK_NEXT)
	{
		CDiscoveryDoc* pDoc = GetDocument();
		m_fBorderY += (	(nChar == VK_UP)	?   1.0f :
						(nChar == VK_DOWN)	?  -1.0f :
						(nChar == VK_PRIOR)	?  10.0f : -10.0f);
		if (m_fBorderY < m_fYMin/*0.0f*/)
			m_fBorderY = m_fYMin/*0.0f*/;
		else if (m_fBorderY > m_fYMax/*100.0f*/)
			m_fBorderY = m_fYMax/*100.0f*/;
		RedrawWindow();
		pDoc->UpdateAllViews(this, HINT_HISTOCHANGE);
	}
	else if (nChar == VK_LEFT || nChar == VK_RIGHT)
	{
		CDiscoveryDoc* pDoc = GetDocument();
		if (pDoc->m_iQsdSelProfile < 0)
			return;
		CQsdProfile *pq	= &pDoc->m_QsdProfiles[pDoc->m_iQsdSelProfile];
		float fn		= (float)pDoc->GetQsdRecordCount((pDoc->m_iRunType == 4) ? 1 : 0);
		int nx			= pq->GetValueCount();
		ASSERT( nx > 0 );
		for (register int ix=0; ix < nx; ix++)
		{
			float fv = pq->m_pfValues[ix] * 100.0f / fn;
			if (fv < m_fBorderY)
				break;
		};
		if (nChar == VK_LEFT)
		{
			if (ix > 0)
				ix--;
		}
		else
		{
			//if (ix < nx-1)
			ix++;
		};
		if (ix < nx)
			m_fBorderY = pq->m_pfValues[ix] * 100.0f / fn + 0.00001f;
		else
			m_fBorderY = m_fYMin;
		RedrawWindow();
		pDoc->UpdateAllViews(this, HINT_HISTOCHANGE);
	}
	else
		CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CQsdHistogramView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	TRACE("CQsdHistogramView::OnContextMenu() - processing WM_CONTEXTMENU\n");

	// See remarks in CProfileListView::OnContextMenu()
	GetParentFrame()->ActivateFrame();
	CPoint pt(point);
	if (pt.x < 0 && pt.y < 0)
	{
		pt.x = pt.y = 0;
		ClientToScreen(&pt);
	};
	CMenu Menu, *pSubMenu;
	Menu.LoadMenu(IDR_QSD_HISTO);
	pSubMenu  = Menu.GetSubMenu(0);
	UINT uCmd = GetPopupMenuCmd(pSubMenu, pt.x, pt.y, AfxGetMainWnd());
	if (uCmd != 0)
		AfxGetMainWnd()->PostMessage(WM_COMMAND, uCmd);
}

void CQsdHistogramView::OnViewQsdHistoFull() 
{
	m_bZoom = FALSE;
	RedrawWindow();
}

void CQsdHistogramView::OnUpdateViewQsdHistoFull(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(m_bZoom == FALSE);
}

void CQsdHistogramView::OnViewQsdHistoZoom() 
{
	m_bZoom = TRUE;
	RedrawWindow();
}

void CQsdHistogramView::OnUpdateViewQsdHistoZoom(CCmdUI* pCmdUI) 
{
	pCmdUI->SetRadio(m_bZoom);
}
