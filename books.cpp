  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "books.h"
#include "debug.h"
#include "network.h"
#include "worldmain.h"
#include "globals.h"
#include "items.h"
#include "chars.h"
#include "utils.h"
#include "basics.h"

namespace Books
{
	std::map<UI32, cBook> books;
	LOGICAL shouldsave = false;

	/*!
	\author Akron
	\brief Load books from savefile

	This function read the nxwBook.wsc savefile and load all the books saved in it.
	*/
	void LoadBooks()
	{
		std::string fileName = SrvParms->savePath + SrvParms->bookWorldfile + SrvParms->worldfileExtension;
		std::ifstream file(fileName.c_str());

		ConOut("Loading books worldfile...\n");

		cBook p;
		while(file)
		{
				p.ReadFrom(file);
				if ( p.GetIndex() )
				{
					books[p.GetIndex()] = p;
					ConOut("Loaded the book %d\n", p.GetIndex());
				}
				else
				{
					WarnOut("Error loading a book...\n");
				}
		}
	}

	/*!
	\author Akron
	\brief Save books to savefile

	This function write in the nxwBooks.wsc savefile the data about books.
	*/
	void SaveBooks()
	{
		if ( !shouldsave ) // save only if needed - requested by Endymion
			return;

		std::string fileName = SrvParms->savePath + SrvParms->bookWorldfile + SrvParms->worldfileExtension;
		std::ofstream file(fileName.c_str(), std::ios::out|std::ios::trunc);

		for(std::map<UI32, cBook>::iterator it = books.begin(); it != books.end(); it++)
			(*it).second.DumpTo(file);
	}

	/*!
	\author Akron
	\brief Set shouldsave flag to 1
	\note this function wait if the system is world saving for change the book
	*/
	void ShouldSave()
	{
		while ( cwmWorldState->Saving() );
		shouldsave = true;
	}

	/*!
	\author Akron
	\brief archive old savefiles
	*/
	void archive()
	{
		std::string saveFileName = SrvParms->savePath + SrvParms->bookWorldfile + SrvParms->worldfileExtension;
		std::string timeNow( getNoXDate() );
		for( int i = timeNow.length() - 1; i >= 0; --i )
			switch( timeNow[i] )
			{
				case '/' :
				case ' ' :
				case ':' :
					timeNow[i]= '-';
			}
		std::string archiveFileName( SrvParms->archivePath + SrvParms->bookWorldfile + timeNow + SrvParms->worldfileExtension );

		if( rename( saveFileName.c_str(), archiveFileName.c_str() ) ) {
			LogWarning("Could not rename/move file '%s' to '%s'\n", saveFileName.c_str(), archiveFileName.c_str() );
		} else {
			InfoOut("Renamed/moved file '%s' to '%s'\n", saveFileName.c_str(), archiveFileName.c_str() );
		}

	}

	/*!
	\author AKron
	\brief save the current savefile
	*/
	void safeoldsave()
	{
		std::string oldFileName( SrvParms->savePath + SrvParms->bookWorldfile + SrvParms->worldfileExtension );
		std::string newFileName( SrvParms->savePath + SrvParms->bookWorldfile + SrvParms->worldfileExtension + "$" );
		remove( newFileName.c_str() );
		rename( oldFileName.c_str(), newFileName.c_str() );

	}

	/*!
	\author Akron
	\brief add a new book to the map with book as bookitem
	\param book pointer to the book item
	*/
	void addNewBook(P_ITEM book)
	{
		cBook b(book);
		books[b.GetIndex()] = b;
		book->morez = b.GetIndex();
	}

	/*!
	\author Akron
	\brief Manage the doubleclick of a book-type item
	\param s socket that double clicked it
	\param book pointer to the book item
	\todo complete the support for xss-generated books
	*/
	void DoubleClickBook(NXWSOCKET s, P_ITEM book)
	{
		P_CHAR pc = MAKE_CHAR_REF(currchar[s]);
		if ( !ServerScp::g_nEnableBooks )
		{
			pc->sysmsg(TRANSLATE("Books are currently disabled, sorry :("));
			return;
		}

		if ( book->morez )
		{
			std::map<UI32, cBook>::iterator it = books.find(book->morez);
			if ( it == books.end() )
				book->morez = 0;
		}

		if ( book->morex != 666 && book->morex != 999 && book->morex != 333 )
		{
			if ( book->morez )
			{
				if ( books[book->morez].IsReadOnly() )
					book->morex = 333;
				else
					book->morex = 666;
			}
			else
					book->morex = 666;
		}

		if ( !book->morez )
		{
			if ( !book->morey )
				book->morey = 4;
			addNewBook(book);
		}
		else
			book->morey = books[book->morez].GetNumPages();

		if ( book->morex == 666 )
		{
			books[book->morez].OpenBookReadWrite(s, book);
		}
		else if ( book->morex == 999 )
		{
			books[book->morez].OpenBookReadOnly(s, book);
		}
	}

