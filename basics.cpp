  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "basics.h"
#include "inlines.h"

/*!
\brief convert a char* into a number with the specified base
\author Xanathar
\return int the number or 0 if no conversion possible
\param sz the string
\param base number's base
*/
int str2num( char* sz, int base )
{
	char *dummy;
	return strtol(sz, &dummy, base );
}

/*!
\brief Convert a wchar_t* into a number with the specified base
\author Endymion
\return int the number or 0 if no conversion possible
\param sz the string
\param base number's base
*/
int str2num( wchar_t* sz, int base )
{
	wchar_t *dummy;
	return wcstol(sz, &dummy, base );
}


/*!
\brief Returns a random number between bounds
\author ?
\return int the number
\param nLowNum lower bound
\param nHighNum higher bound
*/
int RandomNum(int nLowNum, int nHighNum)
{
	if (nHighNum - nLowNum + 1)
		return ((rand() % (nHighNum - nLowNum + 1)) + nLowNum);
	else
		return nLowNum;
}

/*!
\brief fills an integer array with tokens extracted from a string
\author Xanathar
\return int the number of number read from the string
\param str the string
\param array the array
\param maxsize the size of array
\param defval -1 -> the default value for uninitialized items
\param base the base for number conversion
*/
int fillIntArray(char* str, int *array, int maxsize, int defval, int base)
{
	int i=0;
	char tmp[1048];
	char *mem;

	if (strlen(str) > 1024) {
		mem = new char[strlen(str)+5];
	} else mem = tmp;  //xan -> we avoid dyna-alloc for strs < 1K
	strcpy(tmp, str);

	char *s;
	char *delimiter = " ";

	if (base != BASE_INARRAY) for (i = 0; i < maxsize; i++) array[i] = defval;

	i = 0;

	s = strtok(tmp,delimiter);

	while ((s!=NULL)&&(i < maxsize)) {
		if (base == BASE_INARRAY) {
			array[i] = str2num(s, array[i]);
			i++;
		}
		else {
			array[i++] = str2num(s, base);
		}
		s = strtok(NULL, delimiter);
	}

	if (mem != tmp) safedeletearray(mem);
	return i;
}

/*!
\brief reads a line splitted (read2 clone)
\author Xanathar
\param F the file to read from
\param script1 where we'll put the first token of the string
\param script2 where we'll put the rest of the string
*/
void readSplitted(FILE* F, char* script1, char* script2)
{
	char str[1024];
	fgets(str, 1000, F);
	str[1000] = '\0';
	str[strlen(str)-1] = '\0';

	int ln = strlen(str)-1;
	if (ln>0) {
		if ((str[ln]== '\r')||(str[ln]== '\n')) {
			str[ln] = '\0';
		} // remove CR+LF :]
	}

	int i=0;
	script1[0]=0;
	script2[0]=0;

	while((str[i]!=0) && (str[i]!=' ') && (str[i]!='=') && (i<1024) )
	{
		i++;
	}

	strncpy(script1, str, i);
	script1[i]=0;
	if ((script1[0]!='}') && (str[i]!=0)) strcpy(script2,(char*)( str+i+1));
}




/*!
\brief return current local time
\return char* the time (time_str)
\param time_str a string with enough memory allocated
*/
char *RealTime(char *time_str)
{
	struct tm *curtime;
	time_t bintime;
	time(&bintime);
	curtime = localtime(&bintime);
	strftime(time_str, 256, "%A %B %d %I:%M:%S %p %Y %z", curtime);
	return time_str;
}

/*!
\brief build a Location structure
\author Anthalir
\return The Location structure that represent the, ehm, location
\since 0.82a
\param x X-Coordinate
\param y Y-Coordinate
\param z Z-Coordinate
\param dispz displayed z value
*/
Location Loc(SI32 x, SI32 y, SI08 z, SI08 dispz)
{
	Location l= {x, y, z, 0};
	if(dispz==0)
		l.dispz=z;
	else
		l.dispz= dispz;

	return l;
}
