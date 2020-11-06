// DiscoveryView.cpp : implementation of the CDiscoveryView class
// Created: 11.01.99 - Latest update: 01.02.99
// Ported to Windows 8.1 by Fulcrum.013 Mariupol, Ukraine 
// 17 september 2020


#include "stdafx.h"
#include "Discovery.h"

#include "DiscoveryDoc.h"
#include "DiscoveryView.h"
#include "DiscoveryControls.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryView

IMPLEMENT_DYNCREATE(CDiscoveryView, CView)

BEGIN_MESSAGE_MAP(CDiscoveryView, CView)
	//{{AFX_MSG_MAP(CDiscoveryView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateEditSelectAll)
	ON_COMMAND(ID_EDIT_SELECT_INV, OnEditSelectInv)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_INV, OnUpdateEditSelectInv)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_FILE_SAVE_DATA, OnFileSaveData)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_DATA, OnUpdateFileSaveData)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_REPORT_SELECT_COLUMN, ID_REPORT_SORT_NO, OnReportHeaderCmd)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	// User defined messages
	ON_MESSAGE(WM_REPORT_COLUMNORDERCHANGED, OnReportColumnOrderChanged)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryView construction/destruction

CDiscoveryView::CDiscoveryView()
{
	m_iPrintFontHeight	= 200;	// 10 points default font size for printing
	m_nPrintLines		= 0;
	m_nPrintLinesPerPage= 0;
}

CDiscoveryView::~CDiscoveryView()
{
}

BOOL CDiscoveryView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryView drawing

void CDiscoveryView::OnDraw(CDC* pDC)
{
	CDiscoveryDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryView printing

// If at least one row has been selected, preset "Selection" in printer dialog,
// otherwise preset "All".
BOOL CDiscoveryView::OnPreparePrinting(CPrintInfo* pInfo)
{
	if (m_Report.GetSelectedCount() > 0 && !pInfo->m_bDirect)
	{
		pInfo->m_pPD->m_pd.Flags &= ~PD_NOSELECTION;
		pInfo->m_pPD->m_pd.Flags &= ~PD_ALLPAGES;
		pInfo->m_pPD->m_pd.Flags |=  PD_SELECTION;
	};

	return DoPreparePrinting(pInfo);
}

// Calculates the font size, the number of lines per page and the number of pages
void CDiscoveryView::OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo)
{
	// Get number of columns
	int nColumns = m_Report.GetColumnCount();

	// Calculate total width of list in pixels
	int itotwid	= 0;
	for (register int ic=0; ic < nColumns; ic++)
		itotwid += m_Report.GetColumnWidth(ic);

	// Total width and height of a page in inches
	float fwidthtot  = (float)pDC->GetDeviceCaps(HORZRES) / (float)pDC->GetDeviceCaps(LOGPIXELSX);
	float fheighttot = (float)pDC->GetDeviceCaps(VERTRES) / (float)pDC->GetDeviceCaps(LOGPIXELSY);

	// Printable width = total width minus both 1/2 inch left and right margin
	float fwid		 = fwidthtot - 1.0f;	// in inches
	float fwidtwips	 = fwid * 1440.0f;		// in twips (1 inch = 1440 twips)

	// Estimate number of characters over the total list width
	LOGFONT lf;
	CFont *pListFont = m_Report.GetFont();
	pListFont->GetLogFont(&lf);
	float fnListChars = (float)itotwid / (float)abs(lf.lfHeight);

	// Use Twips mapping mode to adjust font size to twips
	pDC->SetMapMode(MM_TWIPS);

	// Calculate font size in twips and store that for CDiscoveryView::OnPrint().
	// Use TEXTMETRIC.tmHeight to calculate the numbers of lines per page.
	TEXTMETRIC tm;
	CFont NewFont, *pOldFont;
	m_iPrintFontHeight = (int)ROUND(fwidtwips / fnListChars);
	if (m_iPrintFontHeight > 240)
		m_iPrintFontHeight = 240;	// not bigger than 12 points
	lf.lfHeight	= -1*m_iPrintFontHeight;
	NewFont.CreateFontIndirect(&lf);
	pOldFont	= (CFont*)pDC->SelectObject(&NewFont);
	pDC->GetTextMetrics(&tm);
	int iFontHeight	= tm.tmHeight + tm.tmExternalLeading;

	// Now calculate the numbers of lines per page. Therefore regard total height
	// minus 1 inch upper and lower margin.
	float fhtwips		 = (fheighttot - 2.0f) * 1440.0f;
	m_nPrintLinesPerPage = (int)(fhtwips / (float)iFontHeight) - 1;	// subtract -1 for title

	// Set number of pages (1..n)
	m_nPrintLines = (pInfo->m_pPD->PrintRange() ? m_Report.GetSelectedCount() : m_Report.GetItemCount());
	pInfo->SetMaxPage(1 + (m_nPrintLines-1)/m_nPrintLinesPerPage);

	pDC->SelectObject(pOldFont);
}

