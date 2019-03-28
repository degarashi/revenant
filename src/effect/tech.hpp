#pragma once
#include "tech_if.hpp"
#include "vertex.hpp"
#include "../drawcmd/cmd.hpp"

namespace rev {
	class UniformEnt;
	class Tech : public ITech {
		protected:
			HProg			_program;
			//! Setting: Uniformデフォルト値(texture, vector, float, bool)設定を含む
			//! GLDeviceの設定クラスリスト
			GLState_SPV		_setting;
			//! Uniform非デフォルト値エントリIdセット (主にユーザーの入力チェック用)
			UniIdSet		_noDefValue;
			//! Attribute: 頂点セマンティクスに対する頂点Id
			FWVMap			_vmap;
			Name			_name;
			// 必ず継承して使う
			Tech() {}

			virtual void _makeUniform(UniformEnt& u) const;

		public:
			const GLState_SPV& getSetting() const override;
			const UniIdSet& getNoDefaultValue() const noexcept override;
			const FWVMap& getVMap() const noexcept override;
			const HProg& getProgram() const noexcept override;
			const Name& getName() const noexcept override;
			void dcmd_uniform(draw::IQueue& q) const override;
			void dcmd_setup(draw::IQueue& q) const override;
			void dcmd_resetState(draw::IQueue& q) const override;
			DEF_DEBUGGUI_PROP
	};
}
