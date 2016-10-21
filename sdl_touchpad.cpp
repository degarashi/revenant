#include "input_dep_sdl.hpp"

namespace rev {
	HInput SDLTouchpad::s_hInput;
	SDLTouchpad::EventQ SDLTouchpad::s_eventQ[2];
	int SDLTouchpad::s_evSw = 0;

	SDLTouchpad::SDLTouchpad(const int touchId) noexcept:
		_touchId(touchId)
	{}
	bool SDLTouchpad::dep_scan(RecvPointer* r) {
		// 貯めておいたイベントを処理
		auto& q = s_eventQ[s_evSw][_touchId];
		if(!q.empty()) {
			// ポインタが増える際はrのaddNewPointer()を呼ぶ
			for(auto& e : q) {
				auto& f = e.tfinger;
				auto itr = _fmap.find(f.fingerId);
				switch(e.type) {
					case SDL_FINGERDOWN: {
						HPtr hP = mgr_pointer.emplace();
						r->newPointer(hP);
						_fmap[f.fingerId] = std::move(hP);
					} break;
					case SDL_FINGERMOTION:
						if(itr != _fmap.end())
							itr->second->setNewAbs(frea::Vec2(f.dx, f.dy));
						break;
					case SDL_FINGERUP:
						if(itr != _fmap.end())
							_fmap.erase(itr);
						break;
				}
			}
			q.clear();
		}
		return true;
	}
	int SDLTouchpad::NumTouchpad() noexcept {
		return 1;
	}
	int SDLTouchpad::ProcessEvent(void*, SDL_Event* e) {
		// タッチパネル関係のイベントだけコピー
		if(e->type >= SDL_FINGERDOWN &&
			e->type <= SDL_FINGERMOTION)
		{
			s_eventQ[s_evSw^1][e->tfinger.touchId].push_back(*e);
			return 1;
		}
		return 0;
	}
	void SDLTouchpad::_Initialize() noexcept {
		SDL_AddEventWatch(SDLTouchpad::ProcessEvent, nullptr);
	}
	void SDLTouchpad::Update() noexcept {
		s_eventQ[s_evSw].clear();
		s_evSw ^= 1;
	}
	void SDLTouchpad::Terminate() noexcept {
		if(s_hInput) {
			SDL_DelEventWatch(SDLTouchpad::ProcessEvent, nullptr);
			s_hInput.reset();
		}
	}
	namespace {
		const std::string c_name("(default touchpad)");
	}
	const std::string& SDLTouchpad::name() const noexcept {
		return c_name;
	}
	WPtr SDLTouchpad::dep_getPointer() const {
		// 何かFingerIDがあれば適当に1つ返す
		if(!_fmap.empty())
			return (_fmap.begin())->second;
		return WPtr();
	}
}