// Print one page of the report list. The font size and the number of lines per page
// have been determined in CDiscoveryView::OnBeginPrinting().
void CDiscoveryView::OnPrint(CDC* pDC, CPrintInfo* pInfo) 
{
	pDC->SetMapMode(MM_TWIPS);

	// For printing, use the same font that is used for the list on the screen,
	// but with the size that has been adjusted in CDiscovery::OnBeginPrinting().
	// Set attribute bold for the selected column as well as both bold and underlined
	// for the title row.
	LOGFONT lf;
	CFont StdFont;					// standard font for the non-selected columns
	CFont SelFont;					// bold font for the selected column
	CFont TitleFont;				// bold and underlined font for the title
	CFont *pOldFont, *pListFont;
	pListFont		= m_Report.GetFont();
	pListFont->GetLogFont(&lf);
	
	lf.lfHeight		= -1 * m_iPrintFontHeight;
	StdFont.CreateFontIndirect(&lf);

	lf.lfWeight		= FW_BOLD;
	SelFont.CreateFontIndirect(&lf);

//	lf.lfUnderline	= 1;
	TitleFont.CreateFontIndirect(&lf);

	pOldFont		= (CFont*)pDC->SelectObject(&StdFont);

	// To position list and headline, convert page size to twips units
	CSize sizePage(	(int)ROUND((float)pDC->GetDeviceCaps(HORZRES)/(float)pDC->GetDeviceCaps(LOGPIXELSX)*1440.0f),
					(int)ROUND((float)pDC->GetDeviceCaps(VERTRES)/(float)pDC->GetDeviceCaps(LOGPIXELSY)*1440.0f));

	// Format string for headline "Report View of ... - Page 1 of 2".
	// Page numbers are formatted right-aligned.
	CString strLine, strFmt;
	strLine.LoadString(IDS_REPORT_VIEW_OF);
	strLine += GetDocument()->GetTitle();
	pDC->TextOut(720, -720, strLine);

	strFmt.LoadString(IDS_PAGE_X_OF_X);
	strLine.Format(strFmt, pInfo->m_nCurPage, pInfo->GetMaxPage());
	CSize sizePageNo = pDC->GetTextExtent(strLine);
	pDC->TextOut(sizePage.cx - 720 - sizePageNo.cx, -720, strLine);

	// The line height in twips will be used to increment the y position
	TEXTMETRIC tm;
	pDC->GetTextMetrics(&tm);
	int nHeight = tm.tmHeight + tm.tmExternalLeading;

	// Get number of columns
	int nColumns = m_Report.GetColumnCount();

	// Get the column widths in twips
	int aiColWidth[MAX_REPORT_COLUMNS], aiColStart[MAX_REPORT_COLUMNS+1];
	for (register int ic=0, ncs=0; ic < nColumns; ic++)
	{
		aiColStart[ic]	= ncs;
		aiColWidth[ic]	= m_Report.GetColumnWidth(ic);
		ncs			   += aiColWidth[ic];
	};
	aiColStart[ic]		= ncs;
	for ( ; ic < MAX_REPORT_COLUMNS; ic++)
		aiColStart[ic+1]= aiColWidth[ic] = -1;
	float fak = (float)(sizePage.cx - 1440) / (float)ncs;
	for (ic=0; ic < nColumns; ic++)
	{
		aiColStart[ic]	= (int)ROUND((float)aiColStart[ic]*fak);
		aiColWidth[ic]	= (int)ROUND((float)aiColWidth[ic]*fak);
	};
	aiColStart[ic]		= (int)ROUND((float)aiColStart[ic]*fak);

	// Left and right margin for each column one average character wide
	int iBorder			= tm.tmAveCharWidth/* / 2*/;

	// Distance of list from the left edge: 1/2 inch, 1 inch from the top.
	CPoint point(720,-1440);

	// Now format the title. Use ellipses if string is wider than column.
	int aiColFmt[MAX_REPORT_COLUMNS];
	pDC->SelectObject(&TitleFont);
	for (ic=0; ic < nColumns; ic++)
	{
		char szText[1024];
		LV_COLUMN lvc;
		lvc.pszText		= szText;
		lvc.cchTextMax	= sizeof(szText);
		lvc.mask		= LVCF_TEXT | LVCF_FMT;
		m_Report.GetColumn(ic, &lvc);
		aiColFmt[ic]	= lvc.fmt & LVCFMT_JUSTIFYMASK;

		UINT uFormat	= DT_SINGLELINE	|		// no line wrap
						  DT_NOPREFIX	|		// don't use & for underlining
						  DT_VCENTER	|
						  DT_END_ELLIPSIS;		// end text with "..." if it does not fit in rectangle
		uFormat		   |= ((aiColFmt[ic] == LVCFMT_RIGHT)  ? DT_RIGHT  :
						   (aiColFmt[ic] == LVCFMT_CENTER) ? DT_CENTER : DT_LEFT);

		// Calculate the text surrounding rectangle for DrawText()
		CRect rcText(point.x+aiColStart[ic]+iBorder, point.y, point.x+aiColStart[ic+1]-iBorder, point.y-nHeight);

		UINT iOldAlign	= pDC->GetTextAlign();
		pDC->DrawText(szText, -1, &rcText, uFormat);
		pDC->SetTextAlign(iOldAlign);
	};

	// Now format the lines, using formatting from list
	int irow	= (pInfo->m_nCurPage - 1) * m_nPrintLinesPerPage;
	int nrows	= m_Report.GetItemCount();
	for (register int i=0; i < m_nPrintLinesPerPage && irow < nrows; i++, irow++)
	{
		point.y -= nHeight;
		for (ic=0; ic < nColumns; ic++)
		{
			char szText[1024];
			if (m_Report.GetItemText(irow, ic, szText, sizeof(szText)) <= 0)
				continue;
			if (strlen(szText) <= 0)
				continue;
		
			// Use the bold font for the selected column
			pDC->SelectObject((ic == m_Report.m_iSel) ? &SelFont : &StdFont);

			UINT uFormat	= DT_SINGLELINE	| DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS;
			uFormat		   |= ((aiColFmt[ic] == LVCFMT_RIGHT)  ? DT_RIGHT  :
							   (aiColFmt[ic] == LVCFMT_CENTER) ? DT_CENTER : DT_LEFT);

			// Calculate the text surrounding rectangle for DrawText()
			CRect rcText(point.x+aiColStart[ic]+iBorder, point.y, point.x+aiColStart[ic+1]-iBorder, point.y-nHeight);
		
			UINT iOldAlign	= pDC->GetTextAlign();
			pDC->DrawText(szText, -1, &rcText, uFormat);
			pDC->SetTextAlign(iOldAlign);
		}
	};

	pDC->SelectObject(pOldFont);
}

void CDiscoveryView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryView diagnostics

#ifdef _DEBUG
void CDiscoveryView::AssertValid() const
{
	CView::AssertValid();
}

void CDiscoveryView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CDiscoveryDoc* CDiscoveryView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDiscoveryDoc)));
	ASSERT_VALID(m_pDocument);
	return (CDiscoveryDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryView message handlers

int CDiscoveryView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Create list control window in report view mode. The size will be set
	// automatically responding to WM_SIZE sent to this view.
	VERIFY( m_Report.Create(LVS_REPORT | LVS_SHOWSELALWAYS | LVS_OWNERDRAWFIXED | 
							WS_CHILD | WS_BORDER | WS_HSCROLL,
							CRect(0,0,0,0), this, 1) );
	
	return 0;
}

void CDiscoveryView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	// Adjust size of embedded list control to its parent view size
	CRect rectView;
	GetClientRect(&rectView);
	m_Report.SetWindowPos(NULL, 0, 0, rectView.Width(), rectView.Height(),
						  SWP_SHOWWINDOW | SWP_NOZORDER);
}

void CDiscoveryView::OnSetFocus(CWnd* pOldWnd) 
{
	CView::OnSetFocus(pOldWnd);

	// Forward the focus from the view to the embedded list control
	m_Report.SetFocus();
}

void CDiscoveryView::OnInitialUpdate() 
{
	//CView::OnInitialUpdate();

	// Enable drag and drop of columns in the Report View
	DWORD dwExStyle  = (DWORD)m_Report.SendMessage(LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
	dwExStyle		|= LVS_EX_HEADERDRAGDROP;
	m_Report.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwExStyle);

	// Since columns may be moved (using header drag and drop), we do not access
	// the column index from CDiscoveryDoc::m_ReportColumns but use the array
	// CReportCtrl::m_aiColInd[] which contains the actual indexes. Before the
	// first drag and drop operation, this contains 0,1,2,....
	// The index is stored in the CColumn object of each column to keep the
	// actual positions persistent.
	//for (register int icol=0; icol < MAX_REPORT_COLUMNS; icol++)
	//	m_Report.m_aiColInd[icol] = icol;

	// Build the list (insert columns, sort items, add items)
	Build();

	//OnUpdate(this, 0, NULL);
}

void CDiscoveryView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint) 
{
	TRACE("Entering CDiscoveryView::OnUpdate()\n");

	// TODO: Add your specialized code here and/or call the base class

	// Build the list (insert columns, sort items, add items)
	Build();
}

// This message is posted from the list control in response to a HDN_ENDDRAG
// notification message. Since Discovery handles the ordering of columns, it
// only registers the new position of the dragged column but then re-builds
// the entire list.
// Returns the result of the Build() function.

LRESULT CDiscoveryView::OnReportColumnOrderChanged (WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return Build();
};

// Build the report view list from scratch.
// This function is called from OnInitialUpdate() and after a column has been
// moved by a drag-and-drop operation. If only the sort order has changed,
// call Rebuild() instead of Build().
// Returns the number of columns if successfull, -1 otherwise.

