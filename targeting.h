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
\brief Header for targeting handling class
*/

#ifndef _TARGETING_INCLUDED
#define _TARGETING_INCLUDED

#include "typedefs.h"

/*!
\brief Struct for package 0x6c
\note the model # and charID should NEVER be trusted.
*/
typedef struct _PKGx6C
{
//Clicking Commands (19 bytes)
//* BYTE cmd
/*!
\brief Cursor Type

<b>0x00</b>: Select Object<br>
<b>0x01</b>: Choose x, y, z
*/
	UI08 type;
//* BYTE[4] charID
	UI08 Tnum;		//!< we only use the low byte anyway, so short
//* BYTE[4] Clicked On ID
	SI32 Tserial; //!< always sent but are only valid if sent by client
	SI16 TxLoc; //!< click xLoc
	SI16 TyLoc; //!< click yLoc
//* BYTE unknown2 (0x00)
	SI08 TzLoc; //!< click zLoc
//* BYTE[2] model # (if a static tile, 0 if a map/landscape tile)
	SI16 model; //!< model # if a static tile, 0 if a map/landscape tile
} PKGx6C;

/*!
\brief class for handle targeting functions
*/
class cTargets
{
private:
	void CharTarget(NXWCLIENT ps, PKGx6C *pt);
	void HouseSecureDown( NXWSOCKET  s ); // Ripper
	void HouseLockdown( NXWSOCKET  s ); // Abaddon
	void HouseRelease( NXWSOCKET  s ); // Abaddon
	void Priv3XTarget(NXWSOCKET s);
	//void ShowPriv3Target(NXWSOCKET s); // Whose been screwing with this? cj 8/11/99
	void PlVBuy(NXWSOCKET s);
	//void RenameTarget(NXWSOCKET s);
	void RemoveTarget(NXWSOCKET s);
	void NewzTarget(NXWSOCKET s);
	void TypeTarget(NXWSOCKET s);
	void IstatsTarget(NXWSOCKET s);
	void GhostTarget(NXWSOCKET s);
	void AmountTarget(NXWSOCKET s);
	void CloseTarget(NXWSOCKET s);
	void VisibleTarget(NXWSOCKET s);
	void DvatTarget(NXWSOCKET s);
	void AllSetTarget(NXWSOCKET s);
	//void TweakTarget(NXWSOCKET s);
	void LoadCannon(NXWSOCKET s);
//	void SetInvulFlag(NXWSOCKET s);
	void SquelchTarg(NXWSOCKET s);
	void SwordTarget(NXWCLIENT pC);
	void NpcTarget(NXWSOCKET s);
	void NpcTarget2(NXWSOCKET s);
	void NpcRectTarget(NXWSOCKET s);
	void NpcCircleTarget(NXWSOCKET s);
	void NpcWanderTarget(NXWSOCKET s);
	void NpcAITarget(NXWSOCKET s);
	void xBankTarget(NXWSOCKET s);
	void xSpecialBankTarget(NXWSOCKET s);//AntiChrist
	void DupeTarget(NXWSOCKET s);
	void MoveToBagTarget(NXWSOCKET s);
	void SellStuffTarget(NXWSOCKET s);
	void GmOpenTarget(NXWSOCKET s);
	void StaminaTarget(NXWSOCKET s);
	void ManaTarget(NXWSOCKET s);
	void MakeShopTarget(NXWSOCKET s);
	void AttackTarget(NXWSOCKET s);
	void FollowTarget(NXWSOCKET s);
	void TransferTarget(NXWSOCKET s);
	void BuyShopTarget(NXWSOCKET s);
	void permHideTarget(NXWSOCKET s);
	void unHideTarget(NXWSOCKET s);
	void SetSpeechTarget(NXWSOCKET s);
	void SetPoisonTarget(NXWSOCKET s);
	void SetPoisonedTarget(NXWSOCKET s);
	void FullStatsTarget(NXWSOCKET s);
	void SetAdvObjTarget(NXWSOCKET s);
	void CanTrainTarget(NXWSOCKET s);
	void SetSplitTarget(NXWSOCKET s);
	void SetSplitChanceTarget(NXWSOCKET s);
	void SetSpaDelayTarget(NXWSOCKET s);
	void NewXTarget(NXWSOCKET s);
	void NewYTarget(NXWSOCKET s);
	void IncXTarget(NXWSOCKET s);
	void IncYTarget(NXWSOCKET s);
	//void IncZTarget(NXWSOCKET s);
	void BoltTarget(NXWSOCKET s);
	void SetDirTarget(NXWSOCKET s);
	void HouseOwnerTarget(NXWSOCKET s);
	void HouseEjectTarget(NXWSOCKET s);
	void HouseBanTarget(NXWSOCKET s);
	void HouseFriendTarget(NXWSOCKET s);
	void HouseUnlistTarget(NXWSOCKET s);
	void BanTarg(NXWSOCKET s);
	void triggertarget(NXWSOCKET ts); // By Magius(CHE)
	//void GlowTarget(NXWSOCKET s);
	//void UnglowTaget(NXWSOCKET s);
	void TargetsMenuPrivTarg(NXWSOCKET s);
	void ResurrectionTarget( NXWSOCKET  s );
	void MenuPrivTarg(NXWSOCKET s);
	void ShowSkillTarget(NXWSOCKET s);
	//taken from 6904t2(5/10/99) - AntiChrist
	void GuardTarget( NXWSOCKET  s );
	void FetchTarget( NXWSOCKET  s );
	void SetMurderCount( NXWSOCKET s );	// Abaddon 12 Sept 1999
	void ShowAccountCommentTarget(NXWSOCKET s);//AntiChrist
	void SetHome(NXWSOCKET s);
	void SetWork(NXWSOCKET s);
	void SetFood(NXWSOCKET s);
	void AllAttackTarget(NXWSOCKET s);
public:
	void IDtarget(NXWSOCKET s);
	void MultiTarget(NXWCLIENT ps);
	//void Wiping(NXWSOCKET s);
	int NpcMenuTarget(NXWSOCKET s);
	void JailTarget(NXWSOCKET s, int c);
	void ReleaseTarget(NXWSOCKET s, int c);
	int AddMenuTarget(NXWSOCKET s, int x, int addmitem);
	void XTeleport(NXWSOCKET s, int x);
	int BuyShop(NXWSOCKET s, CHARACTER c);
};


