  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#define MAXLINELENGHT 2048
#define PARSELINELIMIT 1600
#define MAXSYMBOLLENGHT 128

#include "scp_parser.h"
#include "console.h"

/*=========================================================================================

  File format :

  A text file divided into sections. Each section is something like

   SECTION <sectionname>
   {
	 ...
   }

  Commands to parse single sections are provided. Also supported are :
	#include <filename>
  and
	#define $<symbolname> <definition>

  Example script

	#include someotherfile.scp
	#define $def1 4610
	#define $per_sign x
	#define $boh $per_sign /

	// A comment
	SECTION SECTIONNAME 1
	{
		// Some comments
		COMMAND $def1 $per_sign $boh $def1
		// Other comments
		COMMAND 491
	}

  Where COMMAND $def1 $per_sign $boh $def1 gets parsed as :

	   COMMAND 4610 x x / 4610

 Enjoy! Xanathar :)

=========================================================================================*/



/********************************************************************************
 * **************************************************************************** *
 * *																		  * *
 * * GLOBALS																  * *
 * *																		  * *
 * **************************************************************************** *
 ********************************************************************************/

static std::map<std::string, std::string> s_mapDefines;

///////////////////////////////////////////////////////////////////
// Function name	 : void parseDefineCommand
// Description		 : parses a #define command
// Return type		 : static
// Author			 : Xanathar
// Argument 		 : char *line -> the line containing the #define command
// Changes			 : none yet
static void parseDefineCommand(char *line)
{
	//syntax is #define $name=value
	std::string str1, str2;
	char *val = NULL;
	char *trimmedline = NULL;
	int i, ln = strlen(line);

	for (i=0; i<ln; i++) {
		if (line[i]=='$') {
			trimmedline = line+i;
			break;
		}
	}

	if (trimmedline == NULL || *trimmedline == '\0') {
		ConOut("[ERROR] invalid define, missing $.\n");
		return;
	}

	ln = strlen(trimmedline);
	for (i=1; i<ln; i++) {
		if (isspace(trimmedline[i])) {
			trimmedline[i] = '\0';
			val = trimmedline + i + 1;
			break;
		}
	}

	val = linestart(val);

	if (val == NULL || *val == '\0') {
		ConOut("[ERROR] invalid define, syntax is #define $symbol content\n");
		return;
	}

	str1 = trimmedline;
	str2 = val;
	s_mapDefines.insert(std::make_pair(str1, str2));
}

/*!
\brief Evalutates the first $symbol in the line
\return true if valid
\author Xanathar
\param szLine the line with *MUST* have some spare bytes at its end
\todo fix the MUST, maybe?
*/
bool evaluateOneDefine (char *szLine)
{
	char szSymbol[MAXSYMBOLLENGHT];
	char szBuffer[MAXLINELENGHT];
	char *szToken;
	char *szAfter = "";
	int i, ln;

	ln = strlen(szLine);
	if (ln>=PARSELINELIMIT) return false;

	szBuffer[0] = szSymbol[0] = '\0'; //initialize strings :)

	szToken = strstr(szLine, "$");
	if (szToken==NULL) return false; //no symbols in string -> go out :]

	// copy the part before the symbol in the buffer
	*szToken = '\0';
	strcpy(szBuffer, szLine);
	*szToken = '$';
	// now extract the symbol name
	ln = strlen(szToken);

	for (i = 0; i < ln; i++) {
		if (!isspace(szToken[i]))
			szSymbol[i] = szToken[i];
		else {
			szSymbol[i] = '\0';
			szAfter = szToken+i;
			break;
		}
	}
	szSymbol[i] = '\0';

	// now szSymbol contains the symbolname, szAfter contains the string after the symbol
	// let's go find the symbol value !
	std::map<std::string, std::string>::iterator iter;

	iter = s_mapDefines.find(std::string(szSymbol));

	if (iter==s_mapDefines.end()) {
		LogError("symbol %s not previously defined.\n", szSymbol);
		return false;
	}

	strcat(szBuffer, iter->second.c_str());
	strcat(szBuffer, szAfter);
	strcpy(szLine, szBuffer);
	return true;
}



