  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


#include "nxwcommn.h"
#include "classes.h"
#include "network.h"
#include "sndpkg.h"
#include "debug.h"
#include "house.h"
#include "boats.h"
#include "set.h"
#include "chars.h"
#include "items.h"
#include "basics.h"
#include "inlines.h"
#include "utils.h"
#include "scripts.h"

//////////////////////////////OLD CODE TO REMOVE ////////////////////////////////////

// placeguildstone() : spawns a renamed gravestone at players location and removes deed
// Placer gets guildmaster, whatever he does ;)
// Force placer to give that damn guild a damn name :)

void cGuilds::StonePlacement(int s)
{
	int /*stone,*/guildnumber;
	P_ITEM pStone;
	//unsigned int k; // lb, msvc++ 5.0 didnt like the guild(int x,inty) ...
	char stonename[60];
	P_CHAR pc = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	Location charpos= pc->getPosition();
	/*if (CheckValidPlace(s)!=1)
	{
	//Commented out for now -- AntiChrist --
	//if (CheckValidPlace(s)!=0)
		sysmessage(s,TRANSLATE("You cannot place guildstones at any other location than your house")); return;
	} */
	P_ITEM pi_fx1=MAKE_ITEM_REF(pc->fx1);
	if(ISVALIDPI(pi_fx1))
	{
		if (pi_fx1->getId()==0x14F0)
		{
			if (pc->GetGuildNumber() !=0)
			{
				itemmessage(s,TRANSLATE("You are already in a guild."),pi_fx1->getSerial32());
				return;
			}
			guildnumber=Guilds->SearchSlot(0,1);
			if (guildnumber==-1)
			{
				itemmessage(s,TRANSLATE("There are already enough guildstones placed."),pi_fx1->getSerial32());
				return;
			}
			pc->SetGuildNumber( guildnumber );
			pStone = item::CreateFromScript( "$item_hardcoded" );
			if (!ISVALIDPI(pStone))
			{//AntiChrist - to prevent crashes				
				pc->sysmsg(TRANSLATE("Cannot create guildstone"));
				return;
			}
			pStone->setId( 0x0ED5 );
			pStone->setCurrentName( "Guildstone for a unnamed guild" );
			if ( pc->getId() == BODY_FEMALE )
				pc->SetGuildTitle("Guildmistress");
			else
				pc->SetGuildTitle("Guildmaster");
			guilds[guildnumber].free = 0;
			guilds[guildnumber].members = 1;
			guilds[guildnumber].member[1] = pc->getSerial32();
			guilds[guildnumber].type = 0;
			guilds[guildnumber].abbreviation[0] = 0;
			strcpy(guilds[guildnumber].webpage,DEFAULTWEBPAGE);
			strcpy(guilds[guildnumber].charter,DEFAULTCHARTER);
			pStone->setPosition( charpos );
			pStone->type= ITYPE_GUILDSTONE;
			pStone->priv= 0;
			pStone->Refresh();
			pi_fx1->Delete();
			pc->fx1 = 0;
			guilds[guildnumber].stone= pStone->getSerial32();
			guilds[guildnumber].master= pc->getSerial32();
//guild will be rewrited tomorrow so np
//			entrygump(s, pc->getSerial().ser1, pc->getSerial().ser2, pc->getSerial().ser3, pc->getSerial().ser4,100,1,40,TRANSLATE("Enter a name for the guild."));
		}
		else
		{
			guildnumber = SearchByStone(s);
			if (guildnumber==-1)
			{//AntiChrist
				pc->sysmsg(TRANSLATE("There are already enough guildstones placed."));
				return;
			}
			if (( pi_fx1->getSerial32()==guilds[guildnumber].stone &&
				pc->getSerial32() == guilds[guildnumber].master) ||
				pc->IsGM() )
			{
				sprintf(stonename, TRANSLATE("Guildstone for %s"), guilds[guildnumber].name);
				pStone = item::CreateFromScript( "$item_hardcoded" );
				if ( !ISVALIDPI(pStone) )
				{//AntiChrist - to preview crashes
					pc->sysmsg(TRANSLATE("Cannot create guildstone"));
					return;
				}
                pStone->setId( 0xED5 );
				pStone->setCurrentName( stonename );
				pStone->setPosition( charpos );
				pStone->type= ITYPE_GUILDSTONE;
				pStone->priv= 0;
				pStone->Refresh();//AntiChrist
				pi_fx1->Delete();
				pc->fx1 = 0;
				guilds[guildnumber].stone = pStone->getSerial32();
			}
			else 
				itemmessage(s,TRANSLATE("You are not the guildmaster of this guild. Only the guildmaster may use this guildstone teleporter."),pi_fx1->getSerial32());
		}
	}
}

