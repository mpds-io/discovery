// QsdRunDlg.cpp : implementation file
// Created: 11.01.1999  -  Updated: 24.03.2001
// Ported to Windows 8.1 by Fulcrum.013 Mariupol, Ukraine 
// 17 september 2020

#define  _CRT_SECURE_NO_WARNINGS 
#include "stdafx.h"
#include "discovery.h"
#include "DiscoveryDoc.h"
#include "MainFrm.h"
#include "QsdRunDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CQsdSelectRunPage property page

IMPLEMENT_DYNCREATE(CQsdSelectRunPage, CPropertyPage)

CQsdSelectRunPage::CQsdSelectRunPage() : CPropertyPage(CQsdSelectRunPage::IDD)
{
	//{{AFX_DATA_INIT(CQsdSelectRunPage)
	m_iRunType = -1;
	m_iLevel = -1;
	//}}AFX_DATA_INIT
}

CQsdSelectRunPage::~CQsdSelectRunPage()
{
}

void CQsdSelectRunPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQsdSelectRunPage)
	DDX_Radio(pDX, IDC_TYPE1, m_iRunType);
	DDX_Radio(pDX, IDC_LEVEL1, m_iLevel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQsdSelectRunPage, CPropertyPage)
	//{{AFX_MSG_MAP(CQsdSelectRunPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQsdSelectRunPage message handlers

BOOL CQsdSelectRunPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	// MFC makes the property sheet larger than needed and places the wizard
	// button at the right margin of the sheet. For a better look, we move the
	// buttons to the left margin and reduce the sheet's width.
	// See MFC Programmer's Source Book: "Creating a Wizard"
	// (http://www.codeguru.com/propertysheet/creating_a_wizard.shtml),
	// by Peter Pearson, ppearson@broad.demon.co.uk

	// Move the buttons to the left margin of the property sheet and make the
	// property sheet a little bit smaller.
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	ASSERT_KINDOF( CPropertySheet, pSheet );

	CWnd* pBack		= pSheet->GetDlgItem(ID_WIZBACK);
	CWnd* pNext		= pSheet->GetDlgItem(ID_WIZNEXT);
	CWnd* pFinish	= pSheet->GetDlgItem(ID_WIZFINISH);
	CWnd* pCancel	= pSheet->GetDlgItem(IDCANCEL);

	// Take the current horizontal distance between Back button and left dialog edge
	// as shift increment of all buttons, but leave 12 dialog units space between the
	// left dialog edge and the first button.

	// Convert 12 horizontal dialog box units to pixels
	int ixofs = (LOWORD(::GetDialogBaseUnits()) * 12) / 4;

	CRect rectBack, rectNext, rectFinish, rectCancel;
	pBack->GetWindowRect(&rectBack);
	pSheet->ScreenToClient(&rectBack);

	// Pixels to move horizontally (will be negative, so shift to the left)
	int ixmove = -rectBack.left + ixofs;

	// Now apply shift to the Back and the other buttons
	rectBack.OffsetRect(ixmove, 0);
	pBack->MoveWindow(&rectBack);

	pNext->GetWindowRect(&rectNext);
	pSheet->ScreenToClient(&rectNext);
	rectNext.OffsetRect(ixmove, 0);
	pNext->MoveWindow(&rectNext);

	pFinish->GetWindowRect(&rectFinish);
	pSheet->ScreenToClient(&rectFinish);
	rectFinish.OffsetRect(ixmove, 0);
	pFinish->MoveWindow(&rectFinish);

	pCancel->GetWindowRect(&rectCancel);
	pSheet->ScreenToClient(&rectCancel);
	rectCancel.OffsetRect(ixmove, 0);
	pCancel->MoveWindow(&rectCancel);

	// The pages have a maximum width of 256 dialog box units. So make the property sheet
	// 256 units wide.
	CRect rectSheet;
	pSheet->GetWindowRect(&rectSheet);
	rectSheet.right = rectSheet.left + (LOWORD(::GetDialogBaseUnits()) * 256) / 4;
	pSheet->MoveWindow(&rectSheet);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CQsdSelectRunPage::OnSetActive() 
{
	BOOL bOk = CPropertyPage::OnSetActive();

	// Since this page is the first page, disable the Back button
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	ASSERT_KINDOF( CPropertySheet, pSheet );

	pSheet->SetWizardButtons(PSWIZB_NEXT);

	return bOk;
}

