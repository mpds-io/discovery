/////////////////////////////////////////////////////////////////////////////
// Discovery - Program to display and encounter property dependencies from a
//             subset of entries of the Pauling File
//
// (C) 1999-2001 Crystal Impact, M.Berndt, K.Brandenburg & H.Putz GbR, Bonn, Germany
//     and: Materials Phases Data System, P.Villars, Vitznau, Switzerland
//
// Author: Klaus Brandenburg
//
// Created: 11.01.1999  -  Updated: 23.03.2001
// (Creation date and Update use the format dd.mm.yyyy in every source file.)
// Ported to Windows 8.1 by Fulcrum.013 Mariupol, Ukraine 
// 17 september 2020

//
// Implemented with Microsoft Visual C++ 4.2 using Microsoft Foundation Classes (MFC)
//
// The program displays a Report View of selected entries with selected database fields.
// ....

// Discovery.cpp : Defines the class behaviors for the application.
//
#define  _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "Discovery.h"

#include "MainFrm.h"
#include "DiscoveryDoc.h"
#include "DiscoveryView.h"

#include <direct.h>			// for _getcwd()

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryApp

BEGIN_MESSAGE_MAP(CDiscoveryApp, CWinApp)
	//{{AFX_MSG_MAP(CDiscoveryApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	//ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryApp construction

CDiscoveryApp::CDiscoveryApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CDiscoveryApp object

CDiscoveryApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryApp initialization

BOOL CDiscoveryApp::InitInstance()
{
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	// This application does not use an INI file (which would be named
	// "Discovery.ini") but uses the Registry to store data in
	// HKEY_CURRENT_USER\Software\<company name>\<section>\<data>.
	// The "company name" will here be provisionally "LPF".
	SetRegistryKey("LPF");

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	//Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Initialize path names of auxiliary data files. These files are in the
	// same directory as the application's exe file.
	// This must be done before ProcessShellCommand() is called, which inits
	// the empty document via CDiscoveryDoc::OnFileNew().

	char szPath[_MAX_PATH], *cp;
	VERIFY( GetModuleFileName(NULL, szPath, _MAX_PATH) > 0 );
	cp = strrchr(szPath, '\\');
	if (cp != NULL)
		*(cp+1) = '\0';	// "C:\MYDIR\SUBDIR\DISCOVERY.EXE" -> "C:\MYDIR\SUBDIR\"
	
	// PROPERTY.DAT: File with elemental properties for QSD
	m_strQsdPropertyFileName  = szPath;
	m_strQsdPropertyFileName += "Property.dat";

	// Drive and directory of the QSD.EXE kernel ("C:\DIR\SUBDIR\").
	// In the current version, this is assumed to be in the same directory as DISCOVERY.EXE.
	m_strQsdExePath			  = szPath;

	// DatabaseFields.dat: File with database field that can be evaluated for Chemistry and QSD.
	m_strDatabaseFieldsFileName	 = szPath;
	m_strDatabaseFieldsFileName += "DatabaseFields.dat";

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CDiscoveryDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CDiscoveryView));
	AddDocTemplate(pDocTemplate);

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Maximize frame window, if previously maximized
	ASSERT_KINDOF(CMainFrame, m_pMainWnd);
	if ( ((CMainFrame*)m_pMainWnd)->m_bMaximizeFrame )
		m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);

	return TRUE;
}

// Switch to QSD view after loading a QSD file, otherwise to report view mode
CDocument* CDiscoveryApp::OpenDocumentFile(LPCTSTR lpszFileName) 
{
	CDocument* pDoc = CWinApp::OpenDocumentFile(lpszFileName);

	if (pDoc != NULL)
	{
		int ifmt = ((CDiscoveryDoc*)pDoc)->m_iFileFormat;
		if (ifmt == FILEFMT_QSD)
			// TRUE as 2nd parameter activates the wizard for the selection of the run mode
			((CMainFrame*)AfxGetMainWnd())->SwitchToViewMode(VIEW_QSD, TRUE);
		else
			((CMainFrame*)AfxGetMainWnd())->SwitchToViewMode(VIEW_REPORT);
	};

	return pDoc;
}

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryApp commands

