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
*/

#ifndef __CACHE_H
#define __CACHE_H

class MULFile;

#include "nxwcommn.h"

// I am not above giving credit where it is due. This is the same cache that UOX currently uses.
// Thanks Zippy for doing the original designs on it, then to fur for fixing and making it run
// smoother. And to the UOX dev for doing some good work. 

class MULFile
{
  private:

  int fmode, ok, bSize, bIndex;
  char *ioBuff;
  FILE *theFile;

  public:

  MULFile(char *, char *);
  ~MULFile();

  inline int qRefill() { return (bIndex >= bSize); };
  inline int ready() { return (ok); };
  void rewind();
  void seek(long, int);
  inline int eof() { return (feof(theFile)); };
  SI08 wpgetch();
  void refill();
  char *gets(char *, int);
  int puts(char *);

  void getUChar(UI08 *, unsigned int = 1);
  void getChar(SI08 *, unsigned int = 1);
  void getText(TEXT *, unsigned int = 1);
  void getUShort(UI16 *, unsigned int = 1);
  void getShort(SI16 *, unsigned int = 1);
  void getULong(UI32 *, unsigned int = 1);
  void getLong(SI32 *, unsigned int = 1);  

  void get_versionrecord(struct versionrecord *, unsigned int = 1);
  void get_st_multi(struct st_multi *, unsigned int = 1);
  void get_land_st(struct land_st *, unsigned int = 1);
  void get_tile_st(struct tile_st *, unsigned int = 1);
  void get_map_st(struct map_st *, unsigned int = 1);
  void get_st_multiidx(struct st_multiidx *, unsigned int = 1);
  void get_staticrecord(struct staticrecord *, unsigned int = 1);
};

#endif
