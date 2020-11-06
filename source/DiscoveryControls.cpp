/////////////////////////////////////////////////////////////////////////////
// DiscoveryControls.cpp : Implementation of several controls
// Created: 18.01.99 - Latest update: 18.01.99
// Ported to Windows 8.1 by Fulcrum.013 Mariupol, Ukraine 
// 17 september 2020

#define  _CRT_SECURE_NO_WARNINGS 
#include "stdafx.h"
#include "DiscoveryControls.h"
#include <dlgs.h>				// resource symbols for common dialogs

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#if 0
/////////////////////////////////////////////////////////////////////////////
// CColorComboBox

CColorComboBox::CColorComboBox()
{
	m_Id = 0;
}

CColorComboBox::~CColorComboBox()
{
}

BOOL CColorComboBox::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext) 
{
	// Control ID merken
	m_Id = nID;
	
	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}

// Farbe <AColor> der Combobox an vorletzter Stelle hinzufuegen, vorausgesetzt,
// die Farbe existiert noch nicht in der Liste.
// Rueckgabe: Index der Farbe in der Liste.
int CColorComboBox::AddColor (COLORREF AColor)
{
	for (register int i=GetCount()-2; i >= 0; i--)
	{
		if (GetItemData(i) == AColor)
			return i;
	};

	return InsertString (GetCount()-1, (LPCSTR)AColor);
}

// Combobox mit den <nColors> Farben in <pColors> initialisieren
void CColorComboBox::Init (int nColors, COLORREF* pColors)
{
	ASSERT(pColors);

	// Erst alles loeschen, dann einen Eintrag fuer "New color" einfuegen
	ResetContent();
	AddString((LPCSTR)RGB_UNDEF);

	for (register int i=0; i < nColors; i++)
		AddColor (pColors[i]);
}

// Ueber die 12 Standardfarben hinausgehende Farben in Konfiguration sichern.
// Rueckgabe: Zahl der Farben, die zur Konfiguration hinzugefuegt wurden.
int CColorComboBox::SaveColors (void)
{
	CConf* pConf = GetConfiguration();

	int nNewColors = GetCount()-1 - 12;
	int nInserted  = 0;
	for (register int i=0; i < nNewColors; i++)
	{
		COLORREF cr = (COLORREF)GetItemData(i+12);
		for (register int k=12; k < pConf->m_nCustomColors; k++)
		{
			if (pConf->m_acrCustomColors[k] == cr)
				break;
		};
		// Neue Farbe an die 12. Stelle einfuegen, den Rest verschieben
		if (k == pConf->m_nCustomColors)
		{
			memmove(pConf->m_acrCustomColors+13, pConf->m_acrCustomColors+12,
					(MAXCUSTOMCOLORS-13)*sizeof(COLORREF));
			pConf->m_acrCustomColors[12] = cr;
			nInserted++;
			if (pConf->m_nCustomColors < MAXCUSTOMCOLORS)
				pConf->m_nCustomColors++;
		}
	};

	return nInserted;
}

// Namen der Farbe <AColor> dem String <AString> zuweisen oder "(<R>,<G>,<B>)"
BOOL CColorComboBox::FormatColorName (CString& AString, COLORREF AColor)
{
	if ( !(AColor == RGB_STANDARD || (AColor >= 0 && AColor <= 0xFFFFFFUL)) )
		return FALSE;

	int nId = (	(AColor == RGB_STANDARD)		? IDS_COLOR_AUTO		:
				(AColor == RGB_BLACK)			? IDS_COLOR_BLACK		:
				(AColor == RGB_WHITE)			? IDS_COLOR_WHITE		:
				(AColor == RGB(255,0,0))		? IDS_COLOR_RED			:
				(AColor == RGB(0,0,255))		? IDS_COLOR_BLUE		:
				(AColor == RGB(0,255,0))		? IDS_COLOR_GREEN		:
				(AColor == RGB(255,255,0))		? IDS_COLOR_YELLOW		:
				(AColor == RGB(0,255,255))		? IDS_COLOR_CYAN		:
				(AColor == RGB(255,0,255))		? IDS_COLOR_PURPLE		:
				(AColor == RGB(64,64,64))		? IDS_COLOR_DARKGREY	:
				(AColor == RGB(128,128,128))	? IDS_COLOR_MEDIUMGREY	:
				(AColor == RGB(192,192,192))	? IDS_COLOR_LIGHTGREY	: -1);

	if (nId != -1)
		AString.LoadString(nId);
	else
		AString.Format("(%u,%u,%u)", GetRValue(AColor), GetGValue(AColor), GetBValue(AColor));

	return TRUE;
}