	cBook::cBook()
	{
		index = 0;
	}

	/*!
	\param book pointer to the book item to bound the book to
	*/
	cBook::cBook(P_ITEM book)
	{
		index = ++books_index;
		readonly = false;
		book->morez = index;

		std::string tmp("");
		std::vector<std::string> tmppage;
		for(int i = 0; i < 8; i++)
			tmppage.push_back(tmp);

		while( pages.size() < book->morey )
			pages.push_back(tmppage);

	}

	/*!
	\param s reference to the istream to read the book from
	*/
	cBook::cBook(std::istream &s)
	{
		readonly = false;
		ReadFrom(s);
	}

	/*!
	\brief Copy constructor for cBook class
	\param oldbook old book to copy the data from
	*/
	cBook::cBook(const cBook &oldbook)
	{
		index = oldbook.GetIndex();
		readonly = oldbook.IsReadOnly();
		author = oldbook.GetAuthor();
		title = oldbook.GetTitle();
		oldbook.GetPages(pages);
	}

	cBook & cBook::operator = (const cBook &oldbook)
	{
		index = oldbook.GetIndex();
		readonly = oldbook.IsReadOnly();
		author = oldbook.GetAuthor();
		title = oldbook.GetTitle();
		oldbook.GetPages(pages);
		return *this;
	}

	/*!
	\todo complete reading
	\param s reference to the istream to read the book from
	*/
	bool cBook::ReadFrom(std::istream &s)
	{
		pages.erase(pages.begin(), pages.end());
		author = "";
		title = "";
		index = 0;
		readonly = false;
		char readbuffer[256], *truebuffer;
		bool readheader = false;
		std::vector<std::string> tmppage;
		std::string line;
		UI32 numpages = 0;
		SI32 np = -1;

		while(s)
		{
			s.getline(readbuffer, 255);
			truebuffer = linestart(readbuffer);

			if ( ! *truebuffer ) // ignore empty lines
				continue;

			if ( truebuffer[0] == '}' )
			{
				if ( !readheader )
				{
					WarnOut("Invalid book script (%s)\n", truebuffer);
					return false;
				}
				else
					break;
			}
			else if ( truebuffer[0] == '{' )
			{
				WarnOut("Invalid book script (%s)\n", truebuffer);
				while(truebuffer[0] != '}')
				{
					s.getline(readbuffer, 255);
					truebuffer = linestart(readbuffer);
				}
				return false;
			}
			else if ( !strncasecmp("SECTION RWBOOK", truebuffer, 14 ) )
			{
				index = atoi(linestart(truebuffer+14));

				if ( ! index )
				{
					WarnOut("Invalid book index (%s)\n", truebuffer);
					return false;
				}
				if ( index > books_index )
					books_index = index;
				else
				{
					std::map<UI32, cBook>::iterator it = books.find(index);
					if ( it != books.end() )
					{
						WarnOut("Book index already in use (%s)\n", truebuffer);
						while(truebuffer[0] != '}')
						{
							s.getline(readbuffer, 255);
							truebuffer = linestart(readbuffer);
						}
						return false;
					}
				}

				s.getline(readbuffer, 255);
				truebuffer = linestart(readbuffer);
				if ( truebuffer[0] != '{' )
				{
					WarnOut("Invalid book script (%s)\n", truebuffer);
					return false;
				}
				readheader = true;
			}
			else if ( ! readheader )
			{
				WarnOut("Invalid entry in book script (%s)\n", truebuffer);
				continue;
			}
			else if ( !strncasecmp("AUTHOR", truebuffer, 6) )
				author = linestart(truebuffer+6);
			else if ( !strncasecmp("TITLE", truebuffer, 5) )
				title = linestart(truebuffer+5);
			else if ( !strncasecmp("NUMPAGES", truebuffer, 8) )
			{
				while ( tmppage.size() < 8 ) tmppage.push_back("");
				numpages = atoi(linestart(truebuffer+8));
				while ( pages.size() < numpages ) pages.push_back(tmppage);
			}
			else if ( !strncasecmp("PAGE", truebuffer, 4) )
				np = atoi(linestart(truebuffer+4));
			else if ( !strncasecmp("LINE", truebuffer, 4) )
			{	
				if ( np == -1 )
				{
					WarnOut("Invalid book script (%s)\n", truebuffer);
					return false;
				}
				char *linenumber = linestart(truebuffer+4);
				char *ls = strchr(linenumber, ' ');
				
				if ( ! ls ) continue;
				*ls = '\0'; ls++;

				int ln = atoi(linenumber);
							
				pages[np][ln] = ls;
			}
			else
				WarnOut("Invalid entry in book script (%s)\n", truebuffer);
		}

		if ( pages.size() < numpages )
		{
			line = "";
			for(int i = 0; i < 8; i++)
				tmppage.push_back(line);
			while( pages.size() < numpages )
				pages.push_back(tmppage);
		}

		return true;
	}

