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
\brief Temp Effect Related Stuff
*/

#ifndef __TMPEFF_H__
#define __TMPEFF_H__

namespace tempfx {
	
enum {
	MODE_START = 0,
	MODE_END,
	MODE_ON,
	MODE_OFF,
	MODE_REVERSE
};

bool	add(P_OBJECT src, P_OBJECT dest, int num, unsigned char more1 = 0, unsigned char more2 = 0, unsigned char more3 = 0,short dur=INVALID, int amxcback = INVALID);
LOGICAL	isSrcRepeatable(int num);
LOGICAL	isDestRepeatable(int num);
void	addTempfxCheck( SERIAL serial );

	
/*
\brief A Temp effect
\author Luxor
*/

class cTempfx {
private:
	SERIAL m_nSrc;
	SERIAL m_nDest;
	SI32 m_nNum;
	SI32 m_nMode;
	UI32 m_nExpireTime;
	SI32 m_nAmxcback;
	SI32 m_nMore1;
	SI32 m_nMore2;
	SI32 m_nMore3;
	LOGICAL m_bDispellable;
	LOGICAL m_bSrcRepeatable;
	LOGICAL m_bDestRepeatable;
public:
	cTempfx( SERIAL nSrc, SERIAL nDest, SI32 num, SI32 dur, SI32 more1, SI32 more2, SI32 more3, SI32 amxcback );
	int getExpireTime() { return m_nExpireTime; }
	bool isDispellable() { return m_bDispellable; }
	void activate();
	void deactivate();
	bool isValid();
	SI08 checkForExpire();
	void executeExpireCode();
	void start();
	int getNum() { return m_nNum; }
	SERIAL getAmxCallback() { return m_nAmxcback; }
};

void tempeffectson();
void tempeffectsoff();
void checktempeffects();

	//End Luxor's temp effects system
}

#endif //__TMPEFF_H__
