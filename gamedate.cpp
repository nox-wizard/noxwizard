  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "calendar.h"

char cGameDate::dateSeparator = '-';
char cGameDate::timeSeparator = ':';
char cGameDate::dateTimeSeparator = ' ';
cGameDate::eDateFormat cGameDate::dateFormat = YMD;

char cGameDate::getDateSeparator()
{
	return dateSeparator;
}

void cGameDate::setDateSeparator( char separator )
{
	dateSeparator = separator;
}

char cGameDate::getTimeSeparator()
{
	return timeSeparator;
}

void cGameDate::setTimeSeparator( char separator )
{
	timeSeparator = separator;
}

char cGameDate::getDateTimeSeparator()
{
	return dateTimeSeparator;
}

void cGameDate::setDateTimeSeparator( char separator )
{
	dateTimeSeparator = separator;
}

cGameDate::cGameDate()
{
	year 	= Calendar::g_nYear;
	month	= Calendar::g_nMonth;
	day	= Calendar::g_nDay;
	hour	= Calendar::g_nHour;
	minute	= Calendar::g_nMinute;
}

cGameDate::cGameDate( const cGameDate &copy )
{
	year 	= copy.year;
	month	= copy.month;
	day	= copy.day;
	hour	= copy.hour;
	minute	= copy.minute;
}

cGameDate::~cGameDate()
{
}

UI16 cGameDate::getYear()
{
	return year;
}

UI08 cGameDate::getMonth()
{
	return month;
}

UI08 cGameDate::getDay()
{
	return day;
}

UI08 cGameDate::getHour()
{
	return hour;
}

UI08 cGameDate::getMinute()
{
	return minute;
}

void cGameDate::setYear( UI16 newYear )
{
	year = newYear;
}

void cGameDate::setMonth( UI08 newMonth )
{
	month = newMonth;
}

void cGameDate::setDay( UI08 newDay )
{
	day = newDay;
}

void cGameDate::setHour( UI08 newHour )
{
	hour = newHour;
}

void cGameDate::setMinute( UI08 newMinute )
{
	minute = newMinute;
}

std::string cGameDate::toDateString( eDateFormat format )
{
	char temp[11];
	switch( format )
	{
		case YMD :
			sprintf( temp, "%04d%c%02d%c%02d", year, dateSeparator, month, dateSeparator, day );
			break;
		case DMY :
			sprintf( temp, "%02d%c%02d%c%04d", day, dateSeparator, month, dateSeparator, year );
			break;
	}
	return std::string(temp);
}

std::string cGameDate::toDateString()
{
	return toDateString( dateFormat );
}

std::string cGameDate::toTimeString()
{
	char temp[6];
	sprintf( temp, "%02d%c%02d", hour, timeSeparator, minute );
	return std::string(temp);
}

std::string cGameDate::toString()
{
	return toString( dateFormat );
}

std::string cGameDate::toString( eDateFormat format )
{
	return toDateString( format ) + dateTimeSeparator + toTimeString();
}

