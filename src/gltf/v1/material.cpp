#include "gltf/v1/material.hpp"
#include "gltf/v1/technique.hpp"
#include "gltf/v1/program.hpp"
#include "gltf/v1/texture.hpp"
#include "../../ovr_functor.hpp"
#include "../../effect/vertex_map.hpp"
#include "../../effect/uniform_ent.hpp"

namespace rev::gltf::v1 {
	using namespace loader;
	Material::Material(const JValue& v, const IDataQuery& q):
		Resource(v),
		technique(Required<DRef_Technique>(v, "technique", q))
	{
		auto itr = v.FindMember("values");
		if(itr != v.MemberEnd()) {
			const JValue& values = itr->value;
			itr = values.MemberBegin();
			while(itr != values.MemberEnd()) {
				uniformOvr.emplace_back(itr->name.GetString(), LoadUniformValue(itr->value, q));
				++itr;
			}
		}
	}
	Resource::Type Material::getType() const noexcept {
		return Type::Material;
	}
	void Material::GLTfTech::_makeUniform(UniformEnt& u) const {
		_makeUniformF(u);
	}
	namespace {
		struct TextureCnv {
			template <class T>
			UniformValue operator()(const T& t) const {
				return t;
			}
			UniformValue operator()(const DRef_Texture& t) const {
				return HTexC(t.data()->getGLResource());
			}
			UniformValue operator()(const std::vector<DRef_Texture>& t) const {
				const auto len = t.size();
				std::vector<HTexC> tex(len);
				for(std::size_t i = 0 ; i<len ; i++) {
					tex[i] = t[i].data()->getGLResource();
				}
				return std::move(tex);
			}
		};
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

			const auto findValue = [&ovr = mtl.uniformOvr](const Name& name) -> const UniformValue* {
				const auto itr = std::find_if(ovr.begin(), ovr.end(),
						[&name](const auto& t){
							return t.first == name;
						});
				if(itr != ovr.end())
					return &itr->second;
				return nullptr;
			};
			struct Unif {
				GLSLName		name;
				GLenum			type;
				std::size_t		count;
				UniformValue	value;

				void setUniform(UniformEnt& ent) const {
					SetUniform(ent, name, value, type, count);
				}
			};
			std::vector<Unif> uniforms;
			// Uniformデフォルト変数
			for(auto& p : tech.param.fixedUniform) {
				auto& name = p.first;
				auto& fixed = *p.second;

				const UniformValue* value;
				// Materialが同名の変数を持っていれば上書き
				if(auto* v = findValue(name))
					value = v;
				else
					value = &fixed.value;
				Assert0(value);

				auto value2 = boost::apply_visitor(TextureCnv(), *value);
				auto& glsl_name = tech.namecnv.uniform.at(name);
				uniforms.emplace_back(Unif{
					.name = glsl_name,
					.type = fixed.type,
					.count = fixed.count,
					.value = std::move(value2)
				});
			}
			// Uniform必須変数
			for(auto& u : tech.param.typedUniform) {
				auto& name = u.first;
				auto& typed = *u.second;

				auto* value = findValue(name);
				Assert0(value);

				auto value2 = boost::apply_visitor(TextureCnv(), *value);
				auto& glsl_name = tech.namecnv.uniform.at(name);
				uniforms.emplace_back(Unif{
					.name = glsl_name,
					.type = typed.type,
					.count = typed.count,
					.value = std::move(value2)
				});
			}

			_makeUniformF = [uniforms = std::move(uniforms)](UniformEnt& ent){
				for(auto& u : uniforms) {
					u.setUniform(ent);
				}
			};
			// ---- 頂点アトリビュート ----
			VSemAttrMap vm;
			for(auto& a : tech.param.attribute) {
				const auto& glsl_name = tech.namecnv.attribute.at(a.first);
				if(const auto at = prog->getAttrib(glsl_name)) {
					const auto& ap = *a.second;
					vm.emplace(VSem_AttrId{
						.sem = ap.semantic,
						.attrId = int(at->id),
						.bInteger = at->bInteger
					});
				}
			}
			_vmap = vm;
		} else {
			// デフォルトのマテリアル
			D_Assert(false, "default material is not implemented yet");
		}
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
