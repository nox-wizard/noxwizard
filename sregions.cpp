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
\brief Functions that handle the spawn regions
*/
#include "nxwcommn.h"
#include "network.h"
#include "debug.h"
#include "sndpkg.h"
#include "amx/amxcback.h"
#include "calendar.h"
#include "sregions.h"

void loadregions()//New -- Zippy spawn regions
{
	int i, noregion, l=0, a=0,loopexit=0;
	char sect[512];
	int actgood=INVALID; // Magius(CHE)
	cScpIterator* iter = NULL;
	char script1[1024];
	char script2[1024];

	for (i=0;i<256;i++)
	{
		region_st &regionRef = region[i];
		
		regionRef.inUse = false;
		regionRef.midilist=0;
		regionRef.priv=0;
		regionRef.drychance=0;
		regionRef.keepchance=0;
		regionRef.wtype = 0;
		regionRef.snowchance=0;
		regionRef.rainchance=0;
		regionRef.forcedseason = INVALID;
		regionRef.ignoreseason = false;
		regionRef.name[0]=0;
		noregion=0;
		for (a=0;a<10;a++)
		{
			regionRef.guardnum[a]=RandomNum(1000,1001);
		}
		for (a=0;a<100;a++)		// added by Magius(CHE)
		{
			regionRef.goodsell[a]=0;
			regionRef.goodbuy[a]=0;
			regionRef.goodrnd1[a]=0;
			regionRef.goodrnd2[a]=0;
		}
		a=0;		// end added by Magius(CHE)

		sprintf(sect, "SECTION REGION %i", i);
		safedelete(iter); //as the name implies, this is safe :P, Xan
		iter = Scripts::Regions->getNewIterator(sect);

		if (iter==NULL) {
			noregion=1;
			continue; //-> goes next loop!
		}
		regionRef.inUse = true;
		
		loopexit=0;
		do
		{
			iter->parseLine(script1, script2);
			if ((script1[0]!='}')&&(script1[0]!='{'))
			{
				if (!(strcmp("GUARDNUM",script1)))
				{
					if (a<10)
					{
						regionRef.guardnum[a]=str2num(script2);
						a++;
					}
					else
					{
						WarnOut("region %i has more than 10 'GUARDNUM', The ones after 10 will not be used\n",i);
					}
				}
				if (!(strcmp("NAME",script1)))
				{
					strcpy(regionRef.name,script2);
					actgood=INVALID; // Magius(CHE)
				}
				// Dupois - Added April 5, 1999
				// To identify whether this region is escortable or not.
				else if (!(strcmp("ESCORTS",script1)))
				{
					// Load the region number in the global array of valid escortable regions
					if ( str2num(script2) == 1 )
					{
						// Store the region index into the valid escort region array
						validEscortRegion[escortRegions] = i;
						escortRegions++;
						//ConOut( "NoX-Wizard: loadregions() %i regions loaded so far\n", escortRegions );
					}
				} // End - Dupois
				if (!(strcmp("GUARDOWNER",script1)))	strcpy(regionRef.guardowner,script2);
				if (!(strcmp("MIDILIST",script1)))		regionRef.midilist=str2num(script2);
				if (!(strcmp("GUARDED",script1)))
				{
					if (str2num(script2)) regionRef.priv|=0x01;
				}
				if (!(strcmp("MAGICDAMAGE",script1)))
				{
					if ((str2num(script2))) regionRef.priv|=0x40; // bugfix LB 12-march-
					// changes from 0=magicdamge,1=no magic damage
					// to			1=			 0=
				}
				if (!(strcmp("NOMAGIC",script1)))
				{
					if ((str2num(script2))) regionRef.priv|=0x80;
				}
				if (!(strcmp("MARK",script1)))
				{
					if (str2num(script2)) regionRef.priv|=0x02;
				}
				if (!(strcmp("GATE",script1)))
				{
					if (str2num(script2)) regionRef.priv|=0x04;
				}
				if (!(strcmp("RECALL",script1)))
				{
					if (str2num(script2)) regionRef.priv|=0x08;
				}
				if (!(strcmp("SNOWCHANCE", script1)))
				{
					regionRef.snowchance=str2num(script2);
				}
				if (!(strcmp("RAINCHANCE", script1)))
				{
					regionRef.rainchance=str2num(script2);
				}
				//xan : quick&dirty weather system :)
				if (!(strcmp("DRYCHANCE", script1)))
				{
					regionRef.drychance=str2num(script2);
				}
				if (!(strcmp("KEEPCHANCE", script1)))
				{
					regionRef.keepchance=str2num(script2);
				}
				if (!(strcmp("FORCESEASON", script1)))
				{
					regionRef.forcedseason =str2num(script2);
				}

				if (!(strcmp("IGNOREMONTHMULTIPLIERS", script1)))
				{
					regionRef.ignoreseason =true;
				}

				if (!(strcmp("GOOD", script1))) // Magius(CHE)
				{
					actgood=str2num(script2);
				}
				if (!(strcmp("BUYABLE", script1))) // Magius(CHE)
				{
					if (actgood>INVALID) regionRef.goodbuy[actgood]=str2num(script2);
					else ErrOut("error in regions.xss. You must write BUYABLE after GOOD <num>!\n");
				}
				if (!(strcmp("SELLABLE", script1))) // Magius(CHE)
				{
					if (actgood>INVALID) regionRef.goodsell[actgood]=str2num(script2);
					else ErrOut("error in regions.xss. You must write SELLABLE after GOOD <num>!\n");
				}
				if (!(strcmp("RANDOMVALUE", script1))) // Magius(CHE) (2)
				{
					if (actgood>INVALID) {
						gettokennum(script2, 0);
						regionRef.goodrnd1[actgood]=str2num(gettokenstr);
						gettokennum(script2, 1);
						regionRef.goodrnd2[actgood]=str2num(gettokenstr);
						if (regionRef.goodrnd2[actgood]<regionRef.goodrnd1[actgood])
						{
							ErrOut("error in regions.xss. You must write RANDOMVALUE NUM2[%i] grater than NUM1[%i].\n",regionRef.goodrnd2[actgood],regionRef.goodrnd1[actgood]);
							regionRef.goodrnd2[actgood]=regionRef.goodrnd1[actgood]=0;
						}
					}
					else ErrOut("error in regions.xss. You must write RANDOMVALUE after GOOD <num>!\n");
				}
				if (!(strcmp("X1", script1)))
				{
					location[l].x1=str2num(script2);
				}
				if (!(strcmp("X2", script1)))
				{
					location[l].x2=str2num(script2);
				}
				if (!(strcmp("Y1", script1)))
				{
					location[l].y1=str2num(script2);
				}
				if (!(strcmp("Y2", script1)))
				{
					location[l].y2=str2num(script2);
					location[l].region=i;
					l++;
				}
			}
		}
		while (script1[0]!='}' && !noregion && (++loopexit < MAXLOOPS) );
	}


	locationcount=l;
	logoutcount=0;
	//Instalog

	strcpy(sect, "SECTION INSTALOG");

	safedelete(iter); //as the name implies, this is safe :P, Xan
	iter = Scripts::Regions->getNewIterator(sect);

	if (iter==NULL) return;

	loopexit=0;
	do
	{
		iter->parseLine(script1, script2);
		if(!(strcmp(script1,"X1"))) logout[logoutcount].x1=str2num(script2);
		if(!(strcmp(script1,"Y1"))) logout[logoutcount].y1=str2num(script2);
		if(!(strcmp(script1,"X2"))) logout[logoutcount].x2=str2num(script2);
		if(!(strcmp(script1,"Y2")))
		{
			logout[logoutcount].y2=str2num(script2);
			logoutcount++;
		}
	} while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	safedelete(iter);


}

