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
\brief some utils functions
*/

#include "nxwcommn.h"
#include "sndpkg.h"
#include "addmenu.h"
#include "sregions.h"
#include "house.h"
#include "commands.h"
#include "tmpeff.h"
#include "accounts.h"
#include "boats.h"
#include "map.h"
#include "items.h"
#include "chars.h"
#include "skills.h"
#include "nox-wizard.h"
#include "scripts.h"
#include "inlines.h"
#include "range.h"
#include "network.h"

cScriptCommand::cScriptCommand( )
{
}

cScriptCommand::cScriptCommand( std::string command, std::string param )
{
	this->command=command;
	this->param=param;
}

cScriptCommand::~cScriptCommand( )
{
	
}

void cScriptCommand::execute( NXWSOCKET s )
{
	if( s<0 )
		return;
	
	P_CHAR pc= pointers::findCharBySerial( currchar[s] );
	VALIDATEPC( pc );

	strupr(command);
	strupr(param);

	if ( command == "GMMENU" ) {
		gmmenu(s, str2num(param));
		return;
	} else if ( command == "MAKEMENU" ) {
		//ndEndy PDFARE
		//Skills::MakeMenu(s, str2num(param), pc->making );
		return;
	} else if ( (SI32)command.find("MENU") != -1 ) {
		itemmenu( s, str2num(param) );
		return;
	} else if ( command == "WEBLINK" ) {
		weblaunch(s, param.c_str());
		return;
	} else if ( command == "SYSMESSAGE" ) {
		sysmessage(s, param.c_str());
		return;
	} else if ( command == "GMPAGE" ) {
		Commands::GMPage(s, param);
		return;
	} else if ( command == "CPAGE" ) {
		Commands::CPage(s, param);
		return;
	} else if ( command == "VERSION" ) {
		sysmessage(s, idname);
		return;
	} else if ( command == "ADDITEM" ) {

		std::string itemnum, amount;
		splitLine( param, itemnum, amount );
		int am = ( amount != "" )?  str2num( amount ) : INVALID; //ndEndy defined amount
		
		item::CreateFromScript( (char*)itemnum.c_str(), pc->getBackpack(), am );
		return;
	} else if ( command == "BATCH" ) {
		executebatch=str2num(param);
		return;
	} else if ( command == "INFORMATION" ) {
		sysmessage(s, TRANSLATE("Connected players [%i out of %i accounts] Items [] Characters []"),
			now,Accounts->Count());
		return;
	} else if ( command == "NPC" ) {
		P_TARGET targ= clientInfo[s]->newTarget( new cLocationTarget() );
		targ->code_callback = target_npcMenu;
		targ->buffer[0]=str2num(param);
		targ->send( getClientFromSocket(s) );
		sysmessage( s, "Select location for NPC. [Number: %i]", targ->buffer[0]);
		return;
	} else if ( command == "NOP" ) {
	    return;
	} else if ( command == "POLY" ) {
		int tmp=hex2num(param);
		pc->setId(tmp);
		pc->setOldId(tmp);
		pc->teleport();
		return;
	} else if ( command == "SKIN" ) {
		int tmp=hex2num(param);
		pc->setColor(tmp);
		pc->setOldColor(tmp);
		pc->teleport();
		return;
	} else if ( command == "LIGHT" ) {
		worldfixedlevel=hex2num(param);
		if (worldfixedlevel!=255) setabovelight(worldfixedlevel);
		else setabovelight(worldcurlevel);

		return;

	} else if ( command == "GCOLLECT" ) {
		gcollect();
		return;
	} else if ( command == "GOPLACE" ) {
		int x, y, z;
		location2xyz( str2num(param), x, y, z );
		if( x>0 ) {
			pc->MoveTo( x, y, z );
			pc->teleport();
		}
	} else if ( command == "ADDBYID" ) {
		if (s<=INVALID) return;
		P_ITEM pb = pc->getBackpack();
		if (!pb) return;

		UI32 i = param.find(' ');
		if ( i == (UI32)-1 )
			return;

		std::string p(param.begin()+i+1, param.end());
		param.erase(param.begin()+i, param.end());

		P_ITEM pi = item::addByID (str2num(param), 1, p.c_str(), 0, 100, 100, 100);
		if (pi==NULL) return;
		pi->setContSerial( pb->getSerial32() );
		pi->SetRandPosInCont(pb);
		pi->Refresh();
	} else if ( command == "@CALL" ) {
		AmxFunction::g_prgOverride->CallFn(param.c_str());
	} else if ( command == "@RUN" ) {
		AmxProgram *prg = new AmxProgram(param.c_str());
		prg->CallFn(INVALID);
		safedelete(prg);
	} else {
	    ErrOut("script command syntax error : unknown command %s", command.c_str());
	}
}


























