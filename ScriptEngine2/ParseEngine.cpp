#include "stdafx.h"
#include "ScriptEngine.h"
#include "stringUtilities.h"
#include "stdmacros.h"
#include <string.h>
#define DECAFF_PE_CTM_RAMERROR LogError("Ran out of RAM while compiling VM.")
#define DECAFF_PE_CTM_UNEXPECTEDERROR LogError("Unexpected error while compiling VM.")
#define DECAFF_PE_CTM_CHECKPTR(ptr) if (!ptr) {	m_error=E_OUTOFRAM;	char buff[1024]={0}; LogError("Ran out of RAM while compiling VM."); return false; }
/*if (!ptr)
{
	m_error=E_OUTOFRAM;
	char buff[1024]={0};
	LogError("Ran out of RAM while compiling VM.");
	return false;
}*/

// Supports for strings still needs work

// PARSE ENGINE SOURCE FILE

enum SYMBOL_t {
	S_OPENBRACKET,S_CLOSEBRACKET,
	S_BEGINSCOPE,S_ENDSCOPE,
	S_BREAKINGCHAR,S_ALPHACHAR,
	S_NUMBER,S_MATHSYMBOL,
	S_SPCSYMBOL,
	S_ENDL,
	S_UNDEFINED
};

enum STRSYMBOL_t {
	SS_IF,
	SS_ELSE,
	SS_DO,
	SS_WHILE,
	SS_FOR,
	SS_BREAK,
	SS_RETURN,
	SS_AND,
	SS_OR,
	SS_EQUALTO,
	SS_NOTEQUALTO,

	SS_CONST,
	SS_PTR,
	SS_INT,
	SS_FLOAT,
	SS_CHAR,
	SS_BYTE,
	SS_VOID,

	SS_COMMENTLINE,
	SS_COMMENTBLOCK,
	SS_UNCOMMENTBLOCK,
	SS_INCLUDE,

	SS_UNDEFINED
};

class SYMBOLITEM
{
public:
	SYMBOLITEM()
	{
		m_pSName=NULL;
	};

	SYMBOLITEM(const char *pSName,STRSYMBOL_t symbol)
	{
		m_pSName=CreateString(pSName);
		m_symbol=symbol;
	};

	~SYMBOLITEM()
	{
		DestroyString(m_pSName);
		m_pSName=NULL;
	};

	int CompareWith(const SYMBOLITEM *pSItem) const
	{
		if (!pSItem)
			return NULL;
		if (!m_pSName||!pSItem->m_pSName)
			return NULL;
		return strcmp(m_pSName,pSItem->m_pSName);
	};

	char*		m_pSName; // string name of symbol
	STRSYMBOL_t m_symbol; // symbol
};

bool DECAFFPARSEENGINEInitStrSymbolTable(BisTree_t<SYMBOLITEM> *pStrSymbolTable)
{
	bool result = true;
	const int nItms = 22;
	const char *pStrings[nItms] = {
		"if",
		"else",
		"do",
		"while",
		"for",
		"break",
		"return",
		"&&",
		"||",
		"==",
		"!=",

		"const",
		"ptr",
		"int",
		"float",
		"char",
		"byte",
		"void",

		"/",
		"/*",
		"*/",
		"#include"
	};
	const STRSYMBOL_t symbols[nItms] = {		
		SS_IF,
		SS_ELSE,
		SS_DO,
		SS_WHILE,
		SS_FOR,
		SS_BREAK,
		SS_RETURN,
		SS_AND,
		SS_OR,
		SS_EQUALTO,
		SS_NOTEQUALTO,

		SS_CONST,
		SS_PTR,
		SS_INT,
		SS_FLOAT,
		SS_CHAR,
		SS_BYTE,
		SS_VOID,

		SS_COMMENTLINE,
		SS_COMMENTBLOCK,
		SS_UNCOMMENTBLOCK,
		SS_INCLUDE
	};
	int i=nItms;
	// insert all items into the table
	SYMBOLITEM *pSymItm;
	while (i--)
	{
		pSymItm = new SYMBOLITEM(pStrings[i],symbols[i]);
		if (!pSymItm)
			return false;
		if(!pStrSymbolTable->Insert(pSymItm))
		{
			delete pSymItm;
			return false;
		}
	}
	return true;
}

