// DiscoveryDoc.cpp : implementation of the CDiscoveryDoc class
// Created: 11.01.1999 - Updated: 24.03.2001
// Ported to Windows 8.1 by Fulcrum.013 Mariupol, Ukraine 
// 17 september 2020

#define  _CRT_SECURE_NO_WARNINGS 
#include "stdafx.h"
#include "Discovery.h"

#include "DiscoveryDoc.h"

#include "QsdRunDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Current version number for native DSC format used by this application
const static auto s_dwVersionNumber = MAKELONG(0,1);

static const char s_szApplication[]="Application",
				  s_szQsd[]="Qsd";

/////////////////////////////////////////////////////////////////////////////
// CQsdRecord

CQsdRecord::CQsdRecord()
{
	memset(m_abElemNo, 0, sizeof(m_abElemNo));
	memset(m_afStoich, 0, sizeof(m_afStoich));
	memset(m_afStoichStd, 0, sizeof(m_afStoichStd));
	m_nElem = 0;
	//m_fProp = 0.0f;	// 0 can be a valid property, so -1 means undefined
	m_fProp = -1.0f;
	//m_fProb = 0.0f;	// ditto for probability
	m_fProb = -1.0f;
	m_afXyz[0]	= m_afXyz[1]  = m_afXyz[2]  = 0.0f;
	m_afNorm[0]	= m_afNorm[1] = m_afNorm[2] = 0.0f;
	m_afView[0]	= m_afView[1] = m_afView[2] = 0.0f;
	::SetRectEmpty(&m_rectView);
	m_bSel	= FALSE;
	m_iSort = -1;		// "not yet sorted"
	m_bNoCoords = FALSE;
	m_bPredict	= FALSE;
};

// Adds a new element and a new stoichiometric factor. The new element is inserted
// rather than appended if necessary to keep the alphabetic order of the elements.
// Returns the index of the added element or -1 if the list is alread full
// or -2 if there is a syntax error.
int CQsdRecord::AddElement (LPCSTR lpszElem, LPCSTR lpszStoich)
{
	if (m_nElem == MAX_ELEM_QSD)
		return -1;
	if (lpszElem == NULL || lpszStoich == NULL)
		return -2;

	// m_abElemNo[0] < m_abElemNo[1] < m_abElemNo[2] < ...
	BYTE bNew = (BYTE)GetElementNo(lpszElem);
	for (register int ipos=0; ipos < m_nElem; ipos++)
	{
		if (m_abElemNo[ipos] > bNew)
			break;
	};
	if (ipos < m_nElem)		// insertion -> shift other elements and stoich.factors
	{
		for (register int i=m_nElem; i > ipos; i--)
		{
			m_abElemNo[i] = m_abElemNo[i-1];
			m_afStoich[i] = m_afStoich[i-1];
		}
	};

	m_abElemNo[ipos] = bNew;
	m_afStoich[ipos] = atof(lpszStoich);
	m_nElem++;
	return ipos;
};

// Calculate the "decomposed chemical formula" from the stoichiometric indexes
// in <m_afStoich>, where the sum of stoichiometric factors is 100, and store
// these indexes in <m_afStoichStd>.
// Returns the number of elemens and stoichiometric numbers, rsp.
int CQsdRecord::StdStoich (int iMax /*=100*/)
{
	float fs = 0.0f;
	for (register int ie=0; ie < m_nElem; ie++)
		fs += m_afStoich[ie];
	ASSERT( fs > 0.0f );
	float fak = (float)iMax / fs;
	for (ie=0; ie < m_nElem; ie++)
		m_afStoichStd[ie] = m_afStoich[ie] * fak;
	return m_nElem;
};

// Format element composition. Symbols separated by commas.
LPCSTR CQsdRecord::FormatComp (CString &str)
{
	char szBuf[32];
	str.Empty();
	for (register int i=0; i < m_nElem; i++)
	{
		if (i > 0)
			str += ",";
		ElementSymbol(szBuf, m_abElemNo[i]);
		str += szBuf;
		if (m_afStoich[i] != 1.0f)		// immediately following: stoichiometry
		{
			sprintf(szBuf, "%5.4f", m_afStoich[i]);
			str += TrimFloat(szBuf);
		}
	};
	return (LPCSTR)str;
};

// Format line for input in QSD kernel, including linefeed.
LPCSTR CQsdRecord::Format (CString &str)
{
	// Begin with 'E' for "experimental" or 'P' for "to be predicted,
	// followed by semicolon.
	char szBuf[32];
	str  = (m_bPredict ? "P;" : "E;");
	_itoa(m_nElem, szBuf, 10);						// specify number of elements
	str += szBuf;
	str += ";";
	for (register int i=0; i < m_nElem; i++)
	{
		ElementSymbol(szBuf, m_abElemNo[i]);		// symbol of the i-th element
		str += szBuf;
		str += ";";
		sprintf(szBuf, "%5.4f", m_afStoich[i]);		// stoichiometry of the i-th element
		str += TrimFloat(szBuf);
		str += ";";
	};
	sprintf (szBuf, "%d", (int)ROUND(m_fProp));		// TO BE CHECKED
	//sprintf(szBuf, "%5.4f", m_fProp);
	str += szBuf;
	str += "\n";	// new line character for output in CStdioFile object
	return (LPCSTR)str;
};

// If <iMatch> is zero, return TRUE if the elements of <this> and <ARecord> match.
// If <iMatch> is one, return TRUE if both elements *and* stoichiometric indexes match.
BOOL CQsdRecord::Match (const CQsdRecord& ARecord, int iMatch)
{
	int nelem2 = ARecord.GetElementCount();
	for (register int ielem1=0; ielem1 < m_nElem; ielem1++)
	{
		for (register int ielem2=0; ielem2 < nelem2; ielem2++)
		{
			if (m_abElemNo[ielem1] == ARecord.m_abElemNo[ielem2])
			{
				if (iMatch == 0)
					break;
				else if (fabs(m_afStoich[ielem1]-ARecord.m_afStoich[ielem2]) < 1e-4)
					break;
			}
		};
		if (ielem2 == nelem2)	// element number <ielem1> not found in <ARecord>
			return FALSE;
	};
	return TRUE;	// if we reach this, all elements in <this> have been found in <ARecord>
};

// Compares this QSD record with <ARecord>. Returns -1 if <this> is less than <ARecord>,
// +1 if <this> is greater than <ARecord>, 0 if <this> is equal with <ARecord>.
// The criteria for comparison are:
//   1st preference: increasing element number,
//   2nd preference: element symbols alphabetically,
//   3rd preference: stoichiometric numbers (only if bit 0 of <uMask> is set),
//   4th preference: experimental < to be predicted (only if bit 1 of <uMask> is set).
int CQsdRecord::Compare (const CQsdRecord& ARecord, UINT uMask) const
{
	int nelem2 = ARecord.GetElementCount();
	if (m_nElem < nelem2)
		return -1;
	else if (m_nElem > nelem2)
		return 1;
	int iret = 0;
	register int i = 0;
	for (; i < m_nElem && iret == 0; i++)
	{
		int ielem1=m_abElemNo[i], ielem2=ARecord.m_abElemNo[i];
		iret = ((ielem1 < ielem2) ? -1 : (ielem1 > ielem2) ? 1 : 0);
	};
	BOOL bStoich = ((uMask & 1) != 0);
	for (i=0; bStoich == TRUE && iret == 0 && i < m_nElem; i++)
	{
		// For comparison, we use the stoichiometric factors given in percent
		// rather than the original factors.
		float fstoich1=m_afStoichStd[i], fstoich2=ARecord.m_afStoichStd[i];
		iret = ((fstoich1 < fstoich2) ? -1 : (fstoich1 > fstoich2) ? 1 : 0);
	};
	if (iret == 0 && (uMask & 2) != 0)
	{
		if ( !m_bPredict && ARecord.m_bPredict )
			iret = -1;
		else if ( m_bPredict && !ARecord.m_bPredict )
			iret = 1;
	};
	return iret;
};

// Static version returning the color associated with index <index>
COLORREF CQsdRecord::GetColor (int index)
{
	static const COLORREF s_acr[MAX_COLOR_QSD]=
	{	RGB(255,0,0),				// 0
		RGB(0,0,255),
		RGB(0,255,0),
		RGB(255,255,0),
		RGB(255,0,255),
		RGB(0,255,255),				// 5
		RGB(255,128,0),
		RGB(255,0,128),
		RGB(0,255,128),
		RGB(0,128,255),
		RGB(128,64,0),				// 10
		RGB(64,128,128),
		RGB(128,0,128),
		RGB(128,64,64),
		RGB(0,128,0),
		RGB(128,128,255),			// 15
		RGB(128,128,0),
		RGB(255,128,255),
		RGB(255,255,128),
		RGB(128,255,255)	};

	int ind = index % MAX_COLOR_QSD;
	return s_acr[ind];
};

