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
\brief Custom Menu Handling functions
\author Xanathar
*/

#ifndef __CUSTMENU_H__
#define __CUSTMENU_H__

typedef vector< string *>	stringList;

//! Classic menu style, with stone background
#define MENUSTYLE_STONE 0
//! Classic menu style, with scroll-like background
#define MENUSTYLE_SCROLL 1
//! Classic menu style, with paper-like background
#define MENUSTYLE_PAPER 2
//! Classic menu style, with blackboard-like background
#define MENUSTYLE_BLACKBOARD 3
//! Classic menu style, with trasparent background
#define MENUSTYLE_TRASPARENCY 4
//! Make-menu style
#define MENUSTYLE_ICONLIST 16
//! Ummon icon-menu style
#define MENUSTYLE_ICONMENU 17


class cCustomMenu {
protected:
	char		**m_strLabels;
	stringList	m_lstButtons;
	stringList	m_lstLabels;
	int			m_nCallback;
	int			m_nNumPerPage;
	int			m_nNumPages;
	int			m_nMaxItemn;
	bool		m_bFirstTime;
	char		m_strTitle[50];
	int			m_nStyle;
	int			m_nColor;
	int			m_nSeed;
	bool		m_bPrepared;
	void SendVecsAsGump( NXWSOCKET  sock, stringList& one, stringList& two, unsigned char type );
	void buildClassicMenu ();
	void buildIconList ();
	void buildIconMenu ();
	void sendIconList (NXWSOCKET  s);
	void sendIconMenu (NXWSOCKET  s);
public:
	cCustomMenu();
	virtual ~cCustomMenu();
	void setParameters(int numPerPage, int numpages);
	void addMenuItem(int page, int idx, char *desc);
	void setCallback(int cback) { m_nCallback = cback; }
	void buildMenu ();
	void showMenu (NXWSOCKET  s);
	void setTitle (char *str);
	void setWidth (int width);
	void setStyle(int style, int color = 0);
	void setColor(int color);
	virtual void buttonSelected(NXWSOCKET s, unsigned short int buttonPressed, int type );
};

#endif //__CUSTMENU_H__
