  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "network.h"
#include "sndpkg.h"
#include "layer.h"
#include "npcai.h"
#include "scp_parser.h"
#include "set.h"
#include "items.h"
#include "chars.h"
#include "inlines.h"
#include "utils.h"
#include "range.h"
#include "scripts.h"

std::map< SI32, SI32 > mountinfo;

void loadmounts()
{

	cScpIterator* iter = NULL;
	std::string script1;
	std::string script2;
	SI32 id=INVALID;
	SI32 anim=INVALID;
	int mount=0;

	do
	{
		safedelete(iter);
		iter = Scripts::Mountable->getNewIterator("SECTION MOUNT %i", mount++);
		if( iter == 0 ) continue;

		id=INVALID;
		anim=INVALID;

		do
		{
			iter->parseLine(script1, script2);
			if	( script1 == "ANIM" )	anim = str2num(script2);
			else if ( script1 == "ID" )
			{
				id = str2num(script2);
				//ConOut("Mount %d", id );
			}
		}
		while ( script1[0] != '}' );

		if((id!=INVALID) && (anim!=INVALID))
			mountinfo[anim]=id;
	}
	while ( iter != 0 );

	safedelete(iter);

}


/*!
\brief Mount a creature
\author Unknow, restyle by Endymion
\param mount the creature
\note remove creature and give player a creature item
*/
void cChar::mounthorse( P_CHAR mount ) 
{

	NXWCLIENT ps = getClient();
	if( ps==NULL ) return;
	VALIDATEPC( mount );
	
	if ( !char_inRange(this, mount, 2) && !IsGM())
		return;

	
	if( this->amxevents[EVENT_CHR_ONMOUNT])
	{
		g_bByPass=false;
		this->amxevents[EVENT_CHR_ONMOUNT]->Call(this->getSerial32(),mount->getSerial32());
		if(g_bByPass) return;
	}
	/*
	runAmxEvent( EVENT_CHR_ONMOUNT, getSerial32(), mount->getSerial32() );
	if(g_bByPass) 
		return;
	*/

	//Unavowed
	if ( (mount->getOwnerSerial32() == getSerial32()) || IsGM())
	{
		if (onhorse)
		{
			sysmsg( TRANSLATE("You are already on a mount."));
			return;
		}
		
		std::map<SI32,SI32>::iterator iter = mountinfo.find(mount->getId());
		if( iter==mountinfo.end() ) { //not mountable creature
			sysmsg( "This is not a mountable creature" );
			return;
		}

		onhorse = true;

		P_ITEM pi = item::CreateFromScript( "$item_mount", this );
		VALIDATEPI(pi);

		pi->setId( iter->second );
		pi->setColor( mount->getColor() );
		pi->setCurrentName( mount->getCurrentName() );

		/*pi->id1 = 0x3E;

		switch ( mount->id2)
		{
			case 0xC8:
				pi->id2 = (unsigned char)'\x9F';
				break; // Horse or Mare (based on color currently)
			case 0xE2:
				pi->id2 = (unsigned char)'\xA0';
				break; // Horse
			case 0xE4:
				pi->id2 = (unsigned char)'\xA1';
				break; // Horse
			case 0xCC:
				pi->id2 = (unsigned char)'\xA2';
				break; // Horse
			case 0xD2:
				pi->id2 = (unsigned char)'\xA3';
				break; // Desert Ostard
			case 0xDA:
				pi->id2 = (unsigned char)'\xA4';
				break; // Frenzied Ostard
			case 0xDB:
				pi->id2 = (unsigned char)'\xA5';
				break; // Forest Ostard
			case 0xDC:
				pi->id2 = (unsigned char)'\xA6';
				break; // llama
// New mountable creatures
			case 0x7A:
				pi->id2 = (unsigned char)'\xB4'; // unicorn
				break;

			case 0xBB:
				pi->id2 = (unsigned char)'\xB8';
				break; // Ridgeback
			case 0x17:
				pi->id2 = (unsigned char)'\xBC'; // Giant Beetle
				break;
			case 0x19:
				pi->id2 = (unsigned char)'\xBB'; // Skeletal Mount
				break;
			case 0x1a:
				pi->id2 = (unsigned char)'\xBD'; // swamp dragon
				break;
			case 0x1f:
				pi->id2 = (unsigned char)'\xBE'; // armor dragon
				break;
			default :
				break; // llama

		}

		*//* XAN -- known horse valuez :
		9F -> A2 : horses
		A3->A5 : ostards
		A6 : llama
		A7 : nightmare
		A8 : dream
		A9 : nightmare again
		AA : ethereal horse
		AB : ethereal llama
		AC : ethereal ostard
		AD : bronze horse
		AE : invisible mount
		AF : dark bronze horse
		B0 : nightmare again 2
		B1 : light nightmare
		B2 : violet nightmare
		B3 : dark dream
		B4 : white horse
		B5 : darker nightmare
		B6 : dark nightmare
		B7 : black nightmare
		B8 : golden ostard

		***/

		
		pi->setContSerial(getSerial32());
		pi->layer = LAYER_MOUNT;

		// v-- is not cheched for decay, so useless
		if (mount->summontimer != 0) {
			pi->setDecayTime( mount->summontimer );
		} else { //Luxor bug fix
			pi->setDecay(false);
		}

			
		NxwSocketWrapper sw;
		sw.fillOnline( this, false );
		for( sw.rewind(); !sw.isEmpty(); sw++ )
		{
			NXWSOCKET si=sw.getSocket();
			if( si!=INVALID )
				wornitems(si, this );
		}

		// if this is a gm lets tame the animal in the process
		if (IsGM())
		{
  			mount->setOwnerSerial32( getSerial32() );
			mount->tamed = true;
			mount->npcaitype = NPCAI_GOOD;
		}


		mount->mounted=true;
#ifdef SPAR_C_LOCATION_MAP
		pointers::delFromLocationMap( mount );
#else
		mapRegions->remove( mount );
#endif
		pointers::pMounted.insert( make_pair( getSerial32(), mount ) );

		sw.clear();
		sw.fillOnline( this, false );

		SERIAL mount_serial = mount->getSerial32();

		for( sw.rewind(); !sw.isEmpty(); sw++ )		
		{
			NXWSOCKET si=sw.getSocket();
			if(si!=INVALID)
			{
				SendDeleteObjectPkt(si, mount_serial);
			}
		}

		sysmsg( "Now you are riding %s", mount->getCurrentNameC());

		mount->war = 0;
		mount->attackerserial=INVALID;

		mount->time_unused = 0;
		mount->timeused_last = uiCurrentTime;
	}
	else
		sysmsg(TRANSLATE("You dont own that creature."));
}