// Returns color associated with the property of this QSD record
COLORREF CQsdRecord::GetColor (void)
{
	int ind = GetPropertyIndex() % MAX_COLOR_QSD;
	return GetColor(ind);
};

// Returns index for the property associated with the property of this QSD record
int CQsdRecord::GetPropertyIndex (void)
{
	int ind = (int)ROUND(m_fProp);		// TO BE CHECKED
	return ind;
};

/////////////////////////////////////////////////////////////////////////////
// CQsdProfile

IMPLEMENT_SERIAL(CQsdProfile, CObject, 0)

CQsdProfile::CQsdProfile()
{
	m_nMax		= m_nVal = 0;
	m_pfValues	= NULL;
	m_pfErrors	= NULL;
	m_ip1		= m_ip2 = m_ip3 = 0;
	m_io1		= m_io2 = m_io3 = 0;
};

CQsdProfile::CQsdProfile (int nMax, int ip1, int io1, int ip2, int io2, int ip3, int io3)
{
	m_nMax = nMax;
	m_nVal = 0;
	if (nMax > 0)
	{
		m_pfValues = new float[nMax];
		m_pfErrors = new float[nMax];
	}
	else
	{
		m_pfValues = NULL;
		m_pfErrors = NULL;
	};
	m_ip1 = ip1;
	m_ip2 = ip2;
	m_ip3 = ip3;
	m_io1 = io1;
	m_io2 = io2;
	m_io3 = io3;
};

CQsdProfile::~CQsdProfile()
{
	if (m_pfValues != NULL)
		delete m_pfValues;
	if (m_pfErrors != NULL)
		delete m_pfErrors;
};

const CQsdProfile& CQsdProfile::operator = (const CQsdProfile& AProfile)
{
	m_nMax	= AProfile.m_nMax;
	m_nVal	= AProfile.m_nVal;
	if (AProfile.m_pfValues != NULL)
	{
		if (m_pfValues != NULL)
			delete m_pfValues;
		m_pfValues = new float[m_nMax];
		memcpy(m_pfValues, AProfile.m_pfValues, m_nMax*sizeof(float));
	};
	if (AProfile.m_pfErrors != NULL)
	{
		if (m_pfErrors != NULL)
			delete m_pfErrors;
		m_pfErrors = new float[m_nMax];
		memcpy(m_pfErrors, AProfile.m_pfErrors, m_nMax*sizeof(float));
	};
	m_ip1 = AProfile.m_ip1;
	m_ip2 = AProfile.m_ip2;
	m_ip3 = AProfile.m_ip3;
	m_io1 = AProfile.m_io1;
	m_io2 = AProfile.m_io2;
	m_io3 = AProfile.m_io3;
	return *this;
};

int CQsdProfile::AddValue (LPCSTR lpszVal)
{
	if (m_nVal == m_nMax)
		return -1;
	// Examine value and error (in parantheses) if given
	float fv, fe;
	const char *cp1 = strchr(lpszVal, '(');
	const char *cp2 = strchr(lpszVal, ')');
	fv		  = atof(lpszVal);
	if (cp1 != NULL && cp2 != NULL && isdigit(*(cp1+1)) )
		fe	  = atof(cp1+1);
	else
		fe	  = 0.0f;
	m_pfValues[m_nVal] = fv;
	m_pfErrors[m_nVal] = fe;
	return m_nVal++;
};

/////////////////////////////////////////////////////////////////////////////
// CReportItem

IMPLEMENT_SERIAL(CReportItem, CObject, 0)

CReportItem::CReportItem()
{
	m_nNumbers = 0;
	m_pNumbers = NULL;
};

CReportItem::CReportItem (CString &str, float *pf, int nf)
{
	m_strText  = str;
	m_nNumbers = nf;
	if (nf > 0)
	{
		m_pNumbers = new float[nf];
		for (register int i=0; i < nf; i++)
			m_pNumbers[i] = pf[i];
	}
	else
		m_pNumbers = NULL;
};

CReportItem::~CReportItem()
{
	if (m_pNumbers != NULL)
		delete m_pNumbers;
};

const CReportItem& CReportItem::operator = (const CReportItem& AnItem)
{
	m_strText  = AnItem.m_strText;
	m_nNumbers = AnItem.m_nNumbers;
	if (m_nNumbers > 0)
	{
		m_pNumbers = new float[m_nNumbers];
		for (register int i=0; i < m_nNumbers; i++)
			m_pNumbers[i] = AnItem.m_pNumbers[i];
	};
	return *this;
};

/////////////////////////////////////////////////////////////////////////////
// CColumn

IMPLEMENT_SERIAL(CColumn, CObject, 0)

CColumn::CColumn()
{
	m_iFieldType		= FT_UNKNOWN;
	m_iColInd			= -1;
	m_bNumeric			= FALSE;
	m_bSel				= FALSE;
	m_iSort				= 0;
	m_iColWidth			= -1;	// -1=try best width
};

const CColumn& CColumn::operator = (const CColumn& AColumn)
{
	m_iFieldType		= AColumn.m_iFieldType;
	m_iColInd			= AColumn.m_iColInd;
	m_strFieldName		= AColumn.m_strFieldName;
	m_strDescription	= AColumn.m_strDescription;
	m_strLabel			= AColumn.m_strLabel;
	m_bNumeric			= AColumn.m_bNumeric;
	m_bSel				= AColumn.m_bSel;
	m_iSort				= AColumn.m_iSort;
	m_iColWidth			= AColumn.m_iColWidth;
	// ....
	return *this;
};

int CColumn::GetRowCount (void)
{
	return m_Items.GetSize();
};

int CColumn::AddItem (CString &str, float *pf /*=NULL*/, int nf /*=0*/)
{
	CReportItem NewItem(str, pf, nf);
	return m_Items.Add(NewItem);
};

// Copy the text of item <irow> to <str>
BOOL CColumn::GetItemText (CString &str, int irow)
{
	if (irow >= 0 && irow < m_Items.GetSize())
	{
		str = m_Items[irow].m_strText;
		return TRUE;
	}
	else
		return FALSE;
};

// Get a constant pointer to the text of item <irow>
LPCSTR CColumn::GetItemText (int irow)
{
	if (irow >= 0 && irow < m_Items.GetSize())
		return m_Items[irow].m_strText;
	else
		return NULL;
};