BOOL CQsdSelectRunPage::OnKillActive() 
{
	TRACE("CQsdSelectRunPage::OnKillActive() called\n");

	// This checks if the setting "Single Run" (or "Prediction") has changed to
	// "Search" (which is a multiple run) or vice versa. In that case replace
	// the second page of the wizard.

	// Store the old content of the radio buttons to compare after DoDataExchange() is done
	int iOldType = m_iRunType;

	// OnKillActive() calls CPropertyPage::DoDataExchange()
	BOOL bOk = CPropertyPage::OnKillActive();

	// If DoDataExchange() was successfull, replace the second page of the wizard
	if ( bOk && iOldType != m_iRunType)
	{
		CQsdRunSheet* pSheet = (CQsdRunSheet*)GetParent();
		ASSERT_KINDOF( CQsdRunSheet, pSheet );

		// Since there is no function like CPropertySheet::InsertPage() to replace
		// page #1, we first remove all pages including page #1, then add page #1
		// and finally add page #2.
		pSheet->RemovePage(2);
		pSheet->RemovePage(1);
		if (m_iRunType == 0)
			pSheet->AddPage(&pSheet->m_MultPage);
		else
		{
			pSheet->AddPage(&pSheet->m_SinglePage);
			pSheet->m_SinglePage.m_bPredict = (m_iRunType == 2);
		};
		pSheet->AddPage(&pSheet->m_ConfPage);
	}

	return bOk;
}

/////////////////////////////////////////////////////////////////////////////
// CQsdSingleRunPage property page

IMPLEMENT_DYNCREATE(CQsdSingleRunPage, CPropertyPage)

CQsdSingleRunPage::CQsdSingleRunPage() : CPropertyPage(CQsdSingleRunPage::IDD)
{
	//{{AFX_DATA_INIT(CQsdSingleRunPage)
	m_iPropX = -1;
	m_iPropY = -1;
	m_iPropZ = -1;
	m_iOpX = -1;
	m_iOpY = -1;
	m_iOpZ = -1;
	m_strFilename = _T("");
	//}}AFX_DATA_INIT

	m_bPredict = FALSE;
}

CQsdSingleRunPage::~CQsdSingleRunPage()
{
}

void CQsdSingleRunPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQsdSingleRunPage)
	DDX_Control(pDX, IDC_OP_Z, m_OpZ);
	DDX_Control(pDX, IDC_OP_Y, m_OpY);
	DDX_Control(pDX, IDC_OP_X, m_OpX);
	DDX_Control(pDX, IDC_PROP_Z, m_PropZ);
	DDX_Control(pDX, IDC_PROP_Y, m_PropY);
	DDX_Control(pDX, IDC_PROP_X, m_PropX);
	DDX_CBIndex(pDX, IDC_PROP_X, m_iPropX);
	DDX_CBIndex(pDX, IDC_PROP_Y, m_iPropY);
	DDX_CBIndex(pDX, IDC_PROP_Z, m_iPropZ);
	DDX_CBIndex(pDX, IDC_OP_X, m_iOpX);
	DDX_CBIndex(pDX, IDC_OP_Y, m_iOpY);
	DDX_CBIndex(pDX, IDC_OP_Z, m_iOpZ);
	DDX_Text(pDX, IDC_FILENAME, m_strFilename);
	//}}AFX_DATA_MAP
}

// Write elemental properties into combo box <pCombo>.
// Returns TRUE if successfull, FALSE if file open error.
BOOL CQsdSingleRunPage::InitProperties (CComboBox* pCombo)
{
	CDiscoveryDoc* pDoc = (CDiscoveryDoc*)((CMainFrame*)AfxGetMainWnd())->GetActiveDocument();
	ASSERT_VALID(pDoc);

	int imax = pDoc->m_astrQsdProperties.GetSize();
	if (imax <= 0)
		return FALSE;

	for (register int i=0; i < imax; i++)
		pCombo->AddString(pDoc->m_astrQsdProperties[i]);

	return TRUE;
};

// Write operations (Sum, Ratio, etc.) into combo box <pCombo>.
// Returns TRUE.
BOOL CQsdSingleRunPage::InitOperations (CComboBox* pCombo)
{
	CString str;
	for (register int i=1; i <= 7; i++)
		pCombo->AddString(GetQsdOperationString(str, i));

//	CDiscoveryDoc* pDoc = (CDiscoveryDoc*)((CMainFrame*)AfxGetMainWnd())->GetActiveDocument();
//	ASSERT_VALID(pDoc);

//	pCombo->AddString(CString((LPCSTR)IDS_SUM));			// index = 1
//	pCombo->AddString(CString((LPCSTR)IDS_DIFFERENCE));		// 2
//	pCombo->AddString(CString((LPCSTR)IDS_RATIO));			// 3
//	pCombo->AddString(CString((LPCSTR)IDS_PRODUCT));		// 4
//	pCombo->AddString(CString((LPCSTR)IDS_MAXIMUM));		// 5

	return TRUE;
};

