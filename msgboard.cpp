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

#include "nxwcommn.h"
#include "network.h"
#include "sregions.h"
#include "sndpkg.h"
#include "debug.h"
#include "msgboard.h"
#include "npcai.h"
#include "scp_parser.h"
#include "items.h"
#include "chars.h"
#include "utils.h"
#include "inlines.h"
#include "nox-wizard.h"
#include "scripts.h"

namespace MsgBoards
{

/*!
\note xan -> do not move in headers, plz
*/
extern char g_strScriptTemp[TEMP_STR_SIZE];

UI08 postAcked[MAXCLIENT][MAXPOSTS][5]; //!< List of message serial numbers ACK'd by client
UI32 postCount[MAXCLIENT]; //!< Total number of posts sent to client
UI32 postAckCount[MAXCLIENT]; //!< Total number of ACK's received by client

/*!
\brief Char array for messages to client.
Message body (when entering body of post) can hold a maximum of 1975 chars (approx)
*/
UI08 msg[MAXBUFFER];

//! Buffer to be used when posting messages
UI08 msg2Post[MAXBUFFER] = "\x71\xFF\xFF\x05\x40\x00\x00\x19\x00\x00\x00\x00";
//                                     |Pid|sz1|sz2|mTy|b1 |b2 |b3 |b4 |m1 |m2 |m3 |m4 |

/*!
\param s player socket
Used to retrieve the current post type in order to tell the user what type of
mode they are in.
*/
void MsgBoardGetPostType( NXWSOCKET s )
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	PostType type = (PostType)pc->postType;

	switch ( type )
	{
		case LOCALPOST: // LOCAL post
			sysmessage( s, TRANSLATE("Currently posting LOCAL messages") );
			break;

		case REGIONALPOST: // REGIONAL post
			sysmessage( s, TRANSLATE("Currently posting REGIONAL messages") );
			break;

		case GLOBALPOST: // GLOBAL POST
			sysmessage( s, TRANSLATE("Currently posting GLOBAL messages" ));
			break;
	}
	return;


}

/*!
\param s player socket
\param nPostType type of post

Used to set the postType for the current user (Typically a GM)
There is a local array that holds each players curreny posting
type.  Using the command to set the post type updates the
value in the array for that player so that they can post
different types of messages.
*/
void MsgBoardSetPostType( NXWSOCKET s, PostType nPostType )
{
	P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
	VALIDATEPC(pc);

	pc->postType = nPostType;

	switch ( nPostType )
	{
		case LOCALPOST: // LOCAL post
			sysmessage( s, TRANSLATE("Post type set to LOCAL" ));
			break;

		case REGIONALPOST: // REGIONAL post
			sysmessage( s, TRANSLATE("Post type set to REGIONAL") );
			break;

		case GLOBALPOST: // GLOBAL POST
			sysmessage( s, TRANSLATE("Post type set to GLOBAL") );
			break;
	}
	return;

}

/*!
\param s player serial number
Called when player dbl-clicks on a Message Board thereby
requesting a list of messages posted on the board.
*/
void MsgBoardOpen(int s)
{
	// In Response to a doubleclick() message from a client
	// [SEND:5] 06 40 07 ba 3d
	// Message to client     |Pak|sz1|sz2|mTy|sn1|sn2|sn3|sn4| b | u | l | l | e | t | i | n |   | b | o | a | r | d |<------------  Unknown, don't care right now ----------------->|
	unsigned char msgBoardHeader[] = "\x71\x00\x26\x00\xFF\xFF\xFF\xFF\x62\x75\x6c\x6c\x65\x74\x69\x6e\x20\x62\x6f\x61\x72\x64\x00\x00\x00\x00\x00\x00\x00\x00\x40\x20\x00\xff\x00\x00\x00\x00";

	// Can place up to 20 customizable chars in Message Board header to give Message Board a unique name
	// Might be able to do more, but why, it usually overruns the area designated for the name anyway
	SERIAL msgBoardSerial;
	msgBoardSerial = LongFromCharPtr(buffer[s] +1);

	P_ITEM p_msgboard = pointers::findItemBySerial(msgBoardSerial);

	LongToCharPtr(msgBoardSerial, msgBoardHeader +4);

	// If the name the item (Bulletin Board) has been defined, display it
	// instead of the default "Bulletin Board" title.
	if ( strncmp(p_msgboard->getCurrentNameC(), "#", 1) )
		strncpy( (char *)&msgBoardHeader[8], p_msgboard->getCurrentNameC(), 20);

	// Send Message Board header to client
	Xsend(s, msgBoardHeader, (sizeof(msgBoardHeader)-1) );


	// Send draw item message to client with required info to draw the message board
	// Base size plus however many messages are in the list
	// Example:
	// s1 s2 s3 s4 m# m# ?? st st xx xx yy yy b1 b2 b3 b4 c1 c2
	// 40 1c 53 eb 0e b0 00 00 00 00 3a 00 3a 40 07 ba 3d 00 00    Size = 19 (min size for msg = 24 bytes)

	// Standard header :        |Pak|sz1|sz2|sg1|sg2|<--------------- See above ------------------->                            |
	//                         "\x3c\x00\x18\x00\x01\x40\x1c\x53\xeb\x0e\xb0\x00\x00\x00\x00\x3a\x00\x3a\x40\x00\x00\x19\x00\x00";

	// Read bbi file to determine messages on boards list
	// Get Message Board serial number from message buffer
	FILE *file = NULL;
	// 50 chars for prefix and 4 for the extension plus the ending NULL
	char fileName[256] = "";
	char fileName1[55];
	char fileName2[55];
	char fileName3[55];

	msg[0] = 0x3c;  // Packet type (Items in Container)
	msg[1] = 0x00;  // High byte of packet size
	msg[2] = 0x00;  // Low byte of packet size
	msg[3] = 0x00;  // High byte of number of items
	msg[4] = 0x00;  // Low byte of number of items

	int offset = 5; // Offset to next msg[] value
	UI32 count  = 0; // Number of messages (times through while loop)
	//int x      = 0;

	int currentFile  = 1;  // Starting file to open and iterate through (1=GLOBAL.bbp, 2=REGIONAL.bbp, 3=LOCAL.bbp)

	// Determine what type of message this is in order to determine which file to open
	// GLOBAL   Posts start at 01 00 00 00 -> 01 FF FF FF
	// REGIONAL Posts start at 02 00 00 00 -> 02 FF FF FF
	// LOCAL    Posts start at 03 00 00 00 -> 03 FF FF FF



	// GLOBAL post file
	strcpy( fileName1, "global.bbi" );

	// REGIONAL post file
	//sprintf( fileName2, "region%s.bbi", region[calcRegionFromXY(items[msgBoardSerial].x, items[msgBoardSerial].y)].name );
	sprintf( fileName2, "region%d.bbi", calcRegionFromXY( p_msgboard->getPosition() ) );

	// LOCAL post file
	sprintf( fileName3, "%08x.bbi", msgBoardSerial);

	while ( currentFile <= 3 )
	{
		// If a MSBBOARDPATH has been define in the SERVER.CFG file, then use it
		if (SrvParms->msgboardpath)
			strcpy( fileName, SrvParms->msgboardpath );

		// Open the next file to process
		switch ( currentFile )
		{
		case 1:
			// Start with the GLOBAL.bbp file first
			//sysmessage( s, "Opening GLOBAL.bbi messages");
			strcat( fileName, fileName1 );
			file = fopen( fileName, "rb" );
			break;

		case 2:
			// Set fileName to REGIONAL.bbi
			//sysmessage( s, "Opening REGIONAL.bbi messages");
			strcat( fileName, fileName2 );
			file = fopen( fileName, "rb" );
			break;

		case 3:
			// Set fileName to LOCAL.bbi
			//sysmessage( s, "Opening LOCAL.bbi messages");
			strcat( fileName, fileName3 );
			file = fopen( fileName, "rb" );
			break;

		default:
			ErrOut("MsgBoardOpen() Unhandle case value: %d", currentFile);
			return;
		}

		// If the file doesn't exist, increment the currenFile count and move onto the next file
		if ( file != NULL )
		{
			// Ignore first 4 bytes of bbi file as this is reserverd for the current max message serial number being used

			if ( fseek( file, 4, SEEK_SET ) )
			{
				ErrOut("MsgBoardOpen() failed to seek to first message segment in bbi file\n");
				return;
			}

			// Loop until we have reached the end of the file or the maximum number of posts allowed
			// to be displayed
			while ( (!feof(file)) && (count<=MAXPOSTS) )
			{
				// Fill up the msg with data from the bbi file
				if ( fread( &msg[offset], sizeof(char), 19, file ) != 19 )
				{
					if ( feof(file) ) break;
				}
				//  |Off| 1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18
				//  |mg1|mg2|mg3|mg4|mo1|mo2|???|sg1|sg2|xx1|xx2|yy1|yy2|bn1|bn2|bn3|bn4|co1|co2|
				// "\x40\x1c\x53\xeb\x0e\xb0\x00\x00\x00\x00\x3a\x00\x3a\x40\x00\x00\x19\x00\x00";

				// If the segment 6 is 0x00 then the message is marked for deletion so skip it
				if ( msg[offset+6] )
				{
					// Set the Board SN fields the proper value for the board clicked on
					LongToCharPtr(msgBoardSerial, &msg[offset+13]);

					// Store message ID into array for later acknowledgement
					postAcked[s][count][0] = msg[offset+0];
					postAcked[s][count][1] = msg[offset+1];
					postAcked[s][count][2] = msg[offset+2];
					postAcked[s][count][3] = msg[offset+3];

					// Increment the offset by 19 bytes for next message index
					offset += 19;

					// Increment the message count
					count++;
				}
			}
		}

		// Close the current bbi file
		if( file ) fclose( file );

		// Increment to the next file
		currentFile++;
	}

	// Close bbi file
	if( file ) fclose( file );

	// Update size fields of message with new values
	ShortToCharPtr(offset, msg +1);
	ShortToCharPtr(count, msg +3);

	// Set global variable that holds the count of the number of posts being sent
	// to this particular client
	postCount[s] = count;

	// Set the postAckCount to zero in preparation of the client ACKing the message
	// about to be sent
	postAckCount[s] = 0;

	// Send Draw Item message to client
	Xsend( s, msg, offset);

}

