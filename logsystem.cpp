  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "logsystem.h"
#include "globals.h"
#include "inlines.h"


#define MAXLINESIZE	200

SI32 entries_e=0, entries_c=0, entries_w=0, entries_m=0;
LogFile ServerLog("server.log");

void ErrOut(char *txt, ...);

/*!
\brief Write text to the GM log file
\author Anthalir
\param pc current PC
\param msg message
\remarks \remark msg is formatted so you can use "file_%s", my_str as a filename
*/
void WriteGMLog(P_CHAR pc, char *msg, ...)
{
	va_list vargs;
	char tmp[MAXLINESIZE];

	va_start(vargs, msg);
		vsnprintf(tmp, sizeof(tmp)-1, msg, vargs);
	va_end(vargs);

	LogFile gmlog("GM/%s.log", pc->getCurrentNameC());
	gmlog.Write("%s", tmp);
}

/*!
\brief Constructor of the class
\author Anthalir
\param format name of the log file
*/
LogFile::LogFile(char *format, ...)
{
	char tmp[MAXLINESIZE], tmp2[MAXLINESIZE];
	va_list vargs;

	va_start(vargs, format);
		vsnprintf(tmp2, sizeof(tmp2)-1, format, vargs);
	va_end(vargs);

	// add path
	strcpy(tmp, SrvParms->log_path);
	strcat(tmp, tmp2);

	filename= new char[strlen(tmp)+1];
	strcpy(filename, tmp);

	file= fopen(tmp, "a");

	if( file==NULL )
	{
		ErrOut(TRANSLATE("unable to open/create log file %s"), tmp);
		return;
	}
}

LogFile::LogFile(string name)
{
	string tmp;

	// add path
	tmp= SrvParms->log_path + name;

	filename= new char[tmp.size()+1];
	strcpy(filename, tmp.c_str());

	file= fopen(filename, "a");

	if( file==NULL )
	{
		ErrOut(TRANSLATE("unable to open/create log file %s"), tmp.c_str());
		return;
	}
}

/*!
\brief Destructor of the class
\author Anthalir
*/
LogFile::~LogFile()
{
	if( file ) fclose(file);
	if( filename ) safedelete(filename);
}

/*!
\brief Write text to the log file
\author Anthalir
\param format msg to write
\remarks a timestamp is added before the string
*/
void LogFile::Write(char *format, ...)
{
	char tmp[MAXLINESIZE], tmp2[MAXLINESIZE];
	va_list vargs;

	if( file==NULL ) return;
	va_start(vargs, format);
		vsnprintf(tmp2, sizeof(tmp2)-1, format, vargs);
	va_end(vargs);

    time_t currtime= time(NULL);
    struct tm* T= localtime(&currtime);

	sprintf(tmp, "[%02d/%02d/%04d %02d:%02d:%02d] %s", T->tm_mday, T->tm_mon+1, T->tm_year+1900,
		T->tm_hour, T->tm_min, T->tm_sec, tmp2);

	if( fwrite(tmp, strlen(tmp), 1, file)==0 )
	{
		ErrOut(TRANSLATE("Unable to write to log file %s"), filename);
	}
}

void LogFile::Write(string str)
{
	char tmp[MAXLINESIZE];

	if( file==NULL ) return;

    time_t currtime= time(NULL);
    struct tm* T= localtime(&currtime);

	sprintf(tmp, "[%02d/%02d/%04d %02d:%02d:%02d] %s", T->tm_mday, T->tm_mon+1, T->tm_year+1900,
		T->tm_hour, T->tm_min, T->tm_sec, str.c_str());

	if( fwrite(tmp, strlen(tmp), 1, file)==0 )
	{
		ErrOut(TRANSLATE("Unable to write to log file %s"), filename);
	}
}

string SpeechLogFile::MakeFilename(P_CHAR pc)
{
	char tmp[MAXLINESIZE];

	if( !ISVALIDPC(pc) ) return "bad npc";

	sprintf(tmp, "speech/speech_[%d][%d][%s].txt", pc->account, pc->getSerial32(), pc->getCurrentNameC());
	string str(tmp);

	return str;
}

// SpeechLogFile