// Correlation list of field names and field type constants
const static struct {
	int t;
	char s[16];
}	s_aFields[]={
{ FT_STRUCTURE_TYPE						, "[S4A13]"			},
{ FT_PEARSON_SYMBOL						, "[S4A12]"			},
{ FT_PARTHE_SYMBOL						, "[S4G1]"			},
{ FT_SPACE_GROUP_NUMBER					, "[S12C21]"		},
{ FT_MODIFICATION						, "[S4B1]"			},
{ FT_COLOR_STRUC						, "[S6I1]"			},
{ FT_DENSITY_CALCULATED					, "[S12D4]"			},
{ FT_DENSITY_MEASURED					, "[S12H3]"			},
{ FT_CONGRUENT_MELTING_POINT			, "[S6A3]"			},
{ FT_PERITECTIC_TEMPERATURE				, "[S6B3]"			},
{ FT_PERITECTOID_TEMPERATURE			, "[S6C3]"			},
{ FT_ATOMIC_ENVIRONMENT					, "[S12F214]"		},	// TO BE CHECKED!!
{ FT_COUNT_ON_DISTINCT_AE				, "[S12F214]"		},	// TO BE CHECKED!!
{ FT_COORDINATION_NUMBER				, "[S12F213]"		},
{ FT_NIGGLIS_REDUCED_CELL				, "[S12E720]"		},
{ FT_MISMATCH_STRUCTURE					, "[S4P]"			},
{ FT_ORDER_DISORDER_STRUCTURE			, "[S4Q]"			},
{ FT_MAGNETIC_STRUCTURE					, "[S4S1]"			},
{ FT_STOICHIOMETRY						, "[S4A52]"			},	// TO BE CHECKED!!

{ FT_FORMERS_NONFORMERS					, "[FNF]"			},	// TO BE CHECKED!!

{ FT_FERROELASTICITY					, "[P3P]"			},
{ FT_HYDROGEN_DIFFUSION					, "[P4S]"			},
{ FT_DIFFUSION							, "[P4T]"			},
{ FT_IONIC_CONDUCTION					, "[P5Y]"			},
{ FT_PHOTOCONDUCTIVITY					, "[P5L]"			},
{ FT_NONLINEAR_OPTICAL_PROPERTIES		, "[P6M]"			},
{ FT_LUMINESCENCE						, "[P6P]"			},
{ FT_KONDO_BEHAVIOUR					, "[P8V]"			},
{ FT_RESONANCE_MEASUREMENTS				, "[P8X]"			},
{ FT_DE_HAAS_VAN_ALPHEN_MEASUREMENTS	, "[P8Z]"			},
{ FT_CRITICAL_CURRENT					, "[P9J]"			},
{ FT_HARDNESS							, "[P3B3]"			},
{ FT_YOUNGS_MODULUS						, "[P3H15]"			},
{ FT_SHEAR_MODULUS						, "[P3L5]"			},
{ FT_POSSIONS_RATIO						, "[P3M1]"			},
{ FT_ISOBARIC_VOLUME_THERMAL_EXPANSION	, "[P4A35]"			},
{ FT_ENTHALPY_OF_COMPOUND_FORMATION		, "[P4D3]"			},
{ FT_DEBYE_TEMPERATURE					, "[P4H13]"			},
{ FT_SEEBECK_COEFFICIENT				, "[P4Q3]"			},
{ FT_THERMAL_CONDUCTIVITY				, "[P4R5]"			},
{ FT_ENERGY_GAP							, "[PD15]"			},
{ FT_ELECTRICAL_CONDUCTIVITY			, "[P5H5]"			},
{ FT_ELECTRICAL_RESISTIVITY				, "[P5I5]"			},
{ FT_HALL_COEFFICIENT					, "[P5O5]"			},
{ FT_ELECTRON_MOBILITY					, "[P5V5]"			},
{ FT_COLOR_PROP							, "[P6A1]"			},
{ FT_STATIC_DIELECTRIC_CONSTANT			, "[P6F11]"			},
{ FT_WORK_FUNCTION						, "[P6O3]"			},
{ FT_FERROELECTRIC_CURIE_TEMPERATURE	, "[P7A3]"			},
{ FT_DIELECTRIC_PERMITTIVITY			, "[P7C1]"			},
{ FT_PIEZOELECTRIC_STRAIN_CONSTANT		, "[P7D]"			},
{ FT_PYROELECTRIC_COEFFICIENT			, "[P7E]"			},
{ FT_MAGNETIC_SUSCEPTIBILITY			, "[P8A13]"			},
{ FT_PARAMAGNETIC_CURIE_TEMPERATURE		, "[P8C3]"			},
{ FT_NEEL_TEMPERATURE					, "[P8D13]"			},
{ FT_CURIE_TEMPERATURE					, "[P8E13]"			},
{ FT_SATURATION_MAGNETIZATION			, "[P8J17]"			},
{ FT_CRITICAL_TEMPERATURE				, "[P9A3]"			},
{ FT_COHERENCE_LENGTH					, "[P9K3]"			},
{ FT_LONDON_PENETRATION_DEPTH			, "[P9L3]"			}
};

int CColumn::SetType (void)
{
	for (register int i=0, n=sizeof(s_aFields)/sizeof(s_aFields[0]); i < n; i++)
	{
		if (_stricmp(m_strFieldName, s_aFields[i].s) == 0)
		{
			m_iFieldType = s_aFields[i].t;
			return m_iFieldType;
		}
	};
	return FT_UNKNOWN;
};

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryDoc

IMPLEMENT_DYNCREATE(CDiscoveryDoc, CDocument)

BEGIN_MESSAGE_MAP(CDiscoveryDoc, CDocument)
	//{{AFX_MSG_MAP(CDiscoveryDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryDoc construction/destruction

CDiscoveryDoc::CDiscoveryDoc()
{
	// TODO: add one-time construction code here

}

CDiscoveryDoc::~CDiscoveryDoc()
{
}

void CDiscoveryDoc::DeleteContents() 
{
	m_nQsdMaxNeighbours/*Single*/ = 0;
	//m_nQsdMaxNeighboursMultiple = 0;
	m_QsdProfiles.RemoveAll();
	m_iQsdSelProfile = -1;
	m_afQsdScalings[0] = FErr;
	m_afQsdScalings[1] = FErr;
	m_afQsdScalings[2] = FErr;
	m_iQsdSlabAxis	   = 0;
	m_afQsdSlabRange[0]= m_afQsdSlabRange[1] = FErr;

	m_QsdRecords.RemoveAll();
	m_astrQsdProperties.RemoveAll();

	m_QsdResultsReport.Empty();

	m_astrQsdLegend.RemoveAll();

	m_iRunType = 0;		// will be set after call to CQsdRunSheet::DoModal()
	m_iProbFilter = -1;	// will be set in CHistogramView::OnDraw()
	
	CDocument::DeleteContents();
}

BOOL CDiscoveryDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	m_iFileFormat			= FILEFMT_UNKNOWN;
	m_dwFileVersionNumber	= s_dwVersionNumber;

	// Compare OnOpenDocument()
	BOOL bSuccess			= Init();
	if ( !bSuccess )
	{
		// Display error message for failed initialization...
	};

	return bSuccess;
}

// Before the actual serialization starts (from within CDocument::OnOpenDocument()),
// check the format of the file <lpszPathName>.
BOOL CDiscoveryDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
	// Compare MFC source file DOCCORE.CPP for CDocument::OnOpenDocument()
	CFileException fe;
	CFile* pFile = GetFile(lpszPathName, CFile::modeRead | CFile::shareDenyWrite, &fe);
	if (pFile == NULL)
	{
		ReportSaveLoadException(lpszPathName, &fe, FALSE, AFX_IDP_FAILED_TO_OPEN_DOC);
		return FALSE;
	};

	// Set the file format of <lpszPathName> to unknown unless recognized
	m_iFileFormat = FILEFMT_UNKNOWN;

	// Check for the header of the binary native Discovery format "DSC vX.Y<EOF>".
	// The total size of the header is DSC_HEADERSIZE bytes, with the rest filled
	// with zeros.
	char szHeader[DSC_HEADERSIZE];
	memset(szHeader, 0, sizeof(szHeader));
	pFile->Read(szHeader, DSC_HEADERSIZE);
	if (_strnicmp(szHeader, "DSC", 3) == 0)
	{
		// Evaluate version number following "DSC"
		UINT uLow, uHigh;
		if (sscanf(szHeader, "%*s v%u.%u", &uHigh, &uLow) == 2)
		{
			m_dwFileVersionNumber	= MAKELONG((WORD)uLow, (WORD)uHigh);
			m_iFileFormat			= FILEFMT_DSC;
		}
	}

	// If the header begins with "QSD", this is a special file that is not
	// used for input for the normal LPF-CD-ROM but for evaluation purposes
	// (e.g. by U.S. Air Force).
	else if (_strnicmp(szHeader, "QSD", 3) == 0)
	{
		m_iFileFormat = FILEFMT_QSD;
	}

	// Otherwise interpret file data as CSV file containing the rows and columns
	// for the Report View.
	else
	{
		m_iFileFormat = FILEFMT_CSV;
	};

	// Examination of the file format is now complete, so close the file.
	ReleaseFile(pFile, FALSE);

	if (m_iFileFormat == FILEFMT_UNKNOWN)
	{
		ReportSaveLoadException(lpszPathName, NULL, FALSE, IDS_UNKNOWN_FILE_FORMAT);
		return FALSE;
	};

	// The serialization is called from within CDocument::OnOpenDocument().
	BOOL bSuccess = CDocument::OnOpenDocument(lpszPathName);

	// If an error has occurred during serialization, the file format has been
	// reset to "unknown". In that case show an error message.
	if (m_iFileFormat == FILEFMT_UNKNOWN)
	{
		ReportSaveLoadException(lpszPathName, NULL, FALSE, IDS_LOAD_ERROR);
		return FALSE;
	};

	// Finally initialize some data fields and additional data from auxiliary files,
	// for example for QSD
	if ( bSuccess )
	{
		bSuccess = Init();
		if ( !bSuccess )
		{
			// Display error message for failed initialization...
		}
	}
	
	return bSuccess;
}

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryDoc serialization

void CDiscoveryDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// When called for saving data, CDiscoveryDoc::Serialize() is only called
		// for the native, binary DSC format.
		SerializeDsc(ar);
	}
	else	// loading
	{
		// The file format must have been checked by CDiscoveryDoc::OnOpenDocument()
		ASSERT( m_iFileFormat != FILEFMT_UNKNOWN );

		if		(m_iFileFormat == FILEFMT_DSC)
			SerializeDsc(ar);
		else if (m_iFileFormat == FILEFMT_CSV)
			SerializeCsv(ar);
		else if (m_iFileFormat == FILEFMT_QSD)
			SerializeQsd(ar);
		else
		{
			TRACE("CDiscoveryDoc::Serialize() called for unsupported file format %d\n",
				m_iFileFormat);
		}
	}
}