// guildstonemenu() : Opens the guild menu for a player
// Recognizes Guildmaster with the owner fields of the stone.
// Ofcourse checks for membership before opening any gump ;)
void cGuilds::Menu(int s, int page)
{
	int	total,
		i,
		counter,
		guild,
		recruit,
		war,
		member;
	int 	lentext;
	int 	gumpnum;
	char 	guildfealty[60],
		guildt[16],
		toggle[6];
	static char mygump[MAXMEMRECWAR][257];

	P_CHAR pc	= MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);
	P_ITEM pStone	= MAKE_ITEM_REF( pc->fx1 );
	VALIDATEPI(pStone);

	//VALIDATEPC(pc);
	//VALIDATEPI(pStone);

	int guildnumber=Guilds->SearchByStone(s);
	
	if (guildnumber==-1)
	{
		pc->sysmsg(TRANSLATE("You are not a member of this guild. Ask an existing guildmember to invite you into this guild."));
		return;
	}

	if ((guilds[guildnumber].stone!=pStone->getSerial32())&&
		(!(pc->IsGM())))
	{
		itemmessage(s,TRANSLATE("You are not a member of this guild. Ask an existing guildmember to invite you into this guild."), pStone->getSerial32());
		return;
	}

	strcpy(guildfealty,"yourself");
	if ((pc->GetGuildFealty() != pc->getSerial32())&&(pc->GetGuildFealty() !=0))
	{
		for (member=1;member<MAXGUILDMEMBERS; ++member)
		{
			if (pc->GetGuildFealty() == guilds[guildnumber].member[member])
			{
				P_CHAR pcm=pointers::findCharBySerial( guilds[guildnumber].member[member] );
				if(ISVALIDPC(pcm))
					strcpy(guildfealty, pcm->getCurrentNameC());
				break;
			}
		}
	}
	else
		pc->SetGuildFealty( pc->getSerial32() );

	if (guilds[guildnumber].master==0) 
		Guilds->CalcMaster(guildnumber);

	switch (guilds[guildnumber].type)
	{
		case 0:		strcpy(guildt, " Standard");	break;
		case 1:		strcpy(guildt, "n Order");		break;
		case 2:		strcpy(guildt, " Chaos");		break;
	}

	if (pc->HasGuildTitleToggle())
		strcpy(toggle, "On");
	else
		strcpy(toggle, "Off");

	UI08 gmprefix[9] = { 0x7C, 0x00, };

	P_CHAR pguildmaster=pointers::findCharBySerial( guilds[guildnumber].master );

	switch(page)
	{
	case 1:
		gumpnum=9;
		ShortToCharPtr(8001, gmprefix +7);
		VALIDATEPC(pguildmaster);
		lentext=sprintf(mygump[0], "%s (%s %s)",guilds[guildnumber].name, pguildmaster->GetGuildTitle(),pguildmaster->getCurrentNameC());
		strcpy(mygump[1],TRANSLATE("Recruit someone into the guild."));
		strcpy(mygump[2],TRANSLATE("View the current roster."));
		strcpy(mygump[3],TRANSLATE("View the guild's charter."));
		sprintf(mygump[4],TRANSLATE("Declare your fealty. You are currently loyal to %s."),guildfealty);
		sprintf(mygump[5],TRANSLATE("Toggle showing the guild's abbreviation in your name to unguilded people. Currently %s."),toggle);
		strcpy(mygump[6],TRANSLATE("Resign from the guild."));
		strcpy(mygump[7],TRANSLATE("View list of candidates who have been sponsored to the guild."));
		if ((pc->getSerial32()==guilds[guildnumber].master) || (pc->IsGM()))
		{
			//
			// Guildmaster Access?
			//
			gumpnum=10;
			ShortToCharPtr(8000, gmprefix +7);
			sprintf(mygump[8],TRANSLATE("Access %s functions."),pguildmaster->GetGuildTitle());
			sprintf(mygump[9],TRANSLATE("View list of guild that %s has declared war on."),guilds[guildnumber].name);
			sprintf(mygump[10],TRANSLATE("View list of guilds that have declared war on %s."),guilds[guildnumber].name);
		}
		else
		{
			//
			// Normal Members access!
			//
			sprintf(mygump[8],TRANSLATE("View list of guilds that %s have declared war on."),guilds[guildnumber].name);
			sprintf(mygump[9],TRANSLATE("View list of guilds that have declared war on %s."),guilds[guildnumber].name);
		}
		break;
	case 2:														// guildmaster menu
		gumpnum=14;
		lentext=sprintf(mygump[0], TRANSLATE("%s, %s functions"),guilds[guildnumber].name,pguildmaster->GetGuildTitle());
		strcpy(mygump[1], TRANSLATE("Set the guild name."));
		strcpy(mygump[2], TRANSLATE("Set the guild's abbreviation."));
		sprintf(mygump[3], TRANSLATE("Change the type of the guild. (Currently a%s guild.)"),guildt);
		strcpy(mygump[4], TRANSLATE("Set the guild's charter."));
		strcpy(mygump[5], TRANSLATE("Dismiss a member."));
		strcpy(mygump[6], TRANSLATE("Declare war from menu."));
		strcpy(mygump[7], TRANSLATE("Declare war from targeting enemy."));
		strcpy(mygump[8], TRANSLATE("Declare peace."));
		strcpy(mygump[9], TRANSLATE("Accept a candidate seeking membership."));
		strcpy(mygump[10],TRANSLATE("Refuse a candidate seeking membership."));
		strcpy(mygump[11],TRANSLATE("Set the guildmaster's title."));
		strcpy(mygump[12],TRANSLATE("Grant a title to another member."));
		strcpy(mygump[13],TRANSLATE("Move this guildstone."));
		strcpy(mygump[14],TRANSLATE("Return to the main menu."));
		ShortToCharPtr(8002, gmprefix +7);
		break;
	case 3:														// guild type
		gumpnum=4;
		lentext=sprintf(mygump[0], TRANSLATE("Please select the type you want your guild to be related to."));
		strcpy(mygump[1], TRANSLATE("Select this to return to main menu."));
		strcpy(mygump[2], TRANSLATE("Set to Standard."));
		strcpy(mygump[3], TRANSLATE("Set to Order."));
		strcpy(mygump[4], TRANSLATE("Set to Chaos."));
		ShortToCharPtr(8003, gmprefix +7);
		break;
	case 4:														// edit charter
		gumpnum=3;
		lentext=sprintf(mygump[0], TRANSLATE("Set %s charter."),guilds[guildnumber].name);
		strcpy(mygump[1], TRANSLATE("Select this to return to main menu."));
		strcpy(mygump[2], TRANSLATE("Set the charter."));
		strcpy(mygump[3], TRANSLATE("Set the webpage."));
		ShortToCharPtr(8004, gmprefix +7);
		break;
	case 5:														// view charter
		gumpnum=2;
		lentext=sprintf(mygump[0], TRANSLATE("%s charter."),guilds[guildnumber].name);
		sprintf(mygump[1], TRANSLATE("%s. Select this to return to the main menu."),guilds[guildnumber].charter);
		sprintf(mygump[2], TRANSLATE("Visit the guild website at %s"),guilds[guildnumber].webpage);
		ShortToCharPtr(8005, gmprefix +7);
		break;
	case 6:														// Candidates list
		gumpnum=guilds[guildnumber].recruits+1;
		lentext=sprintf(mygump[0], TRANSLATE("%s list of candidates."),guilds[guildnumber].name);
		sprintf(mygump[1], TRANSLATE("Select this to return to the menu."));
		counter=1;
		for (recruit=1;recruit<MAXGUILDRECRUITS; ++recruit )
		{
			if (guilds[guildnumber].recruit[recruit]!=0)
			{
				counter++;
				P_CHAR recr=pointers::findCharBySerial(guilds[guildnumber].recruit[recruit]);
				if(ISVALIDPC(recr))
					strcpy(mygump[counter],recr->getCurrentNameC());
			}
		}
		ShortToCharPtr(8006, gmprefix +7);
		break;
	case 7:														// roster
		gumpnum=guilds[guildnumber].members+1;
		lentext=sprintf(mygump[0], TRANSLATE("%s members roster."),guilds[guildnumber].name);
		strcpy(mygump[1], TRANSLATE("Select this to return to the menu."));
		counter=1;
		for (member=1;member<MAXGUILDMEMBERS; ++member)
		{
			P_CHAR memb=pointers::findCharBySerial(guilds[guildnumber].member[member]);
			if (ISVALIDPC(memb))
			{
				counter++;
				strcpy(mygump[counter], memb->getCurrentNameC());
			}
		}
		ShortToCharPtr(8007, gmprefix +7);
		break;
	case 8:														// member dismiss
		gumpnum=guilds[guildnumber].members+1;
		lentext=sprintf(mygump[0], TRANSLATE("Dismiss what member?"));
		strcpy(mygump[1], TRANSLATE("Select this to return to the menu."));
		counter=1;
		for (member=1;member<MAXGUILDMEMBERS; ++member )
		{
			P_CHAR memb=pointers::findCharBySerial(guilds[guildnumber].member[member]);
			if (ISVALIDPC(memb))
			{
				counter++;
				strcpy(mygump[counter],memb->getCurrentNameC());
			}
		}
		ShortToCharPtr(8008, gmprefix +7);
		break;
	case 9:														// Refuse Candidates
		gumpnum=guilds[guildnumber].recruits+1;
		lentext=sprintf(mygump[0], TRANSLATE("Refuse what candidate?"),guilds[guildnumber].name);
		strcpy(mygump[1], TRANSLATE("Select this to return to the menu."));
		counter=1;
		for (recruit=1;recruit<MAXGUILDRECRUITS; ++recruit)
		{
			P_CHAR recr = pointers::findCharBySerial(guilds[guildnumber].recruit[recruit]);
			if (ISVALIDPC(recr))
			{
				counter++;
				strcpy(mygump[counter], recr->getCurrentNameC());
			}
		}
		ShortToCharPtr(8009, gmprefix +7);
		break;
	case 10:														// Accept Candidates
		gumpnum=guilds[guildnumber].recruits+1;
		lentext=sprintf(mygump[0], TRANSLATE("Accept what candidate?."),guilds[guildnumber].name);
		strcpy(mygump[1], TRANSLATE("Select this to return to the menu."));
		counter=1;
		for (recruit=1;recruit<MAXGUILDRECRUITS; ++recruit)
		{
			P_CHAR recr = pointers::findCharBySerial(guilds[guildnumber].recruit[recruit]);
			if (ISVALIDPC(recr))
			{
				counter++;
				strcpy(mygump[counter],recr->getCurrentNameC());
			}
		}
		ShortToCharPtr(8010, gmprefix +7);
		break;
	case 11:														// War list
		gumpnum=guilds[guildnumber].wars+1;
		lentext=sprintf(mygump[0], TRANSLATE("Guild that %s has declared war on."),guilds[guildnumber].name);
		strcpy(mygump[1],TRANSLATE("Select this to return to the menu."));
		counter=1;
		for (war=1;war<MAXGUILDWARS; ++war)
		{
			if (guilds[guildnumber].war[war]!=0)
			{
				counter++;
				strcpy(mygump[counter],guilds[guilds[guildnumber].war[war]].name);
			}
		}
		ShortToCharPtr(8011, gmprefix +7);
		break;
	case 12:														// grant title
		gumpnum=guilds[guildnumber].members+1;
		lentext=sprintf(mygump[0], TRANSLATE("Grant a title to whom?"));
		strcpy(mygump[1], TRANSLATE("Select this to return to the menu."));
		counter=1;
		for (member=1;member<MAXGUILDMEMBERS; ++member)
		{
			P_CHAR membr=pointers::findCharBySerial(guilds[guildnumber].member[member]);
			if(ISVALIDPC(membr))
			{
				counter++;
				strcpy(mygump[counter],membr->getCurrentNameC());
			}
		}
		ShortToCharPtr(8012, gmprefix +7);
		break;
	case 13:														// fealty
		gumpnum=guilds[guildnumber].members+1;
		lentext=sprintf(mygump[0], TRANSLATE("Whom do you wish to be loyal to?"));
		strcpy(mygump[1], TRANSLATE("Select this to return to the menu."));
		counter=1;
		for (member=1;member<MAXGUILDMEMBERS; ++member)
		{
			P_CHAR membr=pointers::findCharBySerial(guilds[guildnumber].member[member]);
			if(ISVALIDPC(membr))
			{
				counter++;
				strcpy(mygump[counter],membr->getCurrentNameC());
			}
		}
		ShortToCharPtr(8013, gmprefix +7);
		break;
	case 14:														// declare War list
		int dummy;
		gumpnum=1;
		lentext=sprintf(mygump[0], TRANSLATE("What guilds do you with to declare war?"));
		strcpy(mygump[1], TRANSLATE("Select this to return to the menu."));
		counter=1;
		for (guild=1;guild<MAXGUILDS; ++guild)
		{
			if ((guilds[guild].free==0)&&(guildnumber!=guild))
			{
				dummy=0;
				for (war=1;war<MAXGUILDWARS;war++)
				{
					if (guilds[guildnumber].war[war]==guild) dummy=1;
				}
				if (dummy!=1) {counter++;gumpnum++;strcpy(mygump[counter],guilds[guild].name);}
			}
		}
		ShortToCharPtr(8014, gmprefix +7);
		break;
	case 15:														// declare peace list
		gumpnum=guilds[guildnumber].wars+1;
		lentext=sprintf(mygump[0], TRANSLATE("What guilds do you with to declare peace?"));
		strcpy(mygump[1], TRANSLATE("Select this to return to the menu."));
		counter=1;
		for (war=1;war<MAXGUILDWARS; ++war)
		{
			if (guilds[guildnumber].war[war]!=0)
			{
				counter++;
				strcpy(mygump[counter],guilds[guilds[guildnumber].war[war]].name);
			}
		}
		ShortToCharPtr(8015, gmprefix +7);
		break;
	case 16:														// War list 2
		gumpnum=1;
		lentext=sprintf(mygump[0], TRANSLATE("Guilds that have decalred war on %s."),guilds[guildnumber].name);
		strcpy(mygump[1], TRANSLATE("Select this to return to the menu."));
		counter=1;
		for (guild=1;guild<MAXGUILDS; ++guild)
		{
			if (guilds[guild].free==0)
			{
				for (war=1;war<MAXGUILDWARS;war++)
				{
					if (guilds[guild].war[war]==guildnumber)
					{
						counter++; gumpnum++;
						strcpy(mygump[counter],guilds[guild].name);
					}
				}
			}
		}
		ShortToCharPtr(8016, gmprefix +7);
		break;
	default:
		return;
	}

	total=9+1+lentext+1;
	for (i=1;i<=gumpnum;i++)
	{
		total+=4+1+strlen(mygump[i]);
	}
	ShortToCharPtr(total, gmprefix +1);
	LongToCharPtr(pc->getSerial32(), gmprefix +3);
	Xsend(s, gmprefix, 9);
	Xsend(s, &lentext, 1);
	Xsend(s, mygump[0], lentext);
	lentext = gumpnum;
	Xsend(s, &lentext, 1);

	UI08 gmmiddle[4] = {0x00, };
	for (i = 1; i <= gumpnum; i++)
	{
		gmmiddle[0]=0;
		gmmiddle[1]=0;
		Xsend(s,gmmiddle,4);
		lentext=strlen(mygump[i]);
		Xsend(s,&lentext,1);
		Xsend(s,mygump[i],lentext);
	}