// Checks if there are three different properties selected for the three axes
// and if every combo box has a selection. Otherwise the Next button keeps disabled.
// Returns TRUE if Next button is enabled or FALSE if disabled.
BOOL CQsdSingleRunPage::CheckSelections (void)
{
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	ASSERT_KINDOF( CPropertySheet, pSheet );

	UpdateData(TRUE);

	BOOL bEnable;
/*	if (m_iPropY == 0 && m_iOpY == 0 && m_iPropZ == 0 && m_iOpZ == 0)
		bEnable = (m_iPropX != -1 && m_iOpX != -1);
	else */ if (m_iPropZ == 0 && m_iOpZ == 0)
		bEnable = (	m_iPropX != -1 && m_iOpX != -1 && m_iPropY != -1 && m_iOpY != -1 &&
					((m_iPropX != m_iPropY) || (m_iOpX != m_iOpY)) );
	else
		bEnable = (	m_iPropX != -1 && m_iPropY != -1 && m_iPropZ != -1 &&
					m_iOpX   != -1 && m_iOpY   != -1 && m_iOpZ   != -1 &&
					((m_iPropX != m_iPropY) || (m_iOpX != m_iOpY))     &&
					((m_iPropX != m_iPropZ) || (m_iOpX != m_iOpZ))     &&
					((m_iPropY != m_iPropZ) || (m_iOpY != m_iOpZ)) );

	// In prediction mode, a file name must have been specified
	if ( m_bPredict && m_strFilename.IsEmpty() )
		bEnable = FALSE;

	if ( bEnable )
		pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	else
		pSheet->SetWizardButtons(PSWIZB_BACK);

	return bEnable;
};

BEGIN_MESSAGE_MAP(CQsdSingleRunPage, CPropertyPage)
	//{{AFX_MSG_MAP(CQsdSingleRunPage)
	ON_CBN_SELCHANGE(IDC_PROP_X, OnSelchangePropX)
	ON_CBN_SELCHANGE(IDC_PROP_Y, OnSelchangePropY)
	ON_CBN_SELCHANGE(IDC_PROP_Z, OnSelchangePropZ)
	ON_CBN_SELCHANGE(IDC_OP_X, OnSelchangeOpX)
	ON_CBN_SELCHANGE(IDC_OP_Y, OnSelchangeOpY)
	ON_CBN_SELCHANGE(IDC_OP_Z, OnSelchangeOpZ)
	ON_BN_CLICKED(IDC_LOAD, OnLoad)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQsdSingleRunPage message handlers

BOOL CQsdSingleRunPage::OnSetActive() 
{
	BOOL bOk = CPropertyPage::OnSetActive();

	// Since this is the second page, enable both Back and Next button,...
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	ASSERT_KINDOF( CPropertySheet, pSheet );

	// Show "Load file" button only in prediction mode
	GetDlgItem(IDC_LOAD)->ShowWindow( m_bPredict );
	GetDlgItem(IDC_FILENAME)->ShowWindow( m_bPredict );

	// ...but Next button only if all combo boxes have selections
	BOOL bFile = ((m_bPredict && m_strFilename.IsEmpty()) ? FALSE : TRUE);
	if (m_PropX.GetCurSel() >= 0 && m_PropY.GetCurSel() >= 0 && m_PropZ.GetCurSel() >= 0 &&
		m_OpX.GetCurSel()   >= 0 && m_OpY.GetCurSel()   >= 0 && m_OpZ.GetCurSel()   >= 0 &&
		bFile )
		pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
	else
		pSheet->SetWizardButtons(PSWIZB_BACK);

	return bOk;
}

BOOL CQsdSingleRunPage::OnKillActive() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CPropertyPage::OnKillActive();
}

