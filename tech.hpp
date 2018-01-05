#pragma once
#include "tech_if.hpp"
#include "uniform_ent.hpp"

namespace rev {
	class Tech : public ITech {
		private:
			HFx				_fx;
		protected:
			//! Uniformデフォルト値(と対応するId)
			UniformEnt		_uniform;
			//! Setting: Uniformデフォルト値(texture, vector, float, bool)設定を含む
			//! GLDeviceの設定クラスリスト
			GLState_SPV		_setting;
			//! Uniform非デフォルト値エントリIdセット (主にユーザーの入力チェック用)
			UniIdSet		_noDefValue;
			//! Attribute: 頂点セマンティクスに対する頂点Id
			VSemAttrV		_vattr;

			Name			_name;
		public:
			const GLState_SPV& getSetting() const override;
			const UniIdSet& getNoDefaultValue() const noexcept override;
			const VSemAttrV& getVAttr() const noexcept override;
			const HProg& getProgram() const noexcept override;
			const UniformEnt& getDefaultValue() const noexcept override;
			const Name& getName() const noexcept override;
	};
}