int CDiscoveryView::Build (void)
{
	CDiscoveryDoc* pDoc = GetDocument();

	if (pDoc->m_ReportColumns.GetSize() <= 1)	// no document loaded?
		return 0;

	// If this function is called e.g. after header drag and drop, the old columns
	// must be removed.
	register int icol;
	for (icol=m_Report.GetColumnCount()-1; icol >= 0; icol--)
		m_Report.DeleteColumn(icol);

	// The number of columns to be inserted is one less than the number of
	// columns in CDiscoveryDoc::m_ReportColumns, since the first column only
	// stores stoichiometry informations.
	int nCols = pDoc->m_ReportColumns.GetSize() - 1;

	// Since the original order of the columns may have been changed by a header drag
	// and drop operation, request the actual indexes to CDiscoveryDoc::m_ReportColumns[].
	int aiColInd[MAX_REPORT_COLUMNS];
	pDoc->GetReportColumnIndexes(aiColInd);

	// To justify the column widths, we need the extents of characters of the list control's font
	CDC *pDC  = m_Report.GetDC();
	CSize siz = pDC->GetTextExtent(" ");
	int iswid = siz.cx * 8;				// see CReportCtrl::OnDrawItem()
	m_Report.ReleaseDC(pDC);

	for (icol=0; icol < nCols; icol++)
	{
		int iColInd			= aiColInd[icol];
		CColumn* pColumn	= &pDoc->m_ReportColumns[iColInd];
		CString strHeader	= pColumn->m_strLabel;

		// If the width has not been previously defined (-1), calculate a suitable
		// width from the width of the label plus space for the sort order icon.
		int iwid = pColumn->m_iColWidth;
		if (iwid < 0)
			iwid = m_Report.GetStringWidth(strHeader) + iswid;

		m_Report.InsertColumn(icol, strHeader, LVCFMT_LEFT, iwid, icol);
	};

	// Make all header items owner-drawn
	CHeaderCtrl* pHeader = m_Report.GetHeaderCtrl();
	nCols				 = pHeader->GetItemCount();
	ASSERT( nCols > 0 );
	for (icol=0; icol < nCols; icol++)
	{
		HD_ITEM hi;
		hi.mask = HDI_FORMAT;
		VERIFY( pHeader->GetItem(icol, &hi) );
		hi.fmt |= HDF_OWNERDRAW;
		VERIFY( pHeader->SetItem(icol, &hi) );
	};

	// Now call Rebuild() to sort and insert the items and subitems
	Rebuild();

	return nCols;
};

// Deletes all items and subitems for the report view, then calls the Sort()
// function and adds the sorted items and subitems to the list.
// Returns the number of rows if successfull, -1 otherwise.

int CDiscoveryView::Rebuild (void)
{
	CDiscoveryDoc* pDoc = GetDocument();

	// First remove all items and subitems
	m_Report.DeleteAllItems();

	// Sort the items and subitems, which sets the row indexes in <m_piRowInd>.
	Sort();

	// If there is only the stoichiometry column or no column, or if there are
	// no rows in the current document, we cannot do anything here.
	int nCols  = pDoc->m_ReportColumns.GetSize() - 1;
	if (nCols <= 0)
		return 0;

	int nItems = pDoc->m_ReportColumns[0].GetRowCount();
	if (nItems <= 0)
		return 0;

	// First add the items, which means the contents of the first visible column.
	// The index may be different from zero if header drag and drop has been performed.
	int aiColInd[MAX_REPORT_COLUMNS];
	pDoc->GetReportColumnIndexes(aiColInd);
	int iColInd			= aiColInd[0];
	CColumn* pColFirst	= &pDoc->m_ReportColumns[iColInd];
	register int irow, icol;
	for (irow=0; irow < nItems; irow++)
	{
		CString strItem;
		pColFirst->GetItemText(strItem, m_Report.m_piRowInd[irow]);
		m_Report.InsertItem(irow, strItem);
	};

	// Now add the subitems, which means the 2nd, 3rd, etc. column, for all
	// previously added items.
	for (irow=0; irow < nItems; irow++)
	{
		int iRowInd = m_Report.m_piRowInd[irow];
		for (icol=1; icol < nCols; icol++)
		{
			CString strSubItem;
			pDoc->m_ReportColumns[aiColInd[icol]].GetItemText(strSubItem, iRowInd);
			m_Report.SetItemText(irow, icol, strSubItem);
		}
	};

	// Finally transfer the selection and sort criteria from CDiscoveryDoc::m_ReportColumns[]
	// to the list control (for drawing the items, subitems, and header items)
	m_Report.m_iSel	= -1;
	nCols			= m_Report.GetColumnCount();
	for (icol=0; icol < nCols; icol++)
	{
		CColumn* pColumn = &pDoc->m_ReportColumns[aiColInd[icol]];
		if ( pColumn->m_bSel )
			m_Report.m_iSel = icol;

		// SetSort() tells the header item that it will be ownerdrawn
		m_Report.SetSort(icol, pColumn->m_iSort);
	};

	return m_Report.GetItemCount();
};

// Set the column with the index <iold> to the new position <inew>.
// Since this means an insertion, the indexes of most of the other columns must
// be shifted.
// Note: The calling function must post the message WM_REPORT_COLUMNORDERCHANGED
// to this view to rebuild the columns from scratch. Otherwise, the list control
// will handle the rank order of the columns itself.

int CDiscoveryView::RegisterColumnMove (int iold, int inew)
{
	CDiscoveryDoc* pDoc = GetDocument();

	return pDoc->MoveReportColumnIndex(iold, inew);
};

// Tell CDiscoveryDoc::m_ReportColumns[] that column <isel> of the list control
// is now being selected.
void CDiscoveryView::RegisterColumnSelection (int isel)
{
	CDiscoveryDoc* pDoc = GetDocument();

	for (register int ic=0, nc=pDoc->m_ReportColumns.GetSize(); ic < nc; ic++)
	{
		CColumn* pColumn = &pDoc->m_ReportColumns[ic];
		if (pColumn->m_iColInd >= 0)
		{
			pColumn->m_bSel = (pColumn->m_iColInd == isel);
		}
	}
};

// Tell CDiscoveryDoc::m_ReportColumns[] that column <icol> of the list control is
// now being sorted in ascending (1) or descending (-1) order or will not be sorted (0).
void CDiscoveryView::RegisterSortOrder (int icol, int isort)
{
	CDiscoveryDoc* pDoc = GetDocument();

	for (register int ic=0, nc=pDoc->m_ReportColumns.GetSize(); ic < nc; ic++)
	{
		CColumn* pColumn = &pDoc->m_ReportColumns[ic];
		if (pColumn->m_iColInd == icol)
		{
			pColumn->m_iSort = isort;
		}
	}
};

// Tell CDiscoveryDoc::m_ReportColumns[] that column <icol> now has the new width
// <iwid> in pixels.
void CDiscoveryView::RegisterColumnWidth (int icol, int iwid)
{
	CDiscoveryDoc* pDoc = GetDocument();

	for (register int ic=0, nc=pDoc->m_ReportColumns.GetSize(); ic < nc; ic++)
	{
		CColumn* pColumn = &pDoc->m_ReportColumns[ic];
		if (pColumn->m_iColInd == icol)
		{
			pColumn->m_iColWidth = iwid;
		}
	}
};

// Callback function for qsort(), which is called from CDiscovery::Sort()
// to sort the rows in the report view. The columns are sorted in left-to-right
// order. That means, the leftmost column has the highest priority.
// Uses the static array s_aiColInd[] to access the <s_nCols> indexes of the columns
// (which can be different from that in CDiscoveryDoc::m_ReportColumns[] because
// of user-defined rearrangement) and the static pointer <s_pReportColumns> to
// CDiscoveryDoc::m_ReportColumns[] to access the column contents.
// Returns code as usual for qsort().

static CColumnArray *s_pReportColumns;
static int s_aiColInd[MAX_REPORT_COLUMNS];
static int s_nCols;