//<Anthalir>
short calcRegionFromXY(Location pos)
{
	return calcRegionFromXY(pos.x, pos.y);
}
//</Anthalir>

short calcRegionFromXY(int x, int y)
{
	int i;
	for (i=0;i<locationcount;i++)
	{
		if (location[i].x1<=x && location[i].y1<=y && location[i].x2>=x &&
			location[i].y2>=y)
		{
			return location[i].region;
		}
	}

	return INVALID;


}

void checkregion(P_CHAR pc)
{
//	P_CHAR pc = MAKE_CHAR_REF( i );
	VALIDATEPC( pc );
 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	int calcreg, s, j;

	calcreg=calcRegionFromXY( pc->getPosition() );
	if (calcreg!= pc->region)
	{
		/*
		if ( pc->amxevents[EVENT_CHR_ONREGIONCHANGE] )
			pc->amxevents[EVENT_CHR_ONREGIONCHANGE]->Call( pc->getSerial32(), pc->region, calcreg);
		*/
		pc->runAmxEvent( EVENT_CHR_ONREGIONCHANGE, pc->getSerial32(), pc->region, calcreg);

		s = pc->getSocket();
		if (s!=INVALID)
		{
			pweather(s);
			Calendar::commitSeason(pc);
			if (region[ pc->region ].name[0]!=0)
			{
				sprintf( temp, TRANSLATE("You have left %s."), region[ pc->region].name);
				sysmessage(s, temp);
			}
			if (region[calcreg].name[0]!=0)
			{
				sprintf( temp, TRANSLATE("You have entered %s."), region[calcreg].name);
				sysmessage(s, temp);
			}
			j=strcmp(region[calcreg].guardowner, region[pc->region].guardowner);
			if ( (region[calcreg].priv&0x01)!=(region[pc->region].priv&0x01) ||
				(region[calcreg].priv&0x01 && j))
			{
				if (region[calcreg].priv&0x01)
				{
					if (region[calcreg].guardowner[0]==0)
					{
						sysmessage(s, TRANSLATE("You are now under the protection of the guards."));
					}
					else
					{
						sprintf( temp, TRANSLATE("You are now under the protection of %s guards."), region[calcreg].guardowner);
						sysmessage(s, temp);
					}
				}
				else
				{
					if (region[pc->region].guardowner[0]==0)
					{
						sysmessage(s, TRANSLATE("You are no longer under the protection of the guards."));
					}
					else
					{
						sprintf(temp, TRANSLATE("You are no longer under the protection of %s guards."), region[pc->region].guardowner);
						sysmessage(s, temp);
					}
				}
			}
		}
		pc->region=calcreg;
		if (s!=INVALID) dosocketmidi(s);
	}

}