class TargetLocation
{
private:
	P_CHAR m_pc;
	int    m_pcSerial;
	P_ITEM m_pi;
	int    m_piSerial;
	int    m_x;
	int    m_y;
	int    m_z;
	void   init(int x, int y, int z);
	void   init(P_CHAR pc);
	void   init(P_ITEM pi);

public:
	//!creates a target loc from a character
	TargetLocation(P_CHAR pc) { init(pc); }
	//!creates a target loc from an item
	TargetLocation(P_ITEM pi) { init(pi); }
	//!creates a target loc from a target net packet
	TargetLocation(PKGx6C* pp);
	//!creates a target loc from a xyz position in the map
	TargetLocation(int x, int y, int z) { init(x,y,z); }

	//!recalculates item&char from their serial
	void revalidate();

	//!gets the targetted char if any, NULL otherwise
	inline P_CHAR getChar() { return m_pc; }
	//!gets the targetted item if any, NULL otherwise
	inline P_ITEM getItem() { return m_pi; }
	//!gets the XYZ location of this target location
	inline void getXYZ(int& x, int& y, int& z) { x = m_x; y = m_y; z = m_z; }
	//!extends a P_ITEM data to P_CHAR and x,y,z
	void extendItemTarget();
};

/*!
\brief new wrapper for packet 0x6c
\author Sparhawk
*/
class cPacketTargeting
{
	public:
			cPacketTargeting();
			~cPacketTargeting();

		UI08	getPacketType( NXWSOCKET socket );
		UI08	getTargetType( NXWSOCKET socket );
		SERIAL	getCharacterSerial( NXWSOCKET socket );
		UI08	getCursorType( NXWSOCKET socket );
		SERIAL	getItemSerial( NXWSOCKET socket );
		SI16	getX( NXWSOCKET socket );
		SI16	getY( NXWSOCKET socket );
		SI08	getZ( NXWSOCKET socket );
		SI16	getModel( NXWSOCKET socket );
		SI08	getUnknown( NXWSOCKET socket );
};


#endif	// _TARGETING_INCLUDED
