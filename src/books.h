  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

/*!
\file books.h
\brief Declaration of Books Namespace and cBook Class
\author Akron
\since 0.82r3
\date 07/12/1999 first write
\date 14/03/2003 total rewriting complete by Akron
\date 19/03/2003 books are now saved only if modified
*/

#ifndef __books_h
#define __books_h

#include <map>
#include <fstream>

/*!
\brief Books stuff
\author Akron
\since 0.82r3

new books readonly -> morex 999<br>
new books writeable -> morex 666<br>

*/
namespace Books
{
	void LoadBooks();
	void SaveBooks();
	void safeoldsave();
	void archive();
	void addNewBook(P_ITEM book);
	void DoubleClickBook(NXWSOCKET s, P_ITEM book);

	/*!
	\brief Item class for books
	\author Akron (complete rewrite)
	\todo create xss-based books

	There are two types of books, read only books, defined by scripters, in this case they are loaded
	from xss scripts and read/write books, wrote in game, that are saved to a save file on disk.

	Two books can have the same contents if it's defined by xss script, but not if they are
	read/write books.<br>
	For this reason, we need to duplicate the book's data if we are duping a book item, else we could
	have two books "linked" together, changing one, will change the other too.

	a read/write book is defined on the file like that (without tabs identation obviously :P):

	<pre>
	SECTION RWBOOK index
	{
		AUTHOR author
		TITLE title
		PAGE
		{
			LINE blablabla
			LINE bblalblal
		}
		PAGE
		{
			LINE blablabla
		}
		PAGE
		{
			LINE blablabla
		}
	}
	</pre>

	*/
	class cBook
	{
		public:
			typedef std::vector< std::vector<std::string> > tpages;

			//@{
			/*!
			\name Constructors and operators
			\author Akron
			*/
			cBook();					//!< default constructor
			cBook(const cBook &oldbook);			//!< copy constructor
			cBook(P_ITEM book);				//!< item constructor
			cBook(std::istream &s);				//!< constructor that read the book from stream (see cBooks::ReadFrom)
			cBook &operator = (const cBook & oldbook); 	//!< assignment operator =
			//@}

			//@{
			/*!
			\name Book Dump/Load functions
			\author Akron
			These functions are used to write and read books from a file or, in general from a stream.<br>
			This mean that we can truly use files for store books, but also other streams, like string streams.
			*/
			void DumpTo(std::ostream &s);			//!< write the book to a stream
			bool ReadFrom(std::istream &s);			//!< read the book from a stream
			//@}

			void OpenBookReadOnly(NXWSOCKET s, P_ITEM book);
			void OpenBookReadWrite(NXWSOCKET s, P_ITEM book);
			void SendPageReadOnly(NXWSOCKET s, P_ITEM book, UI16 p);

			void ChangeAuthor(char *auth);
			void ChangeTitle(char *titl);
			void ChangePages(char *packet, UI16 p, UI16 l, UI16 s);

			UI32 GetIndex() const				//!< gets the book index
				{ return index; }
			inline LOGICAL IsReadOnly() const		//!< return true if books is readonly
				{ return readonly; }
			inline void GetPages(tpages &pags) const	//!< return the pages of the book
				{ pags = pages; }
			std::string GetAuthor() const			//!< return the author of the book
				{ return author; }
			inline std::string GetTitle() const		//!< return the title of the book
				{ return title; }
			inline UI32 GetNumPages() const			//!< return the number of pages of the book
				{ return pages.size(); }

		private:
			LOGICAL readonly;				//!< if the book is loaded from script it must be true
			std::string author;				//!< author of the book
			std::string title;				//!< title of the book
			tpages pages;					//!< pages of the book

			/*!
			\brief index of the book
			\note When creating new books, it should be greater than cBook::books counter.

			Needed for get informations about the book, also used for book indexing.
			*/
			UI32 index;

			static UI32 books_index;			//!< 1-based books counter
	};

	extern std::map<UI32, cBook> books;
};

#endif
