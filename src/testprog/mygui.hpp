#pragma once
#include "../debug_gui/logger.hpp"
#include "../drawable.hpp"

namespace rev {
	namespace info {
		class GPU;
	}
	class ResourceView;
	namespace debug {
		class Profiler;
	}
	namespace test {
		class MyGUI :
			public DrawableObjT<MyGUI>,
			public std::enable_shared_from_this<MyGUI>
		{
			private:
				template <class T>
				using SP = std::shared_ptr<T>;
				HDGroup						_dg;
				Logger						_logger;
				SP<info::GPU>				_gpu;
				SP<ResourceView>		_resview;
				SP<debug::Profiler>		_profiler;
				mutable struct {
					bool demo = false;
					bool log = false;
					bool gpu = false;
					bool profile = false;
				} _show;

				struct St;
			public:
				MyGUI(const HDGroup& dg);
				DEF_DEBUGGUI_NAME
		};
	}
}
