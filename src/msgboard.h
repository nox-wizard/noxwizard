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
\brief Message board functions
\author Dupois and Akron (rewrite and cleanup)
\note using namespaces and c++ style
*/

#ifndef __MSGBOARDS
#define __MSGBOARDS

#ifdef __unix__
#include <dirent.h>
#endif


/*!
\brief Message boards handling
\author Akron
\since 0.82r3
\todo rewrite functions
*/
namespace MsgBoards
{
	/*!
	\brief Maximum number of posts per board

	Buffer Size = 2560<br/>
	Therefore 0x3c max size = 2560 - 5 (0x3c header info) = 2550<br/>
	2550 / 19 (item segment size per msg) = 134<br/>
	Round down to 128 messages allowable on a message board (better safe than sorry)
	*/
	const UI32 MAXPOSTS = 128;

	const UI32 MAXENTRIES = 256; //!< maximum number of entries in a ESCORTS list in the MSGBOARD.sSCP file

	/*!
	\brief different types of user posts
	*/
	enum PostType { LOCALPOST = 0, REGIONALPOST, GLOBALPOST };

	/*!
	\brief different types of QUESTS
	Added for Quests (ESCORTS)<br/>
	Reason for starting high and counting down, is that I store the entire user
	posted message as is (which includes the message type).  The message type
	for a user post is 0x05 and I also use this field to determine whether the
	post is marked for deletion (0x00).  In order to allow for the maximum number
	of different quest types, I opted to start high and count down.
	*/
	enum QuestType { ESCORTQUEST = 0xFF, BOUNTYQUEST = 0xFE, ITEMQUEST = 0xFD, QTINVALID = 0x0 };

	void	MsgBoardEvent( NXWSOCKET s );
	void	MsgBoardSetPostType( NXWSOCKET s, PostType nPostType );
	void	MsgBoardGetPostType( NXWSOCKET s );
	int	MsgBoardPostQuest( NXWSOCKET s, QuestType nQuestType );
	void	MsgBoardQuestEscortCreate( NXWSOCKET s );
	void	MsgBoardQuestEscortArrive( P_CHAR pc, P_CHAR pc_k );
	void	MsgBoardQuestEscortDelete( int nNPCIndex );
	void	MsgBoardQuestEscortRemovePost( int nNPCIndex );
	void	MsgBoardMaintenance();
	bool	MsgBoardRemoveGlobalPostBySerial( int nPostSerial );
	#if defined(__unix__)
	std::vector<std::string> MsgBoardGetFile( char* pattern, char* path) ;
	#endif
};

#endif
