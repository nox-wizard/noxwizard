#include "amx_api.h"


int AMXAPI amx_SetStringUnicode(cell *dest, std::vector<unsigned char>* source )
{
	std::vector<unsigned char>::iterator iter( source->begin() ), end( source->end() );
	int i=0; unsigned short v=0;
	for( ; iter!=end; iter++, ++i ) {
		if( i%2 )
			dest[i/2]=v +(*iter);
		else
			v=(*iter)<< 8;
	}
    /* On Big Endian machines, the characters are well aligned in the
     * cells; on Little Endian machines, we must swap all cells.
    */
    if (amx_LittleEndian) {
		int len = ( source->size()/2 ) / sizeof(cell);
		while (len>=0)
			swapcell((ucell *)&dest[len--]);
    }

	return AMX_ERR_NONE;
}

int AMXAPI amx_GetStringUnicode( std::vector<unsigned char>* dest, cell* source )
{

	cell temp;
	int i=0;
	do {
		temp=source[i];
	    if (amx_LittleEndian) swapcell( (ucell *)&temp );
		dest->push_back( temp >>8 );
		dest->push_back( temp & 0xFF );
	} while ( temp!=0 );

	return AMX_ERR_NONE;
}

