  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "nxwcommn.h"
#include "extractionskills.h"
#include "data.h"
#include "items.h"
#include "chars.h"
#include "skills.h"
#include "inlines.h"


cResources ores;


cResources::cResources( UI32 areawidth, UI32 areaheight )
{
	resources.clear();
	setDimArea( areawidth, areaheight );
	n=10;
	time=600;
	rate=3;
	stamina=-5;
}

cResources::~cResources()
{
}

void cResources::setDimArea( UI32 areawidth, UI32 areaheight )
{
	area_width=areawidth;
	area_height=areaheight;
}

UI64 cResources::getBlocks( Location location  )
{
	return (UI64)( (UI64)(location.x / this->area_width) <<32 ) + ( location.y / this->area_height );
}


P_RESOURCE cResources::getResource( Location location )
{

	UI64 p = this->getBlocks( location );
	
	RESOURCE_MAP::iterator iter( this->resources.find( p ) );
	if( iter==this->resources.end() )
		return NULL;
	else 
		return &(iter->second); 
}

P_RESOURCE cResources::createBlock( Location location )
{
	UI64 p = this->getBlocks( location );
	
	this->resources.insert( make_pair( p, cResource( ) ) );
	RESOURCE_MAP::iterator iter( this->resources.find( p ) );
	return &iter->second;
}

void cResources::checkResource( Location location, P_RESOURCE& res )
{

	if( res==NULL )
		return;

	if( this->checkRes( res ) ) { //delete
		res=NULL;
		this->deleteBlock( location );
	}

}

void cResources::decreaseResource( Location location, P_RESOURCE res )
{
	if( res == NULL ) { //create it!
		res = this->createBlock( location );
	}
	res->consumed++;
}


bool cResources::thereAreSomething( P_RESOURCE res )
{
	return ( res==NULL ) || ( res->consumed<=this->n );
}


bool cResources::checkRes( P_RESOURCE res )
{

	if( TIMEOUT( res->timer ) )
	{
		if( res->consumed>ores.n )
			res->consumed=ores.n;
		for( UI32 c=0; c<ores.n; c++ ) //Find howmany periods have been by, give 1 more ore for each period.
		{
			if(( TIMEOUT( res->timer+(c* ores.time*MY_CLOCKS_PER_SEC)) ) && res->consumed>=ores.rate )
				res->consumed-=ores.rate;//AntiChrist
		}
		res->timer= uiCurrentTime + ores.time*MY_CLOCKS_PER_SEC;
		if( res->consumed == 0)
			return true; // delete itself because FULL
	}

	return false;
}


void cResources::checkAll()
{

	static TIMERVAL timer=uiCurrentTime+2*60*MY_CLOCKS_PER_SEC;

	if(TIMEOUT( timer ) ) {

		RESOURCE_MAP::iterator next( this->resources.begin() );
		for( ; next!=resources.end(); ) {
			RESOURCE_MAP::iterator iter( next ); next++;
			if( this->checkRes( &iter->second ) ) {
				this->resources.erase( iter );
			}
		}
		timer=uiCurrentTime+2*60*MY_CLOCKS_PER_SEC;
	}
}

void cResources::deleteBlock( Location location ) 
{
	UI64 p = this->getBlocks( location );
	
	RESOURCE_MAP::iterator iter( this->resources.find( p ) );
	if( iter!=this->resources.end() )
		this->resources.erase( iter );
}











static LOGICAL canMine( P_CHAR pc, P_ITEM weapon )
{

	VALIDATEPCR(pc,false);
	
	if( !ISVALIDPI(weapon) )
		pc->sysmsg("You can't mine with nothing in your hand !!");
	else
		switch( weapon->getId() )
		{
			case 0x0E85	:
			case 0x0E86	:
			case 0x0F39	:
			case 0x0F3A	:
				if (pc->isMounting())
					pc->sysmsg( TRANSLATE("You cant mine while on a horse!"));
				else
					if( !pc->IsGM() && (ores.stamina<0) && (abs( ores.stamina )>pc->stm) )
						pc->sysmsg( TRANSLATE("You are too tired to mine."));
					else
						return true;
				break;
			default :
				pc->sysmsg( TRANSLATE("You must have a pickaxe or shovel in hand in order to mine."));
		}

	return false;
}


