#include "gltf/technique.hpp"
#include "gltf/check.hpp"
#include "gltf/dc_common.hpp"
#include "gltf/node.hpp"
#include "../gl_if.hpp"
#include "../gl_bstate.hpp"
#include "../gl_vstate.hpp"
#include "../camera3d.hpp"
#include "../ovr_functor.hpp"
#include "gltf/qm_usem_if.hpp"
#include "semantic_if.hpp"

namespace rev::gltf {
	namespace {
		// GLbooleanの時はunsigned char -> Integerと解釈されてしまうので特別にbool変換
		template <class T>
		struct GLArgCnv { using type = T; };
		template <>
		struct GLArgCnv<GLboolean> { using type = bool; };

		struct I_VStateMaker {
			virtual ~I_VStateMaker() {}
			virtual HGLState make(const JValue& v) const = 0;
		};
		template <class Func>
		class VStateMaker;
		template <class... Ts>
		class VStateMaker<void (IGL::*)(Ts...)> :
			public I_VStateMaker
		{
			private:
				using Types = lubee::Types<Ts...>;
				template <int N>
				using TypesAt = typename GLArgCnv<typename Types::template At<N>>::type;

				void (IGL::*func)(Ts...);

				template <std::size_t... Idx>
				HGLState _makeWithArgs(const JValue& v, std::index_sequence<Idx...>) const {
					return MakeGL_VState(
						func,
						loader::FindLoader_t<TypesAt<Idx>>{
							v[rapidjson::SizeType(Idx)]
						}...
					);
				}
				template <
					std::size_t TsN = sizeof...(Ts),
					ENABLE_IF(TsN == 1)
				>
				HGLState _make(const JValue& v) const {
					const auto m = [&](auto& v){
						return MakeGL_VState(func, loader::FindLoader_t<TypesAt<0>>{v});
					};
					if(v.GetType() != rapidjson::Type::kArrayType)
						return m(v);
					return m(v[0]);
				}
				template <
					std::size_t TsN = sizeof...(Ts),
					ENABLE_IF(TsN != 1)
				>
				HGLState _make(const JValue& v) const {
					return _makeWithArgs(v, std::make_index_sequence<TsN>{});
				}
			public:
				VStateMaker(void (IGL::*f)(Ts...)):
					func(f)
				{}
				HGLState make(const JValue& v) const override {
					return _make(v);
				}
		};
		template <class Func>
		VStateMaker<Func> MakeVStateMaker(Func f) {
			return VStateMaker<Func>(f);
		}

		const GLenum c_bool[] = {
			GL_BLEND,
			GL_CULL_FACE,
			GL_DEPTH_TEST,
			GL_POLYGON_OFFSET_FILL,
			GL_SAMPLE_ALPHA_TO_COVERAGE,
			GL_SCISSOR_TEST
		};
		struct ValueType {
			std::string				name;
			const I_VStateMaker&	maker;

