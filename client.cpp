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
#include "client.h"
#include "sndpkg.h"
#include "debug.h"
#include "set.h"
#include "itemid.h"
#include "speech.h"

cNxwClientObj::cNxwClientObj( NXWSOCKET s ) {
	this->m_sck=s;
	currentCommand= NULL;
}

void cNxwClientObj::sysmsg(short color, char* txt, ...)
{
	UI08 talk2[18]={ 0xAE, 0x00, };
	char unicodetext[512];

	va_list argptr;
	char msg[512];
	va_start( argptr, txt );
	vsnprintf( msg, sizeof(msg)-1, txt, argptr );
	va_end( argptr );
	int ucl = ( strlen ( msg ) * 2 ) + 2 ;
	int tl = ucl + 48 ;

	char2wchar(msg);
	memcpy(unicodetext, Unicode::temp, ucl);

	ShortToCharPtr(tl, talk2 +1);
	LongToCharPtr(0x01010101, talk2 +3); 	// ID
	ShortToCharPtr(0x0101, talk2 +7);	// Model
	talk2[9] = 0;				// Type
	ShortToCharPtr(color, talk2 +10);	// Color
	ShortToCharPtr(0x0003, talk2 +12);	// Font type

	talk2[14] = server_data.Unicodelanguage[0];
	talk2[15] = server_data.Unicodelanguage[1];
	talk2[16] = server_data.Unicodelanguage[2];
	talk2[17] = 0;

	send(talk2, 18);
	unsigned char sysname[31]="System\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
	send(sysname, 30);
	send(unicodetext, ucl);
//AoS/	Network->FlushBuffer(m_sck);
}

void cNxwClientObj::sysmsg(char* txt, ...)
{
	UI08 talk2[19]={ 0xAE, 0x00, };
	char unicodetext[512];

	va_list argptr;
	char msg[512];
	va_start( argptr, txt );
	vsnprintf( msg, sizeof(msg)-1, txt, argptr );
	va_end( argptr );

	int ucl = ( strlen ( msg ) * 2 ) + 2 ;
	int tl = ucl + 48 ;

	char2wchar(msg);
	memcpy(unicodetext, Unicode::temp, ucl);

	ShortToCharPtr(tl, talk2 +1);
	talk2[3] = 1;
	talk2[4] = 1;
	talk2[5] = 1;
	talk2[6] = 1;
	talk2[7] = 1;
	talk2[8] = 1;
	talk2[9] = 6;

	ShortToCharPtr(0x0387, talk2 +10); //Color  - Previous default was 0x0040 - 0x03E9
	ShortToCharPtr(0x0003, talk2 +12);

	talk2[14] = server_data.Unicodelanguage[0];
	talk2[15] = server_data.Unicodelanguage[1];
	talk2[16] = server_data.Unicodelanguage[2];
	talk2[17] = 0;

	send(talk2, 18);
	unsigned char sysname[31]="System\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
	send(sysname, 30);
	send(unicodetext, ucl);
//AoS/	Network->FlushBuffer(m_sck);
}

NXWSOCKET cNxwClientObj::toLegacySocket() 
{ 
	return m_sck; 
}

void cNxwClientObj::setLegacySocket(NXWSOCKET s) 
{ 
	if (s < 0 || s > MAXCLIENT) return;
	m_sck = s;
}

bool cNxwClientObj::isDragging() 
{ 
	return (DRAGGED[m_sck]>0); 
}

void cNxwClientObj::setDragging() 
{ 
	DRAGGED[m_sck]=1; 
}

void cNxwClientObj::resetDragging() 
{ 
	DRAGGED[m_sck]=0; 
}

bool cNxwClientObj::inGame()
{ 
	return perm[m_sck]!='\0'; 
}

P_CHAR cNxwClientObj::currChar() 
{ 
	
	return MAKE_CHAR_REF(currchar[m_sck]); 
}

int cNxwClientObj::currCharIdx()
{ 
	return currchar[m_sck]; 
}

int cNxwClientObj::getRealSocket() 
{ 
	return client[m_sck]; 
}

BYTE *cNxwClientObj::getRcvBuffer()
{ 
	return &buffer[m_sck][0];
}


void cNxwClientObj::send(const void *point, int length)
{ 
	if (m_sck>-1 && m_sck < now) Xsend(m_sck,point,length);
}

