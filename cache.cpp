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
\brief MUL Caching Stuff
\todo Xan: since mul caching is done ONLY in startup phase, exception handling is bad
*/

#include "cache.h"
#include "debug.h"

#define IOBUFFLEN 2048

#include "nxw_utils.h"
#include "srvparms.h"

MULFile::MULFile(char *fileName, char *mode)
{
  char  localMode[16];

  fmode = -1, ok = 0;

  ioBuff = new char[IOBUFFLEN];

  memset(ioBuff,0x00,sizeof(BYTE)*IOBUFFLEN);

  if(ioBuff != NULL)
  {
    strcpy(localMode,mode);

		if (*mode == 'r')
			fmode = 0;
		else if (*mode == 'w')
			fmode = 1;

    theFile = fopen(fileName, localMode);
#ifdef WIN32
	if ((ServerScp::g_nAutoDetectMuls)&&(theFile==NULL)) {
		char *s;
		char fn[800], fn2[800];
		char *f;
		strcpy(fn2, fileName);
		f = splitPath(fn2);
		s = getHKLMRegistryString("SOFTWARE\\Origin Worlds Online\\Ultima Online\\1.0", "ExePath");
		if (s!=NULL) {
			splitPath(s);
			strcpy(fn,s);
			safedelete(s); //xan, to avoid common mem leaks
			strcat(fn,"\\");
			strcat(fn,f);
			ConOut("*** Can't open %s, trying %s ***\n", fileName, fn);
			theFile = fopen(fn, localMode);
		}
	}
#endif

		if (theFile == NULL)
		{
			ok = 0;
			return; 
		}
		else 
			bSize = bIndex = IOBUFFLEN, ok = 1;  
	}
}

MULFile::~MULFile()
{
  if(ioBuff!=NULL) 
    safedeletearray(ioBuff);
  if(theFile) 
    fclose(theFile);
}

void MULFile::rewind()
{
  fseek(theFile, 0, SEEK_SET);
  bSize = bIndex = IOBUFFLEN;
}

void MULFile::seek(long offset, int whence)
{
  if(whence == SEEK_SET || whence == SEEK_CUR || whence == SEEK_END)
  {
    fseek(theFile, offset, whence);
    bSize = bIndex = IOBUFFLEN;
  }
}

SI08 MULFile::wpgetch()
{
	if (qRefill())
		refill();
	
	if (bSize != 0)
		return ioBuff[bIndex++];
	else 
		return -1;
}

void MULFile::refill()
{
  bSize = fread(ioBuff, sizeof(char), IOBUFFLEN, theFile);
  bIndex = 0;
}

char *MULFile::gets(char *lnBuff, int lnSize)
{
	int loopexit = 0, loopexit2 = 0;
	if (fmode == 0)
	{
		int i;
		
		lnSize--;
		i = 0;
		do
		{
			if (qRefill())
				refill();
			
			loopexit = 0;
			while (i < lnSize && bIndex != bSize && ioBuff[bIndex] != 0x0A &&(++loopexit < MAXLOOPS))
			{
				if (ioBuff[bIndex] == 0x0D)
					bIndex++;
				else 
					lnBuff[i++] = ioBuff[bIndex++];
			}
			
			if (ioBuff[bIndex] == 0x0A) 
			{
				bIndex++;
				if (i != lnSize)
					lnBuff[i++] = 0x0A;
				break;
			}
			
			if (i == lnSize)
			{
				break; 
			}
		} while ((bSize != 0) &&(++loopexit2 < MAXLOOPS));
		
		lnBuff[i] = '\0';
		
		if (bSize != IOBUFFLEN && i == 0)
			return NULL;
		
		return lnBuff;
	}
	else 
		return NULL;
}

int MULFile::puts(char *lnBuff)
{
  if(fmode == 1)
  {
    if(lnBuff)
     return fwrite(lnBuff, sizeof(char), strlen(lnBuff), theFile);
  }

  return -1;
}

void MULFile::getUChar(UI08 *buff, unsigned int number)
{
  for(unsigned int i = 0; i < number; i++)
    buff[i] = static_cast<UI08>(this->wpgetch());
}

void MULFile::getChar(SI08 *buff, unsigned int number)
{
  for(unsigned int i = 0; i < number; i++)
    buff[i] = static_cast<SI08>(this->wpgetch());
}

void MULFile::getText(TEXT *buff, unsigned int number)
{
  for(unsigned int i = 0; i < number; i++)
    buff[i] = static_cast<TEXT>(this->wpgetch());
}

void MULFile::getUShort(UI16 *buff, unsigned int number)
{
  for(unsigned int i = 0; i < number; i++)
  {
    buff[i] = static_cast<UI16>((this->wpgetch() &0xFF));
    buff[i] |= static_cast<UI16>((this->wpgetch() &0xFF) << 8);
  }
}

void MULFile::getShort(SI16 *buff, unsigned int number)
{
  for(unsigned int i = 0; i < number; i++)
  {
    buff[i] = static_cast<SI16>((this->wpgetch() &0xFF));
    buff[i] |= static_cast<SI16>((this->wpgetch() &0xFF) << 8);
  }
}