			bool operator == (const char* str) const noexcept {
				return name == str;
			}
		};
		const ValueType c_value[] = {
			{"blendColor",					MakeVStateMaker(&IGL::glBlendColor)},
			{"blendEquationSeparate",		MakeVStateMaker(&IGL::glBlendEquationSeparate)},
			{"blendFuncSeparate",			MakeVStateMaker(&IGL::glBlendFuncSeparate)},
			{"colorMask",					MakeVStateMaker(&IGL::glColorMask)},
			{"cullFace",					MakeVStateMaker(&IGL::glCullFace)},
			{"depthFunc",					MakeVStateMaker(&IGL::glDepthFunc)},
			{"depthMask",					MakeVStateMaker(&IGL::glDepthMask)},
			{"depthRange",					MakeVStateMaker(&IGL::glDepthRange)},
			{"frontFace",					MakeVStateMaker(&IGL::glFrontFace)},
			{"lineWidth",					MakeVStateMaker(&IGL::glLineWidth)},
			{"polygonOffset",				MakeVStateMaker(&IGL::glPolygonOffset)},
			{"scissor",						MakeVStateMaker(&IGL::glScissor)}
		};
		const I_VStateMaker& g_ccw = MakeVStateMaker(&IGL::glFrontFace);
		const I_VStateMaker& g_depthmask = MakeVStateMaker(&IGL::glDepthMask);
		const GLenum c_glsltype[] = {
			GL_BYTE,
			GL_UNSIGNED_BYTE,
			GL_SHORT,
			GL_UNSIGNED_SHORT,
			GL_UNSIGNED_INT,
			GL_FLOAT,
			GL_FLOAT_VEC2,
			GL_FLOAT_VEC3,
			GL_FLOAT_VEC4,
			GL_FLOAT_MAT2,
			GL_FLOAT_MAT3,
			GL_FLOAT_MAT4,
			GL_INT,
			GL_INT_VEC2,
			GL_INT_VEC3,
			GL_INT_VEC4,
			GL_BOOL,
			GL_BOOL_VEC2,
			GL_BOOL_VEC3,
			GL_BOOL_VEC4,
			GL_SAMPLER_2D,
		};
	}
	using namespace loader;
	// ---------------------------- Technique::ParamBase ----------------------------
	Technique::ParamBase::ParamBase(const JValue& v):
		type(Required<loader::GLEnum>(v, "type")),
		count(Optional<Integer>(v, "count", 1))
	{
		CheckEnum(c_glsltype, type);
	}

	// ---------------------------- Technique::AttrParam ----------------------------
	Technique::AttrParam::AttrParam(const JValue& v):
		ParamBase(v)
	{
		const char* sem = Required<String>(v, "semantic");
		semantic = *V_Semantic::FromString(sem);
	}

	// ---------------------------- Technique::UnifParam_Type ----------------------------
	Technique::UnifParam_Type::UnifParam_Type(const JValue& v):
		ParamBase(v)
	{}

	// ---------------------------- Technique::UnifParam_Fixed ----------------------------
	Technique::UnifParam_Fixed::UnifParam_Fixed(const JValue& v):
		ParamBase(v),
		value(LoadUniformValue(v["value"]))
	{}

	// ---------------------------- Technique::UnifParam_Sem ----------------------------
	Technique::UnifParam_Sem::UnifParam_Sem(const JValue& v) {
		const char* sem = Required<String>(v, "semantic");
		semantic = *U_Semantic::FromString(sem);
	}
	namespace {
		void ExportSemantic(ISemanticSet& s, const dc::JointId id, const IQueryMatrix_USem& qm, const USemantic semantic) {
			if(semantic == USemantic::Viewport)
				qm.exportViewport(s);
			else
				qm.exportSemantic(s, id, semantic);
		}
	}
	void Technique::UnifParam_Sem::exportUniform(ISemanticSet& s, const dc::JointId currentId, const SkinBindSet_SP&, const IQueryMatrix_USem& qm) const {
		ExportSemantic(s, currentId, qm, semantic);
	}

	// ---------------------------- Technique::UnifParam_JointMat ----------------------------
	Technique::UnifParam_JointMat::UnifParam_JointMat(const JValue& v):
		count(Required<Integer>(v, "count"))
	{}
	void Technique::UnifParam_JointMat::exportUniform(ISemanticSet& s, const dc::JointId currentId, const SkinBindSet_SP& bind, const IQueryMatrix_USem& qm) const {
		Assert0(bind && bind->bind.size() == count);
		s.set(qm.getJointMat(qm.getGlobal(currentId), bind), true);
	}