/*!
\brief make char unmount horse (Remove horse item and spawn new horse)
\return 1 on error, 0 else
\remark only online players can unmount
*/
int cChar::unmountHorse()
{
	NXWCLIENT ps = getClient();
	if(ps==NULL) 
		return 1;

	
	if(this->amxevents[EVENT_CHR_ONDISMOUNT]) // Unavowed
	{
		g_bByPass=false;
		this->amxevents[EVENT_CHR_ONDISMOUNT]->Call(this->getSerial32(),INVALID);
		if(g_bByPass) return 1;
	}
	/*
	runAmxEvent( EVENT_CHR_ONDISMOUNT, getSerial32() );
	if(g_bByPass)
		return 1;
	*/

	NxwItemWrapper si;
	si.fillItemWeared( this, true, true, false );
	for( si.rewind(); !si.isEmpty(); si++ )
	{
		P_ITEM pi=si.getItem();
		if( ISVALIDPI(pi) && pi->layer == LAYER_MOUNT)
		{

			onhorse = false;


			std::map< SERIAL, P_CHAR >::iterator iter( pointers::pMounted.find( getSerial32() ) );

			if( ( iter!=pointers::pMounted.end() ) ) {

				P_CHAR p_pet=iter->second;
				pointers::pMounted.erase( iter );

				if( ISVALIDPC( p_pet ) ) {

				
					p_pet->timeused_last = getclock();
					p_pet->time_unused = 0;
				
					p_pet->mounted=false;

					p_pet->MoveTo( getPosition() );
					p_pet->dir = dir; // Luxor
					p_pet->teleport();

					pi->Delete();
					teleport( TELEFLAG_SENDWORNITEMS );
					return 0;
				}
			}

			pi->Delete();
			teleport( TELEFLAG_SENDWORNITEMS );
			InfoOut("Horse not found");
			return 1;

		}
	}
	return INVALID;
}

/*!
\brief Get horse current mounted
\author Endymion
\return the horse serial or INVALID
*/
SERIAL cChar::getHorse()
{
	std::map<SERIAL, P_CHAR >::iterator iter( pointers::pMounted.find( this->getSerial32() ) );
	if( iter!=pointers::pMounted.end() ) {
		if( !ISVALIDPC( iter->second ) ) { //remove it
			pointers::pMounted.erase( iter );
			return INVALID;
		}
		else 
			return (iter->second)->getSerial32();
	}
	else 
		return INVALID;
}

/*!
\brief Check if is on horse
\author Endymion
\return true if mounting
*/
bool cChar::isMounting( )
{
	return onhorse;
}

/*!
\brief Check if is on given horse
\author Endymion
\return true if mounting
\param horse horse
*/
bool cChar::isMounting( P_CHAR horse )
{
	VALIDATEPCR( horse, false );
	return horse->getSerial32()==getHorse();
}

/*!
\brief Set on horse
\author Endymion
\warning use cChar::mountHorse() for mount an npc, this is used internal
*/
void cChar::setOnHorse()
{
	onhorse=true;
}

