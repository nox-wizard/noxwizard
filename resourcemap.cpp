#include "resourcemap.h"
#include "basics.h"

std::map<UI32, cResourceMap *> resourceMaps;
UI32 resourcemapSerial =0;

cResourceMap::cResourceMap(LOGICAL persistent)
{
	this->persistent=persistent;
}

cResourceMap::cResourceMap(std::string filename)
{
	this->filename=filename;
	this->persistent=true;
}

cResourceMap::~cResourceMap()
{
	;
}
	

void cResourceMap::setValue(SI32 value)
{
	return ;
}

SI32 cResourceMap::getValue()
{
	return 0;
}

cResourceMap *cResourceMap::getMap(UI32 index)
{
	std::map<UI32, cResourceMap *>::iterator iter = resourceMaps.find(index);
	if ( iter != resourceMaps.end())
		return iter->second;
	else
		return NULL;
}

UI32 cResourceMap::addMap(cResourceMap *newMap)
{
	std::map<UI32, cResourceMap *>::iterator iter = resourceMaps.begin();
	for ( ; iter !=  resourceMaps.end();iter++)
	{
		if (iter->second ==newMap )
			return 0;
	}
	resourceMaps.insert(make_pair(++resourcemapSerial, newMap));
	return resourcemapSerial;
}

void cResourceMap::deleteMap(UI32 index)
{
	std::map<UI32, cResourceMap *>::iterator iter = resourceMaps.find(index);
	if ( iter != resourceMaps.end())
		resourceMaps.erase(iter);
}

void cResourceMap::serialize(ostream *myStream)
{
}

void cResourceMap::deserialize(istream *myStream)
{
}

// now for the real maps

void cResourceStringMap::deserialize(istream *myStream)
{
	std::string tempKey;
	UI32 value;
	while (! myStream->eof() )
	{
		*myStream >> tempKey;
		*myStream >> value;
		myStream->ignore(1);
		this->setValue(tempKey, value);
	}
	return ;
}

void cResourceStringMap::serialize(ostream *myStream)
{
	std::map<std::string, SI32>::iterator iter = resourceMap.begin();
	for ( ; iter !=  resourceMap.end();iter++)
	{
		*myStream << iter->first << ends;
		*myStream << iter->second << endl;
	}
	return ;
}

void cResourceStringMap::setValue(std::string key, SI32 value)
{
	if ( getFile() != "" )
	{
	}
	else
		resourceMap.insert(make_pair(key, value));
}

SI32 cResourceStringMap::getValue(std::string key)
{
	if ( getFile() != "" )
	{
		return NULL;
	}
	else
	{
		std::map<std::string, SI32>::iterator iter = resourceMap.find(key);
		if ( iter != resourceMap.end())
			return iter->second;
		else
			return -1;
	}
}

void cResourceLocationMap::deserialize(istream *myStream)
{
	UI16 x,y;
	SI08 z;
	UI32 value;
	while (! myStream->eof() )
	{
		*myStream >> x;
		*myStream >> y;
		*myStream >> z;
		*myStream >> value;
		myStream->ignore(1);
		cCoord tempKey(x,y,z);
		this->setValue(tempKey, value);
	}
	return ;
}

void cResourceLocationMap::serialize(ostream *myStream)
{
	std::map<cCoord, SI32>::iterator iter = resourceMap.begin();
	for ( ; iter !=  resourceMap.end();iter++)
	{
		*myStream << iter->first.x;
		*myStream << iter->first.y;
		*myStream << iter->first.z;
		*myStream << iter->second << endl;
	}
	return ;
}

void cResourceLocationMap::setValue(cCoord key, SI32 value)
{
	if ( getFile() != "" )
	{
	}
	else
	{
		std::map<cCoord, SI32>::iterator iter = resourceMap.find(key);
		if ( iter != resourceMap.end())
			resourceMap.erase(iter);
		resourceMap.insert(make_pair(key, value));
	}
}

SI32 cResourceLocationMap::getValue(cCoord key)
{
	if ( getFile() != "" )
	{
		return NULL;
	}
	else
	{
		std::map<cCoord, SI32>::iterator iter = resourceMap.find(key);
		if ( iter != resourceMap.end())
			return iter->second;
		else
			return -1;
	}
}