//AoS/	Network->FlushBuffer(s);
}

// OKAY (but take another look)
// guildresign() : Player gets removed from the guilddatabase, and gets a message.
// Offcourse guilddatabase gets checked for members left, if everyone is gone, then vanish
// the guildstone. After Guildmaster resigns, the fealty of each remaining member calculates
// a new guildmaster, if there is a draw then we'll have no master until they change their minds ;)

void cGuilds::Resign( P_CHAR pc, NXWSOCKET socket )
{
	int guildnumber = pc->GetGuildNumber();
	if (guildnumber==-1)
	{
		pc->sysmsg(TRANSLATE("You are in no guild"));
		return;
	}
	struct guild_st *guild = &guilds[guildnumber];

	Guilds->EraseMember( currchar[socket] );

	pc->sysmsg(TRANSLATE("You are no longer in that guild."));

	if (guild->master == pc->getSerial32() && guild->members!=0 )
	{
		guild->master=0;
		Guilds->CalcMaster(guildnumber);
	}

	if (guild->members==0)
	{
		Guilds->EraseGuild(guildnumber);
		pc->sysmsg(TRANSLATE("You have been the last member of that guild so the stone vanishes."));
	}
}

// OKAY
// guilderaseguild() Wipes all information about a guild and removes the guildstone
void cGuilds::EraseGuild(int guildnumber)
{
	if (guildnumber<0 || guildnumber >=MAXGUILDS) return;

	P_ITEM pi_stone = pointers::findItemBySerial(guilds[guildnumber].stone);
	VALIDATEPI(pi_stone);

	int war;
	int counter;
	memset(&guilds[guildnumber], 0, sizeof(guild_st));
	guilds[guildnumber].free=1;
	pi_stone->Delete();
	for (counter=1;counter<MAXGUILDS; ++counter)
	{
		if (guilds[counter].free==0)
		{
			for (war=1;war<MAXGUILDWARS; ++war)
			{
				if (guilds[counter].war[war]==guildnumber)
				{
					guilds[counter].war[war]=0;
					guilds[counter].wars--;
				}
			}
		}
	}
}

static void RemoveShields(P_CHAR pc)
{
	//cwmWorldState->RemoveItemsFromCharBody(pc,0x1B, 0xC3);
	//cwmWorldState->RemoveItemsFromCharBody(pc,0x1B, 0xC4);
	P_ITEM pPack= pc->getBackpack();
	if (pPack)
	{
		pPack->DeleteAmount(666,0x1BC3);	// hope they don't have more than 666 shields ;-) (Duke)
		pPack->DeleteAmount(666,0x1BC4);
	}
}

// guilderasemember() Wipes all guild related data from a player

void cGuilds::EraseMember(int c)
{
	P_CHAR pc = MAKE_CHAR_REF(c);
	VALIDATEPC(pc);

	int guildnumber = pc->GetGuildNumber();

	if (guildnumber>=0 && guildnumber <MAXGUILDS)
	{
		int j,member;
		for(j=0;j<=guilds[guildnumber].members; ++j)
		{
			if (guilds[guildnumber].member[j] == pc->getSerial32())
			{
				P_CHAR hold = pointers::findCharBySerial(guilds[guildnumber].member[j]);
				if (ISVALIDPC(hold))
				{
					RemoveShields(hold);
				}
			}
		}

		for (member=1;member<MAXGUILDMEMBERS; ++member)
		{
			if (guilds[guildnumber].member[member] == pc->getSerial32())
			{
				guilds[guildnumber].member[member] = 0;
				guilds[guildnumber].members--;
				pc->SetGuildNumber(0);
				pc->ResetGuildTitleToggle( );
				pc->SetGuildFealty( 0 );
				pc->SetGuildTitle("");
				return;
			}
		}
	}
}

// guildtoggleabbreviation() Toggles the settings for showing or not showing the guild title
// Informs player about his change

