#include "stdafx.h"
#include "ScriptEngine.h"
#include "stringUtilities.h"

bool DECAFFSCCMDCreate(DECAFF::SCCMD *obj,DECAFF::SCCMD *initObj,int arIdx,int relIdx)
{
	// get init object
	initObj = initObj + relIdx;
	// copy command
	obj->m_command=initObj->m_command;
	// copy nParams
	obj->m_nParams=initObj->m_nParams;
	// zero out initializer's params
	initObj->m_nParams = 0;
	// transfer pointer to this object
	switch (initObj->m_command)
	{
	case DECAFF::SCCMD::NEWVAR:
		obj->m_params.pVarDesc=initObj->m_params.pVarDesc;
		initObj->m_params.pVarDesc=NULL;
		break;
	case DECAFF::SCCMD::INCLUDE:
		obj->m_params.pStr=initObj->m_params.pStr;
		initObj->m_params.pStr=NULL;
		break;
	default:
		obj->m_params.pVarIDs=initObj->m_params.pVarIDs;
		initObj->m_params.pVarIDs=NULL;
		break;
	};
	return true;
}

//////// SCCMD
// SCCMD Constructor
DECAFF::SCCMD::SCCMD()
{
	m_nParams = 0;
}
// SCCMD Destructor
DECAFF::SCCMD::~SCCMD()
{
	switch (m_command)
	{
	case NEWVAR:
		if (m_nParams>1)
			delete [] m_params.pVarDesc;
		else
			delete m_params.pVarDesc;
		m_params.pVarDesc=NULL;
		break;
	case INCLUDE:
		if (m_nParams>1)
			delete [] m_params.pStr;
		else
			delete m_params.pStr;
		m_params.pStr=NULL;
		break;
	default:
		if (m_nParams>1)
			delete [] m_params.pVarIDs;
		else
			delete m_params.pVarIDs;
		m_params.pVarIDs=NULL;
		break;
	};
}

//////// VARDesc_t
// VARDesc_t Constructor
DECAFF::VARDesc_t::VARDesc_t()
{
	m_options = NULL;
}
// VARDesc_t Constructor with Initialization
DECAFF::VARDesc_t::VARDesc_t(DECAFF::VARDesc_t* pInit,int arIdx,int relIdx)
{
	// get init data
	pInit+=relIdx;
	// copy var type
	m_varType = pInit->m_varType;
	// copy var options
	m_options = pInit->m_options;
	// copy var value
	switch (pInit->m_varType)
	{
	case INT_T:
		m_value.intN = pInit->m_value.intN;
		break;
	case FLT_T:
		m_value.floatN = pInit->m_value.floatN;
		break;
	case CHAR_T:
		m_value.charN = pInit->m_value.charN;
		break;
	case BYTE_T:
		m_value.byteN = pInit->m_value.byteN;
		break;
	case INTPTR_T:
		m_value.intPtr = pInit->m_value.intPtr;
		break;
	case FLTPTR_T:
		m_value.floatPtr = pInit->m_value.floatPtr;
		break;
	case CHARPTR_T:
		m_value.charPtr = pInit->m_value.charPtr;
		break;
	case BYTEPTR_T:
		m_value.bytePtr = pInit->m_value.bytePtr;
		break;
	case VOIDPTR_T:
		m_value.voidPtr = pInit->m_value.voidPtr;
		break;
	case FUNCPTR_T:
		m_value.funcPtr = pInit->m_value.funcPtr;
		break;
	default:
		// error
		break;
	}
}
// VARDesc_t Destructor
DECAFF::VARDesc_t::~VARDesc_t()
{
	m_options = NULL;
}
//////// FUNCDesc_t
// FUNCDesc_t Constructor
DECAFF::VARDesc_t::FUNCDesc_t::FUNCDesc_t()
{
	m_format = NULL;
	m_nVars	 = 0;
	m_lineNum= 0;
}
// FUNCDesc_t Destructor
DECAFF::VARDesc_t::FUNCDesc_t::~FUNCDesc_t()
{
	delete [] m_format;
	m_format = NULL;
	m_nVars	 = 0;
	m_lineNum= 0;
}

//////// NAME SPACE
// NAMESPACE Constructor
DECAFF::NAMESPACE::NAMESPACE()
{
	m_varArray.SetStepSize(DECAFF_DEFAULT_NAMESPACEARRAYSTEPSIZE);
}
// NAMESPACE Destructor
DECAFF::NAMESPACE::~NAMESPACE()
{
	m_varArray.SetStepSize(DECAFF_DEFAULT_NAMESPACEARRAYSTEPSIZE);
}
//////// CTSTKELMT
// CTSTKELMT Constructor
DECAFF::CTSTKELMT::CTSTKELMT()
{
	m_lineNumber	= 0;
	m_returnVarID	= 0;
}
// CTSTKELMT Destructor
DECAFF::CTSTKELMT::~CTSTKELMT()
{
	m_lineNumber	= 0;
	m_returnVarID	= 0;
}

//////// SCNAMESPACE
// SCNAMESPACE Constructor
DECAFF::PARSEENGINE::SCNAMESPACE::SCNAMESPACE()
{
	m_varNames.SetIsOwner(true);
}