BOOL CQsdSingleRunPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// OnInitDialog() is called once for the duration of the wizard (when the
	// user has selected "Single Run" and has clicked the "Next" button).

	// Fill the combo boxes with the elemental properties and the operations, rsp.
	// For a two-dimensional run, we add the option "<None>" to the z-property
	// and pre-select it, that means we pre-select two-dimensional Discovery Space.
	InitProperties(&m_PropX);
	//m_PropY.AddString("<none>");	// that was for 1-dimensional search
	InitProperties(&m_PropY);
	m_PropZ.AddString("<none>");
	InitProperties(&m_PropZ);

	InitOperations(&m_OpX);
	//m_OpY.AddString("<none>");	// ditto
	InitOperations(&m_OpY);
	m_OpZ.AddString("<none>");
	InitOperations(&m_OpZ);

	//m_PropY.SetCurSel(0);			// ditto
	m_PropZ.SetCurSel(0);
	//m_OpY.SetCurSel(0);			// ditto
	m_OpZ.SetCurSel(0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CQsdSingleRunPage::OnWizardNext() 
{
	// Check if there are different indexes for the property indexes
/*	UpdateData(TRUE);
	if (m_iPropX < 0 || m_iPropY < 0 || m_iPropZ < 0 ||	m_iOpX < 0 || m_iOpY < 0 || m_iOpZ < 0)
		return -1;
	if ((m_iPropX == m_iPropY && m_iOpX == m_iOpY) ||
		(m_iPropX == m_iPropY && m_iOpX == m_iOpY) ||
		(m_iPropX == m_iPropY && m_iOpX == m_iOpY))
		return -1;	*/
	
	return CPropertyPage::OnWizardNext();
}

void CQsdSingleRunPage::OnSelchangePropX() 
{
	CheckSelections();
}

void CQsdSingleRunPage::OnSelchangePropY() 
{
	CheckSelections();
}

void CQsdSingleRunPage::OnSelchangePropZ() 
{
	CheckSelections();
}

void CQsdSingleRunPage::OnSelchangeOpX() 
{
	CheckSelections();
}

void CQsdSingleRunPage::OnSelchangeOpY() 
{
	CheckSelections();
}

void CQsdSingleRunPage::OnSelchangeOpZ() 
{
	CheckSelections();
}

// Open a file with compounds whose properties are to be predicted
void CQsdSingleRunPage::OnLoad() 
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY,
					"Property files (*.dat;*.qsd)|*.dat;*.qsd|All files (*.*)|*.*||");
	if (dlg.DoModal() == IDOK)
	{
		m_strFilename = dlg.GetPathName();

		// Make a simple check, if the file has the right input format
		CStdioFile f;
		CString strLine, strMsg;
		if ( !f.Open(m_strFilename, CFile::modeRead | CFile::typeText | CFile::shareDenyNone) )
		{
			strMsg.Format("File Error!\nCannot open the file %s !", m_strFilename);
			AfxMessageBox(strMsg);
			return;
		};
		f.ReadString(strLine);		// first line must be "QSD"
		if (strLine.CompareNoCase("QSD") != 0)
		{
			strMsg.Format("Syntax Error!\n%s is no input file for QSD/Discovery!",
						  m_strFilename);
			AfxMessageBox(strMsg);
			return;
		};
		f.ReadString(strLine);		// second line contains number of compounds
		if (atoi(strLine) <= 0)
		{
			strMsg.Format("Syntax Error!\nNumber of compounds not given correctly in input file\n%s",
						  m_strFilename);
			AfxMessageBox(strMsg);
			return;
		};
		f.Close();

		SetDlgItemText(IDC_FILENAME, m_strFilename);

		CheckSelections();
	}
}

/////////////////////////////////////////////////////////////////////////////
// CQsdMultipleRunPage property page

IMPLEMENT_DYNCREATE(CQsdMultipleRunPage, CPropertyPage)

CQsdMultipleRunPage::CQsdMultipleRunPage() : CPropertyPage(CQsdMultipleRunPage::IDD)
{
	//{{AFX_DATA_INIT(CQsdMultipleRunPage)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nSelProps	= 0;
	m_nSelOps	= 0;
	m_pSelProps	= NULL;
	m_pSelOps	= NULL;
}

CQsdMultipleRunPage::~CQsdMultipleRunPage()
{
	if (m_pSelProps != NULL)
		delete m_pSelProps;
	if (m_pSelOps != NULL)
		delete m_pSelOps;
}

void CQsdMultipleRunPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQsdMultipleRunPage)
	DDX_Control(pDX, IDC_RUNS, m_Runs);
	DDX_Control(pDX, IDC_OP, m_Op);
	DDX_Control(pDX, IDC_PROP, m_Prop);
	//}}AFX_DATA_MAP
}

