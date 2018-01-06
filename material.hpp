#pragma once
#include "tech_if.hpp"
#include "spine/rflag.hpp"
#include "uniform_ent.hpp"

namespace rev {
	struct GLState;
	using GLState_SP = std::shared_ptr<GLState>;
	using GLState_SPV = std::vector<GLState_SP>;
	//! Techに追加パラメータを付加した物
	class Material : public ITech {
		private:
			#define SEQ \
				((Base)(Tech_SP)) \
				((M_Uniform)(UniformEnt)(Base)) \
				((F_DefaultValue)(UniformEnt)(M_Uniform)) \
				((M_Setting)(GLState_SPV)) \
				((F_Setting)(GLState_SPV)(Base)(M_Setting))
			// M_Uniform = ベースとするTechの必須パラメータへ渡す値
			// M_Setting = ベースに加えて追加でセットする値
			// F_DefaultValue = BaseのUniformデフォルト変数にMaterialのデフォルト変数を上書きした物
			// F_Setting = Base::Setting + M_Setting
			RFLAG_DEFINE(Material, SEQ)
		public:
			RFLAG_GETMETHOD_DEFINE(SEQ)
			RFLAG_SETMETHOD_DEFINE(SEQ)
			RFLAG_REFMETHOD_DEFINE(SEQ)
			#undef SEQ

			UniformEnt& refM_Uniform();
			const GLState_SPV& getSetting() const override;
			const UniIdSet& getNoDefaultValue() const noexcept override;
			const VSemAttrV& getVAttr() const noexcept override;
			const HProg& getProgram() const noexcept override;
			const UniformEnt& getDefaultValue() const noexcept override;
			const Name& getName() const noexcept override;
	};
}