void DECAFFPARSEENGINEInitSymbolTable(SYMBOL_t asciiSymbols[])
{
	unsigned i=256;
	while (i--)
	{
		switch (i)
		{ // case (unsigned)'
		case (unsigned)'(':
			asciiSymbols[i]=S_OPENBRACKET;
			break;
		case (unsigned)')':
			asciiSymbols[i]=S_CLOSEBRACKET;
			break;
		case (unsigned)'{':
			asciiSymbols[i]=S_BEGINSCOPE;
			break;
		case (unsigned)'}':
			asciiSymbols[i]=S_ENDSCOPE;
			break;
		case (unsigned)';':
			asciiSymbols[i]=S_ENDL;
			break;
		case (unsigned)'\n': // newline char
		case (unsigned)'\f': // form feed char
		case (unsigned)'\r': // carraige return char
		case (unsigned)'\t': // tab char
		case (unsigned)' ': // space char
			asciiSymbols[i]=S_BREAKINGCHAR;
			break;
		case (unsigned)'#':
			asciiSymbols[i]=S_SPCSYMBOL;
			break;
		case (unsigned)'+':
		case (unsigned)'-':
		case (unsigned)'*':
		case (unsigned)'/':
		case (unsigned)'^':
		case (unsigned)'!':
		case (unsigned)'=':
		case (unsigned)'|':
		case (unsigned)'&':
			asciiSymbols[i]=S_MATHSYMBOL;
			break;
		default:
			if ((ISINRANGE((unsigned)'a',i,(unsigned)'z'))||
				(ISINRANGE((unsigned)'A',i,(unsigned)'Z')))
				asciiSymbols[i]=S_ALPHACHAR;
			else if (ISINRANGE((unsigned)'0',i,(unsigned)'9'))
				asciiSymbols[i]=S_NUMBER;
			else
				asciiSymbols[i]=S_UNDEFINED;
			break;
		}
	}
}

