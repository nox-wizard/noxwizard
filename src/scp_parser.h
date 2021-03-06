  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\file
\brief SCP Parser Stuff
*/
#ifndef __SCP_PARSER_H__
#define __SCP_PARSER_H__

#include "nxwcommn.h"

bool evaluateOneDefine (char *szLine, bool check=true);

/*!
\brief script iterator

This class provides access to a section's entries
\todo is this really necessary? it does not provide much additional features to cScpSection class
*/
class cScpIterator
{
private:
	int m_nPosition;
	class cScpSection* m_section;
protected:
public:
	explicit cScpIterator(cScpSection* section);
	explicit cScpIterator(cScpSection& section);
	~cScpIterator();
	void parseLine(std::string& strParam1, std::string& strParam2);
	void parseLine(char* szParam1, char* szParam2);
	class cScpEntry* getEntry();
	void rewind();
};

/*!
\brief a script entry

This class encapsulates a script line
*/
class cScpEntry
{
private:
	std::string m_strFullLine;
	std::string m_strParam1;
	std::string m_strParam2;
protected:
public:
	explicit cScpEntry(char *szStringToParse);
	std::string getFullLine();
	std::string getParam1();
	std::string getParam2();
};


/*!
\brief script section

encapsulates a script section
*/
class cScpSection
{
private:
	std::vector<class cScpEntry> m_vLines;
protected:
public:
	cScpSection (FILE *file, class cScpScript* parent);
	cScpSection (){};
	~cScpSection();
	void parseLine(int nIdx, std::string& strParam1, std::string& strParam2);
	void parseLine(int nIdx, char* szParam1, char* szParam2);
	cScpEntry* getEntry(int nPosition);
};

/*!
\brief script class

\todo add REAL parsing capabilities, as now it's only a text container with #define parsing. Should have a parse(parsingRules) member that calls appropriate parsing functions for each command. This would make other classes' parsers easier to write nad more standard
*/
class cScpScript
{
private:
protected:
	std::map<std::string, class cScpSection> m_mapSections;
	int m_nErrors;

	std::map<std::string, class cScpSection>::iterator m_iter;	// Sparhawk

public:
	cScpScript(char* szFileName);
	void load(char* szFileName);
	~cScpScript();
	class cScpSection*	getSection(std::string strIdentifier);
	class cScpSection*	getSection(char* szIdentifier);
	class cScpSection*	getSectionInStr(char* szIdentifier);
	class cScpIterator* getNewIterator(std::string& strIdentifier);
//	class cScpIterator* getNewIterator(char* szIdentifier);
	class cScpIterator* getNewIterator(char* format, ...);
	class cScpIterator* getNewIteratorInStr(char* szIdentifier);

	// Sparhawk	Additions for race script processing
		UI32		countSectionInStr(std::string& strIdentifier);
	    UI32		countSectionInStr(char* szIdentifier);
  		string*		getFirstSectionParmsInStr(std::string& strIdentifier);
		string*		getNextSectionParmsInStr(std::string& strIdentifier);
};


namespace xss {
	int getIntFromDefine (char *, bool check=true);
	char* evalDefines (char *szLine);
}


#endif //__SCP_PARSER_H__


