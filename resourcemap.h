#ifndef __resourcemaps_h
#define __resourcemaps_h

#include "common_libs.h"
#include "typedefs.h"
#include "coord.h"
#include "constants.h"

extern UI32 resourcemapSerial ;

class cResourceMap
{
	std::string filename;
	LOGICAL keepInMemory;
	ResourceMapType type;
	
public:
	cResourceMap(LOGICAL keepInMemory=true);
	cResourceMap(std::string filename, LOGICAL keepInMemory=true);
	~cResourceMap();
	virtual void deserialize(ifstream *myStream);
	virtual void serialize(ofstream *myStream);
	
	inline void setFile(std::string filename)
	{ this->filename = filename; };
	inline std::string getFile()
	{ return this->filename ; };
	inline void setType(ResourceMapType type)
	{ this->type = type; };
	inline ResourceMapType getType()
	{ return this->type ; };
	inline LOGICAL isInMemory()
	{ return keepInMemory; };
	inline void setInMemory(LOGICAL keepInMemory)
	{ this->keepInMemory=keepInMemory; };
	virtual void setValue(SI32 value);
	virtual SI32 getValue();
	static cResourceMap * getMap(UI32 index);
	static UI32 addMap(cResourceMap *map);
	static void deleteMap(UI32 index);
	static void save();
	static void load();

};

extern std::map<UI32, cResourceMap *> resourceMaps;

class cResourceStringMap : public cResourceMap
{
	private:

	std::map<std::string, SI32> resourceMap;

	public:
	cResourceStringMap(std::string filename, LOGICAL keepInMemory=true);
	void deserialize(ifstream *myStream);
	void serialize(ofstream *myStream);
	void setValue(std::string key, SI32 value);
	SI32 getValue(std::string key);
	
};


class cResourceLocationMap : public cResourceMap
{
	private:

	std::map<cCoord, SI32> resourceMap;

	public:
	cResourceLocationMap(std::string filename, LOGICAL keepInMemory=true);
	void deserialize(ifstream *myStream);
	void serialize(ofstream *myStream);
	void setValue(cCoord key, SI32 value);
	SI32 getValue(cCoord key);

};

typedef cResourceMap *P_RESOURCEMAP;

#endif