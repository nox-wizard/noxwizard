  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#ifndef __CGAMEDATE__
#define __CGAMEDATE__

class cGameDate
{
	enum eDateFormat
	{
		YMD,
		DMY
	};

	private:
		static	char		dateSeparator;
		static	char		timeSeparator;
		static	char		dateTimeSeparator;
		static	eDateFormat	dateFormat;
	public:
		static	char		getDateSeparator();
		static	void		setDateSeparator( char separator );
		static	char		getTimeSeparator();
		static	void		setTimeSeparator( char separator );
		static	char		getDateTimeSeparator();
		static	void		setDateTimeSeparator( char separator );
	public:
					cGameDate();
					cGameDate( const cGameDate &copy );
					~cGameDate();
	private:
		UI16			year;
		UI08			month;
		UI08			day;
		UI08			hour;
		UI08			minute;
	public:
		UI16			getYear();
		UI08			getMonth();
		UI08			getDay();
		UI08			getHour();
		UI08			getMinute();
		void			setYear( UI16 newYear );
		void			setMonth( UI08 newMonth );
		void			setDay( UI08 newDay );
		void			setHour( UI08 newHour );
		void			setMinute( UI08 newMinute );
		std::string		toDateString();
		std::string		toDateString( eDateFormat format );
		std::string		toTimeString();
		std::string		toString();
		std::string		toString( eDateFormat format );
		void			fromString( const std::string& arg );
		void			fromString( const std::string& arg, eDateFormat format );
	private:
		void			setDefaultDate();
		void			setDefaultYear();
		void			setDefaultMonth();
		void			setDefaultDay();
		void			setDefaultTime();
		void			setDefaultHour();
		void			setDefaultMinute();
};

#endif
