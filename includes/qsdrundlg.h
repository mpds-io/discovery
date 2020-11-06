// QsdRunDlg.h : header file
// Ported to Windows 8.1 by Fulcrum.013 Mariupol, Ukraine 
// 17 september 2020
//

/////////////////////////////////////////////////////////////////////////////
// CQsdSelectRunPage dialog
#define  _CRT_SECURE_NO_WARNINGS
class CQsdSelectRunPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CQsdSelectRunPage)

// Construction
public:
	CQsdSelectRunPage();
	~CQsdSelectRunPage();

// Dialog Data
	//{{AFX_DATA(CQsdSelectRunPage)
	enum { IDD = IDD_QSD_SELECT_RUN };
	int		m_iRunType;
	int		m_iLevel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CQsdSelectRunPage)
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CQsdSelectRunPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CQsdSingleRunPage dialog

class CQsdSingleRunPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CQsdSingleRunPage)

// Construction
public:
	CQsdSingleRunPage();
	~CQsdSingleRunPage();

	BOOL InitProperties (CComboBox*);
	BOOL InitOperations (CComboBox*);
	BOOL CheckSelections (void);

// Dialog Data
	//{{AFX_DATA(CQsdSingleRunPage)
	enum { IDD = IDD_QSD_SINGLE_RUN };
	CComboBox	m_OpZ;
	CComboBox	m_OpY;
	CComboBox	m_OpX;
	CComboBox	m_PropZ;
	CComboBox	m_PropY;
	CComboBox	m_PropX;
	int		m_iPropX;
	int		m_iPropY;
	int		m_iPropZ;
	int		m_iOpX;
	int		m_iOpY;
	int		m_iOpZ;
	CString	m_strFilename;
	//}}AFX_DATA

	BOOL m_bPredict;	// TRUE for prediction run, FALSE for standard single run


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CQsdSingleRunPage)
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual LRESULT OnWizardNext();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CQsdSingleRunPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangePropX();
	afx_msg void OnSelchangePropY();
	afx_msg void OnSelchangePropZ();
	afx_msg void OnSelchangeOpX();
	afx_msg void OnSelchangeOpY();
	afx_msg void OnSelchangeOpZ();
	afx_msg void OnLoad();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CQsdMultipleRunPage dialog

class CQsdMultipleRunPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CQsdMultipleRunPage)

// Construction
public:
	CQsdMultipleRunPage();
	~CQsdMultipleRunPage();

	void UpdateTotalRuns (void);

// Dialog Data
	//{{AFX_DATA(CQsdMultipleRunPage)
	enum { IDD = IDD_QSD_MULTIPLE_RUN };
	CStatic	m_Runs;
	CListBox	m_Op;
	CListBox	m_Prop;
	//}}AFX_DATA

	int m_nSelProps, m_nSelOps;		// numbers of selected properties and operations
	int *m_pSelProps, *m_pSelOps;	// and indexes when returning


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CQsdMultipleRunPage)
	public:
	virtual BOOL OnKillActive();
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CQsdMultipleRunPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeProp();
	afx_msg void OnSelchangeOp();
	afx_msg void OnAll();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CQsdConfPage dialog

class CQsdConfPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CQsdConfPage)

// Construction
public:
	CQsdConfPage();
	~CQsdConfPage();

// Dialog Data
	//{{AFX_DATA(CQsdConfPage)
	enum { IDD = IDD_QSD_CONFIGURATION };
	CSpinButtonCtrl	m_Spin3;
	CSpinButtonCtrl	m_Spin2;
	CSpinButtonCtrl	m_Spin1;
	int		m_iMax;
	int		m_iNum;
	BOOL	m_bError;
	int		m_iQuality;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CQsdConfPage)
	public:
	virtual BOOL OnSetActive();
	virtual BOOL OnWizardFinish();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CQsdConfPage)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////
// CQsdRunSheet

class CQsdRunSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CQsdRunSheet)

// Construction
public:
	CQsdRunSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CQsdRunSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:
	CQsdSelectRunPage	m_SelPage;
	CQsdSingleRunPage	m_SinglePage;
	CQsdMultipleRunPage	m_MultPage;
	CQsdConfPage		m_ConfPage;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQsdRunSheet)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CQsdRunSheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(CQsdRunSheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CQsdProgressDlg dialog

class CQsdProgressDlg : public CDialog
{
	CString m_strProgressFileName;	// complete path to "PROGRESS.DAT"
	HANDLE m_hProcess;	// process handle of QSD.EXE
	UINT m_hTimer;		// ID of the timer that scans the progress info file
	int m_iCycle;		// 1 when QSD.EXE is "preparing", 2 when "analyzing" data

// Construction
public:
	CQsdProgressDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CQsdProgressDlg)
	enum { IDD = IDD_QSD_PROGRESS };
	CStatic	m_Status;
	CStatic	m_Result;
	CProgressCtrl	m_Progress;
	//}}AFX_DATA

	int m_nRuns;		// count of multiple runs in the first step
	int m_nBests;		// count of "best runs" in the second step

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQsdProgressDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CQsdProgressDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// CQsdConfSingleDlg dialog

class CQsdConfSingleDlg : public CDialog
{
// Construction
public:
	CQsdConfSingleDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CQsdConfSingleDlg)
	enum { IDD = IDD_QSD_CONF_SINGLE };
	CSpinButtonCtrl	m_Spin;
	UINT	m_uMax;
	BOOL	m_bError;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQsdConfSingleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CQsdConfSingleDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
/////////////////////////////////////////////////////////////////////////////
// CQsdImportDlg dialog

class CQsdImportDlg : public CDialog
{
// Construction
public:
	CQsdImportDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CQsdImportDlg)
	enum { IDD = IDD_QSD_IMPORT };
	CProgressCtrl	m_Progress;
	CStatic	m_Status;
	CStatic	m_What;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CQsdImportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CQsdImportDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