///////////////////////////////////////////////////////////////////
// Function name	 : char* evaluateAllDefines
// Description		 : Evaluates ALL the defines
// Return type		 : static
// Author			 : Xanathar
// Argument 		 : char *szLine -> the line which *MUST* have some free bytes at its end
// Changes			 : none yet
static char* evaluateAllDefines (char *szLine)
{
	//this is an XSS builder metacommand, internal use
	if ((szLine[0]=='/')&&(szLine[1]=='/')&&(szLine[2]=='$')&&(szLine[3]=='$')) return szLine;

	if ((szLine[0]=='#')&&(szLine[1]=='d')) return szLine;
	while (evaluateOneDefine(szLine));
	return szLine;
}


namespace xss {
///////////////////////////////////////////////////////////////////
// Function name	 : char* getIntFromDefine
// Description		 : Evaluates ALL the defines into an integer
// Return type		 : int
// Author			 : Xanathar
// Argument 		 : char *szLine -> the line which *MUST* have some free bytes at its end
// Changes			 : none yet
int getIntFromDefine (char *szLine)
{
	char buffer[2000];
	strncpy(buffer, szLine, 1990);
	return atoi(evaluateAllDefines(buffer));
}

///////////////////////////////////////////////////////////////////
// Function name	 : char* evaluateAllDefines
// Description		 : Evaluates ALL the defines, public and size safe
// Return type		 : char*
// Author			 : Xanathar
// Argument 		 : char *szLine -> the line which *MUST* have some free bytes at its end
// Changes			 : none yet
char* evalDefines (char *szLine)
{
	char buffer[2000];
	strncpy(buffer, szLine, 1990);
	int maxlen = strlen(buffer);
	evaluateAllDefines(buffer);
	buffer[maxlen] = '\0'; //trunc !
	strncpy(szLine, buffer, maxlen);
	return szLine;
}

} //namespace xss

///////////////////////////////////////////////////////////////////
// Function name	 : char* getSzFromFile
// Description		 : enhanced version of fgets which trims leading spaces,
//					   trailing new lines and parses $symbols
// Return type		 : static
// Author			 : Xanathar
// Argument 		 : char* buffer -> the buffer in which the sz should be stored
// Argument 		 : int len -> maximum lenght of the buffer
// Argument 		 : FILE* FILEHANDLE -> the file handle
// Changes			 : none yet
static char* getSzFromFile(char* buffer, int len, FILE* fileHandle)
{
	int i, idx;
	fgets(buffer, len, fileHandle);
	buffer[MAXLINELENGHT-2] = '\0'; //xan : this to avoid reading over with strlen

	//remove LF
	idx = strlen(buffer)-1;
	if (idx<0) idx = 0;
	if (idx>(MAXLINELENGHT-2)) idx = MAXLINELENGHT-2;
	buffer[idx] = '\0';

	//remove eventual CR
	idx = strlen(buffer)-1;
	if (idx<=0) idx = 0;
	if (idx>(MAXLINELENGHT-2)) idx = MAXLINELENGHT-2;
	if ((buffer[idx]=='\n')||(buffer[idx]=='\r')) buffer[idx] = '\0';

	idx = strlen(buffer);

	for (i = 0; i <= idx; i++)
	{
		if ((buffer[i]=='\0')||(!isspace(buffer[i]))) return evaluateAllDefines(buffer+i);
	}

	return evaluateAllDefines(buffer+idx);
}





/********************************************************************************
 * **************************************************************************** *
 * *																		  * *
 * * CLASS cScpEntry														  * *
 * *																		  * *
 * **************************************************************************** *
 ********************************************************************************/


///////////////////////////////////////////////////////////////////
// Function name	 : cScpEntry::cScpEntry
// Description		 : Constructs a new cScpEntry object parsing a string
// Return type		 : <constructor>
// Author			 : Xanathar
// Argument 		 : char* szStringToParse -> the string to be parsed
// Changes			 : none yet
// TO DO			 : change it to use dynamic constants!!
cScpEntry::cScpEntry(char* szStringToParse)
{
	//store the full line
	m_strFullLine = szStringToParse;
	//split the full line and store it into m_strParam1 & m_strParam2
	int ln = strlen(szStringToParse);
	int i;
	char *szPar2 = NULL;

	for (i=0; i<ln; i++) {
		if (szStringToParse[i]==' ') {
			szStringToParse[i] = '\0';
			szPar2 = szStringToParse + i + 1;
			break;
		}
	}
	if (szPar2 == NULL) szPar2 = "";
	m_strParam1 = szStringToParse;
	m_strParam2 = szPar2;
}


