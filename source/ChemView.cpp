// ChemView.cpp : implementation of the CChemView class
// Created: 25.01.99 - Updated: 25.01.99
// Ported to Windows 8.1 by Fulcrum.013 Mariupol, Ukraine 
// 17 september 2020

#include "stdafx.h"
#include "discovery.h"
#include "ChemView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChemView

IMPLEMENT_DYNCREATE(CChemView, CView)

CChemView::CChemView()
{
}

CChemView::~CChemView()
{
}


BEGIN_MESSAGE_MAP(CChemView, CView)
	//{{AFX_MSG_MAP(CChemView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChemView drawing

void CChemView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CChemView diagnostics

#ifdef _DEBUG
void CChemView::AssertValid() const
{
	CView::AssertValid();
}

void CChemView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CChemView message handlers