/*!
\brief Constructor of speech log file
\author Anthalir
\since 0.82a
\param pc character pointer
*/
SpeechLogFile::SpeechLogFile(cChar *pc) : LogFile(MakeFilename(pc))
{

}

char CurrentFile[100];
int CurrentLine;
char LogType= 'M';

#ifdef WIN32
	char *basename(char *path)
	{
		char *ret= path+strlen(path);				// ret= end of string path

		while( (*ret!='\\') && (*ret!='/') ) ret--;	// stop on the first '/' or '\' encountered
		return ++ret;
	}
#endif

void prepareLogs(char type, char *fpath, int lnum)
{
	fpath= basename(fpath);
	strncpy(CurrentFile, fpath, sizeof(CurrentFile)-1);
	CurrentFile[sizeof(CurrentFile)-1]= 0;
	CurrentLine= lnum;
	LogType= type;
}

/********************************************************
 *                                                      *
 *  Function to be called when a string is ready to be  *
 *    written to the log.  Insert access to your log in *
 *    this function.                                    *
 *                                                      *
 *  Rewritten/Improved/touched by LB 30-July 2000       *
 ********************************************************/
static void MessageReady(char *OutputMessage)
{
	char file_name[256];
	char b1[16],b2[16],b3[16],b4[16];
	unsigned long int ip=inet_addr(serv[0][1]);
	char i1,i2,i3,i4;

	i1=(char) (ip>>24);
	i2=(char) (ip>>16);
	i3=(char) (ip>>8);
	i4=(char) (ip%256);

	numtostr(i4 , b1);
	numtostr(i3 , b2);
	numtostr(i2, b3);
	numtostr(i1, b4);

    switch (LogType)
	{
	   case 'E': { strcpy(file_name,"errors_log_");          entries_e++; break; }
	   case 'C': { strcpy(file_name,"critical_errors_log_"); entries_c++; break; }
	   case 'W': { strcpy(file_name,"warnings_log_");        entries_w++; break; }
	   case 'M': { strcpy(file_name,"messages_log_");        entries_m++; break; }
	}

    strcat(file_name,b1);strcat(file_name,"_");
    strcat(file_name,b2);strcat(file_name,"_");
    strcat(file_name,b3);strcat(file_name,"_");
    strcat(file_name,b4);strcat(file_name,".txt");

	LogFile logerr(file_name);

	if (entries_c==1 && LogType=='C') // @serverstart, write out verison# !!!
	{
		logerr.Write("\nRunning NoX-Wizard Version: %s\n\n",VERNUMB);
		logerr.Write("******************************************************************************************************************************************\n");
		logerr.Write("* to increase the stability and quality of this software please send this file to the NoXWizard developers - thanks for your support !!! *\n");
		logerr.Write("******************************************************************************************************************************************\n\n");

	}

    if ( (entries_e==1 && LogType=='E') || (entries_w==1 && LogType=='W') || (entries_m==1 && LogType=='M'))
	{
	   logerr.Write("\nRunning NoX-Wizard Version: %s\n\n",VERNUMB);
	}

	logerr.Write("%s", OutputMessage);
}

/********************************************************
 *                                                      *
 *  Rountine to process and stamp a message.            *
 *                                                      *
 ********************************************************/
void LogMessageF(char *Message, ...)
{
	char fullMessage[512];
	char fullMessage2[512];
	va_list argptr;

	va_start(argptr, Message);
	vsnprintf(fullMessage, sizeof(fullMessage)-1, Message, argptr);
	va_end(argptr);

	switch( LogType )
	{
//	case 'M': InfoOut("%s\n", fullMessage); break;
    case 'W': WarnOut("%s\n",fullMessage); break;
    case 'E': ErrOut("%s\n",fullMessage); break;
    case 'C': PanicOut("%s\n",fullMessage); break;
	}

	if( LogType != 'M' )
		sprintf(fullMessage2, "[%s:%d] %s\n", CurrentFile, CurrentLine, fullMessage);
	else
		sprintf(fullMessage2, "%s\n", fullMessage);

	MessageReady(fullMessage2);
}

void LogSocketError(char* message, int err) {
	#ifndef __unix__
   		LogError("Socket Send error WSA_%i\n", WSAGetLastError());
	#else
   		LogError("Socket Send error %i (%s)\n", err, strerror(err)) ;
	#endif
}