/*****************************************************************************

  This function returns true if char c exists in string str.

*****************************************************************************/
bool DECAFF::PARSEENGINE::CompileVM()
{
	// check error code
	if (m_error!=E_NOERROR)
		return false;
	// validate parameters
	if (!m_pScript)
	{
		m_error = E_NOSCRIPTFILELOADED;
		return false;
	}
	if (!m_pOutput)
	{
		m_error = E_NOVMOUTPUTISSPECIFIED;
	}
	unsigned strIdx = 0; // index into script text
	unsigned scriptLineN=1; // line number in script text
	unsigned strLen = strlen(m_pScript); // length of script text
	if (!strLen)
	{
		m_error = E_NOSCRIPTFILELOADED;
		return false;
	}
	// **** Init Symbol Tables ****
	SYMBOL_t asciiSymbols[256];
	DECAFFPARSEENGINEInitSymbolTable(asciiSymbols);
	BisTree_t<SYMBOLITEM> strSymbolTable(true);
	DECAFFPARSEENGINEInitStrSymbolTable(&strSymbolTable);

	const unsigned readBuffSZ = 1024;
	char readBuff[readBuffSZ]={0}; // read buffer
	unsigned readBuffIdx=0; // index into read buffer

	enum PMODE_t {
		P_SCAN, // scan for valid char
		P_READSTR, // read string into read buffer
		P_PROCSTR, // process read string
		P_SKIPLINE, // skips to the next newline char
		P_SKIPBLOCK, // scans to the end of a commented block
		P_READMATH // read math
	} parseMode = P_SCAN;

	STRSYMBOL_t symbolReadMode = SS_UNDEFINED; // current string symbol being read
	int symbolReadStep = 0; // stage in reading that symbol

	// compile stuff
	while (strIdx<strLen)
	{
		SCCMD cmd; // buffer for new commands
		// count new lines
		switch (*(m_pScript+strIdx))
		{
		case '\n': // newline char
		case '\f': // form feed char
		case '\r': // carraige return char
			scriptLineN++; // up line count
		default:
			break;
		};
		// parse string
		switch (parseMode)
		{
		case P_SCAN:
			switch (asciiSymbols[(unsigned)*(m_pScript+strIdx)])
			{ // Switch Char Type
			case S_BEGINSCOPE:
				// add new command to vm array
				cmd.m_command = SCCMD::BEGINSCOPE;
				m_pOutput->AddElmts(1,&cmd);
				// push current name space into name space stack
				if (!m_nameStack.Push(m_pNameSpace))
				{
					m_error = E_OUTOFRAM;
					DECAFF_PE_CTM_RAMERROR;
					return false;
				}
				// create new namespace
				m_pNameSpace = new SCNAMESPACE();
				// check pointer
				DECAFF_PE_CTM_CHECKPTR(m_pNameSpace);
				break;

			case S_ENDSCOPE:
				// add new command to vm array
				cmd.m_command = SCCMD::ENDSCOPE;
				m_pOutput->AddElmts(1,&cmd);
				// delete current name space
				delete m_pNameSpace;
				m_pNameSpace=NULL;
				// pop next name space
				if (m_nameStack.m_size)
					if (!m_nameStack.Pop(&m_pNameSpace))
					{
						m_error = E_UNEXPECTED_ERROR;
						DECAFF_PE_CTM_UNEXPECTEDERROR;
						return false;
					}
				else // or create new name space
					m_pNameSpace = new SCNAMESPACE();
				// check pointer
				DECAFF_PE_CTM_CHECKPTR(m_pNameSpace);
				break;

			case S_ALPHACHAR:
				// switch to reading a string
				parseMode=P_READSTR;
				// rewind by 1
				strIdx--;
				break;
			case S_MATHSYMBOL:
				/// HARD CODED IN !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
				if (*(m_pScript+strIdx)=='/')
				{ // if '/' then
					// switch to reading a string
					parseMode=P_READSTR;
					break;
				}
			case S_NUMBER:
			case S_OPENBRACKET:
			case S_CLOSEBRACKET:
				// syntax error (symbol out of place)
				m_error=E_SYNTAXERROR;
				{
					char buff[1024]={0};
					sprintf(buff,"Syntax error, out of place symbol found at line: %i",scriptLineN);
					LogError(buff);
				}
				return false;
				break;

			case S_BREAKINGCHAR:
				// skip breaking chars
				break;

			case S_UNDEFINED:
				// Error Found
				m_error=E_UNEXPECTEDSYMBOL;
				{
					char buff[1024]={0};
					sprintf(buff,"Unexpected symbol found at line: %i",scriptLineN);
					LogError(buff);
				}
				return false;
				break;

			case S_ENDL: // skip endl's
				break;

			default:
				break;
			}; // end P_SCAN - Switch Char Type
			break;

		case P_READSTR:
			switch (asciiSymbols[(unsigned)*(m_pScript+strIdx)])
			{
			case S_BREAKINGCHAR:
			case S_ENDL:
			case S_OPENBRACKET:
			case S_MATHSYMBOL:
				// append null char
				readBuff[readBuffIdx]=NULL;
				// end reading 
				parseMode=P_SCAN;
				// rewind
				strIdx--;
				break;
			case S_ALPHACHAR:
			case S_NUMBER:
				// record symbol
				readBuff[readBuffIdx]=*(m_pScript+strIdx);
				// increment read index
				readBuffIdx++;
				if (readBuffIdx>=readBuffSZ)
				{
					// syntax error (out of read buffer space)
					m_error=E_IDENTIFYERNAMEISTOOLONG;
					{
						char buff[1024]={0};
						sprintf(buff,"Syntax error, the identifyer name is too long on line: %i",scriptLineN);
						LogError(buff);
					}
				}
				break;
			default:
				// syntax error (symbol out of place)
				m_error=E_SYNTAXERROR;
				{
					char buff[1024]={0};
					sprintf(buff,"Syntax error, out of place symbol found at line: %i",scriptLineN);
					LogError(buff);
				}
				return false;
				break;
			}; // end P_READSTR - Switch Char Type
			break;

		case P_PROCSTR:
			// process read string
			SYMBOLITEM readSymbol(readBuff,SS_UNDEFINED), *pSymbol = &readSymbol;
			strSymbolTable.LookUp(pSymbol); // search for symbol
            switch (pSymbol->m_symbol)
			{
			default:
				/*// syntax error (symbol out of place)
				m_error=E_SYNTAXERROR;
				{
					char buff[1024]={0};
					sprintf(buff,"Syntax error, out of place symbol found at line: %i",scriptLineN);
					LogError(buff);
				}*/
				return false;
				break;
			}
			// empty read buffer
			readBuff[0]=NULL;
			readBuffIdx=0;
			break;

		case P_SKIPLINE:
			switch (*(m_pScript+strIdx))
			{
			case '\n': // newline char
			case '\f': // form feed char
			case '\r': // carraige return char
				// newline found
				// switch to scan mode
				parseMode=P_SCAN;
			default:
				break;
			}; // end P_SKIPLINE - Switch Char
			break;

		case P_SKIPBLOCK: /// HARD CODED IN !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			if (*(m_pScript+strIdx)=='*')
				readBuffIdx=1;
			else if ((*(m_pScript+strIdx)=='/')&&readBuffIdx)
			{ // end of block found
				// switch to scan mode
				parseMode=P_SCAN;
			}
			else
				readBuffIdx=0;
			break;

		default:
			break;
		};// end switch parseMode
		strIdx++;
	}
	return true;
}