static int _Sort (const void *elem1, const void *elem2)
{
	int i1 = *(int*)elem1;	// index of the first row to be compared
	int i2 = *(int*)elem2;	// index of the second row to be compared

	int isort = 0;
	for (register int ic=0; ic < s_nCols && isort == 0; ic++)
	{
		int icol = s_aiColInd[ic];	// index of the column in m_ReportColumns[]
		CColumn* pColumn = &(*s_pReportColumns)[icol];
		if (pColumn->m_iSort == 0)	// do not sort this column?
			continue;
		
		CReportItem* pi1 = &pColumn->m_Items[i1];
		CReportItem* pi2 = &pColumn->m_Items[i2];

		// The two items are sorted either alphabetically or numerically
		if (pColumn->m_bNumeric && pi1->m_nNumbers > 0 && pi2->m_nNumbers > 0)
		{
			int nn = min(pi1->m_nNumbers, pi2->m_nNumbers);
			for (register int in=0; in < nn && isort == 0; in++)
				isort = ((pi1->m_pNumbers[in] < pi2->m_pNumbers[in]) ? -1 :
						 (pi1->m_pNumbers[in] > pi2->m_pNumbers[in]) ?  1 : 0);
		}
		else
			isort = _stricmp(pi1->m_strText, pi2->m_strText);

		// Reverse result when sorting in descending order
		isort *= pColumn->m_iSort;
	};

	return isort;
};

// Sorts the rows of the report view using qsort(). The columns are sorted in
// left-to-right order. That means, the leftmost column has the highest priority.
// Returns the number of rows.

int CDiscoveryView::Sort (void)
{
	CDiscoveryDoc* pDoc = GetDocument();

	// First reset the indexes of the rows to 0,1,2,...
	int nItems = pDoc->m_ReportColumns[0].GetRowCount();
	if (m_Report.m_piRowInd != NULL)
		delete m_Report.m_piRowInd;
	if (nItems > 0)
	{
		m_Report.m_piRowInd = new int[nItems];
		for (register int irow=0; irow < nItems; irow++)
			m_Report.m_piRowInd[irow] = irow;
	}
	else
		m_Report.m_piRowInd = NULL;

	// Give the static callback sorting function access to CDiscoveryDoc::m_ReportColumns[]
	s_pReportColumns = &pDoc->m_ReportColumns;

	// Store the actual indexes of the columns in CDiscoveryDoc::m_ReportColumns[],
	// because the order of the columns in the list control is decisive.
	s_nCols = pDoc->GetReportColumnIndexes(s_aiColInd);

	// Now sort the items using qsort() with _Sort() as callback function.
	qsort(m_Report.m_piRowInd, nItems, sizeof(*m_Report.m_piRowInd), _Sort);

	return nItems;
};

// Whenever the user clicks with the right mouse button inside this view
// (may be on the header or inside the list itself), a WM_CONTEXTMENU message
// is generated. But right mouse button clicks have been processed by the
// Report list itself just before: CReportCtrl::OnRclick(), CReportCtrl::OnNotify().
// The following code will only be executed to process a WM_CONTEXTMENU message
// that has been generated from the keyboard (Shift+F10 or special context menu key).

void CDiscoveryView::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	if ( !m_Report.m_bProcessedContextMenu )
	{
		TRACE("CDiscoveryView::OnContextMenu() - processing WM_CONTEXTMENU\n");

		// See comments in CReportCtrl::OnNotify()
		GetParentFrame()->ActivateFrame();

		// When context menu has been requested by the keyboard, <point> contains
		// the screen coordinates (-1,-1). In that case we put the context menu
		// into the upper left corner of the view's client area.
		if (point.x < 0 && point.y < 0)
		{
			point.x = point.y = 0;
			m_Report.ClientToScreen(&point);
		};

		CMenu Menu, *pSubMenu;
		Menu.LoadMenu(IDR_REPORT_LIST);
		pSubMenu  = Menu.GetSubMenu(0);
	
		UINT uCmd = GetPopupMenuCmd(pSubMenu, point.x, point.y, AfxGetMainWnd());
		if (uCmd != 0)
			AfxGetMainWnd()->PostMessage(WM_COMMAND, uCmd);
	};

	m_Report.m_bProcessedContextMenu = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryView command handlers

void CDiscoveryView::OnEditSelectAll() 
{
	for (register int i=0, imax=m_Report.GetItemCount(); i < imax; i++)
		m_Report.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
}

void CDiscoveryView::OnUpdateEditSelectAll(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_Report.GetItemCount() > 0);
}

void CDiscoveryView::OnEditSelectInv() 
{
	for (register int i=0, imax=m_Report.GetItemCount(); i < imax; i++)
	{
		UINT iState = m_Report.GetItemState(i, LVIS_SELECTED);
		m_Report.SetItemState(i, iState ? 0 : LVIS_SELECTED, LVIS_SELECTED);
	}
}

void CDiscoveryView::OnUpdateEditSelectInv(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_Report.GetItemCount() > 0);
}

// Dummy command range function to enable the commands in the header control context menu
void CDiscoveryView::OnReportHeaderCmd(UINT uCmd)
{
}

// Copy selected rows to clipboard
void CDiscoveryView::OnEditCopy() 
{
	CopyList(m_Report);
}

void CDiscoveryView::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_Report.GetSelectedCount() > 0);
}

// Save report view in a selected text format
void CDiscoveryView::OnFileSaveData() 
{
	// Load the strings that appear in the File Format combo box of the Save As dialog
	CString strFilter((LPCSTR)IDS_SAVELISTFILTER);

	// Use the document file name as default name, but with extension "CSV"
	CString strFileName = GetDocument()->GetPathName();
	SetFileExtension (strFileName, "CSV");

	CFileSaveListDlg dlg(FALSE, NULL, strFileName,
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);

	if (dlg.DoModal() == IDOK)
	{
		// If user did not supply an extension, append a suitable one, depending
		// on the selected file format. A name ending with a dot means that no
		// extension is to be used.
		CString strPathName = dlg.GetPathName();
		if (strPathName.ReverseFind('.') == -1)
		{
			strPathName += ((dlg.m_ofn.nFilterIndex == 1) ? ".CSV" :
							(dlg.m_ofn.nFilterIndex == 2) ? ".TXT" :
							(dlg.m_ofn.nFilterIndex == 3) ? ".PRN" : "");
		};
		SaveList(m_Report, (dlg.m_ofn.nFilterIndex == 2) ? 1 :
						   (dlg.m_ofn.nFilterIndex == 3) ? 2 : 0, strPathName);
	}
}

void CDiscoveryView::OnUpdateFileSaveData(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_Report.GetItemCount() > 0);
}


/////////////////////////////////////////////////////////////////////////////
// CReportCtrl

CReportCtrl::CReportCtrl()
{
	//memset(m_aiColInd, 0, sizeof(m_aiColInd));
	m_piRowInd = NULL;

	memset(m_aiSort, 0, sizeof(m_aiSort));
	m_iSel = -1;

	m_bProcessedContextMenu = FALSE;
}

CReportCtrl::~CReportCtrl()
{
	if (m_piRowInd != NULL)
		delete m_piRowInd;
}

// Access the header control of the report view
CHeaderCtrl* CReportCtrl::GetHeaderCtrl (void)
{
	CWnd* pWnd = GetDlgItem(0);
	//ASSERT_KINDOF(CHeaderCtrl, pWnd);
	return (CHeaderCtrl*)pWnd;
};

// Returns the number of columns in the report view
int CReportCtrl::GetColumnCount (void)
{
	return GetHeaderCtrl()->GetItemCount();
};

// Returns the index of the header item with the x-coordinate <ix> relative
// to the left edge of the header control, or -1 if there is no header item.
int CReportCtrl::GetHeaderItemIndex (int ix)
{
	CHeaderCtrl* pHeader = GetHeaderCtrl();
	for (register int ih=0, nh=pHeader->GetItemCount(), iw=0; ih < nh; ih++)
	{
		HD_ITEM hdi;
		hdi.mask = HDI_WIDTH;
		pHeader->GetItem(ih, &hdi);
		if (ix >= iw && ix < hdi.cxy+iw)
			return ih;
		iw += hdi.cxy;
	};
	return -1;
};