void cGuilds::ToggleAbbreviation(int s)
{
	P_CHAR pc = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	int guildnumber = pc->GetGuildNumber();

	if (guildnumber<0 || guildnumber>MAXGUILDS)
	{
		pc->sysmsg(TRANSLATE("you are in no guild"));
		return;
	}

	if (guilds[guildnumber].type!=0)							// Check for Order/Chaos
	{
		pc->sysmsg(TRANSLATE("You are in an Order/Chaos guild, you cannot toggle your title."));
		// They may not toggle it off!
	}
	else
	{
		if (!pc->HasGuildTitleToggle())						// If set to Off then
		{
			pc->SetGuildTitleToggle();					// Turn it On
			pc->sysmsg(TRANSLATE("You toggled your abbreviation on."));	// Tell player about the change
		}
		else									// Otherwise
		{
			pc->ResetGuildTitleToggle();					// Turn if Off
			pc->sysmsg(TRANSLATE("You toggled your abbreviation off."));	// And tell him also
		}
	}
	Guilds->Menu(s,1);								// Send him back to the menu
}

// guildrecruit() Let the guild members recruit some player into the guild.
// Checks the guild database if "to be recruited" player already in any other guild.
// puts a tag with players serial number into the guilds recruit database.

void cGuilds::Recruit(int s)
{
	P_CHAR Me=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(Me);

	int slot, dummy;
	int guildnumber = SearchByStone(s);

	if (guildnumber==-1)
	{
		Me->sysmsg(TRANSLATE("you are in no guild"));
		return;
	}

	if( LongFromCharPtr(buffer[s] +11) == INVALID ) return; // check if user canceled operation - Morrolan
//	int serial = calcserial(buffer[s][7],buffer[s][8],buffer[s][9],buffer[s][10]);
//	P_CHAR pc = pointers::findCharBySerial( serial );
	P_CHAR pc = pointers::findCharBySerPtr(buffer[s]+7);

	if(pc != NULL)
	{
			if (pc->GetGuildNumber()!=0)
				Me->sysmsg(TRANSLATE("This person is already in a guild."));
			else
			{
				if (!pc->npc)
				{
					slot = SearchSlot(guildnumber, 3);
					for ( dummy = 1; dummy < MAXGUILDRECRUITS; ++dummy )
					{
						if ( guilds[guildnumber].recruit[dummy] == pc->getSerial32() ) 
							slot = 0;
					}
					if ((slot!=-1)&&(slot!=0))
					{
						++guilds[guildnumber].recruits;
						guilds[guildnumber].recruit[slot] = pc->getSerial32();
					}
					else
					{
						if (slot==-1)
							Me->sysmsg(TRANSLATE("No more recruit slots free."));
						if (slot==0)
							Me->sysmsg(TRANSLATE("This being is already a candidate."));
					}
				}
				else
					Me->sysmsg(TRANSLATE("This is not a player."));
			}
	}
	Guilds->Menu(s,1);
}

// guildtargetwar() Let us target some player and add his guild to the warlist
// Checks the guild database if "to be wared" guild is already in warlist.
// puts a tag with guilds number into the guilds war database.

void cGuilds::TargetWar(int s)
{
	P_CHAR Me=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(Me);

	int slot, dummy;
	int guildnumber = Guilds->SearchByStone(s);
	char text [200];
	if (guildnumber==-1)
	{
		Me->sysmsg(TRANSLATE("you are in no guild"));
		return;
	}

	if( LongFromCharPtr(buffer[s] +11) == INVALID )
		return; // check if user canceled operation - Morrolan
//	SERIAL serial=calcserial(buffer[s][7],buffer[s][8],buffer[s][9],buffer[s][10]);
//	P_CHAR pc = pointers::findCharBySerial( serial );
	P_CHAR pc = pointers::findCharBySerPtr(buffer[s] +7);
	if( pc != NULL)
	{
			if (pc->GetGuildNumber()==0)
				Me->sysmsg(TRANSLATE("This person is not in a guild."));
			else if (pc->GetGuildNumber()==guildnumber)
				Me->sysmsg(TRANSLATE("War yourself? Nah."));
			else
			{
				if (!pc->npc)
				{
					slot = Guilds->SearchSlot( guildnumber, 4 );
					for ( dummy = 1; dummy < MAXGUILDWARS; ++dummy )
					{
						if ( guilds[guildnumber].war[dummy] == pc->GetGuildNumber() ) slot = 0;
					}
					if ((slot!=-1)&&(slot!=0))
					{
						guilds[guildnumber].wars++;
						guilds[guildnumber].war[slot] = pc->GetGuildNumber();
						sprintf(text,TRANSLATE("%s declared war to %s"),guilds[guildnumber].name,guilds[pc->GetGuildNumber()].name);
						Guilds->Broadcast(guildnumber,text);
					}
					else
					{
						if (slot==-1) Me->sysmsg(TRANSLATE("No more war slots free."));
						if (slot==0) Me->sysmsg(TRANSLATE("This guild is already in our warlist."));
					}
				}
				else Me->sysmsg(TRANSLATE("This is not a player."));
			}
	}
	Guilds->Menu(s,2);
}

// guildstonemove() : Let the guildmaster move the stone anytime
// removes the stone item, gives Guildmaster a teleporterstone
// guild will be still active
// teleporterstone id 0x1869

void cGuilds::StoneMove(int s)
{
	if ( s < 0 || s >= now )
		return;
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	int guildnumber=Guilds->SearchByStone(s);
	if (guildnumber==-1) return;
	P_ITEM stone = pointers::findItemBySerial( guilds[guildnumber].stone );
	VALIDATEPI(stone);
	// Get stone

	P_ITEM pNewstone;		// For the new stone
	char stonename[80];	// And for its name

	// Give it a name
	sprintf(stonename,"a guildstone teleporter for %s",guilds[guildnumber].name);

	// Spawn the stone in the masters backpack
	pNewstone = item::CreateFromScript( "$item_hardcoded", pc->getBackpack() );

	if (!ISVALIDPI(pNewstone)) return; //AntiChrist
	pNewstone->setCurrentName( stonename );
	pNewstone->setId( 0x1869 );

	pNewstone->type = ITYPE_GUILDSTONE;			// Set Guildstone to Type 'Guild Related'
	guilds[guildnumber].stone=pNewstone->getSerial32();	// Remember its serial number
	stone->Delete();				// Remove the guildstone
	pc->sysmsg(TRANSLATE("Take care of that stone!"));	// And tell him also
}

// guildsearch() Okay this is for highlighting/guards and other stuff, so you know what relation
// player 1 and player 2 have. results are:
// 1= both in same guild (so fighting is okay, green hightlighting)
// 2= both in opposite guilds/guildtypes (so fighting is okay, orange highlighting)
// 0= no guildwarfare, or no guild relation (so no fighting, normal highlighting)
// Oh, Order/Order or Chaos/Chaos guilds (in different guilds) may not war eachother!

int cGuilds::Compare(P_CHAR player1,P_CHAR player2)
{
	VALIDATEPCR(player1,0);
	VALIDATEPCR(player2,0);
	int counter;
	int guildnumber=player1->GetGuildNumber();
	int guildnumber2=player2->GetGuildNumber();

	// one of both not in a guild -> no guildwarfare

	if (guildnumber<0 || guildnumber>=MAXGUILDS || guildnumber2<0 || guildnumber2>=MAXGUILDS) return 0;

	if ((guildnumber!=0)&&(guildnumber2!=0))
	{
		if (guildnumber==guildnumber2)
			return 1;

		if (((guilds[guildnumber].type==1)&&(guilds[guildnumber2].type==2))|| ((guilds[guildnumber].type==2)&&(guilds[guildnumber2].type==1)))
			return 2;

		for (counter=1;counter<MAXGUILDWARS; ++counter)
			if (guilds[guildnumber].war[counter]==guildnumber2)
				for (counter=1;counter<MAXGUILDWARS; ++counter)
					if (guilds[guildnumber2].war[counter]==guildnumber)
						return 2;
	}
	return 0;
}


// guildgumpinput() result branching for those niffy little gump text fields
// Called by: gumpinput()

void cGuilds::GumpInput(int s, int type, int index, char *text)
{
	if (type==100)
	{
		switch (index)
		{
		case 1: // Guild name requester
			ChangeName(s,text);
			return;
		case 2: // Guild abbreviation requester
			ChangeAbbreviation(s,text);
			return;
		case 3: // Guild master title requester
			ChangeTitle(s,text);
			return;
		case 4: // Guild member title requester
			ChangeTitle(s,text);
			return;
		case 5: // Guild charter requester
			ChangeCharter(s,text);
			return;
		case 6: // Guild webpage requester
			ChangeWebpage(s,text);
			return;
		}
	}
}

