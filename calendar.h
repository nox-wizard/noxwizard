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
\brief Calendar related stuff
\author Xanathar
*/
#ifndef __CALENDAR_H__
#define __CALENDAR_H__

/*!
\brief Calendar related stuff
\author Xanathar
\since 0.52a
*/
namespace Calendar {

	extern int g_nWeekday, g_nDay, g_nMonth, g_nYear, g_nHour, g_nMinute;
	extern int g_nMaxWeekday, g_nMaxMonth;
	extern int g_nCurDawnHour, g_nCurDawnMin, g_nCurSunsetHour, g_nCurSunsetMin;
	extern int g_nWinterOverride;
	extern float g_fCurSnowMod, g_fCurRainMod, g_fCurDryMod;

	void loadCalendarScp ();
	bool advanceMinute();
	void setSeason(int nSeason);
	void commitSeason(P_CHAR pc);

	/*!
	\brief Class cMonth
	\author Xanathar
	\since 0.52a
	\note updated by Sparhawk to open up calendar related info for usage in amx
	*/
	class cMonth
	{
		public:
			char*	m_strName;
			int	m_nDays;
			int	m_nCompensation;
			float	m_fSnowMod;
			float	m_fRainMod;
			float	m_fDryMod;
			int	m_nDawnHour;
			int	m_nDawnMin;
			int	m_nSunsetHour;
			int	m_nSunsetMin;
			int	m_nSeason;
			int	getDays();
	};

	/*!
	\brief Class cWeekday
	\author Xanathar
	\since 0.52a
	*/
	class cWeekday
	{
		public:
			char*	m_strName;
	};

	extern cMonth g_Months[];
	extern cWeekday g_Weekday[];
}

#endif