// Gets the rectangle (coordinates relative to the upper left corner of the
// header control) of the header item with the index <index>.
// Returns TRUE if <lprect> contains valid coordinates, otherwise FALSE.
BOOL CReportCtrl::GetHeaderItemRect (int index, LPRECT lprect)
{
	CHeaderCtrl* pHeader = GetHeaderCtrl();
	for (register int ih=0, nh=pHeader->GetItemCount(), iw=0; ih < nh; ih++)
	{
		HD_ITEM hdi;
		hdi.mask = HDI_WIDTH;
		pHeader->GetItem(ih, &hdi);
		if (ih == index)
		{
			CRect rcHeader;
			pHeader->GetClientRect(&rcHeader);
			lprect->top    = rcHeader.top;
			lprect->bottom = rcHeader.bottom;
			lprect->left   = iw;
			lprect->right  = iw + hdi.cxy;
			return TRUE;
		};
		iw += hdi.cxy;
	};
	return FALSE;
};

// Sorts and rebuilds the list
/*int CReportCtrl::Rebuild (void)
{
	return 0;
};*/

// Draws text of the item and all subitems of a given row
void CReportCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	//TRACE("Entering CReportCtrl::DrawItem()\n");

	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);
	int nSavedDC = dc.SaveDC();

	CRect rcItem(lpDrawItemStruct->rcItem);
	int nItem	 = lpDrawItemStruct->itemID;	// nItem is the index of the row

	LV_ITEM lvi;
	lvi.mask		= LVIF_STATE;				// is row selected and/or focussed?
	lvi.iItem		= nItem;
	lvi.iSubItem	= 0;
	lvi.stateMask	= LVIS_SELECTED | LVIS_DROPHILITED | LVIS_FOCUSED;
	GetItem(&lvi);

	BOOL bHighlight	= ((lvi.state & LVIS_DROPHILITED) != 0 ||
					   (lvi.state & LVIS_SELECTED)    != 0);

	// Background and text color depend on if the row is highlighted, and if it
	// is highlighted, if the list has the focus (blue) or not (grey).
	COLORREF crBack, crText;
	if ( bHighlight )
	{
		if (::GetFocus() == GetSafeHwnd())
		{
			crBack = ::GetSysColor(COLOR_HIGHLIGHT);
			crText = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
		}
		else
		{
			crBack = ::GetSysColor(COLOR_INACTIVECAPTION);
			crText = ::GetSysColor(COLOR_INACTIVECAPTIONTEXT);
		}
	}
	else
	{
		crBack = ::GetSysColor(COLOR_WINDOW);
		crText = ::GetSysColor(COLOR_WINDOWTEXT);
	};
	dc.FillSolidRect(rcItem, crBack);	// erase entire row
	dc.SetBkColor(crBack);
	dc.SetTextColor(crText);

	// Now iterate through all columns
	LV_COLUMN lvc;
	lvc.mask = LVCF_WIDTH | LVCF_FMT | LVCF_SUBITEM;
	CRect rcSubItem(rcItem.left, rcItem.top, rcItem.left, rcItem.bottom);
	for (int iColumn=0; GetColumn(iColumn, &lvc); iColumn++)
	{
		rcSubItem.right += lvc.cx;
		int nSubItem	= lvc.iSubItem;
		CString strText = GetItemText(nItem, nSubItem);	// nSubItem may be different from iColumn
		if (strText.GetLength() == 0)
			continue;
		UINT uJustify = (((lvc.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT)  ? DT_RIGHT  :
						 ((lvc.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_CENTER) ? DT_CENTER :
																			 DT_LEFT);

		// If nSubItem corresponds to the selected column, use bold text
		BOOL bBold = (nSubItem == m_iSel);
		CFont BoldFont, *pOldFont = nullptr;
		if ( bBold )
		{
			LOGFONT lf;
			CFont* pFont = GetFont();
			pFont->GetLogFont(&lf);
			lf.lfWeight	 = 700;		// 400=normal, 700=bold
			BoldFont.CreateFontIndirect(&lf);
			pOldFont = dc.SelectObject(&BoldFont);
		};

		dc.DrawText(strText, -1, rcSubItem, uJustify
					| DT_SINGLELINE			// no line wrap
					| DT_NOPREFIX			// don't use & for underlining
					| DT_VCENTER
					| DT_END_ELLIPSIS);		// end text with "..." if it does not fit in rectangle

		if ( bBold )
			dc.SelectObject(pOldFont);

		// Next item begins at right end of current item
		rcSubItem.left = rcSubItem.right;
	};

	// Draw focus rectangle if item has the focus
	if (lvi.state & LVIS_FOCUSED && ::GetFocus() == GetSafeHwnd())
		dc.DrawFocusRect(rcItem);

	// Restore device context and detach HDC from lpDrawItemStruct->hDC
	dc.RestoreDC(nSavedDC);
	dc.Detach();
};

// Set sorting direction of column no. <icol>:
//   isort ==  1: Sort in ascending order,
//            -1: Sort in descending order,
//             0: Do not sort.
// Returns previous sorting order of that column.

int CReportCtrl::SetSort (int icol, int isort)
{
	CHeaderCtrl* pHeader = GetHeaderCtrl();

	ASSERT( icol >= 0 && icol < pHeader->GetItemCount() );
	ASSERT( isort >= -1 && isort <= 1 );
	ASSERT( icol < MAX_REPORT_COLUMNS );

	int iOldSort	= m_aiSort[icol];
	m_aiSort[icol]	= isort;

	// Request format of header item
	HD_ITEM hi;
	hi.mask = HDI_FORMAT;
	VERIFY( pHeader->GetItem(icol, &hi) );

	// The header item must be owner-drawn, if the column is selected (bold text)
	// or the column is sorted (arrow indicating sort order).
/*	if (isort != 0 || m_iSel == icol)
	{
*/		hi.fmt |= HDF_OWNERDRAW;
/*	}
	else
	{
		hi.fmt &= ~HDF_OWNERDRAW;
	};
*/
	pHeader->SetItem(icol, &hi);

	// Force the header control to be redrawn
	pHeader->Invalidate();

	return iOldSort;
};

BEGIN_MESSAGE_MAP(CReportCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CReportCtrl)
	ON_WM_DRAWITEM()
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnclick)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReportCtrl message handlers

// Draw one item of the report view's header. This code is taken from
// "MFC Programmer's Source Book",
// http://www.codeguru.com/listview/indicating_sort_order.shtml
// by Zafir Anjum.
// Note: Zafir subclasses an object derived from CHeaderCtrl, using CHeaderCtrl::DrawItem()
// for drawing. But subclassing does not work, since the header control (id=0) is not
// yet available when calling CReportCtrl::SubclassWindow().
// So I use the classic method of processing a WM_DRAWITEM message, which is sent
// to the parent window.

