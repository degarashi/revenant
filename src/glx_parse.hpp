#pragma once
#include "glx_macro.hpp"
#include "gl_types.hpp"
#include "vertex.hpp"
#include "frea/src/vector.hpp"
#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/container.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/optional.hpp>
#include <unordered_map>

namespace rev {
	namespace parse {
		struct Value {
			int	dim;
			enum Type {
				Bool,
				Int,
				Float,
				Other
			} type;
		};
		extern const Value Value_info[BOOST_PP_SEQ_SIZE(SEQ_GLTYPE)];

		using namespace boost::spirit;
		#define SEQ_BLOCK (attribute)(varying)(uniform)(const)
		#define PPFUNC_ADD(ign, data, elem) (BOOST_PP_STRINGIZE(elem), BOOST_PP_CAT(elem, data))
		#define PPFUNC_ENUM(ign, data, elem) BOOST_PP_CAT(elem, data),
		#define DEF_TYPE(typ, name, seq) struct typ##_ : x3::symbols<unsigned> { \
				enum TYPE { BOOST_PP_SEQ_FOR_EACH(PPFUNC_ENUM, T, seq) }; \
				const static char* cs_typeStr[BOOST_PP_SEQ_SIZE(seq)]; \
				typ##_(): x3::symbols<unsigned>(std::string(name)) { \
					add \
					BOOST_PP_SEQ_FOR_EACH(PPFUNC_ADD, T, seq); } }; \
				extern const typ##_ typ;
		#define DEF_VEC(typ, n, prefix) \
			using prefix##Vec##n = frea::Vec_t<typ, n, false>;
		DEF_VEC(int32_t, 1, I)
		DEF_VEC(int32_t, 2, I)
		DEF_VEC(int32_t, 3, I)
		DEF_VEC(int32_t, 4, I)
		DEF_VEC(float, 1, F)
		DEF_VEC(float, 2, F)
		DEF_VEC(float, 3, F)
		DEF_VEC(float, 4, F)
		DEF_VEC(bool, 1, B)
		DEF_VEC(bool, 2, B)
		DEF_VEC(bool, 3, B)
		DEF_VEC(bool, 4, B)
		#undef DEF_VEC

		using UniformVal = boost::variant<
			IVec1, IVec2, IVec3, IVec4,
			FVec1, FVec2, FVec3, FVec4,
			BVec1, BVec2, BVec3, BVec4
		>;
		// ---------------- GLXシンボルリスト ----------------
		//! GLSL変数型
		DEF_TYPE(GLType, "GLSL-ValueType", SEQ_GLTYPE)
		#define SEQ_INOUT (in)(out)(inout)
		//! GLSL変数入出力フラグ
		DEF_TYPE(GLInout, "InOut-Flag", SEQ_INOUT)
		#undef SEQ_INOUT
		//! GLSL頂点セマンティクス
		DEF_TYPE(GLSem, "VertexSemantics", SEQ_VSEM)
		//! GLSL浮動小数点数精度
		DEF_TYPE(GLPrecision, "PrecisionFlag", SEQ_PRECISION)

		//! boolでフラグを切り替える項目
		struct GLBoolsetting_ : x3::symbols<unsigned> {
			GLBoolsetting_(): x3::symbols<unsigned>(std::string("BooleanSettingName")) {
				add("cullface", GL_CULL_FACE)
					("polyoffsetfill", GL_POLYGON_OFFSET_FILL)
					("scissortest", GL_SCISSOR_TEST)
					("samplealphatocoverage", GL_SAMPLE_ALPHA_TO_COVERAGE)
					("samplecoverage", GL_SAMPLE_COVERAGE)
					("stenciltest", GL_STENCIL_TEST)
					("depthtest", GL_DEPTH_TEST)
					("blend", GL_BLEND)
					("dither", GL_DITHER);
			}
		};
		struct GLFillMode_ : x3::symbols<unsigned> {
			GLFillMode_(): x3::symbols<unsigned>(std::string("FillMode")) {
				add("point", GL_POINT)
					("line", GL_LINE)
					("fill", GL_FILL);
			}
		};
		#define PPFUNC_GLSET_ENUM(ign,data,elem) BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(0,elem), data),
		#define PPFUNC_GLSET_ADD(ign,data,elem) (BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(0,elem)), BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(0,elem),data))
		//! 数値指定するタイプの設定項目フラグ
		struct GLSetting_ : x3::symbols<unsigned> {
			enum TYPE : unsigned { BOOST_PP_SEQ_FOR_EACH(PPFUNC_GLSET_ENUM, T, SEQ_GLSETTING) };
			const static char* cs_typeStr[BOOST_PP_SEQ_SIZE(SEQ_GLSETTING)];
			GLSetting_(): x3::symbols<unsigned>(std::string("SettingName")) {
					add
					BOOST_PP_SEQ_FOR_EACH(PPFUNC_GLSET_ADD, T, SEQ_GLSETTING);
			}
		};
		#undef PPFUNC_GLSET_ENUM
		#undef PPFUNC_GLSET_ADD
		//! 設定項目毎に用意したほうがいい？
		struct GLStencilop_ : x3::symbols<unsigned> {
			GLStencilop_(): x3::symbols<unsigned>(std::string("StencilOperator")) {
				add("keep", GL_KEEP)
					("zero", GL_ZERO)
					("replace", GL_REPLACE)
					("increment", GL_INCR)
					("decrement", GL_DECR)
					("invert", GL_INVERT)
					("incrementwrap", GL_INCR_WRAP)
					("decrementwrap", GL_DECR_WRAP);
			}
		};
		struct GLFunc_ : x3::symbols<unsigned> {
			GLFunc_(): x3::symbols<unsigned>(std::string("CompareFunction")) {
				add("never", GL_NEVER)
					("always", GL_ALWAYS)
					("less", GL_LESS)
					("lessequal", GL_LEQUAL)
					("equal", GL_EQUAL)
					("greater", GL_GREATER)
					("greaterequal", GL_GEQUAL)
					("notequal", GL_NOTEQUAL);
			}
		};
		struct GLEq_ : x3::symbols<unsigned> {
			GLEq_(): x3::symbols<unsigned>(std::string("BlendEquation")) {
				add("add", GL_FUNC_ADD)
					("subtract", GL_FUNC_SUBTRACT)
					("invsubtract", GL_FUNC_REVERSE_SUBTRACT);
			}
		};
		struct GLBlend_ : x3::symbols<unsigned> {
			GLBlend_(): x3::symbols<unsigned>(std::string("BlendOperator")) {
				add("zero", GL_ZERO)
					("one", GL_ONE)
					("invsrccolor", GL_ONE_MINUS_SRC_COLOR)
					("invdstcolor", GL_ONE_MINUS_DST_COLOR)
					("srccolor", GL_SRC_COLOR)
					("dstcolor", GL_DST_COLOR)
					("invsrcalpha", GL_ONE_MINUS_SRC_ALPHA)
					("invdstalpha", GL_ONE_MINUS_DST_ALPHA)
					("srcalpha", GL_SRC_ALPHA)
					("dstalpha", GL_DST_ALPHA)
					("invconstantalpha", GL_ONE_MINUS_CONSTANT_ALPHA)
					("constantalpha", GL_CONSTANT_ALPHA)
					("srcalphasaturate", GL_SRC_ALPHA_SATURATE);
			}
		};
		struct GLFace_ : x3::symbols<unsigned> {
			GLFace_(): x3::symbols<unsigned>(std::string("Face(Front or Back)")) {
				add("front", GL_FRONT)
					("back", GL_BACK)
					("frontandback", GL_FRONT_AND_BACK);
			}
		};
		struct GLFacedir_ : x3::symbols<unsigned> {
			GLFacedir_(): x3::symbols<unsigned>(std::string("FaceDir")) {
				add("ccw", GL_CCW)
					("cw", GL_CW);
			}
		};
		struct GLColormask_ : x3::symbols<unsigned> {
			GLColormask_(): x3::symbols<unsigned>(std::string("ColorMask")) {
				add("r", 0x08)
					("g", 0x04)
					("b", 0x02)
					("a", 0x01);
			}
		};
		struct GLShadertype_ : x3::symbols<unsigned> {
			GLShadertype_(): x3::symbols<unsigned>(std::string("ShaderType")) {
				add("vertexshader", (unsigned)rev::ShType::Vertex)
					("fragmentshader", (unsigned)rev::ShType::Fragment)
					("geometryshader", (unsigned)rev::ShType::Geometry);
			}
		};
		//! 変数ブロックタイプ
		DEF_TYPE(GLBlocktype, "BlockType", SEQ_BLOCK)
		#undef DEF_TYPE
		#undef PPFUNC_ADD
		#undef PPFUNC_ENUM

		extern const GLSetting_ GLSetting;
		extern const GLBoolsetting_ GLBoolsetting;
		extern const GLFillMode_ GLFillMode;
		extern const GLStencilop_ GLStencilop;
		extern const GLFunc_ GLFunc;
		extern const GLEq_ GLEq;
		extern const GLBlend_ GLBlend;
		extern const GLFace_ GLFace;
		extern const GLFacedir_ GLFacedir;
		extern const GLColormask_ GLColormask;
		extern const GLShadertype_ GLShadertype;

		// ---------------- GLX構文解析データ ----------------
		struct EntryBase {
			boost::optional<unsigned>	prec;
			int							type;
			std::string					name;
		};
		std::ostream& operator << (std::ostream& os, const EntryBase& e);
		//! Attribute宣言エントリ
		struct AttrEntry : EntryBase {
			unsigned				sem;
			boost::optional<int>	index;
		};
		std::ostream& operator << (std::ostream& os, const AttrEntry& e);
		//! Varying宣言エントリ
		struct VaryEntry : EntryBase {
			boost::optional<int>	arraySize;
		};
		std::ostream& operator << (std::ostream& os, const VaryEntry& e);
		//! Uniform宣言エントリ
		struct UnifEntry : EntryBase {
			boost::optional<int>		arraySize;
			boost::optional<UniformVal>	defaultValue;
		};
		std::ostream& operator << (std::ostream& os, const UnifEntry& e);
		//! Const宣言エントリ
		struct ConstEntry : EntryBase {
			UniformVal			defVal;
		};
		std::ostream& operator << (std::ostream& os, const ConstEntry& e);
		//! Bool設定項目エントリ
		struct BoolSetting {
			GLuint				type;		//!< 設定項目ID
			bool				value;		//!< 設定値ID or 数値
		};
		std::ostream& operator << (std::ostream& os, const BoolSetting& s);
		//! 数値設定エントリ
		struct ValueSetting {
			using ValueT = boost::variant<boost::blank, unsigned int,float,bool>;

			unsigned				type;
			std::vector<ValueT>		value;
		};
		std::ostream& operator << (std::ostream& os, const ValueSetting& s);

		//! 変数ブロック使用宣言
		struct BlockUse {
			unsigned					type;	// Attr,Vary,Unif,Const
			bool						bAdd;
			std::vector<std::string>	name;
		};
		std::ostream& operator << (std::ostream& os, const BlockUse& b);
		//! シェーダー設定エントリ
		struct ShSetting {
			int							type;
			std::string					shName;
			// 引数指定
			std::vector<UniformVal>		args;
		};
		std::ostream& operator << (std::ostream& os, const ShSetting& s);
		//! マクロ宣言エントリ
		struct MacroEntry {
			std::string						fromStr;
			boost::optional<std::string>	toStr;
		};
		std::ostream& operator << (std::ostream& os, const MacroEntry& e);

		template <class Ent>
		struct Struct {
			std::string					name;
			std::vector<std::string>	derive;
			std::vector<Ent>			entry;

			Struct() = default;
			Struct(const Struct& a) = default;
			Struct(Struct&& a): Struct() { swap(a); }
			Struct& operator = (const Struct& a) {
				this->~Struct();
				new(this) Struct(a);
				return *this;
			}
			void swap(Struct& a) noexcept {
				std::swap(name, a.name);
				std::swap(derive, a.derive);
				std::swap(entry, a.entry);
			}
			void iterate(std::function<void (const Ent&)> cb) const {
				for(const auto& e : entry)
					cb(e);
			}
			void output(std::ostream& os) const {
				using std::endl;

				// print name
				os << '"' << name << '"' << endl;

				// print derives
				os << "derives: ";
				for(auto& d : derive)
					os << d << ", ";
				os << endl;

				// print entries
				for(auto& e : entry) {
					os << e << endl;
				}
			}
		};
		using AttrStruct = Struct<AttrEntry>;
		using VaryStruct = Struct<VaryEntry>;
		using UnifStruct = Struct<UnifEntry>;
		using ConstStruct = Struct<ConstEntry>;
		template <class T>
		std::ostream& operator << (std::ostream& os, const Struct<T>& s) {
			s.output(os);
			return os;
		}

		struct ArgItem {
			int			type;
			std::string	name;
		};
		using StrV = std::vector<std::string>;
		struct ShStruct {
			//! シェーダータイプ
			uint32_t				type;
			//! バージョン文字列
			std::string				version_str;
			//! OpenGL ESの際はtrue
			bool					bES;
			//! 組み込みコードブロック名
			StrV					code;
			//! シェーダー名
			std::string				name;
			//! 引数群(型ID + 名前)
			std::vector<ArgItem>	args;
			//! シェーダーの中身(文字列)
			std::string				info;
			mutable std::string		info_str;	//!< シェーダー文字列を1つに纏めた物

			std::string versionString() const;
			const std::string& getShaderString() const;
		};
		struct CodeStruct {
			//! コードブロック名
			std::string				name;
			//! シェーダーコード文字列
			std::string				info;
		};
		std::ostream& operator << (std::ostream& os, const CodeStruct& t);

		//! Tech,Pass兼用
		struct TPStruct;
		struct TPStruct {
			std::string					name;

			std::vector<BlockUse>		blkL;
			std::vector<BoolSetting> 	bsL;
			std::vector<MacroEntry>		mcL;
			std::vector<ShSetting>		shL;
			std::vector<ValueSetting> 	vsL;

			// -------------- Tech時のみ使用 --------------
			// 内包するPassのリスト
			std::vector<boost::recursive_wrapper<TPStruct>>		tpL;
			// 継承するTechのリスト
			StrV						derive;
		};
		std::ostream& operator << (std::ostream& os, const TPStruct& t);

		template <typename T>
		using NameMap = std::unordered_map<std::string, T>;
		//! エフェクト全般
		struct GLXStruct {
			NameMap<AttrStruct>			atM;
			NameMap<ConstStruct>		csM;
			NameMap<ShStruct>			shM;
			std::vector<TPStruct>		tpL;
			NameMap<UnifStruct>			uniM;
			NameMap<VaryStruct>			varM;
			NameMap<CodeStruct>			codeM;
			StrV						incl;		//!< インクルードファイル名
		};
		std::ostream& operator << (std::ostream& os, const GLXStruct& glx);

		void ParseGlx(GLXStruct& dst, std::string str);
	}
}