void CDiscoveryApp::OnFileOpen() 
{
	// The standard implementation <CWinApp::OnFileOpen> would call
	// <CWinApp::DoPromptFileName(...)> to execute the File Open dialog with
	// only the native Discovery format in the File Type combo box of that dialog.
	// This version can also read other formats (e.g. CSV).
	CString strFilter((LPCSTR)IDS_OPENFILTER);
	CFileDialog dlg(TRUE, NULL, NULL, OFN_CREATEPROMPT, strFilter);

	// File Open dialog shall start in current working directory
	char szWorkDir[512];
	_getcwd (szWorkDir, sizeof(szWorkDir)-1);
	dlg.m_ofn.lpstrInitialDir = szWorkDir;

	// Zuletzt gewaehlten Dateityp (z.B. "CIF" statt "DSF") vorauswaehlen:
	//dlg.m_ofn.nFilterIndex = GetConfiguration()->m_iLastOpenFormat;
	if (dlg.DoModal() == IDOK)
	{
		//GetConfiguration()->m_iLastOpenFormat = dlg.m_ofn.nFilterIndex;
		OpenDocumentFile(dlg.GetPathName());
	}
}

// App command to run the dialog
void CDiscoveryApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// Global functions for Discovery application

/////////////////////////////////////////////////////////////////////////////
// GetPopupMenuCmd()
// Input: pMenu: Pointer to popup menu object;
//        x, y: screen coordinates of reference position;
//        pWnd: pointer to owner of menu/receiver of commands;
//        uFlags: flags for alignment.
// Returns: >0: menu command, 0=cancelled.
// Opens a context menu with CMenu::TrackPopupMenu().
// This function is a workaround, since the normal use of CMenu::TrackPopupMenu()
// crashes on some Windows installations. With the special flags TPM_NONOTIFY |
// TPM_RETURNCMD TrackPopupMenu() does not return unless the user has chosen a
// command or closed the context menu (with Escape key or by clicking outside
// the context menu). To call the OnUpdate...() handlers of the menu owner,
// WM_INITIMENUPOPUP messages are sent for the popup menu and all containing
// popup submenus to the receiver window <pWnd>.

UINT GetPopupMenuCmd (CMenu* pMenu, int x, int y, CWnd* pWnd,
					  UINT uFlags /*=TPM_LEFTALIGN|TPM_RIGHTBUTTON*/)
{
	if (pMenu == NULL || pMenu->GetSafeHmenu() == 0)
		return 0;

	int nItems = pMenu->GetMenuItemCount();
	if (nItems <= 0)
		return 0;

	pWnd->SendMessage(WM_INITMENUPOPUP, (WPARAM)pMenu->GetSafeHmenu(), (LPARAM)0);

	for (register int iItem=0; iItem < nItems; iItem++)
	{
		HMENU hSubMenu = ::GetSubMenu(pMenu->GetSafeHmenu(), iItem);
		if (hSubMenu > 0)
			pWnd->SendMessage(WM_INITMENUPOPUP, (WPARAM)hSubMenu, (LPARAM)0);
	};

	UINT uCmd = pMenu->TrackPopupMenu(uFlags | TPM_NONOTIFY | TPM_RETURNCMD, x, y, pWnd);

	return uCmd;
};


/////////////////////////////////////////////////////////////////////////////
// CopyList()
// Input: Reference to CListCtrl object.
// Returns number of copied rows.
// Copies the rows that are selected in the list control <list> to the Windows clipboard.
// Subitems will be separated by tabs, rows by CRLF.