void CReportCtrl::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	//TRACE("Entering CReportCtrl::OnDrawItem()\n");

	ASSERT( nIDCtl == 0 );				// can only come from header, which has id = 0

	CHeaderCtrl* pHeader = GetHeaderCtrl();

	int itemID = lpDrawItemStruct->itemID;
	ASSERT( itemID >= 0 && itemID < MAX_REPORT_COLUMNS );

	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);
	int nSavedDC = dc.SaveDC();			// save DC and restore it at the end

	CRect rcLabel(lpDrawItemStruct->rcItem);	// to derive the text surrounding rectangle

	// Set clipping region to limit drawing within column
	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rcLabel);
	dc.SelectObject(&rgn);
	rgn.DeleteObject();

	dc.FillRect(rcLabel, &CBrush(::GetSysColor(COLOR_3DFACE)));	// draw the background

	// Labels are offset by a certain amount.
	// This offset is related to the width of a space character.
	CSize sizeText = dc.GetTextExtent(" ");
	int offset	   = sizeText.cx * 2;

	// Get the column text and format
	char buf[256];
	HD_ITEM hi;
	hi.mask			= HDI_TEXT | HDI_FORMAT;
	hi.pszText		= buf;
	hi.cchTextMax	= 255;
	VERIFY( pHeader->GetItem(itemID, &hi) );

	// Determine format for drawing column label.
	// The flag DT_END_ELLIPSIS is used to end the text with "...", if the text
	// does not fit within the rectangle.
	UINT uFormat = DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER | DT_END_ELLIPSIS;
	if		((hi.fmt & HDF_CENTER) != 0)
		uFormat |= DT_CENTER;
	else if	((hi.fmt & HDF_RIGHT)  != 0)
		uFormat |= DT_RIGHT;
	else
		uFormat |= DT_LEFT;

	// Adjust the rect if the mouse button is pressed on it
	if (lpDrawItemStruct->itemState == ODS_SELECTED)
	{
		rcLabel.left++;
		rcLabel.top += 2;
		rcLabel.right++;
	};

	// Adjust the rect further if Sort arrow is to be displayed
	if (m_aiSort[itemID] != 0)
	{
		rcLabel.right -= 3 * offset;
	};

	rcLabel.left	+= offset;
	rcLabel.right	-= offset;

	// Draw the column label. If the column is selected, use a bold font.
	if (rcLabel.left < rcLabel.right)
	{
		LOGFONT lf;
		CFont *pFont, BoldFont, *pOldFont = nullptr;
		BOOL bBold = (m_iSel == itemID);
		if ( bBold )
		{
			pFont = pHeader->GetFont();
			VERIFY( pFont->GetLogFont(&lf) );
			lf.lfWeight = 700;				// 400=normal, 700=bold
			BoldFont.CreateFontIndirect(&lf);
			pOldFont = dc.SelectObject(&BoldFont);
		};
		
		dc.DrawText (buf, -1, rcLabel, uFormat);

		if ( bBold )
		{
			dc.SelectObject(pOldFont);
		}
	};

	// Draw the Sort arrow
	if (m_aiSort[itemID] != 0)
	{
		CRect rcIcon( lpDrawItemStruct->rcItem );
	
		// Set up pens to use for drawing the triangle
		CPen penLight(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
		CPen penShadow(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
		CPen *pOldPen = dc.SelectObject( &penLight );
		
		if (m_aiSort[itemID] == 1)			// ascending
		{
			// Draw triangle pointing upwards
			dc.MoveTo( rcIcon.right - 2*offset, offset-1);
			dc.LineTo( rcIcon.right - 3*offset/2, rcIcon.bottom - offset );
			dc.LineTo( rcIcon.right - 5*offset/2-2, rcIcon.bottom - offset );
			dc.MoveTo( rcIcon.right - 5*offset/2-1, rcIcon.bottom - offset-1 );
			dc.SelectObject( &penShadow );
			dc.LineTo( rcIcon.right - 2*offset, offset-2);
		}
		else
		{
			// Draw triangle pointing downwords
			dc.MoveTo( rcIcon.right - 3*offset/2, offset-1);
			dc.LineTo( rcIcon.right - 2*offset-1, rcIcon.bottom - offset + 1 );
			dc.MoveTo( rcIcon.right - 2*offset-1, rcIcon.bottom - offset );
			dc.SelectObject( &penShadow );
			dc.LineTo( rcIcon.right - 5*offset/2-1, offset -1 );
			dc.LineTo( rcIcon.right - 3*offset/2, offset -1);
		};
		
		dc.SelectObject(pOldPen);	// restore the pen
	};
	
	dc.RestoreDC(nSavedDC);			// restore dc
	
	dc.Detach();					// detach the dc before returning
}

// Register a right mouse button click on the header control to display a special
// context menu for the header (Column selection, Sort order).

BOOL CReportCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	HD_NOTIFY *pHDN	= (HD_NOTIFY*)lParam;
	NMHDR* pNH		= (NMHDR*)lParam;
	NMHEADER* pNMH	= (NMHEADER*)lParam;

	// Test
	if (wParam == 0)
	{
		static int s_iDragCol = -1;

		if ((pNH->code == HDN_BEGINTRACKA || pNH->code == HDN_BEGINTRACKW))
		{
			TRACE("CReportCtrl::OnNotify() - HDN_BEGINTRACK called...\n");
		}
		else if ((pNH->code == HDN_ENDTRACKA || pNH->code == HDN_ENDTRACKW))
		{
			TRACE("CReportCtrl::OnNotify() - HDN_ENDTRACK called for item %d\n", pHDN->iItem);

			CDiscoveryView* pView = (CDiscoveryView*)GetParent();
			ASSERT_KINDOF( CDiscoveryView, pView );

			// Register the new width of the column with index given in <HD_NOTIFY.iItem>
			pView->RegisterColumnWidth(pHDN->iItem, GetColumnWidth(pHDN->iItem));
		}

		// The following information about the new header notification messages
		// HDN_BEGINDRAG and HDN_ENDDRAG are taken from:
		// http://premium.microsoft.com/msdn/library/periodic/period98/html/msj0798spy.htm,
		// "Control Spy Exposes the Clandestine Life of Windows Common Controls, Part I",
		// by Mark Finocchio:

		// "There are some issues with the Header control concerning its notification
		// messages. When the HDN_ITEMCLICK, HDN_ITEMDBLCLICK, or HDN_BEGINDRAG message
		// is received, a pointer to a NMHEADER structure is sent along with it.
		// This structure contains information about the item to which the
		// notification refers. One member of the NMHEADER is not set when these
		// notifications are received: pItem. This member points to a structure
		// called HDITEM that contains additional information. However, it is set to null.
		// Three other notifications make use of NMHEADER: HDN_ENDDRAG, HDN_BEGINTRACK,
		// and HDN_ENDTRACK. These notifications set pItem, but they don’t set the
		// lParam member of the HDITEM structure to which it points.
		// Take this into consideration when writing notification handlers."

		else if (pNH->code == HDN_BEGINDRAG)
		{
			//HDITEM* phdi = pNMH->pitem;

			// NMHEADER.iItem = subitem index of the column being dragged.
			// Note: NMHEADER.pitem is NULL.
			TRACE("CReportCtrl::OnNotify() - HDN_BEGINDRAG - NMHEADER.iItem=%d, NMHEADER.iButton=%d\n",
				pNMH->iItem, pNMH->iButton);

			s_iDragCol = pNMH->iItem;
		}
		else if (pNH->code == HDN_ENDDRAG)
		{
			HDITEM* phdi = pNMH->pitem;

			// NMHEADER.pitem.iOrder = new index of the dragged column
			TRACE("CReportCtrl::OnNotify() - HDN_ENDDRAG - HDITEM.iOrder = %d\n",
				phdi->iOrder);

			// Column no. <s_iDragCol> becomes column no. <phdi->iOrder>.
			// Since we don't let the list control itself handle the order of the
			// columns but store the indexes in CDiscoveryDoc::m_ReportColumns[],
			// we post the WM_REPORT_COLUMNORDERCHANGED message to completely
			// re-build the list.
			CDiscoveryView* pView = (CDiscoveryView*)GetParent();
			ASSERT_KINDOF( CDiscoveryView, pView );

			if (s_iDragCol != phdi->iOrder)
			{
				pView->RegisterColumnMove(s_iDragCol, phdi->iOrder);
				pView->PostMessage(WM_REPORT_COLUMNORDERCHANGED);
			};

			return TRUE;
		}
	};

	// wParam is zero when the notification message comes from the header control
	if (wParam == 0)
	{
		// Right click on header control
		if (pNH->code == NM_RCLICK)
		{
			// Determine column number
			CPoint ptScreen(GetMessagePos());
			CPoint pt(ptScreen);
			CHeaderCtrl* pHeader = GetHeaderCtrl();
			HWND hwndHeader		 = pHeader->GetSafeHwnd();
			pHeader->ScreenToClient(&pt);

			// Determine the column index.
			// Since Header_GetItemRect() does not work with older versions of
			// COMCTL32.DLL, we use a self-defined function: GetHeaderItemIndex().
			int index = GetHeaderItemIndex(pt.x);
			if (index < 0)
				return FALSE;
/*			int index = -1;
			CRect rcCol;
			for (int i=0; Header_GetItemRect(hwndHeader, i, &rcCol); i++)
			{
				if ( rcCol.PtInRect(pt) )
				{
					index = i;
					break;
				}
			};	*/

			TRACE("CReportCtrl::OnNotify() - Right click on column #%d\n", index);

			// Assure that list is active. (List may not be active when user works
			// with another application and then clicks with the right mouse button
			// on the Report list.)
			GetParentFrame()->ActivateFrame();

			// Load the special menu for header control
			CMenu Menu, *pSubMenu;
			Menu.LoadMenu(IDR_REPORT_HEADER);
			pSubMenu = Menu.GetSubMenu(0);

			// Inform parent view that the context menu has already been processed
			// when it receives the WM_CONTEXTMENU message.
			m_bProcessedContextMenu = TRUE;

			UINT uCmd = GetPopupMenuCmd(pSubMenu, ptScreen.x, ptScreen.y, AfxGetMainWnd());

			// Since the commands in this context menu are special commands for the list,
			// we do not forward the command to the parent view (via the application
			// main window) but handle it immediately here.
			if (uCmd != 0 && index >= 0)
			{
				CDiscoveryView* pView = (CDiscoveryView*)GetParent();
				ASSERT_KINDOF( CDiscoveryView, pView );

				if (uCmd == ID_REPORT_SELECT_COLUMN)
				{
					// Tell the parent view and the document, that from now on
					// the column with the index <index> will be the selected one.
					// (No sorting or re-building necessary.)
					m_iSel = index;
					pView->RegisterColumnSelection(index);
				}
				else	// ID_REPORT_SORT_ASC, ID_REPORT_SORT_DESC, ID_REPORT_SORT_NO
				{
					// If the sort order for this column is different from the
					// previous sort order, tell the parent view and the document,
					// that the sort order has changed, and the list must be
					// sorted and re-built.
					int iOldSort	= m_aiSort[index];
					m_aiSort[index] = ((uCmd == ID_REPORT_SORT_ASC)  ?  1 :
									   (uCmd == ID_REPORT_SORT_DESC) ? -1 : 0);
					if (m_aiSort[index] != iOldSort)
					{
						pView->RegisterSortOrder(index, m_aiSort[index]);
						pView->Rebuild();
					}
				};

				// In any case, redraw all items
				InvalidateRect(NULL);
				UpdateWindow();
			};

			return TRUE;
		}
	};

	return FALSE;
}