void location2xyz(int loc, int& x, int& y, int& z)
{
	int  loopexit=0;
	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
    cScpIterator* iter = NULL;
    char script1[1024];
    char script2[1024];


	sprintf(temp, "SECTION LOCATION %i", loc);
	iter = Scripts::Location->getNewIterator(temp);

	if ((iter!=NULL))
	{
		do
		{
			iter->parseLine(script1, script2);
			if (!(strcmp(script1,"X")))
			{
				x=str2num(script2);
			}
			else if (!(strcmp(script1,"Y")))
			{
				y=str2num(script2);
			}
			else if (!(strcmp(script1,"Z")))
			{
				z=str2num(script2);
			}
		}
		while ( (strcmp(script1,"}")) && (++loopexit < MAXLOOPS) );
	}
	safedelete(iter);
}

/*!
\brief for putting single worlds of cline into comm array
*/
void splitline()
{
	int i=0;
	char *s;
	char *d;

	d=" ";
	s=strtok((char*)cline,d);
	int loopexit=0;
	while ( (s!=NULL) && (++loopexit < MAXLOOPS) )
	{
		comm[i]=(unsigned char*)s;
		i++;
		s=strtok(NULL,d);
	}
	tnum=i;
}

int strtonum(int countx, int base/*= 0*/)
{
	char *err= NULL;
	int n;

	if(comm[countx] == NULL)
		return 0;

	n= strtol((char*)comm[countx], &err, base);

	if(*err != '\0')	// invalid character found
	{
		WarnOut("error in strtonum: %c invalid digit for base %2d\n", *err, base);
		return 0;
	}

	return n;
}

/*!
\brief converts hex string comm[countx] to int
\param countx the line of comm array to convert
*/
int hexnumber(int countx)
{
	// sscanf is an ANSI function to read formated data from a string.
	if (comm[countx] == NULL)
		return 0;

	int i;
	sscanf((char*)comm[countx], "%x", &i);

	return i;
}


/*!
\brief plays the proper door sfx for doors/gates/secretdoors
\author Dupois
\since Oct 8, 1998
*/
static void doorsfx(P_ITEM pi, int x, int y)
{
 
	const int OPENWOOD = 0x00EA;
	const int OPENGATE = 0x00EB;
	const int OPENSTEEL = 0x00EC;
	const int OPENSECRET = 0x00ED;
	const int CLOSEWOOD = 0x00F1;
	const int CLOSEGATE = 0x00F2;
	const int CLOSESTEEL = 0x00F3;
	const int CLOSESECRET = 0x00F4;

	if (y==0) // Request open door sfx
	{
		if (((x>=0x0695)&&(x<0x06C5))|| // Open wooden / ratan door
			((x>=0x06D5)&&(x<=0x06F4)))
			soundeffect3(pi,OPENWOOD);

		if (((x>=0x0839)&&(x<=0x0848))|| // Open gate
			((x>=0x084C)&&(x<=0x085B))||
			((x>=0x0866)&&(x<=0x0875)))
			soundeffect3(pi,OPENGATE);

		if (((x>=0x0675)&&(x<0x0695))|| // Open metal
			((x>=0x06C5)&&(x<0x06D5)))
			soundeffect3(pi,OPENSTEEL);

		if ((x>=0x0314)&&(x<=0x0365)) // Open secret
			soundeffect3(pi,OPENSECRET);
	}
	else if (y==1) // Request close door sfx
	{
		if (((x>=0x0695)&&(x<0x06C5))|| // close wooden / ratan door
			((x>=0x06D5)&&(x<=0x06F4)))
			soundeffect3(pi,CLOSEWOOD);

		if (((x>=0x0839)&&(x<=0x0848))|| // close gate
			((x>=0x084C)&&(x<=0x085B))||
			((x>=0x0866)&&(x<=0x0875)))
			soundeffect3(pi,CLOSEGATE);

		if (((x>=0x0675)&&(x<0x0695))|| // close metal
			((x>=0x06C5)&&(x<0x06D5)))
			soundeffect3(pi,CLOSESTEEL);

		if ((x>=0x0314)&&(x<=0x0365)) // close secret
			soundeffect3(pi,CLOSESECRET);
	}

} // doorsfx() END