int CopyList (CListCtrl& list) 
{
	CWaitCursor waitCursor;
	CStringList Strings;
	int iSize  = 0;
	int nLines = 0;
	for (register int ind=-1; (ind=list.GetNextItem(ind, LVNI_SELECTED)) != -1; )
	{
		CString strLine;
		LV_COLUMN lvc;
		for (register int icol=0; list.GetColumn(icol, &lvc); icol++)
		{
			if (icol > 0)
				strLine += "\t";
			strLine += list.GetItemText(ind, icol);
		};
		strLine += "\r\n";
		iSize += strLine.GetLength();
		Strings.AddTail(strLine);
		nLines++;
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

	if (list.OpenClipboard())
	{
		EmptyClipboard();
		SetClipboardData(CF_TEXT, hBuffer);
		CloseClipboard();
	};

	return nLines;
};


/////////////////////////////////////////////////////////////////////////////
// SaveList()
// Saves the contents of the CListCtrl object <list> in the file with name <lpszFileName>.
// <iFmt> defines the file format. All formats are text formats that are also
// used by MS Excel.
//   0=CSV format: fields are separated by semicolons (if a field contains a semicolon,
//                 encloses this text in ""), lines by CRLF sequences.
//   1=TXT format: like CSV, but fields are separated by tabs.
//   2=PRN-Format: format with fixed column widths. Depending on the width of a column
//                 empty space will be filled with blanks. If a text is too wide, it will
//                 be shortened (e.g. "1.2345(6...").
// Returns number of written records (rows); -1=file error.

int SaveList (CListCtrl& list, int iFmt, LPCSTR lpszFileName)
{
	ASSERT(iFmt >= 0 && iFmt <= 2 && lpszFileName != NULL);

	// Datei hat auf jeden Fall Textformat, daher <CStdioFile>.
	CStdioFile file;
	if ( !file.Open(lpszFileName, CFile::modeCreate | CFile::modeWrite, NULL) )
		return -1;

	CString strLine, strItem;
	register int irow, icol, nrows=list.GetItemCount();

	// Request number of columns
	LV_COLUMN lvc;
	for (int nColumns=0; list.GetColumn(nColumns, &lvc); nColumns++);

	if (iFmt != 2)
	{
		// Bei CSV und TXT einfach nur Texte auslesen und mit den jeweiligen
		// Trennzeichen in die Datei schreiben.
		char cSep = ((iFmt == 1) ? '\t' : ';');

		// Erst die Spaltentitel, dann die <nrows> Zeilen
		for (icol=0; icol < nColumns; icol++)
		{
			char szTitle[256];
			LV_COLUMN lvc;
			lvc.mask = LVCF_TEXT;
			lvc.pszText = szTitle;
			lvc.cchTextMax = sizeof(szTitle);
			list.GetColumn(icol, &lvc);
			if (icol > 0)
				strLine += cSep;
			strLine += szTitle;
		};
		strLine += "\n";
		file.WriteString(strLine);

		for (irow=0; irow < nrows; irow++)
		{
			strLine.Empty();
			for (icol=0; icol < nColumns; icol++)
			{
				if (icol > 0)
					strLine += cSep;
				strItem = list.GetItemText(irow, icol);
				if ( !strItem.IsEmpty() )
				{
					// In "" einschliessen, wenn Trennzeichen ';' im Text vorhanden
					BOOL bSem = (strItem.Find(cSep) >= 0);
					if (bSem)
						strLine += "\"";
					strLine += strItem;
					if (bSem)
						strLine += "\"";
				}
			};
			strLine += "\n";
			file.WriteString(strLine);
		}
	}
	else	// (iFmt == 2)
	{
		// Feste Spaltenbreite: Zunaechst fuer jede Spalte Breite (in Textzeichen)
		// ermitteln und dann Texte links- oder rechtsbuendig formatieren.
		// Ueberlaengen einfach abschneiden.
		TEXTMETRIC tm;
		CClientDC dcList(&list);
		dcList.GetTextMetrics(&tm);

		// Ausrichtungen (links/rechtsbuendig, zentriert) fuer jede Spalte ermitteln.
		// Spaltenbreiten von Pixel in Zeichen umrechnen (mit Hilfe der mittleren Zeichenbreite).
		// Bei der Gelegenheit gleich Spaltentitel ausgeben.
		int aiColWidths[MAX_REPORT_COLUMNS];
		int aiColFormat[MAX_REPORT_COLUMNS];
		for (icol=0; icol < nColumns; icol++)
		{
			char szTitle[256];
			LV_COLUMN lvc;
			lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
			lvc.pszText = szTitle;
			lvc.cchTextMax = sizeof(szTitle);
			list.GetColumn(icol, &lvc);
			aiColWidths[icol] = (int)ROUND((float)lvc.cx / (float)tm.tmAveCharWidth);
			aiColFormat[icol] = lvc.fmt & LVCFMT_JUSTIFYMASK;
			if (aiColFormat[icol] == LVCFMT_RIGHT)
				strItem.Format("%*.*s", aiColWidths[icol], aiColWidths[icol], szTitle);
			else if (aiColWidths[icol] == LVCFMT_CENTER)
				strItem.Format("%-*.*s", aiColWidths[icol], aiColWidths[icol], szTitle);
			else
				strItem.Format("%-*.*s", aiColWidths[icol], aiColWidths[icol], szTitle);
			strLine += strItem;
		};
		strLine += "\n";
		file.WriteString(strLine);

		// Jetzt die <nrows> Zeilen ausgeben
		for (irow=0; irow < nrows; irow++)
		{
			strLine.Empty();
			for (icol=0; icol < nColumns; icol++)
			{
				CString strText = list.GetItemText(irow, icol);
				if (aiColFormat[icol] == LVCFMT_RIGHT)
					strItem.Format("%*.*s", aiColWidths[icol], aiColWidths[icol], strText);
				else
					strItem.Format("%-*.*s", aiColWidths[icol], aiColWidths[icol], strText);
				strLine += strItem;
			};
			strLine += "\n";
			file.WriteString(strLine);
		}
	};

	return nrows+1;	// +1 wegen Titelzeile
};


/////////////////////////////////////////////////////////////////////////////
// SetFileExtension()
// Input: strFileName: Reference to string with filename;
//        lpszExt: pointer to string with extension.
// Returns: TRUE=successful, FALSE=error.
// Appends the extension <lpszExt> to the file name <strFileName>,
// or replaces an existing extension.

BOOL SetFileExtension (CString &strFileName, LPCSTR lpszExt)
{
	ASSERT(lpszExt != NULL);

	if ( strFileName.IsEmpty() )
		return FALSE;

	int idot = strFileName.ReverseFind('.');
	if (idot >= 0)
	{
		CString strName	 = strFileName.Left(idot+1);
		strName			+= lpszExt;
		strFileName		 = strName;
	}
	else
	{
		strFileName		+= ".";
		strFileName		+= lpszExt;
	};
	return TRUE;
};

/////////////////////////////////////////////////////////////////////////////
// Element symbols for the following element symbol interpreting functions:
// 0=Dummy, 1=H, 2=He, ..., 105=Ha, special H isotopes: 108=D, 109=T
/*	static const char Symbols[MAXELE][3]=
{"? ","H ","He","Li","Be","B ","C ","N ","O ","F ","Ne",
      "Na","Mg","Al","Si","P ","S ","Cl","Ar",
      "K ","Ca","Sc","Ti","V ","Cr","Mn","Fe","Co","Ni","Cu","Zn",
      "Ga","Ge","As","Se","Br","Kr",
      "Rb","Sr","Y ","Zr","Nb","Mo","Tc","Ru","Rh","Pd","Ag","Cd",
      "In","Sn","Sb","Te","I ","Xe",
      "Cs","Ba","La",
      "Ce","Pr","Nd","Pm","Sm","Eu","Gd","Tb","Dy","Ho","Er","Tm","Yb","Lu",
      "Hf","Ta","W ","Re","Os","Ir","Pt","Au","Hg",
      "Tl","Pb","Bi","Po","At","Rn",
      "Fr","Ra","Ac",
      "Th","Pa","U ","Np","Pu","Am","Cm","Bk","Cf","Es","Fm","Md","No","Lr",
      "Rf","Ha","  ","  ","D ","T "}	*/

/////////////////////////////////////////////////////////////////////////////
// Alphabetically sorted array with element symbols including "D" for deuterium
// and "T" for tritium. Used to convert element symbols in numbers and vice versa.
// Note: These numbers are no ordinal numbers.
static const char Symbols[][3]=
 {  "  ","Ac","Ag","Al","Am","Ar","As","At","Au","B ","Ba","Be","Bi","Bk",	//  0..13
    "Br","C ","Ca","Cd","Ce","Cf","Cl","Cm","Co","Cr","Cs","Cu",			// 14..25
    "D ","Dy","Er","Es","Eu","F ","Fe","Fm","Fr","Ga","Gd","Ge","H ","Ha","He",	// 26..40
    "Hf","Hg","Ho","I ","In","Ir","K ","Kr","La","Li","Lr","Lu","Md","Mg",	// 41..54
    "Mn","Mo","N ","Na","Nb","Nd","Ne","Ni","No","Np","O ","Os","P ",		// 55..67
    "Pa","Pb","Pd","Pm","Po","Pr","Pt","Pu","Ra","Rb","Re","Rf","Rh","Rn",	// 68..81
    "Ru","S ","Sb","Sc","Se","Si","Sm","Sn","Sr","T ","Ta","Tb","Tc",		// 82..94
    "Te","Th","Ti","Tl","Tm","U ","V ","W ","Xe","Y ","Yb","Zn",			// 95..106
    "Zr" };																	// 107

/////////////////////////////////////////////////////////////////////////////
// ElementSymbol()
// Writes the element symbol of the element with the ordinal number <no>
// into the buffer <buf>. If <no> is no valid number, <buf> keeps empty.
// Returns a pointer to the string buffer <buf>.

LPSTR ElementSymbol (LPSTR buf, int no)
{
	const char *cp;
	if (no < 0 || no >= sizeof(Symbols)/sizeof(Symbols[0])/*MAXELE*/)
		buf[0] = '\0';
	else
	{
		cp = Symbols[no];
		buf[0] = *cp; buf[1] = *(cp+1);
		// Terminate string after one or two letters, rsp.
		if (buf[1] == ' ')
			buf[1] = '\0';
		else 
			buf[2] = '\0';
	};
	return buf;
};

/////////////////////////////////////////////////////////////////////////////
// GetElementNo()
// Returns the ordinal number of the element with the symbol <lpszSymbol>.
// Note that D and T use special ordinal numbers.
// Returns -1 if not successfull.

int GetElementNo (LPCSTR lpszSymbol)
{
	register int i;
	char szbuf[4];
	const char *cp;
	// Copy element symbol and append blank for a one-character symbol
	szbuf[0] = toupper(lpszSymbol[0]);
	szbuf[1] = ((lpszSymbol[1] != '\0' && isalpha(lpszSymbol[1])) ?
				tolower(lpszSymbol[1]) : ' ');
	szbuf[2] = '\0';
	// Maybe the symbol contains 'attached hydrogens', e.g. "NH".
	// (Exception for "Rh" and "Th".)
	if (szbuf[1]=='h')
	{
		if (szbuf[0] != 'R' && szbuf[0] != 'T')
			szbuf[1] = ' ';
	};
	int nMax = sizeof(Symbols)/sizeof(Symbols[0])/*MAXELE*/;
	for (i=0; i < nMax; i++)
	{
		cp = Symbols[i];
		if (szbuf[0] == *cp && szbuf[1] == *(cp+1))
			break;
	};
	return ((i < nMax/*MAXELE*/) ? i : -1);
};

/////////////////////////////////////////////////////////////////////////////
// ReportFileError
// Displays a message box referring to an error code that is stored in the
// CFileException pointer <pe>. The file name is also stored in <pe>.

void ReportFileError (CFileException *pe)
{
	CString strError, strMsg;
	strError.LoadString((pe->m_cause == CFileException::none) ? IDS_NO_ERROR :
						(pe->m_cause == CFileException::genericException) ? IDS_GENERIC_ERROR :
						(pe->m_cause == CFileException::fileNotFound) ? IDS_FILE_NOT_FOUND :
						(pe->m_cause == CFileException::badPath) ? IDS_BAD_PATH :
						(pe->m_cause == CFileException::tooManyOpenFiles) ? IDS_TOO_MANY_OPEN_FILES :
						(pe->m_cause == CFileException::accessDenied) ? IDS_ACCESS_DENIED :
						(pe->m_cause == CFileException::removeCurrentDir) ? IDS_REMOVE_CURRENT_DIR :
						(pe->m_cause == CFileException::directoryFull) ? IDS_DIRECTORY_FULL :
						(pe->m_cause == CFileException::badSeek) ? IDS_BAD_SEEK :
						(pe->m_cause == CFileException::hardIO) ? IDS_HARD_IO :
						(pe->m_cause == CFileException::sharingViolation) ? IDS_SHARING_VIOLATION :
						(pe->m_cause == CFileException::lockViolation) ? IDS_LOCK_VIOLATION :
						(pe->m_cause == CFileException::diskFull) ? IDS_DISK_FULL :
						(pe->m_cause == CFileException::endOfFile) ? IDS_END_OF_FILE :
																	 IDS_UNKNOWN_FILE_ERROR);
	strMsg  = pe->m_strFileName;
	strMsg += ":\n";
	strMsg += strError;
	AfxMessageBox(strMsg);
};

/////////////////////////////////////////////////////////////////////////////
// ReportQsdExeError
// Called when ::GetExitCodeProcess() has returned an exit code of 1 indicating
// that QSD.EXE reports an error. Then display the contents of the file
// "QSD.ERR" in a message box.

void ReportQsdExeError (void)
{
	CString strErr = ((CDiscoveryApp*)AfxGetApp())->m_strQsdExePath;
	strErr += "QSD.ERR";
	CStdioFile f;
	CString strLine, strMsg;
	if ( f.Open(strErr, CFile::typeText | CFile::modeRead | CFile::shareDenyNone) )
	{
		strMsg = "QSD.EXE reports the following error message:\n";
		while ( f.ReadString(strLine) )
		{
			strMsg += strLine;
			strMsg += "\n";
		}
	}
	else
		strMsg = "QSD.EXE encountered an error.\n"
				 "But QSD/Discovery is not able to open the error file QSD.ERR!";
	AfxMessageBox(strMsg, MB_ICONEXCLAMATION | MB_OK);
};

/////////////////////////////////////////////////////////////////////////////
// GetCsvItems
// Extracts all items from the string <str> that are separated by semicolons
// and stores them in the string array <items> without the semicolons.
// " characters will not be removed.
// If there are already items in <items>, these will not be removed here.
// Trailing or preceding whitespaces in <str> will not be removed here. (If necessary
// call TrimRight() and TrimLeft() for that string before calling this function.
// If <bRemove> is TRUE remove preceding and trailing "" characters from the items.
// Returns the number of items found.

int GetCsvItems (CString &str, CStringArray &items, BOOL bRemove)
{
	// Make a copy of <str>, since strtok() will overwrite the semicolons in <str>.
	int nlen	= str.GetLength();
	if (nlen <= 0)
		return 0;
	char *pbuf	= new char[nlen+1];
	strcpy(pbuf, str);
	char *cp	= strtok(pbuf, ";");
	while (cp != NULL)
	{
		if ( bRemove && strchr(cp, '\"') != NULL )
		{
			CString strItem;
			strItem = cp;
			strItem.TrimRight();
			strItem.TrimLeft();
			int nl	= strItem.GetLength();
			if (strItem[0] == '\"' && strItem[nl-1] == '\"')
			{
				char *pbuf2 = new char[nl+1];
				strcpy(pbuf2, strItem);
				pbuf2[0]	= pbuf2[nl-1] = '\0';
				items.Add(pbuf2+1);
				delete pbuf2;
			}
		}
		else
		{
			items.Add(cp);
		};
		cp		= strtok(NULL, ";");
	};
	delete pbuf;
	return items.GetSize();
};

/////////////////////////////////////////////////////////////////////////////
// RemoveComment
// Removes a comment from the string <str> including the preceding # character.
// Preceding and trailing whitespaces will also be removed.

void RemoveComment (CString &str)
{
	int nlen = str.GetLength();
	if (nlen <= 0)
		return;
	int npos = str.Find('#');
	if (npos >= 0)
	{
		char *cp = str.GetBuffer(nlen+1);
		cp[npos] = '\0';
		str.ReleaseBuffer();
	};
	str.TrimRight();
	str.TrimLeft();
};

/////////////////////////////////////////////////////////////////////////////
// ExtractNumbers
// Tries to extract a maximum number of <imax> values from the string <lpstr>
// and writes these numbers as floating point values to <pf>.
// Returns the number of extracted numbers.

int ExtractNumbers (LPCSTR lpstr, float* pf, int imax)
{
	ASSERT( pf != NULL && lpstr != NULL );

	int nlen	= strlen(lpstr);
	if (nlen <= 0 || imax <= 0)
		return 0;

	int icnt	= 0;
	char *pbuf	= new char[nlen+1];
	strcpy(pbuf, lpstr);

	char *ptok	= strtok(pbuf, ",;\t ");
	while (ptok != NULL && icnt < imax)
	{
		float f		= atof(ptok);	// returns zero if not successful
		pf[icnt++]	= f;
		ptok		= strtok(NULL, ",;\t ");
	};
	
	delete pbuf;
	return icnt;
};

/////////////////////////////////////////////////////////////////////////////
// GetQsdOperationString
// Loads the string that describes the operation with the index <ind>.
// Returns a pointer to the initialized string <str>.

LPCSTR GetQsdOperationString (CString& str, int ind)
{
	str.LoadString(	(ind == 1) ? IDS_SUM		:
					(ind == 2) ? IDS_DIFFERENCE	:
					(ind == 3) ? IDS_RATIO		:
					(ind == 4) ? IDS_PRODUCT	:
					(ind == 5) ? IDS_PRODUCT2	:
					(ind == 6) ? IDS_MAXIMUM	: 
					(ind == 7) ? IDS_MINIMUM	: IDS_NOOPERATION);
	return (LPCSTR)str;
};

/////////////////////////////////////////////////////////////////////////////
// MulVector4
// Calculates v2 := m * v1, where v1 and v2 are 4-point vectors and m is a 4x4 matrix.

void MulVector4 (float *v1, float *v2, MAT44 &m)
{
float x, y, z, w;
	x = v1[0]; y = v1[1]; z = v1[2]; w = v1[3];
	for (register int i=0; i < 4; i++)
		v2[i] = x*m[0][i] + y*m[1][i] + z*m[2][i] + w*m[3][i];
};

/////////////////////////////////////////////////////////////////////////////
// MulVector3
// Calculates v2 := m * v1, where v1 and v2 are 3-point vectors and m is a 4x4 matrix.

void MulVector3 (float *v1, float *v2, MAT44 &m)
{
float v[4];
	v[0]=v1[0];  v[1]=v1[1];  v[2]=v1[2];  v[3]=1.0F;
	MulVector4(v, v, m);
	v2[0]=v[0];  v2[1]=v[1];  v2[2]=v[2];
};

/////////////////////////////////////////////////////////////////////////////
// ResetMatrix
// Initializes the 4x4 values of the identity matrix

void ResetMatrix (MAT44 &mat)
{															// (1 0 0 0)
	memset (mat, 0, sizeof(MAT44));							// (0 1 0 0)
	for (register int i=0; i < 4; i++)						// (0 0 1 0)
		mat[i][i] = (float)1.0;								// (0 0 0 1)
};

/////////////////////////////////////////////////////////////////////////////
// MulMatrix
// Performs matrix multiplication: c = b * a.

void MulMatrix (MAT44 &a, MAT44 &b, MAT44 &c)
{
	float b0, b1, b2, b3;
	for (register int i = 0; i < 4; i++)
	{
		b3=b[3][i]; b2=b[2][i]; b1=b[1][i]; b0=b[0][i];
		c[0][i] = a[0][0]*b0 + a[0][1]*b1 + a[0][2]*b2 + a[0][3]*b3;
		c[1][i] = a[1][0]*b0 + a[1][1]*b1 + a[1][2]*b2 + a[1][3]*b3;
		c[2][i] = a[2][0]*b0 + a[2][1]*b1 + a[2][2]*b2 + a[2][3]*b3;
		c[3][i] = a[3][0]*b0 + a[3][1]*b1 + a[3][2]*b2 + a[3][3]*b3; 
	}
};

/////////////////////////////////////////////////////////////////////////////
// GetRotationMatrix
// Initializes the 4x4 matrix <m> for a rotation along the axis specified in
// <ax> (0=x-, 1=y-, 2=z-axis) with the amount <ang> in degrees.

void GetRotationMatrix (int ax, float ang, MAT44& m)
{
	float w, sinw, cosw;
	w    = ang * DEG;		// convert degrees to radians
	sinw = sin(w);
	cosw = cos(w);
	ResetMatrix(m);
	if      (ax == 0)		// x-axis
	{
		m[1][1] =         m[2][2] = cosw;
		m[1][2] = -1.0F * (m[2][1] = sinw);
	}
	else if (ax == 1)		// y-axis
	{
		m[0][0] =         m[2][2] = cosw;
		m[2][0] = -1.0F * (m[0][2] = sinw);
	}
	else if (ax == 2)		// z-axis
	{
		m[0][0] =         m[1][1] = cosw;
		m[0][1] = -1.0F * (m[1][0] = sinw);
	}
};

/////////////////////////////////////////////////////////////////////////////
// TrimFloat
// Removes surplus zero digits from the floating point number and, if possible,
// the surplus decimal point.

LPCSTR TrimFloat (LPSTR lpszBuf /*, BOOL bPoint*/)
{
	for (char *cp=lpszBuf+strlen(lpszBuf)-1; *cp != '\0' && *cp == '0'; cp--)
		*cp = '\0';
	int ilen = strlen(lpszBuf);
	if (ilen >= 2 && lpszBuf[ilen-1] == '.' && isdigit(lpszBuf[ilen-2]) )
		lpszBuf[ilen-1] = '\0';
	return lpszBuf;
};

/***************************************************************************/
/*  RectInRect                                                             */
/*  Eingabe: s.Beschreibung.                                               */
/*  Ausgabe: "                                                             */
/*  Prueft, ob das Atom <pRect1> innerhalb des Rechtecks <pRect2> liegt.   */
/*  Ist <bTotal> TRUE, wird nur dann TRUE zurueckgegeben, wenn <pRect1>    */
/*  vollstaendig innerhalb des Rechtecks liegt, sonst FALSE.               */
/*  Ist <bTotal> dagegen FALSE, reicht es, wenn bereits ein Teil von       */
/*  <pRect1> innerhalb von <pRect2> liegt.                                 */
/*  Hinweis: Beide Rechtecke muessen normalisiert vorliegen.               */
/***************************************************************************/

BOOL RectInRect (LPCRECT pRect1, LPCRECT pRect2, BOOL bTotal)
{
	ASSERT(pRect1 != NULL && pRect2 != NULL);

	if (bTotal)
	{
		return (pRect1->left >= pRect2->left && pRect1->right  <= pRect2->right &&
				pRect1->top  >= pRect2->top  && pRect1->bottom <= pRect2->bottom);
	}
	else
	{
		CRgn Rgn;
		Rgn.CreateRectRgnIndirect(pRect2);
		BOOL bInside = Rgn.RectInRegion(pRect1);
		Rgn.DeleteObject();
		return bInside;
	}
};
