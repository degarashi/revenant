#include "glx_parse.hpp"

#define DEF_SETVAL(fn_name, name) const auto fn_name = [](auto& ctx){ _val(ctx).name = _attr(ctx); };
#define DEF_PUSHVAL(fn_name, name) const auto fn_name = [](auto& ctx){ _val(ctx).name.push_back(_attr(ctx)); };
#define DEF_INSVAL(fn_name, name, name_entry) const auto fn_name = [](auto& ctx){ _val(ctx).name.emplace(_attr(ctx).name_entry, _attr(ctx)); };

namespace rev {
	namespace parse {
		const GLType_ GLType;
		const GLInout_ GLInout;
		const GLSem_ GLSem;
		const GLPrecision_ GLPrecision;
		const GLBoolsetting_ GLBoolsetting;
		const GLFillMode_ GLFillMode;
		const GLSetting_ GLSetting;
		const GLStencilop_ GLStencilop;
		const GLFunc_ GLFunc;
		const GLEq_ GLEq;
		const GLBlend_ GLBlend;
		const GLFace_ GLFace;
		const GLFacedir_ GLFacedir;
		const GLColormask_ GLColormask;
		const GLShadertype_ GLShadertype;
		const GLBlocktype_ GLBlocktype;

		const Value Value_info[BOOST_PP_SEQ_SIZE(SEQ_GLTYPE)] = {
			{0, Value::Type::Other},
			{1, Value::Type::Bool},
			{1, Value::Type::Int},
			{1, Value::Type::Float},
			{2, Value::Type::Float},
			{3, Value::Type::Float},
			{4, Value::Type::Float},
			{2, Value::Type::Int},
			{3, Value::Type::Int},
			{4, Value::Type::Int},
			{2, Value::Type::Bool},
			{3, Value::Type::Bool},
			{4, Value::Type::Bool},
			{2, Value::Type::Other},
			{3, Value::Type::Other},
			{4, Value::Type::Other},
			{0, Value::Type::Other},
			{0, Value::Type::Other},
		};
		#define PPFUNC_STR(ign, data, elem) BOOST_PP_STRINGIZE(elem),
		const char* GLType_::cs_typeStr[] = {
			BOOST_PP_SEQ_FOR_EACH(PPFUNC_STR, EMPTY, SEQ_GLTYPE)
		};
		const char* GLSem_::cs_typeStr[] = {
			BOOST_PP_SEQ_FOR_EACH(PPFUNC_STR, EMPTY, SEQ_VSEM)
		};
		const char* GLPrecision_::cs_typeStr[] = {
			BOOST_PP_SEQ_FOR_EACH(PPFUNC_STR, EMPTY, SEQ_PRECISION)
		};
		const char* GLSetting_::cs_typeStr[] = {
			BOOST_PP_SEQ_FOR_EACH(PPFUNC_STR, EMPTY, SEQ_GLSETTING)
		};
		const char* GLBlocktype_::cs_typeStr[] = {
			BOOST_PP_SEQ_FOR_EACH(PPFUNC_STR, EMPTY, SEQ_BLOCK)
		};
		#undef PPFUNC_STR

		// GLX文法のEBNF記述
		namespace glx_rule {
			namespace x3 = boost::spirit::x3;
			const x3::rule<class Glx, GLXStruct>						Glx;
			const x3::rule<class String, std::string>					String;		//!< "で囲まれた文字列
			const x3::rule<class NameToken, std::string>				NameToken;	//!< 文字列トークン
			const x3::rule<class Bracket, std::string>					Bracket;	//!< 任意の括弧で囲まれたブロック
			const x3::rule<class FileToken, std::string>				FileToken;	//!< ファイル文字列

			// 変数宣言(attribute, uniform, varying)
			const x3::rule<class AttrEnt, AttrEntry> 					AttrEnt;
			const x3::rule<class VaryEnt, VaryEntry>					VaryEnt;
			const x3::rule<class UnifEnt, UnifEntry>					UnifEnt;
			const x3::rule<class ConstEnt, ConstEntry>					ConstEnt;
			const x3::rule<class MacroEnt, MacroEntry>					MacroEnt;

