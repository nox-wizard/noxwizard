#include "amxfile.h"

amxFiles amxFileServer;

amxFiles::amxFiles()
{
	lastHandleIssued = 0;
}

amxFiles::~amxFiles()
{
	if( !files.empty() )
	{
		miAmxFile	iter( files.begin() ), end( files.end() );
		do
		{
			fclose( iter->second );
			++iter;
		}
		while( iter != end );
	}
}

SI32 amxFiles::open( const std::string &name, const std::string &mode )
{
	FILE *pFile = fopen( name.c_str(), mode.c_str() );
	if( !pFile )
		return INVALID;
	files[ ++lastHandleIssued ] = pFile;
	return lastHandleIssued;
}

LOGICAL amxFiles::close( const SI32 handle )
{
	miAmxFile iter( files.find( handle ) );
	if( iter == files.end() )
		return false;
	fclose( iter->second );
	files.erase( iter );
	return true;
}

LOGICAL	amxFiles::eof( const SI32 handle )
{
	miAmxFile iter( files.find( handle ) );
	if( iter == files.end() )
		return false;
	return feof( iter->second );
}

LOGICAL	amxFiles::write( const SI32 handle, const std::string &line )
{
	miAmxFile iter( files.find( handle ) );
	if( iter == files.end() )
		return false;
	fprintf( iter->second, line.c_str() );
	return true;
}
/*
std::string amxFiles::read( const SI32 handle )
{
	miAmxFile iter( files.find( handle ) );
	if( iter != files.end() )
	{
		char line[1024];
		if( fgets( line, 1024, iter->second ) )
			return std::string( line );
	}
	return "";
}
*/
std::string amxFiles::read( const SI32 handle )
{
	miAmxFile iter( files.find( handle ) );
	if( iter != files.end() )
	{
		int i;
		LOGICAL finished = false;
		char c;
		char line[1024];
		line[0]=0;
		while ( !finished )
		{
			i = 0;
			if ( feof( iter->second ) )
				finished = true;
			else
			{
				c = (char) fgetc( iter->second );
				while ( c != 10 )
				{
					if ( c != 13 )
					{
						line[i]=c;
						++i;
					}
					if( feof( iter->second ) || i >= 1024 )
						break;
					else
						c = (char) fgetc( iter->second );
				}
				line[i] = 0;
				finished = true;
			}
		}
		return std::string( line );
	}
	return "";
}