// guildgumpchoice() branches the results of those checkbox gumps
// Called by: choice()

void cGuilds::GumpChoice(NXWSOCKET socket, int main, int sub)
{
	P_CHAR pc = MAKE_CHAR_REF( currchar[socket] );
	VALIDATEPC( pc );
	
	int member, recruit, war, guild, counter, slot;
	//int members[MAXGUILDMEMBERS];
	//int recruits[MAXGUILDRECRUITS];
	char text[200];
	int guildnumber=Guilds->SearchByStone(socket);

	if (guildnumber == INVALID )
		return ;

	switch(main)
	{
	case 8000:
	case 8001:// main menu
		switch( sub )
		{
		case 1: { /*
			P_TARGET targ = clientInfo[socket]->newTarget( new cCharTarget() );
			targ->code_callback=Guilds->Recruit(socket);
			targ->send( getClientFromSocket(s) );
			sysmessage( socket, TRANSLATE("Select person to invite into the guild."));
			*/
			}
			break;
		case 2:	Menu(socket,7);
			break;
		case 3: Menu(socket,5);
			break;
		case 4: Menu(socket,13);
			break;
		case 5: ToggleAbbreviation(socket);
			break;
		case 6: Resign( MAKE_CHAR_REF( currchar[socket] ), socket );
			break;
		case 7: Menu(socket,6);
			break;
		default:
			if( sub == 8 && main == 8000 )
				Menu( socket, 2 );
			else
				if( ( sub == 8 && main == 8001 ) || ( sub == 9 && main == 8000 ) )
					Menu( socket, 11 );
				else
					if( ( sub == 9 && main == 8001 ) || ( sub == 10 && main == 8000 ) )
						Menu( socket, 16 );
			break;
		}
		return;
	case 8002:// guildmaster menu
		switch(sub)
		{
		case 1: 
			//entrygump(socket,s1,s2,s3,s4,100,1,40,TRANSLATE("Enter a new guildname."));
			break;
		case 2: 
			//entrygump(socket,s1,s2,s3,s4,100,2,3,TRANSLATE("Enter a new guild abbreviation."));
			break;
		case 3: 
			Guilds->Menu(socket,3);
			break;
		case 4: 
			Guilds->Menu(socket,4);
			break;
		case 5: 
			Guilds->Menu(socket,8);
			break;
		case 6: 
			Guilds->Menu(socket,14);
			break;
		case 7: {/*
			P_TARGET targ = clientInfo[socket]->newTarget( new cCharTarget() );
			targ->code_callback=Guilds->TargetWar(s);
			targ->send( getClientFromSocket(s) );
			sysmessage( socket, TRANSLATE("Select person to declare war to."));*/
			}
			break;
		case 8: 
			Guilds->Menu(socket,15);
			break;
		case 9:	
			Guilds->Menu(socket,10);
			break;
		case 10:
			Guilds->Menu(socket,9);
			break;
		case 11:
			//entrygump(socket, s1, s2, s3, s4, 100, 3, 20, TRANSLATE("Enter new guildmastertitle."));
			break;
		case 12: 
			Guilds->Menu(socket,12);
			break;
		case 13:
			Guilds->StoneMove(socket);
			break;
		case 14:
			Guilds->Menu(socket,1);
			break;
		}
		return;
	case 8003:// set type menu
		if (sub >= 2 && sub <=4)
			Guilds->SetType(guildnumber, sub-2);
		Guilds->Menu(socket,2);
		return;
	case 8004:
		// edit charter menu
		switch( sub )
		{
		case 1: Menu( socket, 2 );
			break;
		case 2: //entrygump( socket, s1, s2, s3, s4, 100, 5, 50, TRANSLATE("Enter a new charter.") );
			break;
		case 3:	//entrygump( socket, s1, s2, s3, s4, 100, 6, 50, TRANSLATE("Enter a new URL.") );
			break;
		}
		return;
	case 8005:
		// view charter menu
		if (sub==1) 
			Guilds->Menu(socket,1);
		if (sub==2)
			weblaunch(socket,guilds[guildnumber].webpage);
		return;
	case 8006:// candidates menu
		Guilds->Menu(socket,1);
		return;
	case 8007:// roster menu
		Guilds->Menu(socket,1);
		return;
	case 8008:// dismiss menu
		counter=1;
		for (member=1;member<MAXGUILDMEMBERS;member++)
		{
			if (guilds[guildnumber].member[member]!=0)
			{
				counter++;
				if (sub==counter)
				{
					if (guilds[guildnumber].member[member]==pc->getSerial32())
					{
						pc->sysmsg(TRANSLATE("You cannot dimiss yourself, please resign from the guild if you wish."));
					}
					else
					{
						P_CHAR pc_member = pointers::findCharBySerial( guilds[guildnumber].member[member] );
						if( ISVALIDPC( pc_member ) )
						{
							pc->sysmsg(TRANSLATE("Kicked %s out of the guild."), pc_member->getCurrentNameC());
							if( pc_member->IsOnline() )
								pc_member->sysmsg(TRANSLATE("You got kicked out of your guild."));
						}
						else
						{
							pc->sysmsg(TRANSLATE("Invalid member %d"), guilds[guildnumber].member[member] );
							ErrOut(TRANSLATE("Invalid member %d"), guilds[guildnumber].member[member]);
						}
						Guilds->EraseMember(member);
					}
					break;
				}
			}
		}
		Guilds->Menu(socket,2);
		return;
	case 8009:// refuse menu
		counter=1;
		for (recruit=1;recruit<MAXGUILDRECRUITS;recruit++)
		{
			if (guilds[guildnumber].recruit[recruit]!=0)
			{
				++counter;
				if (sub==counter)
				{
					P_CHAR pc_recruit = pointers::findCharBySerial( guilds[guildnumber].recruit[recruit] );
					if( ISVALIDPC( pc_recruit ) )
						pc->sysmsg(TRANSLATE("Removed candidate %s from the list."), pc_recruit->getCurrentNameC());
					else
					{
						pc->sysmsg(TRANSLATE("Invalid candidate %d\n"), guilds[guildnumber].recruit[recruit] );
						ErrOut(TRANSLATE("Invalid candidate %d\n"), guilds[guildnumber].recruit[recruit] );
					}
					guilds[guildnumber].recruit[recruit]=0;
					guilds[guildnumber].recruits--;
				}
			}
		}
		Guilds->Menu(socket,2);
		return;
	case 8010:// accept
		counter=1;
		for (recruit=1;recruit<MAXGUILDRECRUITS;recruit++)
		{
			if (guilds[guildnumber].recruit[recruit]!=0)
			{
				counter++;
				if (sub==counter)
				{
					P_CHAR pc_recruit = pointers::findCharBySerial( guilds[guildnumber].recruit[recruit] );
					if ( ISVALIDPC(pc_recruit) )
						if (pc_recruit->GetGuildNumber()==0)
						{
							slot = Guilds->SearchSlot(guildnumber,2);
							if (slot != INVALID)
							{
								guilds[guildnumber].member[slot] = pc_recruit->getSerial32();
								guilds[guildnumber].members++;
								pc_recruit->SetGuildNumber( guildnumber );
								guilds[guildnumber].recruit[recruit] = 0;
								guilds[guildnumber].recruits--;
								if (guilds[guildnumber].type!=0) 
									pc->SetGuildTitleToggle();
								pc->sysmsg(TRANSLATE("Candidate %s is now a guildmember."), pc_recruit->getCurrentNameC());
								if ( guilds[guildnumber].type == 1 )
									item::CreateFromScript( "$item_order_shield", pc_recruit->getBackpack() );
							    if ( guilds[guildnumber].type == 2 )
									item::CreateFromScript( "$item_chaos_shield", pc_recruit->getBackpack() );
							}
							else 
								pc->sysmsg(TRANSLATE("This guild is full, maximum amount of members reached!") );
						}
						else
						{
							pc->sysmsg(TRANSLATE("Candidate %s is already in another guild, I'll remove him from your list now."), pc_recruit->getCurrentNameC());
							if( pc_recruit->IsOnline() )
									pc_recruit->sysmsg(TRANSLATE("You've been removed as candidate from guild %s as you are in another guild"),
									guilds[guildnumber].name );
							guilds[guildnumber].recruit[recruit] = 0;
							guilds[guildnumber].recruits--;
						}
				}
			}
		}
		Guilds->Menu(socket,2);
		return;
	case 8011:	// warlist menu
		Guilds->Menu(socket,1);
		return;
	case 8012:	// grant title menu
		if (sub==1) 
			Guilds->Menu(socket,2);
		counter=1;
		for (member=1;member<MAXGUILDMEMBERS;member++)
		{
			if (guilds[guildnumber].member[member]!=0)
			{
				counter++;
				if (sub==counter)
				{
					guilds[guildnumber].priv = calcCharFromSer( guilds[guildnumber].member[member] );
					//entrygump(socket, s1, s2, s3, s4, 100, 3, 20, "Enter new guildtitle.");
					return;
				}
			}
		}
		return;
	case 8013:	// fealty menu
		counter=1;
		for (member=1;member<MAXGUILDMEMBERS;member++)
		{
			if (guilds[guildnumber].member[member]!=0)
			{
				++counter;
				if (sub==counter)
					pc->SetGuildFealty( guilds[guildnumber].member[member] );
			}
		}
		Guilds->Menu(socket,1);
		return;
	case 8014:	// declare war menu
		counter=1;
		for (guild=1;guild<MAXGUILDS;guild++)
		{
			if ((strcmp(guilds[guildnumber].name,guilds[guild].name)) && (guilds[guild].free!=1))
			{
				++counter;
				if (sub==counter)
				{
					slot=Guilds->SearchSlot(guildnumber,4);
					for (war=1;war<MAXGUILDWARS;++war)
						if (guilds[guildnumber].war[war]==guild)
							slot=0;

					if ((slot!=-1)&&(slot!=0))
					{
						++guilds[guildnumber].wars;
						guilds[guildnumber].war[slot]=guild;
						sprintf(text,TRANSLATE("%s declared war to %s"),guilds[guildnumber].name,guilds[guild].name);
						Guilds->Broadcast(guildnumber,text);
					}
					else
					{
						if (slot==-1) 
							pc->sysmsg(TRANSLATE("No more war slots free."));
						else 
							if (slot == 0) 
								pc->sysmsg(TRANSLATE("This guild is already in our warlist."));
					}
				}
			}
		}
		Guilds->Menu(socket,2);
		return;
	case 8015:	// declare peace menu
		counter=1;
		for (war=1;war<MAXGUILDWARS;war++)
		{
			if (guilds[guildnumber].war[war]!=0)
			{
				++counter;
				if (sub==counter)
				{
					sprintf(text,TRANSLATE("%s declared peace to %s"),guilds[guildnumber].name,guilds[guilds[guildnumber].war[war]].name);
					Guilds->Broadcast(guildnumber,text);
					guilds[guildnumber].war[war]=0;
					guilds[guildnumber].wars--;
				}
			}
		}
		Guilds->Menu(socket,2);
		return;
	case 8016:	// warlist menu 2
		Guilds->Menu(socket,1);
		return;
	}
}

