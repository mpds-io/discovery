// Discovery.h : main header file for the DISCOVERY application
// Ported to Windows 8.1 by Fulcrum.013 Mariupol, Ukraine 
// 17 september 2020

//
#define  _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include <math.h>
#include "resource.h"       // main symbols

// Format of input file
#define FILEFMT_UNKNOWN	-1
#define FILEFMT_DSC		 1
#define FILEFMT_CSV		 2
#define FILEFMT_QSD		 3

// Size of the DSC header
#define DSC_HEADERSIZE  32

// Maximum number of elements in a QSD record
#define MAX_ELEM_QSD	8

// Maximum number of integer properties in a QSD record
#define MAX_SYSPROP_QSD	20
#define MAX_COLOR_QSD	20

// Special ordinal numbers
//#define MAXELE        110		// number of (pseudo) elements
//#define ELEM_DUMMY      0		// 0=dummy element
//#define ELEM_D        108		// deuterium
//#define ELEM_T        109		// tritium

// View types
#define VIEW_REPORT		1	// default view when app starts
#define VIEW_QSD		2	// QSD graphics plus table (using splitter)
#define VIEW_CHEMISTRY	3	// Chemistry Design (2D matrix)
#define VIEW_STATISTICS	4	// Statistics Design

// Maximum number of columns allowed in report view
#define MAX_REPORT_COLUMNS		  100

// Maximum number of numbers associated to a numeric field in report view
#define MAX_REPORT_ITEM_NUMBERS		6

// Identifiers for database field types
#define FT_INTERNAL_STOICHIOMETRY				100	// stoichiometry column, invisible to user

#define	FT_UNKNOWN								-1
#define FT_STRUCTURE_TYPE						 1
#define FT_PEARSON_SYMBOL						 2
#define FT_PARTHE_SYMBOL						 3
#define FT_SPACE_GROUP_NUMBER					 4
#define FT_MODIFICATION							 5
#define FT_COLOR_STRUC							 6
#define FT_DENSITY_CALCULATED					 7
#define FT_DENSITY_MEASURED						 8
#define FT_CONGRUENT_MELTING_POINT				 9
#define FT_PERITECTIC_TEMPERATURE				10
#define FT_PERITECTOID_TEMPERATURE				11
#define FT_ATOMIC_ENVIRONMENT					12
#define FT_COUNT_ON_DISTINCT_AE					13
#define FT_COORDINATION_NUMBER					14
#define FT_NIGGLIS_REDUCED_CELL					15
#define FT_MISMATCH_STRUCTURE					16
#define FT_ORDER_DISORDER_STRUCTURE				17
#define FT_MAGNETIC_STRUCTURE					18
#define FT_STOICHIOMETRY						19

#define FT_FORMERS_NONFORMERS					31

#define FT_FERROELASTICITY						41
#define FT_HYDROGEN_DIFFUSION					42
#define FT_DIFFUSION							43
#define FT_IONIC_CONDUCTION						44
#define FT_PHOTOCONDUCTIVITY					45
#define FT_NONLINEAR_OPTICAL_PROPERTIES			46
#define FT_LUMINESCENCE							47
#define FT_KONDO_BEHAVIOUR						48
#define FT_RESONANCE_MEASUREMENTS				49
#define FT_DE_HAAS_VAN_ALPHEN_MEASUREMENTS		50
#define FT_CRITICAL_CURRENT						51
#define FT_HARDNESS								52
#define FT_YOUNGS_MODULUS						53
#define FT_SHEAR_MODULUS						54
#define FT_POSSIONS_RATIO						55
#define FT_ISOBARIC_VOLUME_THERMAL_EXPANSION	56
#define FT_ENTHALPY_OF_COMPOUND_FORMATION		57
#define FT_DEBYE_TEMPERATURE					58
#define FT_SEEBECK_COEFFICIENT					59
#define FT_THERMAL_CONDUCTIVITY					60
#define FT_ENERGY_GAP							61
#define FT_ELECTRICAL_CONDUCTIVITY				62
#define FT_ELECTRICAL_RESISTIVITY				63
#define FT_HALL_COEFFICIENT						64
#define FT_ELECTRON_MOBILITY					65
#define FT_COLOR_PROP							66
#define FT_STATIC_DIELECTRIC_CONSTANT			67
#define FT_WORK_FUNCTION						68
#define FT_FERROELECTRIC_CURIE_TEMPERATURE		69
#define FT_DIELECTRIC_PERMITTIVITY				70
#define FT_PIEZOELECTRIC_STRAIN_CONSTANT		71
#define FT_PYROELECTRIC_COEFFICIENT				72
#define FT_MAGNETIC_SUSCEPTIBILITY				73
#define FT_PARAMAGNETIC_CURIE_TEMPERATURE		74
#define FT_NEEL_TEMPERATURE						75
#define FT_CURIE_TEMPERATURE					76
#define FT_SATURATION_MAGNETIZATION				77
#define FT_CRITICAL_TEMPERATURE					78
#define FT_COHERENCE_LENGTH						79
#define FT_LONDON_PENETRATION_DEPTH				80

