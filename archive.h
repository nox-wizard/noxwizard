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
\brief Archive stuff
*/

#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__


typedef std::map< SERIAL, P_OBJECT > OBJECT_MAP;

class cAllObjects {

protected:

	friend class cAllObjectsIter;
	friend class cAllCharsIter;
	friend class cAllItemsIter;


	static OBJECT_MAP all; //!< all objects

	static SERIAL current_char_serial; //!< current char serial
	static SERIAL current_item_serial; //!< current item serial

public:

	cAllObjects();
	~cAllObjects();

	void insertObject( P_OBJECT obj );
	void eraseObject( P_OBJECT obj );
	P_OBJECT findObject(SERIAL nSerial);
	SERIAL getNextCharSerial();
	SERIAL getNextItemSerial();
	void updateCharSerial( SERIAL ser );
	void updateItemSerial( SERIAL ser );
	void clear();

};


extern cAllObjects objects;

class cAllObjectsIter {

public:

	OBJECT_MAP::iterator curr; //current, can be deleted safaly
	OBJECT_MAP::iterator next; //next for save delete of current

public:

	cAllObjectsIter( );
	~cAllObjectsIter();

	void rewind();
	bool IsEmpty();
	P_OBJECT getObject();
	SERIAL getSerial();

	cAllObjectsIter& operator++(int);          

};

	
class cAllCharsIter : public cAllObjectsIter{

private:

public:

	cAllCharsIter(  );
	~cAllCharsIter();

	void rewind();
	bool IsEmpty();
	P_CHAR getChar();

	cAllCharsIter& operator++(int);          

};

class cAllItemsIter: public cAllObjectsIter {

private:

public:

	cAllItemsIter(  );
	~cAllItemsIter();

	void rewind();
	bool IsEmpty();
	P_CHAR getItem();

	
	cAllItemsIter& operator++(int);          

};

namespace archive {

	namespace item
	{
		P_ITEM	New( LOGICAL forCopyPurpose = false );
	}

	void	deleteItem( P_ITEM pi );
	void	deleteItem( SERIAL i );

	namespace character
	{
		P_CHAR	New();
		P_CHAR	Instance();
		void	Delete( P_CHAR pc );
		void	Delete( SERIAL k  );
	}
}


#endif
