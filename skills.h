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
\brief Skill related functions
*/

#ifndef __SKILLS_H__
#define __SKILLS_H__

#include "target.h"

/*!
\brief Skill related stuff
*/
namespace Skills {
	//@{
	/*!
	\name General Skill stuff
	*/
	char AdvanceSkill(CHARACTER s, int sk, char skillused);
	void AdvanceStats(CHARACTER s, int sk);
	char CheckSkillSparrCheck(int c, unsigned short int sk, int low, int high, P_CHAR pcd);
	void SkillUse(NXWSOCKET s, int x);
	void updateSkillLevel(P_CHAR pc, int s);
	//@}

	//@{
	/*!
	\name Tracking stuff
	*/
	void Tracking(NXWSOCKET s, int selection);
	int TrackingDirection(NXWSOCKET s, CHARACTER i);
	void Track(CHARACTER i);
	void CreateTrackingMenu(NXWSOCKET s, int m);
	void TrackingMenu(NXWSOCKET s, int gmindex);
	//@}

	//@{
	/*!
	\name Hiding/Stealth stuff
	*/
	void Hide(NXWSOCKET s);
	void Stealth(NXWSOCKET s);
	//@}

	//@{
	/*!
	\name Musicianship stuff
	*/
	void PeaceMaking(NXWSOCKET s);
	void PlayInstrumentWell(NXWSOCKET s, int i);
	void PlayInstrumentPoor(NXWSOCKET s, int i);
	int GetInstrument(NXWSOCKET s);
	void ProvocationTarget1(NXWSOCKET s);
	void ProvocationTarget2(NXWSOCKET s);
	void EnticementTarget1(NXWSOCKET s);
	void EnticementTarget2(NXWSOCKET s);
	//@}

	void TellScroll(char *menu_name, int player, long item_param);

	void Meditation(NXWSOCKET s);

	//@{
	/*!
	\name Blacksmithing stuff
	*/
	int CalcRank(NXWSOCKET s,int skill); // by Magius(CHE)
	void ApplyRank(NXWSOCKET s,int c,int rank); // by Magius(CHE)
	void Zero_Itemmake(NXWSOCKET s); // by Magius(CHE)
	int GetSubIngotAmt(int p, char id1, char id2, char color1, char color2);
	int DeleSubIngot(int p, int id1, int id2, int color1, int color2, int amount);
	void target_repair( NXWCLIENT ps, P_TARGET t );
	void target_smith( NXWCLIENT ps, P_TARGET t );
	//@}

	//@{
	/*!
	\name ID-Stuff
	*/
	void TasteIDTarget(NXWSOCKET s);
	void ItemIdTarget(NXWSOCKET s);
	//@}

	//@{
	/*!
	\name Alchemy stuff
	*/
	void CreatePotion(CHARACTER s, char type, char sub, int mortar);
	void DoPotion(NXWSOCKET s, SI32 type, SI32 sub, P_ITEM mortar);
	void target_alchemy( NXWCLIENT ps, P_TARGET t );
	void target_bottle( NXWCLIENT ps, P_TARGET t );
	void PotionToBottle(P_CHAR pc, P_ITEM mortar);
	//@}

	//@{
	/*!
	\name Tinkering stuff
	*/
	void target_tinkerAxel( NXWCLIENT ps, P_TARGET t );
	void target_tinkerAwg( NXWCLIENT ps, P_TARGET t );
	void target_tinkerClock( NXWCLIENT ps, P_TARGET t );
	//@}

	//@{
	/*!
	\name Cooking stuff
	*/
	void target_cookOnFire( NXWCLIENT ps, P_TARGET t );
	//@}

	//@{
	/*!
	\name Tailoring stuff
	*/
	void target_tailoring( NXWCLIENT ps, P_TARGET t );
	void target_wheel( NXWCLIENT ps, P_TARGET t );
	void target_loom( NXWCLIENT ps, P_TARGET t );
	//@}

	//@{
	/*!
	\name Bowcrafting stuff
	*/
	void target_fletching( NXWCLIENT ps, P_TARGET t );
	void BowCraft(NXWSOCKET s);
	//@}

	void RemoveTraps(NXWSOCKET s);
	
	void target_carpentry( NXWCLIENT ps, P_TARGET t );

	P_ITEM MakeMenuTarget(NXWSOCKET s, int x, int skill, int amount = INVALID);
	void MakeMenu( P_CHAR pc, int m, int skill, P_ITEM first, P_ITEM second = NULL );

	//@{
	/*!
	\name Mining stuff
	*/
	void target_mine( NXWCLIENT ps, P_TARGET t );
	void GraveDig(NXWSOCKET s);
	//@}

	void target_smeltOre( NXWCLIENT ps, P_TARGET t );
	void TreeTarget(NXWSOCKET s);

	void DetectHidden(NXWSOCKET s);

	//@{
	/*!
	\name Healing stuff
	*/
	void target_healingSkill( NXWCLIENT ps, P_TARGET t );
	//@}

	void SpiritSpeak(NXWSOCKET s);
	
	void ArmsLoreTarget(NXWSOCKET s);
	
	void Evaluate_int_Target(NXWSOCKET s);
	
	void AnatomyTarget(NXWSOCKET s);
	
	void TameTarget(NXWSOCKET s);

	//@{
	/*!
	\name Thievery stuff
	*/
	void RandomSteal(NXWCLIENT ps);
	void StealingTarget(NXWCLIENT ps);
	void PickPocketTarget(NXWCLIENT ps);
	void target_lockpick( NXWCLIENT ps, P_TARGET t );
	//@}

	void BeggingTarget(NXWSOCKET s);
	
	void AnimalLoreTarget(NXWSOCKET s);
	
	void ForensicsTarget(NXWSOCKET s);
	
	void PoisoningTarget(NXWCLIENT ps);

	void Inscribe(NXWSOCKET s);
	
	int EngraveAction(NXWSOCKET s, int i, int cir, int spl);
	
	void TDummy(NXWSOCKET s);
	
	void target_tinkering( NXWCLIENT ps, P_TARGET t );
	
	void AButte(NXWSOCKET s1, P_ITEM pButte);

	void Persecute(NXWSOCKET s); //!< AntiChrist persecute stuff

	//@{
	/*!
	\name Cartography Stuff
	*/
	void Cartography(NXWSOCKET s); //!< By Polygon - opens the cartography skillmenu
	bool HasEmptyMap(CHARACTER cc); //!< By Polygon - checks if player has an empty map
	bool DelEmptyMap(CHARACTER cc); //!< By Polygon - deletes an empty map from the player's pack
	void Decipher(P_ITEM tmap, NXWSOCKET s); //!< By Polygon - attempt to decipher a tattered treasure map
	//@}

	int GetAntiMagicalArmorDefence(CHARACTER p); //!< blackwind meditation armor stuff
};
void snooping( P_CHAR snooper, P_ITEM cont );


void SkillVars();
void loadskills();
void TellScroll( char *menu_name, int player, long item_param );

#endif
