  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/


#include "nxwcommn.h"
#include "version.h"
#include "debug.h"
#include "accounts.h"
#include "scp_parser.h"
#include "globals.h"
#include "chars.h"
#include "items.h"
#include "inlines.h"
#include "basics.h"
#include "scripts.h"

void offlinehtml()//HTML
{
	char sect[512], hfile[512],time_str[80];
	unsigned int total,hr,min,sec,loopexit=0; //bugfix LB
	FILE *html;

	total=(uiCurrentTime-starttime)/MY_CLOCKS_PER_SEC;
	hr=total/3600;
	total-=hr*3600;
	min=total/60;
	total-=min*60;
	sec=total;

	cScpIterator* iter = NULL;
    char script1[1024];
    char script2[1024];

	strcpy(sect,"SECTION OFFLINE");

    iter = Scripts::HtmlStrm->getNewIterator(sect);
    if (iter==NULL) return;
	strcpy(script1, iter->getEntry()->getFullLine().c_str()); //discard  {

	strcpy(script1, iter->getEntry()->getFullLine().c_str());
	strcpy(hfile, script1);
	html = fopen(hfile, "w");
	if (html == NULL) // LB
	{
		WarnOut("Could not create html file, plz check html.xss\n");
		safedelete(iter);
		return;
	}

	do {
		iter->parseLine(script1, script2);
		if(!(strcmp(script1,"LINE"))) fprintf(html,"%s\n",script2);
		else if(!(strcmp(script1,"TIME"))) fprintf(html,RealTime(time_str));
		else if(!(strcmp(script1,"UPTIME"))) fprintf(html,"%i:%i:%i",hr,min,sec);
	} while( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	fclose(html);
	safedelete(iter);
}

void updatehtml()//HTML
{
	double eps=0.00000000001;
	char sect[512],time_str[80],hfile[512] /*,sh[3],sm[3],ss[3]*/;
	int a, n=0;
	//unsigned long int ip;
	int gm=0,cns=0,ccount=0,npccount=0,loopexit=0;
	unsigned long int total;
	unsigned int hr,min,sec;
	FILE *html;

	cScpIterator* iter = NULL;
	//char script1[1024];
	//char script2[1024];
	std::string script1;
	std::string script2;

	strcpy(sect,"SECTION ONLINE");

	iter = Scripts::HtmlStrm->getNewIterator(sect);
	if (iter==NULL)
		return;
	script1 = iter->getEntry()->getFullLine(); //discard  {

	script1 = iter->getEntry()->getFullLine();
	strcpy( hfile, script1.c_str() );

	//html=fopen(hfile,"w+");
	//a=remove(hfile);
	//ConOut("html-a: %i %s\n",a,hfile);

	html=fopen(hfile,"w");  // remove old one first

	if (html == NULL) // LB
	{
		WarnOut("Could not create html file, please check html.xss\n");
		safedelete(iter);
		return;
	}


	do {
		iter->parseLine(script1, script2);
		if( script1 == "LINE" )
		{
			fprintf(html,"%s\n",script2.c_str() );
		}
		else if( script1 == "TIME" )
		{
			fprintf(html,"%s <BR>",RealTime(time_str));
		}
		else if( script1 == "NOW" )
		{
			P_CHAR pc= MAKE_CHAR_REF(currchar[n]);
			//if(online(currchar[n])) //bugfix LB
			if( ISVALIDPC(pc) && pc->IsOnline() )
			{
				fprintf(html,pc->getCurrentNameC());
				n++;
			}
		}
		else if( script1 == "WHOLIST" )
		{
			a=0;
			for (n=0;n<now;n++)
			{
				P_CHAR pc= MAKE_CHAR_REF(currchar[n]);

				//if (online(currchar[n])) // bugfix, LB
				if( ISVALIDPC(pc) && pc->IsOnline() )
				{
					a++;
					fprintf(html,"%i) %s <BR>\n",a,pc->getCurrentNameC()); // bugfix lb
				}
			}
		}
		else if( script1 == "NOWNUM")
			fprintf(html,"%i",now);
		else if( script1 == "ACCOUNTNUM" )
			fprintf(html,"%i",Accounts->Count());
		else if( script1 == "CHARCOUNT" )
		{
			if(ccount==0)
			{
				npccount=0;
				/*for(a=0;a<charcount;a++)
				{
					P_CHAR pc_a=MAKE_CHAR_REF(a);
					if(ISVALIDPC(pc_a)) {
						if(!pc_a->free) ccount++;
						if(pc_a->npc && !pc_a->free) npccount++;
					}
				}*/
			}
			fprintf(html,"%i",ccount);
		}
		else if( script1 == "NPCS" )
		{
			if(npccount==0)
			{
				ccount=0;
				/*for(a=0;a<charcount;a++)
				{
					P_CHAR pc_a=MAKE_CHAR_REF(a);
					if(ISVALIDPC(pc_a)) {
						if(!pc_a->free) ccount++;
						if(pc_a->npc && !pc_a->free) npccount++; //bugfix LB
					}
				}*/
			}
			fprintf(html,"%i",npccount);
		}
		else if( script1 == "ITEMCOUNT" )
		{
			//fprintf(html,"%i",itemcount);
		}
		else if( script1 == "UPTIME" )
		{
			total=(uiCurrentTime-starttime)/MY_CLOCKS_PER_SEC;
			hr=total/3600;
//			if(hr<10 && hr<=60) sprintf(sh,"0%lu",hr);
//			else sprintf(sh,"%lu",hr);
			total-=hr*3600;
			min=total/60;
//			if(min<10 && min<=60) sprintf(sm,"0%lu",min);
//			else sprintf(sm,"%lu",min);
			total-=min*60;
			sec=total;
//			if(sec<10 && sec <=60) sprintf(ss,"0%lu",sec);
//			else sprintf(ss,"%lu",sec);
//			fprintf(html,"%s:%s:%s",sh,sm,ss);
			fprintf(html,"%02d:%02d:%02d",hr,min,sec);
		}
		else if( script1 == "IP" )
		{
			//ip=inet_addr(serv[str2num(script2)-1][1]);
			fprintf(html,serv[str2num(script2)-1][1]);
		}
		else if( script1 == "GMNUM" )
		{
			if(gm==0)
			{
				for(a=0;a<now;a++)
				{
					P_CHAR pc_a=MAKE_CHAR_REF(currchar[a]);
					if(ISVALIDPC(pc_a) && clientInfo[a]->ingame ) {
						if( pc_a->IsGM() ) gm++;
						else if( pc_a->IsCounselor() ) cns++;
					}
				}
			}
			fprintf(html,"%i",gm);
		}
		else if( script1 == "CNSNUM" )
		{
			if(cns==0)
			{
				for(a=0;a<now;a++)
				{
					P_CHAR pc_a=MAKE_CHAR_REF(currchar[a]);
					if(ISVALIDPC(pc_a) && clientInfo[a]->ingame ) {
						if( pc_a->IsGM() ) gm++;
						else if( pc_a->IsCounselor() ) cns++; //bugfix LB
					}
				}
			}
			fprintf(html,"%i",cns);
		}
		else if( script1 == "PDUMP" )
		{
			fprintf(html,"Network code: %fmsec [%i samples] <BR>",(float)((float)networkTime/(float)networkTimeCount),  networkTimeCount);
			fprintf(html,"Timer code: %fmsec [%i samples] <BR>" , (float)((float)timerTime/(float)timerTimeCount) , timerTimeCount);
			fprintf(html,"Auto code: %fmsec [%i samples] <BR>" , (float)((float)autoTime/(float)autoTimeCount) , autoTimeCount);
			fprintf(html,"Loop Time: %fmsec [%i samples] <BR>" , (float)((float)loopTime/(float)loopTimeCount) , loopTimeCount);
//			fprintf(html,"Characters: %i/Dynamic    Items: %i/Dynamic <BR>" , charcount, itemcount);
			if (!(loopTime <eps ||  loopTimeCount<eps)) //Bugfix LB
				fprintf(html,"Simulation Cycles: %f per sec <BR>" , (1000.0*(1.0/(float)((float)loopTime/(float)loopTimeCount))));
			else fprintf(html,"Simulation Cylces: too fast to be measured <BR>");

		}
		else if( script1 == "SIMCYC" )
		{
			if (!(loopTime <eps ||  loopTimeCount<eps))
				fprintf(html,"%f" , (1000.0*(1.0/(float)((float)loopTime/(float)loopTimeCount))));
			else fprintf(html,"too fast to be measured");
		}
		else if( script1 == "UDTIME" )
			fprintf(html,"%f",(float)(SrvParms->html/60));
		else if( script1 == "VER" ) fprintf(html,"%s %s [%s]",VER, VERNUMB, OS);
	} while( (script1[0]!='}') && (++loopexit < MAXLOOPS) );

	fclose(html);
	safedelete(iter);
}


