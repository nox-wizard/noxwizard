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
\brief Prototypes for misc functions
\todo split it into different files, IMHO (Akron)
*/

#ifndef __PROTOTYPES_H__
#define __PROTOTYPES_H__

void splitLine( char* source, char *head, char *tail );
int check_house_decay();

//bool ishouse(ITEM i);
//bool ishouse(int id1, int id2);

void cleanup(int s);
P_ITEM Packitem(P_CHAR pc);
void titletarget(int s);
//int ishuman(int p);
void npcact(int s);
void objTeleporters(P_CHAR pc);
void SkillVars();
void StartClasses();
void SetGlobalVars();
void BuildPointerArray();
void InitMultis();
void DeleteClasses();
int chardir(int a, int b);
NXWSOCKET  calcSocketFromChar(CHARACTER i);
int validNPCMove(int x, int y, signed char z, P_CHAR pc_s);
int calcItemFromSer(unsigned char s1, unsigned char s2, unsigned char s3, unsigned char s4);
int calcItemFromSer(int ser); // Added by Magius(CHE) (2)
int calcCharFromSer(int s1, int s2, int s3, int s4);
int calcCharFromSer(int ser);
int calcSerFromChar(int ser);
int checkBoundingBox(int xPos, int yPos, int fx1, int fy1, int fz1, int fx2, int fy2);
int checkBoundingCircle(int xPos, int yPos, int fx1, int fy1, int fz1, int radius);
void telltime(int s);
void impaction(int s, int act);
int fielddir(int s, int x, int y, int z);
bool npcattacktarget(int target2, int target);
void npcsimpleattacktarget(int target2, int target);
int RandomNum(int nLowNum, int nHighNum);
void enlist(int s, int listnum); // For enlisting in army

// Day and Night related prototypes
char indungeon(P_CHAR pc);
void setabovelight(unsigned char);

//void tweakmenu(int s, int j, int type);
int validtelepos(P_CHAR pc);

void addhere(int s, signed char z);
void whomenu(int s, int type);
void playermenu(int s, int type);
void gmmenu(int s, int m);
void scriptcommand (NXWSOCKET s, std::string script1, std::string script2);
void endmessage(int x);

void goldsfx(int s, int goldtotal);

void gcollect();

//signed char mapheight(int x, int y);
//void seekland(int landnum, land_st *land);
void weather(int s, unsigned char bolt);
//void xbanktarget(int s);
void openspecialbank(NXWSOCKET s, int i);//AntiChrist
void delequan(int s, short id, int amount, int & not_deleted = dummy__);
void gettokennum(std::string s, int num);
char* getRandomName(char * namelist);
void setrandomname(P_CHAR pc, char * namelist);
void donewithcall(int s, int type);
void initque();
void choice(int s);
//void mounthorse(int s, int x);
char *title1(P_CHAR pc);
char *title2(P_CHAR pc);
char *title3(P_CHAR pc);
char *complete_title(P_CHAR pc);
//int getname(int i, char* itemname);
void buyaction(int s);
//void restock(bool total);
void dooruse(NXWSOCKET s, P_ITEM pi/*int item*/);
void readFullLine ();

void who(int s);
void gms(int s);
void sellaction(int s);
void addgold(int s, int totgold);
int calcValue(int i, int value);
int calcGoodValue(int npcnum, int i, int value,int goodtype); // by Magius(CHE) for trade system
void StoreItemRandomValue(P_ITEM pi,int tmpreg); // by Magius(CHE) (2) for trade system

P_ITEM tradestart(P_CHAR pc1, P_CHAR pc2);
void clearalltrades();
void trademsg(int s);
void dotrade(P_ITEM cont1,P_ITEM cont2);
void dosocketmidi(int s);
void wipe(int s);

void loadmetagm();
void xteleport(int s,int x);
void wornitems(NXWSOCKET  s, CHARACTER j);
void itemmenu(int s, int m);
void showAddMenu (P_CHAR pc, int menu); //xan new itemmenuz :)
int findsection (unsigned char *s);


