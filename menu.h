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
\author Endymion
*/

#ifndef _MENU_H_
#define _MENU_H_


#include "nxwcommn.h"
#include "packets.h"


#define MENUSTYLE_LARGE 128

//rename after from M_ to MENUTYPE_
enum MENU_TYPE {
	MENUTYPE_INVALID = INVALID,
	MENUTYPE_STONE = 0,
	MENUTYPE_SCROLL,
	MENUTYPE_PAPER,
	MENUTYPE_BLACKBOARD,
	MENUTYPE_TRASPARENCY,
	MENUTYPE_CUSTOM,
	MENUTYPE_ICONLIST = 16,
	MENUTYPE_ADDMENU
};

#define MENU_CLOSE 0


/*!
\brief an Basic Menu
\author Endymion
*/
class cBasicMenu {

	friend class cOldMenu;
	friend class cNewAddMenu;
	friend class cMenus;

	private:

		std::set<SERIAL> whoSeeThis; //!< who are seeing this menu

	protected:

		AmxFunction* callback; //!< function callback

		virtual cServerPacket* createPacket();
		virtual cServerPacket* build();

	public:

		MENU_TYPE id; //!< gump id

		cBasicMenu( MENU_TYPE id );
		~cBasicMenu();

		SERIAL	serial; //!< serial

		void setCallBack( std::string& arg );
		void setCallBack( FUNCIDX fn );

		virtual void handleButton( NXWCLIENT ps, cClientPacket* pkg  );
		virtual void show( P_CHAR pc );


};


typedef cBasicMenu* P_MENU;

#define ISVALIDPM( G )	( G!=NULL ) 
#define VALIDATEPM( G )	if( G==NULL )	return;
#define VALIDATEPMR( G, R )	if( G==NULL )	return R;

#define MENU_BUFF_COUNT 4
#define ISVALIDMENUBUFFER( I ) ( (I>INVALID) && (I<MENU_BUFF_COUNT) )

/*!
\brief an Menu
\author Endymion
*/
class cMenu : public cBasicMenu
{

	private:

		bool closeable;
		bool moveable;
		bool disposeable;
		
		SERIAL buttonCurrent;	//!< current button serial

		typedef struct { FUNCIDX fn; SI32 returnCode; } buttonCallbacks_st;
		map< SERIAL, buttonCallbacks_st > buttonCallbacks;	//!< all callback for button

		std::vector<SERIAL>* switchs; //!< switch ids on after menu selection
		std::map< SERIAL, std::wstring >* textResp; //!< edit field response

		std::map< SERIAL, SERIAL > propEditInfo; //!< sub props for property edit

		UI32 addString( wstring& s );

		void removeCommand( std::string& command );
		void removeCommand( char* s, ... );

		void setPropertyField( SERIAL type, SERIAL obj, SERIAL prop, SERIAL subProp, SERIAL subProp2, bool data );
		void setPropertyField( SERIAL type, SERIAL obj, SERIAL prop, SERIAL subProp, SERIAL subProp2, std::wstring& data );
		
		bool getPropertyFieldBool( SERIAL type, SERIAL obj, SERIAL prop, SERIAL subProp, SERIAL subProp2 );
		std::wstring getPropertyField( SERIAL type, SERIAL obj, SERIAL prop, SERIAL subProp, SERIAL subProp2 );

	protected:
		std::vector< std::string >	commands; //!< all commands
		std::vector< wstring >	texts; //!< all strings
	
		virtual cServerPacket* createPacket();

	public:
				
		UI32 x; //!< x coord where show
		UI32 y; //!< y location where show
		UI32 pageCount;	//!< page count
		UI32 pageCurrent;	//!< current page
		SERIAL buffer[MENU_BUFF_COUNT];
		std::string buffer_str[MENU_BUFF_COUNT];

		cMenu( MENU_TYPE id, UI32 x, UI32 y, bool canMove, bool canClose, bool canDispose );
		~cMenu();