	/*!
	\author Akron
	\param s reference to the ostream where to dump the book
	*/
	// Modified by Wintermute, to support ms vc compilers
	void cBook::DumpTo(std::ostream &s)
	{
// #ifndef _MSC_VER
		if ( readonly )	// a readonly book shouldn't be dumped, but if we don't want to test it first...
			return; // note that a class should be totally independent for OOP...
		char indexStr[12];
		numtostr(index, indexStr);
		char pageStr[12];
		numtostr(pages.size(), pageStr);
		s	<< "SECTION RWBOOK " << indexStr << std::endl
			<< "{" << std::endl
			<< "AUTHOR " << author << std::endl
			<< "TITLE " << title << std::endl
			<< "NUMPAGES " << pageStr << std::endl;
		UI32 i=0;
		for(tpages::iterator it = pages.begin(); it != pages.end(); it++)
		{
			numtostr(i++,pageStr);
			s	<< "PAGE " << pageStr << " " << std::endl;

			for(std::vector<std::string>::iterator it2 = (*it).begin(); it2 != (*it).end(); it2++)
				if ( ! (*it2).empty() )
				{
					numtostr(distance((*it).begin(), it2), pageStr);
					s << "LINE " << pageStr << " " << *it2 << std::endl;
				}
		}

		s << "}" << std::endl << std::endl;
  // #endif
	}

	/*!
	\brief parse the 0x66 package replied by client for page changes
	\author Akron
	\param packet pointer to packet got from socket
	\param p index of page to change
	\param l lines to process
	\param size size of the packet buffer
	*/
	void cBook::ChangePages(char *packet, UI16 p, UI16 l, UI16 size)
	{
		p--; l--;
		UI16 bp = 0, lin = 0, lp = 0;
		char ch;
		char s[34];
		if ( p >= pages.size() )
		{
			WarnOut("Invalid page index in packet (index: %d max: %d) [books.cpp]\n", p, pages.size());
			return;
		}
		if ( l >= pages[p].size() )
		{
			WarnOut("Invalid line index in packet (index: %d max: %d) [books.cpp]\n", l, pages[p].size());
			return;
		}

		while( bp < size )
		{
			if ( bp > 512 ) bp = size;
			ch = packet[bp];

			if ( lin < 33 )
				s[lin] = ch;
			else
				s[33] = ch;

			bp++;
			lin++;

			if ( ! ch )
			{
				lp++;
				lin = 0;
				pages[p][lp-1] = s;
			}

		}
		ShouldSave();
	}

	/*!
	\author Akron
	\brief Change the book author
	\param auth new author of the book
	*/
	void cBook::ChangeAuthor(char *auth)
	{
		author = auth;
		ShouldSave();
	}

	/*!
	\author Akron
	\brief Change the book title
	\param titl new title of the book
	*/
	void cBook::ChangeTitle(char *titl)
	{
		title = titl;
		ShouldSave();
	}

