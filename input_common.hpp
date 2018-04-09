#pragma once
#include "spine/resmgr.hpp"
#include "frea/vector.hpp"
#include "spine/singleton.hpp"
#include "handle/input.hpp"
#include "spine/enum.hpp"
#include "resource.hpp"

namespace rev {
	struct TPos2D : Resource {
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
}