// Farbe <AColor> auswaehlen. Wenn diese Farbe noch nicht existiert, neu hinzufuegen.
// Rueckgabe: Index der Farbe in Liste.
int CColorComboBox::SetColor (COLORREF AColor)
{
	if (AColor == RGB_UNDEF)
		return SetCurSel(-1);

	for (register int i=GetCount()-2; i >= 0; i--)
	{
		if (GetItemData(i) == AColor)
			return SetCurSel(i);
	};

	// Neue Farbe im Anschluss an die 12 Standardfarben einfuegen
	int ind = InsertString (12, (LPCSTR)AColor);

	return SetCurSel(ind);
}

// Aktuell ausgewaehlte Farbe holen.
COLORREF CColorComboBox::GetSelColor (void)
{
	int ind = GetCurSel();
	if (ind >= 0 && ind < GetCount()-1)
		return (COLORREF)GetItemData(ind);
	else
		return RGB_UNDEF;
}

BEGIN_MESSAGE_MAP(CColorComboBox, CComboBox)
	//{{AFX_MSG_MAP(CColorComboBox)
	ON_CONTROL_REFLECT(CBN_SELCHANGE, OnSelChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorComboBox message handlers

void CColorComboBox::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	lpMeasureItemStruct->itemHeight = 12;
}

void CColorComboBox::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// Letzter Eintrag in Liste -> "New color"
	BOOL bNew   = ((int)lpDrawItemStruct->itemID == GetCount()-1);

	CDC* pDC	= CDC::FromHandle(lpDrawItemStruct->hDC);
	COLORREF cr	= (COLORREF)lpDrawItemStruct->itemData;

	if ((lpDrawItemStruct->itemAction & (ODA_DRAWENTIRE | ODA_SELECT)) != 0)
	{
		// Farbiges Rechteck in linker Ecke des Combobox-Eintrags
		CBrush brColor(cr);
		CRect rColor;
		rColor.left	  = lpDrawItemStruct->rcItem.left + 2;
		rColor.top	  = lpDrawItemStruct->rcItem.top  + 2;
		rColor.right  = rColor.left + 16;
		rColor.bottom = lpDrawItemStruct->rcItem.bottom - 2;

		CRect rText(lpDrawItemStruct->rcItem.left, rColor.top, lpDrawItemStruct->rcItem.right, rColor.bottom);

		// Bei Auswahl weisser Text auf dunkelblauem Untergrund, sonst schwarz auf weiss.
		// Falls das Steuerelement nicht ausgewaehlt ist, mit grauem Untergrund arbeiten.
		COLORREF crText;
		if ((lpDrawItemStruct->itemState & ODS_SELECTED) != 0)
		{
			CBrush brSelect(RGB(0,0,128));
			pDC->FillRect(&lpDrawItemStruct->rcItem, &brSelect);
			crText = RGB(255,255,255);
		}
		else
		{
			BOOL bDisabled = ((lpDrawItemStruct->itemState & ODS_DISABLED) != 0); 
			CBrush brSelect(bDisabled ?	RGB(192,192,192) : RGB(255,255,255));
			pDC->FillRect(&lpDrawItemStruct->rcItem, &brSelect);
			crText = (bDisabled ? RGB(128,128,128) : RGB(0,0,0));
		}

		// Wenn im undefinierten Zustand, keinen Text und kein Farbfeld 
		if (lpDrawItemStruct->itemID == -1)
			return;

		// Farbbezeichnung formatieren, z.B. "Blue" oder "RGB(120,64,140)", oder "New color"
		CString strColor;
		if ( bNew )
			strColor.LoadString(IDS_NEWCOLOR);
		else
			FormatColorName (strColor, cr);

		// Text erscheint rechts neben dem Farbfeld
		int iOldBkMode = pDC->SetBkMode(TRANSPARENT);
		COLORREF crOld = pDC->SetTextColor(crText);
		pDC->TextOut(rColor.right+2, rText.top, strColor);
		pDC->SetTextColor(crOld);
		pDC->SetBkMode(iOldBkMode);

		// Schliesslich Farbfeld zeichnen
		if ( !bNew )
		{
			CBrush* pOldBrush = (CBrush*)pDC->SelectObject(&brColor);
			CPen*   pOldPen   = (CPen*)pDC->SelectStockObject(BLACK_PEN);
			pDC->Rectangle(rColor);
			pDC->SelectObject(pOldPen);
			pDC->SelectObject(pOldBrush);
		}
	}
}