void dooruse(NXWSOCKET  s, P_ITEM pi /* was ITEM item*/)
{
	P_CHAR pc;
//!!! NOT VALIDATE !!, this function is called with invalid socket when automatic door close
	if(s <0)
		pc = NULL;
	else
		pc =MAKE_CHAR_REF(currchar[s]);
//	const P_ITEM pi=MAKE_ITEMREF_LR(item);	// on error return

	int i, db, x;//, z;
	char changed=0;

	/*if (ISVALIDPC(pc) && (item_inRange(pc,pi,2)==0) && s>INVALID) {
		sysmessage(s, TRANSLATE("You cannot reach the handle from here"));
		return;
	}*/

	x=pi->getId();
	for (i=0;i<DOORTYPES;i++)
	{
		db=doorbase[i];

		if (x==(db+0))
		{
			pi->setId( pi->getId()+1 );
			pi->setPosition("x", pi->getPosition("x") - 1);
			pi->setPosition("y", pi->getPosition("y") + 1);
			pi->Refresh();
			changed=1;
			doorsfx(pi, x, 0);
			tempfx::add(pc, pi, tempfx::AUTODOOR, 0, 0, 0);
			pi->dooropen=1;
		} else if (x==(db+1))
		{
			pi->setId( pi->getId()-1 );
			pi->setPosition("x", pi->getPosition("x") + 1);
			pi->setPosition("y", pi->getPosition("y") - 1);
			pi->Refresh();
			changed=1;
			doorsfx(pi, x, 1);
			pi->dooropen=0;
		} else if (x==(db+2))
		{
			pi->setId( pi->getId()+1 );
			pi->setPosition("x", pi->getPosition("x") + 1);
			pi->setPosition("y", pi->getPosition("y") + 1);
			pi->Refresh();
			changed=1;
			doorsfx(pi, x, 0);
			tempfx::add(pc , pi, tempfx::AUTODOOR, 0, 0, 0);
			pi->dooropen=1;
		} else if (x==(db+3))
		{
			pi->setId( pi->getId()-1 );
			pi->setPosition("x", pi->getPosition("x") - 1);
			pi->setPosition("y", pi->getPosition("y") - 1);
			pi->Refresh();
			changed=1;
			doorsfx(pi, x, 1);
			pi->dooropen=0;
		} else if (x==(db+4))
		{
			pi->setId( pi->getId()+1 );
			pi->setPosition("x", pi->getPosition("x") - 1);
			pi->Refresh();
			changed=1;
			doorsfx(pi, x, 0);
			tempfx::add(pc, pi, tempfx::AUTODOOR, 0, 0, 0);
			pi->dooropen=1;
		} else if (x==(db+5))
		{
			pi->setId( pi->getId()-1 );
			pi->setPosition("x", pi->getPosition("x") + 1);
			pi->Refresh();
			changed=1;
			doorsfx(pi, x, 1);
			pi->dooropen=0;
		} else if (x==(db+6))
		{
			pi->setId( pi->getId()+1 );
			pi->setPosition("x", pi->getPosition("x") + 1);
			pi->setPosition("y", pi->getPosition("y") - 1);
			pi->Refresh();
			changed=1;
			doorsfx(pi, x, 0);
			tempfx::add(pc, pi, tempfx::AUTODOOR, 0, 0, 0);
			pi->dooropen=1;
		} else if (x==(db+7))
		{
			pi->setId( pi->getId()-1 );
			pi->setPosition("x", pi->getPosition("x") - 1);
			pi->setPosition("y", pi->getPosition("y") + 1);
			pi->Refresh();
			changed=1;
			doorsfx(pi, x, 1);
			pi->dooropen=0;
		} else if (x==(db+8))
		{
			pi->setId( pi->getId()+1 );
			pi->setPosition("x", pi->getPosition("x") + 1);
			pi->setPosition("y", pi->getPosition("y") + 1);
			pi->Refresh();
			changed=1;
			doorsfx(pi, x, 0);
			tempfx::add(pc, pi, tempfx::AUTODOOR, 0, 0, 0);
			pi->dooropen=1;
		} else if (x==(db+9))
		{
			pi->setId( pi->getId()-1 );
			pi->setPosition("x", pi->getPosition("x") - 1);
			pi->setPosition("y", pi->getPosition("y") - 1);
			pi->Refresh();
			changed=1;
			doorsfx(pi, x, 1);
			pi->dooropen=0;
		} else if (x==(db+10))
		{
			pi->setId( pi->getId()+1 );
			pi->setPosition("x", pi->getPosition("x") + 1);
			pi->setPosition("y", pi->getPosition("y") - 1);
			pi->Refresh();
			changed=1;
			doorsfx(pi, x, 0);
			tempfx::add(pc, pi, tempfx::AUTODOOR, 0, 0, 0);
			pi->dooropen=1;
		} else if (x==(db+11))
		{
			pi->setId( pi->getId()-1 );
			pi->setPosition("x", pi->getPosition("x") - 1);
			pi->setPosition("y", pi->getPosition("y") + 1);
			pi->Refresh();
			changed=1;
			doorsfx(pi, x, 1);
			pi->dooropen=0;
		}
		else if (x==(db+12))
		{
			pi->setId( pi->getId()+1 );
			pi->Refresh();
			changed=1;
			doorsfx(pi, x, 0);
			tempfx::add(pc, pi, tempfx::AUTODOOR, 0, 0, 0);
			pi->dooropen=1;
		} else if (x==(db+13))
		{
			pi->setId( pi->getId()-1 );
			pi->Refresh();
			changed=1;
			doorsfx(pi, x, 1);
			pi->dooropen=0;
		} else if (x==(db+14))
		{
			pi->setId( pi->getId()+1 );
			pi->setPosition("y", pi->getPosition("y") - 1);
			pi->Refresh();
			changed=1;
			doorsfx(pi, x, 0);
			tempfx::add(pc, pi, tempfx::AUTODOOR, 0, 0, 0);
			pi->dooropen=1;
		} else if (x==(db+15))
		{
			pi->setId( pi->getId()-1 );
			pi->setPosition("y", pi->getPosition("y") + 1);
			pi->Refresh();
			changed=1;
			doorsfx(pi, x, 1);
			pi->dooropen=0;
		}
	}

	if (changed && ISVALIDPC(pc))
	{
		
		pc->objectdelay=uiCurrentTime+ (server_data.objectdelay/4)*MY_CLOCKS_PER_SEC;
		// house refreshment when a house owner or friend of a houe opens the house door

		int j, houseowner_serial,ds;
		P_ITEM pi_house=findmulti( pi->getPosition() );
		if(ISVALIDPI(pi_house))
		{
			const P_ITEM pi2=pi_house;
			if ( pi_house->IsHouse() )
			{
				houseowner_serial=pi2->getOwnerSerial32();
				j=on_hlist(pi_house, pc->getSerial().ser1,  pc->getSerial().ser2,  pc->getSerial().ser3,  pc->getSerial().ser4, NULL);
				if ( j==H_FRIEND || (pi2->getOwnerSerial32()==pc->getSerial32()) ) // house_refresh stuff, LB, friends of the house can do.
				{
					if (s!=INVALID)
					{
						if (SrvParms->housedecay_secs!=0)
							ds=((pi2->time_unused)*100)/(SrvParms->housedecay_secs);
						else ds=INVALID;

						if (ds>=50) // sysmessage iff decay status >=50%
						{
							if (houseowner_serial!= pc->getSerial32())
								sysmessage(s,TRANSLATE("You refreshed your friend's house"));
							else
								sysmessage(s,TRANSLATE("You refreshed the house"));
						}
					}

					pi2->time_unused=0;
					pi2->timeused_last=getclock();
				}
				//ConOut("house name: %s\n",pi2->name);
			} // end of is_house
		} // end of is_multi
	}

	if (changed==0 && s>INVALID) 
		sysmessage(s, TRANSLATE("This doesnt seem to be a valid door type. Contact a GM."));
}

