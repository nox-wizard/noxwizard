  /*-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    || NoX-Wizard UO Server Emulator (NXW) [http://noxwizard.sourceforge.net]  ||
    ||                                                                         ||
    || This software is free software released under GPL2 license.             ||
    || You can find detailed license information in nox-wizard.cpp file.       ||
    ||                                                                         ||
    || For any question post to NoX-Wizard forums.                             ||
    -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

#include "particles.h"
#include "magic.h"

namespace particles
{

	// sta_str layout:

	// 0..3 already used in 2d-staticeffect
	// effect 4  -> tile1
	// effect 5  -> tile2
	// effect 6  -> speed1
	// effect 7  -> speed1
	// effect 8  -> effect1
	// effect 9  -> effect2
	// effect 10 -> reserved, dont use
	// effect 11 ->
	// effect 12 ->


	/*!
	 \brief Write particleSystem packet for UO3D Static effects
	 \author Unknown - pseudo-totally rewritten by Akron
	 \param pc_cs source character
	 \param sta effect
	 \param particleSystem packet
	 */
	void staticeffectUO3D(P_CHAR pc_cs, ParticleFx *sta, UI08 *particleSystem)
	{
		VALIDATEPC(pc_cs);

		particleSystem[0]= 0xc7;
		particleSystem[1]= 0x3;

		LongToCharPtr(pc_cs->getSerial32(), particleSystem+2);

		particleSystem[6]= 0x0; // always 0 for this type
		particleSystem[7]= 0x0;
		particleSystem[8]= 0x0;
		particleSystem[9]= 0x0;

		particleSystem[10]= sta->effect[4]; // tileid1
		particleSystem[11]= sta->effect[5]; // tileid2

		ShortToCharPtr(pc_cs->getPosition().x, particleSystem+12);
		ShortToCharPtr(pc_cs->getPosition().y, particleSystem+14);
		particleSystem[16] = (UI08)pc_cs->getPosition().z;

		ShortToCharPtr(pc_cs->getPosition().x, particleSystem+17);
		ShortToCharPtr(pc_cs->getPosition().y, particleSystem+19);
		particleSystem[21] = (UI08)pc_cs->getPosition().z;

		particleSystem[22]= sta->effect[6]; // unkown1
		particleSystem[23]= sta->effect[7]; // unkown2

		particleSystem[24]=0x0; // only non zero for type 0
		particleSystem[25]=0x0;

		particleSystem[26]=0x1;
		particleSystem[27]=0x0;

		particleSystem[28]=0x0;
		particleSystem[29]=0x0;
		particleSystem[30]=0x0;
		particleSystem[31]=0x0;
		particleSystem[32]=0x0;
		particleSystem[33]=0x0;
		particleSystem[34]=0x0;
		particleSystem[35]=0x0;

		particleSystem[36]=sta->effect[8]; // effekt #
		particleSystem[37]=sta->effect[9];

		particleSystem[38]=sta->effect[11];
		particleSystem[39]=sta->effect[12];

		particleSystem[40]=0x00;
		particleSystem[41]=0x00;

		LongToCharPtr(pc_cs->getSerial32(), particleSystem+42);

		particleSystem[46]=0x0; // layer, gets set afterwards for multi layering
		particleSystem[47]=0x0; // has to be always 0 for all types
		particleSystem[48]=0x0;
	}

	// ParticleFx layout:
	// 0..4 already used in 2d-move_effect

	// effect 5  -> tile1
	// effect 6  -> tile2
	// effect 7  -> speed1
	// effect 8  -> speed2
	// effect 9  -> effect1
	// effect 10 -> effect2
	// effect 11 -> impact effect1
	// effect 12 -> impact effect2
	// effect 13 -> unkown1, does nothing, but gets set on OSI shards
	// effect 14 -> unkown2
	// effect 15 -> adjust
	// effect 16 -> explode on impact

	/*!
	 \brief Forge the particle system packet
	 \author Unknown - pseudo-totally rewritten by Akron
	 \param pc_cs source player
	 \param pc_cd destination player
	 \param eff effect
	 \param particleSystem the package to write
	 */
	void movingeffectUO3D(P_CHAR pc_cs, P_OBJECT po_cd, ParticleFx *eff, UI08 *particleSystem)
	{
		VALIDATEPC(pc_cs);
		VALIDATEPO(po_cd);

		particleSystem[0]=0xc7;
		particleSystem[1]=0x0;

		LongToCharPtr(pc_cs->getSerial32(), particleSystem+2);
		LongToCharPtr(po_cd->getSerial32(), particleSystem+6);

		particleSystem[10]=eff->effect[5]; // tileid1
		particleSystem[11]=eff->effect[6]; // tileid2

		ShortToCharPtr(pc_cs->getPosition().x, particleSystem+12);
		ShortToCharPtr(pc_cs->getPosition().y, particleSystem+14);
		particleSystem[16] = (UI08)pc_cs->getPosition().z;
		
		ShortToCharPtr(po_cd->getPosition().x, particleSystem+17);
		ShortToCharPtr(po_cd->getPosition().y, particleSystem+19);
		particleSystem[21] = (UI08)po_cd->getPosition().z;

		particleSystem[22]= eff->effect[7]; // speed1
		particleSystem[23]= eff->effect[8]; // speed2

		particleSystem[24]=0x0;
		particleSystem[25]=0x0;

		particleSystem[26]=eff->effect[15]; // adjust
		particleSystem[27]=eff->effect[16]; // explode

		particleSystem[28]=0x0;
		particleSystem[29]=0x0;
		particleSystem[30]=0x0;
		particleSystem[31]=0x0;
		particleSystem[32]=0x0;
		particleSystem[33]=0x0;
		particleSystem[34]=0x0;
		particleSystem[35]=0x0;

		particleSystem[36]=eff->effect[9]; //  moving effekt
		particleSystem[37]=eff->effect[10];
		particleSystem[38]=eff->effect[11]; // effect on explode
		particleSystem[39]=eff->effect[12];

		particleSystem[40]=eff->effect[13]; // ??
		particleSystem[41]=eff->effect[14];

		particleSystem[42]=0x00;
		particleSystem[43]=0x00;
		particleSystem[44]=0x00;
		particleSystem[45]=0x00;

		particleSystem[46]=0xff; // layer, has to be 0xff in that modus

		particleSystem[47]=eff->effect[17];
		particleSystem[48]=0x0;
	}

	//! same sta-layout as staticeffectuo3d
	void itemeffectUO3D(P_ITEM pi, ParticleFx *sta, UI08 *particleSystem)
	{
		// please no optimization of p[...]=0's yet :)
		VALIDATEPI(pi);

		particleSystem[0]=0xc7;
		particleSystem[1]=0x2;

		if ( !sta->effect[11] )
			LongToCharPtr(pi->getSerial32(), particleSystem+2);
		else
			LongToCharPtr(0, particleSystem+2);

		LongToCharPtr(0, particleSystem+6); // always 0 for this type

		particleSystem[10]=sta->effect[4]; // tileid1
		particleSystem[11]=sta->effect[5]; // tileid2

		ShortToCharPtr(pi->getPosition().x, particleSystem+12);
		ShortToCharPtr(pi->getPosition().y, particleSystem+14);
		particleSystem[16]= (UI08)pi->getPosition().z;

		ShortToCharPtr(pi->getPosition().x, particleSystem+17);
		ShortToCharPtr(pi->getPosition().y, particleSystem+19);
		particleSystem[21]= (UI08)pi->getPosition().z ;

		particleSystem[22]= sta->effect[6]; // unkown1
		particleSystem[23]= sta->effect[7]; // unkown2

		particleSystem[24]=0x0; // only non zero for type 0
		particleSystem[25]=0x0;

		particleSystem[26]=0x1;
		particleSystem[27]=0x0;

		LongToCharPtr(0, particleSystem+28); // always 0 for this type
		LongToCharPtr(0, particleSystem+32); // always 0 for this type

		particleSystem[36]=sta->effect[8]; // effekt #
		particleSystem[37]=sta->effect[9];

		particleSystem[38]=0; // unknown
		particleSystem[39]=1;

		particleSystem[40]=0x00;
		particleSystem[41]=0x00;

		LongToCharPtr(pi->getSerial32(), particleSystem+42);

		particleSystem[46]=0xff;

		particleSystem[47]=0x0;
		particleSystem[48]=0x0;

	}

	void bolteffectUO3D(P_CHAR pc_cs, UI08 *particleSystem)
	{
	#if 0
		Magic->doStaticEffect(player, 30);
	#endif
	}

	///////////////////////////////////////////////////////////////////
	// Function name     : ParticleFx::initWithSpellMoveEffect
	// Description       : init moving particle effect for casted spell
	// Return type       : void 
	// Author            : Lord Binary, Xanathar 
	// Argument          : magic::SpellId num -> casted spell
	// Changes           : none yet
	void ParticleFx::initWithSpellMoveEffect( magic::SpellId num )
	{
	    switch( num ) {
	    case magic::SPELL_MAGICARROW:
					effect[0]=0x36; effect[1]=0xE4; effect[2]=0x05; effect[3]=0x00; 
					effect[4]=0x01; effect[5]=0x36; effect[6]=0xe4; effect[7]=0x07; 
					effect[8]=0x00; effect[9]=0x0b; effect[10]=0xbe; effect[11]=0x0f; 
					effect[12]=0xa6; effect[13]=0x00; effect[14]=0x00; 
					effect[15]=0; effect[16]=0; effect[17]=0;
	                break;

	    case magic::SPELL_FIREBALL:    
					effect[0]=0x36; effect[1]=0xD5; effect[2]=0x07; effect[3]=0x00; effect[4]=0x01; 
	                effect[5]=0x36; effect[6]=0xd4; effect[7]=0x07; effect[8]=0x00; effect[9]=0x0b; 
					effect[10]=0xcb; effect[11]=0x0f; effect[12]=0xb3; 
					effect[13]=0x1; effect[14]=0x60;effect[15]=0; effect[16]=1; effect[17]=0;
	                break;

	    case magic::SPELL_EXPLOSION:
					effect[0]=0x37; effect[1]=0x9F; effect[2]=0x07; effect[3]=0x00; effect[4]=0x01; 
	                effect[5]=0x37; effect[6]=0x9f; effect[7]=0x07; effect[8]=0x00; effect[9]=0x0b; 
					effect[10]=0xe3; effect[11]=0x0f; effect[12]=0xcb; effect[13]=0x2; effect[14]=0x11; 
					effect[15]=0; effect[16]=1; effect[17]=0;
	                break;

	    case magic::SPELL_METEORSWARM:    
					effect[0]=0x36; effect[1]=0xD5; effect[2]=0x07; effect[3]=0x00; effect[4]=0x01;
					effect[10]=0x1d; effect[11]=0x00; effect[12]=0x01; effect[13]=0x0; effect[14]=0x0;
					effect[15]=0; effect[16]=1; effect[17]=1;
	                break;
	                                

	    default:    effect[0]=-1;   effect[1]=-1;   effect[2]=-1;   effect[3]=-1;   effect[4]=-1;   break;
	    }
	}




	///////////////////////////////////////////////////////////////////
	// Function name     : ParticleFx::initWithSpellStatEffect
	// Description       : init particle stat effect with effect for spell casted
	// Return type       : void
	// Author            : Lord Binary, Xanathar
	// Argument          : magic::SpellId num -> spell casted
	// Changes           : none yet
	void ParticleFx::initWithSpellStatEffect( magic::SpellId num )
	{
	    int te0,te1,te2,te3,te4;      // 2d effects
	    int te5,te6,te7,te8,te9,te10,te11,te12; // 3d particles
		//te5=te6=te7=te8=te9=te10=0;
	    te11=0; te12=1;

	    switch( num+1 )
	    {
	    case 1:
					te0=0x37; te1=0x4A; te2=0x00; te3=15; 
	                te4=0x00; te5=0x00; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0x8a; te10=0xffffff00;            
	                break;
	    case 3:     te0=0x37; te1=0x4A; te2=0x00; te3=15; 
	                te4=0x37; te5=0x79; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0x8c; te10=0xffffff00;
	                break;
	    case 4:     te0=0x37; te1=0x6A; te2=0x09; te3=0x06; 
	                te4=0x37; te5=0x6A; te6=0x09; te7=0x20;
	                te8=0x13; te9=0x8d; te10=0xffffff00;
	                break;
	    case 7:     te0=0x37; te1=0x3A; te2=0x00; te3=15;
	                te4=0x37; te5=0x6A; te6=0x09; te7=0x20;
	                te8=0x13; te9=0x90; te10=0xffffff03;
	                break; 
	    case 8:     te0=0x37; te1=0x4A; te2=0x00; te3=15;
	                te4=0x37; te5=0x79; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0x91; te10=0xffffff00;
	                break;
	    case 9:     te0=0x37; te1=0x3A; te2=0x00; te3=15;
	                te4=0x37; te5=0x5a; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0x92; te10=0xffffff03;
	                break;
	    case 10:    te0=0x37; te1=0x3A; te2=0x00; te3=15;
	                te4=0x37; te5=0x5a; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0x93; te10=0xffffff00;
	                break;
	    case 11:    te0=0x37; te1=0x6A; te2=0x09; te3=0x06;
	                te4=0x37; te5=0x3a; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0x94; te10=0xffffff03;
	                break;
	    case 12:    te0=0x37; te1=0x4A; te2=0x09; te3=0x07; 
	                te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0x95; te10=0xffffff03;
	                break;
	    case 15:    te0=0x37; te1=0x6A; te2=0x09; te3=0x06; 
	                te4=0x37; te5=0x5a; te6=0x09; te7=0x14;
	                te8=0x13; te9=0x98; te10=0xffffff03;
	                break;
	    case 16:    te0=0x37; te1=0x3A; te2=0x00; te3=15; 
	                te4=0x37; te5=0x5a; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0x99; te10=0xffffff03;
	                break;
	    case 17:    te0=0x37; te1=0x3A; te2=0x00; te3=15; 
	                te4=0x37; te5=0x3a; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0x9a; te10=0xffffff02;
	                break;
	    case 20:    te0=0x37; te1=0x4A; te2=0x00; te3=15;
	                te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0x9d; te10=0xffffff03;
	                break;
	    case 22:    te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
	                te4=0x37; te5=0x28; te6=0x0a; te7=0x00;
	                te8=0x13; te9=0x9f; te10=0xffffff00;
	                break;
	    case 25:    te0=0x37; te1=0x6A; te2=0x09; te3=0x06; 
	                te4=0x37; te5=0x6a; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0x94; te10=0xffffff03;
	                break;
	    case 26:    te0=0x37; te1=0x5A; te2=0x09; te3=0x06; 
	                te4=0x37; te5=0x5a; te6=0x09; te7=0x14;
	                te8=0x13; te9=0xa3; te10=0xffffff03;
	                break;
	    case 27:    te0=0x37; te1=0x4A; te2=0x00; te3=15;
	                te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0xa4; te10=0xffffff03;
	                break;
	    case 29:    te0=0x37; te1=0x6A; te2=0x09; te3=0x06; 
	                te4=0x37; te5=0x6a; te6=0x09; te7=0x20;
	                te8=0x13; te9=0xa6; te10=0xffffff03;
	                break;
	    case 49:
	    case 30:    te0=0x37; te1=0x4A; te2=0x00; te3=15;       
	                te4=0x00; te5=0x00; te6=0x0a; te7=0x05;
	                te8=0x13; te9=0xa7; te10=0xffffff03;
	                break;
	    case 31:    te0=0x37; te1=0x4A; te2=0x00; te3=15;       
	                te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0xa8; te10=0xffffff00;
	                break;
	    case 32:    te0=0x37; te1=0x4A; te2=0x00; te3=15;       
	                te4=0x00; te5=0x00; te6=0x0a; te7=0x05;
	                te8=0x13; te9=0xa9; te10=0xffffff03;
	                break;
	    case 33:    te0=0x37; te1=0x4A; te2=0x00; te3=15;       
	                te4=0x37; te5=0x28; te6=0x0a; te7=0x00;
	                te8=0x13; te9=0xaa; te10=0xffffff00;
	                break;
	    case 35:    te0=0x37; te1=0x4A; te2=0x00; te3=15;       
	                te4=0x37; te5=0x3a; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0xac; te10=0xffffff00;
	                break;
	    case 36:    te0=0x37; te1=0x3A; te2=0x00; te3=15;
	                te4=0x37; te5=0x5a; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0xad; te10=0xffffff03;                
	                break;
	    case 37:    te0=0x37; te1=0x4A; te2=0x00; te3=15;               
	                te4=0x00; te5=0x00; te6=0x0a; te7=0x0f;
	                te8=0x07; te9=0xf6; te10=0xffffff00;    
	                break;
	    case 38:    te0=0x37; te1=0x35; te2=0x00; te3=30;
	                te4=0x37; te5=0x6a; te6=0x06; te7=0x01;
	                te8=0x13; te9=0xaf; te10=0xffffff05;
	                break;
	    case 40:
	    case 48:    te0=0x37; te1=0x35; te2=0x00; te3=30;
	                te4=0x37; te5=0x28; te6=0x0a; te7=0x0a;
	                te8=0x13; te9=0xb1; te10=0xffffff00;
	                break;
	    case 43:    te0=0x36; te1=0xB0; te2=0x09; te3=0x09; 
	                te4=0x36; te5=0xbd; te6=0x0a; te7=0x0a;
	                te8=0x13; te9=0xb4; te10=0xffffff00;
	                break;
	    case 46:    te0=0x37; te1=0x4A; te2=0x00; te3=15; 
	                te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0x95; te10=0xffffff03;         
	                break;      
	    case 51:    te0=0x37; te1=0x09; te2=0x09; te3=0x19;                
	                te4=0x37; te5=0x09; te6=0x0a; te7=0x1e;
	                te8=0x13; te9=0xbc; te10=0xffffff05;
	                break;
	    case 53:    te0=0x37; te1=0x4A; te2=0x00; te3=15; 
	                te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0xbe; te10=0xffffff00;        
	                break;
	    case 54:    te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
	                te4=0x37; te5=0x28; te6=0x0a; te7=0x0a;
	                te8=0x13; te9=0x9f; te10=0xffffff00;        
	                break;
	    case 55:    te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
	                te4=0x00; te5=0x00; te6=0x0a; te7=0x05;
	                te10=rand()%3;
	                switch (te10)
	                {
	                  case 0: te8=0x13; te9=0xc0; break;
	                  case 1: te8=0x17; te9=0xa8; break;
	                  case 2: te9=0x1b; te9=0x90; break;
	                  default: te8=0x13; te9=0xc0; 
	                }
	                te10=0xffffff00;        
	                break;
	    case 58:    te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
	                te4=0x00; te5=0x00; te6=0x0a; te7=0x05;
	                te8=0x13; te9=0xc3; te10=0xffffff00;
	                break;
	    case 59:    te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
	                te4=0x36; te5=0x6a; te6=0x09; te7=0x20;
	                te8=0x25; te9=0x1d; te10=0xffffff03;            
	                break;
	    case 60:    te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
	                te4=0x00; te5=0x00; te6=0x0a; te7=0x20;
	                te8=0x13; te9=0xc5; te10=0xffffff03;            
	                //te11=27; te12=0xf;
	                break;
	    case 61:    te0=0x37; te1=0x2A; te2=0x09; te3=0x06;
	                te4=0x00; te5=0x00; te6=0x0a; te7=0x00;
	                te8=0x13; te9=0xc6; te10=0xffffff03;            
	                break;
	    case 62:    te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
	                te4=0x00; te5=0x00; te6=0x0a; te7=0x20;
	                te8=0x13; te9=0xc7; te10=0xffffff03;            
	                break;
	    case 63:    te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
	                te4=0x00; te5=0x00; te6=0x0a; te7=0x20;
	                te8=0x13; te9=0xc8; te10=0xffffff03;            
	                break;
	    case 64:    te0=0x37; te1=0x2A; te2=0x09; te3=0x06; 
	                te4=0x00; te5=0x00; te6=0x09; te7=0x00;
	                te8=0x13; te9=0xc9; te10=0xffffff03;            
	                break;

	    case 66:    te0=0x36; te1=0xB0; te2=0x09; te3=0x09; 
	                te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f;
	                te8=0x13; te9=0x9e; te10=0xffffff00;    
	                break;

	    case 99:    te0=0x37; te1=0x35; te2=0x00; te3=0x30;  // fizzle
	                te4=0x00; te5=0x00; te6=0x0a; te7=0x0f;
	                te8=0x0f; te9=0xcb; te10=0xffffff01;    
	                break;           

	    default:    te0=-1; te1=-1; te2=-1; te3=-1; break;
	    }

	    effect[0]=te0; effect[1]=te1; effect[2]=te2; effect[3]=te3; effect[4]=te4;
	    effect[5]=te5; effect[6]=te6; effect[7]=te7; effect[8]=te8; effect[9]=te9;
	    effect[10]=te10; effect[11]=te11; effect[12]=te12;
		
	}



	///////////////////////////////////////////////////////////////////
	// Function name     : ParticleFx::initWithSpellStatEffects_pre
	// Description       : inits stat effect particle for spell casting
	// Return type       : void 
	// Author            : Lord Binary, Xanathar 
	// Argument          : magic::SpellId num -> spell number
	// Changes           : none yet
	void ParticleFx::initWithSpellStatEffects_pre( magic::SpellId num)
	{
	   
		for (int i = 0; i<=18; i++) effect[i] = -1;

	    int te4,te5,te6,te7,te8,te9,te10,te11,te12; // 3d particles

	    te11=0; te12=1;
		te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x23; te10=0xffff0102;
		
		if (num == magic::SPELL_BLADESPIRITS) te10=0x02010102;
		int arte9[magic::MAX_SPELLS] = {
			0x47, 0x33, 0x47, 0x65, 0x51, 0x47, 0x33, 0x47,
			0x65, 0x65, 0x65, 0x51, 0x29, 0x29, 0x33, 0x65,
			0x51, 0x51, 0x29, 0x5b, 0x47, 0x47, 0x29, 0x33,
			0x65, 0x33, 0x47, 0x51, 0x65, 0x3d, 0x47, 0x47,
			0x50, 0x2a, 0x2a, 0x34, 0x48, 0x34, 0x5c, 0x50, 
			0x2a, 0x3e, 0x51, 0x2a, 0x2a, 0x47, 0x34, 0x2a,
			0x3e, 0x3e, 0x52, 0x48, 0x48, 0x2a, 0x52, 0x2a,
			0x34, 0x48, 0x66, 0x32, 0x5a, 0x3c, 0x5a, 0x6e };
		
		if ((num<0)||(num>=magic::MAX_SPELLS)) { 
			te4=-1; te5=-1; te6=-1; te7=-1; 
		} else te9 = arte9[num];
	    

	    effect[4]=te4; effect[5]=te5; effect[6]=te6; effect[7]=te7;
	    effect[8]=te8; effect[9]=te9; effect[10]=te10; effect[11]=te11;
	    effect[12]=te12;
	    
	}


	///////////////////////////////////////////////////////////////////
	// Function name     : ParticleFx::initWithSpellStatEffects_post
	// Description       : inits particle effects with fx for end of casting
	// Return type       : void 
	// Author            : Lord Binary, Xanathar 
	// Argument          : magic::SpellId num -> spell number
	// Changes           : none yet
	void ParticleFx::initWithSpellStatEffects_post( magic::SpellId num)
	{
		for (int i = 0; i<=18; i++) effect[i] = -1;
	    int te4,te5,te6,te7,te8,te9,te10, te11,te12;

	    te11=0; te12=1;
	    switch( num+1 )
	    {
			case magic::SPELL_NIGHTSIGHT:    
				te4=0x37; te5=0x6a; te6=0x09; te7=0x20; te8=0x13; te9=0x8f; te10=0xffff0003; 
				break;        
			case magic::SPELL_MINDBLAST:   
				te4=0x37; te5=0x4a; te6=0x0a; te7=0x0f; te8=0x13; te9=0xae; te10=0xffff0000; 
				break;
			case magic::SPELL_PARALYZE:   
				te4=0x00; te5=0x00; te6=0x0a; te7=0x05; te8=0x13; te9=0xaf; te10=0xffffff05; te11=0x27; te12=0x0f; 
				break;
			default: te4=-1; te5=-1; te6=-1; te7=-1; break;
	    }

	    effect[4]=te4; effect[5]=te5; effect[6]=te6; effect[7]=te7;
	    effect[8]=te8; effect[9]=te9; effect[10]=te10; effect[11]=te11;
	    effect[12]=te12;    
	}




	///////////////////////////////////////////////////////////////////
	// Function name     : ParticleFx::initWithSpellStatEffects_item
	// Description       : inits stat effect particle for item casting
	// Return type       : void 
	// Author            : Lord Binary, Xanathar 
	// Argument          : magic::SpellId num -> spell number
	// Changes           : none yet
	void ParticleFx::initWithSpellStatEffects_item( magic::SpellId num)
	{
		for (int i = 0; i<=18; i++) effect[i] = -1;
	    int te4,te5,te6,te7,te8,te9,te10, te11;

	    switch( num )
	    {     
	       case 13:   te4=0x37; te5=0x6a; te6=0x09; te7=0x0a; te8=0x13; te9=0x96; te10=0xffff0000; te11=0; break;
	       case 14:   te4=0x37; te5=0x6a; te6=0x09; te7=0x20; te8=0x13; te9=0x8f; te10=0xffff0003; te11=0; break;
	       case 19:   te4=0x37; te5=0x6a; te6=0x09; te7=0x20; te8=0x13; te9=0x9c; te10=0xffff0002; te11=0; break;
	       case 22:   te4=0x37; te5=0x28; te6=0x0a; te7=0x00; te8=0x07; te9=0xe7; te10=0xffff0002; te11=0; break;
	       case 23:   te4=0x37; te5=0x6a; te6=0x09; te7=0x20; te8=0x13; te9=0xa0; te10=0xffff0003; te11=0; break;
	       case 24:   te4=0x37; te5=0x6a; te6=0x09; te7=0x0a; te8=0x13; te9=0xa1; te10=0xffff0003; te11=0; break;
	       case 28:   te4=0x37; te5=0x6a; te6=0x0f; te7=0x0a; te8=0x13; te9=0xa5; te10=0xffff0000; te11=0; break;
	       case 32:   te4=0x00; te5=0x00; te6=0x00; te7=0x00; te8=0x13; te9=0xa9; te10=0xffff0003; te11=0; break;
	       case 39:   te4=0x37; te5=0x6a; te6=0x09; te7=0x0a; te8=0x13; te9=0xb0; te10=0xffff0000; te11=0; break;
	       case 44:   te4=0x37; te5=0x6a; te6=0x0a; te7=0x0f; te8=0x13; te9=0xb5; te10=0xffff0003; te11=0; break;
	       case 45:   te4=0x37; te5=0x79; te6=0x0a; te7=0x0f; te8=0x13; te9=0xb6; te10=0xffff0003; te11=0; break;
	       case 47:   te4=0x37; te5=0x6a; te6=0x09; te7=0x0a; te8=0x13; te9=0xb8; te10=0xffff0003; te11=0; break;
	       case 50:   te4=0x37; te5=0x6a; te6=0x09; te7=0x0a; te8=0x13; te9=0xbb; te10=0xffff0003; te11=0; break;
	       case 59:   te4=0x00; te5=0x00; te6=0x00; te7=0x00; te8=0x13; te9=0xc4; te10=0xffff0003; te11=0; break;

	       default: te4=-1; te5=-1; te6=-1; te7=-1; break;
	    }

	    effect[4]=te4; effect[5]=te5; effect[6]=te6; effect[7]=te7;
	    effect[8]=te8; effect[9]=te9; effect[10]=te10; effect[11]=te11;
	}

} // namespace