// Loads or stores data in native, binary DSC format
void CDiscoveryDoc::SerializeDsc (CArchive& ar)
{
	char szHeader[DSC_HEADERSIZE];
	if (ar.IsStoring())
	{
		// First write a DSC_HEADERSIZE byte sized header beginning with "DSC vX.Y.<EOF>",
		// where X.Y describes the current/highest version number.
		memset (szHeader, 0, DSC_HEADERSIZE);
		sprintf (szHeader, "DSC v%u.%u\x1A", HIWORD(s_dwVersionNumber), LOWORD(s_dwVersionNumber));
		ar.Write(szHeader, DSC_HEADERSIZE);

		// Now write the actual data of the document
		// ....
	}
	else	// loading
	{
		// The header has just been checked during CDiscoveryDoc::OnOpenDocument()
		ar.Read(szHeader, DSC_HEADERSIZE);

		// Now read the actual data for the document
		// ....
	}
};

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryDoc::SerializeCsv()
// Loads or stores data in CSV format. These are the data for the Report View.
// The following description is taken from email to Van and Tuan (21 Jan 1999).
//
// The protocol file (flat file) is used to create a "Report View" (in Visual
// C++/MFC: a CListCtrl object). But Discovery needs some more information that
// will be not visible for the LPF-CD-ROM user. These additional informations
// are:
//
// a) the LPF-CD-ROM database field names for each column, and
// b) the stoichiometry of each compound.
//
// Thus the protocol file will have the following format:
//
// - First line: Database field names of the columns, separated by semicolons.
//   (Discovery needs these informations to recognize some of the LPF-CD-ROM
//   database fields for evaluations in the Chemistry and QSD tools.)
// - Second line: Corresponding labels of the columns, each label included in
//   "", labels separated by semicolons. (These labels will be used to display
//   the text in the column headers.)
// - Third and following lines: The field contents, separated by semicolons,
//   textual information included in "", numerical information without "".
//   (Discovery uses the presence/absence of "" to sort the fields alphabetically
//   and numerically, rsp.)
//
// - First column: Stoichiometry field. This has the general form "AxBy", and
//   must be composed from the four database fields [S4A51] (A), [S4A52] (x),
//   [S4A53] (B), and [S4A54] (y). The contents of such an "AxBy" field is
//   generated by the expression:
//
//   [S4A51]||[S4A52]||[S4A53]||[S4A54]
//
//   But I will take only the first field name [S4A51] as representative to
//   identify the column.
//
// - Second column: Sentrynumber [sentrynumber].
// - Third column: Formula [S4A22].
//
// The first three columns will always be included, the rest is defined by the
// user in setting his restraints conditions (4th, 5th, etc. column).
//
// A protocol file will look like this (in this example, the LPF-CD-ROM user
// has set coden and melting point as restraints, and there are two entries
// matching.)
//
//   [S4A51];sentrynumber;[S4A22];[D3A];[S6A3]
//   "Stoichiometry";"Sentrynumber";"Formula";"Coden";"Melting point"
//   "AB2";125666;"Al2Ge4";"ACCRA9";1256
//   "A2B";155655;"Cu2Pb";"ZEMTAE";560
//
// In the Report View of Discovery this will look in principal like this:
//    Sentrynumber    |      Formula   |    Coden    |   Melting point
//    125666          |      Al2Ge4    |     ACCRA9  |   1256
//    155655          |      Cu2Pb     |     ZEMTAE  |   560

void CDiscoveryDoc::SerializeCsv (CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else	// loading
	{
		CString strLine;
		CStringArray strItems;	// collects the items of one line, separated by semicolons

		// Read the first line and try to identify those columns that can be used
		// for evaluation in the QSD and Chemistry tool.
		ar.ReadString(strLine);
		strLine.TrimRight();
		strLine.TrimLeft();
		int nColumns = GetCsvItems(strLine, strItems, TRUE);	// TRUE: remove ""

		// There must be at least three columns
		if (nColumns < 3)
		{
			ReportSaveLoadException(GetPathName(), NULL, FALSE, IDS_CSV_NEED_AT_LEAST_THREE_COLUMNS);
			m_iFileFormat == FILEFMT_UNKNOWN;
			return;
		};

		// The first column must contain the stoichiometries of the compounds.
		// This must be indicated by a string containing "[S4A51]". "[S4A52]",
		// "[S4A53]", and "[S4A54]" will also be permitted here.
		if ( !(strItems[0].Find("S4A51") >= 0 || strItems[0].Find("S4A52") >= 0 ||
			   strItems[0].Find("S4A53") >= 0 || strItems[0].Find("S4A54") >= 0) )
		{
			ReportSaveLoadException(GetPathName(), NULL, FALSE, IDS_CSV_FIRST_COLUMN_NOT_STOICHIOMETRY);
			m_iFileFormat == FILEFMT_UNKNOWN;
			return;
		};

		// Initialize the stoichiometry column to be the very first column
		CColumn colStoich;
		colStoich.m_strFieldName = "[S451]";
		colStoich.m_iFieldType	 = FT_INTERNAL_STOICHIOMETRY;
		m_ReportColumns.Add(colStoich);

		// Identify the residual <nColumns>-1 columns
		int ni = m_DatabaseFieldNames.GetSize();
		for (register int icol=1; icol < nColumns; icol++)
		{
			// For each column, define a column info, which will be used to identify
			// the type of information as well as the data items.
			CColumn column;
			column.m_strFieldName = strItems[icol];

			// Look if current field type can be evaluated for QSD and Chemistry
			for (register int ii=0; ii < ni; ii++)
			{
				CString strName;
				strName = m_DatabaseFieldNames[ii];
				if (strName.Find(column.m_strFieldName) >= 0)
				{
					// Identify the field type from the field name, e.g.
					// [S6A3] (congruent melting point) -> FT_CONGRUENT_MELTING_POINT
					column.SetType();

					// Extract the description of that field. This is separated by blanks
					// or tabs from the preceding database field name.
					char *cp = strName.GetBuffer(strName.GetLength()+1);
					while ( !isspace(*cp++) );
					while ( isspace(*cp++) );
					if (*cp != '\0')	// cp now points to the first character of description
						column.m_strDescription = cp;
					strName.ReleaseBuffer();
				}
			};

			// Set the index of the column in the Report View. The initial indexes
			// will be 0,1,2... but may be swapped by header drag and drop operations.
			column.m_iColInd = icol - 1;

			// Add column description. (Other descriptors for <column> will come later.)
			m_ReportColumns.Add(column);
		};

		ASSERT( m_ReportColumns.GetSize() == nColumns );

		// The second line contains the labels for each column. These labels will
		// be displayed in the headers of the Report View (CListCtrl).
		ar.ReadString(strLine);
		strLine.TrimRight();
		strLine.TrimLeft();
		strItems.RemoveAll();
		int nLabels = GetCsvItems(strLine, strItems, TRUE);

		// At this point we simply assume that the number of labels corresponds to
		// the number of columns. If there are less labels, the residual labels
		// will keep blank. (But there must not be more labels than columns!)
		if (nLabels != nColumns)
		{
			TRACE("CDiscoveryDoc::SerializeCsv() - %d labels for %d columns!\n",
				nLabels, nColumns);
			if (nLabels > nColumns)
				nLabels = nColumns;
		};

		ASSERT( m_ReportColumns.GetSize() >= nLabels );

		for (icol=0; icol < nLabels; icol++)
		{
			m_ReportColumns[icol].m_strLabel = strItems[icol];
		};

		// The third line is the first line containing the field contents of the
		// first compound. We use this line to decide, if informations are to be
		// treated as text (when they are included in "") or as numbers.
		ar.ReadString(strLine);
		strLine.TrimRight();
		strLine.TrimLeft();
		strItems.RemoveAll();
		int nItems = GetCsvItems(strLine, strItems, FALSE);	// do not remove the ""

		if (nItems != nColumns)
		{
			TRACE("CDiscoveryDoc::SerializeCsv() - %d items in third row for %d columns!\n",
				nItems, nColumns);
			if (nItems > nColumns)
				nItems = nColumns;
		};

		ASSERT( m_ReportColumns.GetSize() >= nItems );

		for (icol=0; icol < nItems; icol++)
		{
			CColumn* pColumn = &m_ReportColumns[icol];

			// There might be preceding or trailing whitespaces left that have
			// not been removed by the last call to GetCsvItems().
			CString strItem;
			strItem = strItems[icol];
			strItem.TrimRight();
			strItem.TrimLeft();

			// Are both first and last character " ?
			int nl = strItem.GetLength();
			if (strItem[0] == '\"' && strItem[nl-1] == '\"')
			{
				strItem.SetAt(0, ' ');
				strItem.SetAt(nl-1, ' ');
				strItem.TrimRight();
				strItem.TrimLeft();
				pColumn->m_bNumeric = FALSE;
				pColumn->AddItem(strItem);
			}
			else
			{
				pColumn->m_bNumeric = TRUE;

				// Try to extract the number or an array of numbers.
				// These numbers will be used for intelligent sorting.
				float afNumbers[MAX_REPORT_ITEM_NUMBERS];
				int nNumbers = ExtractNumbers(strItem, afNumbers, MAX_REPORT_ITEM_NUMBERS);

				// Add the original string of the item and the evaluated array of numbers
				pColumn->AddItem(strItem, afNumbers, nNumbers);
			}
		};

		// Now collect the items from the fourth, fifth, etc. line.

		// CArchive::ReadString() will throw a CArchiveException when end of file
		// is encountered (the Debug version traces this as "First chance exception").
		// ReadString() catches this exception and returns NULL in that case.

		while ( ar.ReadString(strLine) )
		{
			strLine.TrimRight();
			strLine.TrimLeft();
			strItems.RemoveAll();

			// Here we remove the "" indicating a text field, since we assume that
			// if an item is indicated as text in the third row, it is also indicated
			// as text field in the forth, fifth, etc. row.
			nItems = GetCsvItems(strLine, strItems, TRUE);

			if (nItems != nColumns)
			{
				TRACE("CDiscoveryDoc::SerializeCsv() - %d items in row number %d for %d columns!\n",
					nItems, m_ReportColumns[0].GetRowCount(), nColumns);
				if (nItems > nColumns)
					nItems = nColumns;
			};

			ASSERT( m_ReportColumns.GetSize() >= nItems );

			for (icol=0; icol < nItems; icol++)
			{
				CColumn* pColumn = &m_ReportColumns[icol];

				// Try to evaluate numerical informations
				int nNumbers = 0;
				float afNumbers[MAX_REPORT_ITEM_NUMBERS];
				if ( pColumn->m_bNumeric )
				{
					nNumbers = ExtractNumbers(strItems[icol], afNumbers, MAX_REPORT_ITEM_NUMBERS);
				};

				pColumn->AddItem(strItems[icol], afNumbers, nNumbers);
			}
		};

		TRACE("CDiscoveryDoc::SerializeCsv() - %d columns with %d rows read.\n",
			m_ReportColumns.GetSize(), m_ReportColumns[0].GetRowCount());
	}
};