///////////////////////////////////////////////////////////////////
// Function name	 : cScpEntry::getFullLine
// Description		 : gets a full line
// Return type		 : std::string
// Author			 : Xanathar
// Changes			 : none yet
std::string cScpEntry::getFullLine ()
{
	return m_strFullLine;
}


///////////////////////////////////////////////////////////////////
// Function name	 : cScpEntry::getParam1
// Description		 : gets the prefix of a splitted line
// Return type		 : std::string
// Author			 : Xanathar
// Changes			 : none yet
std::string cScpEntry::getParam1 ()
{
	return m_strParam1;
}


///////////////////////////////////////////////////////////////////
// Function name	 : cScpEntry::getParam2
// Description		 : get the rest of a splitted string
// Return type		 : std::string
// Author			 : Xanathar
// Changes			 : none yet
std::string cScpEntry::getParam2 ()
{
	return m_strParam2;
}


//=========================================================================================


/********************************************************************************
 * **************************************************************************** *
 * *																		  * *
 * * CLASS cScpSection														  * *
 * *																		  * *
 * **************************************************************************** *
 ********************************************************************************/


///////////////////////////////////////////////////////////////////
// Function name	 : cScpSection::cScpSection
// Description		 : Creates a cScpSection object loading it from a file
// Return type		 : <constructor>
// Author			 : Xanathar
// Argument 		 : FILE *file -> the opened file to be loaded
// Argument 		 : cScpScript* parent -> the cScpScript loading this section
// Changes			 : none yet
cScpSection::cScpSection (FILE *file, cScpScript* parent)
{
	char realbuffer[MAXLINELENGHT];
	char* buffer;
	cScpEntry* cse;

	realbuffer[0] = '\0';
	buffer = realbuffer;
	m_vLines.clear();

	while ((!feof(file))&&(buffer[0]!='}')) {
   	buffer = getSzFromFile(realbuffer, MAXLINELENGHT-3, file);
		if (feof(file)) {
			ConOut("[FAIL] : Unexpected end of file!..");
			break;
		}
		if ((buffer[0]!='/')&&(buffer[0]!='#')&&(buffer[0]!=';')) {
			cse = new cScpEntry(buffer);
			m_vLines.push_back(*cse);
			safedelete(cse);
		}
		if (buffer[0]=='#') {
			if (!strncmp(buffer,"#copy ", strlen("#copy "))) {
				char script1[1024];
				cScpIterator* iter = NULL;
				sprintf(script1, "SECTION %s", buffer+strlen("#copy "));
				iter = parent->getNewIterator(script1);
				if (iter==NULL) {
					ConOut("Can't extend section : %s\n", buffer);
					continue;
				}
				do {
					strcpy(script1, iter->getEntry()->getFullLine().c_str());
					if ((script1[0]!='}')&&(script1[0]!='{')) {
						cse = new cScpEntry(script1);
						m_vLines.push_back(*cse);
						safedelete(cse);
					}
				} while (script1[0]!='}');
				safedelete(iter);
			}
		}
	}
}


///////////////////////////////////////////////////////////////////
// Function name	 : cScpSection::~cScpSection
// Description		 : -- as default --
// Return type		 : <destructor>
// Author			 : Xanathar
// Changes			 : none yet
cScpSection::~cScpSection()
{
	m_vLines.clear();
}


///////////////////////////////////////////////////////////////////
// Function name	 : cScpSection::getEntry
// Description		 : gets a pointer to an cScpEntry object
// Return type		 : cScpEntry*
// Author			 : Xanathar
// Argument 		 : int nIdx -> the index of the entry we want
// Changes			 : none yet
cScpEntry* cScpSection::getEntry(int nIdx)
{
	return &m_vLines[nIdx];
}


///////////////////////////////////////////////////////////////////
// Function name	 : cScpSection::parseLine
// Description		 : parses a line into two std::string parameters
// Return type		 : void
// Author			 : Xanathar
// Argument 		 : int nIdx -> the index of the entry we want
// Argument 		 : std::string& strParam1 -> the 1st parameter
// Argument 		 : std::string& strParam2 -> the 2nd parameter
// Changes			 : none yet
void cScpSection::parseLine(int nIdx, std::string& strParam1, std::string& strParam2)
{
	class cScpEntry entry = m_vLines[nIdx];
	strParam1 = entry.getParam1();
	strParam2 = entry.getParam2();
}