void CColorComboBox::OnSelChange() 
{
	// Farbauswahl-Dialog aufrufen, wenn letzter Eintrag ausgewaehlt wird
	int nTot = GetCount();
	int nSel = GetCurSel();
	if (nTot > 0 && nSel == nTot-1)
	{
		TRACE("CColorComboBox::OnSelChange() called for \"New color\" item\n");

		CColorDialog ColorDlg;
		if (ColorDlg.DoModal() == IDOK)
		{
			SetColor (ColorDlg.GetColor());
			
			// Uebergeordnetem Dialog mitteilen, dass neue Farbe definiert wurde
			GetParent()->SendMessage(WM_COMMAND, MAKELONG(m_Id,CBN_SELCHANGE),
									 (LPARAM)m_hWnd);
		}
	}
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileSaveDataDlg

IMPLEMENT_DYNAMIC(CFileSaveDataDlg, CFileDialog)

CFileSaveDataDlg::CFileSaveDataDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
}

// Extension des Dateinamens an aktuellen Datentyp anpassen,
// speziell fuers Speichern des Datenblatts.
// Vergleiche <CFileSaveDlg>.
void CFileSaveDataDlg::OnTypeChange() 
{
	// Aktuellen Dateinamen heranziehen
	char szPathName[_MAX_PATH];
	GetParent()->SendMessage(CDM_GETSPEC, _MAX_PATH, (LPARAM)szPathName);

	// Neuen Index fuer den Dateityp auswerten und entsprechende Extension bereitstellen.
	// Typ 1=RTF, 2=TXT.
	CString strExt;
	strExt = ((m_ofn.nFilterIndex == 1) ? "RTF" : "TXT");

	// Wenn kein Punkt, dann gar keine Extension verwenden, d.h. auch keine anhaengen.
	char *cp = strrchr(szPathName, '.');
	if (cp != NULL)
	{
		strcpy(cp+1, strExt);

		// Neuen Dateinamen mit korrigierter Extension zurueckschreiben
		GetParent()->SendMessage(CDM_SETCONTROLTEXT, edt1, (LPARAM)szPathName);
	}

	//CFileDialog::OnTypeChange();
}


BEGIN_MESSAGE_MAP(CFileSaveDataDlg, CFileDialog)
	//{{AFX_MSG_MAP(CFileSaveDataDlg)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFileSaveListDlg

IMPLEMENT_DYNAMIC(CFileSaveListDlg, CFileDialog)

CFileSaveListDlg::CFileSaveListDlg(BOOL bOpenFileDialog, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog(bOpenFileDialog, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd)
{
}


// Extension des Dateinamens an aktuellen Datentyp anpassen,
// speziell fuer Speichern einer Liste (Atome, Abstaende/Winkel etc.).
// Vergleiche <CFileSaveDlg>.
void CFileSaveListDlg::OnTypeChange() 
{
	// Aktuellen Dateinamen heranziehen
	char szPathName[_MAX_PATH];
	GetParent()->SendMessage(CDM_GETSPEC, _MAX_PATH, (LPARAM)szPathName);

	// Neuen Index fuer den Dateityp auswerten und entsprechende Extension bereitstellen.
	// Typ 1=CSV, 2=TXT, 3=PRN.
	CString strExt;
	strExt = (	(m_ofn.nFilterIndex == 1) ? "CSV" :
				(m_ofn.nFilterIndex == 2) ? "TXT" : "PRN");

	// Wenn kein Punkt, dann gar keine Extension verwenden, d.h. auch keine anhaengen.
	char *cp = strrchr(szPathName, '.');
	if (cp != NULL)
	{
		strcpy(cp+1, strExt);

		// Neuen Dateinamen mit korrigierter Extension zurueckschreiben
		GetParent()->SendMessage(CDM_SETCONTROLTEXT, edt1, (LPARAM)szPathName);
	}

	//CFileDialog::OnTypeChange();
}


BEGIN_MESSAGE_MAP(CFileSaveListDlg, CFileDialog)
	//{{AFX_MSG_MAP(CFileSaveListDlg)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

