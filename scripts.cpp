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
\brief Functions that handle the timer controlled stuff (? Akron)
\author Duke
\date 26/10/2000
*/

#include "nxwcommn.h"
#include "debug.h"

char g_strScriptTemp[TEMP_STR_SIZE]; //xan -> this is terrible :(





void readwscline () // Read line from *.WSC
{


	int i, valid=0;
	char c;
	g_strScriptTemp[0]=0;
	while (!valid)
	{
		i=0;
		if (feof(wscfile)) return;
		c=(char)fgetc(wscfile);
		while (c!=10)
		{
			if (c!=13)
			{
				g_strScriptTemp[i]=c;
				i++;
			}
			if (feof(wscfile) || i>=512 ) return;
			c=(char)fgetc(wscfile);
		}
		g_strScriptTemp[i]=0;
		valid=1;
		if (g_strScriptTemp[0]=='/' && g_strScriptTemp[1]=='/') valid=0;
		if (g_strScriptTemp[0]=='{') valid=0;
		if (g_strScriptTemp[0]==0) valid=0;
		if (g_strScriptTemp[0]==10) valid=0;
		if (g_strScriptTemp[0]==13) valid=0;
	}

}

// reads the ENTIRE line, spaces + ='s DONT't tokenize the line, but sections, line feeds etc are ignored
// LB
void readFullLine ()
{


	int i=0;
	readwscline();
	script1[0]=0;
	while(g_strScriptTemp[i]!=0  && i<1024 ) i++;
	strncpy(script1, g_strScriptTemp, i);
	script1[i]=0;
	return;

}

void readw2 ()
{


	int i=0;

	readwscline();
	script1[0]=0;
	script2[0]=0;
	script3[0]=0;
	while(g_strScriptTemp[i]!=0 && g_strScriptTemp[i]!=' ' && g_strScriptTemp[i]!='=' && i<1024 ) i++;
	strncpy(script1, g_strScriptTemp, i);
	script1[i]=0;
	if (script1[0]!='}' && g_strScriptTemp[i]!=0) strcpy(script2, (g_strScriptTemp+i+1));
	return;

}


void readw3 ()
{


	int i=0,j;

	readwscline();
	script1[0]=0;
	script2[0]=0;
	script3[0]=0;
	while(g_strScriptTemp[i]!=0 && g_strScriptTemp[i]!=' ' && g_strScriptTemp[i]!='=' && i<1024 ) i++;
	strncpy(script1, g_strScriptTemp, i);
	script1[i]=0;
	if (script1[0]=='}' || g_strScriptTemp[i]==0) return;
	i++;
	j=i;
	while(g_strScriptTemp[i]!=0 && g_strScriptTemp[i]!=' ' && g_strScriptTemp[i]!='=' && i <1024 ) i++;
	strncpy(script2, (g_strScriptTemp+j), i-j);
	script2[i-j]=0;
	strcpy(script3, (g_strScriptTemp+i+1));


}

/*!
\brief Gets the token number num in the passed string
\author Unknown, ported to std::string by Akron
\param s the string to get the token from
\param num 0-based index of the token
*/
void gettokennum(std::string s, int num)
{
	int j = 0;

	memset(gettokenstr, 0, 255);

	std::string::iterator it( s.begin() ), end( s.end() );
	int tokenfound = -1;

	while( it != end && tokenfound != num )
	{
		if( isspace( *it ) )
			++it;
		else
		{
			if( ++tokenfound == num )
			{
				do
				{
					gettokenstr[ j++ ] = *it;
				} while( (++it != end ) && !isspace( *it ) );
			}
			else
			{
				++it;
				while( it != end )
					if( !isspace( *it ) )
						++it;
					else
						break;
			}
		}
	}
/*
	int j = 0, loopexit = 0;
	std::string::iterator i = s.begin();
	memset(gettokenstr, 0, 255);
	
	while( num != 0 && (++loopexit < MAXLOOPS) )
	{
		if ( *i == 0 )
			return;
		else
		{
			if( isspace(*i) && i != s.begin() && !isspace(*(i-1)) )
				num--;
			i++;
		}
	}

	loopexit=0;
	while(num!=-1 && (++loopexit < MAXLOOPS) )
	{
		if ( *i == 0 )
			return;
		else
		{
			if ( isspace(*i) && i != s.begin() && !isspace(*(i+1)) )
				num--;
			else
				gettokenstr[j++] = *i;
			i++;
		}
	}
*/
}

// Added by Krozy on 7-Sep-98
// New getstatskillvalue function.
// Takes a string, gets the tokens.
// If its one value - It returns that value.
// If its two values - It gets a random number between the values
int getRangedValue(char *stringguy) {


	char values[512];
	int lovalue,hivalue,retcode;

	strcpy(values, stringguy);
	gettokennum(values, 0);
	lovalue=str2num(gettokenstr);
	gettokennum(values, 1);
	hivalue=str2num(gettokenstr);

	if (hivalue) {
		retcode = RandomNum(lovalue, hivalue);
	} else {
		retcode = lovalue;
	}
	return retcode;

}

void splitLine( char* source, char *head, char *tail )
{
  char* sourceIndex = source;
  char* headIndex = head;
  char* tailIndex = tail;

  while (*sourceIndex != '\0' && *sourceIndex == ' ' )
    ++sourceIndex;

  while (*sourceIndex != '\0' && *sourceIndex != ' ' )
  {
    *headIndex = *sourceIndex;
    ++sourceIndex;
    ++headIndex;
  }
  *headIndex = '\0';

  while (*sourceIndex != '\0' && *sourceIndex == ' ' )
    ++sourceIndex;

  while (*sourceIndex != '\0')
  {
    *tailIndex = *sourceIndex;
    ++sourceIndex;
    ++tailIndex;
  }
  *tailIndex = '\0';
}

