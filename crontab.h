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
\author Xanathar
\brief Simple crontab-like scheduler for UOX servers
\remark All the cron-jobs are in crontab.uox under the uox directory

\note The entries are checked in the same order entered

\note If you enables the DEBUGMODE macro commands are not executed but a system string is
 broadcasted to everyone with the command line at execution time

\note Note that the "resolution" is 2 minutes  so the schedule "* * * * * SAVE"
 is a save executed every 2 minutes

\note This piece of code is actually ALPHA TESTING ;) don't blame me if it destroy your server

\note any line not beginning with a number is considered a comment

\note since this is a rather old piece of code written originnaly for uox3 (but then discarded)
 it may (and probably will) contain some bugs :(

\note On NoX-Wizard 0.50 and later, this will become the main scheduling engine for all
 realworld time scheduled tasks, like weather changes, world saves, backups, and more

\note Can be edited with the not pratical :D text editor of remote administration and
 eventually reloaded. New tasks can be added at runtime with no save by amx fns with
 <br /><font face="monospace">addCronTask ( const format[], ...);</font><br />
 which simply adds a schedule with that ConOut like syntax command.

\note Also the commands ADDCRON and ADDCRONW of remote administration takes a string
 and insert it directly in the tasks list, the secondo updating the file also.
*/

#ifndef __CRONTAB_H_
#define __CRONTAB_H_

//! Kills the scheduler, just for reloads otherwise useless (OS frees memory automatically)
void killCronTab();
//! Inits the scheduler, must be called right before the main loop
void initCronTab();
//! checks for ready schedules, should be called at the beginning of the main loop
void checkCronTab();

#endif
