#ifndef __AMXFILE_H__
#define __AMXFILE_H__

#include "nxwcommn.h"

typedef map< UI32, FILE* >	mAmxFile;
typedef mAmxFile::iterator	miAmxFile;


class amxFiles
{
	private:
		mAmxFile	files;
		UI32		lastHandleIssued;
	public:
				amxFiles();
				~amxFiles();
	public:
		SI32		open( const std::string &name, const std::string &mode );
		LOGICAL		close( const SI32 handle );
		LOGICAL		eof( const SI32 handle );
		LOGICAL		write( const SI32 handle, const std::string &line );
		std::string	read( const SI32 handle );
};

extern amxFiles amxFileServer;

#endif