// Callback function to sort QSD records for:
//   1st preference: increasing element number,
//   2nd preference: element symbols alphabetically,
//   3rd preference: stoichiometric numbers,
//   4th preference: experimental < to be predicted.
static int _SortQsdRecords (const void* elem1, const void* elem2)
{
	const CQsdRecord *q1 = (const CQsdRecord*)elem1;
	const CQsdRecord *q2 = (const CQsdRecord*)elem2;
	return q1->Compare(*q2, 3);
};

// Loads or stores data in special text format for QSD evaluation purposes
void CDiscoveryDoc::SerializeQsd (CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else	// loading
	{
		CString strMsg;
		char szLine[1024], szCopy[1024];
		ar.ReadString(szLine, sizeof(szLine)-1);	// read the first line containing "QSD"
		ar.ReadString(szLine, sizeof(szLine)-1);	// the second line contains the number of records
		int nRecords = atoi(szLine);

		// A legend for the system properties 0, 1, etc. may be following the
		// list of normal records, after a line with "[Legend]".
		BOOL bLegend = FALSE;

		// (Compare SerializeCsv() for comments about CArchiveException.)
		while ( ar.ReadString(szLine, sizeof(szLine)-1) )
		{
			CQsdRecord QsdRecord;

			// Each record consists of N*2+1 items for N elements, e.g.:
			// "Al;1;Be;2.333;C;2.5;0". The last value comes after the last semicolon.
			if (_stricmp(szLine, "[Legend]") == 0)
			{
				bLegend = TRUE;
				break;
			}

			char *cp = strrchr(szLine, ';');
			ASSERT( cp != NULL );
			ASSERT( *(cp+1) != '\0' );
			QsdRecord.m_fProp = atof(cp+1);

			// Cut the property value so that it is not interpreted as element
			*cp	= '\0';

			// Now collect the elements and their stoichiometric indexes.
			// (We use a copy of <szLine> for the case that a syntax error is encountered.)
			strcpy(szCopy, szLine);
			char *pElem	= strtok(szCopy, ";"), *pStoich;
			BOOL bOk	= TRUE;
			while (pElem != NULL && bOk )
			{
				pStoich	 = strtok(NULL, ";");
				int irtc = QsdRecord.AddElement(pElem, pStoich);
				if (irtc < 0)
				{
					bOk = FALSE;
					if (irtc == -2)
						strMsg.Format("Syntax error in line \"%s\"!\nContinue?", szLine);
					else
						strMsg.Format("Too many elements in line \"%s\"!\nContinue?", szLine);
					if (AfxMessageBox(strMsg, MB_YESNO) == IDNO)
						goto ReadErr;
				};
				pElem	 = strtok(NULL, ";");
			};

			if ( bOk )
			{
				// Make the sum of stoichiometric factors = 100
				QsdRecord.StdStoich();

				m_QsdRecords.Add(QsdRecord);
			}
		};

		// Read the legend - if found
		if ( bLegend )
		{
			m_astrQsdLegend.SetSize(0,8);
			while ( ar.ReadString(szLine, sizeof(szLine)-1) )
			{
				m_astrQsdLegend.Add(szLine);
			}
		};

ReadErr:

		TRACE("CDiscoveryDoc::SerializeQsd() - %d records mentioned, %d records assigned, %d records in legend\n",
			nRecords, m_QsdRecords.GetSize(), m_astrQsdLegend.GetSize());

		// Now sort the records for increasing element numbers (1st preference),
		// element symbols (alphabetically, 2nd preference), and stoichiometric
		// indexes (3rd preference).
		qsort(m_QsdRecords.GetData(), m_QsdRecords.GetSize(), sizeof(CQsdRecord),
			  _SortQsdRecords);

		// Now we check the records for doublets
		CString strRec, strErr;
		CStringArray astrErrors;
		for (register int irec=1, nrec=m_QsdRecords.GetSize(); irec < nrec; irec++)
		{
			if (m_QsdRecords[irec-1].Compare(m_QsdRecords[irec], 3) == 0)
			{
				strErr.Format("#%d: %s", irec+1, m_QsdRecords[irec].Format(strRec));
				astrErrors.Add(strErr);
			}
		};
		int irtc	= 0;
		int nErrors	= astrErrors.GetSize();
		if (nErrors > 0)
		{
			strMsg.Format("%d doublets found in input file \"%s\":\n",
						  nErrors, (LPCSTR)ar.GetFile()->GetFileName());
			if (nErrors > 20)
			{
				nErrors = 21;
				astrErrors[20] = "(more...)\n";
			};
			for (register int ierr=0; ierr < nErrors; ierr++)
				strMsg += astrErrors[ierr];
			strMsg += "Remove doublets?";
			irtc = IDNO;
//			irtc = AfxMessageBox(strMsg, MB_YESNO);
		};
		if (irtc == IDYES)
		{
			for (irec=m_QsdRecords.GetSize()-1, nErrors=0; irec >= 1; irec--)
			{
				if (m_QsdRecords[irec-1].Compare(m_QsdRecords[irec], 3) == 0)
				{
					m_QsdRecords.RemoveAt(irec);
					nErrors++;
				}
			};
			strMsg.Format("%d doublets have been removed", nErrors);
			AfxMessageBox(strMsg);
		};

#ifdef _DEBUG
		for (irec=1, nrec=m_QsdRecords.GetSize(); irec < nrec; irec++)
		{
			if (m_QsdRecords[irec-1].Compare(m_QsdRecords[irec], 3) >= 0)
			{
				TRACE("CDiscoveryDoc::SerializeQsd() - m_QsdRecord[%d] > m_QsdRecord[%d]\n",
					irec-1, irec);
			}
		}
#endif
	}
};

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryDoc operations