#define TRANSFORM_STRUCT_MEMBER(ign, name, member) (decltype(name::member), member)
#define FUSION_ADAPT_STRUCT_AUTO(name, members) \
			BOOST_FUSION_ADAPT_STRUCT(name, BOOST_PP_SEQ_FOR_EACH(TRANSFORM_STRUCT_MEMBER, name, members))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::AttrEntry, (prec)(type)(name)(sem)(index))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::VaryEntry, (prec)(type)(name)(arraySize))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::UnifEntry, (prec)(type)(name)(arraySize)(defaultValue))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::ConstEntry, (prec)(type)(name)(defVal))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::BoolSetting, (type)(value))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::ValueSetting, (type)(value))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::ShSetting, (type)(shName)(args))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::MacroEntry, (fromStr)(toStr))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::AttrStruct, (name)(derive)(entry))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::VaryStruct, (name)(derive)(entry))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::UnifStruct, (name)(derive)(entry))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::ConstStruct, (name)(derive)(entry))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::ShStruct, (type)(version_str)(bES)(name)(args)(info))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::TPStruct, (name)(blkL)(bsL)(mcL)(shL)(vsL)(tpL)(derive))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::GLXStruct, (atM)(csM)(shM)(tpL)(uniM)(varM)(incl))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::ArgItem, (type)(name))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::BlockUse, (type)(bAdd)(name))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::FVec1, (x))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::FVec2, (x)(y))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::FVec3, (x)(y)(z))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::FVec4, (x)(y)(z)(w))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::IVec1, (x))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::IVec2, (x)(y))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::IVec3, (x)(y)(z))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::IVec4, (x)(y)(z)(w))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::BVec1, (x))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::BVec2, (x)(y))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::BVec3, (x)(y)(z))
FUSION_ADAPT_STRUCT_AUTO(rev::parse::BVec4, (x)(y)(z)(w))
#undef FUSION_ADAPT_STRUCT_AUTO
#undef TRANSFORM_STRUCT_MEMBER