void endmessage(int x) // If shutdown is initialized
{
	UI32 igetclock = uiCurrentTime;

	if (endtime<igetclock)
		endtime=igetclock;

	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	sprintf(temp, "server going down in %i minutes.\n",
		((endtime-igetclock)/MY_CLOCKS_PER_SEC)/60);
	sysbroadcast(temp);
	InfoOut(temp);
}

/*!
\brief Execute command from script
\author Unknown, ported to std::string by Akron
\param s the socket of the executer
\param script1 the command
\param script2 parameters of the command
*/
void scriptcommand (NXWSOCKET s, std::string script1, std::string script2) // Execute command from script
{
	cScriptCommand command( script1, script2 );
	command.execute( s );
}

void batchcheck(int s) // Do we have to run a batch file
{
	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	char script1[1024], script2[1024];
    cScpIterator* iter = NULL;


	sprintf(temp, "SECTION BATCH %i", executebatch);

    iter = Scripts::Menus->getNewIterator(temp);
    if (iter==NULL) return;

	int loopexit=0;
	do
	{
		iter->parseLine(script1, script2);
		if ((script1[0]!='}')&&(script1[0]!='{')) scriptcommand(s, script1, script2);
	}
	while ( (script1[0]!='}') && (++loopexit < MAXLOOPS) );
	safedelete(iter);
	executebatch=0;
}



