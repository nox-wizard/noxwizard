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

/*!
\brief an Basic Menu
\author Endymion
*/
class cBasicMenu {
	protected:
		UI32	serial; //!< serial
		UI32	id; //!< gump id

		AmxEvent* callback; //function callback

	public:
		cBasicMenu( SERIAL menu, UI32 id );
		virtual ~cBasicMenu();

		void	setCallBack( const std::string& arg );
		void	setId( const UI32 arg );

		virtual void 	handleButton( const NXWSOCKET socket, const UI32 button );
		virtual void	show( P_CHAR pc );


};


/*!
\brief an Menu
\author Endymion
*/
class cMenu : public cBasicMenu
{
	private:
		std::vector< std::string >	commands; //!< all commands
		std::vector< wstring >	texts; //!< all strings
		UI08	options; //!< menu options
		UI32	x; //!< x coord where show
		UI32	y; //!< y location where show
		map< UI32, std::string > responseMap; //!< response for every input

	private:
		UI32 addString( wstring& s );

	public:
				
		cMenu( SERIAL menu, UI32 id, UI32 x, UI32 y, bool canMove, bool canClose, bool canDispose );
		virtual ~cMenu();

		void	setX( const UI32 arg );
		void	setY( const UI32 arg );

		virtual void 	handleButton( const NXWSOCKET socket, const UI32 button );
		virtual void	show( P_CHAR pc );

		void	addCommand( const std::string& command = "" );
		void	addCommand( char const* s, ... );

		void	addBackground( UI32 gumpId, UI32 width, UI32 height );
		void 	addButton( UI32 x, UI32 y, UI32 up, UI32 down, UI32 returnCode, UI32 page=1 );
		//void 	addCheckbox( UI32 x, UI32 y, UI32 off, UI32 on, UI32 checked, UI32 result );
		//void 	addCheckertrans( UI32 x, UI32 y, UI32 width, UI32 height );
		//void	addCroppedText( UI32 x, UI32 y, UI32 width, UI32 height, wstring& text, UI32 hue );
		//void 	addGroup( UI32 group = 0 );
		//void	addGump( UI32 x, UI32 y, UI32 gump, UI32 hue );
		//void	addHtmlGump( UI32 x, UI32 y, UI32 width, UI32 height, wstring& html, UI32 hasBack, UI32 canScroll );
		//void 	addInputField( UI32 x, UI32 y, UI32 width, UI32 height, UI32 textId, wstring& data, UI32 hue = 0 );
		void 	addPage( UI32 page = 0 );
		void 	addPageButton( UI32 x, UI32 y, UI32 up, UI32 down, UI32 page );
		//void 	addRadioButton( UI32 x, UI32 y, UI32 off, UI32 on, UI32 checked, UI32 result  );
		void 	addResizeGump( UI32 x, UI32 y, UI32 gumpId, UI32 width, UI32 height );
		void 	addText( UI32 x, UI32 y, wstring& data, UI32 hue = 0 );
		//void 	addTilePic( UI32 x, UI32 y, UI32 tile, UI32 hue = 0 );
		//void	addTiledGump( UI32 x, UI32 y, UI32 width, UI32 height, UI32 gump, UI32 hue );
		//void 	addXmfHtmlGump( UI32 x, UI32 y, UI32 width, UI32 height, wstring& clilocid, UI32 hasBack , UI32 canScroll );
		
		void	setOptions( const UI08 options, const bool value );
		void	setOptions( const UI08 options );
		void	setCloseAble( const bool arg );
		void	setDisposeAble( const bool arg );
		void	setMoveAble( const bool arg );

};



typedef cBasicMenu* P_MENU;
typedef std::map< UI32, P_MENU > MENU_MAP;

/*!
\brief all Menus
*/
class cMenus
{
	private:
		SERIAL current_serial; //!< current serial
		MENU_MAP menuMap; //!< every opened menus
	public:

		cMenus();
		virtual ~cMenus();

		SERIAL createMenu( P_MENU menu );
		LOGICAL	deleteMenu( SERIAL gump );
		LOGICAL	handleMenu( NXWCLIENT ps );
		P_MENU selectMenu( SERIAL menu );
		LOGICAL	showMenu( SERIAL menu, P_CHAR pc );
};

extern cMenus menus;