// Initialize additional data and load data from auxiliary files.
// Returns: Success.
BOOL CDiscoveryDoc::Init (void)
{
	CDiscoveryApp* pApp = (CDiscoveryApp*)AfxGetApp();

	// ....

	// Load elemental properties from file "PROPERTY.DAT"
	CStdioFile f;
	CFileException e;
	CString str;
	if ( f.Open(pApp->m_strQsdPropertyFileName, CFile::modeRead, &e) )
	{
		m_astrQsdProperties.RemoveAll();

		// The first line contains only the number of elemental properties.
		// The following lines define one property each. The residual lines with
		// the actual property values will be ignored here but used by the QSD kernel.
		// The sequential number of the properties is important for the QSD runs,
		// since these numbers will be used by the QSD kernel.
		f.ReadString(str);
		int nprop = atoi(str);
		for (register int i=0; i < nprop; i++)
		{
			if ( f.ReadString(str) )
				m_astrQsdProperties.Add(str);
		};

		// Now there must be <nprop> properties in the property string array
		if (m_astrQsdProperties.GetSize() != nprop)
		{
			AfxFormatString1(str, IDS_COULD_NOT_READ_ALL_PROPERTIES, pApp->m_strQsdPropertyFileName);
			AfxMessageBox(str);
		};

		f.Close();
	}
	else
	{
		ReportFileError(&e);
		return FALSE;
	};

	// Load database field names that can be evaluated by QSD and Chemistry tool
	// from file "DatabaseFields.dat".
	// Not necessary since there will be no Report View in this application!
//	if ( f.Open(pApp->m_strDatabaseFieldsFileName, CFile::modeRead, &e) )
//	{
//		m_DatabaseFieldNames.RemoveAll();

		// Each line in this file should have the format
		// "[<field name>] <description>", e.g. "[S4A13] Structure type".
		// (Field name and description may be separated by blanks or tabs.)
		// Comments in this file are preceded with '#'.
//		while ( f.ReadString(str) )
//		{
//			RemoveComment(str);
//			if ( !str.IsEmpty() )
//			{
//				m_DatabaseFieldNames.Add(str);
//			}
//		};

//		f.Close();
//	}
//	else
//	{
//		ReportFileError(&e);
//		return FALSE;
//	};

	// This will receive the process handle if the QSD kernel is running
	m_hQsdProcess = NULL;

	// If we reach this point, initialization has become o.k.
	return TRUE;
};

// Writes the indexes of the columns in m_ReportColumns[] into the array <pInd>.
// pInd[]={1,3,2} means that m_ReportColumns[1] goes into the first column (#0) of
// the report view's list control object, m_ReportColumns[3] into the second one
// (#1) and m_ReportColumns[2] into the third one (#2).
// Returns the number of indexes written to <pInd>.
int CDiscoveryDoc::GetReportColumnIndexes (int *pInd)
{
	ASSERT( pInd != NULL );

	int nCols = m_ReportColumns.GetSize();
	for (register int icol=0, nind=0; icol < nCols; icol++)
	{
		int iind = m_ReportColumns[icol].m_iColInd;
		if (iind >= 0)	// columns that do not appear in the report view have index -1
		{
			pInd[iind] = icol;
			nind++;
		}
	};
	return nind;
};

// Reverse function of GetReportColumnIndexes().
// The <nInd> indexes in <pInd> point to columns in m_ReportColumns[], where
// the n-th column in the report view will be assigned to the <pInd[n]>-th
// column in m_ReportColumns[].
// Returns the number of indexes assigned.
int CDiscoveryDoc::SetReportColumnIndexes (int *pInd, int nInd)
{
	ASSERT( pInd != NULL );

	for (register int i=0; i < nInd; i++)
	{
		m_ReportColumns[pInd[i]].m_iColInd = i;
	};

	return nInd;
};

// This function is called after a header drag and drop operation in the report view.
// The column with the previous index <iold> gets the new index <inew>, which means
// that it is removed from the old position and inserted at the new position. Thus
// the indexes of the other columns must be updated.
int CDiscoveryDoc::MoveReportColumnIndex (int iold, int inew)
{
	int aiInd[MAX_REPORT_COLUMNS];
	int nCols = m_ReportColumns.GetSize();
	int nInd  = GetReportColumnIndexes(aiInd);	// should be nCols-1

	//   iold=2, inew=4   ->
	//   1,2,3,4,5,6,7,8  ->
	//   1,2,4,5,3,6,7,8

	// Move the indexes between <iold> and <inew> one position down or up
	int iind = aiInd[iold];
	register int i;
	if (inew > iold)
	{
		for (i=iold; i < inew; i++)
			aiInd[i] = aiInd[i+1];
	}
	else
	{
		for (i=iold; i > inew; i--)
			aiInd[i] = aiInd[i-1];
	};

	// Set the previous index at position <iold> to position <inew>
	aiInd[inew] = iind;
	SetReportColumnIndexes(aiInd, nInd);

	return inew;
};

// Init the records of the main input file of the QSD kernel. Previous contents
// will be removed.
// Returns the number of created records.

int CDiscoveryDoc::CreateQsdRecords (void)
{
	m_QsdRecords.RemoveAll();

	// TO BE CHECKED

	return m_QsdRecords.GetSize();
};

// Callback function to sort the profiles in <CDiscoveryDoc::m_QsdProfiles>
static int _SortProfiles (const void* elem1, const void* elem2)
{
	const CQsdProfile *p1 = (const CQsdProfile*)elem1;
	const CQsdProfile *p2 = (const CQsdProfile*)elem2;
	int imax1			  = (int)p1->m_pfValues[p1->GetValueCount()-1];
	int imax2			  = (int)p2->m_pfValues[p2->GetValueCount()-1];
	return ((imax1 < imax2) ? 1 : (imax1 > imax2) ? -1 : 0);	// maximum value comes first!
};

// Read the profiles from the file <lpszFile> (usually "PROFILES.DAT").
// <bMultiple> is used to distinguish between Single Run (FALSE) and Multiple Runs
// (TRUE), so Discovery knows where to store the maximum number of neighbours.
// Returns the number of profiles; -1 when file read error.

int CDiscoveryDoc::ReadQsdProfiles (LPCSTR lpszFile, BOOL bMultiple)
{
	m_QsdProfiles.RemoveAll();

	// "QSD PROFILE";1;2;3;4;5;...
	// "01-01 / 01-02 / 02-02 ";5787(0);5156(4);4720(9);4325(12);3998(14);...
	// "01-01 / 01-02 / 02-01 ";5719(0);5193(1);4796(2);4464(5);4219(6);...
	CStdioFile f;
	CFileException e;
	if ( !f.Open(lpszFile, CFile::modeRead | CFile::typeText, &e) )
	{
		ReportFileError(&e);
		return -1;
	};

	CString strLine;
	CStringArray strItems;

	// Get the "number of neighbours" from the first line. Each line in the profile
	// has <nItems>+1 items (that means columns).
	f.ReadString(strLine);
	int nItems = GetCsvItems(strLine, strItems, TRUE) - 1;
	ASSERT( strItems[0] == "QSD PROFILE" );
	
	// Now read the profiles, one profile for each line.
	// The first item describes the indexes of the properties and operations for the
	// three axes x,y,z, followed by <nItems>.
	while ( f.ReadString(strLine) )
	{
		int ip1, ip2, ip3, io1, io2, io3;
		if ( strLine.IsEmpty() )
			continue;
		strItems.RemoveAll();
		VERIFY( GetCsvItems(strLine, strItems, TRUE) == nItems+1 );
		VERIFY( sscanf(strItems[0], "%d-%d / %d-%d / %d-%d", &ip1, &io1, &ip2, &io2, &ip3, &io3) == 6 );

		CQsdProfile Profile(nItems, ip1, io1, ip2, io2, ip3, io3);
		for (register int i=0; i < nItems; i++)
		{
			VERIFY( Profile.AddValue(strItems[i+1]) >= 0 );
		};
		m_QsdProfiles.Add(Profile);
	};

	TRACE("CDiscoveryDoc::ReadQsdProfiles() - %d profiles read\n", m_QsdProfiles.GetSize());

	// We sort the profiles for decreasing best values (last item).
	// There may be more than "NumBestResults" profiles in the profile file.
	// In this case we delete the superfluous profiles.
	int nProfiles = m_QsdProfiles.GetSize();
	qsort(m_QsdProfiles.GetData(), nProfiles, sizeof(CQsdProfile),
		  _SortProfiles);
	int nBestResults = AfxGetApp()->GetProfileInt(s_szQsd, "NumBestResults", 20);
	if (nBestResults < nProfiles)
		m_QsdProfiles.SetSize(nBestResults);

//	if ( bMultiple )
//		m_nQsdMaxNeighbours/*Multiple*/ = nItems;
//	else
//		m_nQsdMaxNeighbours/*Single*/   = nItems;

	return m_QsdProfiles.GetSize();
};

// Updates the profile with the index <iProfile> with the contents of the file
// <lpszFile> (usually "PROFILES.DAT"). This is normally called after QSD.EXE has
// been called for one of the previously calculated multiple runs.
// Returns the number of neighbours regarded or -1 when file read error or -2 when
// profile index <iProfile> is wrong.

