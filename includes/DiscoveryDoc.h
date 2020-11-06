// DiscoveryDoc.h : interface of the CDiscoveryDoc class
// Ported to Windows 8.1 by Fulcrum.013 Mariupol, Ukraine 
// 17 september 2020
//
/////////////////////////////////////////////////////////////////////////////

#include <afxtempl.h>
#define  _CRT_SECURE_NO_WARNINGS TRUE
// One QSD record, consisting of several elements and one system property
class CQsdRecord
{
	int m_nElem;					// number of valid elements in record
public:
	BYTE m_abElemNo[MAX_ELEM_QSD];	// ordinal numbers of up to 8 elements
	float m_afStoich[MAX_ELEM_QSD];	// stoichometric indexes of these elements
	float m_afStoichStd[MAX_ELEM_QSD];	// stoichiometric factors in percent
	float m_fProp;					// the system property
	float m_fProb;					// probability of property if predicted
	float m_afXyz[3];				// coordinate triplet in Discovery Space
	float m_afNorm[3];				// normalized coordinates (0..1)
	float m_afView[3];				// normalized view coordinates
	RECT m_rectView;				// device coordinates of the ball
	BOOL m_bSel;					// TRUE=datapoint is selected
	BOOL m_bNoCoords;				// TRUE=datapoint with "undefined" coordinates
	BOOL m_bPredict;				// TRUE=property is to be/was predicted
	long m_iSort;					// index after Z-sorting of points
public:
	CQsdRecord();
	int AddElement(LPCSTR, LPCSTR);	// add an element and stoichiometric index
	int StdStoich (int=100);		// make the sum of stoichiometric indexes = 100
	LPCSTR Format(CString&);		// format a record for QSD kernel input file
	LPCSTR FormatComp(CString&);	// format element composition
	int GetPropertyIndex (void);	// get index of associated property
	COLORREF GetColor (void);		// get color associated to property
	static COLORREF GetColor (int);	// get color associated to index
	BOOL Match (const CQsdRecord&, int);	// do elements (and stoichiometric indexes) match?
	int Compare (const CQsdRecord&, UINT) const;	// used to sort QSD records
	int GetElementCount() const	{	return m_nElem;	}
};

typedef CArray<CQsdRecord, CQsdRecord&> CQsdRecordArray;

// One QSD profile
class CQsdProfile : public CObject
{
	DECLARE_SERIAL(CQsdProfile);
	int m_nMax, m_nVal;				// maximum and current number of values, rsp.
public:
	float *m_pfValues, *m_pfErrors;	// list of values and errors, rsp.
	int m_ip1, m_io1, m_ip2, m_io2,	// indexes of properties and operations, rsp.,
		m_ip3, m_io3;				// for axes x,y,z
	CQsdProfile();
	CQsdProfile (int, int, int, int, int, int, int);
	~CQsdProfile();
	const CQsdProfile& operator = (const CQsdProfile&);
	int AddValue (LPCSTR);			// extract value and error and add to list
	int GetValueCount (void) const
		{	return m_nVal;	}
};

typedef CArray<CQsdProfile, CQsdProfile&> CQsdProfileArray;

// One item (contents of a database field)
class CReportItem : public CObject
{
	DECLARE_SERIAL(CReportItem);
public:
	CString m_strText;				// the original text of the field
	short m_nNumbers;				// number of float values in <m_pNumbers>
	float *m_pNumbers;				// <m_nNumbers> numbers if field is numeric
	CReportItem();
	CReportItem (CString&, float*, int);
	~CReportItem();
	const CReportItem& operator = (const CReportItem&);
};

typedef CArray<CReportItem, CReportItem&> CReportItemArray;

