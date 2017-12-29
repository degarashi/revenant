#pragma once 
#include "tech_if.hpp"

namespace rev {
	struct GLState;
	using GLState_SP = std::shared_ptr<GLState>;
	using GLState_SPV = std::vector<GLState_SP>;
	class Tech : public ITech {
		protected:
			virtual void _onDeviceReset(const IEffect& e, Runtime&) = 0;

			Name			_name;
			HProg			_program;
			//! Setting: Uniformデフォルト値(texture, vector, float, bool)設定を含む
			//! GLDeviceの設定クラスリスト
			GLState_SPV		_setting;
			Runtime			_runtime;
		private:
			//! lost/resetのチェック用 (Debug)
			bool			_bInit = false;	

		public:
			void ts_onDeviceLost() override;
			void ts_onDeviceReset(const IEffect& e) override;
			const HProg& getProgram() const noexcept override;
			const Runtime& getRuntime() const noexcept override;
			void applySetting() const override;
			const Name& getName() const noexcept override;
	};
}