		void setCloseable( bool canClose );
		bool getCloseable();
		void setDisposeable( bool canDispose );
		bool getDisposeable();
		void setMoveable( bool canMove );
		bool getMoveable();

		virtual void handleButton( NXWCLIENT ps,  cClientPacket* pkg  );

		void addCommand( std::string& command );
		void addCommand( char* s, ... );

		void clear();
 
		void addBackground( UI32 gumpId, UI32 width, UI32 height );
		void addButton( UI32 x, UI32 y, UI32 up, UI32 down, SERIAL returnCode, bool pressable );
		void addButtonFn( UI32 x, UI32 y, UI32 up, UI32 down, SI32 returnCode, bool pressable, FUNCIDX fn );
		void addCheckbox( UI32 x, UI32 y, UI32 off, UI32 on, UI32 checked, UI32 result );
		void addCheckertrans( UI32 x, UI32 y, UI32 width, UI32 height );
		void addCroppedText( UI32 x, UI32 y, UI32 width, UI32 height, wstring& text, UI32 hue );
		void addGump( UI32 x, UI32 y, UI32 gump, UI32 hue );
		void addHtmlGump( UI32 x, UI32 y, UI32 width, UI32 height, wstring& html, UI32 hasBack, UI32 canScroll );
		void addInputField( UI32 x, UI32 y, UI32 width, UI32 height, UI32 textId, wstring& data, UI32 hue = 0 );
		void addPropertyField( UI32 x, UI32 y, UI32 width, UI32 height, UI32 property, UI32 subProperty, UI32 hue = 0, UI32 subProperty2 = INVALID );
		void addRadioButton( UI32 x, UI32 y, UI32 off, UI32 on, UI32 checked, UI32 result  );
		void addResizeGump( UI32 x, UI32 y, UI32 gumpId, UI32 width, UI32 height );
		void addText( UI32 x, UI32 y, wstring& data, UI32 hue = 0 );
		void addTilePic( UI32 x, UI32 y, UI32 tile, UI32 hue = 0 );
		void addTiledGump( UI32 x, UI32 y, UI32 width, UI32 height, UI32 gump, UI32 hue );
		void addXmfHtmlGump( UI32 x, UI32 y, UI32 width, UI32 height, wstring& clilocid, UI32 hasBack , UI32 canScroll );
		//void 	addGroup( UI32 group = 0 );
		void addPage( UI32 page );
		void addPageButton( UI32 x, UI32 y, UI32 up, UI32 down, UI32 page );

		bool getCheckBox( SERIAL checkbox );
		bool getRadio( SERIAL radio );
		std::wstring* getText( SERIAL text );
};


class cIconListMenu : public cBasicMenu
{

	private:

		std::vector< pkg_icon_list_menu_st > icons;
		std::map< SERIAL, SI32 > iconData;

	protected:

		virtual cServerPacket* createPacket();

	public:

		std::string question;

		cIconListMenu();
		~cIconListMenu();

		virtual void handleButton( NXWCLIENT ps,  cClientPacket* pkg  );
		void addIcon( UI16 model, COLOR color, std::string response );
		void addIcon( UI16 model, COLOR color, SI32 data, std::string response );

};


inline bool isIconList( NXWSOCKET s );
inline bool isIconList( UI08 cmd );

typedef std::map< UI32, P_MENU > MENU_MAP;

/*!
\brief all Menus
*/
class cMenus
{
	
	friend class cBasicMenu;

	private:
		SERIAL current_serial; //!< current serial
		MENU_MAP menuMap; //!< every opened menus

		std::map< SERIAL, std::set<SERIAL> > whoSeeWhat; //!< player see menus

		bool removeFromView( P_MENU menu, SERIAL chr );

	public:

		cMenus();
		~cMenus();

		P_MENU insertMenu( P_MENU menu );
		SERIAL removeMenu( SERIAL menu, P_CHAR pc = NULL );
		bool handleMenu( NXWCLIENT ps );
		P_MENU getMenu( SERIAL menu );
		bool showMenu( SERIAL menu, P_CHAR pc );

};

extern cMenus Menus;

#endif