// Calculate the total number of runs from the numbers of selected properties and operations
void CQsdMultipleRunPage::UpdateTotalRuns (void)
{
	int nProp	= m_Prop.GetSelCount();
	int nOp		= m_Op.GetSelCount();
	int nTotal	= nProp * nOp;
	if (nProp >= 1 && nOp >= 1)
		nTotal	= (nTotal * (nTotal - 1) * (nTotal - 2) / 6) +
				  (nTotal * (nTotal - 1) / 2)	/*			 +
				   nTotal	*/ ;
	else
		nTotal	= 0;
	SetDlgItemInt(IDC_RUNS, nTotal);

	// "Next" button will be disabled unless at least one property and at least
	// one operation have been selected.
	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	ASSERT_KINDOF( CPropertySheet, pSheet );
	pSheet->SetWizardButtons((nTotal > 0) ? PSWIZB_BACK | PSWIZB_NEXT : PSWIZB_BACK);
};

BEGIN_MESSAGE_MAP(CQsdMultipleRunPage, CPropertyPage)
	//{{AFX_MSG_MAP(CQsdMultipleRunPage)
	ON_LBN_SELCHANGE(IDC_PROP, OnSelchangeProp)
	ON_LBN_SELCHANGE(IDC_OP, OnSelchangeOp)
	ON_BN_CLICKED(IDC_ALL, OnAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQsdMultipleRunPage message handlers

BOOL CQsdMultipleRunPage::OnKillActive() 
{
	// Store the indexes of the selected properties and operations
	if (m_pSelProps != NULL)
		delete m_pSelProps;
	if (m_pSelOps != NULL)
		delete m_pSelOps;
	m_nSelProps	= m_Prop.GetSelCount();
	m_nSelOps	= m_Op.GetSelCount();
	if (m_nSelProps > 0)
	{
		m_pSelProps = new int[m_nSelProps];
		m_Prop.GetSelItems(m_nSelProps, m_pSelProps);
	};
	if (m_nSelOps > 0)
	{
		m_pSelOps = new int[m_nSelOps];
		m_Op.GetSelItems(m_nSelOps, m_pSelOps);
	};
	
	return CPropertyPage::OnKillActive();
}

BOOL CQsdMultipleRunPage::OnSetActive() 
{
	BOOL bOk = CPropertyPage::OnSetActive();

	CPropertySheet* pSheet = (CPropertySheet*)GetParent();
	ASSERT_KINDOF( CPropertySheet, pSheet );

	// Update the total number of runs. This depends on how many elemental
	// properties and operations are selected.
	// This function also enables or disables the Next button.
	UpdateTotalRuns();

	return bOk;
}

BOOL CQsdMultipleRunPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// Fill the list boxes with the elemental properties and the operations
	CDiscoveryDoc* pDoc = (CDiscoveryDoc*)((CMainFrame*)AfxGetMainWnd())->GetActiveDocument();
	ASSERT_VALID(pDoc);

	int imax = pDoc->m_astrQsdProperties.GetSize();
	for (register int i=0; i < imax; i++)
		m_Prop.AddString(pDoc->m_astrQsdProperties[i]);

	m_Op.AddString(CString((LPCSTR)IDS_SUM));			// index = 1
	m_Op.AddString(CString((LPCSTR)IDS_DIFFERENCE));	// 2
	m_Op.AddString(CString((LPCSTR)IDS_RATIO));			// 3
	m_Op.AddString(CString((LPCSTR)IDS_PRODUCT));		// 4
	m_Op.AddString(CString((LPCSTR)IDS_PRODUCT2));		// 5
	m_Op.AddString(CString((LPCSTR)IDS_MAXIMUM));		// 6
	m_Op.AddString(CString((LPCSTR)IDS_MINIMUM));		// 7

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CQsdMultipleRunPage::OnSelchangeProp() 
{
	// Recalculate the total number of runs from the numbers of selected
	// elemental properties and operations, rsp.
	UpdateTotalRuns();
}

void CQsdMultipleRunPage::OnSelchangeOp() 
{
	UpdateTotalRuns();
}

// Select all properties and operations when "All" button has been pressed
void CQsdMultipleRunPage::OnAll() 
{
	m_Prop.SetSel(-1, TRUE);
	m_Op.SetSel(-1, TRUE);
	UpdateTotalRuns();
}


/////////////////////////////////////////////////////////////////////////////
// CQsdConfPage property page

IMPLEMENT_DYNCREATE(CQsdConfPage, CPropertyPage)

CQsdConfPage::CQsdConfPage() : CPropertyPage(CQsdConfPage::IDD)
{
	//{{AFX_DATA_INIT(CQsdConfPage)
	m_iMax = 0;
	m_iNum = 0;
	m_bError = FALSE;
	m_iQuality = 0;
	//}}AFX_DATA_INIT
}

CQsdConfPage::~CQsdConfPage()
{
}

void CQsdConfPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQsdConfPage)
	DDX_Control(pDX, IDC_SPIN3, m_Spin3);
	DDX_Control(pDX, IDC_SPIN2, m_Spin2);
	DDX_Control(pDX, IDC_SPIN1, m_Spin1);
	DDX_Text(pDX, IDC_MAX, m_iMax);
	DDV_MinMaxInt(pDX, m_iMax, 1, 100);
	DDX_Text(pDX, IDC_NUM, m_iNum);
	DDV_MinMaxInt(pDX, m_iNum, 1, 100);
	DDX_Check(pDX, IDC_ERROR, m_bError);
	DDX_Text(pDX, IDC_QUALITY, m_iQuality);
	DDV_MinMaxInt(pDX, m_iQuality, 0, 100);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQsdConfPage, CPropertyPage)
	//{{AFX_MSG_MAP(CQsdConfPage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQsdConfPage message handlers

