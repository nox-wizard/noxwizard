/*
    This file is part of NoX-Wizard
    -----------------------------------------------------------------------------
    Portions are copyright 2001, Marco Mastropaolo (Xanathar).

    NoX-Wizard is a modified version of Uox (and before that of wolfpack)
    so all previous copyright notices applies and above all

	Copyright 1997, 98 by Marcus Rating (Cironian)

    -----------------------------------------------------------------------------

    NoX-Wizard is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    NoX-Wizard is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with NoX-Wizard; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    -----------------------------------------------------------------------------

    NoX-Wizard also contains portions of code from the Small toolkit.

    The software toolkit "Small", the compiler, the abstract machine and the
    documentation, are copyright (c) 1997-2001 by ITB CompuPhase.
	See amx_vm.c and/or small_license.txt for more information about this.

    ==============================================================================
*/

#include "nxwcommn.h"
#include "network.h"
#include "sndpkg.h"
#include "collector.h"
#include "custmenu.h"

#define MAXSTRLEN 50
#define MENUSTYLE_LARGE 128

#define INRANGE(A,B,C) ((A>=B)&&(A<C))

static void limited_strcpy (char *a, char *b)
{
	int i, ln = strlen(b);
	ln = qmin(ln, 48);
	for (i=0; i<=ln; i++)
		a[i] = b[i];
}

/*** XAN : This is ripped from UOX3-20d :) ***/
void cCustomMenu::SendVecsAsGump( NXWSOCKET  sock, stringList& one, stringList& two, unsigned char type )
// PRE:		Sock exists, one and two exists, type is valid num
// POST:	Sends to socket sock the data in one and two.  One is control, two is data
// CODER:	Abaddon
{
	unsigned int linecount = 0, linecount1 = 0;
	unsigned char i = 0;
	char sect[512];
	int length, length2;

	length = 21;
	length2 = 1;

	linecount = one.size();
	linecount1 = two.size();
	unsigned int line, textlines;
	for( line = 0; line < linecount; line++ )
	{
		if( one[line]->length() == 0 )
			break;
		length += one[line]->length() + 4;
		length2 += one[line]->length() + 4;
	}

	length += 3;
	textlines = 0;
	line = 0;

	for(line = 0; line < linecount1; line++ )
	{
		if( two[line]->length() == 0 )
			break;
		length += two[line]->length() * 2 + 2;
		textlines++;
	}

	int n = rand()%0x7FFFFFFF;
	unsigned char gump1[22]="\xB0\x04\x0A\x40\x91\x51\xE7\x00\x00\x00\x03\x00\x00\x00\x6E\x00\x00\x00\x46\x02\x3B";
	gump1[3] = n&(0xFF);
	gump1[4] = (n>>8)&(0xFF);
	gump1[5] = (n>>16)&(0xFF);
	gump1[6] = (n>>24)&(0xFF);
	m_nSeed = n;
	gump1[1] = length>>8;
	gump1[2] = length%256;
	gump1[7] = 0;
	gump1[8] = 0;
	gump1[9] = 0;
	gump1[10] = type; // Gump Number
	gump1[19] = length2>>8;
	gump1[20] = length2%256;
	Xsend( sock, gump1, 21 );

	for( line = 0; line < linecount; line++ )
	{
		sprintf(sect, "{ %s }", one[line]->c_str() );
		Xsend( sock, sect, strlen( sect ) );
	}

	unsigned char gump2[4]="\x00\x00\x00";
	gump2[1] = textlines>>8;
	gump2[2] = textlines%256;

	Xsend( sock, gump2, 3 );

	unsigned char gump3[3]="\x00\x00";
	for( line = 0; line < linecount1; line++ )
	{
		if( two[line]->length() == 0 )
			break;
		gump3[0] = ( two[line]->length() )>>8;
		gump3[1] = ( two[line]->length() )%256;
		Xsend( sock, gump3, 2 );
		gump3[0]=0;
		for ( i = 0; i < two[line]->length(); i++ )
		{
			gump3[1] = (*two[line])[i];
			Xsend( sock, gump3, 2 );
		}
	}
}