int checkBoundingBox(int xPos, int yPos, int fx1, int fy1, int fz1, int fx2, int fy2)
{
	if (xPos>=((fx1<fx2)?fx1:fx2) && xPos<=((fx1<fx2)?fx2:fx1))
		if (yPos>=((fy1<fy2)?fy1:fy2) && yPos<=((fy1<fy2)?fy2:fy1))
			if (fz1==INVALID || abs(fz1-getHeight(Loc(xPos, yPos, fz1)))<=5)
				return 1;
			return 0;
}

int checkBoundingCircle(int xPos, int yPos, int fx1, int fy1, int fz1, int radius)
{
	if ( (xPos-fx1)*(xPos-fx1) + (yPos-fy1)*(yPos-fy1) <= radius * radius)
		if (fz1==INVALID || abs(fz1-getHeight(Loc(xPos, yPos, fz1)))<=5)
			return 1;
		return 0;
}

UI32 getclockday()
{
	UI32 seconds;
	UI32 days ;
#ifdef __unix__
	timeval buffer ;
	gettimeofday(&buffer,NULL) ;
	seconds = buffer.tv_sec ;
#else
	timeb buffer ;
	::ftime(&buffer) ;
	seconds = buffer.time ;
#endif
	days = seconds/86400 ;  // (60secs/minute * 60 minute/hour * 24 hour/day)
	return days ;
}

UI32 getclock()
{
	UI32 milliseconds;
	UI32 seconds ;
#ifdef __unix__
	timeval buffer ;
	gettimeofday(&buffer,NULL) ;
	seconds = buffer.tv_sec ;
	milliseconds = buffer.tv_usec/1000 ;
#else
	timeb buffer ;
	::ftime(&buffer) ;
	seconds = buffer.time ;
	milliseconds = buffer.millitm ;
#endif
	if (milliseconds < initialservermill)
	{
		milliseconds = milliseconds + 1000 ;
		seconds  = seconds - 1 ;
	}
	milliseconds = ((seconds - initialserversec) * 1000) + (milliseconds -initialservermill ) ;
	return milliseconds ;
}

/*!
\author Keldan
\since 0.82r3
\brief get current system clock time

used by getSystemTime amx function
*/
UI32 getsysclock()
{
   UI32 seconds ;
#ifdef __unix__
   timeval buffer ;
   gettimeofday(&buffer,NULL) ;
   seconds = buffer.tv_sec ;
#else
   timeb buffer ;
   ::ftime(&buffer) ;
   seconds = buffer.time ;
#endif
   return seconds ;
}

void setabovelight(unsigned char lightchar)
{

	if (lightchar != worldcurlevel)
	{
		worldcurlevel=lightchar;
		NxwSocketWrapper sw;
		sw.fillOnline();
		for( sw.rewind(); !sw.isEmpty(); sw++ )
		{
			dolight(sw.getSocket(), worldcurlevel);
		}
	}
}

char indungeon(P_CHAR pc)
{

	VALIDATEPCR( pc, 0 );

	if ( pc->getPosition().x < 5119)
		return 0;

	int x1 = (pc->getPosition().x - 5119) >> 8;
	int y1 = (pc->getPosition().y >> 8);

	switch (y1)
	{
	case 5:
	case 0:	return 1;
	case 1:
		if (x1 != 0) return 1;
		return 0;
	case 2:
	case 3:
		if (x1 < 3) return 1;
		else return 0;
	case 4:
	case 6:
		if (x1 == 0) return 1;
		else return 0;
	case 7:
		if (x1 < 2) return 1;
		else return 0;
	}
	return 0;

}