// OnColumnclick() registers a left mouse button click on the header.
// Depending on the position, this click selects a column or changes the sort order.
// (Right mouse button clicks on the header are caught by CReportCtrl::OnNotify(),
// right mouse button clicks inside the list by CReportCtrl::OnRclick().)

void CReportCtrl::OnColumnclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	int index				 = pNMListView->iSubItem;
	CHeaderCtrl* pHeader	 = GetHeaderCtrl();

	CPoint pt(GetMessagePos());
	pHeader->ScreenToClient(&pt);

	// Header_GetItemRect() does not work with older versions of COMCTL32.
	// For that case, we use a self-defined function: GetHeaderItemRect().
	CRect rcCol;
	VERIFY( GetHeaderItemRect(index, &rcCol) );
	//rcCol.SetRectEmpty();
	//Header_GetItemRect(pHeader->GetSafeHwnd(), index, &rcCol);

	//TRACE("CReportCtrl::OnColumnclick() - column #%d, pt.x=%d, rcCol={%d..%d}\n",
	//	index, pt.x, rcCol.left, rcCol.right);

	// Check if user has clicked the arrow symbol that indicates the sort order.
	// The arrow symbol occupies six times the width of a space character of the
	// current font. If clicked, increment the sort order, which means:
	//   0 ->  1: Sort ascending when no sorting before.
	//   1 -> -1: Sort descending when sorting ascending before.
	//  -1 ->  0: No sorting when sorting descending before.
	CDC* pDC = pHeader->GetDC();
	ASSERT( pDC != NULL );

	CDiscoveryView* pView = (CDiscoveryView*)GetParent();
	ASSERT_KINDOF( CDiscoveryView, pView );

	CSize sizeBlank = pDC->GetTextExtent(" ");
	int iwidth		= sizeBlank.cx * 6;
	if (pt.x >= rcCol.right-iwidth && pt.x < rcCol.right)
	{
		TRACE("CReportCtrl::OnColumnclick() - arrow symbol matched\n");

		m_aiSort[index]++;
		if (m_aiSort[index] > 1)
			m_aiSort[index] = -1;

		// List must be sorted and re-built. (Compare OnNotify().)
		pView->RegisterSortOrder(index, m_aiSort[index]);
		pView->Rebuild();
	}

	// When clicked outside the symbol, use this to select the column
	else
	{
		TRACE("CReportCtrl::OnColumnclick() - click outside arrow symbol\n");
		
		// (No sorting or re-building necessary)
		m_iSel = index;
		pView->RegisterColumnSelection(index);
	};

	// Redraw all items
	//RedrawItems(0, GetItemCount()-1);
	InvalidateRect(NULL);
	UpdateWindow();
	
	*pResult = 0;
}

// Called when the user clicks on an item or subitem with the right mouse button,
// but not when he clicks inside the header.
void CReportCtrl::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here

	TRACE("Entering CReportCtrl::OnRclick()\n");

	// See comments in CReportCtrl::OnNotify()
	GetParentFrame()->ActivateFrame();

	CMenu Menu, *pSubMenu;
	Menu.LoadMenu(IDR_REPORT_LIST);
	pSubMenu = Menu.GetSubMenu(0);

	m_bProcessedContextMenu = TRUE;

	CPoint ptScreen(GetMessagePos());
	UINT uCmd = GetPopupMenuCmd(pSubMenu, ptScreen.x, ptScreen.y, AfxGetMainWnd());
	if (uCmd != 0)
		AfxGetMainWnd()->PostMessage(WM_COMMAND, uCmd);
	
	*pResult = 0;
}

// Trying to set a special mouse cursor when it is over a triangle that indicates
// the sort order. Does not work, since the header control does not give the parent
// (the list control) a chance to process WM_SETCURSOR.
BOOL CReportCtrl::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
/*	CRect rectHeader;
	CPoint pt(GetMessagePos());
	CHeaderCtrl* pHeader = GetHeaderCtrl();
	pHeader->GetWindowRect(&rectHeader);
	if ( rectHeader.PtInRect(pt) )
	{
		TRACE("CReportCtrl::OnSetCursor() - point (%d,%d) inside header\n", pt.x, pt.y);
	};	*/
	
	return CListCtrl::OnSetCursor(pWnd, nHitTest, message);
}


