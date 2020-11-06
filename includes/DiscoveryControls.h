// DiscoveryControls.h : header file
// Ported to Windows 8.1 by Fulcrum.013 Mariupol, Ukraine 
// 17 september 2020

//

#ifndef __DISCOVERYCONTROLS_H__
#define __DISCOVERYCONTROLS_H__

/////////////////////////////////////////////////////////////////////////////
// CColorComboBox window

class CColorComboBox : public CComboBox
{
	int m_Id;

// Construction
public:
	CColorComboBox();

// Attributes
public:

// Operations
public:
	int AddColor (COLORREF);		// Farbe hinzufuegen
	void Init (int, COLORREF*);		// mit ... Farben initialisieren
	int SaveColors (void);			// Farben in Konfiguration sichern
	int SetColor (COLORREF);		// Farbe ... selektieren
	COLORREF GetSelColor (void);	// ausgewaehlte Farbe holen

private:
	BOOL FormatColorName (CString&, COLORREF);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CColorComboBox)
	public:
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CColorComboBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CColorComboBox)
	afx_msg void OnSelChange();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CFileSaveDataDlg dialog

class CFileSaveDataDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CFileSaveDataDlg)

public:
	CFileSaveDataDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

protected:
	virtual void OnTypeChange();

protected:
	//{{AFX_MSG(CFileSaveDataDlg)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CFileSaveListDlg dialog

class CFileSaveListDlg : public CFileDialog
{
	DECLARE_DYNAMIC(CFileSaveListDlg)

public:
	CFileSaveListDlg(BOOL bOpenFileDialog, // TRUE for FileOpen, FALSE for FileSaveAs
		LPCTSTR lpszDefExt = NULL,
		LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		LPCTSTR lpszFilter = NULL,
		CWnd* pParentWnd = NULL);

protected:
	virtual void OnTypeChange();

protected:
	//{{AFX_MSG(CFileSaveListDlg)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif	/* __DISCOVERYCONTROLS_H__ */