static inline char hex2byte (char *str)
{
	char c;
	char b = 0;

	c = str[0];
	if ((c >= '0')&&(c <= '9')) c -= '0';
	else if ((c >= 'A')&&(c <= 'F')) c -= 'A'-'\xa';
	else if ((c >= 'a')&&(c <= 'f')) c -= 'a'-'\xa';
	else c = 0;

	b = c << 4;

	c = str[1];
	if ((c >= '0')&&(c <= '9')) c -= '0';
	else if ((c >= 'A')&&(c <= 'F')) c -= 'A'-'\xa';
	else if ((c >= 'a')&&(c <= 'f')) c -= 'a'-'\xa';
	else c = 0;

	b += (c & 0xF);

	return b;
}

void cCustomMenu::sendIconMenu (NXWSOCKET  s)
{
}


void cCustomMenu::sendIconList (NXWSOCKET  s)
{
	int total, i;
	int lentext = strlen(m_strTitle);
	total = 9+1+lentext+1;

	for (i=0;i<m_nMaxItemn;i++) total+=strlen(m_strLabels[i]);

	unsigned char gmprefix[10]="\x7C\x00\x00\x01\x02\x03\x04\x00\x64";
	gmprefix[1]=total>>8;
	gmprefix[2]=total%256;

	int n = rand()%0x7FFFFFFF;
	gmprefix[3] = n&(0xFF);
	gmprefix[4] = (n>>8)&(0xFF);
	gmprefix[5] = (n>>16)&(0xFF);
	gmprefix[6] = (n>>24)&(0xFF);
	m_nSeed = n;

	gmprefix[7]=16384>>8; //custmenuidx = 16384
	gmprefix[8]=16384%256;

	Xsend(s, gmprefix, 9);
	Xsend(s, &lentext, 1);   //XAN :=> THIS IS LITTLE-ENDIAN ONLY!!!!, should be fixed :[
	Xsend(s, m_strTitle, lentext);
	lentext=m_nMaxItemn;
	Xsend(s, &lentext, 1);   //XAN :=> THIS IS LITTLE-ENDIAN ONLY!!!!, should be fixed :[
	unsigned char gmmiddle[5]="\x00\x00\x00\x00";
	for (i=0;i<m_nMaxItemn;i++)
	{
		gmmiddle[0]=hex2byte(m_strLabels[i]);
		gmmiddle[1]=hex2byte(m_strLabels[i]+2);
		Xsend(s, gmmiddle, 4);
		lentext=strlen(m_strLabels[i]+5);
		Xsend(s, &lentext, 1);
		Xsend(s, m_strLabels[i]+5, lentext);
	}
}


void cCustomMenu::showMenu(NXWSOCKET  target)
{
	switch (m_nStyle)
	{
		case MENUSTYLE_STONE:
		case MENUSTYLE_SCROLL:
		case MENUSTYLE_PAPER:
		case MENUSTYLE_BLACKBOARD:
		case MENUSTYLE_TRASPARENCY:
			SendVecsAsGump( target, m_lstButtons, m_lstLabels, 130 );
			break;
		case MENUSTYLE_ICONLIST:
			sendIconList(target);
			break;
		case MENUSTYLE_ICONMENU:
			sendIconMenu(target);
			break;
		default:
			m_nStyle = MENUSTYLE_STONE;
			buildClassicMenu();
			WarnOut("cCustomMenu::buildMenu() : unsupported menu style was used\n");
			break;
	}



}


cCustomMenu::cCustomMenu()
{
	m_bFirstTime = true;
	m_strLabels = NULL;
	m_nCallback = -3;
	m_nStyle = 0;
	m_strTitle[0] = '\0';
	m_bPrepared = false;
}



cCustomMenu::~cCustomMenu()
{
	int i;

	for (i = 0; i<m_nMaxItemn; i++)
		safedelete(m_strLabels[i]);

	if ( m_bPrepared ) {
		int size = m_lstLabels.size();
		for (i = 0; i < size; i++ )
			safedelete(m_lstLabels[i]);
		size = m_lstButtons.size();
		for (i = 0; i < size; i++ )
			safedelete(m_lstButtons[i]);
	}
}


void cCustomMenu::setStyle (int style, int color)
{
	m_nStyle = style;
	m_nColor = color;
}

void cCustomMenu::setColor (int color)
{
	m_nColor = color;
}