BOOL CQsdConfPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	// Set ranges for the spin buttons
	m_Spin1.SetRange(0,100);	// number of best results
	m_Spin2.SetRange(0,100);	// maximum neighbours
	m_Spin3.SetRange(0,100);	// minimum quality (percent)

	// Hide the checkbox for "Neighbour error", since we presumably do not more use it.
	GetDlgItem(IDC_ERROR)->ShowWindow(SW_HIDE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CQsdConfPage::OnSetActive() 
{
	BOOL bOk = CPropertyPage::OnSetActive();

	// Since this is the third and last page, enable both Back and Finish button
	CQsdRunSheet* pSheet = (CQsdRunSheet*)GetParent();
	ASSERT_KINDOF( CQsdRunSheet, pSheet );

	pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_FINISH);

	// Disable the input fields for the "Number of best results" and "Minimum quality"
	// for single run.
	int iRunType = pSheet->m_SelPage.m_iRunType;
	if (iRunType == 0)							// fast scan (0)
	{
		GetDlgItem(IDC_MAX)->EnableWindow(FALSE);
		GetDlgItem(IDC_NUM)->EnableWindow(FALSE);
		GetDlgItem(IDC_QUALITY)->EnableWindow(FALSE);
	}
	else if (iRunType == 1)						// precise scan
	{
		GetDlgItem(IDC_QUALITY)->EnableWindow(FALSE);
	}											// iRunType == 2 -> all enabled
	else if (iRunType == 3 || iRunType == 4)	// single run (3) or prediction (4)
	{
		GetDlgItem(IDC_NUM)->EnableWindow(FALSE);
		GetDlgItem(IDC_QUALITY)->EnableWindow(FALSE);
	};

	return bOk;
}

BOOL CQsdConfPage::OnWizardFinish() 
{
	// Call of OnWizardFinish() is necessary because UpdateData() is not called
	// when "Finish" button has been clicked (for whatever reason) !
	if ( !UpdateData(TRUE) )
		return FALSE;
	
	return CPropertyPage::OnWizardFinish();
}


/////////////////////////////////////////////////////////////////////////////
// CQsdRunSheet

IMPLEMENT_DYNAMIC(CQsdRunSheet, CPropertySheet)

CQsdRunSheet::CQsdRunSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CQsdRunSheet::CQsdRunSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CQsdRunSheet::~CQsdRunSheet()
{
}


BEGIN_MESSAGE_MAP(CQsdRunSheet, CPropertySheet)
	//{{AFX_MSG_MAP(CQsdRunSheet)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQsdRunSheet message handlers


/////////////////////////////////////////////////////////////////////////////
// CQsdProgressDlg dialog

CQsdProgressDlg::CQsdProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQsdProgressDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQsdProgressDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nRuns = 0;
	m_nBests = 0;
	m_hProcess = 0;
	m_hTimer = 0;
	m_iCycle = 1;
}


void CQsdProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQsdProgressDlg)
	DDX_Control(pDX, IDC_WHAT, m_Status);
	DDX_Control(pDX, IDC_RESULT, m_Result);
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQsdProgressDlg, CDialog)
	//{{AFX_MSG_MAP(CQsdProgressDlg)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQsdProgressDlg message handlers