// One column in the report view
class CColumn : public CObject
{
	DECLARE_SERIAL(CColumn);
public:
	short m_iFieldType;				// code that identifies the type of database field
	short m_iColInd;				// index of the column in the Report View
	CString m_strFieldName;			// LPF database field name, e.g. "[S452]"
	CString m_strDescription;		// description of a field, e.g. "London penetration depth (nm)"
	CString m_strLabel;				// label which appears in the header
	BYTE m_bNumeric;				// TRUE if items can be interpreted as number(s)
	BYTE m_bSel;					// TRUE if corresponding column is selected
	short m_iSort;					// 0=do not sort, -1=sort descending, 1=ascending
	short m_iColWidth;				// width of column in pixels
	CReportItemArray m_Items;		// contains the items of each row in that column
	CColumn();
	const CColumn& operator = (const CColumn&);
	int AddItem (CString&, float* =NULL, int=0);
	BOOL GetItemText (CString&, int);	// copy text from item ...
	LPCSTR GetItemText (int);		// get constant pointer to text
	int GetRowCount (void);
	int SetType (void);				// identify <m_iFieldType> from <m_strFieldName>
};

typedef CArray<CColumn, CColumn&> CColumnArray;

/////////////////////////////////////////////////////////////////////////////
// The Discovery document

class CDiscoveryDoc : public CDocument
{
protected: // create from serialization only
	CDiscoveryDoc();
	DECLARE_DYNCREATE(CDiscoveryDoc)

// Attributes
public:
	int m_iFileFormat;				// format of the input file
	DWORD m_dwFileVersionNumber;	// version number of the input file

	CColumnArray m_ReportColumns;	// the columns of the report view

	int m_iRunType;					// single run, prediction run, etc.
	int m_iProbFilter;				// used for communication between histogram and cube
	CQsdRecordArray m_QsdRecords;
	CStringArray m_astrQsdProperties;
	CStringArray m_astrQsdLegend;
	HANDLE m_hQsdProcess;			// process handle of QSD kernel
	CQsdProfileArray m_QsdProfiles;
	int m_nQsdMaxNeighbours;
	//int m_nQsdMaxNeighboursMultiple, m_nQsdMaxNeighboursSingle;
	int m_iQsdSelProfile;
	float m_afQsdScalings[3];		// maximum values on x-,y-,z-axis of Discovery Space
	CString m_QsdResultsReport;
	int m_iQsdSlabAxis;				// 1=x-, 2=y-, 3=z-axis is orthogonal to slab, 0=no slab
	float m_afQsdSlabRange[2];		// minimum and maximum value of the slab in axis-units
	
	CStringArray m_DatabaseFieldNames;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDiscoveryDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

	void SerializeDsc(CArchive &ar);	// serialize actual data in binary DSC format
	void SerializeCsv(CArchive &ar);	// serialize rows and columns for report view
	void SerializeQsd(CArchive &ar);	// serialize special text format for QSD evaluation

	BOOL Init (void);				// initialize auxiliary data
	int GetReportColumnIndexes (int*);	// what info comes into what column of the report view?
	int SetReportColumnIndexes (int*, int);	// reverse of GetReportColumnIndexes()
	int MoveReportColumnIndex (int, int);	// move column index to new position

	int CreateQsdRecords (void);	// init m_QsdRecords[] from selected column in m_ReportColumns[]
	//int WriteQsdInputFiles (void);	// write QSD.CFG and input file for QSD kernel
	int ReadQsdProfiles (LPCSTR, BOOL);	// read QSD profiles from given file name
	int UpdateQsdProfile (LPCSTR, int);	// update QSD profile number ...
	long ReadQsdResults (LPCSTR);		// read QSD Results Report
	int ReadQsdDiscoverySpace (LPCSTR);	// read QSD Discovery Space
	int RemoveQsdRecords (int);			// removes QSD E- or P-records
	int ImportQsdRecords (LPCSTR, int);	// add QSD records from a file to m_QsdRecords[]
	int GetQsdDimensionCount (void);	// is QSD Discovery Space 3-, 2- or 1-dimensional?
	int GetQsdRecordCount (int);		// returns number of QSD E- or P-records

// Implementation
public:
	virtual ~CDiscoveryDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CDiscoveryDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