/*!
\param s player serial number
After Bulletin Board is displayed and client ACK's all posted
items this is called to send the details of the posted item to
the Bulletin Board so that it can be listed on the board.
*/
void MsgBoardList( int s )
{
	int loopexit=0, loopexit2=0;
	// READ IN bbp FILE (for list on message board)

	// Client sends:
	// Example  [SEND:12] 71 00 0c 04 40 07 ba 3d 40 1c 53 eb
	//
	// Server responds with:
	// Sample Response format for message board message list item
	//                                                         |sA|au|sS| Subject                                                                                                                                                |sD| Date                                    |
	//[RECV:85] 71 00 55 01 40 07 ba 3d 40 1c 53 eb 00 00 00 00 01 00 33 46 6f 58 20 74 68 65 20 44 65 6d 65 6e 74 65 64 3a 20 20 34 30 30 30 67 6f 6c 64 2e 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 00 0e 44 61 79 20 33 20 40 20 32 33 3a 32 30 00

	// 50 chars for prefix and 4 for the extension plus the ending NULL
	char	fileName[256] = "";
	char	fileName1[55];  // for global file
	char	fileName2[55];  // for regional file
	char	fileName3[55];  // for local file
	FILE	*file = NULL;

	SI32	msgBytes     = 0;  // Number of bytes to send to client (message size)
	SI32	msgOffset    = 0;  // Total number of bytes between messages from start of file
	UI32	segmentSize  = 0;  // Size of a segment (Author, Subject, Date)
	SI32	foundMsg     = 0;  // Flag when message has been found
	SERIAL	boardSN      = 0;  // Bulletin Boards serial number (to determine what regions messages to display
	SI32	currentFile  = 1;  // Starting file to open and iterate through (1=GLOBAL.bbp, 2=REGIONAL.bbp, 3=LOCAL.bbp)
	UI32	w            = 0;  // Counter
	SI32	x            = 0;  // Counter

	// Determine what type of message this is in order to determine which file to open
	// GLOBAL   Posts start at 01 00 00 00 -> 01 FF FF FF
	// REGIONAL Posts start at 02 00 00 00 -> 02 FF FF FF
	// LOCAL    Posts start at 03 00 00 00 -> 03 FF FF FF

	// Determine the Bulletin Boards serial number
	boardSN = LongFromCharPtr(buffer[s] +4);

	P_ITEM p_msgboard = pointers::findItemBySerial(boardSN);

	// GLOBAL post file
	strcpy( fileName1, "global.bbp" );

	// REGIONAL post file
	// sprintf( fileName2, "%s.bbp", region[calcRegionFromXY(items[boardSN].x, items[boardSN].y)].name );
	sprintf( fileName2, "region%d.bbp", calcRegionFromXY( p_msgboard->getPosition() ) );

	// LOCAL post file
	sprintf( fileName3, "%08x.bbp", boardSN);

	// Open the bbp file for read
	//file = fopen( fileName3, "rb");

	//if ( file == NULL ) return;   // Put file not found error control here
	//if ( feof(file) ) return;     // Put end of file error control here

	while ( currentFile <= 3 )
	{
		// If a MSBBOARDPATH has been define in the SERVER.CFG file, then use it
		if (SrvParms->msgboardpath)
			strcpy( fileName, SrvParms->msgboardpath );

		// Open the next file to process
		switch ( currentFile )
		{
		case 1:
			// Start with the GLOBAL.bbp file first
			//sysmessage( s, "Opening GLOBAL.bbp messages");
			strcat( fileName, fileName1 );
			file = fopen( fileName, "rb");
			break;

		case 2:
			// Set fileName to REGIONAL.bbp
			//sysmessage( s, "Opening REGIONAL.bbp messages");
			strcat( fileName, fileName2 );
			file = fopen( fileName, "rb" );
			break;

		case 3:
			// Set fileName to LOCAL.bbp
			//sysmessage( s, "Opening LOCAL.bbp messages");
			strcat( fileName, fileName3 );
			file = fopen( fileName, "rb" );
			break;

		default:
			ErrOut("MsgBoardOpen() Unhandle case value: %d", currentFile);
			return;
		}
		msgOffset = 0;

		// If the file doesn't exist, increment the currenFile count and move onto the next file
		if ( file != NULL )
		{
			while ( w<postCount[s] && (++loopexit < MAXLOOPS) )
			{
				foundMsg = 0;

				// Find Message ID that has been requested
				loopexit2=0;
				while ( !foundMsg && (++loopexit2 < MAXLOOPS) )
				{
					x = 0;
					//                                0       1     2      3      4   5   6      7   8    9    10   11   12   13   14   15
					// Read In the first 12 bytes |PacketID|Size1|Size2|MsgType|bSn1|bSn2|bSn3|bSn4|mSn1|mSn2|mSn3|mSn4|pmSN|pmSN|pmSN|pmSN|
					fread( &msg[x], sizeof(char), 16, file );
					msgBytes = 16;

					// If we have reached the EOF then stop searching
					if ( feof(file) ) break;

					// Check buffered message SN with currently read message SN
					if (( msg[8]  == postAcked[s][w][0] ) &&
						( msg[9]  == postAcked[s][w][1] ) &&
						( msg[10] == postAcked[s][w][2]) &&
						( msg[11] == postAcked[s][w][3]))
					{
						// Don't forget to set the flag to stop searching for the message when we find it
						foundMsg = 1;

						// Increment While loop counter tracking the number of posts we have replied too
						w++;

						// Set the board SN values to the board that was just double-clicked on
						LongToCharPtr(boardSN, msg +4);

						// Read in  author, subject and date info to pass back to client (DO NOT SEND BODY of msg)
						// Count the total number of bytes in posting (not including body as it isn't sent to client)

						// Author, Subject, and Date segments can all be retrieved in the same fashion
						// | size | data........................... 0x00 |
						//
						// size equals DATA + NULL
						// Segments:
						//             0 = Author
						//             1 = Subject
						//             2 = Date
						for ( x=0; x<=2; x++ )
						{
							// Get the size of this segment and store it in the message
							msg[msgBytes] = fgetc( file );

							// Put the size into a variable
							segmentSize = msg[msgBytes];
							msgBytes++;

							// Read in the number of bytes give by the segment size
							if ( segmentSize != fread( &msg[msgBytes], sizeof(char), segmentSize, file ) )
							{
								// If we are unable to read in the number of bytes specified by the segmentSize, ABORT!
								ErrOut("MsgBoardList() couldn't read in entire segment(%i)\n", x);
								fclose( file );
								return;
							}

							// Increase msgBytes to the new size
							msgBytes += segmentSize;
						}

						msgOffset += (msg[1]<<8) + msg[2];

						// Jump to next message
						if ( fseek(file, msgOffset, SEEK_SET) )
							ErrOut("MsgBoardEvent() case 4 : failed to seek start of next message\n");

						// Calculate new message size
						ShortToCharPtr(msgBytes, msg +1);

						// Set packet 0x71 message type to /0x01 (send post item to message board list)
						msg[3] = 1;

						// Send message to client
						Xsend( s, msg, msgBytes);
					}
					else // If this isn't the message were looking for, jump ahead to next message
					{
						// Since we didn't find the message in this pass, get this messages size and jump ahead
						msgOffset += (msg[1]<<8) | msg[2];

						// Jump to next message
						if ( fseek(file, msgOffset, SEEK_SET) )
						{
							ErrOut("MsgBoardEvent() case 4 : failed to seek next message\n");
							break;
						}
					}
				} // End of Inner while loop (redundant but safe -- for now)

				// If we broke out of the loop  because EOF was reached then break out again
				if( feof( file ) )
				{
					fclose(file);
					break;
				}
      }// End of Outer while loop

    }// End of if block

	// Close the current file and increment the currentFile counter
	if ( file ) fclose( file );

	// Increment the current file counter
	currentFile++;

  }// End of While loop

  // If we still have 'file' open, close 'file'
  if ( file ) fclose( file );

}

/*!
\param msgType Type of post (LOCAL, REGIONAL, GLOBAL)
\param autoPost if true, message posted by a user (default); else message posted by system
\return 0 if failed to get maximum serial numbero, 1 if found and updated maximum serial number

Used during posting operation to determine the current maximum
message serial number used on the board.  It then increments
the number and updates the bbi file.  The new message serial
number is returned.
*/
int MsgBoardGetMaxMsgSN( int msgType, int autoPost=0 )
{
	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	FILE        *pFile            = NULL;

	// 50 chars for prefix and 4 for the extension plus the ending NULL
	char        fileName[256]     = "";
	UI08 msgbbiSegment[20] = "\x00\x00\x00\x00\x0e\x0b\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
	UI08 maxMsgSN[4] = { 0x00, };

	SERIAL msgBoardSerial    = 0;
	SERIAL maxSN             = 0;

	struct tm   timeOfPost;
	time_t      now;
	// If a MSBBOARDPATH has been define in the SERVER.CFG file, then use it
	if (SrvParms->msgboardpath)
		strcpy( fileName, SrvParms->msgboardpath );

	msgBoardSerial = LongFromCharPtr(msg2Post +4);

	switch ( msgType )
	{
		// LOCAL post
	case LOCALPOST:
		// Get Message Board serial number from message buffer
		sprintf( temp, "%08x.bbi", msgBoardSerial);
		break;

		// REGIONAL post
	case REGIONALPOST:
		// set the Message Board fileName to the proper region number
		if ( autoPost )
		{
			P_CHAR pc_s=pointers::findCharBySerial(msgBoardSerial);
			if(ISVALIDPC(pc_s))
				sprintf( temp, "region%d.bbi", pc_s->region );
		}
		else
		{
			P_ITEM pi_s=pointers::findItemBySerial(msgBoardSerial);
			if(ISVALIDPI(pi_s))
				sprintf( temp, "region%d.bbi", calcRegionFromXY( pi_s->getPosition() ) );
		}
		break;

		// GLOBAL POST
	case GLOBALPOST:
		strcpy( temp, "global.bbi" );
		break;

		// Invalid post type
	default:
		ErrOut("MsgBoardGetMaxMsgSN() Invalid post type, aborting post\n");
		return 0;
	}

	// Append file name to path
	strcat( fileName, temp );

	// Get the current maximum message s/n from the bbi file
	pFile = fopen( fileName, "rb" );
	if ( pFile == NULL )
	{
		ErrOut("MsgBoardGetMaxMsgSN() bbi not found. Creating file %s\n", fileName );

		// Default to serial number 0
		maxSN = 0;
	}
	else // bbi file exists so read in first 4 bytes to get current maximum serial number
	{
		// Get the first 4 bytes from each message index segment in the bbi file
		if ( fread(maxMsgSN, sizeof(char), 4, pFile) != 4 )
		{
			ErrOut("MsgBoardGetMaxMsgSN() Could not get MaxSN from %s\n", fileName );

			fclose( pFile );
			return 0;
		}

		// Calculate the maxSN in decimal
		maxSN = LongFromCharPtr(maxMsgSN);

		// Increment maxSN to new value
		maxSN++;
	}

	// Done retrieving maxMsgSN so close bbi file
	if ( pFile ) fclose( pFile );

	// If the maxSN == 0 then the file does not exist yet so create the file
	if ( maxSN == 0 )
	{
		pFile = fopen( fileName, "ab+" );

		if ( pFile == NULL )
		{
			ErrOut("MsgBoardGetMaxMsgSN() Error creating bbi file, aborting post\n");
			return 0;
		}
		else
		{
			// GLOBAL   Posts start at 01 00 00 00 -> 01 FF FF FF
			// REGIONAL Posts start at 02 00 00 00 -> 02 FF FF FF
			// LOCAL    Posts start at 03 00 00 00 -> 03 FF FF FF
			switch ( msgType )
			{
				// LOCAL post
			case LOCALPOST:
				// Write out the serial number as 4 bytes
				// Write 03 00 00 00 to bbi file (can't start at 00 00 00 00 because client crashes if this is true)
				if ( fwrite("\x03\x00\x00\x00", sizeof(char), 4, pFile) != 4 )
				{
					ErrOut("MsgBoardGetMaxMsgSN() Error writing to bbi file, aborting post\n");
					fclose( pFile );
					return 0;
				}
				// Set maxSN to its startup default
				maxSN = 0x3000000;
				break;

				// REGIONAL post
			case REGIONALPOST:
				// Write 02 00 00 00 to bbi file (can't start at 00 00 00 00 because client crashes if this is true)
				if ( fwrite("\x02\x00\x00\x00", sizeof(char), 4, pFile) != 4 )
				{
					ErrOut("MsgBoardGetMaxMsgSN() Error writing to bbi file, aborting post\n");
					fclose( pFile );
					return 0;
				}
				// Set maxSN to its startup default
				maxSN = 0x2000000;
				break;

				// GLOBAL POST (any other value)
			case GLOBALPOST:
				// Write 01 00 00 00 to bbi file (can't start at 00 00 00 00 because client crashes if this is true)
				if ( fwrite("\x01\x00\x00\x00", sizeof(char), 4, pFile) != 4 )
				{
					ErrOut("MsgBoardGetMaxMsgSN() Error writing to bbi file, aborting post\n");
					fclose( pFile );
					return 0;
				}

				// Set maxSN to its startup default
				maxSN = 0x1000000;
				break;

				// Invalid post type
			default:
				ErrOut("MsgBoardGetMaxMsgSN() Invalid post type, aborting post\n");
				fclose( pFile );
				return 0;
			}
		}
	}
	else
	{
		// File must have existed already if maxSN > 0 so open the file as read/write
		pFile = fopen( fileName, "rb+" );

		if ( pFile == NULL )
		{
			ErrOut("MsgBoardGetMaxMsgSN() Failed to create bbi file, aborting post\n");
			return 0;
		}
		else
		{
			// Set file pointer to BOF
			if ( fseek(pFile, 0, SEEK_SET) )
			{
				ErrOut("MsgBoardGetMaxMsgSN() Failed to set pFile to BOF in bbi file\n");
				fclose( pFile );
				return 0;
			}
			else
			{
				// Convert maxSN to an char array
				LongToCharPtr(maxSN, maxMsgSN);

				// Write out new maxSN for this post
				if ( fwrite( maxMsgSN, sizeof(char), 4, pFile) != 4 )
				{
					ErrOut("MsgBoardGetMaxMsgSN() Error writing to bbi file, aborting post\n");
					fclose( pFile );
					return 0;
				}

				// Now jump to EOF to write next msgbbiSegment info
				if ( fseek(pFile, 0, SEEK_END) )
				{
					ErrOut("MsgBoardGetMaxMsgSN() Failed to set pFile to EOF in bbi file\n");
					fclose( pFile );
					return 0;
				}
			}
		}
	}

	// Set bytes to proper values in bbi message array
	LongToCharPtr(maxSN, msgbbiSegment);
	msgbbiSegment[6]  = msg2Post[3];     // 05 = user posted message, 0xFF and lower  is a quest post (0xFF is escort quest)

	// Calculate current time and date ( for later bulletin board maintenance routine )
	time( &now );
	timeOfPost = *localtime( &now );

	msgbbiSegment[7]  = (timeOfPost.tm_yday+1)/256;
	msgbbiSegment[8]  = (timeOfPost.tm_yday+1)%256;

	// If this is an autoPost then set the CHAR or ITEM serial number in order to mark it for deletion
	// after the quest is done.
	if ( autoPost )
	{
		msgbbiSegment[13]  = msg2Post[4];  // CHAR or ITEM SN1
		msgbbiSegment[14]  = msg2Post[5];  // CHAR or ITEM SN2
		msgbbiSegment[15]  = msg2Post[6];  // CHAR or ITEM SN3
		msgbbiSegment[16]  = msg2Post[7];  // CHAR or ITEM SN4
	}

	// GLOBAL   Posts start at 01 00 00 00 -> 01 FF FF FF
	// REGIONAL Posts start at 02 00 00 00 -> 02 FF FF FF
	// LOCAL    Posts start at 03 00 00 00 -> 03 FF FF FF
	switch ( msgType )
	{
		// LOCAL post
	case LOCALPOST:
		// Check to see if Maximum number of posts have been reached
		//if ( ((maxSN-0x03000000) >= MAXPOSTS) || (maxSN >= 0xFFFFFFFF) )
		if ( maxSN >= 0x03FFFFFF )
		{
			ErrOut("MsgBoardGetMaxMsgSN() Max posts reached in %s\n", fileName );
			fclose( pFile );
			return 0;
		}
		break;

		// REGIONAL post
	case REGIONALPOST:
		// Check to see if Maximum number of posts have been reached
		//if ( ((maxSN-0x02000000) >= MAXPOSTS) || (maxSN >= 0x02FFFFFF) )
		if ( maxSN >= 0x02FFFFFF )
		{
			ErrOut("MsgBoardGetMaxMsgSN() Max posts reached in %s\n", fileName );
			fclose( pFile );
			return 0;
		}
		break;

		// GLOBAL POST (any other value)
	case GLOBALPOST:
		// Check to see if Maximum number of posts have been reached
		//if ( ((maxSN-0x01000000) >= MAXPOSTS) || (maxSN >= 0x01FFFFFF) )
		if ( maxSN >= 0x01FFFFFF )
		{
			ErrOut("MsgBoardGetMaxMsgSN() Max posts reached in %s\n", fileName );
			fclose( pFile );
			return 0;
		}
		break;

		// Invalid post type
	default:
		ErrOut("MsgBoardGetMaxMsgSN() Invalid post type, aborting post\n");
		fclose( pFile );
		return 0;
	}

	// Write out bbi message array to file
	if ( fwrite(msgbbiSegment, sizeof(char), (sizeof(msgbbiSegment)-1), pFile) != (sizeof(msgbbiSegment)-1) )
	{
		ErrOut("MsgBoardGetMaxMsgSN() Error writing to bbi file, aborting post\n");
		fclose( pFile );
		return 0;
	}

	// Close bbi file for the final time
	fclose( pFile );

	// Return int value of new posts serial number
	return maxSN;

}