	// ---------------------------- Technique::UnifParam_NodeSem ----------------------------
	Technique::UnifParam_NodeSem::UnifParam_NodeSem(const JValue& v):
		node(Required<String>(v, "node"))
	{
		const char* sem = Required<String>(v, "semantic");
		semantic = *U_Semantic::FromString(sem);
		D_Assert0(semantic != USemantic::JointMatrix);
	}
	void Technique::UnifParam_NodeSem::resolve(const ITagQuery& q) {
		node.resolve(q);
	}
	void Technique::UnifParam_NodeSem::exportUniform(ISemanticSet& s, const dc::JointId, const SkinBindSet_SP&, const IQueryMatrix_USem& qm) const {
		ExportSemantic(s, node.data()->jointId, qm, semantic);
	}
	// ---------------------------- Technique::State ----------------------------
	Technique::State::State(const JValue& v) {
		{
			const auto bs = Optional<Array<loader::GLEnum>>(v, "enable", {});
			for(auto& b : bs) {
				const auto& idx = &CheckEnum(c_bool, b) - c_bool;
				state.emplace_back(std::make_shared<GL_BState>(true, c_bool[idx]));
			}
		}
		{
			const auto itr = v.FindMember("functions");
			if(itr != v.MemberEnd()) {
				const auto& funcs = itr->value;
				auto itr2 = funcs.MemberBegin();
				const auto itr2E = funcs.MemberEnd();
				while(itr2 != itr2E) {
					const auto& def = CheckEnum(c_value, itr2->name.GetString());
					state.emplace_back(def.maker.make(itr2->value));
					++itr2;
				}
			}
		}
	}

	namespace {
		template <class CB>
		void LoadParameter(CB&& cb, const JValue& v) {
			const auto itr = v.FindMember("semantic");
			if(itr != v.MemberEnd()) {
				const char* sem = itr->value.GetString();
				if(const auto vs = V_Semantic::FromString(sem)) {
					// Vertex Attribute
					return cb(std::make_unique<Technique::AttrParam>(v));
				}
				if(const auto us = U_Semantic::FromString(sem)) {
					if(us == USemantic::JointMatrix) {
						// JointMatrix
						return cb(std::make_unique<Technique::UnifParam_JointMat>(v));
					}
					if(v.HasMember("node")) {
						// NodeSem
						return cb(std::make_unique<Technique::UnifParam_NodeSem>(v));
					}
					// Sem
					return cb(std::make_unique<Technique::UnifParam_Sem>(v));
				}
			} else {
				if(v.HasMember("value")) {
					// Fixed
					return cb(std::make_unique<Technique::UnifParam_Fixed>(v));
				} else {
					// Type Only
					return cb(std::make_unique<Technique::UnifParam_Type>(v));
				}
			}
		}
	}
	// ---------------------------- Technique ----------------------------
	Technique::Technique(const JValue& v):
		Resource(v),
		state(Optional<State>(v, "states")),
		program(Required<String>(v, "program")),
		namecnv{
			Optional<Dictionary<StdString>>(v, "attributes", {}),
			Optional<Dictionary<StdString>>(v, "uniforms", {})
		}
	{
		const auto flipMap = [](auto& m) {
			std::decay_t<decltype(m)> tmp;
			for(auto& a : m)
				tmp[a.second] = a.first;
			std::swap(m, tmp);
		};
		flipMap(namecnv.attribute);
		flipMap(namecnv.uniform);
		param.rtUniform = std::make_shared<RTUParams>();
		const auto itr = v.FindMember("parameters");
		if(itr != v.MemberEnd()) {
			auto itr2 = itr->value.MemberBegin();
			const auto itr2E = itr->value.MemberEnd();
			while(itr2 != itr2E) {
				auto* srcName = itr2->name.GetString();
				LoadParameter(OVR_Functor{
					[this, srcName](std::unique_ptr<AttrParam> p){
						param.attribute.emplace_back(srcName, std::move(p));
					},
					[this, srcName](std::unique_ptr<RTUniform> p){
						param.rtUniform->emplace_back(
							namecnv.uniform.at(srcName),
							std::move(p)
						);
					},
					[this, srcName](std::unique_ptr<UnifParam_Fixed> p){
						param.fixedUniform.emplace_back(srcName, std::move(p));
					},
					[this, srcName](std::unique_ptr<UnifParam_Type> p){
						param.typedUniform.emplace_back(srcName, std::move(p));
					},
				}, itr2->value);
				++itr2;
			}
		}
	}
	Resource::Type Technique::getType() const noexcept {
		return Type::Technique;
	}
	void Technique::resolve(const ITagQuery& q) {
		for(auto& p : *param.rtUniform)
			p.second->resolve(q);
		program.resolve(q);
	}
}