int CDiscoveryDoc::UpdateQsdProfile (LPCSTR lpszFile, int iProfile)
{
	CStdioFile f;
	CFileException e;
	if ( !f.Open(lpszFile, CFile::modeRead | CFile::typeText, &e) )
	{
		ReportFileError(&e);
		return -1;
	};

	if (iProfile <= 0 || iProfile >= m_QsdProfiles.GetSize())
		return -2;

	CQsdProfile* pProfile = &m_QsdProfiles[iProfile];

	// Compare <CDiscoveryDoc::ReadQsdProfiles()>
	CString strLine;
	CStringArray strItems;
	f.ReadString(strLine);
	int nItems = GetCsvItems(strLine, strItems, TRUE) - 1;
	ASSERT( strItems[0] == "QSD PROFILE" );
	while ( f.ReadString(strLine) )
	{
		int ip1, ip2, ip3, io1, io2, io3;
		if ( strLine.IsEmpty() )
			continue;
		strItems.RemoveAll();
		VERIFY( GetCsvItems(strLine, strItems, TRUE) == nItems+1 );
		VERIFY( sscanf(strItems[0], "%d-%d / %d-%d / %d-%d", &ip1, &io1, &ip2, &io2, &ip3, &io3) == 6 );

		CQsdProfile NewProfile(nItems, ip1, io1, ip2, io2, ip3, io3);
		for (register int i=0; i < nItems; i++)
		{
			VERIFY( NewProfile.AddValue(strItems[i+1]) >= 0 );
		};
		*pProfile = NewProfile;
	};
	//m_nQsdMaxNeighbours/*Single*/ = nItems;
	return nItems;
};

// Read the QSD Results Report from the file <lpszFile> (usually "RESULTS.DAT").
// Returns the file size or -1 when read error.

long CDiscoveryDoc::ReadQsdResults (LPCSTR lpszFile)
{
	CFile f;
	CFileException e;
	if ( !f.Open(lpszFile, CFile::modeRead | CFile::typeBinary, &e) )
	{
		ReportFileError(&e);
		return -1;
	};

	// We read the complete file in one step and store it in one single string
	// object with multiline text.
	DWORD dwSize		= f.GetLength();
	char *lpszBuf		= new char[dwSize+1];
	f.Read(lpszBuf, dwSize);
	lpszBuf[dwSize]		= '\0';
	m_QsdResultsReport	= lpszBuf;
	delete lpszBuf;

	return (long)dwSize;
};

// Read the QSD Discovery Space from the file <lpszFile> (usually "QSDOUT.DAT").
// This is the result of a single QSD run and contains one coordinates triplet
// for every compound (system). For compounds whose properties are to be predicted,
// the predicted property and the probability follow as 4th and 5th value, rsp.
// Returns the number of coordinate triplets or -1 when read error.

int CDiscoveryDoc::ReadQsdDiscoverySpace (LPCSTR lpszFile)
{
	CStdioFile f;
	CFileException e;
	if ( !f.Open(lpszFile, CFile::modeRead | CFile::typeBinary, &e) )
	{
		ReportFileError(&e);
		return -1;
	};

	CString strLine;

	// The first line is the identifier "QSD DISCOVERY SPACE"
	f.ReadString(strLine);
	VERIFY( _strnicmp(strLine, "QSD DISCOVERY SPACE", strlen("QSD DISCOVERY SPACE")) == 0 );

	// The second line contains the scaling factors for x, y, and z-axis,
	// "256.000; 60.667;  1.433" leads to absolute values of 128, 30.334, 0.717,
	// if the coordinate triplet for a compound is 50, 50, 50.
	float fXScale=1.0f, fYScale=1.0f, fZScale=1.0f;
	f.ReadString(strLine);
	VERIFY( sscanf(strLine, "%f;%f;%f", &fXScale, &fYScale, &fZScale) == 3 );
	
	// The third, fourth, etc., line contain the 0-th, 1st, etc., coordinate
	// triplet for the 0-th, 1st, etc., compound in m_QsdRecords[].
	for (register int iq=0, nq=m_QsdRecords.GetSize(); iq < nq; iq++)
	{
		if ( !f.ReadString(strLine) )
		{
			TRACE("CDiscoveryDoc::ReadQsdDiscoverySpace() - no coordinates for QSD record #%d\n",
				iq);
			break;
		};

		float x, y, z, prop, prob;
		CQsdRecord *pRecord = &m_QsdRecords[iq];
		int nval = sscanf(strLine, "%f;%f;%f;%f;%f", &x, &y, &z, &prop, &prob);
		ASSERT( (pRecord->m_bPredict && nval == 5) || (!pRecord->m_bPredict && nval >= 3) );

		// -1,-1,-1 indicates that the point has undefined coordinates
		if (x == -1.0f && y == -1.0f && z == -1.0f)
		{
			pRecord->m_bNoCoords = TRUE;
		}
		else
		{
			pRecord->m_afXyz[0]  = x * fXScale * 0.01f;
			pRecord->m_afXyz[1]  = y * fYScale * 0.01f;
			pRecord->m_afXyz[2]  = z * fZScale * 0.01f;
			pRecord->m_afNorm[0] = x * 0.01f;			// normalized coordinates (0..1)
			pRecord->m_afNorm[1] = y * 0.01f;
			pRecord->m_afNorm[2] = z * 0.01f;
			pRecord->m_bNoCoords = FALSE;

			if ( pRecord->m_bPredict )
			{
				pRecord->m_fProp = prop;
				pRecord->m_fProb = prob;
			}
		}
	};

	// Save the three scaling factors for labelling the axes in <CQsdGraphView>
	m_afQsdScalings[0] = fXScale;
	m_afQsdScalings[1] = fYScale;
	m_afQsdScalings[2] = fZScale;

	return nq;
};

// Removes all compounds with experimental data from <m_QsdRecords> if <iopt> == 0,
// otherwise it removes all compounds with data to be predicted.
// Returns the number of removed records.
int CDiscoveryDoc::RemoveQsdRecords (int iopt)
{
	CQsdRecordArray NewRecords;
	int nrec = m_QsdRecords.GetSize();
	NewRecords.SetSize(nrec, 256);
	for (register int irec=0, inew=0; irec < nrec; irec++)
	{
		if (iopt == 0)
		{
			if ( m_QsdRecords[irec].m_bPredict )
				NewRecords[inew++] = m_QsdRecords[irec];
		}
		else
		{
			if ( !m_QsdRecords[irec].m_bPredict )
				NewRecords[inew++] = m_QsdRecords[irec];
		}
	};
	m_QsdRecords.SetSize(inew, 256);
	m_QsdRecords.FreeExtra();
	for (irec=0; irec < inew; irec++)
		m_QsdRecords[irec] = NewRecords[irec];
	return (nrec - inew);
};