//////////////////////////////////////////////////////////////////////////////
// FUNCTION:    MsgBoardPost( int s, int msgType, int autoPost )
//
// PURPOSE:     Called when a user clicks on the Post button after typing in a
//              message or the server is creating a quest and wants to post
//              a message to a regional board
//              if message posted by a user, then autoPost = 0
//              if message posted by system, then autoPost = 1

//
// PARAMETERS:  s           Players/NPC serial number
//              msgType     Type of post (LOCAL, REGIONAL, GLOBAL)
//              autoPost    0 = message posted by a user
//                          1 = message posted by system
//
// RETURNS:     int         0 = Failed to post message
//                          1 = Post successful
//////////////////////////////////////////////////////////////////////////////
int MsgBoardPost( int s, int msgType, int autoPost )
{
	// WRITE FILE OUT (POST MESSAGE)

	// 50 chars for prefix and 4 for the extension plus the ending NULL
	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	int loopexit3=0;
	char        fileName[256] = "";
	FILE        *pFile = NULL;

	int         origMsgSize           = 0;
//	int         tempMsgSize           = 0;
	int         newMsgSize            = 0;
	SERIAL      newMsgSN              = 0;
	SERIAL      maxMsgSN              = 0;
	SERIAL      isReply               = 0;
	SERIAL      msgBoardSerial        = 0;
	int         x, y, z, pos, offset;

	UI08        msgHeader[17]         = "";
	char        msgSubject[257]       = "";
	char        msgBody[MAXBUFFER]    = "";
	char        msgAuthor[52]         = "";   // Maximum name size from char_st (Size + Name)
	char        msgDate[17]           = "";   // Maximum date size based on Size + "Day ### @ hh:mm" format

	struct tm   timeOfPost;
	time_t      now;

	// If this is a users post (done from the client) then copy the client buffer[s] into our buffer
	if ( !autoPost )
	{
		memcpy( msg2Post, buffer[s], (buffer[s][1]*256 + buffer[s][2]) );

		// Determine what type of post this is supposed to be and then set the proper file name
		// Also, if this is a reply to a base post, then abort posting if the reply is to a
		// GLOBAL or REGIONAL message.  No one can reply to GLOBAL or REGIONAL messages as they
		// as for informational purposes only (discussions should be taken offline).  There is no
		// reason to reply to a quest post execpt to fill up the message board.
		isReply = LongFromCharPtr(msg2Post +8);

		// If this is a reply to anything other than a LOCAL post, abort
		if ( (isReply>0) && (isReply<0x03000000) )
		{
#ifdef DEBUG
			ErrOut("MsgBoardPost() Attempted reply to a global or regional post\n");
#endif
			sysmessage( s, TRANSLATE("You can not reply to global or regional posts") );
			return 0;
		}
	}

	// If everything passed the check above then,
	// Determine the new messages serial number for the type of post being done
	maxMsgSN = MsgBoardGetMaxMsgSN( msgType, autoPost );

	// If the value returned is zero, then abort the posting
	if ( maxMsgSN == 0 )
	{
		ErrOut("MsgBoardPost() Could not retrieve a valid message serial number\n");
		sysmessage( s, TRANSLATE("Post failed!") );
		return 0;
	}

	// If a MSBBOARDPATH has been define in the SERVER.cfg file, then use it
	if (SrvParms->msgboardpath)
		strcpy( fileName, SrvParms->msgboardpath );

	msgBoardSerial = LongFromCharPtr(msg2Post +4);

	switch ( msgType )
	{
		// LOCAL post
	case LOCALPOST:
		// Get Message Board serial number from message buffer
		sprintf( temp, "%08x.bbp", msgBoardSerial);
		break;

		// REGIONAL post
	case REGIONALPOST:
		// set the Message Board fileName to the proper region number
		if ( autoPost )
		{
			P_CHAR pc_s = pointers::findCharBySerial(msgBoardSerial);
			if(ISVALIDPC(pc_s))
				sprintf( temp, "region%d.bbp", pc_s->region );
		}
		else
		{
			P_ITEM pi_s = pointers::findItemBySerial(msgBoardSerial);
			if(ISVALIDPI(pi_s))
				sprintf( temp, "region%d.bbp", calcRegionFromXY( pi_s->getPosition() ) );
		}
		break;

		// GLOBAL POST
	case GLOBALPOST:
		strcpy( temp, "global.bbp" );
		break;

		// Invalid post type
	default:
		ErrOut("MsgBoardPost() Invalid post type, aborting post\n");
		sysmessage( s, TRANSLATE("Invalid post type!" ));
		return 0;
	}

	// Append file name to end of path
	strcat( fileName, temp );

	// Open the file for appending
	pFile=fopen( fileName, "ab+");

	// If we couldn't open the file, send an error message to client
	if ( pFile == NULL )
	{
		ErrOut("MsgBoardPost() Unable to open bbp file, aborting post\n");
		return 0;
	}

	// Calculate original size of the message that the client sent to us
	origMsgSize = ShortFromCharPtr(msg2Post +1);

	// Get the messages header info (packet type, size, type, board S/N, parent msg S/N(replies only))
	for ( x=0; x<12; x++ )
		msgHeader[x] = msg2Post[x];


	// Set new messages serial number to maxMsgSN from the bbi file
	LongToCharPtr(maxMsgSN, msgHeader +8);

	// Get the new messages serial number (which is its post position on the board- anything other than 00 00 00 00
	// (base post) is a reply to a specific message ID )
	newMsgSN = LongFromCharPtr(msg2Post +8);

	// If the newMsgSN is 0 then it is a base post, other wise it is a reply to a previous post
	if ( newMsgSN )
	{
		// Create the proper parent message ID segment for the new post
		msgHeader[12] = msg2Post[8];
		msgHeader[13] = msg2Post[9];
		msgHeader[14] = msg2Post[10];
		msgHeader[15] = msg2Post[11];
	}
	else
	{
		// If this isn't a reply post, the parent message SN fields should be 0x00
		msgHeader[12] = 0;
		msgHeader[13] = 0;
		msgHeader[14] = 0;
		msgHeader[15] = 0;
	}

	newMsgSize = origMsgSize + 4;  // added 4 bytes for parent msg ID

	// This is the position within the msg2Post[] array
	pos = x; // Should equal to 12 (pointing to size of Subject field)

	// Get the messages subject info (size, subject)
	y = msg2Post[pos];  // get the size of the subject

	for ( x=0; x<=y; x++ )    // Do while we get all bytes
		msgSubject[x] = msg2Post[pos+x];  // get the subject message (size and data)

	pos += x;

	// Get the messages body info (body section, body size, body data)
	z = msg2Post[pos];     // Total number of NULL's in Body of message
	offset = 0;

	// Check if body of post is empty (NULL)
	if ( z )
	{
		// Get the FIRST Body segment size + 2 for pre segment size bytes
		y = msg2Post[pos+1] + 2;

		// Loop until number of remaining NULLS equal zero
		while (z && (++loopexit3 < MAXLOOPS) )
		{
			// Store Body segment into msgBody ( continue until NULL reached )
			for ( x=0; x<y; x++ )
				msgBody[x+offset] = msg2Post[pos+x];

			offset += x;
			pos += x;

			// Size of NEXT Body segment
			y = msg2Post[pos] + 1;

			// Decrement NULL count (processed one segment of the Body)
			z--;
		}
	}
	else  // Body is empty (NULL) so write out set msgBody[] = "\x00"
	{
		msgBody[0] = msg2Post[pos];
		offset++;
		//pos++;
	}

	// Get the Authors info and create msgAuthor packet
	// if this was a user posting, get the characters name, other wise leave it blank
	if ( !autoPost ) {
		P_CHAR pc_cur=MAKE_CHAR_REF(currchar[s]);
		if(ISVALIDPC(pc_cur))
			strncpy( &msgAuthor[1], pc_cur->getCurrentNameC(), (sizeof(msgAuthor)-1) );
	}
	msgAuthor[0] = strlen(&msgAuthor[1]) + 1;  // get the length of the name + 1 for null

	newMsgSize += (msgAuthor[0]+1);   // Update the new total length of the message
	// + 1 is for the byte giving the size of Author segment

	// Calculate current time and date
	time( &now );
	timeOfPost = *localtime( &now );

	// Create msgDate data packet
	sprintf( &msgDate[1], "Day %i @ %i:%02i",
		(timeOfPost.tm_yday+1),
		timeOfPost.tm_hour,
		timeOfPost.tm_min );

	// get the length of the date + 1 for null
	msgDate[0] = strlen(&msgDate[1]) + 1;

	// Update the new total length of the message (+ 1 is for the byte giving the size of Date segment)
	newMsgSize += (msgDate[0]+1);

	// Start writing information out to a file
	// msgHeader + sizeof(msgAuthor) + msgAuthor

	// Write out the msgHeader
	ShortToCharPtr(newMsgSize, msgHeader +1);
	fwrite( msgHeader, sizeof(char), (sizeof(msgHeader)-1), pFile );

	// Write out the msgAuthor
	fwrite( msgAuthor, sizeof(char), (msgAuthor[0]+1), pFile );

	// Write out the msgSubject
	fwrite( msgSubject, sizeof(char), (msgSubject[0]+1), pFile );

	// Write out the msgDate
	fwrite( msgDate, sizeof(char), (msgDate[0]+1), pFile );

	// Write out the msgBody
	fwrite( msgBody, sizeof(char), offset, pFile );

	// Close the file
	fclose( pFile );

	// if this was a user post, then immediately update the message board with the newly created message
	if ( !autoPost )
	{
		// Add item   |s1|s2|s3|s4|m1|m2|??|#i|#i|x1|x2|y1|y2|b1|b2|b3|b4|c1|c2
		//[RECV:20] 25 40 2b 38 1a 0e b0 00 00 00 00 00 00 00 40 07 ba 3d 00 00
		char addItem[21] = "\x25\x00\x00\x00\x00\x0e\xb0\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

		// Insert posts serial number into addItem[] packet
		addItem[1]  = msgHeader[8];
		addItem[2]  = msgHeader[9];
		addItem[3]  = msgHeader[10];
		addItem[4]  = msgHeader[11];

		// Insert posts bulleting board serial number into addItem[] packet
		addItem[14] = msgHeader[4];
		addItem[15] = msgHeader[5];
		addItem[16] = msgHeader[6];
		addItem[17] = msgHeader[7];

		// Setup buffer to expect to receive an ACK from the client for this posting
		postCount[s]       = 1;
		postAckCount[s]    = 0;
		postAcked[s][0][0] = msgHeader[8];
		postAcked[s][0][1] = msgHeader[9];
		postAcked[s][0][2] = msgHeader[10];
		postAcked[s][0][3] = msgHeader[11];

		// Send "Add Item to Container" message to client
		Xsend( s, addItem, 20);
	}
	// Return success (the new message serial number)
	return maxMsgSN;

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:    MsgBoardOpenPost( int s )
//
// PURPOSE:     Opens a posting when double-clicked in order to view the
//              full message.
//
// PARAMETERS:  s           Players serial number
//
// RETURNS:     void
//////////////////////////////////////////////////////////////////////////////
void MsgBoardOpenPost( int s )
{
	// READ IN bbp FILE  (Client dbl-clicked on posted message on message board list)
	// Get Message Board serial number from message buffer

	// 50 chars for prefix and 4 for the extension plus the ending NULL
 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	char fileName[256] = "";
	FILE *file = NULL;

	int loopexit4=0,loopexit2=0;
	SERIAL msgSN    = 0;
	SERIAL msgBoardSerial  = 0;
	int msgBytes        = 0;
	int dateBytes       = 0;
	int authorBytes     = 0;
	int subjectBytes    = 0;
	int foundMsg        = 0;
	int x, y, z;


	// Calculate the messages SN to determine which file to open
	// GLOBAL   Posts start at 01 00 00 00 -> 01 FF FF FF
	// REGIONAL Posts start at 02 00 00 00 -> 02 FF FF FF
	// LOCAL    Posts start at 03 00 00 00 -> 03 FF FF FF

	// If a MSBBOARDPATH has been define in the SERVER.cfg file, then use it
	if (SrvParms->msgboardpath)
		strcpy( fileName, SrvParms->msgboardpath );


	msgBoardSerial = LongFromCharPtr(buffer[s] +4);
	msgSN = LongFromCharPtr(buffer[s] +8);

	// Is msgSN within the GLOBAL post range
	if ( (msgSN>=0x01000000) && (msgSN<=0x01FFFFFF) )
	{
#ifdef DEBUG
		sysmessage( s, "Opening GLOBAL.bbp posting");
#endif
		strcat( fileName, "global.bbp" );
		file = fopen( fileName, "rb" );
	}
	// Is msgSN within the REGIONAL post range
	else if ( (msgSN>=0x02000000) && (msgSN<=0x02FFFFFF) )
	{
#ifdef DEBUG
		sysmessage( s, "Opening REGIONAL.bbp posting");
#endif

		P_ITEM p_msgboard = pointers::findItemBySerial(msgBoardSerial);
		sprintf( temp, "region%d.bbp", calcRegionFromXY( p_msgboard->getPosition() ) );
		strcat( fileName, temp );
		file = fopen( fileName, "rb" );
	}
	// Is msgSN within the LOCAL post range
	else if ( (msgSN>=0x03000000) && (msgSN<=(SI32)0xFFFFFFFF) )
	{
#ifdef DEBUG
		sysmessage( s, "Opening LOCAL.bbp posting");
#endif
		sprintf( temp, "%08x.bbp", msgBoardSerial);
		strcat( fileName, temp );
		file = fopen( fileName, "rb" );
	}
	// This msgSN does not fall within a valid range
	else
	{
		ErrOut("MsgBoardOpenPost() Invalid message SN: %08x", msgSN);
		sysmessage( s, TRANSLATE("Post not valid, please notify GM"));
		return;
	}

	if (file == NULL) return;   // Put file not found error control here
	if (feof(file)) return;     // Put end of file error control here

	// Find Message ID that has been requested
	while ( !foundMsg && (++loopexit4 < MAXLOOPS))
	{
		//                                0       1     2      3      4   5   6      7   8    9    10   11   12   13   14   15
		// Read In the first 12 bytes |PacketID|Size1|Size2|MsgType|bSn1|bSn2|bSn3|bSn4|mSn1|mSn2|mSn3|mSn4|pmSN|pmSN|pmSN|pmSN|
		for ( x=0; x<12; x++)
			msg[x] = fgetc( file );

		// If we have reached the EOF then stop searching
		if ( feof(file) )
		{
			ErrOut("MsgBoardEvent() case 3: message not found \n");
			break;
		}

		// Find post that was ACK'd by client
		if ( LongFromCharPtr(msg +8) == msgSN )
		{
			// Don't forget to set the flag to stop searching for the message when we find it
			foundMsg = 1;

			// Jump ahead 4 bytes in bbp file to skip
			// the parent message serial number section as it is not required
			if ( fseek(file, 4, SEEK_CUR) )
				ErrOut("MsgBoardEvent() case 3 : failed to seek Author segment\n");

			// Read in  author, subject and date info to pass back to client (DO NOT SEND BODY of msg)
			// Count the total number of bytes in posting (not including body as it isn't sent to client)
			msgBytes = x;

			// Get size of Author segment
			msg[msgBytes] = fgetc( file );
			if ( feof(file) ) return;
			msgBytes++;

			// Store size of Author segment
			authorBytes = msg[msgBytes-1];

			// Fill in msg[] with Author data
			for ( x=0; x<authorBytes; x++)
			{
				msg[msgBytes+x] = fgetc( file );
				if ( feof(file) ) return;
			}
			msgBytes += x;
			// Get size of Subject segment
			msg[msgBytes] = fgetc( file );
			if ( feof(file) ) return;
			msgBytes++;

			// Store size of Subject segment
			subjectBytes = msg[msgBytes-1];

			// Fill in msg[] with Subject data
			for ( x=0; x<subjectBytes; x++)
			{
				msg[msgBytes+x] = fgetc( file );
				if ( feof(file) ) return;
			}
			msgBytes += x;

			// Get size of Date segment
			msg[msgBytes] = fgetc( file );
			if ( feof(file) ) return;
			msgBytes++;

			// Store size of Date segment
			dateBytes = msg[msgBytes-1];

			// Fill in msg[] with Date data
			for ( x=0; x<dateBytes; x++)
			{
				msg[msgBytes+x] = fgetc( file );
				if ( feof(file) ) return;
			}
			msgBytes += x;

			// Weird stuff ???  don't know what it does...  Always has to be some thing to screw stuff up.
			// but if it isn't inserted into the message..... KABOOM!!!  no more client (page faults the client!)
			// 29 bytes
			char weird[30]="\x01\x90\x83\xea\x06\x15\x2e\x07\x1d\x17\x0f\x07\x37\x1f\x7b\x05\xeb\x20\x3d\x04\x66\x20\x4d\x04\x66\x0e\x75\x00\x00";

			// Fill in weird portion between DATE and BODY
			for ( x=0; x<29; x++ )
				msg[msgBytes+x] = weird[x];

			msgBytes += x;
			// Hope this works!!!
			// Yup it worked!   Whew, thats a load off...


			// Get the messages body info (body section, body size, body data)
			z = fgetc( file );     // Total number of NULL's in Body of message
			msg[msgBytes] = z;
			msgBytes++;

			loopexit2=0;
			while (z 	&& (++loopexit2 < MAXLOOPS) )  // Loop until number of remaining NULLS equal zero
			{
				y = fgetc( file );     // Get the FIRST Body segment size
				if ( feof(file) )
				{
					fclose( file );
					break;
				}

				msg[msgBytes] = y;
				msgBytes++;

				// Get Body segment into msg[] ( continue until 0x00 reached )
				for ( x=0; x<y; x++ )
				{
					msg[msgBytes+x] = fgetc( file );
				}

				msgBytes += x;

				// Decrement NULL count (processed one segment of the Body)
				z--;
			}

			ShortToCharPtr(msgBytes, msg +1);
			msg[3] = 2;              // Set packet 0x71 message type to 0x02 (send full message)
   }
   else // If this isn't the message were looking for, jump ahead to next message
   {
	   // Since we didn't find the message in this pass, get this messages size and jump ahead
	   msgBytes += ShortFromCharPtr(msg +1);

	   // Jump to next message
	   if ( fseek(file, msgBytes, SEEK_SET) )
		   ErrOut("MsgBoardEvent() case 3 : failed to seek next message\n");
   }

  }// End of while loop

  // Close bbp file and return
  fclose(file);

  // Send message to client
  Xsend(s, msg, msgBytes);

}


//////////////////////////////////////////////////////////////////////////////
// FUNCTION:    MsgBoardRemovePost( int s )
//
// PURPOSE:     Marks the posting for a specific message for deletion, thereby
//              removing it from the bulletin boards viewable list.
//
// PARAMETERS:  s        Serial number of the player removing the post
//
// RETURNS:     void
//////////////////////////////////////////////////////////////////////////////
void MsgBoardRemovePost( int s )
{
 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	// Sample REMOVE POST message from client
	//             | 0| 1| 2| 3| 4| 5| 6| 7| 8| 9|10|11|
	//             |p#|s1|s2|mt|b1|b2|b3|b4|m1|m2|m3|m4|
	// Client sends 71  0  c  6 40  0  0 18  1  0  0  4

	// Read bbi file to determine messages on boards list
	// Find the post and mark it for deletion
	// thereby removing it from the bulletin boards list

	// int s = calcSerFromChar( serial );

	FILE *file = NULL;
	// 50 chars for prefix and 4 for the extension plus the ending NULL
	char fileName[256] = "";

	SERIAL msgSN      = 0;
	SERIAL msgBoardSN = 0;

	// Get the integer value of the message serial number
	msgSN = LongFromCharPtr(buffer[s] +8);

	// Calculate the Bulletin Boards serial number
	msgBoardSN = LongFromCharPtr(buffer[s] +4);

	P_ITEM p_msgboard = pointers::findItemBySerial(msgBoardSN);

	// Switch depending on what type of message this is:
	// GLOBAL = 0x01000000 -> 0x01FFFFFF
	// REGION = 0x02000000 -> 0x02FFFFFF
	// LOCAL  = 0x03000000 -> 0x03FFFFFF
	switch ( buffer[s][8] )
	{
	case 0x01:
		{
			// GLOBAL post file
			strcpy( temp, "global.bbi" );
			break;
		}

	case 0x02:
		{
			// REGIONAL post file
			sprintf( temp, "region%d.bbi", calcRegionFromXY( p_msgboard->getPosition() ) );
			break;
		}

	default:
		{
			// LOCAL post file
			sprintf( temp, "%08x.bbi", msgBoardSN);
			break;
		}
	}

	// If a MSBBOARDPATH has been define in the SERVER.cfg file, then use it
	if (SrvParms->msgboardpath)
		strcpy( fileName, SrvParms->msgboardpath );

	// Create the full path to the file we need to open
	strcat( fileName, temp );
	file = fopen( fileName, "rb+" );

	// If the file exists continue, othewise abort with an error
	if ( file != NULL )
	{
		// Ignore first 4 bytes of bbi file as this is reserverd for the current max message serial number being used
		if ( fseek( file, 4, SEEK_SET ) )
		{
			ErrOut("MsgBoardRemovePost() failed to seek first message seg in bbi\n");
			sysmessage( s, TRANSLATE("Failed to find post to be removed." ));
			return;
		}
		int loopexit=0;

		// Loop until we have reached the end of the file
		while ( !feof(file) && (++loopexit < MAXLOOPS) )
		{
			//  | 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18
			//  |mg1|mg2|mg3|mg4|mo1|mo2|DEL|sg1|sg2|xx1|xx2|yy1|yy2|NS1|NS2|NS3|NS4|co1|co2|
			// "\x40\x1c\x53\xeb\x0e\xb0\x00\x00\x00\x00\x3a\x00\x3a\x40\x00\x00\x19\x00\x00";

			// Fill up the msg with data from the bbi file
			if ( fread( msg, sizeof(char), 19, file ) != 19 )
			{
				ErrOut("MsgBoardRemovePost() Could not find message to mark deleted\n");
				if ( feof(file) ) break;
			}

			if ( LongFromCharPtr(msg) == msgSN )
			{
				// Jump back to the DEL segment in order to mark the post for deletion
				fseek( file, -13, SEEK_CUR );

				// Write out the mark for deletion value (0x00)
				fputc( 0, file );

				// Inform user that the post has been removed
				sysmessage( s, TRANSLATE("Post removed.") );

				// We found the message we wanted so break out and close the file
				break;
			}

		}
	}

	// Close bbi file
	if ( file ) fclose( file );

	// Put code to actually remove the post from the bulletin board here.
	// Posted messages use serial numbers from 0x01000000 to 0x03FFFFFF so they
	// will not interfere with other worldly objects that start at serial number 0x40000000
	// If, however, this is a problem, then simply remove this portion of code and the
	// messages will not be removed on the client but will still be marked for removal
	// in the message board files.
	SendDeleteObjectPkt(s, msgSN);
	// Remove code above to prevent problems with client if necessary

	return;

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:    MsgBoardRemoveGlobalPostBySerial( int nPostSerial )
//
// PURPOSE:     Marks the posting for a specific message in the GLOBAL
//              bulletin board file for deletion, thereby
//              removing it from the bulletin boards viewable list.
//
// PARAMETERS:  nPostSerial Serial number of post to be removed
//
// RETURNS:     TRUE  Post was successfully found and marked as deleted
//              FALSE Post could not be found
//////////////////////////////////////////////////////////////////////////////
bool MsgBoardRemoveGlobalPostBySerial( SERIAL nPostSerial )
{
	// Sample REMOVE POST message from client
	//             | 0| 1| 2| 3| 4| 5| 6| 7| 8| 9|10|11|
	//             |p#|s1|s2|mt|b1|b2|b3|b4|m1|m2|m3|m4|
	// Client sends 71  0  c  6 40  0  0 18  1  0  0  4

	// Read bbi file to determine messages on boards list
	// Find the post and mark it for deletion
	// thereby removing it from the bulletin boards list

 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	bool bReturn = true;
	FILE *file = NULL;
	// 50 chars for prefix and 4 for the extension plus the ending NULL
	char fileName[256] = "";

	// GLOBAL post file
	strcpy( temp, "global.bbi" );

	// If a MSBBOARDPATH has been define in the SERVER.cfg file, then use it
	if (SrvParms->msgboardpath)
		strcpy( fileName, SrvParms->msgboardpath );

	// Create the full path to the file we need to open
	strcat( fileName, temp );
	file = fopen( fileName, "rb+" );

	// If the file exists continue, othewise abort with an error
	if ( file != NULL )
	{
		// Ignore first 4 bytes of bbi file as this is reserverd for the current max message serial number being used
		if ( fseek( file, 4, SEEK_SET ) )
		{
			ErrOut("MsgBoardRemoveGlobalPostBySerial() failed to seek first message seg in bbi\n");
      fclose( file );
			return false;
		}

		int loopexit=0;
		// Loop until we have reached the end of the file
		while ( !feof(file) && (++loopexit < MAXLOOPS) )
		{
			//  | 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18
			//  |mg1|mg2|mg3|mg4|mo1|mo2|DEL|sg1|sg2|xx1|xx2|yy1|yy2|NS1|NS2|NS3|NS4|co1|co2|
			// "\x40\x1c\x53\xeb\x0e\xb0\x00\x00\x00\x00\x3a\x00\x3a\x40\x00\x00\x19\x00\x00";

			// Fill up the msg with data from the bbi file
			if ( fread( msg, sizeof(char), 19, file ) != 19 )
			{
                bReturn = false;
				ErrOut("MsgBoardRemoveGlobalPostBySerial() Could not find message to mark deleted\n");
				if ( feof(file) )
                break;
			}

			if ( LongFromCharPtr(msg) == nPostSerial )
			{
				// Jump back to the DEL segment in order to mark the post for deletion
				fseek( file, -13, SEEK_CUR );

				// Write out the mark for deletion value (0x00)
				fputc( 0, file );

				// We found the message we wanted so break out and close the file
				break;
			}

		}
	}

	// Close bbi file
	if ( file )
    fclose( file );

	return bReturn;

}


//////////////////////////////////////////////////////////////////////////////
// FUNCTION:    MsgBoardEvent( int s )
//
// PURPOSE:     Handle all of the different Bulletin Board message types.
//
// PARAMETERS:  s           Players serial number
//
// RETURNS:     void
//////////////////////////////////////////////////////////////////////////////
void MsgBoardEvent(int s)
{
	// Message \x71 has numerous uses for the Bulletin Board
	// so we need to get the type of message from the client first.
	int msgType        = buffer[s][3];

	// If this was due to a double click event
	if ( buffer[s][0]==0x06 )
		msgType = 0;

	switch (msgType)
	{
	case 0:  // Server->Client: Prepare to draw message board, send title + misc info to client
		{        // show message board code goes here (called by doubleclick())
			MsgBoardOpen( s );
			break;
		}

	case 1:  // Server->Client: Send list of message postings (subjects only)
		{        // message sent out by CASE 4: below
			break;
		}

	case 2:  // Server->Client: Sending body of message to client after subject dbl-clicked
		{        // message sent out by CASE 3: below
			break;
		}

	case 3:  // Client->Server: Client has dbl-clicked on subject, requesting body of message
		{        // Example  [SEND:12] 71 00 0c 03 40 07 ba 3d 40 1c 53 eb
			MsgBoardOpenPost( s );
			break;
		}

	case 4:  // Client->Server: Client has ACK'ed servers download of posting serial numbers
		{
			// Check to see whether client has ACK'd all of our message ID's before proceeding
			postAckCount[s]++;
			//ConOut(" pstAckCont=%d        postCount=%d\n", postAckCount[s], postCount[s]);
			if ( postAckCount[s] != postCount[s] )
				return;

			// Server needs to handle ACK from client that contains the posting serial numbers
			MsgBoardList( s );
			break;
		}

	case 5:  // Client->Server: Client clicked on Post button (either from the main board or after pressing the Reply)
		{        //                 Reply just switches to the Post item.

			P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
			VALIDATEPC(pc);
			// Check privledge level against server.cfg msgpostaccess
			
			if ( (pc->IsGM()) || (SrvParms->msgpostaccess) )
				MsgBoardPost( s, pc->postType, 0 );
			else
				pc->sysmsg(TRANSLATE("Thou art not allowed to post messages."));

			break;
		}

	case 6:  // Remove post from Bulletin board
		{
			P_CHAR pc=MAKE_CHAR_REF(currchar[s]);
			VALIDATEPC(pc);
			//             |p#|s1|s2|mt|b1|b2|b3|b4|m1|m2|m3|m4|
			// Client sends 71  0  c  6 40  0  0 18  1  0  0  4
			if ( (pc->IsGM()) || (SrvParms->msgpostremove) )
				MsgBoardRemovePost( s );
			break;
		}


	default:
		{
			ErrOut("MsgBoardEvent() Unknown msgType:%x for message: %x\n", buffer[s][3], buffer[s][0]);
			break;
		}
	}

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:    MsgBoardPostQuest( int serial, int questType )
//
// PURPOSE:     Used to read in the template for displaying a typical quest
//              message based on the type of quest and the serial number of
//              the NPC or Item.  Certain parameters can be used as variables
//              to replace certain NPC such as %n for NPC name, %t NPC title
//              etc.  See the MSGBOARD.SCP file for an example.
//
// PARAMETERS:  serial      NPC or Item serial number
//              questType   Type of quest being posted (used to determine
//                          if the item_st or char_st should be used to
//                          for replacing paramters in the script.
//
// RETURNS:     0           Failed to post message
//              PostSerial  Serial number of the post if successfull
//
// NOTES:       Currently only escort quests work so this function us still
//              in its early stages in regards to the questType parameter.
//////////////////////////////////////////////////////////////////////////////
int MsgBoardPostQuest( int serial, QuestType questType )
{
	TEXT	subjectEscort[]     = "Escort: Needed for the day.";  // Default escort message
	TEXT	subjectBounty[]     = "Bounty: Reward for capture.";  // Default bounty message
	TEXT	subjectItem[]       = "Lost valuable item.";          // Default item message
	TEXT	subject[50]         = "";                             // String that will hold the default subject
	SI32	sectionEntrys[MAXENTRIES];                            // List of SECTION items to store for randomizing

	UI32	listCount           = 0;  // Number of entries under the ESCORTS section, used to randomize selection
	SI32	entryToUse          = 0;  // Entry of the list that will be used to create random message

	SI32	linesInBody         = 0;  // Count of number of lines in body of post
	SI32	lineLength          = 0;  // Length of the line just read in including terminating NULL
	SI32	offset              = 0;  // Offset to next line in buffer
	SI32	numLinesOffset      = 0;  // Offset to the number of lines in body field

	// msg2Post[] Buffer initialization
	msg2Post[0]   = 0x71;   // Packet ID
	msg2Post[1]   = 0x00;   // Size of packet (High byte)
	msg2Post[2]   = 0x00;   // Size of packet (Low byte)

	// This is the type of quest being posted:
	// The value will start arbitrarily at 0xFF and count down
	//    ESCORT = 0xFF (defined in msgboard.h)
	//    BOUNTY = 0xFE
	//    ITEM   = 0xFD
	switch ( questType )
	{
		case ESCORTQUEST:
			msg2Post[3]   = ESCORTQUEST;
			break;

		case BOUNTYQUEST:
			msg2Post[3]   = BOUNTYQUEST;
			break;

		case ITEMQUEST:
			msg2Post[3]   = ITEMQUEST;
			break;
	}

	// Since quest posts can only be regional or global, can use the BullBoard SN fields as CHAR or ITEM fields
	LongToCharPtr(serial, msg2Post +4);  // Normally Bulletin Board SN but used for quests as CHAR or ITEM SN
	LongToCharPtr(0x0000, msg2Post +8);  // Reply to message serial number ( 00 00 00 00 for base post )

    cScpIterator* iter = NULL;
    char script1[1024];
    char script2[1024];

	int loopexit=0;

	safedelete(iter);
	switch ( questType )
	{
	case ESCORTQUEST:
		{
			// Find the list section in order to count the number of entries in the list
			iter = Scripts::MsgBoard->getNewIterator("SECTION ESCORTS");
			if (iter==NULL) return 0;

			// Count the number of entries under the list section to determine what range to randomize within
			int loopexit=0;
			do
			{
				iter->parseLine(script1, script2);
				if ( !(strcmp("ESCORT", script1)) )
				{
					if ( listCount >= MAXENTRIES )
					{
						ErrOut("MsgBoardPostQuest() Too many entries in ESCORTS list [MAXENTRIES=%d]\n", MAXENTRIES );
						break;
					}

					sectionEntrys[listCount] = str2num(script2);
					listCount++;
				}
			} while ( script1[0]!='}' && script1[0]!=0 	&& (++loopexit < MAXLOOPS) );

			safedelete(iter);

			// If no entries are found in the list, then there must be no entries at all.
			if ( listCount == 0 )
			{
				ConOut( "NoX-Wizard: MsgBoardPostQuest() No msgboard.scp entries found for ESCORT quests\n" );
				return 0;
			}

			// Choose a random number between 1 and listCount to use as a message
			entryToUse = RandomNum( 1, listCount );
#ifdef DEBUG
			ErrOut("MsgBoardPostQuest() listCount=%d  entryToUse=%d\n", listCount, entryToUse );
#endif
			// Open the script again and find the section choosen by the randomizer
			char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

			sprintf( temp, "SECTION ESCORT %i", sectionEntrys[entryToUse-1] );
			iter = Scripts::MsgBoard->getNewIterator(temp);

			if (iter==NULL)
			{
				ConOut( "NoX-Wizard: MsgBoardPostQuest() Couldn't find entry %s for ESCORT quest\n", temp );
				return 0;
			}
			break;
		}

  case BOUNTYQUEST:
    {
			// Find the list section in order to count the number of entries in the list
			safedelete(iter);
			iter = Scripts::MsgBoard->getNewIterator("SECTION BOUNTYS");
			if (iter==NULL) return 0;

			// Count the number of entries under the list section to determine what range to randomize within
			loopexit=0;
			do
			{
				iter->parseLine(script1, script2);
				if ( !(strcmp("BOUNTY", script1)) )
				{
					if ( listCount >= MAXENTRIES )
					{
						ErrOut("MsgBoardPostQuest() Too many entries in BOUNTYS list [MAXENTRIES=%d]\n", MAXENTRIES );
						break;
					}

					sectionEntrys[listCount] = str2num(script2);
					listCount++;
				}
			} while ( script1[0]!='}' && script1[0]!=0 	&& (++loopexit < MAXLOOPS)  );

			safedelete(iter);

			// If no entries are found in the list, then there must be no entries at all.
			if ( listCount == 0 )
			{
				ConOut( "NoX-Wizard: MsgBoardPostQuest() No msgboard.scp entries found for BOUNTY quests\n" );
				return 0;
			}

			// Choose a random number between 1 and listCount to use as a message
			entryToUse = RandomNum( 1, listCount );
#ifdef DEBUG
			ErrOut("MsgBoardPostQuest() listCount=%d  entryToUse=%d\n", listCount, entryToUse );
#endif
			// Open the script again and find the section choosen by the randomizer
 			char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

			sprintf( temp, "BOUNTY %i", sectionEntrys[entryToUse-1] );

			safedelete(iter);
			iter = Scripts::MsgBoard->getNewIterator(temp);
			if (iter==NULL)
			{
				ConOut( "NoX-Wizard: MsgBoardPostQuest() Couldn't find entry %s for BOUNTY quest\n", temp );
				return 0;
			}
      break;
    }

	default:
		{
			ConOut( "NoX-Wizard: MsgBoardPostQuest() Invalid questType %d\n", questType );
			return 0;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////
	//  Randomly picked a message, now get the message data and fill in up the buffer //
	////////////////////////////////////////////////////////////////////////////////////

	char  *flagPos = NULL;
	char  flag;
	char  tempString[64];

	if (iter==NULL) return 0;
	strcpy(script1, iter->getEntry()->getFullLine().c_str());		//discards {

	// Insert the default subject line depending on the type of quest selected
	switch ( questType )
	{
	case ESCORTQUEST:
		// Copy the default subject to the generic subject string
		strncpy( subject, subjectEscort, sizeof(subject) );
		break;

	case BOUNTYQUEST:
		// Copy the default subject to the generic subject string
		strncpy( subject, subjectBounty, sizeof(subject) );
		break;

	case ITEMQUEST:
		// Copy the default subject to the generic subject string
		strncpy( subject, subjectItem, sizeof(subject) );
		break;

	default:
		ErrOut("MsgBoardPostQuest() invalid quest type\n");
		return 0;
	}

	// Set the SizeOfSubject field in the buffer and copy the subject string to the buffer
	msg2Post[12] = strlen(subject) + 1;
	strncpy( (char*)&msg2Post[13], subject, msg2Post[12] );

	// Set the offset to one past linesInBody count value of the buffer
	// Point to the Size of the line segment
	offset += ( msg2Post[12] + 13 + 1 );

	// Set the subject
	numLinesOffset = offset - 1;

	loopexit=0;
	int loopexit2=0;
	// Read in the random post message choosen above and fill in buffer body for posting
	char *temp = script1; //xan -> holy shit those globals :(
	while ( (++loopexit < MAXLOOPS)  )
	{
		strcpy(script1, iter->getEntry()->getFullLine().c_str());
		// If we reached the ending curly brace, exit the loop
		if ( !strcmp(script1, "}") ) break;

		flagPos = strchr( script1, '%' );

		// Loop until we don't find anymore replaceable parameters
		loopexit2=0;
		while ( flagPos && (++loopexit2 < MAXLOOPS)  )
		{
			if ( flagPos )
			{
				// Move the the letter indicating what text to insert
				flag = *(flagPos + 1);

				// Save the remainder of the original string temporarily
				strcpy( tempString, (flagPos+2) );

				// Replace the flag with the requested text
				P_CHAR pc_s=pointers::findCharBySerial(serial);
				VALIDATEPCR(pc_s,0);
				switch ( flag )
				{
					// NPC Name
				case 'n':
					{
						strcpy( flagPos, pc_s->getCurrentNameC() );
						strcat( temp, tempString );
						break;
					}

					// LOCATION in X, Y coords
				case 'l':
					{
						Location charpos= pc_s->getPosition();
						sprintf( flagPos, "%d, %d", charpos.x, charpos.y );
						strcat( temp, tempString );
						break;
					}

					// NPC title
				case 't':
					{
						strcpy( flagPos, pc_s->title.c_str() );
						strcat( temp, tempString );
						break;
					}

					// Destination Region Name
				case 'r':
					{
						strcpy( flagPos, region[pc_s->questDestRegion].name );
						strcat( temp, tempString );
						break;
					}

					// Region Name
				case 'R':
					{
						strcpy( flagPos, region[pc_s->region].name );
						strcat( temp, tempString );
						break;
					}

					// Gold amount
				case 'g':
					{
						
						
						char szBounty[32] = "";

						sprintf(szBounty,"%d",pc_s->questBountyReward) ;
						strcpy( flagPos, szBounty );
						strcat( temp, tempString );
						break;
					}

				default:
					{
						break;
					}
				}

				// Look for another replaceable parameter
				flagPos = strchr( flagPos, '%' );
			}
		}
		// Get the length of the line read into 'temp'
		// after being modified with any extra info due to flags (plus one for the terminating NULL)
		lineLength = ( strlen(temp) + 1 );

		msg2Post[offset] = lineLength;
		offset++;

		// Append the string in the msg2Post buffer
		memcpy( &msg2Post[offset], temp, (lineLength+1) );
		offset += lineLength;

		// Increment the total number of lines read in
		linesInBody++;
	}

	safedelete(iter);

	ShortToCharPtr(offset, msg2Post +1);
	msg2Post[numLinesOffset] = linesInBody;

	// If the message is posted to the message board successfully
	// RETURN 1 otherwise RETURN 0 to indicate a failure of some sort
	// Insert the default subject line depending on the type of quest selected
  switch ( questType )
	{
	case ESCORTQUEST:
    // return the value of the new message serial number ( 0 = failed post )
		return MsgBoardPost( 0, REGIONALPOST, 1 );
	case BOUNTYQUEST:
    // return the value of the new message serial number ( 0 = failed post )
		return MsgBoardPost( 0, GLOBALPOST, 1 );
	default:
		ErrOut("MsgBoardPostQuest() invalid quest type or quest not implemented\n");
	}

	// Post failed
	return 0;

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:    MsgBoardQuestEscortCreate( int npcIndex )
//
// PURPOSE:     Used to generate an escort quest based on the currently
//              NPC's serial number
//
// PARAMETERS:  npcIndex     NPC index value in chars[] array
//
// RETURNS:     void
//////////////////////////////////////////////////////////////////////////////
void MsgBoardQuestEscortCreate( int npcIndex )
{
	P_CHAR npc=MAKE_CHAR_REF(npcIndex);
	VALIDATEPC(npc);
	
	// Choose a random region as a destination for the escort quest (except for the same region as the NPC was spawned in)
	int loopexit=0;
	do
	{
		if ( escortRegions )
		{
			// If the number of escort regions is 1, check to make sure that the only
			// valid escort region is not the NPC's current location - if it is Abort
			if ( (escortRegions==1) && (validEscortRegion[0]==npc->region) )
			{
				npc->questDestRegion = 0;
				break;
			}

			npc->questDestRegion = validEscortRegion[RandomNum(0, (escortRegions-1))];
		}
		else
		{
			npc->questDestRegion = 0;  // If no escort regions have been defined in REGIONS.SCP then we can't do it!!
			break;
		}
	} while ( (npc->questDestRegion == npc->region) 	&& (++loopexit < MAXLOOPS)  );

	// Set quest type to escort
	npc->questType = ESCORTQUEST;

	// Make sure they don't move until an player accepts the quest
	npc->npcWander       = WANDER_NOMOVE;                // Don't want our escort quest object to wander off.
	npc->npcaitype = NPCAI_GOOD;                // Remove any AI from the escort (should be dumb, just follow please :)
	npc->questOrigRegion = npc->region;  // Store this in order to remeber where the original message was posted

	// Set the expirey time on the NPC if no body accepts the quest
	if ( SrvParms->escortinitexpire )
		npc->summontimer = ( uiCurrentTime + ( MY_CLOCKS_PER_SEC * SrvParms->escortinitexpire ) );

	// Make sure the questDest is valid otherwise don't post and delete the NPC
	if ( !npc->questDestRegion )
	{
		ErrOut("MsgBoardQuestEscortCreate() No valid regions defined for escort quests\n");
		npc->Delete();
		//deletechar( npcIndex );
		return;
	}

	// Post the message to the message board in the same REGION as the NPC
	if ( !MsgBoardPostQuest(npc->getSerial32(), ESCORTQUEST) )
	{
		ConOut( "NoX-Wizard: MsgBoardQuestEscortCreate() Failed to add quest post for %s\n", npc->getCurrentNameC() );
		ConOut( "NoX-Wizard: MsgBoardQuestEscortCreate() Deleting NPC %s\n", npc->getCurrentNameC() );
		npc->Delete();
		//deletechar( npcIndex );
		return;
	}

	// Debugging messages
#ifdef DEBUG
	ErrOut("MsgBoardQuestEscortCreate() Escort quest for:\n       %s to be escorted to %s\n", npc->name, region[npc->questDestRegion].name );
#endif

}


//////////////////////////////////////////////////////////////////////////////
// FUNCTION:    MsgBoardQuestEscortArrive( int npcIndex, int pcIndex )
//
// PURPOSE:     Called when escorted NPC reaches its destination
//
// PARAMETERS:  npcIndex   Index number of the NPC in the chars[] array
//              pcIndex    Index number of the player in the chars[] array
//
// RETURNS:     void
//////////////////////////////////////////////////////////////////////////////
void MsgBoardQuestEscortArrive( P_CHAR pc, P_CHAR pc_k)
{



 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var
	// Calculate payment for services rendered
	int servicePay = ( RandomNum(0, 20) * RandomNum(1, 30) );  // Equals a range of 0 to 600 possible gold with a 5% chance of getting 0 gold

	// If they have no money, well, oops!
	if ( servicePay == 0 )
	{
		sprintf( temp, TRANSLATE("Thank you %s for thy service. We have made it safely to %s. Alas, I seem to be a little short on gold. I have nothing to pay you with."), pc_k->getCurrentNameC(), region[pc->questDestRegion].name );
		pc->talk( pc_k->getSocket(), temp, 0 );
	}
	else // Otherwise pay the poor sod for his time
	{
		// Less than 75 gold for a escort is pretty cheesey, so if its between 1 and 75, add a randum amount of between 75 to 100 gold
		if ( servicePay < 75 ) servicePay += RandomNum(75, 100);
		addgold( pc_k->getSocket(), servicePay );
		pc_k->playSFX( goldsfx(servicePay) );
		sprintf( temp, TRANSLATE("Thank you %s for thy service. We have made it safely to %s. Here is thy pay as promised."), pc_k->getCurrentNameC(), region[pc->questDestRegion].name );
		pc->talk( pc_k->getSocket(), temp, 0 );
	}

	// Inform the PC of what he has just been given as payment
	pc_k->sysmsg(TRANSLATE("You have just received %d gold coins from %s %s"), servicePay, pc->getCurrentNameC(), pc->title.c_str() );

	// Take the NPC out of quest mode
	pc->npcWander = WANDER_FREELY_CIRCLE;         // Wander freely
	pc->ftargserial = INVALID;            // Reset follow target
	pc->questType = QTINVALID;         // Reset quest type
	pc->questDestRegion = 0;   // Reset quest destination region

	// Set a timer to automatically delete the NPC
	pc->summontimer = ( uiCurrentTime + ( MY_CLOCKS_PER_SEC * SrvParms->escortdoneexpire ) );

	//removefromptr(&cownsp[chars[k].ownserial%HASHMAX], k);
    /*
	pc->own1=255;
	pc->own2=255;
	pc->own3=255;
	pc->own4=255;
	pc->ownserial=-1;
    */
    pc->setOwnerSerial32Only(-1);

}


//////////////////////////////////////////////////////////////////////////////
// FUNCTION:    MsgBoardQuestEscortDelete( int npcIndex )
//
// PURPOSE:     Called when escorted NPC needs to be deleted from the world
//              (just a wrapper in case some additional logic needs to be added)
//
// PARAMETERS:  npcIndex   Index number of the NPC in the chars[] array
//
// RETURNS:     void
//////////////////////////////////////////////////////////////////////////////
void MsgBoardQuestEscortDelete( int npcIndex )
{
	P_CHAR npc=MAKE_CHAR_REF(npcIndex);
	VALIDATEPC(npc);
	npc->Kill();
	npc->Delete();

}


//////////////////////////////////////////////////////////////////////////////
// FUNCTION:    MsgBoardQuestEscortRemovePost( int npcIndex )
//
// PURPOSE:     Marks the posting for a specific NPC for deletion, thereby
//              removing it from the bulletin boards viewable list.
//
// PARAMETERS:  npcIndex   Index number of the NPC in the chars[] array
//
// RETURNS:     void
//////////////////////////////////////////////////////////////////////////////
void MsgBoardQuestEscortRemovePost( int npcIndex )
{
	// Read bbi file to determine messages on boards list
	// Find the post related to this NPC's quest and mark it for deletion
	// thereby removing it from the bulletin boards list

	P_CHAR npc=MAKE_CHAR_REF(npcIndex);
	VALIDATEPC(npc);

	SERIAL npc_serial = npc->getSerial32();

	int loopexit=0;

	FILE *file = NULL;
	// 50 chars for prefix and 4 for the extension plus the ending NULL
	char fileName[256] = "";
 	char temp[TEMP_STR_SIZE]; //xan -> this overrides the global temp var

	// REGIONAL post file
	sprintf( temp, "region%d.bbi", npc->questOrigRegion );

	// If a MSBBOARDPATH has been define in the SERVER.cfg file, then use it
	if (SrvParms->msgboardpath)
		strcpy( fileName, SrvParms->msgboardpath );

	// Set fileName to REGIONAL.bbi
	//sysmessage( s, "Opening REGIONAL.bbi messages");
	strcat( fileName, temp );
	file = fopen( fileName, "rb+" );

	// If the file exists continue, othewise abort with an error
	if ( file != NULL )
	{
		// Ignore first 4 bytes of bbi file as this is reserverd for the current max message serial number being used
		if ( fseek( file, 4, SEEK_SET ) )
		{
			ErrOut("MsgBoardQuestEscortRemovePost() failed to seek first message seg in bbi\n");
			return;
		}

		// Loop until we have reached the end of the file
		while ( !feof(file) 	&& (++loopexit < MAXLOOPS)  )
		{
			//  | 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18     NS = NPC Serial
			//  |mg1|mg2|mg3|mg4|mo1|mo2|DEL|sg1|sg2|xx1|xx2|yy1|yy2|NP1|NS2|NS3|NS4|co1|co2|
			// "\x40\x1c\x53\xeb\x0e\xb0\x00\x00\x00\x00\x3a\x00\x3a\x40\x00\x00\x19\x00\x00";

			// Fill up the msg with data from the bbi file
			if ( fread( msg, sizeof(char), 19, file ) != 19 )
			{
				ErrOut("MsgBoardQuestEscortRemovePost() Could not find message to mark deleted\n");
				if ( feof(file) ) break;
			}

			if ( LongFromCharPtr(msg +13)  == npc_serial )
			{
				// Jump back to the DEL segment in order to mark the post for deletion
				fseek( file, -13, SEEK_CUR );

				// Write out the mark for deletion value (0x00)
				fputc( 0, file );

				// We found the message we wanted so break out and close the file
				break;
			}

		}
	}

	// Close bbi file
	if ( file ) fclose( file );

	return;

}

//////////////////////////////////////////////////////////////////////////////
// FUNCTION:    MsgBoardMaintenance( void )
//
// PURPOSE:     Cleans out old posts which are older than the MSGRETENTION
//              period set in SERVER.cfg and any posts that have been marked
//              for deletion (such as escort quests after they have been
//              accepted or posts that have been "removed" through the user
//              interface.  This is called as a cleanup routine on server
//              startup.  Compacts and reassigns message serial numbers.
//
// PARAMETERS:  void
//
// RETURNS:     void
//
// NOTES:       This function uses the _findfirst() and _findnext() functions
//              which are OS specific.  There will definetly be an issue in
//              compiling this on LINUX because I have no idea what the
//              structure to be passed to the functions needs to be for LINUX.
//              This will definetly have to be #ifdef'd to handle this.
//              Anyone with LINUX experience please feel free to fix it up.
//////////////////////////////////////////////////////////////////////////////
void MsgBoardMaintenance( void )
{
	int loopexit=0, loopexit2=0;
	char                  filePath[256]   = "";
	char                  fileName[256]   = "";
	char                  fileBBITmp[256] = "";
	char                  fileBBPTmp[256] = "";
	UI08                  msg2[MAXBUFFER];

	FILE                  *pBBINew        = NULL;
	FILE                  *pBBIOld        = NULL;

	FILE                  *pBBPNew        = NULL;
	FILE                  *pBBPOld        = NULL;

	// WINDOWS OS structure to be passed to _findfirst() and _findnext()
	// too make this work with LINUX some #ifdef'ing will have to happen.
#if defined(__unix__)
	std::vector<std::string> vecFiles ;
#else
	struct _finddata_t    BBIFile;
	struct _finddata_t    BBPFile;
	long                  hBBIFile = 0;
	long                  hBBPFile = 0;
#endif


	struct tm             currentDate;
	time_t                now;
	int                   dayOfYear;
	int                   postDay;
	int                   postAge;
	int                   count;

	SERIAL                newPostSN  = 0;
	SERIAL                basePostSN = 0;
	unsigned int          sizeOfBBP  = 0;

	UI32		      index = 0 ;

	// Display progress message
	//ErrOut("Bulletin Board Maintenace - Cleaning and compacting BBI & BBP files.\nNoX-Wizard: Progress");
	InfoOut("Bulletin board maintenance... ");

	// Load the MSGBOARDPATH into an array
	// If a MSBBOARDPATH has been define in the SERVER.cfg file, then use it
	if (SrvParms->msgboardpath)
		strcpy( filePath, SrvParms->msgboardpath );
	ConOut("1\n");
	// Set the Tmp file names
	strcpy( fileBBITmp, filePath  );
	strcat( fileBBITmp, "bbi.tmp" );

	strcpy( fileBBPTmp, filePath  );
	strcat( fileBBPTmp, "bbp.tmp" );
	ConOut("2\n");
	// Setup for the starting findfirst() call
#if defined(__unix__)
	vecFiles = MsgBoardGetFile(".bbi",filePath) ;
#else
	strcpy( fileName, filePath );
	strcat( fileName, "*.bbi"  );
#endif
	ConOut("3\n");
	// Calculate current time and date to check if post retention period has expired
	time( &now );
	currentDate = *localtime( &now );
	dayOfYear   = currentDate.tm_yday+1;
	ConOut("4\n");
	// Find a *.BBI file, if none exist then no bulletin boards exist
#if defined(__unix__)
	if(vecFiles.empty())
#else
	if( (hBBIFile = _findfirst( fileName, &BBIFile )) == -1L )
#endif
	{
		//ConOut( "\n\tNo BBI files found." );
		// Setup for the BBP findfirst() call
		ConOut("5\n");
#if defined(__unix__)
		ConOut("Before\n");
		vecFiles = MsgBoardGetFile(".bbp",filePath) ;
		ConOut("After\n");
		if ( vecFiles.empty() )
#else
		strcpy( fileName, filePath );
		strcat( fileName, "*.bbp"  );
		if( (hBBPFile = _findfirst( fileName, &BBPFile )) == -1L )
#endif
			;//ConOut( "\n\tNo BBP files found.\n\tNo Bulletin Board files found to process.\n" );
		else
			ConOut( "[ERROR]\n\tBBP files found with no matching BBI files.\n\t!!! Clean your bulletin board directory !!!\n", filePath );

		// Close the BBI & BBP file handles and exit routine
#ifndef __unix__
		_findclose( hBBIFile );
		_findclose( hBBPFile );
#endif
		ConOut("[DONE]\n");
		return;
	}

	ConOut("FUCK\n");
	// If we made it through the first check then we found a BBI file.
	do
	{
		// Set the number of messages compressed back to 0
		count = 0;

		// Add the file path first then the name of the current BBI file
		strcpy( fileName, filePath     );
	#if defined(__unix__)
		strcat( fileName, vecFiles[index].c_str() );
	#else
		strcat( fileName, BBIFile.name );
	#endif

		// Setting up BBI file for cleaning and compression
		// Rename the BBI file to the temporary file
		rename( fileName, fileBBITmp );

		// Open the new file with the same name as the original BBI file
		pBBINew = fopen( fileName, "wb" );

		// Open the old BBI.TMP file
		pBBIOld = fopen( fileBBITmp, "rb" );

		// Make sure ALL files opened ok
		if ( feof(pBBINew) || feof(pBBIOld) )
		{
			fclose( pBBINew );
			fclose( pBBIOld );

			// Delete the new file if it exists
			remove ( fileName );
				
			// Put the old file name back
			rename( fileBBITmp, fileName );

			ConOut("[ERROR]\n\t  couldn't open all the BBI files needed, aborting!\n");
			break;
		}

		// Set the fileName to the proper extension for the BBP file
		fileName[strlen(fileName)-1] = 'p';

		// Rename the BBP file to the temporary file
		rename( fileName, fileBBPTmp );

		// Open the new file with the same name as the original BBP file
		if ( ( pBBPNew = fopen( fileName, "a+b" ) )==NULL )
		{
			ConOut("[ERROR]\n\t  couldn't open all the BBP files needed, aborting!\n");
			return;
		}

		// Open the old BBI.TMP file
		if ( ( pBBPOld = fopen( fileBBPTmp, "rb" ) )==NULL )
		{
			ConOut("[ERROR]\n\t  couldn't open all the BBP files needed, aborting!\n");
			fclose( pBBPNew );
			return;
		}

		// Make sure ALL files opened ok
		if ( feof(pBBPNew) || feof(pBBPOld) )
		{
			fclose( pBBPNew );
			fclose( pBBPOld );

			// Delete the new file if it exists
			remove ( fileName );

			// Put the old file name back
			rename( fileBBPTmp, fileName );

			ConOut("[ERROR]\n\t  couldn't open all the BBP files needed, aborting!\n");
			break;
		}

		// Determine what type of file this is and initialize its starting post serial number accordingly
		switch ( fileName[strlen(filePath)] )
		{
			// global.bbp
			case 'g':
				newPostSN = 0x01000000;
				break;

			// region.bbp
			case 'r':
				newPostSN = 0x02000000;
				break;

				// local.bbp ( ie 40000000.bbp )
			default:
				newPostSN = 0x03000000;
				break;
		}

		// Save the newPostSN for the BBP file
		basePostSN = newPostSN;

		// Write out the new base SN to the new BBI file
		LongToCharPtr(newPostSN, msg +0);

		if ( fwrite( msg, sizeof(char), 4, pBBINew ) != 4 )
			ConOut("[FAIL]\n\tMsgBoardMaintenance() Failed to write out newPostSN to pBBINew\n");

		// Now lets find out what posts we keep and what posts to remove

		// Fill post2Keep array with all posts that are not marked for deletion or past the retention period
		// Ignore first 4 bytes of bbi file as this is reserverd for the current max message serial number being used
		if ( fseek( pBBIOld, 4, SEEK_SET ) )
		{
			ConOut("[FAIL]\n\tMsgBoardMaintenance() failed to seek to first message segment in pBBIOld\n");
			return;
		}

		// Loop until we have reached the end of the BBI file
		while ( !feof(pBBIOld) 	&& (loopexit < MAXLOOPS) )
		{
			//Increment progress dots
			ConOut(".");

			// Fill up msg with data from the bbi file
			if ( fread( msg, sizeof(char), 19, pBBIOld ) != 19 )
				if ( feof(pBBIOld) ) break;

			// Day that post was created
			postDay = ShortFromCharPtr(msg +7);

			// Calculate the age of this post;
			postAge = dayOfYear - postDay;

			// If postAge is negative, then we are wrapping around the end of the year so add 365 to
			// make it positive
			if ( postAge < 0 )
				postAge += 365;

			//  |Off| 1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18
			//  |mg1|mg2|mg3|mg4|mo1|mo2|???|sg1|sg2|xx1|xx2|yy1|yy2|cS1|cS2|cS3|cS4|co1|co2|
			// "\x40\x1c\x53\xeb\x0e\xb0\x00\x00\x00\x00\x3a\x00\x3a\x40\x00\x00\x19\x00\x00";
			// cS = Charater SN ( only has a value when an NPC posted the message )

			// Check to see whether the post is a dangling quest posting.  Can occur if a quest was
			// generate and posted and then the server crashed without saving the world file.
			// You would then have a post with no quest object related to it.  So we have to
			// scan through the WSC file to figure out if the quest posted has a related object
			// in the world.
			// Message type > 0x05 is a quest && every quest must have an object associated with it
			// So if this is true we must have a quest post with a valid quest object
			if (  (msg[6]>0x05) && ( msg[13] || msg[14] || msg[15] || msg[16]) )
			{
				// Convert the post objects serial number to an int.
				SERIAL postObjectSN  = LongFromCharPtr(msg +13);
				int postQuestType = msg[6];
				int foundMatch    = 0;

				P_CHAR pc_z=pointers::findCharBySerial(postObjectSN);

				switch ( postQuestType )
				{
					case ESCORTQUEST:
						{
							if (ISVALIDPC(pc_z))
							{
								if ( pc_z->npc && ( pc_z->questType>0 ) )
								{
									// Now lets reset all of the escort timers after the server has reloaded the WSC file
									// If this is an Escor Quest NPC
									if ( (pc_z->questType==ESCORTQUEST) )
									{
										// And it doesn't have a player escorting it yet
										if ( pc_z->ftargserial==INVALID )
										{
											// Lets reset the summontimer to the escortinit
											pc_z->summontimer = ( uiCurrentTime + ( MY_CLOCKS_PER_SEC * SrvParms->escortinitexpire ) );
										}
										else // It must have an escort in progress so set the escortactiveexpire timer
										{
											// Lets reset the summontimers to the escortactive value
											pc_z->summontimer = ( uiCurrentTime + ( MY_CLOCKS_PER_SEC * SrvParms->escortactiveexpire ) );
										}
										// Found a matching NPC for this posted quest so flag the post for compression
										foundMatch = 1;
										break;
									}
								}
							}
						}
						break;
					case BOUNTYQUEST:
						{
							if (ISVALIDPC(pc_z))
							{
								if ( (pc_z->npc == 0) && (pc_z->questBountyReward  >  0) )
								{
									// Check that if this is a BOUNTYQUEST that should be removed first!
									if( ( postAge>=SrvParms->bountysexpire ) && ( SrvParms->bountysexpire!=0 ) )
									{
										// Reset the Player so they have no bounty on them
										pc_z->questBountyReward     = 0;
										pc_z->questBountyPostSerial = 0;
									}
									else
									{
										// Found a matching PC for this posted quest and the post
										// has not expired so flag the post for compression
													foundMatch = 1;
									}
									break;
								}
							}
						}
						break;
					default:
						{
							ConOut("[WARNING]\n\tUnhandled QuestType found during maintenance\n");
						}
				}

				// After looking through the char_st for a matching SN for the object that posted the message
				// If we found a match , then everything is ok, other wise there is a dangling post with no
				// related object owning it in the world.
				if ( !foundMatch )
				{
					// Show the operator a message indicating that a dangling post has been removed
					ConOut("[WARNING]\n\tDangling Post found (SN = %08x, questType = %02x) REMOVING!\n", postObjectSN, msg[6] );
					// Set the flag to delete the dangling post
					msg[6]=0x00;
				}
			}

			// If the segment 6 is 0x00 OR the postAge is greater than the MSGRETENTION period
			// then the message is marked for deletion so don't add it to the post2Keep array
			if ( (msg[6]!=0x00 || msg[6]==BOUNTYQUEST) && (postAge<=SrvParms->msgretention) )
			{
				// We found a message to be saved and compressed so lets find the matching
				// message in the BBP file and compress it
				// Loop until we have reached the end of the BBP file
				loopexit2=0;
				while ( !feof(pBBPOld) 	&& (++loopexit2 < MAXLOOPS) )
				{
					//Increment progress dots
					ConOut(".");
					// Fill up msg2 with the first 12 bytes of data from the bbp file
					if ( fread( msg2, sizeof(char), 12, pBBPOld ) != 12 )
						break;
					// Calculate the size of the remainder of this BBP segment ( -12 because we just read the first 12 bytes)
					sizeOfBBP = ShortFromCharPtr(msg2 +1) - 12;
					// Fill up the rest of the msg2 with data from the BBP file
					if ( fread( &msg2[12], sizeof(char), sizeOfBBP, pBBPOld ) != sizeOfBBP )
						if ( feof(pBBPOld) ) break;
					// Check to see that the post SN of the message just read matches the SN in the BBI file
					if ( LongFromCharPtr(msg2 +8) == LongFromCharPtr(msg +0) )
					{
						// This is a match so write the message out to the new BBP file
						// First set the serial number of this post to the newPostSN
						LongToCharPtr(newPostSN, msg2 +8);

						// If this is a BOUNTYQUEST, then make sure you update the
						// PC that references this bounty with the new BountyPostSerial#
						if( msg[6] == BOUNTYQUEST )
						{
        						SERIAL postObjectSN  = LongFromCharPtr(msg +13);
							P_CHAR pc_z=pointers::findCharBySerial(postObjectSN);
							if (ISVALIDPC(pc_z))
							{
								if ( /*(pc_z->getSerial32()== postObjectSN) &&*/ (pc_z->npc== 0) && (pc_z->questBountyReward  >  0) )
								{
									pc_z->questBountyPostSerial = newPostSN;
								}
							}
						}

						// Write out the entire message
						if ( fwrite( msg2, sizeof(char), (sizeOfBBP+12), pBBPNew ) != (sizeOfBBP+12) )
							ConOut("[FAIL]\n\tMsgBoardMaintenance() Failed to write out BBP segment to pBBPNew\n");

						// We found the message we are looking for so exit the loop leaving the file
						// pointer where it is (messages must be in the same order in both files).
						// Update msg[] with newPostSN value
						LongToCharPtr(newPostSN, msg +0);

						// Write out new BBI segment to pBBINew
						if ( fwrite( msg, sizeof(char), 19, pBBINew ) != 19)
							ConOut("[FAIL]\n\t MsgBoardMaintenance() Failed to write out BBI segment to pBBINew\n");
						// Increment the newPostSN
						newPostSN++;
						// Increment the count of the number of times we compressed a message
						count++;
						// We found the message we wanted so break out of this BBP loop
						break;
					}
				}
			}
		}

		// Finished iterating through the BBI & BBP file so set the new max message SN in the BBI file
		// and clean up in order to get ready for the next set of BBI & BBP files

		// Jump to the start of the pBBINew file
		if ( fseek( pBBINew, 0, SEEK_SET ) )
				ConOut("[FAIL]\n\tMsgBoardMaintenance() failed to seek to start of pBBINew file\n");

		// If we the number of times through the loop is 0 then we need to increment the newPostSN
		if ( count == 0 ) newPostSN++;

		// Set the buffer to the newPostSN
		LongToCharPtr(newPostSN-1, msg +0);

		// Write out the newPostSN
		if ( fwrite( msg, sizeof(char), 4, pBBINew ) != 4)
				ConOut("[FAIL]\n\tMsgBoardMaintenance() Failed to write out newPostSN pBBINew\n");


		// Close both BBP files
		fclose( pBBPOld );
		fclose( pBBPNew );

		// Delete the BBP temp file
		remove( fileBBPTmp );

		// Close both BBI files
		fclose( pBBIOld );
		fclose( pBBINew );

		// Delete the BBI temp file
		remove( fileBBITmp );

		loopexit=0;

		index++ ;
#if defined(__unix__)
	}
	while ((vecFiles.size() < index)   && (++loopexit < MAXLOOPS)  );
#else
	}
	while ( (_findnext( hBBIFile, &BBIFile ) == 0) 	&& (++loopexit < MAXLOOPS)  );

	// Close the _findfirst handle
	_findclose( hBBIFile );
#endif
	ConOut("[ OK ]\n");
	return;
}



#if defined(__unix__)

std::vector<std::string> MsgBoardGetFile( char* pattern, char* path)
{

	// Vector of matching files
	std::vector<std::string>   vecFile ;

#ifndef __BEOS__
		long  count ;

		std::string sFilename ;
		std::string sPattern(pattern) ;
		std::string sPath(path) ;

		dirent  **stDirectory ;
		count = scandir(path,&stDirectory,0,alphasort) ;
		if (count > -1)
		{
			while (count--)
			{
				sFilename = sFilename.assign(stDirectory[count]->d_name) ;
				// Was the pattern found?
				if (sFilename.rfind(sPattern) != string::npos)
				{
			 	    // add the file path to it (NO, windows version doesn't either
				    //sFilename = sPath + sFilename ;
				    vecFile.push_back(sFilename) ;
				}
				free(stDirectory[count]) ;
			}
			free(stDirectory) ;
	}

#endif //__BEOS__


	return vecFile ;

}
#endif

}; //namespace

