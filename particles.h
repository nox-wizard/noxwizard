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
\brief particle data
*/
#ifndef __PARTICLES_H__
#define __PARTICLES_H__
namespace particles
{
	class ParticleFx;
}

#include "magic.h"

namespace particles
{

	/*!
	\brief Handling of ParticleFX
	*/
	class ParticleFx {
	private:

	public:
		int effect[18];
		void initWithSpellStatEffects_item( magic::SpellId num);
		void initWithSpellStatEffects_post( magic::SpellId num);
		void initWithSpellStatEffects_pre( magic::SpellId num);
		void initWithSpellStatEffect( magic::SpellId num );
		void initWithSpellMoveEffect( magic::SpellId num );
	};

	void staticeffectUO3D(P_CHAR pc_cs, ParticleFx *sta, UI08 *particleSystem);
	void movingeffectUO3D(P_CHAR pc_cs, P_OBJECT po_cd, ParticleFx *eff, UI08 *particleSystem);
	void bolteffectUO3D(P_CHAR pc_cs, UI08 *particleSystem);
	void itemeffectUO3D(P_ITEM pi, ParticleFx *sta, UI08 *particleSystem);

} // namespace
#endif //__PARTICLES_H__
