  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __NXWGUMP_H__
#define __NXWGUMP_H__

#include "nxwcommn.h"
#include "nxwStringList.h"


class nxwGump
{
	typedef UI32					GUMPID;
	typedef UI32					SUBJECTID;
	typedef UI32					PROPERTY;

	typedef map<GUMPID, class nxwGump* >		GUMPMAP;
	typedef GUMPMAP::iterator			GUMPMAPIT;

	typedef map<GUMPID, std::string >		CALLBACKMAP;
	typedef CALLBACKMAP::iterator			CALLBACKMAPIT;

	typedef map<PROPERTY, std::string >		PROPERTYMAP;
	typedef PROPERTYMAP::iterator			PROPERTYMAPIT;

	typedef map<SUBJECTID, PROPERTYMAP>		SUBJECTPROPERTYMAP;
	typedef SUBJECTPROPERTYMAP::iterator		SUBJECTPROPERTYMAPIT;

	typedef map< GUMPID, SUBJECTPROPERTYMAP >	GUMPSUBJECTPROPERTYMAP;
	typedef GUMPSUBJECTPROPERTYMAP::iterator	GUMPSUBJECTPROPERTYMAPIT;

	typedef map< SERIAL, GUMPSUBJECTPROPERTYMAP >	RESPONSEMAP;
	typedef RESPONSEMAP::iterator			RESPONSEMAPIT;

	private:
		static GUMPMAP	gumpMap;
		static map<UI32, std::string > responseMap;
		static LOGICAL	addResponse( UI32 fieldID, std::string value );
	public:
		static LOGICAL	createGump( GUMPID gump, UI32 x, UI32 y, bool canMove, bool canClose, bool canDispose, UI32 serial);
		static LOGICAL	deleteGump( GUMPID gump );
		static LOGICAL	handleGump( const P_CHAR pc, const UI08 *data );
		static nxwGump* selectGump( GUMPID gump );
		static LOGICAL	showGump( GUMPID gump, P_CHAR pc );

		static LOGICAL	selectResponse( UI32 fieldId, std::string &value );

	private:
		nxwStringList	commands;
		nxwStringList	strings;
		bool		moveable;
		bool		closeable;
		bool		disposeable;
		BYTE*		packet;
		bool		dirty;
		GUMPID		gumpId;
		UI32		serial;
		UI32		x;
		UI32		y;

	private:
		void		createPacket();
		void		dropPacket();
		bool		isDirty();
		void		resetDirty();
		void		setDirty();

	public:
				nxwGump();
				nxwGump( GUMPID gump, UI32 x, UI32 y, bool canMove, bool canClose, bool canDispose, UI32 serial );
		virtual		~nxwGump();

		void 		addBackground( GUMPID gumpId, UI32 width, UI32 height );
		void 		addButton( UI32 x, UI32 y, UI32 up, UI32 down, UI32 returnCode );
		void 		addCheckbox( UI32 x, UI32 y, UI32 off, UI32 on, UI32 checked, UI32 result );
		void 		addCheckertrans( UI32 x, UI32 y, UI32 width, UI32 height );
		void		addCommand( const std::string& command = "" );
		void		addCroppedText( UI32 x, UI32 y, UI32 width, UI32 height, const char* text, UI32 hue );
		void 		addGroup( UI32 group = 0 );
		void		addGump( UI32 x, UI32 y, GUMPID gump, UI32 hue );
		void		addHtmlGump( UI32 x, UI32 y, UI32 width, UI32 height, const char* html, UI32 hasBack, UI32 canScroll );
		void 		addInputField( UI32 x, UI32 y, UI32 width, UI32 height, UI32 textId, const char* data, UI32 hue = 0 );
		void 		addPage( UI32 page = 0 );
		void 		addPageButton( UI32 x, UI32 y, UI32 up, UI32 down, UI32 page );
		void 		addRadioButton( UI32 x, UI32 y, UI32 off, UI32 on, UI32 checked, UI32 result  );
		void 		addResizeGump( UI32 x, UI32 y, GUMPID gumpId, UI32 width, UI32 height );
		void		addString( const std::string& text = "" );
		void 		addText( UI32 x, UI32 y, const char* data, UI32 hue = 0 );
		void 		addTilePic( UI32 x, UI32 y, UI32 tile, UI32 hue = 0 );
		void		addTiledGump( UI32 x, UI32 y, UI32 width, UI32 height, UI32 GUMPID, UI32 hue );
		void 		addXmfHtmlGump( UI32 x, UI32 y, UI32 width, UI32 height, const char* clilocid, UI32 hasBack , UI32 canScroll );
		void 		handleButton( const NXWSOCKET socket, const UI32 button );
		void		setCallBack( const std::string& arg );
		void		setCloseAble( const bool arg );
		void		setDisposeAble( const bool arg );
		void		setGumpId( const UI32 arg );
		void		setMoveAble( const bool arg );
		void		setX( const UI32 arg );
		void		setY( const UI32 arg );
		void		setSerial( const UI32 arg );

		void		show( P_CHAR pc );
		void		print();
};

#endif
