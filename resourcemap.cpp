#include "resourcemap.h"
#include "basics.h"

std::map<UI32, cResourceMap *> resourceMaps;
UI32 resourcemapSerial =0;

cResourceMap::cResourceMap(LOGICAL keepInMemory)
{
	this->keepInMemory=keepInMemory;
	if ( !keepInMemory )
	{
		this->filename=resourcemapSerial + ".resourcemap.nxw.bin";
	}
}

cResourceMap::cResourceMap(std::string filename, LOGICAL keepInMemory)
{
	this->filename=filename;
	this->keepInMemory=keepInMemory;
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
	if ( isInMemory() )
	{
		resourceMap.insert(make_pair(key, value));
	}
	if ( !isInMemory() && getFile() != "" )
	{
		fstream datafile(getFile().c_str(), ios::in|ios::out );
		if (!datafile.is_open()) 
		{ 
			// file didn't exist until now
			datafile << key << ends << value << endl;
		}
		else
		{
// now the complicated part begins
// search for the key linear
			std::string tempKey;
			do
			{
				datafile >> tempKey;
				// ignore data and end line sign
				datafile.ignore(sizeof(value)+1);
				if ( tempKey == key )
				{
					// the value has been previously used and needs to be updated now
					datafile.seekp(-(SI32)(tempKey.size()), ios::cur);
					datafile << tempKey << ends;
					datafile << value << endl;
					break;
				}
			}
			while ( !datafile.eof() && tempKey > key );
			if ( tempKey < key )
			{
				// another condition, to keep the file sorted, the new key has to be inserted ahead of the current key
				// i suppose it is faster to copy the file up to the current position to a backup file
				// then insert the new key and append the rest of the file
				
				std::string backupFilename=getFile()+".new";
				ofstream backupFile(backupFilename.c_str(), ios::out);
				datafile.seekp(-(SI32)(tempKey.size()), ios::cur);
				UI32 currOffset=datafile.tellg();
				datafile.seekg(0, ios::beg);
				// Since there does not seem to be a way to push data into the stream without destroying the content 
				// all data from the current offset is moved to the end
				char buffer[4096];

				while ( datafile.tellg() < currOffset-4096 )
				{
					UI32 readBytes;
					datafile.read(buffer, 4096);
					readBytes=datafile.gcount();
					backupFile.write(buffer, readBytes);
				}
				// if the current offset is still lower than the old offset, then write the remaining bytes
				if ( datafile.tellg() < currOffset)
				{
					UI32 offset;
					datafile.read(buffer, currOffset-datafile.tellg());
					offset=datafile.gcount();
					backupFile.write(buffer, offset);
				}
				// now we can finally write the new key
				datafile << tempKey << ends;
				datafile << value << endl;
				// and now the rest of the datafile
				while ( !datafile.eof() )
				{
					UI32 readBytes;
					datafile.read(buffer, 4096);
					readBytes=datafile.gcount();
					backupFile.write(buffer, readBytes);
				}
				backupFile.flush();
				backupFile.close();
				datafile.close();
				remove( getFile().c_str() );
				rename( backupFilename.c_str(), getFile().c_str() );


			}
		}
	}
}

