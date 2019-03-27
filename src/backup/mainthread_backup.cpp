#include "mainthread.hpp"
#include "../sdl/rw.hpp"
#include "../sound/sound.hpp"
#include "serialization/sdl_rw.hpp"
#include "serialization/uri_file.hpp"
#include "serialization/uri_data.hpp"
#include "serialization/sound.hpp"
#include "serialization/uriwrap.hpp"

namespace rev {
	void MainThread::_Backup(Manager& m, std::ostream& os) {
		cereal::BinaryOutputArchive oa(os);
		oa(m.rwm);
		oa(m.snd);
		m.snd->invalidate();
		m.snd.reset();
	}
	void MainThread::_Restore(Manager& m, std::istream& is) {
		cereal::BinaryInputArchive ia(is);
		ia(m.rwm);
		ia(m.snd);
		m.snd->makeCurrent();
		m.snd->update();
	}
}