void cCustomMenu::setParameters(int numPerPage, int numpages)
{
	m_nNumPerPage = numPerPage;
	m_nNumPages = numpages;
	m_nMaxItemn = numPerPage*numpages;

	m_strLabels = new char*[m_nMaxItemn];
	for (int i = 0; i<m_nMaxItemn; i++)
	{
		m_strLabels[i] = new char[MAXSTRLEN];
		m_strLabels[i][0] = '\0';
	}
}

void cCustomMenu::setTitle (char *str)
{
	limited_strcpy(m_strTitle, str);
}

void cCustomMenu::addMenuItem(int page, int idx, char *desc)
{
	if (INRANGE(page,0,m_nNumPages) && INRANGE(idx,0,m_nNumPerPage))
		limited_strcpy (m_strLabels[page*m_nNumPerPage+idx],  desc);
}





/*** Builds the menu ***/
void cCustomMenu::buildClassicMenu ()
{
	char temp[512];
	int i,k;
	unsigned int pagenum = 1, position = 80, linenum = 1, buttonnum = m_nNumPerPage;
	int realCount = 0;
	int pagebtny = 307;

	if (m_strLabels==NULL) return;

	m_bPrepared = true;

	if (!m_bFirstTime) {
		int minSize;

		minSize = qmin( m_lstButtons.size(), m_lstLabels.size() );
		for( i = 0; i < minSize; i++ )
		{
			safedelete(m_lstButtons[i]);
			safedelete(m_lstLabels[i]);
		}
		int size = m_lstButtons.size();
		for( i = minSize; i < size; i++ )
			safedelete(m_lstButtons[i]);
		size = m_lstLabels.size();
		for( i = minSize; i < size; i++ )
			safedelete(m_lstLabels[i]);

		m_lstButtons.resize( 0 );
		m_lstLabels.resize( 0 );
	}

	m_bFirstTime = false;

	int width = 320;
	if (m_nStyle&MENUSTYLE_LARGE) width = 512;

	m_nStyle&=~MENUSTYLE_LARGE;

	//--static pages
	if (m_nStyle==MENUSTYLE_STONE) {
		m_lstButtons.push_back( new string( "noclose" ) );
		m_lstButtons.push_back( new string( "page 0"  ) );
		sprintf( temp,"resizepic 0 0 %i %i 340", 2600, width );
		m_lstButtons.push_back( new string( temp ) );
		sprintf( temp, "button 250 17 %i %i 1 0 1", 4017, 4017 + 1 );
		m_lstButtons.push_back( new string( temp ) );
		sprintf( temp, "text 30 40 %i 0", m_nColor );           //text <Spaces from Left> <Space from top> <Length, Color?> <# in order>
		m_lstButtons.push_back( new string( temp ) );
		pagebtny = 300;
	}
	if (m_nStyle==MENUSTYLE_BLACKBOARD) {
		m_lstButtons.push_back( new string( "noclose" ) );
		m_lstButtons.push_back( new string( "page 0"  ) );
		sprintf( temp, "resizepic 0 0 %i 320 340", 2620/*0x1432*/ );
		m_lstButtons.push_back( new string( temp ) );
		sprintf( temp, "button 250 17 %i %i 1 0 1", 4017, 4017 + 1 );
		m_lstButtons.push_back( new string( temp ) );
		sprintf( temp, "text 45 17 %i 0", m_nColor );
		m_lstButtons.push_back( new string( temp ) );
		pagebtny = 307;
	}

	if (m_nStyle==MENUSTYLE_PAPER) {
		m_lstButtons.push_back( new string( "noclose" ) );
		m_lstButtons.push_back( new string( "page 0"  ) );
		sprintf( temp, "resizepic 0 0 %i 320 340", 0xdac/*0x1432*/ );
		m_lstButtons.push_back( new string( temp ) );
		sprintf( temp, "button 250 7 %i %i 1 0 1", 4017, 4017 + 1 );
		m_lstButtons.push_back( new string( temp ) );
		sprintf( temp, "text 45 7 %i 0", m_nColor );
		m_lstButtons.push_back( new string( temp ) );
		pagebtny = 307;
	}

	if (m_nStyle==MENUSTYLE_SCROLL) {
		m_lstButtons.push_back( new string( "noclose" ) );
		m_lstButtons.push_back( new string( "page 0"  ) );
		sprintf( temp, "resizepic 0 0 %i 320 340", 0x1432/*0x1432*/ );
		m_lstButtons.push_back( new string( temp ) );
		sprintf( temp, "button 250 27 %i %i 1 0 1", 4017, 4017 + 1 );
		m_lstButtons.push_back( new string( temp ) );
		sprintf( temp, "text 45 27 %i 0", m_nColor );
		m_lstButtons.push_back( new string( temp ) );
		pagebtny = 290;
	}
	if (m_nStyle==MENUSTYLE_TRASPARENCY) {
		sprintf( temp, "button 250 27 %i %i 1 0 1", 4017, 4017 + 1 );
		m_lstButtons.push_back( new string( temp ) );
		sprintf( temp, "text 45 27 %i 0", m_nColor );
		m_lstButtons.push_back( new string( temp ) );
		pagebtny = 290;
	}




	sprintf( temp, "page %i", pagenum );
	m_lstButtons.push_back( new string( temp ) );

	sprintf( temp, m_strTitle );
	m_lstLabels.push_back( new string( temp ) );

	int oldk = 0;

	for (k = 0; k < m_nNumPages; k++) {


		for( i = 0; i < m_nNumPerPage; i++ )
		{
				if ( k > oldk )
				{
					position = 80;
					pagenum++;
					sprintf(temp, "page %i", pagenum );
					m_lstButtons.push_back( new string( temp ) );
					oldk = k;
				}

				buttonnum = k*m_nNumPerPage+i+10;
				if (m_strLabels[k*m_nNumPerPage+i][0]!='\0') {
					sprintf( temp, "text 80 %i %i %i", position, m_nColor, linenum );
					m_lstButtons.push_back( new string( temp ) );
					sprintf( temp, "button 50 %i %i %i %i 0 %i", position+3, 4005, 4005 + 1, pagenum, buttonnum );
					m_lstButtons.push_back( new string( temp ) );

					sprintf( temp, " %s",m_strLabels[k*m_nNumPerPage+i] );
					m_lstLabels.push_back( new string( temp ) );
				}

				position += 20;
				linenum++;
				realCount++;

		}
	}

	//now add back and forward buttonz
	for( i = 1; i <= m_nNumPages; i++ )
	{
		sprintf( temp, "page %i", i );
		m_lstButtons.push_back( new string( temp ) );
		if( i > 1 )
		{
			sprintf( temp, "button 50 %i %i %i 0 %i", pagebtny,4014, 4014 + 1, i - 1 ); //back button
			m_lstButtons.push_back( new string( temp ) );
		}
		if( i < m_nNumPages )
		{
			sprintf( temp, "button 254 %i %i %i 0 %i", pagebtny, 4005, 4005 + 1, i + 1 );
			m_lstButtons.push_back( new string( temp ) );
		}
	}
}