			const x3::rule<class BoolSet, BoolSetting>					BoolSet;
			const x3::rule<class ValueSet, ValueSetting>				ValueSet;
			const x3::rule<class BlockUse_t, BlockUse>					BlockUse;

			const x3::rule<class ShSet, ShSetting>						ShSet;

			// 各種定義ブロック
			const x3::rule<class AttrBlock, AttrStruct>					AttrBlock;
			const x3::rule<class VaryBlock, VaryStruct>					VaryBlock;
			const x3::rule<class UnifBlock, UnifStruct>					UnifBlock;
			const x3::rule<class ConstBlock, ConstStruct>				ConstBlock;
			const x3::rule<class ShBlock, ShStruct>						ShBlock;
			const x3::rule<class CodeBlock, CodeStruct>					CodeBlock;
			const x3::rule<class MacroBlock, std::vector<MacroEntry>>	MacroBlock;
			const x3::rule<class PassBlock, TPStruct>					PassBlock;
			const x3::rule<class TechBlock, TPStruct>					TechBlock;
			const x3::rule<class Import, std::string>					Import;

			using x3::int_;
			using x3::float_;
			using x3::bool_;
			using x3::uint_;
			using x3::repeat;
			using x3::lit;
			using x3::char_;
			using x3::lexeme;
			using x3::no_skip;
			using x3::no_case;
			using x3::alnum;
			const auto fnSetX = [](auto& ctx){ _val(ctx).x = _attr(ctx); };
			// 各種変数定義
			const x3::rule<class R_IVec1, frea::Vec_t<int32_t, 1, false>>	R_IVec1;
			const auto R_IVec1_def = ('[' >> int_[fnSetX] >> ']') | int_[fnSetX];
			const x3::rule<class R_IVec2, frea::Vec_t<int32_t, 2, false>>	R_IVec2;
			const auto R_IVec2_def = '[' >> int_ >> int_ >> ']';
			const x3::rule<class R_IVec3, frea::Vec_t<int32_t, 3, false>>	R_IVec3;
			const auto R_IVec3_def = '[' >> int_ >> int_ >> int_ >> ']';
			const x3::rule<class R_IVec4, frea::Vec_t<int32_t, 4, false>>	R_IVec4;
			const auto R_IVec4_def = '[' >> int_ >> int_ >> int_ >> int_ >> ']';

			const x3::rule<class R_FVec1, frea::Vec_t<float, 1, false>>	R_FVec1;
			const auto R_FVec1_def = ('[' >> float_[fnSetX] >> ']') | float_[fnSetX];
			const x3::rule<class R_FVec2, frea::Vec_t<float, 2, false>>	R_FVec2;
			const auto R_FVec2_def = '[' >> float_ >> float_ >> ']';
			const x3::rule<class R_FVec3, frea::Vec_t<float, 3, false>>	R_FVec3;
			const auto R_FVec3_def = '[' >> float_ >> float_ >> float_ >> ']';
			const x3::rule<class R_FVec4, frea::Vec_t<float, 4, false>>	R_FVec4;
			const auto R_FVec4_def = '[' >> float_ >> float_ >> float_ >> float_ >> ']';

			const x3::rule<class R_BVec1, frea::Vec_t<bool, 1, false>>	R_BVec1;
			const auto R_BVec1_def = ('[' >> bool_[fnSetX] >> ']') | bool_[fnSetX];
			const x3::rule<class R_BVec2, frea::Vec_t<bool, 2, false>>	R_BVec2;
			const auto R_BVec2_def = '[' >> bool_ >> bool_ >> ']';
			const x3::rule<class R_BVec3, frea::Vec_t<bool, 3, false>>	R_BVec3;
			const auto R_BVec3_def = '[' >> bool_ >> bool_ >> bool_ >> ']';
			const x3::rule<class R_BVec4, frea::Vec_t<bool, 4, false>>	R_BVec4;
			const auto R_BVec4_def = '[' >> bool_ >> bool_ >> bool_ >> bool_ >> ']';

			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wunused-parameter"
			BOOST_SPIRIT_DEFINE(
				R_IVec1, R_IVec2, R_IVec3, R_IVec4,
				R_FVec1, R_FVec2, R_FVec3, R_FVec4,
				R_BVec1, R_BVec2, R_BVec3, R_BVec4
			)
			#pragma GCC diagnostic pop
			const auto UniformVal =
				R_IVec1 | R_IVec2 | R_IVec3 | R_IVec4 |
				R_FVec1 | R_FVec2 | R_FVec3 | R_FVec4 |
				R_BVec1 | R_BVec2 | R_BVec3 | R_BVec4;