// TESTED: OKAY
// guildnamechange(character, text) copies the text info the guildstones name field
// (guildnumber gets calculated from the double clicked guildstones), and notifies all online
// guildmambers about the change.

void cGuilds::ChangeName(NXWSOCKET s, char *text)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	int guildnumber=Guilds->SearchByStone(s);

	if (guildnumber==-1) 
		return;
	
	P_ITEM pStone = pointers::findItemBySerial( guilds[guildnumber].stone );

	if (!ISVALIDPI(pStone)) 
		return;

	int guild;
	bool exists = false;
	char txt[200];

	for (guild=1;guild<MAXGUILDS;guild++)
		if (!(strcmp(text,guilds[guild].name)))
		{
			exists = true;
			break;
		}

	if (!exists)
	{
		char tmp[200];

		strcpy(guilds[guildnumber].name, text);
		sprintf(tmp,TRANSLATE("Guildstone for %s"),guilds[guildnumber].name);
		pStone->setCurrentName(tmp);
		sprintf(txt,TRANSLATE("Your guild got renamed to %s"),guilds[guildnumber].name);
		Guilds->Broadcast(guildnumber,txt);
	}
	else 
		pc->sysmsg(TRANSLATE("This name is already taken by another guild."));
}

// TESTED: OKAY
// guildabbreviationchange(character, text) copies the text info the guildstones abbreviation field
// (guildnumber gets calculated from the double clicked guildstones), and notifies editing player about
// the change.

void cGuilds::ChangeAbbreviation(int s, char *text)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	int guildnumber=Guilds->SearchByStone(s);

	if (guildnumber==-1) 
		return;

	int guild;
	bool exists = false;
	char txt[200];

	for ( guild = 1; guild < MAXGUILDS; guild++ )
		if (!(strcmp(text,guilds[guild].abbreviation))) 
		{
			exists = true;
			break;
		}

	if (!exists)
	{
		strcpy(guilds[guildnumber].abbreviation,text);
		sprintf(txt,TRANSLATE("Your guild has now the abbreviation: %s"),guilds[guildnumber].abbreviation);
		Guilds->Broadcast(guildnumber,txt);
	}
	else 
		pc->sysmsg(TRANSLATE("This abbreviation is already taken by another guild."));
	Guilds->Menu(s,2);
}

// guildtitlechange(character, text) copies the text info the characters title field (guildnumber
// gets calculated from the double clicked guildstones), changed character was buffered in the guilds
// private field (as backup buffer) and notifies editing player about the change.
void cGuilds::ChangeTitle(int s, char *text)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	int guildnumber=Guilds->SearchByStone(s);
	
	if (guildnumber==-1) 
		return;

	int member=guilds[guildnumber].priv;

	if (member==0) 
		member=s;

	guilds[guildnumber].priv=0;

	P_CHAR membr=pointers::findCharBySerial(member);
	VALIDATEPC(membr);
	membr->SetGuildTitle( text );

	if (member==s) 
		pc->sysmsg(TRANSLATE("You changed your own title."));
	else 
		pc->sysmsg(TRANSLATE("You changed the title."));

	Guilds->Menu(s,2);
}

// TESTED: OKAY
// guildcharterchange(character, text) copies the text info the guildstones charter field (guildnumber
// gets calculated from the double clicked guildstones), and notifies editing player about the change.
void cGuilds::ChangeCharter(int s, char *text)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	int guildnumber=Guilds->SearchByStone(s);

	if (guildnumber==-1) 
		return;

	strcpy(guilds[guildnumber].charter, text);
	pc->sysmsg(TRANSLATE("You changed the guilds charter."));
	Guilds->Menu(s,2);
}

// TESTED: OKAY
// guildwebpagechange(character, text) copies the text info the guildstones webpage field (guildnumber
// gets calculated from the double clicked guildstones), and notifies editing player about the change.
void cGuilds::ChangeWebpage(int s, char *text)
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	int guildnumber = Guilds->SearchByStone(s);

	if (guildnumber==-1)
		return;

	strcpy(guilds[guildnumber].webpage, text);
	pc->sysmsg(TRANSLATE("You changed the guilds webpage url."));
	Guilds->Menu(s,2);
}



int cGuilds::GetType(int guildnumber)
{

	if (guildnumber<0 || guildnumber >=MAXGUILDS)
		return 0;
	else
		return guilds[guildnumber].type;
}