void cCustomMenu::buildMenu ()
{
	switch (m_nStyle&0x7F)
	{
		case MENUSTYLE_STONE:
		case MENUSTYLE_SCROLL:
		case MENUSTYLE_PAPER:
		case MENUSTYLE_BLACKBOARD:
		case MENUSTYLE_TRASPARENCY:
			buildClassicMenu();
			break;
		case MENUSTYLE_ICONLIST:
			buildIconList();
			break;
		case MENUSTYLE_ICONMENU:
			buildIconMenu();
			break;
		default:
			m_nStyle = MENUSTYLE_STONE;
			buildClassicMenu();
			WarnOut("cCustomMenu::buildMenu() : unsupported menu style was used\n");
			break;
	}
}




void cCustomMenu::buttonSelected (NXWSOCKET  s, unsigned short btn, int seed)
{
	ErrOut("Internal error : pure virtual cCustomMenu::buttonSelected called.\n");
	sysmessage(s, 0x489, "Internal error : pure virtual cCustomMenu::buttonSelected called.");
}

void cCustomMenu::buildIconList ()
{
	char szTemp[70];

	for (int i = 0; i<m_nMaxItemn; i++) {
		if (m_strLabels[i][0]=='<') {
			sprintf(szTemp, "2070 %s", m_strLabels[i]+2);
			limited_strcpy(m_strLabels[i], szTemp);
		}
	}
}

void cCustomMenu::buildIconMenu()
{
}