			const x3::rule<class Arg, ArgItem>							Arg;
			// Arg: GLType NameToken
			const auto Arg_def = GLType > NameToken;
			// FileToken: [:Alnum:_\.]+;
			const auto FileToken_def = lexeme[+(alnum | char_('_') | char_('.'))];
			// Bracket: (\.)[^\1]Bracket?[^\1](\1)
			const auto fnAddStr = [](auto& ctx){
				_val(ctx) += _attr(ctx);
			};
			const auto fnAddStrB= [](auto& ctx){
				_val(ctx) += "{";
				_val(ctx) += _attr(ctx);
				_val(ctx) += "}";
			};
			const auto Bracket_def = lit('{') >
										no_skip[*(char_ - lit('{') - lit('}'))][fnAddStr] >
										*(Bracket[fnAddStrB] > no_skip[*(char_ - lit('{') - lit('}'))][fnAddStr]) >
										lit('}');
			// CodeBlock: code NameToken \{[^\}]*\}
			DEF_SETVAL(fnSetName, name)
			DEF_SETVAL(fnSetInfo, info)
			const auto CodeBlock_def = no_case[lit("code")] > NameToken[fnSetName] > Bracket[fnSetInfo];
			// ShBlock: GLShadertype\([^\)]+\) NameToken \(Arg (, Arg)*\) \{[^\}]*\}
			DEF_SETVAL(fnSetType, type)
			DEF_SETVAL(fnSetVer, version_str)
			DEF_PUSHVAL(fnPushArg, args)
			DEF_PUSHVAL(fnPushCode, code)
			const auto fnSetES_False = [](auto& ctx){
				_val(ctx).bES = false;
			};
			const auto fnSetES_True = [](auto& ctx){
				_val(ctx).bES = true;
			};
			const auto ShBlock_def = x3::eps[fnSetES_False] >> no_case[GLShadertype][fnSetType] >
							'(' > NameToken[fnSetVer] > -(no_case[lit("es")][fnSetES_True]) > -(',' > NameToken[fnPushCode] % ',') > ')' >
							NameToken[fnSetName] > '(' >
							-(Arg[fnPushArg] > *(',' > Arg[fnPushArg])) > ')' > Bracket[fnSetInfo];
			// MacroBlock: macro \{ MacroEnt \}
			const auto MacroBlock_def = no_case[lit("macro")] > '{' > *MacroEnt > '}';
			// PassBlock: pass \{ (BlockUse | BoolSet | MacroBlock | ShSet | ValueSet)* \}
			DEF_PUSHVAL(fnPushBu, blkL)
			DEF_PUSHVAL(fnPushBs, bsL)
			DEF_SETVAL(fnSetMacro, mcL)
			DEF_PUSHVAL(fnPushSh, shL)
			DEF_PUSHVAL(fnPushVs, vsL)
			const auto PassBlock_def = lit("pass") > NameToken[fnSetName] > '{' >
					*(BlockUse[fnPushBu] | ValueSet[fnPushVs] | BoolSet[fnPushBs] |
					MacroBlock[fnSetMacro] | ShSet[fnPushSh]) > '}';
			// TechBlock: technique (: NameToken (, NameToken)*) \{ (PassBlock | BlockUse | BoolSet | MacroBlock | ShSet | ValueSet)* \}
			DEF_PUSHVAL(fnPushTp, tpL)
			DEF_SETVAL(fnSetDerive, derive)
			const auto TechBlock_def = lit("technique")
									> NameToken[fnSetName] > -(':' > (NameToken % ',')[fnSetDerive]) > '{' >
					*(PassBlock[fnPushTp] | BlockUse[fnPushBu] | ValueSet[fnPushVs] | BoolSet[fnPushBs] |
					MacroBlock[fnSetMacro] | ShSet[fnPushSh]) > '}';
			// GLX: (AttrBlock | ConstBlock | ShBlock | TechBlock | UnifBlock | VaryBlock | Import | CodeBlock)*
			DEF_PUSHVAL(fnPushImport, incl)
			DEF_INSVAL(fnInsAttr, atM, name)
			DEF_INSVAL(fnInsConst, csM, name)
			DEF_INSVAL(fnInsSh, shM, name)
			DEF_INSVAL(fnInsUnif, uniM, name)
			DEF_INSVAL(fnInsVary, varM, name)
			DEF_INSVAL(fnInsCode, codeM, name)
			const auto Glx_def = *(
				Import[fnPushImport] |
				AttrBlock[fnInsAttr] |
				ConstBlock[fnInsConst] |
				ShBlock[fnInsSh] |
				TechBlock[fnPushTp] |
				UnifBlock[fnInsUnif] |
				VaryBlock[fnInsVary] |
				CodeBlock[fnInsCode]
			);
			// String: "[^"]+"
			const auto String_def = lit('"') > +(char_ - '"') > '"';
			// NameToken: [:Alnum:_]+;
			const auto NameToken_def = lexeme[+(alnum | char_('_'))];
			// AttrEnt: GLPrecision? GLType NameToken : GLSem;
			const auto AttrEnt_def = (-(GLPrecision) >> GLType >> NameToken >> ':') > GLSem > -('_' > int_) > ';';
			// VaryEnt: GLPrecision? GLType NameToken;
			const auto VaryEnt_def = -(GLPrecision) >> GLType >> NameToken >> -('[' > int_ > ']') > ';';
			// UnifEntry: Precision GLPrecision? GLType ValueName NameToken ([SizeSem])? = UniformVal?;
			const auto UnifEnt_def = (-(GLPrecision) >> GLType) > NameToken >
						-('[' > int_ > ']') >
						-(lit('=') > UniformVal) > ';';
			// MacroEntry: NameToken(=NameToken)?;
			const auto MacroEnt_def = NameToken > -('=' > NameToken) > ';';
			// ConstEntry: GLPrecision? GLType = NameToken UniformVal;
			const auto ConstEnt_def = (-(GLPrecision) >> GLType >> NameToken >>
				lit('=')) > UniformVal > ';';