// Imports compounds from a QSD input file and adds them to m_QsdRecords[].
// These compounds' properties are undefined and will be predicted in QSD.EXE.
// Previous QSD records with the attribute "to be predicted" will be deleted before.
// If <iCheck> is 1, new records will only be added if the elements do not agree
// with the element combination of an existing record. If <iCheck> is 2, it will
// not be added, if both elements and stoichiometry do not match.
// Returns the number of QSD records added to m_QsdRecords[] or
//   -1 for file open error,
//   -2 for illegal input format,
//   -3 for invalid number of records to be imported.
int CDiscoveryDoc::ImportQsdRecords (LPCSTR lpszFilename, int iCheck)
{
	// Import of QSD records and removing doublets may be a lengthy process.
	// So we display a dialog window with status and progress bar.
	CString strStatus;
	CQsdImportDlg dlg;
	dlg.Create(CQsdImportDlg::IDD);
	dlg.ShowWindow(SW_SHOW);

	// First delete all QSD records with <CQsdRecord.m_bPredict> == TRUE.
	// Since deletion with CArray::RemoveAt() may be rather slow, we simply
	// make a copy of the experimental records.
	CQsdRecordArray NewRecords;
	int nrec = m_QsdRecords.GetSize();
	NewRecords.SetSize(nrec, 256);
	dlg.m_What.SetWindowText("Removing previous predictions...");
	for (register int irec=0, inew=0; irec < nrec; irec++)
	{
		if ( !m_QsdRecords[irec].m_bPredict )
			NewRecords[inew++] = m_QsdRecords[irec];
	};
	m_QsdRecords.SetSize(inew, 256);
	m_QsdRecords.FreeExtra();
	for (irec=0; irec < inew; irec++)
		m_QsdRecords[irec] = NewRecords[irec];

	CStdioFile f;
	CString strLine;
	CFileException e;
	if ( !f.Open(lpszFilename, CFile::modeRead | CFile::typeText | CFile::shareDenyWrite, &e) )
	{
		ReportFileError(&e);
		return -1;
	};
	
	f.ReadString(strLine);
	if (strLine.CompareNoCase("QSD") != 0)
		return -2;
	
	f.ReadString(strLine);
	int nNew = atoi(strLine);	// <nNew> = number of records to be imported
	if (nNew <= 0)
		return -3;

	// First we store all the new records in an auxiliary array and sort them
	// like in CDiscoveryDoc::SerializeQsd(). The reason for sorting is to find
	// doublets within the new records.
	NewRecords.SetSize(nNew,256);
	dlg.m_What.SetWindowText("Reading compounds for prediction...");
	dlg.m_Progress.SetRange(0, nNew / 100);	// CProgressCtrl::SetRange() does not work correct with big numbers
	for (inew=0; inew < nNew; inew++)
	{
		// The format is the same as that analyzed in CDiscoveryDoc::SerializeQsd(),
		// but a property value (if any defined) will be ignored.
		CQsdRecord NewRecord;
		NewRecord.m_bPredict = TRUE;
		char szLine[1024], *pElem, *pStoich;
		if ( !f.ReadString(szLine, sizeof(szLine)-1) )
		{
			TRACE("CDiscoveryDoc::ImportQsdRecords() - cannot read record #%d from %s\n",
				irec, lpszFilename);
		};

		// If a digit comes where the next element is expected, this is the property
		// value, which will be ignored here.
		pElem = strtok(szLine, ";");	// e.g. "Al;1;Be;2.333;C;2.5;0"
		while (pElem != NULL && isalpha(pElem[0]) )
		{
			pStoich	= strtok(NULL, ";");
			NewRecord.AddElement(pElem, pStoich);
			pElem	= strtok(NULL, ";");
		};

		// Make the sum of stoichiometric factors = 100
		NewRecord.StdStoich();

		NewRecords[inew] = NewRecord;

		if ((inew % 100) == 0)
		{
			strStatus.Format("%d of %d compounds read for prediction", inew, nNew);
			dlg.m_Status.SetWindowText(strStatus);
			dlg.m_Progress.SetPos(inew / 100);
		}
	};

	// Sort all <nNew> new QSD records
	nNew = NewRecords.GetSize();
	strStatus.Format("Sorting %d compounds...", nNew);
	dlg.m_What.SetWindowText(strStatus);

	qsort(NewRecords.GetData(), nNew, sizeof(CQsdRecord), _SortQsdRecords);

	// TO BE CHECKED - message box for release version
	CString strComp;
	int nDouble = 0;
	strStatus.Format("Checking %d compounds for doublets...", nNew);
	dlg.m_What.SetWindowText(strStatus);
	for (inew=1; inew < nNew; inew++)
	{
		if (NewRecords[inew-1].Compare(NewRecords[inew], 3) == 0)
		{
			TRACE("CDiscoveryDoc::ImportQsdRecords() - doublet found: %s\n",
				NewRecords[inew].FormatComp(strComp));

			// We misuse the <CQsdRecord.m_bSel> flag here to mark doublets
			NewRecords[inew].m_bSel = TRUE;
			nDouble++;

			if ((inew % 100) == 0)
			{
				strStatus.Format("%d of %d compounds checked for doublets", inew, nNew);
				dlg.m_Status.SetWindowText(strStatus);
				dlg.m_Progress.SetPos(inew / 100);
			}
		}
	};

	// If doublets have been found, remove them from the array
	dlg.m_What.SetWindowText("Removing doublets...");
	for (inew=nNew-1; inew >= 0; inew--)
	{
		if ( NewRecords[inew].m_bSel )
			NewRecords.RemoveAt(inew);
	};

	// Now we make a copy of the old records, add the new ones and sort them
	// altogether. We use a copy, since we must remove doublets later down.
	CQsdRecordArray AllRecords;
	nrec = m_QsdRecords.GetSize();
	nNew = NewRecords.GetSize();
	strStatus.Format("Adding %d compounds (prediction) to %d compounds (experimental)",
					 nNew, nrec);
	dlg.m_What.SetWindowText(strStatus);
	AllRecords.SetSize(nrec+nNew, 256);
	for (irec=0; irec < nrec; irec++)
		AllRecords[irec] = m_QsdRecords[irec];
	for (inew=0; inew < nNew; inew++)
		AllRecords[inew+nrec] = NewRecords[inew];

	// Now that all new QSD records from <NewRecords> have been added to <AllRecords>
	// we clear the new QSD records to save memory space.
	NewRecords.RemoveAll();

	nrec = AllRecords.GetSize();

	// TEST: Write QSD records into test files
	CStdioFile fTest;
//	VERIFY( fTest.Open("BeforeSort.dat", CFile::typeText | CFile::modeWrite | CFile::modeCreate) );
//	for (irec=0; irec < nrec; irec++)
//	{
//		strLine.Format("%6d: %s\n", irec, AllRecords[irec].FormatComp(strComp));
//		fTest.WriteString(strLine);
//	};
//	fTest.Close();

	strStatus.Format("Sorting %d compounds...", nrec);
	dlg.m_What.SetWindowText(strStatus);

	qsort(AllRecords.GetData(), nrec, sizeof(CQsdRecord), _SortQsdRecords);

//	VERIFY( fTest.Open("AfterSort.dat", CFile::typeText | CFile::modeWrite | CFile::modeCreate) );
//	for (irec=0; irec < nrec; irec++)
//	{
//		strLine.Format("%6d: %c: %s; %3.2f\n",
//					   irec,
//					   AllRecords[irec].m_bPredict ? 'P' : 'E',
//					   AllRecords[irec].FormatComp(strComp),
//					   AllRecords[irec].m_fProp);
//		fTest.WriteString(strLine);
//	};
//	fTest.Close();

//	for (irec=1; irec < nrec; irec++)
//	{
//		if (AllRecords[irec-1].Compare(AllRecords[irec], 1) == 0)
//		{
//			TRACE("CDiscoveryDoc::ImportQsdRecords() - experimental data found for %s to be predicted\n",
//				AllRecords[irec].FormatComp(strComp));
//		}
//	};

	// Check for doublets between data from experimental compounds and compounds
	// with properties to be predicted.  When double, the compound to be predicted
	// will be removed but the experimental will stay.
	// (For doublets, _SortQsdRecords() lets the one to be predicted come after
	// the experimental one.)
	int nDouble2 = 0;
	dlg.m_What.SetWindowText("Removing doublets...");
	dlg.m_Progress.SetRange(0, nrec / 100);
	for (irec=nrec-2; irec >= 0; irec--)
	{
		if (AllRecords[irec].Compare(AllRecords[irec+1], 1) == 0)
		{
			AllRecords[irec+1].m_bSel = TRUE;
			nDouble2++;

			if ((nDouble2 % 10) == 0)
			{
				strStatus.Format("%d doublets found in %d compounds",
								 nDouble2, nrec-irec);
				dlg.m_Status.SetWindowText(strStatus);
				dlg.m_Progress.SetPos((nrec - irec) / 100);
			}
		}
		else
			AllRecords[irec+1].m_bSel = FALSE;
	};

	// After the doublets have been removed, copy the residual records
	int nold = m_QsdRecords.GetSize();
	m_QsdRecords.SetSize(nrec-nDouble2, 256);
	for (irec=0, inew=0; irec < nrec; irec++)
	{
		if ( !AllRecords[irec].m_bSel )
			m_QsdRecords[inew++] = AllRecords[irec];
	};

	f.Close();

	dlg.DestroyWindow();

	return m_QsdRecords.GetSize() - nold;
};

// Returns the number of dimensions used in the current QSD Discovery Space.
// This is usually 3, but can also be 2 or 1. In these cases, z- (and y-)axis
// have no properties.
int CDiscoveryDoc::GetQsdDimensionCount (void)
{
	if (m_iQsdSelProfile < 0)
		return -1;
	CQsdProfile* pProfile = &m_QsdProfiles[m_iQsdSelProfile];
	int nDim = 3;
	ASSERT( pProfile->m_ip1 > 0 && pProfile->m_io1 > 0 );
	if (pProfile->m_ip2 == 0)
	{
		ASSERT( pProfile->m_io2 == 0 && pProfile->m_ip3 == 0 && pProfile->m_io3 == 0 );
		nDim = 1;
	}
	else if (pProfile->m_ip3 == 0)
	{
		ASSERT( pProfile->m_io3 == 0 );
		nDim = 2;
	};
	return nDim;
};

// Returns the number of QSD records with experimental data if <iopt> == 0,
// otherwise returns the number of QSD records with compounds to be predicted.
int CDiscoveryDoc::GetQsdRecordCount (int iopt)
{
	for (register int i=0, imax=m_QsdRecords.GetSize(), n=0; i < imax; i++)
	{
		if ( (  m_QsdRecords[i].m_bPredict && iopt == 1) ||
			 ( !m_QsdRecords[i].m_bPredict && iopt == 0) )
			 n++;
	};
	return n;
};

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryDoc diagnostics

#ifdef _DEBUG
void CDiscoveryDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CDiscoveryDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryDoc commands
