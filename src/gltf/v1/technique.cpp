#include "gltf/v1/technique.hpp"
#include "../check.hpp"
#include "gltf/v1/dc_common.hpp"
#include "gltf/v1/node.hpp"
#include "../../gl/if.hpp"
#include "../../gl/bstate.hpp"
#include "../../gl/vstate.hpp"
#include "../../camera3d.hpp"
#include "../../ovr_functor.hpp"
#include "gltf/v1/qm_usem_if.hpp"
#include "semantic_if.hpp"

namespace rev::gltf::v1 {
	namespace {
		using gltf::loader::Required;
		using gltf::loader::GLEnum;
		using gltf::loader::Optional;
		using gltf::loader::OptionalDefault;
		using gltf::loader::FindLoader_t;
		using gltf::loader::Array;
		using gltf::loader::Integer;
		using gltf::loader::String;
		using gltf::loader::StdString;
		using gltf::loader::Dictionary;

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
						FindLoader_t<TypesAt<Idx>>{
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
						return MakeGL_VState(func, FindLoader_t<TypesAt<0>>{v});
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
	// ---------------------------- Technique::ParamBase ----------------------------
	Technique::ParamBase::ParamBase(const JValue& v):
		type(Required<GLEnum>(v, "type")),
		count(OptionalDefault<Integer>(v, "count", 1))
	{
		CheckEnum(c_glsltype, type);
	}

	// ---------------------------- Technique::AttrParam ----------------------------
	Technique::AttrParam::AttrParam(const JValue& v):
		ParamBase(v),
		semantic(*VSemanticFromString(Required<String>(v, "semantic")))
	{}

	// ---------------------------- Technique::UnifParam_Type ----------------------------
	Technique::UnifParam_Type::UnifParam_Type(const JValue& v):
		ParamBase(v)
	{}

	// ---------------------------- Technique::UnifParam_Fixed ----------------------------
	Technique::UnifParam_Fixed::UnifParam_Fixed(const JValue& v, const IDataQuery& q):
		ParamBase(v),
		value(LoadUniformValue(v["value"], q))
	{}

	// ---------------------------- Technique::UnifParam_Sem ----------------------------
	Technique::UnifParam_Sem::UnifParam_Sem(const JValue& v) {
		const char* sem = Required<String>(v, "semantic");
		semantic = *U_Semantic::FromString(sem);
	}
	void Technique::UnifParam_Sem::exportUniform(ISemanticSet& s, const dc::JointId currentId, const SkinBindSet_SP& bind, const IQueryMatrix_USem& qm) const {
		qm.exportSemantic(s, currentId, bind, semantic);
	}

	// ---------------------------- Technique::UnifParam_JointMat ----------------------------
	Technique::UnifParam_JointMat::UnifParam_JointMat(const JValue& v):
		count(Required<Integer>(v, "count"))
	{}
	void Technique::UnifParam_JointMat::exportUniform(ISemanticSet& s, const dc::JointId currentId, const SkinBindSet_SP& bind, const IQueryMatrix_USem& qm) const {
		Assert0(bind && bind->bind.size() == count);
		qm.exportSemantic(s, currentId, bind, USemantic::JointMatrix);
	}

	// ---------------------------- Technique::UnifParam_NodeSem ----------------------------
	Technique::UnifParam_NodeSem::UnifParam_NodeSem(const JValue& v, const IDataQuery& q):
		jointId(Required<DRef_Node>(v, "node", q)->jointId)
	{
		const char* sem = Required<String>(v, "semantic");
		semantic = *U_Semantic::FromString(sem);
		D_Assert0(semantic != USemantic::JointMatrix);
	}
	void Technique::UnifParam_NodeSem::exportUniform(ISemanticSet& s, const dc::JointId, const SkinBindSet_SP& bind, const IQueryMatrix_USem& qm) const {
		qm.exportSemantic(s, this->jointId, bind, semantic);
	}
	// ---------------------------- Technique::State ----------------------------
	Technique::State::State(const JValue& v) {
		{
			const auto bs = OptionalDefault<Array<GLEnum>>(v, "enable", {});
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
		void LoadParameter(CB&& cb, const JValue& v, const IDataQuery& q) {
			const auto itr = v.FindMember("semantic");
			if(itr != v.MemberEnd()) {
				const char* sem = itr->value.GetString();
				if(const auto vs = VSemanticFromString(sem)) {
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
						return cb(std::make_unique<Technique::UnifParam_NodeSem>(v, q));
					}
					// Sem
					return cb(std::make_unique<Technique::UnifParam_Sem>(v));
				}
			} else {
				if(v.HasMember("value")) {
					// Fixed
					return cb(std::make_unique<Technique::UnifParam_Fixed>(v, q));
				} else {
					// Type Only
					return cb(std::make_unique<Technique::UnifParam_Type>(v));
				}
			}
		}
	}
	// ---------------------------- Technique ----------------------------
	Technique::Technique(const JValue& v, const IDataQuery& q):
		Resource(v),
		state(Optional<State>(v, "states")),
		program(Required<DRef_Program>(v, "program", q)),
		namecnv{
			.attribute = OptionalDefault<Dictionary<StdString>>(v, "attributes", {}),
			.uniform = OptionalDefault<Dictionary<StdString>>(v, "uniforms", {})
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
				}, itr2->value, q);
				++itr2;
			}
		}
	}
	Resource::Type Technique::getType() const noexcept {
		return Type::Technique;
	}
}
