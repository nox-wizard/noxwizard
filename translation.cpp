  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"


namespace Translation 
{

static std::map<std::string, std::string> s_mapTranslation;

#define MAXTRANSLATIONCBUFFER 512
static char s_szTBuffer[MAXTRANSLATIONCBUFFER+10][2048];
static int s_nTBPtr = 0;
// Xan : we use a circular buffer of 512 strings. This should avoid *all* reentrancy probs
// unless we do a printf with 512 %s in it! :) - crap but works :]


///////////////////////////////////////////////////////////////////
// Function name     : init_translation
// Description       : Loads talk.scp and parses it
// Return type       : void 
// Author            : Elcabesa, Xanathar 
// Changes           : none yet
void init_translation(void)
{   
    char buffer[1024];
    int i,j,ln,lnp,line=0;
    char *ptr;
    std::string mystr;
    std::string mystr2;
	bool bOk, bError, bWereErrors = false;

    ConOut("Initializing translation...");
    FILE *F;

    F = fopen("talk.scp", "rt");
    
    if (F==NULL) {
        ConOut("[FAIL]\n"); 
        return;
    }

    while (!feof(F))
    {
        fgets(buffer, 1000, F);
		line++;
        ln = strlen(buffer);
        if (ln<1) continue;
        if (buffer[0]=='/') continue;
        buffer[ln-1] = '\0';
		if (ln>=2) {
			if ((buffer[ln-2]=='\n')||(buffer[ln-2]=='\r')) buffer[ln-2]='\0';
		}
        ptr = NULL;

        for (i=0; i<ln; i++) 
        { 
            if (buffer[i] == '$') {
                buffer[i] = '\0';
                ptr = buffer+i+1;
                break;
            }
        }

        if (ptr == NULL) continue;

		//Here : check if %'s are correct for printf-like strings
		ln = strlen(buffer);
		lnp = strlen(ptr);
		j = 0;
		bOk = false;
		bError = false;
		for(i=0; i<(ln+lnp+1);i++)
		{
		
			if( (buffer[i]!='a') && (buffer[i]!='b') && (buffer[i]!='c') &&
				(buffer[i]!='d') && (buffer[i]!='e') && (buffer[i]!='f') &&
				(buffer[i]!='g') && (buffer[i]!='h') && (buffer[i]!='i') &&
				(buffer[i]!='j') && (buffer[i]!='k') && (buffer[i]!='l') &&
				(buffer[i]!='m') && (buffer[i]!='n') && (buffer[i]!='o') &&
				(buffer[i]!='p') && (buffer[i]!='q') && (buffer[i]!='r') &&
				(buffer[i]!='s') && (buffer[i]!='t') && (buffer[i]!='u') &&
				(buffer[i]!='v') && (buffer[i]!='w') && (buffer[i]!='x') &&
				(buffer[i]!='y') && (buffer[i]!='z') && (buffer[i]!='A') &&
				(buffer[i]!='B') && (buffer[i]!='C') && (buffer[i]!='D') &&
				(buffer[i]!='E') && (buffer[i]!='F') && (buffer[i]!='G') &&
				(buffer[i]!='H') && (buffer[i]!='I') && (buffer[i]!='J') &&
				(buffer[i]!='K') && (buffer[i]!='L') && (buffer[i]!='M') &&
				(buffer[i]!='N') && (buffer[i]!='O') && (buffer[i]!='P') &&
				(buffer[i]!='Q') && (buffer[i]!='R') && (buffer[i]!='S') &&
				(buffer[i]!='T') && (buffer[i]!='U') && (buffer[i]!='V') &&
				(buffer[i]!='W') && (buffer[i]!='X') && (buffer[i]!='Y') &&
				(buffer[i]!='Z') && (buffer[i]!='1') && (buffer[i]!='2') &&
				(buffer[i]!='3') && (buffer[i]!='4') && (buffer[i]!='5') && 
				(buffer[i]!='6') && (buffer[i]!='7') && (buffer[i]!='8') && 
				(buffer[i]!='9') && (buffer[i]!='0') && (buffer[i]!='!') &&
				(buffer[i]!='"') && (buffer[i]!='#') && (buffer[i]!='$') &&
				(buffer[i]!='%') && (buffer[i]!='&') && (buffer[i]!='\'') &&
				(buffer[i]!='(') && (buffer[i]!=')') && (buffer[i]!='*') && 
				(buffer[i]!='+') && (buffer[i]!=',') && (buffer[i]!='.') && 
				(buffer[i]!='-') && (buffer[i]!='/') && (buffer[i]!=':') && 
				(buffer[i]!=';') && (buffer[i]!='>') && (buffer[i]!='<') && 
				(buffer[i]!='=') && (buffer[i]!='?') && (buffer[i]!='@') && 
				(buffer[i]!='[') && (buffer[i]!=']') && (buffer[i]!='\\') && 
				(buffer[i]!='_') && (buffer[i]!=' ') && (buffer[i]!='\n') &&
				(buffer[i]!='\0')&& (buffer[i]!='`') && (buffer[i]!='\t')// Horizontal tab
				)
			{
				if (!bWereErrors) 
				{
					ConOut("[FAIL]\n");
					bWereErrors = true;
				}
				char buf[2000];
				sprintf(buf, "Incorrect translation at line %i.You have used a bad char like %c.Translation will be ignored.\n", line, buffer[i]);
				ConOut("%s",buf);
				bError = true;
			}
		}
		for(i=0; i<ln; i++) {
			bOk = false;
			
			if ((buffer[i]=='%')&&(buffer[i+1]=='%')) { i++; continue; }
			
			if (buffer[i]=='%') {
				for(; j<lnp; j++) {
					if ((ptr[j]=='%')&&(ptr[j+1]=='%')) { j++; continue; }
					if (ptr[j]=='%') {
						bOk = (ptr[j+1]==buffer[i+1]);
						j++; //otherwise next loop we'll start here :|
						break;
					} //ptr j = %
				} // for j
				if ((j>=lnp)||(!bOk)) {
					if (!bWereErrors) {
						ConOut("[FAIL]\n");
						bWereErrors = true;
					}
					char buf[2000];
					sprintf(buf, "Incorrect translation of \"%s\" in \"%s\" at line %i.Translation will be ignored.\n", buffer, ptr, line);
					ConOut("%s",buf);
					bError = true;
					break;
				} // if not ok 
			} // if buffer[i] == %
		} // for i

		if (!bError) {
			mystr.erase();
			mystr=buffer;
			mystr2.erase();
			mystr2=ptr;
			s_mapTranslation.insert(make_pair(mystr, mystr2));
		}
    } // while not eof
	
	fclose(F);

    if (!bWereErrors) ConOut("[ OK ]\n");
#ifdef _WINDOWS
	if (bWereErrors) MessageBox(NULL, "Error(s) found in talk.scp. Please check them.", "Translation errors", MB_ICONWARNING);
#endif
}


///////////////////////////////////////////////////////////////////
// Function name     : translate 
// Description       : Translates a sz string into another sz string if available in talk.scp
// Return type       : char* 
// Author            : Elcabesa, Xanathar 
// Argument          : char* str -> String to be translated
// Changes           : none yet
char* translate (char* str) 
{
  map<std::string, std::string>::iterator myiter;
  std::string mystr;
  mystr.erase();
  mystr = str;

  myiter = s_mapTranslation.find(str);
  if (myiter == s_mapTranslation.end()) return str;

  s_nTBPtr++;
  s_nTBPtr %= MAXTRANSLATIONCBUFFER;
  strcpy(s_szTBuffer[s_nTBPtr],myiter->second.c_str());
  return s_szTBuffer[s_nTBPtr];
}



} //namespace