SI32 cResourceStringMap::getValue(std::string key)
{
	if ( getFile() != "" )
	{
		SI32 value;
		// since we have a sorted file get Operations should be much faster using a binary sort
		// but we have to find the correct offset first
		fstream datafile(getFile().c_str(), ios::in|ios::out );
		if (!datafile.is_open()) 
		{ 
			// file didn't exist until now
			datafile.close();
			return -1;
		}
		std::string tempKey;
		do
		{
			datafile >> tempKey;
			datafile >> value;
			// ignore data and end line sign
			datafile.ignore(1);
			if ( tempKey == key )
			{
				datafile.close();
				return value;
			}
		}
		while ( !datafile.eof() && tempKey > key );
		datafile.close();
		return -1;
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
	if ( isInMemory() )
	{
		std::map<cCoord, SI32>::iterator iter = resourceMap.find(key);
		if ( iter != resourceMap.end())
			resourceMap.erase(iter);
		resourceMap.insert(make_pair(key, value));
	}
	if ( !isInMemory() && getFile() != "" )
	{
		fstream datafile(getFile().c_str(), ios::in|ios::out );
		if (!datafile.is_open()) 
		{ 
			// file didn't exist until now
			datafile << key.x; 
			datafile << key.y;
			datafile << key.z;
			datafile << value << endl;
		}
		else
		{
// now the complicated part begins
// search for the key binary, so start at half the file records
			const UI32 recordSize=sizeof(key.x)+sizeof(key.y)+sizeof(key.z)+sizeof(value)+1;
			UI16 x,y;
			SI08 z;
			SI08 direction=1;
			UI32 numberOfRecordsToSeek;
			// put pointer to end of file
			datafile.seekg(0, ios::end);
			// seek first in the mid of the file
			numberOfRecordsToSeek=(datafile.tellg() / recordSize / 2);
			// rewind to the beginning
			datafile.seekg(0, ios::beg);
			
			// now read the key at the position, if lower than new key, than go forward in file, else go backward
			while (!datafile.eof())
			{
				datafile.seekg(numberOfRecordsToSeek*recordSize*direction, ios::cur);	
				datafile >> x;
				datafile >> y;
				datafile >> z;
				cCoord tempKey(x,y,z);
				datafile.ignore(sizeof(value)+1);
				if ( tempKey == key ) 
				{
					// the value has been previously used and needs to be updated now
					datafile.seekp(-(SI32)recordSize, ios::cur);
					datafile << tempKey.x; 
					datafile << tempKey.y;
					datafile << tempKey.z;
					datafile << value << endl;
					return;
				}
				// we have to go forward in file
				else if ( tempKey < key )
				{
					numberOfRecordsToSeek=numberOfRecordsToSeek/2;
					// let's see there is only two possibilities
					// there is only one record in file
					// or we have searched into the direction and the new key has to be copied after the current key
					// in both situations the new key has to be written at the current position
					if ( numberOfRecordsToSeek == 0 )
					{
						break;
					}
					direction=1;
				}
				else
				{
					// now the temp key is bigger than the new key, so it has to be inserted before the current position
					numberOfRecordsToSeek=numberOfRecordsToSeek/2;
					// let's see there is only two possibilities
					// there is only one record in file
					// or we have searched into the direction and the new key has to be copied after the current key
					// in both situations the new key has to be written at the current position
					if ( numberOfRecordsToSeek == 0 )
					{
						break;
					}
					direction=-1;
				}
			}
			// now we are at the correct position
			std::string backupFilename=getFile()+".new";
			ofstream backupFile(backupFilename.c_str(), ios::out);
			UI32 currOffset=datafile.tellg();
			datafile.seekg(0, ios::beg);
			// Since there does not seem to be a way to push data into the stream without destroying the content 
			// all data from the current offset is moved to the end
			char buffer[4096];

			while ( datafile.tellg() < currOffset-4096 )
			{
				UI32 readBytes;
				datafile.read(buffer, 4096);
				readBytes=datafile.gcount();
				backupFile.write(buffer, readBytes);
			}
			// if the current offset is still lower than the old offset, then write the remaining bytes
			if ( datafile.tellg() < currOffset)
			{
				UI32 offset;
				datafile.read(buffer, currOffset-datafile.tellg());
				offset=datafile.gcount();
				backupFile.write(buffer, offset);
			}
			// now we can finally write the new key
			datafile << key.x; 
			datafile << key.y;
			datafile << key.z;
			datafile << value << endl;
			// and now the rest of the datafile
			while ( !datafile.eof() )
			{
				UI32 readBytes;
				datafile.read(buffer, 4096);
				readBytes=datafile.gcount();
				backupFile.write(buffer, readBytes);
			}
			backupFile.flush();
			backupFile.close();
			datafile.close();
			remove( getFile().c_str() );
			rename( backupFilename.c_str(), getFile().c_str() );
		}
	}

}

SI32 cResourceLocationMap::getValue(cCoord key)
{
	if ( getFile() != "" )
	{
		fstream datafile(getFile().c_str(), ios::in|ios::out );
		if (!datafile.is_open()) 
		{ 
			return -1;
		}
		SI32 value;
		const UI32 recordSize=sizeof(key.x)+sizeof(key.y)+sizeof(key.z)+sizeof(value)+1;
		UI16 x,y;
		SI08 z;
		SI08 direction=1;
		UI32 numberOfRecordsToSeek;
		// put pointer to end of file
		datafile.seekg(0, ios::end);
		// seek first in the mid of the file
		numberOfRecordsToSeek=(datafile.tellg() / recordSize / 2);
		// rewind to the beginning
		datafile.seekg(0, ios::beg);
		
		// now read the key at the position, if lower than new key, than go forward in file, else go backward
		while (!datafile.eof())
		{
			datafile.seekg(numberOfRecordsToSeek*recordSize*direction, ios::cur);	
			datafile >> x;
			datafile >> y;
			datafile >> z;
			datafile >> value;
			cCoord tempKey(x,y,z);
			datafile.ignore(sizeof(value)+1);
			if ( tempKey == key ) 
			{
				datafile.close();
				return value;
			}
			// we have to go forward in file
			else if ( tempKey < key )
			{
				numberOfRecordsToSeek=numberOfRecordsToSeek/2;
				// let's see there is only two possibilities
				// there is only one record in file
				// or we have searched into the direction and the new key has to be copied after the current key
				// in both situations the new key has to be written at the current position
				if ( numberOfRecordsToSeek == 0 )
				{
					break;
				}
				direction=1;
			}
			else
			{
				// now the temp key is bigger than the new key, so it has to be inserted before the current position
				numberOfRecordsToSeek=numberOfRecordsToSeek/2;
				// let's see there is only two possibilities
				// there is only one record in file
				// or we have searched into the direction and the new key has to be copied after the current key
				// in both situations the new key has to be written at the current position
				if ( numberOfRecordsToSeek == 0 )
				{
					break;
				}
				direction=-1;
			}
		}
		datafile.close();
		return -1;
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