void Skills::target_mine( NXWCLIENT ps, P_TARGET t )
{

	P_CHAR pc = ps->currChar();
	VALIDATEPC( pc );

	NXWSOCKET s = ps->toInt();
	
	P_ITEM weapon = pc->GetItemOnLayer(1);
	
	if( !canMine( pc, weapon ) )
		return;

	Location target = t->getLocation();

	pc->facexy( target.x, target.y );

	AMXEXECSVTARGET( pc->getSerial32(),AMXT_SKITARGS,MINING,AMX_BEFORE);

	if ( pc->hidden )
		pc->unHide();

	LOGICAL floor = false;
	LOGICAL mountain= false;

	map_st map;
	land_st land;

	pc->stm+=ores.stamina;
	if(pc->stm<0) 
		pc->stm=0;
	if(pc->stm>pc->dx) 
		pc->stm=pc->dx;
	pc->updateStats(2);

	

	int cx = abs( (int) (pc->getPosition().x - target.x) );
	int cy = abs( (int) (pc->getPosition().y - target.y) );

	if( (cx>5) || (cy>5) )
	{
		pc->sysmsg(TRANSLATE("You are to far away to reach that"));
		return;
	}

	UI32 id = t->getModel();

	if( SrvParms->minecheck > 0 && !id )
	{
		// mountains are "map0's" and no statics !!!

		data::seekMap( target.x, target.y, map );
		data::seekLand( map.id, land );

		if ( !strcmp(land.name,"rock") || !(strcmp(land.name, "mountain")) || !(strcmp(land.name, "cave")))
			mountain = true;
	}

	data::seekMap( target.x, target.y, map );
	if( !id )
		id= map.id;

	switch( id )
	{
		case 0x0ED3:
		case 0x0EDF:
		case 0x0EE0:
		case 0x0EE1:
		case 0x0EE2:
		case 0x0EE8:
			Skills::GraveDig( s );
			return;
		default:
			break;
	}

	//
	// Caves (Walls & Floors)
	//
	if( (id >= 0x025C && id <= 0x0276) ||
		(id >= 0x027D && id <= 0x0280) ||
		(id >= 0x053B && id <= 0x0553) ||
		(id == 0x056A))
		floor = true;

	// sand (Anthalir)
	if( (id>=0x0017) && (id<=0x0019) )
		floor = true;


	// check if cave floor ENDYMION USE THIS BUT SEE VALUES IN OLD CODE
	/*if ( 
		( (targetData.getModel( s )>>8)==0x05 && ( ((targetData.getModel( s )%256)>=0x3b && (targetData.getModel( s )%256)<=0x4f ) || ((targetData.getModel( s )%256)>=0x51 && (targetData.getModel( s )%256)<=0x53) || (targetData.getModel( s )%256)==0x6a ))&&
		(!( ((targetData.getModel( s )>>8)==0x02)&&
        ( ( ((targetData.getModel( s )%256)>=0x5c) && ((targetData.getModel( s )%256)<=0x76))||(((targetData.getModel( s )%256)>=0x7d)&&((targetData.getModel( s )%256)<=0x80))))))
		
		 floor=1;*/
	

	if ((SrvParms->minecheck!=0)&&(!floor)&&(!mountain))//Mine only mountains & floors
	{
		pc->sysmsg(TRANSLATE("You can't mine that!"));
		return;
	}

	P_RESOURCE res = ores.getResource( target );

	ores.checkResource( target, res );

	if( !ores.thereAreSomething( res ) )
	{
		pc->sysmsg(TRANSLATE("There is no metal here to mine."));
		return;
	}

	if (pc->isMounting())
		pc->playAction(0x1A);
	else
		pc->playAction(0x0B);

	pc->playSFX(0x0125);

	if(!pc->checkSkill(MINING, 0, 1000))
	{
		pc->sysmsg(TRANSLATE("You sifted thru the dirt and rocks, but found nothing useable."));
		if( rand()%2==1)
			return; //Randomly deplete resources even when they fail 1/2 chance you'll loose ore.
	}
	ores.decreaseResource( target, res );

	AmxFunction::g_prgOverride->CallFn( AmxFunction::g_prgOverride->getFnOrdinal(AMXMINING), s);
	AMXEXECSVTARGET(pc->getSerial32(),AMXT_SKITARGS,MINING,AMX_AFTER);
}