//void xsend(int s, void *point, int length, int test);
void loadserverscript(void);
//void saveserverscript(unsigned char);
void saveserverscript(void);
void loadserverdefaults(void);
int numbitsset( int number );
int whichbit( int number, int bit );
char *linestart(char *line);

void strupr(std::string &str);
void strlwr(std::string &str);

//int GetBankCount( P_CHAR, unsigned short itemid, unsigned short color = 0x0000 );
//int DeleBankItem( CHARACTER p, unsigned short itemid, unsigned short color, int amt );

void getSextantCords(signed int x, signed int y, bool t2a, char *sextant);

void usehairdye(int s, P_ITEM pidye);
//void buildhouse(int s, int i);
//void buildhouse(int s, int i,char d_id1,char d_id2,char d_id3, char d_id4);
//void deedhouse(NXWSOCKET  s, P_ITEM pi); //crackerjack 8/9/99
//void killkeys(SERIAL serial); // crackerjack 8/11/99
// house list functions - cj 8/12/99
//int on_hlist(int h, unsigned char s1, unsigned char s2, unsigned char s3, unsigned char s4, int *li);
//int add_hlist(int c, int h, int t);
//int del_hlist(int c, int h);
//
//void house_speech(int s, char *talk);
//void addthere(int s, int xx, int yy, int zz, int t);
//void mtarget(int s, int a1, int a2, int a3, int a4, char b1, char b2, char *txt);

COLOR addrandomcolor(cObject* po, char *colorlist);


void lockpick(int s);


int getRangedValue(char *stringguy);
// for newbie stuff
int bestskill(P_CHAR p);
void newbieitems(P_CHAR pc);


//For custom titles
void loadcustomtitle();
void loadskills();
void loadmenuprivs();

// Profiling
void StartMilliTimer(unsigned long &Seconds, unsigned long &Milliseconds);
unsigned long CheckMilliTimer(unsigned long &Seconds, unsigned long &Milliseconds);

void advancementobjects(CHARACTER s, int x, int always);
void itemsfx(NXWSOCKET  s, short item);
void bgsound(int s);
void splitline();
int strtonum(int countx, int base= 0);
int hexnumber(int countx);

// Dupois - fileArchive() prototypes
// Added Oct 20, 1998
void fileArchive(char *pFile2Archive_chars, char *pFile2Archive_items, char *pArchiveDir);
void ArchiveID(char archiveid[MAXARCHID]);
// End - Dupois

void enlist(int s); // For enlisting in army

inline int calcserial(unsigned char a1,unsigned char a2,unsigned char a3,unsigned char a4) {return (static_cast<int>((a1<<24))|static_cast<int>((a2<<16)) | static_cast<int>((a3<<8)) | static_cast<int>(a4));}

// Pointer.cpp functions
// - set item in pointer array
void setptr(lookuptr_st *ptr, int item);
void removefromptr(lookuptr_st *ptr, int nItem);// - remove item from pointer array
int findbyserial(lookuptr_st *ptr, int nSerial, int nType);// - find item in a pointer array
//void setserial(int nChild, int nParent, int nType);// - set serial#s and ptr arrays

//void possess(int s); //Not done
int line_of_sight(int s, Location a, Location b, int checkfor); // added by Anthalir
int line_of_sight(int s, int x1, int y1, int z1, int x2, int y2, int z2, int checkfor);
void batchcheck(int s);
void readw2();
void readw3();
void entrygump(int s, unsigned char tser1, unsigned char tser2, unsigned char tser3, unsigned char tser4, unsigned char type, char index, short int maxlength, char *text1);

// DasRaetsels' stuff up, don't touch :)


// Dupois message board prototype
// Proto for handling the different message type for message 0x71
void    MsgBoardEvent(int nSerial);
//void selectspell2cast(int s, int num); //Socket, Spell Number
void criminal(P_CHAR pc);
void callguards( int p );
int recursestatcap(int chr); //Morrolan - stat/skill cap
void skillfreq(int chr, int skill); //Morrolan - stat/skill cap
void charcreate(NXWSOCKET  s);	// All the character creation stuff

