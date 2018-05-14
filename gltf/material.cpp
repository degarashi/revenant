#include "gltf/material.hpp"
#include "../ovr_functor.hpp"
#include "gltf/technique.hpp"
#include "gltf/program.hpp"
#include "../vertex_map.hpp"
#include "../uniform_ent.hpp"

namespace rev::gltf {
	using namespace loader;
	Material::Material(const JValue& v):
		Resource(v),
		technique(Required<String>(v, "technique"))
	{
		auto itr = v.FindMember("values");
		if(itr != v.MemberEnd()) {
			const JValue& values = itr->value;
			itr = values.MemberBegin();
			while(itr != values.MemberEnd()) {
				uniformOvr.emplace_back(itr->name.GetString(), LoadUniformValue(itr->value));
				++itr;
			}
		}
	}
	Resource::Type Material::getType() const noexcept {
		return Type::Material;
	}
	void Material::resolve(const ITagQuery& q) {
		technique.resolve(q);
		for(auto& u : uniformOvr) {
			boost::apply_visitor(
				OVR_Functor {
					[](auto&&){},
					[&q](TagTexture& t){
						t.resolve(q);
					},
					[&q](std::vector<TagTexture>& t) {
						for(auto& t0 : t)
							t0.resolve(q);
					}
				},
				u.second
			);
		}
	}
	Material::GLTfTech::GLTfTech(const Material& mtl) {
		if(const auto& t = mtl.technique.data()) {
			auto& tech = *t;
			// Material-nameかTech-nameか、どちらか片方を採用
			if(auto&& name = mtl.username)
				_name = *name;
			else {
				if(auto&& name = tech.username)
					_name = *name;
			}
			// ---- OpenGL ステート設定 ----
			if(tech.state)
				_setting = tech.state->state;
			// ---- Uniform変数の登録 ----
			const auto& prog = tech.program.data()->makeProgram();
			_program = prog;
			UniformEnt ent(*prog, _uniformCmd);

			// Uniformデフォルト変数
			for(auto& u : tech.param.fixedUniform) {
				auto& name = u.first;
				auto& fixed = *u.second;

				const UniformValue* value;
				// Materialが同名の変数を持っていれば上書き
				if(const auto* v = mtl.findValue(name))
					value = v;
				else
					value = &fixed.value;
				auto& glsl_name = tech.namecnv.uniform.at(name);
				SetUniform(ent, glsl_name, *value, fixed.type, fixed.count);
			}
			// Uniform必須変数
			for(auto& u : tech.param.typedUniform) {
				auto& name = u.first;
				auto& typed = *u.second;

				const auto* v = mtl.findValue(name);
				Assert0(v);
				auto& glsl_name = tech.namecnv.uniform.at(name);
				SetUniform(ent, glsl_name, *v, typed.type, typed.count);
			}
			// ---- 頂点アトリビュート ----
			VSemAttrMap vm;
			for(auto& a : tech.param.attribute) {
				const auto& glsl_name = tech.namecnv.attribute.at(a.first);
				if(const auto id = prog->getAttribId(glsl_name)) {
					const auto& ap = *a.second;
					vm.emplace(VSem_AttrId{
						ap.semantic,
						*id
					});
				}
			}
			_vmap = vm;
			_makeSetupCmd();
		} else {
			// デフォルトのマテリアル
			D_Assert(false, "default material is not implemented yet");
		}
	}
	const UniformValue* Material::findValue(const Name& name) const {
		const auto itr = std::find_if(uniformOvr.begin(), uniformOvr.end(),
				[&name](const auto& t){
					return t.first == name;
				});
		if(itr != uniformOvr.end())
			return &itr->second;
		return nullptr;
	}

	const HTech& Material::getTech() const {
		if(!tech_cached) {
			tech_cached = std::make_shared<GLTfTech>(*this);
		}
		return tech_cached;
	}
	RTUParams_SP Material::getRT() const {
		Assert0(technique.data());
		return technique.data()->param.rtUniform;
	}
}