int cNxwClientObj::toInt()
{
	return m_sck;
}

void cNxwClientObj::sendSpellBook(P_ITEM pi)
{
    if (pi==NULL) // item number send by client?
        pi=pointers::findItemBySerPtr(getRcvBuffer()+1);

    P_CHAR pc_currchar = this->currChar();

	P_ITEM p_back=pc_currchar->getBackpack();


    if (!pi )
		if( ISVALIDPI(p_back))
		{
			NxwItemWrapper si;
			si.fillItemsInContainer( p_back, false );
			for( si.rewind(); !si.isEmpty(); si++ )
			{
	            P_ITEM pj=si.getItem();
				if( ISVALIDPI(pj) && pj->type==ITYPE_SPELLBOOK)
				{
	                pi=pj;
					break;
				}
			}
		}
		else
		{
	        NxwItemWrapper si;
			si.fillItemWeared( pc_currchar, true, true, false );
			for( si.rewind(); !si.isEmpty(); si++ )
			{
	            P_ITEM pj=si.getItem();
	            if( ISVALIDPI(pj) && pj->type==ITYPE_SPELLBOOK && pj->layer==ITYPE_CONTAINER)
				{
	                pi=pj;
					break;
				}
			}
		}
    // LB remark: If you want more than one spellbook per player working predictable
    // quite a lot of that function needs to be rewritten !
    // reason: just have a look at the loop above ...

    if (!pi ||  // no book at all
        ((ISVALIDPI(p_back)) && (pi->getContSerial()!=p_back->getSerial32()) &&    // not in primary pack
                !pc_currchar->IsWearing(pi)))       // not equipped
    {
        this->sysmsg(TRANSLATE("In order to open spellbook, it must be equipped in your hand or in the first layer of your backpack."));
        return;
    }

    if (pi->layer!=1) senditem(m_sck,pi); // prevents crash if pack not open

    char sbookstart[8]="\x24\x40\x01\x02\x03\xFF\xFF";
    sbookstart[1]= pi->getSerial().ser1;
    sbookstart[2]= pi->getSerial().ser2;
    sbookstart[3]= pi->getSerial().ser3;
    sbookstart[4]= pi->getSerial().ser4;
    send(sbookstart, 7);

    int spells[70] = {0,};
    int i, scount=0;

    NxwItemWrapper gri;
	gri.fillItemsInContainer( pi, false );
	for( gri.rewind(); !gri.isEmpty(); gri++ )
    {
		P_ITEM pj=gri.getItem(); //Luxor: added support for all-spell scroll
		if(ISVALIDPI(pj))
			if( pj->IsSpellScroll() || pi->CountItems(0x1F6D,-1,false) > 0)
			{
				if (((pj->id()-0x1F2D)>=0)&&((pj->id()-0x1F2D)<70)) spells[(pj->id()-0x1F2D)]=1;
			}
    }

    // Fix for Reactive Armor/Bird's Eye dumbness. :)
    i=spells[0];
    spells[0]=spells[1];
    spells[1]=spells[2];
    spells[2]=spells[3];
    spells[3]=spells[4];
    spells[4]=spells[5];
    spells[5]=spells[6]; // Morac is right! :)
    spells[6]=i;
    // End fix.

    if (spells[64])
    {
        for (i=0;i<70;i++)
            spells[i]=1;
        spells[64]=0;
    }
    spells[64]=spells[65];
    spells[65]=0;

    for (i=0;i<70;i++)
    {
        if (spells[i]) scount++;
    }
    char sbookinit[6]="\x3C\x00\x3E\x00\x03";
    sbookinit[1]=((scount*19)+5)>>8;
    sbookinit[2]=((scount*19)+5)%256;
    sbookinit[3]=scount>>8;
    sbookinit[4]=scount%256;
    if (scount>0) send(sbookinit, 5);

    char sbookspell[20]="\x40\x01\x02\x03\x1F\x2E\x00\x00\x01\x00\x48\x00\x7D\x40\x01\x02\x03\x00\x00";
    for (i=0;i<70;i++)
    {
        if (spells[i])
        {
            sbookspell[0]=0x41;
            sbookspell[1]=0x00;
            sbookspell[2]=0x00;
            sbookspell[3]=i+1;
            sbookspell[8]=i+1;
            sbookspell[13]= pi->getSerial().ser1;
            sbookspell[14]= pi->getSerial().ser2;
            sbookspell[15]= pi->getSerial().ser3;
            sbookspell[16]= pi->getSerial().ser4;
            send(sbookspell, 19);
        }
    }
}