///////////////////////////////////////////////////////////////////
// Function name	 : cScpSection::parseLine
// Description		 : parses a line into two char* parameters
// Return type		 : void
// Author			 : Xanathar
// Argument 		 : int nIdx -> the index of the entry we want
// Argument 		 : char* szParam1 -> the 1st parameter
// Argument 		 : char* szParam2 -> the 2nd parameter
// Changes			 : none yet
void cScpSection::parseLine(int nIdx, char* szParam1, char* szParam2)
{
	class cScpEntry entry = m_vLines[nIdx];
	strcpy(szParam1, entry.getParam1().c_str());
	strcpy(szParam2, entry.getParam2().c_str());
}


//=========================================================================================


/********************************************************************************
 * **************************************************************************** *
 * *																		  * *
 * * CLASS cScpScript														  * *
 * *																		  * *
 * **************************************************************************** *
 ********************************************************************************/



///////////////////////////////////////////////////////////////////
// Function name	 : cScpScript::cScpScript
// Description		 : constructs a cScpScript object reading it from a file
// Return type		 : <constructor>
// Author			 : Xanathar
// Argument 		 : char *szFileName -> name of file to be read
// Changes			 : none yet
cScpScript::cScpScript(char *szFileName)
{
	m_nErrors = 0;
	FILE *file = fopen(szFileName, "rt");
	ConOut("\tLoading script : %s... ", szFileName);

	if (file==NULL) {
		ConOut("[FAIL] - Can't open file\n");
		return;
	} else {
		ConOut("\n");
		fclose(file);
		load(szFileName);
		if (m_nErrors==0)
			ConOut("\t	  [ OK ] - %d section(s) loaded\n\n", m_mapSections.size() );
		else
			ConOut("\t	  [ %d ERRORS ] - %d section(s) loaded\n\n", m_nErrors,m_mapSections.size() );
	}
}



///////////////////////////////////////////////////////////////////
// Function name	 : cScpScript::load
// Description		 : Loads a section from a file (eventually recursing)
// Return type		 : void
// Author			 : Xanathar
// Argument 		 : char *szFileName -> filename to be loaded
// Changes			 : none yet
void cScpScript::load(char *szFileName)
{
	char realbuffer[MAXLINELENGHT];
	char *buffer;

	FILE *file = fopen(szFileName, "rt");

	if (file==NULL) {
		ConOut("[FAIL] - Can't open file");
		m_nErrors++;
		return;
	}

	while (!feof(file))
	{
		buffer = getSzFromFile(realbuffer, MAXLINELENGHT-3, file);
		if (feof(file)) break;
		if (buffer[0]=='#') {
			//preprocessor directive :)
			if (!strncmp(buffer, "#include ", strlen("#include "))) {
				ConOut("\t\tLoading subscript : %s... ", buffer+9);
				load(buffer+9);
				ConOut("\n");
			}
			else if (!strncmp(buffer, "#define ", strlen("#define "))) {
				parseDefineCommand(buffer+8);
			}
			continue;
		}
		if (!strncmp(buffer, "UNIQUE ", strlen("UNIQUE "))) {
			char *p = buffer; //just ignore UNIQUE keyword
			buffer = (strstr(buffer, "SECTION"));
			if (buffer == NULL) buffer = p;
		}
		if (!strncmp(buffer, "SECTION ", strlen("SECTION "))) {
			//a new section was found!
			std::string* str = new std::string(buffer);
			cScpSection* section = new cScpSection(file, this);
			if (section!=NULL) m_mapSections[*str] = *section;
			//if (section!=NULL) m_mapSections.insert(std::make_pair(*str, *section));
			safedelete(section);
			safedelete(str);
		}
	}
	fclose(file);
}


///////////////////////////////////////////////////////////////////
// Function name	 : cScpScript::~cScpScript
// Description		 : -- as default --
// Return type		 : <destructor>
// Author			 : Xanathar
// Changes			 : none yet
cScpScript::~cScpScript()
{
	m_mapSections.erase(m_mapSections.begin(), m_mapSections.end() );
	m_mapSections.clear();
}



///////////////////////////////////////////////////////////////////
// Function name	 : cScpScript::getSection
// Description		 : returns the ptr to a cScpSection object for a given search id
// Return type		 : cScpSection*
// Author			 : Xanathar
// Argument 		 : std::string& strIdentifier -> the section id to be searched
// Changes			 : none yet
cScpSection* cScpScript::getSection(std::string& strIdentifier)
{
	std::map<std::string, class cScpSection>::iterator iter;

	iter = m_mapSections.find(strIdentifier);

	if (iter!=m_mapSections.end()) {
		return &iter->second;
	} else return NULL;
}


