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
\brief an Menu
*/
class cMenu
{
	private:
		std::vector< std::string >	commands; //!< all commands
		std::vector< wstring >	texts; //!< all strings
		UI08	options; //!< menu options
		UI32	id; //!< gump id
		UI32	serial; //!< serial
		UI32	x; //!< x coord where show
		UI32	y; //!< y location where show
		class AmxEvent	*callback; //!< amx callback
		map< UI32, std::string > responseMap; //!< response for every input

	private:
		UI32 addString( wstring& s );

	public:
				
		cMenu();
		cMenu( SERIAL menu, UI32 id, UI32 x, UI32 y, UI08 options );
		cMenu( SERIAL menu, UI32 id, UI32 x, UI32 y, bool canMove, bool canClose, bool canDispose );
		virtual ~cMenu();

		void	setId( const UI32 arg );
		void	setX( const UI32 arg );
		void	setY( const UI32 arg );

		void 	handleButton( const NXWSOCKET socket, const UI32 button );
		void	setCallBack( const std::string& arg );
		void	show( P_CHAR pc );

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




/*!
\brief an Old type menu
*/
class cOldMenu : public cMenu
{

private:

	void buildClassicMenu();
	void buildIconList();
	void buildIconMenu();

	UI32 style;
	UI32 color;
	UI32 width;
	wstring title;
	std::vector< std::vector< std::wstring >  > allPages;

public:
	cOldMenu();
	virtual ~cOldMenu();

	void setParameters(int numPerPage, int numpages);
	void addMenuItem(int page, int idx, char *desc);
	//void setCallback( int cback );
	void buildMenu ();
	void showMenu (NXWSOCKET  s);
	void setTitle (char *str);
	void setWidth (int width);
	void setStyle(int style, int color = 0);
	void setColor(int color);
	virtual void buttonSelected(NXWSOCKET s, unsigned short int buttonPressed, int type );

};


typedef cMenu* P_MENU;
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

		SERIAL	createMenu( UI32 id, UI32 x, UI32 y, bool canMove, bool canClose, bool canDispose );
		SERIAL	createMenu( UI32 id, UI32 x, UI32 y, UI08 options );
		LOGICAL	deleteMenu( SERIAL gump );
		LOGICAL	handleMenu( NXWCLIENT ps );
		P_MENU selectMenu( SERIAL menu );
		LOGICAL	showMenu( SERIAL menu, P_CHAR pc );
};

extern cMenus menus;

