  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "sndpkg.h"
#include "debug.h"
#include "layer.h"
#include "weight.h"
#include "set.h"
#include "chars.h"
#include "items.h"


/*!
\brief Search player's paperdoll and then backpacks for items with weight then set the weight of the current player
\author Ison (02/20/99), rewrote by Tauriel (03/20/99)
\param pc the character
\note  The called character's weight is first set to zero then re-calculated during the function.
*/
void weights::NewCalc(P_CHAR pc)
{
	VALIDATEPC(pc);
	double totalweight=0.0;
	//get weight for items on players
	
	NxwItemWrapper si;
	si.fillItemWeared( pc, false, false, true );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		// Wintermute: Exclude mounted layer (counts as worn item)
		if (ISVALIDPI(pi) && pi->layer != LAYER_MOUNT)
		{
			totalweight+=(pi->getWeightActual()/100.0);
		}
	}

	// Items in players pack
	P_ITEM bp= pc->getBackpack();
	if (bp!=NULL) totalweight+=RecursePacks(bp); //LB

	pc->weight=(int)totalweight;

	return;

}

/*!
\brief recurses through the container given to calculate the total weight
\author Ison (02/20/99), rewritten by Tauriel (03/20/99), rewritten by Duke (04/11/00)
\return the weight
\param bp the item
*/
float weights::RecursePacks(P_ITEM bp)
{
	double totalweight=0.0;

	if (!ISVALIDPI(bp)) return 0.0;
	
	NxwItemWrapper si;
	si.fillItemsInContainer( bp, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		if( !ISVALIDPI(pi)) continue;

		R32 itemsweight=pi->getWeightActual();
		if (pi->isContainer()) //item is another container
		{
			totalweight+=(itemsweight/100.0);		// the weight of container itself
			totalweight+=RecursePacks(pi); //find the item's weight within this container
		}
		
		totalweight+=((itemsweight)/100.0);
	}
	return static_cast<float>(totalweight);
}

/*!
\brief Check when player is walking if overloaded
\author Unknow
\return 1 if k, 0 if overloaded
\param pc the character
*/
int weights::CheckWeight(P_CHAR pc)
{
	const static char steps[4]	= {25,50,75,100};//,
					  //steps2[5]	= {0,25,50,75}; //per cent steps

	int limit	= pc->getStrength()*WEIGHT_PER_STR+30, //weight limit
		percent	= int(((float)pc->weight/(float)limit)*100.0f), //current weight per cent
		ret=1,
		result; //for float rounding purposes

	UI08 index,x;
	float amount;

	if(pc->weight>limit) index=5; //overweight
	else if(percent==100) index=4; //100% weight
	else for(x=0;x<4;x++) if(percent<steps[x]) { index=x; break; } //less than 100%

/*	if(index<4)
	{
		float stepdiff		= float(steps[index]-steps2[index]),
			  percentdiff	= float(percent-steps2[index]),
			  stadiff		= ServerScp::g_fStaminaUsage[index+1]-ServerScp::g_fStaminaUsage[index];

		amount=ServerScp::g_fStaminaUsage[index]+(percentdiff/stepdiff)*stadiff;
	}
	else 
*/		amount=ServerScp::g_fStaminaUsage[index];

	if(pc->running) amount*=2; //if running, double the amount of stamina used
	if(pc->isMounting()) amount*=(float)server_data.staminaonhorse;

	pc->fstm+=amount; //increase the amount of stamina to be subtracted

	if(pc->fstm>=1.0f) //if stamina to be removed is less than 1, wait
	{
		result=(int)pc->fstm; //round it
		pc->fstm-=result;
		pc->stm-=result;
		if(pc->stm<=0)
		{
			pc->stm=0;
			ret=0;
		}
		pc->updateStats(2);
	}
	
	return ret;

}


/*!
\brief Check when player is teleporting if overloaded
\author Morrolan
\return ??
\param pc the character
*/
int weights::CheckWeight2(P_CHAR pc)
{
	if ((pc->weight > (pc->getStrength()*WEIGHT_PER_STR)+30))
	{
		pc->mn -= 30;
		if (pc->mn<=0)
		{
			pc->mn=0;
		}
		return 1;
	}
	return 0;
}

/*!
\brief Get the player weight
\author Unknow, update by Duke (04/11/00)
\return actual weight
\param pItem the container
\param total the total number of item in container and subcontainer
\note total < 0 indicate that not a pack ! on osi servers in that case weigt/items count isnt show
 thus i set it negative, if you want to show it anyway, add something like if (weight<0) weight*=-1; 
*/
float weights::LockeddownWeight(P_ITEM pItem, int *total )
{
	double totalweight=0.0;
	if (!ISVALIDPI(pItem)) 
	{
		*total=0;
		return 0.0;
	}
	
	NxwItemWrapper si;
	si.fillItemsInContainer( pItem, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi= si.getItem();
		if(!ISVALIDPI(pi)) continue;

		R32 itemsweight=pi->getWeightActual();
		*total=*total+1;
		if (pi->isContainer()) //item is another container
		{
			totalweight+=(itemsweight/100.0); //(pi->weight/100);
			totalweight+=LockeddownWeight(pi, total); //find the item's weight within this container
		}
		
		totalweight+=((itemsweight)/100.00); //((pi->weight*pi->amount)/100);  // Ison 2-21-99
	}

	if (*total==0) 
	{ 
		*total=pItem->amount;
		*total=*total*-1; 
		return static_cast<float>(pItem->getWeightActual()/100.0); // if no pack return single item weight*/			
		
	}
	else
		return static_cast<float>(totalweight);
}