///////////////////////////////////////////////////////////////////
// Function name	 : cScpScript::getSection
// Description		 : returns the ptr to a cScpSection object for a given search id
// Return type		 : cScpSection*
// Author			 : Xanathar
// Argument 		 : char* szIdentifier -> the section id to be searched
// Changes			 : none yet
cScpSection* cScpScript::getSection(char* szIdentifier)
{
	std::string str = szIdentifier;
	return getSection(str);
}


///////////////////////////////////////////////////////////////////
// Function name	 : cScpScript::getSectionInStr
// Description		 : returns the ptr to a cScpSection object containing given search id
// Return type		 : cScpSection*
// Author			 : Xanathar
// Argument 		 : char* szIdentifier -> the section id to be searched
// Changes			 : none yet
cScpSection* cScpScript::getSectionInStr(char* szIdentifier)
{
	std::map<std::string, class cScpSection>::iterator iter( m_mapSections.begin() ), end( m_mapSections.end() );

	for (; iter!=end; ++iter) {
		if (strstr(iter->first.c_str(), szIdentifier)) return &iter->second;
	}

	return NULL;
}


///////////////////////////////////////////////////////////////////
// Function name	 : cScpScript::getNewIterator
// Description		 : gets a new iterator object for a section identifier
// Return type		 : cScpIterator*
// Author			 : Xanathar
// Argument 		 : std::string& strIdentifier -> the section identifier
// Changes			 : none yet
cScpIterator* cScpScript::getNewIterator(std::string& strIdentifier)
{
	cScpSection* section = getSection(strIdentifier);
	if (section==NULL) return NULL;
	return new cScpIterator(section);
}


/*!
\brief gets a new iterator object for a section identifier
\author Xanathar
\since 0.82a
\param szIdentifier the section identifier
\return cScpIterator*
\remarks \remark rewritten by Anthalir
*/
cScpIterator* cScpScript::getNewIterator(char *format, ...)
{
	char tmp[512];
	va_list vargs;

	va_start(vargs, format);
	vsnprintf(tmp, 511, format, vargs);
	va_end(vargs);

	cScpSection* section = getSection(tmp);
	if (section==NULL) return NULL;
	return new cScpIterator(section);
}

///////////////////////////////////////////////////////////////////
// Function name	 : cScpScript::getNewIteratorInStr
// Description		 : gets a new iterator object for section containing the identifier
// Return type		 : cScpIterator*
// Author			 : Xanathar
// Argument 		 : char* szIdentifier -> the identifier
// Changes			 : none yet
cScpIterator* cScpScript::getNewIteratorInStr(char* szIdentifier)
{
	cScpSection* section = getSectionInStr(szIdentifier);
	if (section==NULL) return NULL;
	return new cScpIterator(section);
}