void MULFile::getULong(UI32 *buff, unsigned int number)
{
  for(unsigned int i = 0; i < number; i++)
  {
    buff[i] = static_cast<UI32>((this->wpgetch() &0xFF));
    buff[i] |= static_cast<UI32>((this->wpgetch() &0xFF) << 8);
    buff[i] |= static_cast<UI32>((this->wpgetch() &0xFF) << 16);
    buff[i] |= static_cast<UI32>((this->wpgetch() &0xFF) << 24);
  }
}

void MULFile::getLong(SI32 *buff, unsigned int number)
{
  for(unsigned int i = 0; i < number; i++)
  {
   buff[i] = static_cast<SI32>((this->wpgetch() &0xFF));
    buff[i] |= static_cast<SI32>((this->wpgetch() &0xFF) << 8);
    buff[i] |= static_cast<SI32>((this->wpgetch() &0xFF) << 16);
    buff[i] |= static_cast<SI32>((this->wpgetch() &0xFF) << 24);
  }
}
/*
** this implementation of tell is broken since it doesn't take into account that
** the buffer may already hold xxx number of characters. so i've taken it out for now
** (nothing was really needing to use it) - fur

long MULFile::tell()
{
  return ftell(theFile);
}

int MULFile::getChkSum()
{
  long pos, chksum;

  pos = ftell(theFile);
  rewind();
  ftell(theFile);
  chksum = 0L;
  do
  {
if (qRefill())
refill();
    while(bIndex != bSize) chksum += ioBuff[bIndex++];
  }
  while(bSize != 0);

  fseek(theFile, pos, SEEK_SET);

  return chksum;
}

int MULFile::getLength()
{
  long currentPos = ftell(theFile);
  
  fseek(theFile, 0L, SEEK_END);

  long pos = ftell(theFile);

  fseek(theFile, currentPos, SEEK_SET);

  return pos-1;
}
*/

void MULFile::get_versionrecord(struct versionrecord *buff, unsigned int number)
{
  for(unsigned int i = 0; i < number; i++)
  {
    getLong(&buff[i].file);
    getLong(&buff[i].block);
    getLong(&buff[i].filepos);
    getLong(&buff[i].length);
    getLong(&buff[i].unknown);
  }
}
/*
** More info from Alazane & Circonian on this...
**
Index entry:
DWORD: File ID (see index below)
DWORD: Block (Item number, Gump number or whatever; like in the file)
DWORD: Position (Where to find this block in verdata.mul)
DWORD: Size (Size in Byte)
DWORD: Unknown (Perhaps some CRC for the block, most blocks in UO files got this) 

    File IDs: (* means used in current verdata)
00 - map0.mul
01 - staidx0.mul
02 - statics0.mul
03 - artidx.mul
04 - art.mul*
05 - anim.idx
06 - anim.mul
07 - soundidx.mul
08 - sound.mul
09 - texidx.mul
0A - texmaps.mul
0B - gumpidx.mul
0C - gumpart.mul*
0D - multi.i
*/

void MULFile::get_st_multi(struct st_multi *buff, unsigned int number)
{
  for(unsigned int i = 0; i < number; i++)
  {
    getShort(&buff[i].tile);
    getUShort(&buff[i].x);
    getUShort(&buff[i].y);
    getChar(&buff[i].z);
    getChar(&buff[i].empty);
	getLong(&buff[i].visible);
  }
}

void MULFile::get_land_st(struct land_st *buff, unsigned int number)
{
  for(unsigned int i = 0; i < number; i++)
  {
    getUChar(&(buff[i].flag1));
    getUChar(&(buff[i].flag2));
    getUChar(&(buff[i].flag3));
    getUChar(&(buff[i].flag4));
    getUChar(&(buff[i].unknown1));
    getUChar(&(buff[i].unknown2));
    getText(buff[i].name, 20);
  }
}

void MULFile::get_tile_st(struct tile_st *buff, unsigned int number)
{
  for(unsigned int i = 0; i < number; i++)
  {
    getUChar(&(buff[i].flag1));
    getUChar(&(buff[i].flag2));
    getUChar(&(buff[i].flag3));
    getUChar(&(buff[i].flag4));
    getUChar(&(buff[i].weight));
    getChar(&(buff[i].layer));
    getLong(&(buff[i].unknown1));
    getLong(&(buff[i].animation));
    getChar(&(buff[i].unknown2));
    getChar(&(buff[i].unknown3));
    getChar(&(buff[i].height));
    getText(buff[i].name, 20);
  }
}

void MULFile::get_map_st(struct map_st *buff, unsigned int number)
{
  for(unsigned int i = 0; i < number; i++)
  {
    getShort(&buff[i].id);
    getChar(&buff[i].z);
  }
}

void MULFile::get_st_multiidx(struct st_multiidx *buff, unsigned int number)
{
  for(unsigned int i = 0; i < number; i++)
  {
    getLong(&buff[i].start);
    getLong(&buff[i].length);
    getLong(&buff[i].unknown);
  }
}

void MULFile::get_staticrecord(struct staticrecord *buff, unsigned int number)
{
  for(unsigned int i = 0; i < number; i++)
  {
    getShort(&buff[i].itemid);
    getUChar(&buff[i].xoff);
    getUChar(&buff[i].yoff);
    getChar(&buff[i].zoff);
    SI16 extra;
    getShort(&extra);
  }
}

