  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"

/*!
\brief Translation related stuff
\author Xanathar

We use a circular buffer of 512 strings. This should avoid *all* reentrancy probs
unless we do a printf with 512 %s in it!
*/
namespace Translation 
{

	static std::map<std::string, std::string> s_mapTranslation;

	#define MAXTRANSLATIONCBUFFER 512
	static char s_szTBuffer[MAXTRANSLATIONCBUFFER+10][2048];
	static int s_nTBPtr = 0;
	// Xan : we use a circular buffer of 512 strings. This should avoid *all* reentrancy probs
	// unless we do a printf with 512 %s in it! :) - crap but works :]


	/*!
	\brief Loats talk.scp and parsers it
	\author Elcabesa, Xanathar
	*/
	void init_translation()
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

					if(	( (buffer[i] < 'a') || (buffer[i] > 'z') ) &&
						( (buffer[i] < 'A') || (buffer[i] > 'Z') ) &&
						( (buffer[i] < '0') || (buffer[i] > '9') ) &&
						(buffer[i]!='!') &&
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
						ConOut("Incorrect translation at line %i.You have used a bad char like %c.Translation will be ignored.\n", line, buffer[i]);
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
							ConOut("Incorrect translation of \"%s\" in \"%s\" at line %i.Translation will be ignored.\n", buffer, ptr, line);
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

	/*!
	\brief Translates a sz string into another sz string if avaiable in talk.scp
	\param str the untranslated string
	\return the translated string
	\author Elcabesa, Xanathar
	*/
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