/*!
\brief converts x,y coords to sextant coords
\author LB
\param x X-coord of object
\param y Y-coord of object
\param sextant sextant coords as string (char *)
\param t2a are in lost lands?
\warning memory for output string sextant has to be reserved by callee !
		 if not -> crash (has to be >=36 bytes !)
\note thanks goes to Balxan / UOAM for the basic alogithm
	  could be optimized a lot, but the freuqency of being called is probably very low
*/
void getSextantCoords(SI32 x, SI32 y, LOGICAL t2a, char *sextant)
{
   double Tx, Ty, Dx, Dy, Cx, Cy, Wx, Wy, Mx, My, Hx, Hy;
   signed int xH, xM, yH, yM;
   char xHs[20], yHs[20], xMs[20], yMs[20];

   if (t2a) // for t2a the center is in the middle
   {
	   Cy = 3112.0;
	   Cx = 5936.0;
   } else
   {
	  // center, LB's throne *g*
      Cx = 1323.0;
      Cy = 1624.0;
   }

   //  map dimensions
   Wx = 5120.0;
   Wy = 4096.0;

   // convert input ints to float;
   Tx = (double) x;
   Ty = (double) y;

   // main calculation
   Dx = ( (Tx - Cx) * 360.0 ) / Wx;
   Dy = ( (Ty - Cy) * 360.0 ) / Wy;

   ////// now let's get minutes, hours & directions from it
   Hx = (signed int) Dx; // get hours (cut off digits after comma, no idea if there's a cleaner/better way)

   Mx = Dx - Hx; // get minutes
   Mx = Mx * 60;

   Hy = (signed int) Dy;
   My = Dy - Hy;
   My = My * 60;

   // convert the results to ints;
   xH = (signed int) Hx;
   xM = (signed int) Mx;
   yH = (signed int) Hy;
   yM = (signed int) My;

   // now compose result string

   numtostr(abs(xH),xHs);
   numtostr(abs(xM),xMs);
   numtostr(abs(yH),yHs);
   numtostr(abs(yM),yMs);

   strcpy(sextant, xHs);
   strcat(sextant,"o ");
   strcat(sextant, xMs);
   strcat(sextant,"' ");

   if (xH>=0) strcat(sextant,"E"); else strcat(sextant,"W");

   strcat(sextant, "  ");
   strcat(sextant, yHs);
   strcat(sextant,"o ");
   strcat(sextant, yMs);
   strcat(sextant,"' ");
   if (yH>=0) strcat(sextant,"S"); else strcat(sextant,"N");
}


void npcsimpleattacktarget(CHARACTER target2, CHARACTER target)
{
	P_CHAR pc_target2 = MAKE_CHAR_REF( target2 );
	VALIDATEPC( pc_target2 );
	P_CHAR pc_target  = MAKE_CHAR_REF( target  );
	VALIDATEPC( pc_target );
	if ( pc_target->dead || pc_target2->dead || ( pc_target->targserial == pc_target2->getSerial32() && pc_target2->targserial == pc_target->getSerial32() ) )
		return;

	pc_target->fight( pc_target2 );
	pc_target->SetAttackFirst();
	pc_target2->fight( pc_target );
	pc_target2->ResetAttackFirst();
}

/*!
\brief delete a defined quantity of a specified id from a specified char
\param c character
\param id item id
\param amount amount to delete
\param not_deleted number of items that could NOT be deleted
*/
void delequan(int c, short id, int amount, int &not_deleted)
{
	P_CHAR pc=MAKE_CHAR_REF(c);
	if(!ISVALIDPC(pc)) 
	{ 
		LogError("invalid character used");
		not_deleted = amount; 
		return; 
	}

	P_ITEM pi= pc->getBackpack();
	if(!ISVALIDPI(pi)) 
	{ 
		LogError("invalid backpack");
		not_deleted = amount; 
		return; 
	}

	not_deleted = pi->DeleteAmount(amount,id);
}


