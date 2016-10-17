#ifndef __INCLUDED_SCRIPT_ENGINE_H__
#define __INCLUDED_SCRIPT_ENGINE_H__

#include "advdtypes/advdtypes.h"

#define DECAFF_DEFAULT_NAMESPACEARRAYSTEPSIZE 25
#define DECAFF_DEFAULT_VMCMDARRAYSTEPSIZE 100
#define DECAFF_MAXSCRIPTLEN 65536

class DECAFF
{
public:
	enum ERROR_t { E_NOERROR, // system is ok
		// parsing error
		E_PARSEINITFAILED, E_SYNTAXERROR,
			// syntax errors
			E_UNEXPECTEDSYMBOL,
			E_IDENTIFYERNAMEISTOOLONG,
		E_NOSCRIPTFILELOADED,
		E_NOVMOUTPUTISSPECIFIED,
		// general errors
		E_OUTOFRAM, E_FILENOTFOUND, E_LOADERROR, E_UNEXPECTED_ERROR,
		// vm errors
		E_VMINITFAILED,
	};

	class VARDesc_t
	{
	public:
		VARDesc_t();
		VARDesc_t(VARDesc_t* pInit,int arIdx,int relIdx);
		~VARDesc_t();
		enum VARTYPE { INT_T,FLT_T,CHAR_T,BYTE_T,
			// pointer types
			INTPTR_T,FLTPTR_T,CHARPTR_T,BYTEPTR_T,
			VOIDPTR_T, FUNCPTR_T
		};

		class FUNCDesc_t
		{
		public:
			FUNCDesc_t();
			~FUNCDesc_t();
			VARTYPE* m_format; // format of function vars
			unsigned m_nVars;  // number of function vars
			unsigned m_lineNum;// line number of function
		};

		union VARValue_t
		{
			int*			intN; // interger value
			float*			floatN; // float value
			char*			charN; // char value
			unsigned char*	byteN; // byte value
			int**			intPtr; // interger pointer
			float**			floatPtr; // float pointer
			char**			charPtr; // char pointer
			unsigned char**	bytePtr; // byte pointer
			void**			voidPtr; // void pointer
			FUNCDesc_t*		funcPtr; // function pointer
		};

		VARTYPE	   m_varType; // type of variable
		VARValue_t m_value; // value of variable
		unsigned char m_options; // 0 - isConst, 1 - isExternal
	};

	class SCCMD
	{
	public:
		SCCMD();
		~SCCMD();
		enum COMMAND { INCLUDE, // extending
			// dynamic
			EXIT, // stops runinng the script
			BEGINSCOPE,
			ENDSCOPE,
			RETURN,
			BREAK,
			LOOP,
			SKIP, // skips to end of next scope
			NEWVAR, // new variable
			DELVAR, // delete new variable
			COPY, // = sign
			CALLFUNCTION,
			// conditional
			IF,
			ELSE,
			ENDIF,
			// Math
			AND,OR,NOT, // bool
			ISGTR,ISLESS,ISEQUAL, // bool comparative
			ADD,SUB,MULT,DIV,PWR
		};

		COMMAND m_command;

		union
		{
			unsigned*	pVarIDs; // pointer to variable ids
			char*		pStr; // pointer to string data
			VARDesc_t*	pVarDesc; // pointer to variable data

		} m_params; // paramaters
		int m_nParams; // number of parameters
	};

	class NAMESPACE
	{
	public:
		NAMESPACE();
		~NAMESPACE();
		CPtrArray_t<VARDesc_t,VARDesc_t> m_varArray; // array of variables
	};

	class CTSTKELMT
	{
	public:
		CTSTKELMT();
		~CTSTKELMT();
		unsigned m_lineNumber; // line number buffer
		unsigned m_returnVarID; // buffer for any return var, varID
		NAMESPACE m_nameSpace; // name space
	};

	class PARSEENGINE
	{
	public:
		PARSEENGINE();
		PARSEENGINE(CDSArray_t<SCCMD,SCCMD>* pOutput);
		~PARSEENGINE();
		class VARNAME
		{
		public:
			VARNAME();
			~VARNAME();
			char*	 m_pName; // variable string name
			unsigned m_id; // variable id
		};

		class SCNAMESPACE
		{
		public:
			SCNAMESPACE();
			~SCNAMESPACE();
			unsigned AddVar(VarDesc_t *pVar,const char *pName,int nVars = 1);
			void GetVar(VarDesc_t **pVar,unsigned varID);
			NAMESPACE			m_vars;
			BisTree_t<VARNAME>	m_varNames;
		};
		
		char*						m_pErrorStr; // pointer to string for reporting errors
		ERROR_t						m_error; // current error code
		SCNAMESPACE*				m_pNameSpace; // pointer to name space data
		Stack_t<SCNAMESPACE>		m_nameStack; // name space stack
		CDSArray_t<SCCMD,SCCMD>*	m_pOutput; // pointer to vm command output
		const char*					m_pScript; // pointer to script text

		void LogError(const char *pEStr);
		bool CompileVM();
	};

	class VM
	{
	public:
		VM();
		VM(const char *pScriptFile);
		~VM();
		bool SetSCPath(const char *pScriptFile);
		bool LoadScript();
		bool SetScript(const char *pScript);

		void LogError(const char *pEStr);
		bool CompileVM();
		
		char*			m_pErrorStr; // pointer to string for reporting errors
		ERROR_t			m_error; // current error code
		PARSEENGINE*	m_pParseEngine; // pointer to parse engine
		char*			m_pScript; // pointer to script text
		char*			m_pScriptFile; // pointer to script file
		CDSArray_t<SCCMD,SCCMD> m_vmCmds; // array of commands for execution
	};
};

#endif//__INCLUDED_SCRIPT_ENGINE_H__