//Begin Change	1
///////////////////////////////////////////////////////////////////
// Function name     : cScpScript::getFirstSectionParmsInStr
// Description       : returns the first section id given search id
// Return type       : string*
// Author            : Sparhawk 2002-02-01
// Argument          : strIdentifier the section id to be searched
// Changes           : none yet
string* cScpScript::getFirstSectionParmsInStr(std::string& strIdentifier)
{
  std::string::size_type index;
	std::map<std::string, class cScpSection>::iterator end( m_mapSections.end() );
	for (m_iter = m_mapSections.begin(); m_iter!=end; ++m_iter)
	{
		index = m_iter->first.find( strIdentifier );
		if ( index != std::string::npos )
		{
			std::string* parms = new std::string(m_iter->first);
			parms->replace( index, strIdentifier.size(), "");
			return parms;
		}
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////
// Function name     : cScpScript::getNextSectionParmsInStr
// Description       : returns the next section id given search id
// Return type       : string
// Author            : Sparhawk 2002-02-01
// Argument          : strIdentifier the section id to be searched
// Changes           : none yet
string* cScpScript::getNextSectionParmsInStr(std::string& strIdentifier)
{
  std::string::size_type index;
	std::string* parms;
	std::map<std::string, class cScpSection>::iterator end( m_mapSections.end() );

	m_iter++;
	for (; m_iter!=end; ++m_iter)
	{
		index = m_iter->first.find( strIdentifier );
		if ( index != std::string::npos )
		{
			parms = new std::string(m_iter->first);
			parms->replace( index, strIdentifier.size(), "");
			return parms;
		}
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////
// Function name     : cScpScript::countSectionInStr
// Description       : returns the number of cScpSection objects with given search id
// Return type       : UI32
// Author            : Sparhawk 2001-01-30
// Argument          : string& strIdentifier -> the section id to be searched
// Changes           : none yet
UI32 cScpScript::countSectionInStr(std::string& strIdentifier)
{
	std::map<std::string, class cScpSection>::iterator iter( m_mapSections.begin() ), end( m_mapSections.end() );
	unsigned int	count = 0;
	for (; iter!=end; ++iter)
	{
		std::string::size_type index;
		index = iter->first.find( strIdentifier );
		if ( index != std::string::npos )
			count++;
	}

	return count;
}

///////////////////////////////////////////////////////////////////
// Function name     : cScpScript::countSectionInStr
// Description       : returns the number of cScpSection objects with given search id
// Return type       : UI32
// Author            : Sparhawk 2001-01-30
// Argument          : char* szIdentifier -> the section id to be searched
// Changes           : none yet
UI32 cScpScript::countSectionInStr(char* szIdentifier)
{
	std::string str = szIdentifier;
	return countSectionInStr(str);
}
//End	Change	1
//=========================================================================================


/********************************************************************************
 * **************************************************************************** *
 * *																		  * *
 * * CLASS cScpIterator 													  * *
 * *																		  * *
 * **************************************************************************** *
 ********************************************************************************/


///////////////////////////////////////////////////////////////////
// Function name	 : cScpIterator::cScpIterator
// Description		 : constructor which creates an iterator for a given section
// Return type		 : <constructor>
// Author			 : Xanathar
// Argument 		 : cScpSection* section -> -- as default --
// Changes			 : none yet
cScpIterator::cScpIterator(cScpSection* section)
{
	m_section = section;
	m_nPosition = 0;
}


///////////////////////////////////////////////////////////////////
// Function name	 : cScpIterator::cScpIterator
// Description		 : constructor which creates an iterator for a given section
// Return type		 : <constructor>
// Author			 : Xanathar
// Argument 		 : cScpSection& section -> -- as default --
// Changes			 : none yet
cScpIterator::cScpIterator(cScpSection& section)
{
	m_section = &section;
	m_nPosition = 0;
}


///////////////////////////////////////////////////////////////////
// Function name	 : cScpIterator::~cScpIterator
// Description		 : -- as default --
// Return type		 : <destructor>
// Author			 : Xanathar
// Changes			 : none yet
cScpIterator::~cScpIterator()
{
	// Nothing to do here!
}


///////////////////////////////////////////////////////////////////
// Function name	 : cScpIterator::parseLine
// Description		 : parses a splitted script line in two strings
// Return type		 : void
// Author			 : Xanathar
// Argument 		 : std::string& strParam1 -> the 1st string to be filled
// Argument 		 : std::string& strParam2 -> the 2nd string to be filled
// Changes			 : none yet
void cScpIterator::parseLine(std::string& strParam1, std::string& strParam2)
{
	m_section->parseLine(m_nPosition++, strParam1, strParam2);
}

///////////////////////////////////////////////////////////////////
// Function name	 : cScpIterator::parseLine
// Description		 : parses a splitted script line in two char*s
// Return type		 : void
// Author			 : Xanathar
// Argument 		 : char* szParam1 -> the 1st string to be filled
// Argument 		 : char* szParam2 -> the 2nd string to be filled
// Changes			 : none yet
void cScpIterator::parseLine(char* szParam1, char* szParam2)
{
	m_section->parseLine(m_nPosition++, szParam1, szParam2);
}


///////////////////////////////////////////////////////////////////
// Function name	 : cScpIterator::getEntry
// Description		 : gets a cScpEntry object from the section
// Return type		 : cScpEntry*
// Author			 : Xanathar
// Changes			 : none yet
cScpEntry* cScpIterator::getEntry()
{
	return m_section->getEntry(m_nPosition++);
}


///////////////////////////////////////////////////////////////////
// Function name	 : cScpIterator::rewind
// Description		 : rewinds the iterator to its first position
// Return type		 : void
// Author			 : Xanathar
// Changes			 : none yet
void cScpIterator::rewind()
{
	m_nPosition = 0;
}