void donewithcall(int s, int type)
{
	P_CHAR pc = MAKE_CHAR_REF( currchar[s] );
	VALIDATEPC( pc );
	int cn = pc->callnum;
	if(cn!=0) //Player is on a call
	{
		if(type==1) //Player is a GM
		{
			gmpages[cn].handled=1;
			gmpages[cn].name[0]='\0';
			gmpages[cn].reason[0]='\0';
			gmpages[cn].serial.serial32= 0;
			sysmessage(s,"Call removed from the GM queue.");
		}
		else //Player is a counselor
		{
			counspages[cn].handled=1;
			counspages[cn].name[0]='\0';
			counspages[cn].reason[0]='\0';
			gmpages[cn].serial.serial32= 0;
			sysmessage(s,"Call removed from the Counselor queue.");
		}
		pc->callnum=0;
	}
	else
	{
		sysmessage(s,"You are currently not on a call");
	}
}

/*!
\brief calculates the value of an item (maybe)
\return int value
\param i item
\param value unknown
\todo write documentation
*/
int calcValue(int i, int value)
{
	int mod=10;
	const P_ITEM pi=MAKE_ITEM_REF(i);
	VALIDATEPIR(pi, value );

	if (pi->type==19)
	{
		if (pi->morex>500) mod=mod+1;
		if (pi->morex>900) mod=mod+1;
		if (pi->morex>1000) mod=mod+1;
		if (pi->morez>1) mod=mod+(3*(pi->morez -1));
		value=(value*mod)/10;
	}

	// Lines added for Rank System by Magius(CHE)
	if (pi->rank>0 && pi->rank<10 && SrvParms->rank_system==1)
	{
		value=(int) (pi->rank*value)/10;
	}
	if (value<1) value=1;
	// end addon

	// Lines added for Trade System by Magius(CHE) (2)
	if (pi->rndvaluerate<0) pi->rndvaluerate=0;
	if (pi->rndvaluerate!=0 && SrvParms->trade_system==1) {
		value+=(int) (value*pi->rndvaluerate)/1000;
	}
	if (value<1) value=1;
	// end addon

	return value;
}

/*!
\brief trade System
\author Magius(CHE)
\return int ?
\param npcnum2 ?
\param i ?
\param value ?
\param goodtype ?
\todo write documentation
*/
int calcGoodValue(CHARACTER npcnum2, int i, int value,int goodtype)
{ 
	
	P_CHAR npc=MAKE_CHAR_REF(npcnum2);
	VALIDATEPCR(npc,0);
	
	const P_ITEM pi=MAKE_ITEM_REF(i);
	VALIDATEPIR(pi,value);

	int actreg=calcRegionFromXY( npc->getPosition() );
	int regvalue=0;
	int x;
	int good=pi->good;

	if (good<=INVALID || good >255 || actreg<=INVALID || actreg>255) return value;

	if (goodtype==1) regvalue=region[actreg].goodsell[pi->good]; // Vendor SELL
	if (goodtype==0) regvalue=region[actreg].goodbuy[pi->good]; // Vendor BUY

	x=(int) (value*abs(regvalue))/1000;

	if (regvalue<0)	value-=x;
	else value+=x;

	if (value<=0) value=1; // Added by Magius(CHE) (2)

	return value;
}

/*!
\brief count the number of bit set
\return int number of bit set
\param number the number
\remark is this really usefull ?
*/
int numbitsset( int number )
{
	int bitsset = 0;

	while( number )
	{
		if( number & 0x1 ) bitsset++;
		number >>= 1;
	}
	return bitsset;
}


/*!
\todo write documentation
*/
int whichbit( int number, int bit )
{
	int i, setbits = 0, whichbit = 0, intsize = sizeof(int) * 8;

	for( i=0; i<intsize; i++ )
	{
		if( number & 0x1 ) setbits++;

		if( setbits == bit )
		{
			whichbit = i+1;
			break;
		}
		number >>= 1;
	}

	return whichbit;
}

/*!
\author Akron
\brief return the start of the line passed
\param line string that represent the line
\return the pointer to the start of the line, chopping out all initial space characters
\since 0.82r3
*/
char *linestart(char *line)
{
	char*t = line;
	while (isspace(*t)) t++;
	return t;
}

/*!
\author Akron
\brief capitalize a c++ string
\param str the string to capitalize
\since 0.82r3
*/
void strupr(std::string &str)
{
	for(std::string::iterator it = str.begin(); it != str.end(); it++)
		if ( islower(*it) )
			*it -= 0x20;
}

/*!
\author Akron
\brief lowerize a c++ string
\param str the string to lowerize
\since 0.82r3
*/
void strlwr(std::string &str)
{
	for(std::string::iterator it = str.begin(); it != str.end(); it++)
		if ( isupper(*it) )
			*it += 0x20;
}
