// ChemView.h : header file
//
// Ported to Windows 8.1 by Fulcrum.013 Mariupol, Ukraine 
// 17 september 2020
/////////////////////////////////////////////////////////////////////////////
// CChemView view

class CChemView : public CView
{
protected:
	CChemView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CChemView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChemView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CChemView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CChemView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
