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
#include "gamedate.h"
#include "basics.h"

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
	setDefaultDate();
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

void	cGameDate::setDefaultDate()
{
	setDefaultYear()	;
	setDefaultMonth()	;
	setDefaultDay()		;
	setDefaultTime()	;
}

void	cGameDate::setDefaultYear()
{
	year 	= Calendar::g_nYear;
}

void	cGameDate::setDefaultMonth()
{
	month	= Calendar::g_nMonth;
}

void	cGameDate::setDefaultDay()
{
	day	= Calendar::g_nDay;
}

void	cGameDate::setDefaultTime()
{
	setDefaultHour();
	setDefaultMinute();
}

void	cGameDate::setDefaultHour()
{
	hour	= Calendar::g_nHour;
}

void	cGameDate::setDefaultMinute()
{
	minute	= Calendar::g_nMinute;
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

void cGameDate::fromString( const std::string& arg )
{
	fromString( arg, dateFormat );
}

void cGameDate::fromString( const std::string& arg, eDateFormat format )
{
	LOGICAL success = false;

	if( !arg.empty() )
	{
		UI32 start = 0;
		UI32 index = arg.find_first_of( dateSeparator );

		switch( format )
		{
			case YMD:
				if( index == start + 4 )
				{
					setYear( (UI16) str2num( const_cast<char*>( arg.substr( start, 4 ).c_str() ) ) );
					start = index + 1;
					index = arg.find_first_of( dateSeparator, start );
					if( index == start + 2 )
					{
						setMonth( (UI08) str2num( const_cast<char*>( arg.substr( start, 2 ).c_str() ) ) );
						start = index + 1;
						index = arg.find_first_of( dateTimeSeparator, start );
						if( index == start + 2 )
						{
							setDay( (UI08) str2num( const_cast<char*>( arg.substr( start, 2 ).c_str() ) ) );
							start = index + 1;
							index = arg.find_first_of( timeSeparator, start );
							if( index == start + 2 )
							{
								setHour( (UI08) str2num( const_cast<char*>( arg.substr( start, 2 ).c_str() ) ) );
								start = index + 1;
								setMinute( (UI08) str2num( const_cast<char*>( arg.substr( start, 2 ).c_str() ) ) );
								success = true;
							}
						}
					}
				}
				break;
			case DMY:
				if( index == start + 2 )
				{
					setDay( (UI08) str2num( const_cast<char*>( arg.substr( start, 2 ).c_str() ) ) );
					start = index + 1;
					index = arg.find_first_of( dateSeparator, start );
					if( index == start + 2 )
					{
						setMonth( (UI08) str2num( const_cast<char*>( arg.substr( start, 2 ).c_str() ) ) );
						start = index + 1;
						index = arg.find_first_of( dateTimeSeparator, start );
						if( index == start + 4 )
						{
							setYear( (UI16) str2num( const_cast<char*>( arg.substr( start, 4 ).c_str() ) ) );
							start = index + 1;
							index = arg.find_first_of( timeSeparator, start );
							if( index == start + 2 )
							{
								setHour( (UI08) str2num( const_cast<char*>( arg.substr( start, 2 ).c_str() ) ) );
								start = index + 1;
								setMinute( (UI08) str2num( const_cast<char*>( arg.substr( start, 2 ).c_str() ) ) );
								success = true;
							}
						}
					}
				}
				break;
		}
	}

	if( !success )
		setDefaultDate();
}