BOOL CQsdProgressDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Create the working thread for the QSD kernel. The executable QSD.EXE
	// is in the directory <CDiscoveryApp::m_strQsdExePath>.
	// The creation of a working has partially been inspired by:
	// Joerg Koenig, Joerg.Koenig@rhein-neckar.de.
	// From: http://www.codeguru.com/misc/process.shtml.

	CDiscoveryApp* pApp = (CDiscoveryApp*)AfxGetApp();
	char szExe[_MAX_PATH], szDir[_MAX_PATH];
	strcpy(szDir, pApp->m_strQsdExePath);	// working directory is QSD.EXE's directory
	strcpy(szExe, szDir);
	strcat(szExe, "QSD.EXE");				// complete path of the executable
	
	m_strProgressFileName  = szDir;
	m_strProgressFileName += "PROGRESS.DAT";	// path of the progress info file
	DeleteFile(m_strProgressFileName);			// delete if not erased by QSD.EXE

	char *cp;
	if ((cp = strrchr(szDir, '\\')) != NULL)	// remove final backslash from directory
		*cp = '\0';

	// This will receive the handle of the working thread
	PROCESS_INFORMATION ProcessInfo;

	// Define in the startup info that the console window of QSD.EXE shall
	// be invisible and shall not appear in the task bar of Windows 95/98/NT
	STARTUPINFO StartupInfo;
	memset(&StartupInfo, 0, sizeof(STARTUPINFO));
	StartupInfo.cb			= sizeof(STARTUPINFO);
	StartupInfo.dwFlags		= STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow	= SW_HIDE;

	// Now we create the process. For a console application, we must set the
	// pointer to the executable's file name NULL but put both exe file name
	// and arguments (which we don't really use here) into the command line pointer.
	BOOL bStat = CreateProcess(
						NULL,					// pointer to name of executable module 
						szExe,					// pointer to command line string 
						0,						// pointer to process security attributes 
						0,						// pointer to thread security attributes 
						TRUE,					// handle inheritance flag 
						0,						// creation flags 
						0,						// pointer to new environment block 
						szDir,					// working directory
						&StartupInfo,			// pointer to STARTUPINFO 
						&ProcessInfo);			// pointer to PROCESS_INFORMATION 

	ASSERT( bStat );

	TRACE("CQsdProgressDlg::OnInitDialog() - ProcessInfo: hProcess=%d, hThread=%d, dwProcessId=%u, dwThreadId=%u\n",
		ProcessInfo.hProcess, ProcessInfo.hThread, ProcessInfo.dwProcessId, ProcessInfo.dwThreadId);

	m_hProcess = ProcessInfo.hProcess;

	CloseHandle(ProcessInfo.hThread);

	// Unless we get the first feedback from the QSD kernel, we set the progress bar
	// etc. to zero or initial values, rsp. The progress file will tell us the
	// number of steps for a call to CProgressCtrl::SetRange().
	m_Progress.SetPos(0);

	// First step: "Preparing data...", second step: "Analyzing data..."
	CString str((LPCSTR)IDS_PREPARING_DATA);
	m_Status.SetWindowText(str);
	m_iCycle = 1;

	// Before we get the number of runs, we display "Initializing..."
	str.LoadString(IDS_INITIALIZING);
	m_Result.SetWindowText(str);
	//CString strResult;
	//strResult.Format(str, 0, m_nRuns, 0.0f);

	// Start a timer that will check the contents of the file "PROGRESS.DAT"
	// unless the process is terminated. OnTimer() will repeatedly update the
	// progress bar etc.
	m_hTimer = SetTimer(1, 250, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CQsdProgressDlg::OnTimer(UINT_PTR nIDEvent) 
{
	// First check if the QSD kernel process is still running.
	// If yes, check the exit code and end this dialog with IDOK to signal the
	// main program that DoModal() has not been cancelled by the user.
	DWORD dwExitCode;
	VERIFY( GetExitCodeProcess(m_hProcess, &dwExitCode) );
	if (dwExitCode == STILL_ACTIVE)
	{
		// The QSD process is still running. So try to open the file "PROGRESS.DAT"
		// and extract the necessary informations. When the file is just being opened
		// by QSD.EXE, wait a little bit and try again.
		CStdioFile f;
		for (register int nTry=3; nTry > 0; nTry--)
		{
			if ( !f.Open(m_strProgressFileName, CFile::modeRead | CFile::typeText | CFile::shareDenyNone) )
			{
				Sleep(50);		// 0.05 seconds to wait
				continue;
			};

			// When running multiple runs, the QSD kernel writes the following
			// informations into a file "progress.dat" containig one line like this:
			//   10,100,1,94.12
			// which means: run 10 of 100 total runs, cycle no. 1 (first, raw run;
			// cycle no. 2 means exact run with best results), 94.12: best result in
			// percent up to now.
			CString strLine, strResult, strFmt;
			float fPercent;
			int iStep, nSteps, iCycle;
			if ( f.ReadString(strLine) )
			{
				if (sscanf(strLine, "%d,%d,%d,%f", &iStep, &nSteps, &iCycle, &fPercent) == 4)
				{
					// Switch to "Analyzing..." above the progress bar when 2nd cycle begins
					// and update the progress bar's scroll range.
					if (iCycle > m_iCycle)
					{
						strResult.LoadString(IDS_ANALYZING_DATA);
						m_Status.SetWindowText(strResult);
						m_iCycle = iCycle;
						m_Progress.SetPos(0);
						m_Progress.SetRange(0, nSteps);
					};
					m_Progress.SetRange(0, nSteps);
					m_Progress.SetPos(iStep);

					// Percentage of best result keeps undefined (-1), unless
					// minimum quality has been reached.
					if (fPercent >= 0.0f)
					{
						strFmt.LoadString(IDS_RUN_X_OF_Y_BEST_RESULT_PERCENT);
						strResult.Format(strFmt, iStep, nSteps, fPercent);
					}
					else
					{
						strFmt.LoadString(IDS_RUN_X_OF_Y);
						strResult.Format(strFmt, iStep, nSteps);
					};
					m_Result.SetWindowText(strResult);

					nTry = 0;	// success, don't try again in this timer interrupt
				}
			}

			f.Close();
		}
	}

	else	// dwExitCode != STILL_ACTIVE
	{
		TRACE("CQsdProgressDlg::OnTimer() - dwExitCode=%u\n", dwExitCode);

		VERIFY( CloseHandle(m_hProcess) );

		KillTimer(m_hTimer);

		if (dwExitCode == 1)	// compare <CMainFrame::OnQsdStart()>
		{
			EndDialog(IDCANCEL);
			ReportQsdExeError();
		}
		else
		{
			EndDialog(IDOK);
		}
	};
	
	CDialog::OnTimer(nIDEvent);
}

void CQsdProgressDlg::OnCancel() 
{
	// To cancel the QSD process, we simply write an empty file named "STOP."
	// QSD.EXE will stop when it encounters an existing file with that name.
	CDiscoveryApp* pApp = (CDiscoveryApp*)AfxGetApp();
	CFile f;
	CFileException e;
	CString strStopPath	= pApp->m_strQsdExePath;
	strStopPath		   += "STOP.";
	if ( !f.Open(strStopPath, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary, &e) )
	{
		ReportFileError(&e);
		return;
	};

	f.Close();

	// Wait until QSD kernel has really terminated
	DWORD dwExitCode;
	WaitForSingleObject(m_hProcess, INFINITE);
	GetExitCodeProcess(m_hProcess, &dwExitCode);
	VERIFY( CloseHandle(m_hProcess) );
	m_hProcess = 0;

	DeleteFile(strStopPath);

	// Since "cancel" can also be caused by an error from QSD.EXE, we display the
	// error message here rather than from where CQsdProgressDlg::DoModal() is called.
	AfxMessageBox(IDS_QSD_RUN_CANCELED);
	
	CDialog::OnCancel();
}


/////////////////////////////////////////////////////////////////////////////
// CQsdConfSingleDlg dialog

CQsdConfSingleDlg::CQsdConfSingleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQsdConfSingleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQsdConfSingleDlg)
	m_uMax = 0;
	m_bError = FALSE;
	//}}AFX_DATA_INIT
}

void CQsdConfSingleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQsdConfSingleDlg)
	DDX_Control(pDX, IDC_SPIN2, m_Spin);
	DDX_Text(pDX, IDC_MAX, m_uMax);
	DDV_MinMaxUInt(pDX, m_uMax, 0, 100);
	DDX_Check(pDX, IDC_ERROR, m_bError);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQsdConfSingleDlg, CDialog)
	//{{AFX_MSG_MAP(CQsdConfSingleDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQsdConfSingleDlg message handlers

BOOL CQsdConfSingleDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Set range for spin button control
	m_Spin.SetRange(0,50);		// maximum neighbours

	// Hide the checkbox "Neighbour error"
	GetDlgItem(IDC_ERROR)->ShowWindow(SW_HIDE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
/////////////////////////////////////////////////////////////////////////////
// CQsdImportDlg dialog


CQsdImportDlg::CQsdImportDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CQsdImportDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQsdImportDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CQsdImportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQsdImportDlg)
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_STATUS, m_Status);
	DDX_Control(pDX, IDC_WHAT, m_What);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQsdImportDlg, CDialog)
	//{{AFX_MSG_MAP(CQsdImportDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQsdImportDlg message handlers