// Some important macros and datatypes
#define M_PI   3.14159265358979323846F
#define M_LN2  0.693147180559945309417F
#define ROUND(X) floor((X)+0.5F)	// get round(X)
#define DEG    (M_PI/180.0F)		// degrees -> radians
#define DEG_1  (180.0F/M_PI)		// radians -> degrees
#define FErr	-999.0f				// undefined floating point

typedef float MAT44[4][4];			// 4x4 matrix
typedef float POINT3[3];			// coordinate triplet

// Hint codes or masks for OnUpdate() functions of several views
#define HINT_SORTLIST	0x00000010	// OnUpdate() called after sort order has changed

// Codes fuer <lHint> in <UpdateAllViews(CView*, LPARAM lHint, CObject*)>
#define HINT_SELECTOBJECT	(LPARAM)1	// Aenderung der Objektauswahl -> <CObjectInfo>
#define HINT_SELECTALL		(LPARAM)2	// Objektauswahl komplett aktualisieren

#define HINT_DRAWSELECTION	(LPARAM)3	// einzelne Markierung in Strukturfenster XOR-zeichnen
#define HINT_DRAWSELECTIONS	(LPARAM)4	// Markierungen in Strukturfenster XOR-zeichnen

#define HINT_HISTOCHANGE	(LPARAM)5	// user moved cursor in the histogram view

/////////////////////////////////////////////////////////////////////////////
// CDiscoveryApp:
// See Discovery.cpp for the implementation of this class
//

class CDiscoveryApp : public CWinApp
{
public:
	CDiscoveryApp();

	CString m_strQsdPropertyFileName;		// full path of PROPERTY.DAT (for QSD)
	CString m_strDatabaseFieldsFileName;	// file with database fields that can be evaluated
	CString m_strQsdExePath;				// path to QSD.EXE kernel ("C:\DIR\SUBDIR\")

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDiscoveryApp)
	public:
	virtual BOOL InitInstance();
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDiscoveryApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileOpen();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// Global functions for Discovery application

extern UINT GetPopupMenuCmd (CMenu*, int, int, CWnd*, UINT=TPM_LEFTALIGN|TPM_RIGHTBUTTON);
extern int  CopyList (CListCtrl&);
extern int  SaveList (CListCtrl&, int, LPCSTR);
extern BOOL SetFileExtension (CString&, LPCSTR);
extern LPSTR ElementSymbol (LPSTR buf, int no);
extern int	GetElementNo (LPCSTR lpszSymbol);
extern int	GetElementAbcIndex (LPCSTR lpszSymbol);
extern void ReportFileError (CFileException*);
extern void ReportQsdExeError (void);
extern int  GetCsvItems (CString&, CStringArray&, BOOL);
extern void RemoveComment (CString&);
extern int  ExtractNumbers (LPCSTR, float*, int);
extern LPCSTR TrimFloat (LPSTR);
extern LPCSTR GetQsdOperationString (CString&, int);

extern void	ResetMatrix (MAT44&);
extern void	MulVector3 (float*, float*, MAT44&);
extern void MulVector4 (float*, float*, MAT44&);
extern void	GetRotationMatrix (int, float, MAT44&);
extern void	MulMatrix (MAT44&, MAT44&, MAT44&);

extern BOOL RectInRect (LPCRECT, LPCRECT, BOOL);