///////////////////////////////////////////////////////////////////
// Function name     : check_region_weatherchange
// Description       : do the periodical check for weather change
// Return type       : void
// Author            : Xanathar
// Changes           : none yet
void check_region_weatherchange ()
{
	int r,sn=0, rn=0, dr=0, sm, i;

	InfoOut("performing weather change...");

	for (i=0;i<256;i++)
	{
		region_st &regionRef = region[i];
		if ((regionRef.keepchance==0)&&(regionRef.drychance==0)) continue;
		r = rand()%100;
		if ((r<=regionRef.keepchance)||(regionRef.keepchance==100)) continue;
		//we're here, let's change the weeeeather
		dr = (regionRef.wtype==0) ? 0 : regionRef.drychance;
		rn = (regionRef.wtype==1) ? 0 : regionRef.rainchance;
		sn = (regionRef.wtype==2) ? 0 : regionRef.snowchance;
		if (!regionRef.ignoreseason)
		{
			dr = static_cast<int>(static_cast<float>(dr) * Calendar::g_fCurDryMod);
			rn = static_cast<int>(static_cast<float>(rn) * Calendar::g_fCurRainMod);
			sn = static_cast<int>(static_cast<float>(sn) * Calendar::g_fCurSnowMod);
		}
		sm = dr+rn+sn;
		r = rand()%sm;
		if (r < dr) regionRef.wtype = 0;
		else if (r < (rn+dr)) regionRef.wtype = 1;
		else regionRef.wtype = 2;
	}

	//here : we should commit weather changes to players
   wtype=0;
	unsigned char packet[4] = { 0x65, 0xFF, 0x00, 0x20 };

	for (i=0;i<now;i++) { if (perm[i]) { Xsend(i, packet, 4); } }

	for (i=0;i<now;i++) { if (perm[i]) { pweather(i); } }

   ConOut("[ OK ]\n");

}
