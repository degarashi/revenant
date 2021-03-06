#pragma once
#include "gltf/v1/resource.hpp"
#include "gltf/v1/u_semantic.hpp"
#include "gltf/v1/v_semantic.hpp"
#include "gltf/v1/uniform_value.hpp"
#include "gltf/v1/rt_uniform.hpp"
#include "../../gl/format.hpp"
#include "../../handle/opengl.hpp"
#include "../../handle/camera.hpp"

namespace rev::gltf::v1 {
	struct Technique :
		Resource
	{
		struct ParamBase {
			GLenum			type;
			std::size_t		count;

			ParamBase(const JValue& v);
		};
		struct AttrParam : ParamBase {
			VSemantic		semantic;

			AttrParam(const JValue& v);
		};

		// デフォルト値(Materialでの上書き可)
		struct UnifParam_Fixed : ParamBase {
			UniformValue	value;
			UnifParam_Fixed(const JValue& v, const IDataQuery& q);
		};
		// 値型だけの指定(Materialでの上書き必須)
		struct UnifParam_Type : ParamBase {
			UnifParam_Type(const JValue& v);
		};
		// ------ 実行時に決まるUniform変数(Materialでの上書き不可) ------
		// カレントノード + Semanticのみの指定
		struct UnifParam_Sem : RTUniform {
			USemantic		semantic;
			// node == Current-node
			// type == USemantic-specific
			// count == 1

			UnifParam_Sem(const JValue& v);
			void exportUniform(ISemanticSet& s, dc::JointId currentId, const SkinBindSet_SP& bind, const IQueryMatrix_USem& qmu) const override;
			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
		};
		struct UnifParam_JointMat : RTUniform {
			// semantic == JOINTMATRIX
			std::size_t		count;

			UnifParam_JointMat(const JValue& v);
			void exportUniform(ISemanticSet& s, dc::JointId currentId, const SkinBindSet_SP& bind, const IQueryMatrix_USem& qmu) const override;
			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
		};
		// Semantic + 明示的なノード指定
		struct UnifParam_NodeSem : RTUniform {
			dc::JointId		jointId;
			USemantic		semantic;
			// type == MAT4
			// count == 1

			UnifParam_NodeSem(const JValue& v, const IDataQuery& q);
			void exportUniform(ISemanticSet& s, dc::JointId currentId, const SkinBindSet_SP& bind, const IQueryMatrix_USem& qmu) const override;
			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
		};

		// attributeやuniformとして設定するパラメータ
		// [ParameterName] -> [Parameter]
		struct {
			template <class T>
			using NVPArray = std::vector<std::pair<GLSLName, T>>;
			template <class T>
			using NVP_UP_Array = NVPArray<std::unique_ptr<T>>;

			using AParams = NVP_UP_Array<AttrParam>;
			using TypedUParams = NVP_UP_Array<UnifParam_Type>;
			using FixedUParams = NVP_UP_Array<UnifParam_Fixed>;

			AParams			attribute;
			// 型だけ指定
			TypedUParams	typedUniform;
			// Tech生成時に決まる定数
			FixedUParams	fixedUniform;
			// 実行時に決まる変数
			// [GLSL-name] -> [Parameter]
			RTUParams_SP	rtUniform;
		} param;

		// OpenGLステート設定
		struct State {
			using StateV = std::vector<HGLState>;
			StateV			state;

			State(const JValue& v);
		};
		using State_OP = spi::Optional<State>;
		State_OP	state;

		DRef_Program	program;

		// GLSLとの名前対応記述
		struct {
			using AttributeM = std::unordered_map<Name, Name>;
			using UniformM = AttributeM;

			// param -> shader-specific name
			AttributeM		attribute;
			UniformM		uniform;
		} namecnv;

		Technique(Technique&& t) = default;
		Technique& operator = (Technique&& t) = default;
		Technique(const JValue& v, const IDataQuery& q);
		Type getType() const noexcept override;
	};
}