			// BoolSet: GLBoolsetting = Bool;
			const auto BoolSet_def = no_case[GLBoolsetting] > '=' > no_case[bool_] > ';';
			// ValueSet: GLSetting =	GLSetting=
			//							(
			//								(0xUint)|
			//								Int|
			//								Float|
			//								Bool|
			//								GLFunc|
			//								GLStencilop|
			//								GLEq|
			//								GLBlend|
			//								GLFace|
			//								GLFacedir|
			//								GLColormask
			//							){1,4};
			const auto VS_Range = repeat(1,4);
			const auto ValueSet_def = no_case[GLSetting] >> '=' >
				repeat(1,4)[
					(lit("0x") > uint_) |
					no_case[
						GLFunc |
						GLStencilop |
						GLEq |
						GLBlend |
						GLFace |
						GLFacedir |
						GLColormask |
						GLFillMode
					] |
					float_ |
					bool_
				] > ';';
			// BlockUse: GLBlocktype (= | +=) NameToken (, NameToken)*;
			DEF_SETVAL(Bu_fnSetName, name)
			const auto Bu_fnSetFalse = [](auto& ctx){ _val(ctx).bAdd = false; };
			const auto Bu_fnSetTrue = [](auto& ctx){ _val(ctx).bAdd = true; };
			const auto BlockUse_def = no_case[GLBlocktype][fnSetType]
										> (lit('=')[Bu_fnSetFalse] | lit("+=")[Bu_fnSetTrue])
										> (NameToken % ',')[Bu_fnSetName] > ';';
			// ShSet: GLShadertype = NameToken \(UniformVal? (, UniformVal)*\);
			DEF_SETVAL(Sh_fnSetName, shName)
			DEF_PUSHVAL(Sh_fnPushArgs, args)
			const auto ShSet_def = no_case[GLShadertype][fnSetType]
									> '=' > NameToken[Sh_fnSetName]
									> lit('(')
									> (-(UniformVal[Sh_fnPushArgs]) > *(lit(',') > UniformVal[Sh_fnPushArgs]))
									> lit(')') > lit(';');