void cGuilds::SetType(int guildnumber, int type)
{
	int j;
	int member;

	if (guildnumber<0 || guildnumber >=MAXGUILDS) return;

	guilds[guildnumber].type = type;

	// Redone by Ripper and LB - march 19, 2000
	//	I think the idea here is to either remove shields from ALL
	//	(order/chaos) guilds members and replace them with opposite
	//	sheild types, or remove them alltogether

	switch(type)
	{
		case 1:	//	Convert into a order guild
			for(j=0;j<=guilds[guildnumber].members;j++)
			{
				P_CHAR hold = pointers::findCharBySerial(guilds[guildnumber].member[j]);
				if (ISVALIDPC(hold))
				{
					RemoveShields(hold);
					item::CreateFromScript( "$item_order_shield", hold->getBackpack() );	// will not work for offline chars (Duke)
				}
			}
			Guilds->Broadcast( guildnumber, TRANSLATE("Your guild is now an Order guild.") );
			Guilds->Broadcast( guildnumber, TRANSLATE("An order shield has been placed in your pack!") );
			break;
		case 2:	//	Convert guild into an choas guild
			for(j=0;j<=guilds[guildnumber].members;j++)
			{
				P_CHAR hold = pointers::findCharBySerial(guilds[guildnumber].member[j]);
				if (ISVALIDPC(hold))
				{
					RemoveShields(hold);
					item::CreateFromScript( "$item_chaos_shield", hold->getBackpack() );	// will not work for offline chars (Duke)
				}
			}
			Guilds->Broadcast( guildnumber, TRANSLATE("Your guild is now a Chaos guild.") );
			Guilds->Broadcast( guildnumber, TRANSLATE("A chaos shield has been placed in your pack!") );
			break;
		case 0:	// Convert guild into a standard guild
		default:	//	Just let it fall through
			//	Idea is to remove the items from this guilds members(Im gonna use your item loop for clarity)
			for(j = 0;j <= guilds[guildnumber].members; j++)
			{
				P_CHAR hold = pointers::findCharBySerial(guilds[guildnumber].member[j]);
				if (ISVALIDPC(hold))
				{
					RemoveShields(hold);
				}
			}
			Guilds->Broadcast( guildnumber, TRANSLATE("Your guild is now a Standard guild.") );
			break;
	}
	// Im goging to leave this here cause I am not sure what it does.
	if (type!=0)
	{
		for (member=1;member<MAXGUILDMEMBERS;member++)
		{
			if (guilds[guildnumber].member[member]!=0)
			{
				P_CHAR pj = pointers::findCharBySerial(guilds[guildnumber].member[member]);
				if (ISVALIDPC(pj))
					pj->SetGuildTitleToggle();
			}
		}
	}
}

// TESTED: OKAY
// Uhm.. do i need char or socket for sysmessage? hehe.. a socket ofcourse, FiXED iT!
// guildbroadcast(guildnumber, text) broadcasts message to all online members of guild
void cGuilds::Broadcast(int guildnumber, char *text)
{
	int member;

	if (guildnumber==-1) return;

	for (member=1;member<MAXGUILDMEMBERS;member++)
	{
		if (guilds[guildnumber].member[member]!=0)
		{
			P_CHAR pc = pointers::findCharBySerial(guilds[guildnumber].member[member]);
			if( pc->IsOnline() )
				pc->sysmsg(text);
		}
	}
}

// TESTED: OKAY (but not the best solution ;))
// guildSearchByStone(guildstone serial) get the guildnumber from the stones serial number
int cGuilds::SearchByStone(int s)
{
	int guildnumber;

	P_CHAR pc = MAKE_CHAR_REF(currchar[s]);
	VALIDATEPCR(pc, INVALID );

	P_ITEM pStone=MAKE_ITEM_REF(pc->fx1);
	if(!ISVALIDPI(pStone))
		return -1;

	int stone= pStone->getSerial32();


	if (pc->IsGM())
	{
		for ( guildnumber = 1; guildnumber < MAXGUILDS; guildnumber++)
		{
			if ( guilds[guildnumber].stone == stone )
			{
				return guildnumber;
			}
		}
		return pc->GetGuildNumber();
	}
	else
	{
		return pc->GetGuildNumber();
	}
}

// TESTED: OKAY
// guildsearchslot(guildnumber, search type) searches for a free slot of the 4 possible types, results
// the free slot or -1 of no free slot available
// Types  1:guildslots, 2:memberslots, 3: recruitslots, 4: warslots
int cGuilds::SearchSlot(int guildnumber, int type)
{
	register int counter=1;
	switch(type)
	{
	case 1:												// guildslots
		for (; counter<MAXGUILDS; counter++)
			if (guilds[counter].free==1) return counter;
		if (guildnumber<0 || guildnumber>=MAXGUILDS) return -1;
		break;
	case 2:												// memberslots
		for (; counter<MAXGUILDMEMBERS; counter++)
			if (guilds[guildnumber].member[counter]==0) return counter;
		break;
	case 3:												// recruitslots
		for (; counter<MAXGUILDRECRUITS; counter++)
			if (guilds[guildnumber].recruit[counter]==0) return counter;
		break;
	case 4:												// warslots
		for (; counter<MAXGUILDWARS; counter++)
			if (guilds[guildnumber].war[counter]==0) return counter;
		break;
	}
	return -1;
}

// TESTED: OKAY (i think so, but ugly -> redo it!)
// guildmastercalc(guildnumber) counts all fealty settings of all members of 'guildnumber' and sets
// a new guildmaster if there is a draw then there will be no master, til the next check ;)
void cGuilds::CalcMaster(int guildnumber)
{
	if (guildnumber<0 || guildnumber>=MAXGUILDS)
		return;

	struct guild_st *guild = &guilds[guildnumber];

	int member, member2;
	int guildmember[MAXGUILDMEMBERS]={0};
	int currentfealty, currenthighest=0, draw=0;

	for ( member = 1; member < MAXGUILDMEMBERS; ++member )
	{
		P_CHAR pCurrentMember = pointers::findCharBySerial( guild->member[member] );

		if( ISVALIDPC( pCurrentMember) )
			currentfealty = pCurrentMember->GetGuildFealty();
		else
			currentfealty = 0;

		for ( member2 = 1; member2 < MAXGUILDMEMBERS; ++member2 )
		{
			if ( guild->member[member2] == currentfealty && currentfealty != 0 )
				++guildmember[member2];
		}
	}

	for (member=1;member<MAXGUILDMEMBERS;++member)
	{
		if (guildmember[member]>guildmember[currenthighest])
		{
			draw=0;
			currenthighest=member;
		}
		else
			if (guildmember[member]==guildmember[currenthighest])
				draw=1;
	}

	if (draw==0)
		guild->master = guild->member[currenthighest];

	return;
}

cGuilds::~cGuilds()
{
}

//! initializes the guild structure fields, marks them all as free.
cGuilds::cGuilds()
{
	int guildnumber, member, recruit, war;

	for (guildnumber=1;guildnumber<MAXGUILDS;guildnumber++)
	{
		guilds[guildnumber].free=1;
		guilds[guildnumber].name[0] = 0;
		guilds[guildnumber].abbreviation[0] = 0;
		guilds[guildnumber].type=0;
		guilds[guildnumber].charter[0] = 0;
		guilds[guildnumber].webpage[0] = 0;
		guilds[guildnumber].stone=0;
		guilds[guildnumber].master=0;
		guilds[guildnumber].members=0;

		for (member=1;member<MAXGUILDMEMBERS;member++)
		{
			guilds[guildnumber].member[member]=0;
		}

		guilds[guildnumber].recruits=0;

		for (recruit=1;recruit<MAXGUILDRECRUITS;recruit++)
		{
			guilds[guildnumber].recruit[recruit]=0;
		}

		guilds[guildnumber].wars=0;

		for (war=1;war<MAXGUILDWARS;war++)
		{
			guilds[guildnumber].war[war]=0;
		}

		guilds[guildnumber].priv=0;
	}
}


// TESTED: OKAY (fine fine.. now proove that it really works.. )
// guildtitle(viewing character socket, clicked character) displays players title string, over the name
// of clicked character, name color gets calculated from the guild relationship of both players
// Called by: textflags()