void usepotion(int p, P_ITEM pi);

//void tempeffect(int source, int dest, int num, unsigned char more1, unsigned char more2, unsigned char more3,short dur=-1, int amxcback = -3);
//void tempeffect2(int source, P_ITEM dest, int num, unsigned char more1, unsigned char more2, unsigned char more3);
//unsigned char tempeffect(int source, int dest, int num, unsigned char more1, unsigned char more2, unsigned char more3,short dur=-1, int amxcback = -3);
//unsigned char tempeffect2(int source, P_ITEM piDest, int num, unsigned char more1, unsigned char more2, unsigned char more3);
//void tempeffectsoff();
//void tempeffectson();
//void checktempeffects();

void checkauto();	// Check automatic/timer controlled stuff (Like fighting and regeneration)
void checktimers();	// Check shutdown timers

void setcharflag(P_CHAR pc);
LOGICAL setcharflag2(P_CHAR pc);

//P_ITEM ContainerSearch(const int serial, int *ci);
//P_ITEM ContainerSearchFor(const int serial, int *ci, short id, short color=-1);
//P_ITEM FindItemBySerial(int serial);
//P_ITEM FindItemBySerPtr(unsigned char *p);
//P_CHAR FindCharBySerPtr(unsigned char *p);
//P_ITEM GetOutmostCont(P_ITEM pItem, short rec=50);
//P_CHAR GetPackOwner(P_ITEM pItem, short rec=50);
//void doGmMoveEff(P_CHAR pc); //gm movement effects

void AttackStuff (NXWSOCKET  s, P_CHAR victim);
void HelpStuff (P_CHAR pc,P_CHAR pc_i);
void setCrimGrey (P_CHAR pc, int mode);
void pweather(NXWSOCKET  s);
void check_region_weatherchange (void);
//void initNewNPCSpells (void);
void newScriptsInit(void);
void deleteNewScripts(void);

bool checkItemUsability(P_CHAR pc, P_ITEM pi, int type);

void updatehtml();
void offlinehtml();



//Boats --Fucntions in Boats.cpp

P_ITEM findmulti(Location where);	// added by Anthalir
bool inmulti(Location where,P_ITEM pi);
//End Boat functions

void init_creatures(void);

void TellScroll( char *menu_name, int player, long item_param );

void doubleclick(NXWCLIENT ps);
void dbl_click_character(NXWCLIENT ps, P_CHAR target);
void singleclick(NXWSOCKET  s);
char *RealTime(char *time_str);

void walking(P_CHAR pc, int dir, int sequence);
void teleporters(P_CHAR pc);
void read_in_teleport(void);
void npcwalk( P_CHAR pc_i, int newDirection, int type);
void walking2(P_CHAR pc_s);
void savelog(const char *msg, char *logfile);
void npcMovement( P_CHAR pc_i );
//void Karma(int nCharID,int nKilledID, int nKarma);
//void Fame(int nCharID, int nFame);
void charstartup(int s);
void checkdumpdata(unsigned int currenttime); // This dumps data for Ridcully's UOXBot 0.02 (jluebbe@hannover.aball.de)
void killall(int s, int percent, char* sysmsg);

// functions in fishing.cpp
int SpawnFishingMonster(P_CHAR pc, char* cScript, char* cList, char* cNpcID);
int SpawnFishingItem(int nCharID,int nInPack, char* cScript, char* cList, char* cItemID);


// helper func for GOPLACE things :]
void location2xyz(int loc, int& x, int& y, int& z);

unsigned long int getclock();
unsigned long int getsysclock(void);
unsigned long int getclockday() ;
void initclock() ;



NXWCLIENT getClientFromSocket(int s);

void getWorldCoordsFromSerial (int sr, int& px, int& py, int& pz, int& ch, int& it);
NXWCLIENT getClientFromSocket(int s);


#endif //__PROTOTYPES_H__
