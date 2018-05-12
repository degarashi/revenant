#pragma once
#include "tech_if.hpp"
#include "uniform_ent.hpp"
#include "vertex.hpp"

namespace rev {
	class Tech : public ITech {
		protected:
			//! Uniformデフォルト値設定DrawCommand
			UniformEnt		_uniformDefault;
			//! Setting: Uniformデフォルト値(texture, vector, float, bool)設定を含む
			//! GLDeviceの設定クラスリスト
			GLState_SPV		_setting;
			//! Uniform非デフォルト値エントリIdセット (主にユーザーの入力チェック用)
			UniIdSet		_noDefValue;
			//! Attribute: 頂点セマンティクスに対する頂点Id
			VSemAttrV		_vattr;

			//! [Program + GLSetting + UniformDefault]
			draw::CommandVec	_setupCmd;
			void _makeSetupCmd();

			Name			_name;
			// 必ず継承して使う
			Tech() {}
		public:
			const GLState_SPV& getSetting() const override;
			const UniIdSet& getNoDefaultValue() const noexcept override;
			const VSemAttrV& getVAttr() const noexcept override;
			const HProg& getProgram() const noexcept override;
			const UniformEnt& getDefaultValueQ() const noexcept override;
			const Name& getName() const noexcept override;
			void dcmd_setup(draw::IQueue& q) const override;
			DEF_DEBUGGUI_PROP
	};
}