	/*!
	\author Akron
	\brief Sends to the client data for opening book in read/write mode
	\param s socket to send the book to
	\param book pointer to book item
	\note it sends <b>a lot</b> of data to client....
	*/
	void cBook::OpenBookReadWrite(NXWSOCKET s, P_ITEM book)
	{
		UI08 bookopen[10]=   "\x93\x40\x01\x02\x03\x01\x01\x00\x02";

		UI16 bytes;

		char booktitle[60];
		char bookauthor[30];

		UI16 i;
		for(i = 0; i < author.size() && i < 30; i++)
			bookauthor[i] = author[i];
		for(; i < 30; i++)
			bookauthor[i] = '\0';

		for(i = 0; i < title.size() && i < 60; i++)
			booktitle[i] = title[i];
		for(; i < 30; i++)
			booktitle[i] = '\0';

		LongToCharPtr(book->getSerial32(), bookopen+1);
		ShortToCharPtr(pages.size(), bookopen+7);

		Xsend(s, bookopen, 9);

		Xsend(s, booktitle, 60);
		Xsend(s, bookauthor, 30);

		//////////////////////////////////////////////////////////////
		// Now we HAVE to send the ENTIRE Book                     / /
		// Cauz in writeable mode the client only sends out packets  /
		// if something  gets changed                                /
		// this also means -> for each bookopening in writeable mode /
		// lots of data has to be send.                              /
		//////////////////////////////////////////////////////////////

		UI08 bookpage_pre[9] = { 0x66, 0x01, 0x02, 0x40, 0x01, 0x02, 0x03, 0x00, 0x01 };
		UI08 bookpage[4] = { 0x00, 0x00, 0x00, 0x08 };

		bytes=9;

		LongToCharPtr(book->getSerial32(), bookpage_pre+3);

		tpages::iterator it ( pages.begin() );
		for ( ; it != pages.end(); it++ )
		{
			bytes += 4; // 4 bytes for each page
			UI16 j = 0;
			for( std::vector<std::string>::iterator it2 = (*it).begin(); it2 != (*it).end(); it2++, j++ )
				bytes += (*it2).size() + 1;
			while ( j++ < 8 )
				bytes += 2;
		}

		ShortToCharPtr(pages.size(), bookpage_pre+7);
		ShortToCharPtr(bytes, bookpage_pre+1);
		Xsend(s, bookpage_pre, 9);

		i = 1;
		it = pages.begin();
		for ( ; it != pages.end(); it++, i++ )
		{
			ShortToCharPtr(i, bookpage);

			Xsend(s, bookpage, 4);

			UI16 j = 0;
			for( std::vector<std::string>::iterator it2 = (*it).begin(); it2 != (*it).end(); it2++, j++ )
				Xsend(s, (*it2).c_str(), (*it2).size()+1);
			while ( j++ < 8 )
				Xsend(s, " ", 2);
		}
	}

	void cBook::OpenBookReadOnly(NXWSOCKET s, P_ITEM book)
	{
		UI08 bookopen[9] = { 0x93, 0x40, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x02 };

		char booktitle[61];
		char bookauthor[31];

		UI16 i;
		for(i = 0; i < author.size() && i < 31; i++)
			bookauthor[i] = author[i];
		for(; i < 31; i++)
			bookauthor[i] = '\0';

		for(i = 0; i < title.size() && i < 61; i++)
			booktitle[i] = title[i];
		for(; i < 31; i++)
			booktitle[i] = '\0';

		LongToCharPtr(book->getSerial32(), bookopen+1);
		ShortToCharPtr(pages.size(), bookopen+7);

		Xsend(s, bookopen, 9);
		Xsend(s, booktitle, 60);
		Xsend(s, bookauthor, 30);

	}

	/*!
	\brief send to the specified client the page for reading only
	\author Akron
	\param s socket to send the book page
	\param book pointer to book item
	\param p index of page to send
	*/
	void cBook::SendPageReadOnly(NXWSOCKET  s, P_ITEM book, UI16 p)
	{
		UI08 bookpage[13] =
		//	  cmd   -blocksize  --------book id-------  --pages---  --pagenum-  -linenum--
			{ 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x08 };
		//	   0     1     2     3     4     5     6     7     8     9     10    11    12
		UI16 bytes=13;

		if ( p >= pages.size() )
			return;

		std::vector<std::string> selpage = pages[p];

		for(std::vector<std::string>::iterator it = selpage.begin(); it != selpage.end(); it++)
			bytes += (*it).size() + 1;

		ShortToCharPtr(bytes, bookpage+1);
		LongToCharPtr(book->getSerial32(), bookpage+3);
		ShortToCharPtr(p, bookpage+9);
		ShortToCharPtr(selpage.size(), bookpage+11);

		Xsend(s, bookpage, 13);

		UI16 j = 0;
		for(std::vector<std::string>::iterator its = selpage.begin(); its != selpage.end(); its++, j++)
			Xsend(s, (*its).c_str(), (*its).size()+1);
	}

	UI32 cBook::books_index = 0;

};
