  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "bounty.h"
#include "debug.h"
#include "items.h"
#include "chars.h"

/*
//////////////////////////////////////////////////////////////////////////////
// FUNCTION:    BountyAskVictim( int nVictimSerial, int nMurdererSerial )
//
// PURPOSE:     Used to ask the murder victim whether they would like to place
//              a bounty on the head of the murderer.
//
// PARAMETERS:  nVictimSerial   Victim characters serial number
//              nMurdererSerial Murderer characters serial number
//
// RETURNS:     void
//////////////////////////////////////////////////////////////////////////////
void BountyAskVictim( int nVictimSerial, int nMurdererSerial )
{
  int nAmount     = 0;
  int nVictimIdx  = calcCharFromSer( nVictimSerial );
  int nMurderIdx  = calcCharFromSer( nMurdererSerial );
  int err2;
  char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

  P_CHAR pcc_nVictimIdx=MAKE_CHARREF_LOGGED(nVictimIdx,err);
  P_CHAR pcc_nMurderIdx=MAKE_CHARREF_LOGGED(nMurderIdx,err2);

  if (err || err2) return;

  // Indicate that the victim is being asked if they want to place a bounty
  sprintf((char*)temp, "BountyAskVictim():  %s is attempting to place a bounty of %i on %s\n",
          pcc_nVictimIdx->getCurrentNameC(), nAmount, pcc_nMurderIdx->getCurrentNameC() );
  LogMessage((char*)temp);

  // If the amount of the bounty is larger than zero, create a bounty posting
  if( nAmount > 0 )
  {
     if( BountyCreate( pcc_nMurderIdx, nAmount ) )
	 {
         sprintf((char*)temp, "BountyAskVictim():  %s has placed a bounty of %i on %s\n",
                pcc_nVictimIdx->getCurrentNameC(),
                nAmount,
                pcc_nMurderIdx->getCurrentNameC() );
		 LogMessage((char*)temp);
	 }

     else
	 {
       sprintf((char*)temp, "BountyAskVictim():  %s FAILED to place a bounty of %i on %s\n",
               pcc_nVictimIdx->getCurrentNameC(),
               nAmount,
               pcc_nMurderIdx->getCurrentNameC() );
	   LogMessage((char*)temp);
	 }
  }

  return;
} // BountyAskVictim()
*/

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:    BountyCreate( int nMurdererSerial, int nRewardAmount )
//
// PURPOSE:     Used to create the bounty posting message on the global
//              bulletin board for all to see.
//
// PARAMETERS:  nMurdererSerial Murderer characters serial number
//              nRewardAmount   Bounty placed on the murderers head
//
// RETURNS:     TRUE  Bounty post successfully created
//              FALSE Bounty post could not be created
//////////////////////////////////////////////////////////////////////////////
bool BountyCreate( P_CHAR pc, int nRewardAmount )
{
	VALIDATEPCR(pc,false);

  int   nPostSerial = 0;

  // Check that we have a reward amount greater than zero
  if( nRewardAmount > 0 )
  {
    // Check that this murderer doesn't already have a bounty on them
    if( pc->questBountyReward > 0 )
    {
      // This murderer already has a bounty on them because they
      // have a reward amount on their head, so delete old bounty
      // and add the new (updated) one
      nRewardAmount += pc->questBountyReward;
      BountyDelete( pc );
    }

    // Attempt to post the message first
    pc->questBountyReward = nRewardAmount;
    nPostSerial = MsgBoards::MsgBoardPostQuest( pc->getSerial32(), MsgBoards::BOUNTYQUEST );

    // If we received a valid serial number then the post was successfull
    if( nPostSerial > 0 )
    {
      pc->questBountyPostSerial = nPostSerial;
      return true;
    }
  }

  // Failed to post bounty
  LogWarning("BountyCreate():  FAILED to place a bounty of %i on %s (PostSerial=%x)\n",
          nRewardAmount,
          pc->getCurrentNameC(),
          nPostSerial );

  // Post must have failed
  return false;
} // BountyCreate()


bool BountyDelete( P_CHAR pc)
{
	VALIDATEPCR(pc,false);
	bool  bReturn = true;
	
	// Find and mark the post associated with this bounty as deleted
	// so that the bulletin board maintenance routine can clean it up
	bReturn = MsgBoards::MsgBoardRemoveGlobalPostBySerial( pc->questBountyPostSerial );
	
	// Reset all bounty values for this character
	pc->questBountyReward     = 0;
	pc->questBountyPostSerial = 0;
	
	return bReturn;
} // BountyDelete()


bool BountyWithdrawGold( P_CHAR pVictim, int nAmount )
{
	int has = pVictim->countBankGold();
	if (has < nAmount)
		return false;

	P_ITEM pBox = pVictim->GetBankBox();
	if (!pBox)
		return false;	// shouldn't happen coz it's needed in CountBankGold...

	pBox->DeleteAmount(nAmount,0x0EED);
	return true;
}

