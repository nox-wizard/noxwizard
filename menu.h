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
\brief Menu related stuff
*/

#include "nxwcommn.h"
#include "packets.h"

enum MENU_OPTIONS {
	MOVEABLE = 0x01,
	CLOSEABLE = 0x02,
	DISPOSEABLE = 0x04
};

typedef std::map< UI32, class cMenu* > MENU_MAP;

class cMenu
{
	private:
		static MENU_MAP menuMap;
		static map< UI32, std::string > responseMap;
		static LOGICAL	addResponse( UI32 fieldID, std::string value );
	public:
		static LOGICAL	createGump( UI32 gump, UI32 x, UI32 y, bool canMove, bool canClose, bool canDispose, UI32 serial);
		static LOGICAL	createGump( UI32 gump, UI32 x, UI32 y, UI08 options, UI32 serial);
		static LOGICAL	deleteGump( UI32 gump );
		static LOGICAL	handleGump( NXWCLIENT ps );
		static cMenu* selectGump( UI32 gump );
		static LOGICAL	showGump( UI32 gump, P_CHAR pc );

		static LOGICAL	selectResponse( UI32 fieldId, std::string &value );

	private:
		std::vector< std::string >	commands; //!< all commands
		std::vector< ustring >	texts; //!< all strings
		UI08	options; //!< menu options
		UI32	gumpId; //!< gump id
		UI32	serial; //!< serial
		UI32	x; //!< x coord where show
		UI32	y; //!< y location where show
		class AmxEvent	*callback; //!< amx callback

	private:
		UI32 addString( ustring& s );

	public:
				
		cMenu();
		cMenu( UI32 gump, UI32 x, UI32 y, UI08 options, UI32 serial );
		cMenu( UI32 gump, UI32 x, UI32 y, bool canMove, bool canClose, bool canDispose, UI32 serial );
		virtual ~cMenu();

		void	addCommand( const std::string& command = "" );
		void	addCommand( char const* s, ... );

		void	addBackground( UI32 gumpId, UI32 width, UI32 height );
		void 	addButton( UI32 x, UI32 y, UI32 up, UI32 down, UI32 returnCode );
		void 	addCheckbox( UI32 x, UI32 y, UI32 off, UI32 on, UI32 checked, UI32 result );
		void 	addCheckertrans( UI32 x, UI32 y, UI32 width, UI32 height );
		void	addCroppedText( UI32 x, UI32 y, UI32 width, UI32 height, ustring& text, UI32 hue );
		void 	addGroup( UI32 group = 0 );
		void	addGump( UI32 x, UI32 y, UI32 gump, UI32 hue );
		void	addHtmlGump( UI32 x, UI32 y, UI32 width, UI32 height, ustring& html, UI32 hasBack, UI32 canScroll );
		void 	addInputField( UI32 x, UI32 y, UI32 width, UI32 height, UI32 textId, ustring& data, UI32 hue = 0 );
		void 	addPage( UI32 page = 0 );
		void 	addPageButton( UI32 x, UI32 y, UI32 up, UI32 down, UI32 page );
		void 	addRadioButton( UI32 x, UI32 y, UI32 off, UI32 on, UI32 checked, UI32 result  );
		void 	addResizeGump( UI32 x, UI32 y, UI32 gumpId, UI32 width, UI32 height );
		void 	addText( UI32 x, UI32 y, ustring& data, UI32 hue = 0 );
		void 	addTilePic( UI32 x, UI32 y, UI32 tile, UI32 hue = 0 );
		void	addTiledGump( UI32 x, UI32 y, UI32 width, UI32 height, UI32 gump, UI32 hue );
		void 	addXmfHtmlGump( UI32 x, UI32 y, UI32 width, UI32 height, ustring& clilocid, UI32 hasBack , UI32 canScroll );
		void 	handleButton( const NXWSOCKET socket, const UI32 button );
		void	setCallBack( const std::string& arg );

		
		void	setOptions( const UI08 options, const bool value );
		void	setOptions( const UI08 options );
		void	setCloseAble( const bool arg );
		void	setDisposeAble( const bool arg );
		void	setMoveAble( const bool arg );

		void	setGumpId( const UI32 arg );
		void	setX( const UI32 arg );
		void	setY( const UI32 arg );
		void	setSerial( const UI32 arg );

		void	show( P_CHAR pc );

};
