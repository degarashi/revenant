#pragma once
#include "pbr_mr.hpp"
#include "uniform_setter.hpp"
#include "singleton_data.hpp"

namespace rev {
	class U_PBRMetallicRoughness :
		public IUniformSetter
	{
		private:
			using ColorTex = SingletonData<GLTexture, U_PBRMetallicRoughness, 0>;
			using MRTex = SingletonData<GLTexture, U_PBRMetallicRoughness, 1>;
			using NormalTex = SingletonData<GLTexture, U_PBRMetallicRoughness, 2>;
			using OcclusionTex = SingletonData<GLTexture, U_PBRMetallicRoughness, 3>;
			using EmissiveTex = SingletonData<GLTexture, U_PBRMetallicRoughness, 4>;

			ColorTex		_colorTex;
			MRTex			_mrTex;
			NormalTex		_normalTex;
			OcclusionTex	_occlusionTex;
			EmissiveTex		_emissiveTex;
		public:
			static HTex MakeData(ColorTex*);
			static HTex MakeData(MRTex*);
			static HTex MakeData(NormalTex*);
			static HTex MakeData(OcclusionTex*);
			static HTex MakeData(EmissiveTex*);

			PBRMetallicRoughness	info;
			frea::Vec3				lightDir;
			FWVDecl					vdecl;
			HTexC					environment;

			UniformSetF getUniformF(const GLProgram& prog) const override;
	};
}
