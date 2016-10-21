#pragma once
#include "spine/resmgr.hpp"
#include "frea/vector.hpp"
#include "handle.hpp"

namespace rev {
	extern const int InputRange,
					InputRangeHalf;
	struct TPos2D : Resource {
		bool		bTouch;
		// 画面に対する0〜1の比率で記録する
		frea::Vec2	absPos,
					relPos;
		float		pressure;

		const static struct tagClean {} Clean;
		TPos2D() = default;
		TPos2D(tagClean) noexcept;
		void setNewAbs(const frea::Vec2& p) noexcept;
		void setNewRel(const frea::Vec2& p) noexcept;
		const char* getResourceName() const noexcept override;
	};
	#define mgr_pointer (::rev::PointerMgr::ref())
	class PointerMgr : public spi::ResMgr<TPos2D>, public spi::Singleton<PointerMgr> {};

	struct RecvPointer {
		virtual void newPointer(WPtr wptr) = 0;
		virtual ~RecvPointer() {}
	};
	using RecvPointer_SP = std::shared_ptr<RecvPointer>;

	struct RecvPtrGroup : RecvPointer {
		using LSet = std::unordered_set<RecvPointer_SP>;
		LSet	listener;

		void newPointer(WPtr wptr) override;
	};

	enum class InputType {
		Keyboard,
		Mouse,
		Joypad,
		Touchpad
	};
	enum class MouseMode {
		Absolute,
		Clipping,
		Relative
	};
}
