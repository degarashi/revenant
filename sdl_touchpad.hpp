#pragma once
#include "input_constant.hpp"
#include "handle.hpp"
#include <map>
#include <vector>
#include <SDL_events.h>

namespace rev {
	struct RecvPointer;
	class SDLTouchpad {
		// TouchpadID -> Queue
		using EventQ = std::map<int, std::vector<SDL_Event>>;
		static EventQ	s_eventQ[2];
		static int		s_evSw;

		// FingerID -> PtrHandle
		using FMap = std::map<uint32_t, HPtr>;
		FMap	_fmap;
		int		_touchId;
		static HInput s_hInput;
		protected:
			template <class Maker>
			static HInput OpenTouchpad(const int num) {
				if(!s_hInput) {
					s_hInput = Maker()(num);
					_Initialize();
				}
				return s_hInput;
			}
			bool dep_scan(RecvPointer* r);
			static void _Initialize() noexcept;
			WPtr dep_getPointer() const;
			const std::string& name() const noexcept;
		public:
			SDLTouchpad(int touchId) noexcept;
			static void Update() noexcept;
			static void Terminate() noexcept;
			static int NumTouchpad() noexcept;
			static int ProcessEvent(void*, SDL_Event* e);
	};
	using TouchDep = SDLTouchpad;
}