// SCNAMESPACE AdVar adds a new var into name space, 0 on fail, 1+varID on success
unsigned DECAFF::PARSEENGINE::SCNAMESPACE::AddVar(VarDesc_t *pVar,const char *pName,int nVars)
{
	unsigned varID=0;
	m_vars.m_varArray.InsertElmts(nVars,pVar,varID);
	return varID+1;
}
// SCNAMESPACE retrieves var from name space, returns null on error
void DECAFF::PARSEENGINE::SCNAMESPACE::GetVar(VarDesc_t **pVar,unsigned varID)
{
	m_vars.m_varArray.GetElmt(varID);
}
// SCNAMESPACE Destructor
DECAFF::PARSEENGINE::SCNAMESPACE::~SCNAMESPACE()
{
}
//////// VARNAME
// VARNAME Constructor
DECAFF::PARSEENGINE::VARNAME::VARNAME()
{
	m_pName=NULL;
}
// VARNAME Destructor
DECAFF::PARSEENGINE::VARNAME::~VARNAME()
{
	DestroyString(m_pName);
	m_pName=NULL;
}

//////// PARSEENGINE
// PARSEENGINE Constructor
DECAFF::PARSEENGINE::PARSEENGINE()
{
	m_pNameSpace = new SCNAMESPACE();
	m_nameStack.m_isOwner = true;
	m_pOutput = NULL;
	m_pScript = NULL;
	m_pErrorStr = NULL;
	m_error = E_NOERROR;
	if (!m_pNameSpace)
		m_error = E_OUTOFRAM;
}
// PARSEENGINE Constructor
DECAFF::PARSEENGINE::PARSEENGINE(CDSArray_t<SCCMD,SCCMD>* pOutput)
{
	m_pNameSpace = new SCNAMESPACE();
	m_nameStack.m_isOwner = true;
	m_pOutput = pOutput;
	m_pScript = NULL;
	m_pErrorStr = NULL;
	m_error = E_NOERROR;
	if (!m_pNameSpace)
		m_error = E_OUTOFRAM;
}
// PARSEENGINE Destructor
DECAFF::PARSEENGINE::~PARSEENGINE()
{
	delete m_pNameSpace;
	m_pNameSpace = NULL;
	m_pOutput = NULL;
	m_pScript = NULL;
	m_error = E_NOERROR;
	LogError(NULL);
}
// PARSEENGINE Records an error in the error string
void DECAFF::PARSEENGINE::LogError(const char *pEStr)
{
	DestroyString(m_pErrorStr);
	m_pErrorStr=NULL;
	if (pEStr)
		m_pErrorStr=CreateString(pEStr);
}
// The Rest of PARSEENGINE is in ParseEngine.cpp
//...

//////// VIRTUAL MACHINE
// VM Constructor
DECAFF::VM::VM()
{
	m_pParseEngine	= NULL;
	m_pScript		= NULL;
	m_pScriptFile	= NULL;
	m_vmCmds.Init(NULL,NULL,true,DECAFF_DEFAULT_VMCMDARRAYSTEPSIZE);
	m_pErrorStr = NULL;
	m_error = E_NOERROR;
}
// VM Constructor, sets path to script file
DECAFF::VM::VM(const char *pScriptFile)
{
	m_pParseEngine	= NULL;
	m_pScript		= NULL;
	m_pScriptFile	= NULL;
	m_vmCmds.Init(NULL,NULL,true,DECAFF_DEFAULT_VMCMDARRAYSTEPSIZE);
	m_pErrorStr = NULL;
	m_error = E_NOERROR;
	if (pScriptFile)
		if (!SetSCPath(pScriptFile))
			m_error = E_OUTOFRAM;
}
// VM Destructor
DECAFF::VM::~VM()
{
	delete m_pParseEngine;
	DestroyString(m_pScript);
	DestroyString(m_pScriptFile);
	m_pParseEngine	= NULL;
	m_pScript		= NULL;
	m_pScriptFile	= NULL;
	m_vmCmds.Init(NULL,DECAFFSCCMDCreate,true,DECAFF_DEFAULT_VMCMDARRAYSTEPSIZE);
	m_error = E_NOERROR;
	LogError(NULL);
}
// VM Records an error in the error string
void DECAFF::VM::LogError(const char *pEStr)
{
	DestroyString(m_pErrorStr);
	m_pErrorStr=NULL;
	if (pEStr)
		m_pErrorStr=CreateString(pEStr);
}
// VM Sets the script file path variable, returns false on failure
bool DECAFF::VM::SetSCPath(const char *pScriptFile)
{
	delete m_pScriptFile;
	m_pScriptFile	= CreateString(pScriptFile);
	if (!m_pScriptFile)
	{
		m_error = E_OUTOFRAM;
		return false;
	}
	return true;
}
// VM loads the script from the given script file, returns false on failure
bool DECAFF::VM::LoadScript()
{
	delete m_pScript;
	m_pScript = STRUTILEReadTxtFile(m_pScriptFile,DECAFF_MAXSCRIPTLEN);
	if (!m_pScript)
	{
		m_error = E_LOADERROR;
		return false;
	}
	return true;
}
// VM Sets the text for the script, returns false on failure
bool DECAFF::VM::SetScript(const char *pScript)
{
	delete m_pScript;
	m_pScript = CreateString(pScript);
	if (!m_pScript)
	{
		m_error = E_LOADERROR;
		return false;
	}
	return true;
}
// VM Compiles the VM code from the script text
bool DECAFF::VM::CompileVM()
{
	// script text must exist, try to load it if possible
	if (!m_pScript)
		if (!LoadScript())
			return false;
	// create parse engine
	m_pParseEngine = new PARSEENGINE(&m_vmCmds);
	if (!m_pParseEngine)
		return false;
	// copy pointer to script text
	m_pParseEngine->m_pScript=m_pScript;
	bool success = m_pParseEngine->CompileVM();
	// if error then
	if (!success)
	{
		// copy error info
		m_error = m_pParseEngine->m_error;
		LogError(m_pParseEngine->m_pErrorStr);
		// destroy vm commands
		m_vmCmds.Destroy();
	}
	// destroy parse engine
	delete m_pParseEngine;
	m_pParseEngine = NULL;
	// return success
	return success;
};