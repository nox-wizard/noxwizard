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
#include "utils.h"
#include "inlines.h"

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



static std::map<std::string, std::string> s_mapDefines;


/*!
\brief Parses a #define command
\author Xanathar
\param line the line containing the #define command
*/
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



/*!
\brief Evaluates ALL the defines
\return the line evaluated
\author Xanathar
\param szLine the line with *MUST* have some free bytes at its end
\todo fix the MUST, maybe?
*/
static char* evaluateAllDefines (char *szLine)
{
//	//this is an XSS builder metacommand, internal use
//	if ((szLine[0]=='/')&&(szLine[1]=='/')&&(szLine[2]=='$')&&(szLine[3]=='$')) return szLine;

	// ignore comments, so it doesn't give errors for undefined identifiers commented out
	// it make non-sense the previous check. Akron
	if ( szLine[0] == '/' && szLine[1] == '/' ) return szLine;

	if ((szLine[0]=='#')&&(szLine[1]=='d')) return szLine;
	while (evaluateOneDefine(szLine));
	return szLine;
}


namespace xss {
/*!
\brief Evaluates ALL the defines into an integer
\return the integer value of a define 
\author Xanathar
\param szLine the line with *MUST* have some free bytes at its end
\todo fix the MUST, maybe?
*/
int getIntFromDefine (char *szLine)
{
	char buffer[2000];
	strncpy(buffer, szLine, 1990);
	return atoi(evaluateAllDefines(buffer));
}

/*!
\brief Evaluates ALL the defines, public and size safe
\return the line with all defines evaluated
\author Xanathar
\param szLine the line with *MUST* have some free bytes at its end
\todo fix the MUST, maybe?
*/
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

/*!
\brief enhanced version of fgets which trims leading spaces, trailing new lines and parses $symbols
\return line cleaned with defines evaluated
\author Xanathar
\param buffer the buffer in which the sz should be stored
\param len maximum lenght of the buffer
\param fileHandle the file handle
*/
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


/*!
\brief Constructs a new cScpEntry object parsing a string
\author Xanathar
\param szStringToParse the string to be parsed
\todo change it to use dynamic constants!!
*/
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


/*!
\brief Gets a full line
\author Xanathar
*/
std::string cScpEntry::getFullLine ()
{
	return m_strFullLine;
}


/*!
\brief Gets the prefix of a splitted line
\author Xanathar
*/
std::string cScpEntry::getParam1 ()
{
	return m_strParam1;
}


/*!
\brief Gets the rest of a splitted string
\author Xanathar
*/
std::string cScpEntry::getParam2 ()
{
	return m_strParam2;
}


/*!
\brief Creates a cScpSection object loading it from a file
\author Xanathar
\param file the opened file to be loaded
\param parent the cScpScript loading this section
*/
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


cScpSection::~cScpSection()
{
	m_vLines.clear();
}


/*!
\brief Gets a pointer to an cScpEntry object
\author Xanathar
\param nIdx the index of the entry we want
\return the entry
*/
cScpEntry* cScpSection::getEntry(int nIdx)
{
	return &m_vLines[nIdx];
}


/*!
\brief Parses a line into two std::string parameters
\author Xanathar
\param nIdx the index of the entry we want
\param strParam1 the 1st parameter
\param strParam2 the 2nd parameter
*/
void cScpSection::parseLine(int nIdx, std::string& strParam1, std::string& strParam2)
{
	class cScpEntry entry = m_vLines[nIdx];
	strParam1 = entry.getParam1();
	strParam2 = entry.getParam2();
}


/*!
\brief Parses a line into two char* parameters
\author Xanathar
\param nIdx the index of the entry we want
\param szParam1 the 1st parameter
\param szParam2 the 2nd parameter
*/
void cScpSection::parseLine(int nIdx, char* szParam1, char* szParam2)
{
	class cScpEntry entry = m_vLines[nIdx];
	strcpy(szParam1, entry.getParam1().c_str());
	strcpy(szParam2, entry.getParam2().c_str());
}


/*!
\brief Constructs a cScpScript object reading it from a file
\author Xanathar
\param szFileName name of file to be read
*/
cScpScript::cScpScript(char *szFileName)
{
	m_nErrors = 0;
	FILE *file = fopen(szFileName, "rt");
	ConOut("\tLoading script : %s ... ", szFileName);

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


/*!
\brief Loads a section from a file (eventually recursing)
\author Xanathar
\param szFileName name of file to be loaded
*/
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

cScpScript::~cScpScript()
{
	m_mapSections.erase(m_mapSections.begin(), m_mapSections.end() );
	m_mapSections.clear();
}


/*!
\brief Get Section for a given search id
\author Xanathar
\param strIdentifier the section id to be searched
\return the ptr to a cScpSection object or NULL if not found
*/
cScpSection* cScpScript::getSection(std::string strIdentifier)
{
	std::map<std::string, class cScpSection>::iterator iter;

	iter = m_mapSections.find(strIdentifier);

	return (iter!=m_mapSections.end())? &iter->second : NULL;
}


/*!
\brief Get Section for a given search id
\author Xanathar
\param szIdentifier the section id to be searched
\return the ptr to a cScpSection object or NULL if not found
*/
cScpSection* cScpScript::getSection(char* szIdentifier)
{
	return getSection( std::string( szIdentifier ) );
}


/*!
\brief Get Section for a given search id
\author Xanathar
\param szIdentifier the section id to be searched
\return the ptr to a cScpSection object or NULL if not found
\note search as sub-string
*/
cScpSection* cScpScript::getSectionInStr(char* szIdentifier)
{
	std::map<std::string, class cScpSection>::iterator iter;

	for (iter = m_mapSections.begin(); iter!=m_mapSections.end(); ++iter) {
		if (strstr(iter->first.c_str(), szIdentifier)) return &iter->second;
	}

	return NULL;
}


/*!
\author Xanathar
\brief Gets a new iterator object for a section identifier
\param strIdentifier the section identifier
\return the iterator or NULL if not found
*/
cScpIterator* cScpScript::getNewIterator(std::string& strIdentifier)
{
	cScpSection* section = getSection(strIdentifier);
	if (section==NULL) return NULL;
	return new cScpIterator(section);
}


/*!
\brief Gets a new iterator object for a section identifier
\author Xanathar
\since 0.82a
\param format the format string to create the section identifier
\return the iterator or NULL if not found
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
	return (section!=NULL)? new cScpIterator(section) : NULL;
}


/*!
\brief Gets a new iterator object for section containing the identifier
\author Xanathar
\param szIdentifier the identifier
\return the iterator or NULL if not found
\note search as sub-string
*/
cScpIterator* cScpScript::getNewIteratorInStr(char* szIdentifier)
{
	cScpSection* section = getSectionInStr(szIdentifier);
	if (section==NULL) return NULL;
	return new cScpIterator(section);
}


/*!
\brief Returns the first section id given search id
\author Xanathar, Sparhawk
\param strIdentifier the section id to be searched
\return ptr to the first section
*/
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


/*!
\brief Returns the next section id given search id
\author Sparhawk
\param strIdentifier the section id to be searched
\return ptr to the first section
*/
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


/*!
\brief Returns the number of cScpSection objects with given search id
\author Sparhawk
\param strIdentifier the section id to be searched
\return the number of objects
*/
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


/*!
\brief Returns the number of cScpSection objects with given search id
\author Sparhawk
\param szIdentifier the section id to be searched
\return the number of objects
\note search as sub-string
*/
UI32 cScpScript::countSectionInStr(char* szIdentifier)
{
	std::string str = szIdentifier;
	return countSectionInStr(str);
}


/*!
\brief Creates a new iterator for a given section
\author Xanathar
\param section the section to be created
*/
cScpIterator::cScpIterator(cScpSection* section)
{
	m_section = section;
	m_nPosition = 0;
}


/*!
\brief Creates a new iterator for a given section
\author Xanathar
\param section the section to be created
*/
cScpIterator::cScpIterator(cScpSection& section)
{
	m_section = &section;
	m_nPosition = 0;
}


cScpIterator::~cScpIterator()
{
	// Nothing to do here!
}


/*!
\brief Parses a splitted script line in two strings
\author Xanathar
\param strParam1 the 1st string to be filled
\param strParam2 the 2nd string to be filled
*/
void cScpIterator::parseLine(std::string& strParam1, std::string& strParam2)
{
	m_section->parseLine(m_nPosition++, strParam1, strParam2);
}


/*!
\brief Parses a splitted script line in two char*s
\author Xanathar
\param szParam1 the 1st string to be filled
\param szParam2 the 2nd string to be filled
*/
void cScpIterator::parseLine(char* szParam1, char* szParam2)
{
	m_section->parseLine(m_nPosition++, szParam1, szParam2);
}


/*!
\brief Gets a cScpEntry object from the section
\author Xanathar
\return pointer to section entry
*/
cScpEntry* cScpIterator::getEntry()
{
	return m_section->getEntry(m_nPosition++);
}


/*!
\brief Rewinds the iterator to its first position
\author Xanathar
*/
void cScpIterator::rewind()
{
	m_nPosition = 0;
}

