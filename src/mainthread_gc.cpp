#include "mainthread.hpp"

namespace rev {
	void MainThread::_FlyweightGC() {
		// SName
		spi::FlyweightItem<std::string>::GC();
		// FWVMap
		FWVMap::GC();
		// FWVDecl
		FWVDecl::GC();
	}
}