/////////////////////////////////////////////////////////////////////////////
// CReportHeaderCtrl

CReportHeaderCtrl::CReportHeaderCtrl()
{
}

CReportHeaderCtrl::~CReportHeaderCtrl()
{
}


BEGIN_MESSAGE_MAP(CReportHeaderCtrl, CHeaderCtrl)
	//{{AFX_MSG_MAP(CReportHeaderCtrl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReportHeaderCtrl message handlers


/////////////////////////////////////////////////////////////////////////////
// Old sources...
/////////////////////////////////////////////////////////////////////////////

#if 0
	CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rcItem(lpDrawItemStruct->rcItem);	int nItem = lpDrawItemStruct->itemID;
	CImageList* pImageList;	// Save dc state	int nSavedDC = pDC->SaveDC();
	// Get item image and state info	LV_ITEM lvi;
	lvi.mask = LVIF_IMAGE | LVIF_STATE;	lvi.iItem = nItem;	lvi.iSubItem = 0;
	lvi.stateMask = 0xFFFF;		// get all state flags
	GetItem(&lvi);
	// Should the item be highlighted
	BOOL bHighlight =	((lvi.state & LVIS_DROPHILITED)
				|| ( (lvi.state & LVIS_SELECTED)					&& ((GetFocus() == this)
						|| (GetStyle() & LVS_SHOWSELALWAYS)						)					)				);
	// Get rectangles for drawing	CRect rcBounds, rcLabel, rcIcon;
	GetItemRect(nItem, rcBounds, LVIR_BOUNDS);
	GetItemRect(nItem, rcLabel, LVIR_LABEL);	GetItemRect(nItem, rcIcon, LVIR_ICON);
	CRect rcCol( rcBounds ); 	CString sLabel = GetItemText( nItem, 0 );
	// Labels are offset by a certain amount  
	// This offset is related to the width of a space character
	int offset = pDC->GetTextExtent(_T(" "), 1 ).cx*2;	CRect rcHighlight;
	CRect rcClient;	int nExt;	switch( m_nHighlight )	{	case 0: 
		nExt = pDC->GetOutputTextExtent(sLabel).cx + offset;		rcHighlight = rcLabel;
		if( rcLabel.left + nExt < rcLabel.right )
			rcHighlight.right = rcLabel.left + nExt;		break;	case 1:
		rcHighlight = rcBounds;		rcHighlight.left = rcLabel.left;		break;	case 2:
		GetClientRect(&rcClient);		rcHighlight = rcBounds;
		rcHighlight.left = rcLabel.left;		rcHighlight.right = rcClient.right;		break;
	default:		rcHighlight = rcLabel;	}
	// Draw bitmap in the background if one has been set
	if( m_bitmap.m_hObject != NULL )	{		CDC tempDC;
		tempDC.CreateCompatibleDC(pDC);		tempDC.SelectObject( &m_bitmap );
		GetClientRect(&rcClient);		CRgn rgnBitmap;		CRect rcTmpBmp( rcItem );		
		rcTmpBmp.right = rcClient.right;
		// We also need to check whether it is the last item
		// The update region has to be extended to the bottom if it is
		if( nItem == GetItemCount() - 1 )			rcTmpBmp.bottom = rcClient.bottom;
		rgnBitmap.CreateRectRgnIndirect(&rcTmpBmp);		pDC->SelectClipRgn(&rgnBitmap);
		rgnBitmap.DeleteObject();		
		if( pDC->GetDeviceCaps(RASTERCAPS) & RC_PALETTE && m_pal.m_hObject != NULL )
		{			pDC->SelectPalette( &m_pal, FALSE );			pDC->RealizePalette();		}
		CRect rcFirstItem;		GetItemRect(0, rcFirstItem, LVIR_BOUNDS);
		for( int i = rcFirstItem.left; i < rcClient.right; i += m_cxBitmap )
			for( int j = rcFirstItem.top; j < rcClient.bottom; j += m_cyBitmap )
				pDC->BitBlt( i, j, m_cxBitmap, m_cyBitmap, &tempDC, 								0, 0, SRCCOPY );
	}	// Draw the background color	if( bHighlight )	{
		pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
		pDC->FillRect(rcHighlight, &CBrush(::GetSysColor(COLOR_HIGHLIGHT)));	}
	else if( m_bitmap.m_hObject == NULL )
		pDC->FillRect(rcHighlight, &CBrush(::GetSysColor(COLOR_WINDOW)));	
	// Set clip region	rcCol.right = rcCol.left + GetColumnWidth(0);	CRgn rgn;
	rgn.CreateRectRgnIndirect(&rcCol);	pDC->SelectClipRgn(&rgn);
	rgn.DeleteObject();	// Draw state icon	if (lvi.state & LVIS_STATEIMAGEMASK)	{
		int nImage = ((lvi.state & LVIS_STATEIMAGEMASK)>>12) - 1;
		pImageList = GetImageList(LVSIL_STATE);		if (pImageList)		{
			pImageList->Draw(pDC, nImage,
				CPoint(rcCol.left, rcCol.top), ILD_TRANSPARENT);		}	}	
	// Draw normal and overlay icon	pImageList = GetImageList(LVSIL_SMALL);
	if (pImageList)	{		UINT nOvlImageMask=lvi.state & LVIS_OVERLAYMASK;
		pImageList->Draw(pDC, lvi.iImage, 			CPoint(rcIcon.left, rcIcon.top),
			(bHighlight?ILD_BLEND50:0) | ILD_TRANSPARENT | nOvlImageMask );	}		
	// Draw item label - Column 0	rcLabel.left += offset/2;
	rcLabel.right -= offset;
	pDC->DrawText(sLabel,-1,rcLabel,DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP 
				| DT_VCENTER | DT_END_ELLIPSIS);	// Draw labels for remaining columns
	LV_COLUMN lvc;	lvc.mask = LVCF_FMT | LVCF_WIDTH;
	if( m_nHighlight == 0 )		// Highlight only first column	{
		pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
		pDC->SetBkColor(::GetSysColor(COLOR_WINDOW));	}	
	rcBounds.right = rcHighlight.right > rcBounds.right ? rcHighlight.right :
							rcBounds.right;	rgn.CreateRectRgnIndirect(&rcBounds);
	pDC->SelectClipRgn(&rgn);				   
	for(int nColumn = 1; GetColumn(nColumn, &lvc); nColumn++)	{
		rcCol.left = rcCol.right;		rcCol.right += lvc.cx;
		// Draw the background if needed
		if( m_bitmap.m_hObject == NULL && m_nHighlight == HIGHLIGHT_NORMAL )
			pDC->FillRect(rcCol, &CBrush(::GetSysColor(COLOR_WINDOW)));
		sLabel = GetItemText(nItem, nColumn);		if (sLabel.GetLength() == 0)
			continue;		// Get the text justification
		UINT nJustify = DT_LEFT;
		switch(lvc.fmt & LVCFMT_JUSTIFYMASK)
		{
		case LVCFMT_RIGHT:
			nJustify = DT_RIGHT;
			break;
		case LVCFMT_CENTER:
			nJustify = DT_CENTER;
			break;
		default:
			break;
		}		rcLabel = rcCol;		rcLabel.left += offset;
		rcLabel.right -= offset;
		pDC->DrawText(sLabel, -1, rcLabel, nJustify | DT_SINGLELINE 
				| DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS);	}
	// Draw focus rectangle if item has focus
	if (lvi.state & LVIS_FOCUSED && (GetFocus() == this))
		pDC->DrawFocusRect(rcHighlight);
	// Restore dc
	pDC->RestoreDC( nSavedDC );
#endif

/////////////////////////////////////////////////////////////////////////////