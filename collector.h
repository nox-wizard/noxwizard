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
\brief Declaration of Collector template
*/

#ifndef __COLLECTOR_H__
#define __COLLECTOR_H__


/*!
\brief Collector for delayed deletes
\author Xanathar
*/
template <typename T> class Collector {
	protected:
		stack<T*> m_stack;
	public:
		virtual ~Collector() { cleanup(); }
		virtual void push(T* ptr) {	m_stack.push(ptr); }
		virtual void cleanup()
		{
			while (!m_stack.empty()) {
			delete m_stack.top();
			m_stack.pop();
			}
		}
};

#endif //__COLLECTOR_H__


