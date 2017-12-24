#pragma once 
#include "handle.hpp"
#include "uniform_map.hpp"
#include <unordered_set>
#include <vector>

namespace rev {
	struct VSemAttr;
	using VSemAttrV = std::vector<VSemAttr>;
	struct IEffect;
	struct GLState;
	using GLState_SP = std::shared_ptr<GLState>;
	using GLState_SPV = std::vector<GLState_SP>;
	class Material {
		protected:
			struct Runtime {
				using UniIdSet = std::unordered_set<GLint>;
				//! Uniform非デフォルト値エントリIdセット (主にユーザーの入力チェック用)
				UniIdSet		noDefValue;
				//! Uniformデフォルト値と対応するId
				UniformMap		defaultValue;
				//! Attribute: 頂点セマンティクスに対する頂点Id
				VSemAttrV		vattr;

				void clear();
			};
			virtual void _onDeviceReset(const IEffect& e, Runtime&) = 0;

			HProg			_program;
			//! Setting: Uniformデフォルト値(texture, vector, float, bool)設定を含む
			//! GLDeviceの設定クラスリスト
			GLState_SPV		_setting;
			Runtime			_runtime;
		private:
			//! lost/resetのチェック用 (Debug)
			bool			_bInit = false;	

		public:
			//! OpenGL関連のリソースを解放
			/*! GLResourceの物とは別。GLEffectから呼ぶ */
			void ts_onDeviceLost();
			void ts_onDeviceReset(const IEffect& e);

			const HProg& getProgram() const noexcept;
			const Runtime& getRuntime() const noexcept;
			//! OpenGLに設定を適用
			void applySetting() const;
	};
	using Material_SP = std::shared_ptr<Material>;
}