void cGuilds::Title(int s,int player2)
{
	char title[150];
	char abbreviation[5];
	char guildtype[10];

	P_CHAR pc2= pointers::findCharBySerial( player2 );
	VALIDATEPC(pc2);

	if (pc2->GetGuildNumber()<0 || pc2->GetGuildNumber()>=MAXGUILDS) return;

	if ((pc2->GetGuildNumber()!=0)&&(pc2->HasGuildTitleToggle()))
	{
		strcpy(abbreviation,guilds[pc2->GetGuildNumber()].abbreviation);
		if (!(strcmp(abbreviation,"")))
			strcpy(abbreviation,"none");

		if (guilds[pc2->GetGuildNumber()].type!=0)
		{
			if (guilds[pc2->GetGuildNumber()].type==1) strcpy(guildtype,"Order");
			else if (guilds[pc2->GetGuildNumber()].type==2) strcpy(guildtype,"Chaos");

			if (strcmp(pc2->GetGuildTitle(),"")) sprintf(title,"[%s, %s] [%s]",pc2->GetGuildTitle(),abbreviation,guildtype);
			else sprintf(title,"[%s] [%s]",abbreviation, guildtype);
		}
		else
		{
			if (strcmp(pc2->GetGuildTitle(),"")) sprintf(title,"[%s, %s]",pc2->GetGuildTitle(),abbreviation);
			else sprintf(title,"[%s]",abbreviation);
		}

		UI08 sysname[30]={ 0x00, };
		strcpy((char *)sysname, "System");

		SendSpeechMessagePkt(s, pc2->getSerial32(), 0x0101, 0, pc2->emotecolor, 0x0003, sysname, title);
	}
}

// TESTED: OKAY
// guildread(character) reads the guild structure from the wsc file, copies it to the 'character' structure.
// Called by: loadnewworld()
void cGuilds::Read(int guildnumber)
{
	int war=1, member=1, recruit=1,loopexit=0;

	if (guildnumber<0 || guildnumber>=MAXGUILDS)
	{
		std::string errString("invalid guild infmormation entry in " + SrvParms->savePath + SrvParms->guildWorldfile + SrvParms->worldfileExtension );
		LogError( const_cast<char*>(errString.c_str()) );
		return;
	}
	struct guild_st *guild = &guilds[guildnumber];
	do
	{
		readw2();
		if (!strcmp(script1, "NAME")) strcpy(guild->name, script2);
		else if (!strcmp(script1, "ABBREVIATION")) strcpy(guild->abbreviation, script2);
		else if (!strcmp(script1, "TYPE")) guild->type = str2num(script2);
		else if (!strcmp(script1, "CHARTER")) strcpy(guild->charter, script2);
		else if (!strcmp(script1, "WEBPAGE")) strcpy(guild->webpage, script2);
		else if (!strcmp(script1, "STONE")) guild->stone = str2num(script2);
		else if (!strcmp(script1, "MASTER")) guild->master = str2num(script2);
		else if (!strcmp(script1, "RECRUITS")) guild->recruits = str2num(script2);
		else if (!strcmp(script1, "RECRUIT")) { if (recruit<MAXGUILDRECRUITS) { guild->recruit[recruit]=str2num(script2);recruit++; } }
		else if (!strcmp(script1, "MEMBERS")) guild->members=str2num(script2);
		else if (!strcmp(script1, "MEMBER")) { if (member<MAXGUILDMEMBERS) { guild->member[member]=str2num(script2);member++;} }
		else if (!strcmp(script1, "WARS")) guild->wars = str2num(script2);
		else if (!strcmp(script1, "WAR")) { if (war<MAXGUILDWARS) { guild->war[war]=str2num(script2);war++;} }
		guilds[guildnumber].free=0;
	}
	while ( (strcmp(script1,"}")) && (++loopexit < MAXLOOPS) );
}

// TESTED: OKAY
// guildwrite() writes the guild structure to the wsc file.
// Called by: worldmain()
void cGuilds::Write(FILE *wscfile)
{
	int guildnumber, member, recruit, war;

	for (guildnumber=1;guildnumber<MAXGUILDS;guildnumber++)
	{
		if (guilds[guildnumber].free==0)
		{
			fprintf(wscfile,"SECTION GUILD %i\n", guildnumber);
			fprintf(wscfile,"{\n");
			fprintf(wscfile,"NAME %s\n", guilds[guildnumber].name);
			fprintf(wscfile,"ABBREVIATION %s\n", guilds[guildnumber].abbreviation);
			fprintf(wscfile,"TYPE %i\n", guilds[guildnumber].type);
			fprintf(wscfile,"CHARTER %s\n", guilds[guildnumber].charter);
			fprintf(wscfile,"WEBPAGE %s\n", guilds[guildnumber].webpage);
			fprintf(wscfile,"STONE %i\n", guilds[guildnumber].stone);
			fprintf(wscfile,"MASTER %i\n", guilds[guildnumber].master);
			fprintf(wscfile,"MEMBERS %i\n", guilds[guildnumber].members);
			
			for (member=1;member<MAXGUILDMEMBERS;member++)
			{
				if (guilds[guildnumber].member[member]!=0)
				{
					fprintf(wscfile,"MEMBER %i\n", guilds[guildnumber].member[member]);
				}
			}
			
			fprintf(wscfile,"RECRUITS %i\n", guilds[guildnumber].recruits);

			for (recruit=1;recruit<MAXGUILDRECRUITS;recruit++)
			{
				if (guilds[guildnumber].recruit[recruit]!=0)
				{
					fprintf(wscfile,"RECRUIT %i\n", guilds[guildnumber].recruit[recruit]);
				}
			}

			fprintf(wscfile,"WARS %i\n", guilds[guildnumber].wars);

			for (war=1;war<MAXGUILDWARS;war++)
			{
				if (guilds[guildnumber].war[war]!=0)
				{
					fprintf(wscfile,"WAR %i\n", guilds[guildnumber].war[war]);
				}
			}

			fprintf(wscfile,"}\n\n");
		}
	}
}

/*
int cGuilds::CheckValidPlace(int x,int y)
{
	// This is still in work.. please be patient..
	return 1;
}
*/

int cGuilds::CheckValidPlace(int s)
{
	int los;
	P_ITEM pi;
	P_ITEM pack;
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	P_ITEM house=findmulti( pc->getPosition() );
	VALIDATEPIR(house, 0);

	if (!house->IsHouse()) 
		return 0;
	
	pack= pc->getBackpack();

	if(ISVALIDPI(pack))
	{
		los=0;

		NxwItemWrapper si;
		si.fillItemsInContainer( pack, false );
		for( si.rewind(); !si.isEmpty(); si++ ) {
		
			pi=si.getItem();

			if (ISVALIDPI(pi))
				if (pi->type==ITYPE_KEY && calcserial(pi->more1,pi->more2,pi->more3,pi->more4)==house->getSerial32())
				{
					return 1;
					break;
				}
		}
	}

	pc->sysmsg(TRANSLATE("You must be close to a house and have a key in your pack to place that."));
	return 0;
}

/// LB 11'th NOV 2000
/// correkt incorrect guild settings
/// called at startup

void cGuilds::CheckConsistancy(void )
{
	int guildnumber,members,ok=1,error=0;

	P_ITEM stone;
	P_CHAR pc;

	ConOut("Checking guild data consistancy...");

	//////////// check the guilds

	for (guildnumber=1; guildnumber<MAXGUILDS; guildnumber++)
	{
		if (guilds[guildnumber].free==0)
		{
			// is the guildmaster still alive ?
			ok=1;
			pc = pointers::findCharBySerial(guilds[guildnumber].master);
			if (!pc) // if not, erase the guild !
			{
				ok=0;
				LogWarning("guild: %s ereased because guildmaster vanished",guilds[guildnumber].name);
				Guilds->EraseGuild(guildnumber);
			}
			// guildstone deleted ? yes -> erase guild !
			if (ok) // don't erease twice ;)
			{
				stone = pointers::findItemBySerial(guilds[guildnumber].stone);
				if (!stone)
				{
					ok=0;
					LogWarning("guild: %s ereased because guildstone vanished",guilds[guildnumber].name);
					Guilds->EraseGuild(guildnumber);
				}
			}
			if (ok)
			{
			 // check for guildmembers that don't exist anymore and remove from guild structure if so
				for (members=1; members<MAXGUILDMEMBERS; members++)
				{
					pc = pointers::findCharBySerial(guilds[guildnumber].member[members]);
					if (!ISVALIDPC( pc ) )
					{
						ok=0;
						LogWarning("guild: %s had an member that didnt exist anymore, removed\n",guilds[guildnumber].name);
						guilds[guildnumber].member[members]=0;
						guilds[guildnumber].members--;
					}
				}
			}
		}
		if (!ok) error=1;
	}

	if (error) ConOut("[DONE] - errors found, check logs\n"); else ConOut("[ OK ]\n");
}
