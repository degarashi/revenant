#pragma once
#include "vertex.hpp"
#include "prog_unif.hpp"

namespace rev {
	struct IEffect;
	struct VSemAttr;
	using VSemAttrV = std::vector<VSemAttr>;
	using UniIdSet = std::unordered_set<GLint>;
	struct GLState;
	using GLState_SP = std::shared_ptr<GLState>;
	using GLState_SPV = std::vector<GLState_SP>;
	class Tech {
		private:
			HFx				_fx;
		protected:
			//! Uniformデフォルト値(と対応するId)
			Prog_Unif		_prog_unif;
			//! Setting: Uniformデフォルト値(texture, vector, float, bool)設定を含む
			//! GLDeviceの設定クラスリスト
			GLState_SPV		_setting;
			//! Uniform非デフォルト値エントリIdセット (主にユーザーの入力チェック用)
			UniIdSet		_noDefValue;
			//! Attribute: 頂点セマンティクスに対する頂点Id
			VSemAttrV		_vattr;

			Name			_name;
		public:
			virtual ~Tech() {}

			//! OpenGLに設定を適用
			void applySetting() const;
			const UniIdSet& getNoDefaultValue() const noexcept;
			const VSemAttrV& getVAttr() const noexcept;
			const HProg& getProgram() const noexcept;
			const UniformMap& getDefaultValue() const noexcept;
			const Name& getName() const noexcept;
	};
	using Tech_SP = std::shared_ptr<Tech>;
}
