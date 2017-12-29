#pragma once
#include "gl_header.hpp"
#include "uniform_map.hpp"
#include "handle.hpp"
#include <vector>
#include <unordered_set>

namespace rev {
	struct IEffect;
	struct VSemAttr;
	using VSemAttrV = std::vector<VSemAttr>;
	using UniIdSet = std::unordered_set<GLint>;
	struct ITech {
		struct Runtime {
			//! Uniform非デフォルト値エントリIdセット (主にユーザーの入力チェック用)
			UniIdSet		noDefValue;
			//! Uniformデフォルト値と対応するId
			UniformMap		defaultValue;
			//! Attribute: 頂点セマンティクスに対する頂点Id
			VSemAttrV		vattr;
			//! [UniformId -> TextureActiveIndex]
			using TexIndex = std::unordered_map<GLint, GLint>;
			TexIndex		texIndex;

			void clear();
		};
		//! OpenGL関連のリソースを解放
		/*! GLResourceの物とは別。GLEffectから呼ぶ */
		virtual void ts_onDeviceLost() = 0;
		virtual void ts_onDeviceReset(const IEffect& e) = 0;
		//! OpenGLに設定を適用
		virtual void applySetting() const = 0;
		virtual const HProg& getProgram() const noexcept = 0;
		virtual const Runtime& getRuntime() const noexcept = 0;
		virtual const Name& getName() const noexcept;
	};
	using Tech_SP = std::shared_ptr<ITech>;
}
