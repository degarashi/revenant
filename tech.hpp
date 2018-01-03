#pragma once
#include "tech_if.hpp"
#include "prog_unif.hpp"

namespace rev {
	struct GLState;
	using GLState_SP = std::shared_ptr<GLState>;
	using GLState_SPV = std::vector<GLState_SP>;
	class Tech : public ITech {
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
			//! OpenGLに設定を適用
			void applySetting() const override;
			const UniIdSet& getNoDefaultValue() const noexcept override;
			const VSemAttrV& getVAttr() const noexcept override;
			const HProg& getProgram() const noexcept override;
			const UniformMap& getDefaultValue() const noexcept override;
			const Name& getName() const noexcept override;
	};
}