			DEF_PUSHVAL(fnPushEntry, entry)
			// AttrBlock: attribute NameToken (: NameToken)? \{(AttrEnt*)\}
			const auto AttrBlock_def = lit("attribute") > NameToken[fnSetName] > -(':' > (NameToken % ',')[fnSetDerive]) >
								'{' > *AttrEnt[fnPushEntry] > '}';
			// VaryBlock: varying NameToken (: NameToken)? \{VaryEnt*\}
			const auto VaryBlock_def = lit("varying") > NameToken[fnSetName] > -(':' > (NameToken % ',')[fnSetDerive]) >
								'{' > *VaryEnt[fnPushEntry] > '}';
			// UnifBlock: uniform NameToken (: NameToken)? \{UnifEnt*\}
			const auto UnifBlock_def = lit("uniform") > NameToken[fnSetName] > -(':' > (NameToken % ',')[fnSetDerive]) >
								'{' > *UnifEnt[fnPushEntry] > '}';
			// ConstBlock: const NameToken (: NameToken)? \{ConstEnt*\}
			const auto ConstBlock_def = lit("const") > NameToken[fnSetName] > -(':' > (NameToken % ',')[fnSetDerive]) >
								'{' > *ConstEnt[fnPushEntry] > '}';
			// Import: import ImportFileName ;
			const auto fnSetStr = [](auto& ctx){ _val(ctx)=_attr(ctx); };
			const auto Import_def = lit("import") >
									(('"' > FileToken[fnSetStr] > '"') | FileToken[fnSetStr]) >
									';';
			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wunused-parameter"
			BOOST_SPIRIT_DEFINE(ShBlock,
								MacroBlock,
								PassBlock,
								TechBlock,
								CodeBlock,
								Glx);
			BOOST_SPIRIT_DEFINE(Arg,
								Bracket,
								String,
								NameToken,
								FileToken,
								AttrEnt,
								VaryEnt,
								UnifEnt,
								MacroEnt,
								ConstEnt,
								BoolSet,
								ValueSet,
								BlockUse,
								ShSet,
								AttrBlock,
								VaryBlock,
								UnifBlock,
								ConstBlock,
								Import);
			#pragma GCC diagnostic pop
		}
	}
}

#include <regex>
#include <ostream>
#include <boost/format.hpp>
namespace rev {
	namespace parse {
		namespace {
			std::regex re_comment(R"(//[^\n$]+)"),		//!< 一行コメント
						re_comment2(R"(/\*[^\*]*\*/)");		//!< 範囲コメント
		}
		void ParseGlx(GLXStruct& dst, std::string str) {
			// コメント部分を除去 -> スペースに置き換える
			str = std::regex_replace(str, re_comment, " ");
			str = std::regex_replace(str, re_comment2, " ");
			auto itr = str.cbegin();
			try {
				const bool bS = x3::phrase_parse(itr, str.cend(), glx_rule::Glx, x3::standard::space, dst);
				#ifdef DEBUG
					std::cout << ((bS) ? "------- analysis succeeded! -------"
										: "------- analysis failed! -------");
					if(itr != str.cend()) {
						std::cout << (boost::format("<but not reached to end>\nremains: %1%") % std::string(itr, str.cend())).str();
					} else {
						// 解析結果の表示
						std::stringstream ss;
						ss << dst;
						std::cout << ss.str();
					}
				#endif
				if(!bS || itr!=str.cend()) {
					std::stringstream ss;
					ss << "GLEffect parse error:";
					if(itr != str.cend())
						ss << "remains:\n" << std::string(itr, str.cend());
					throw std::runtime_error(ss.str());
				}
			} catch(const x3::expectation_failure<std::string::const_iterator>& e) {
				std::cout << ((boost::format("expectation_failure: \nat: %1%\nwhich: %2%\nwhat: %3%")
							% std::string(e.where(), str.cend())
							% e.which()
							% e.what()).str());
				throw;
			}
		}
		// ------------- ShStruct -------------
		const std::string& ShStruct::getShaderString() const {
			if(info_str.empty()) {
				info_str = '{' + info + '}';
			}
			return info_str;
		}
	}
}