void cNxwClientObj::sendSFX(unsigned char a, unsigned char b, bool bIncludeNearby)
{
	unsigned char sfx[13]="\x54\x01\x12\x34\x00\x00\x06\x40\x05\x9A\x00\x00";
	P_CHAR pc = currChar();
	Location charpos= pc->getPosition();

	sfx[2]= a;
	sfx[3]= b;
	sfx[6]= charpos.x >> 8;
	sfx[7]= charpos.x % 256;
	sfx[8]= charpos.y >> 8;
	sfx[9]= charpos.y % 256;
	
	if (bIncludeNearby) {
		NxwSocketWrapper sw;
		sw.fillOnline( pc );
		for( sw.rewind(); !sw.isEmpty(); sw++ ) {

			NXWCLIENT c= sw.getClient();
			if (c!=NULL)
				c->send(sfx, 12);
		}
	}
	else send(sfx,12); //Endy fix for double send
}

void cNxwClientObj::doTargeting(char* message) {
    sysmsg(message);
    sendTargetCursor(m_sck, 0, 1, 0, 255);
}

void cNxwClientObj::receiveTarget(TargetLocation tl) {
    targets.push_back(tl);
}

td_targets cNxwClientObj::getTargets() {
	return targets;
}

TargetLocation* cNxwClientObj::getLastTarget() {
	if(targets.empty())
		return NULL;
	return &(targets.back());
}

void cNxwClientObj::startCommand(P_COMMAND cmd, char* params) {
    if(cmd==NULL) {
      	sysmsg("Command not found.");
      	return;
    }
    if(cmd->exec!=NULL) {
		currentCommand= cmd->exec;
		char buffer[512]; // the same as in commands.cpp :(
        strcpy(buffer, params);
		cmdParams.clear();
		char* token= strtok(buffer, " ");
        token= strtok(NULL, " ");
        char* warn= NULL;
		bool needtargets= false;
        while(token!=NULL) {
			string param= token;
            strupr(token);
            if(strcmp(token, "LAST")==0) {
                if(warn==NULL) {
					if(cmd->targetingMask && TARG_LAST) {
						if(!targets.empty()) {
                            currentCommand= currentCommand->nextStep;
							needtargets= true;
						} else
                            warn= "There's no last target available.";
                    } else
                        warn= "Last target not supported by the command: %s";
                }
			} else if(strcmp(token, "SELF")==0) {
                if(warn==NULL) {
					if(cmd->targetingMask && TARG_SELF) {
						P_CHAR self = MAKE_CHAR_REF( currchar[toInt()] );
						if(!needtargets) {
							needtargets= true;
							targets.clear();
						}
						receiveTarget(*new TargetLocation(self));
						currentCommand= currentCommand->nextStep;
                    } else
                        warn= "Target \"self\" not supported by the command: %s";
                }
			} else {
				cmdParams.push_back( param );
			}
            token= strtok(NULL, " ");
        }
        if(warn)
            sysmsg(warn, cmd->cmd_name);
		if(!needtargets)
			targets.clear();
        continueCommand();
    } else
      	sysmsg("Command inconsistency error: please call a GM.");
}

void cNxwClientObj::continueCommand() {
    P_COMMANDSTEP cmd= currentCommand;
    if(cmd!=NULL) {
        currentCommand= cmd->nextStep;
        cmd->execute(this);
    } else
        ErrOut("No more steps in continue command.");
}

string cNxwClientObj::getParamsAsString() {
	if(cmdParams.empty())
		return string("");
	string text(cmdParams[0]);
	for(UI32 i=1; i<cmdParams.size(); i++) {
		text+= ' ';
		text+=cmdParams[i];
	}
	return text;
}


/*!
\brief Sends to a client a remove object packet, for objects disappearing
\author Luxor
*/
void cNxwClientObj::sendRemoveObject(P_OBJECT po)
{
	VALIDATEPO(po);

	UI08 removeitem[5]={ 0x1D, 0x00, };
	LongToCharPtr(po->getSerial32(), removeitem +1);
	send(removeitem, 5);
//AoS/	Network->FlushBuffer(m_sck);
}

SERIAL currchar[MAXCLIENT];
