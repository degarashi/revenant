#include "glx_parse.hpp"

#define DEF_SETVAL(fn_name, name) const auto fn_name = [](auto& ctx){ _val(ctx).name = _attr(ctx); };
#define DEF_PUSHVAL(fn_name, name) const auto fn_name = [](auto& ctx){ _val(ctx).name.push_back(_attr(ctx)); };
#define DEF_INSVAL(fn_name, name, name_entry) const auto fn_name = [](auto& ctx){ _val(ctx).name.emplace(_attr(ctx).name_entry, _attr(ctx)); };

namespace rev {
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

		// 各種変数定義
		const x3::rule<class Vec, std::vector<float>>				Vec;
		const x3::rule<class Arg, ArgItem>							Arg;

		using x3::lit;
		using x3::char_;
		using x3::lexeme;
		using x3::no_skip;
		using x3::no_case;
		using x3::alnum;
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
		const auto ShBlock_def = no_case[GLShadertype][fnSetType] >
						'(' > NameToken[fnSetVer] > -(',' > NameToken[fnPushCode] % ',') > ')' >
						NameToken[fnSetName] > '(' >
						-(Arg[fnPushArg] > *(',' > Arg[fnPushArg])) > ')' > Bracket[fnSetInfo];
		// MacroBlock: macro \{ MacroEnt \}
		const auto MacroBlock_def = no_case[lit("macro")] > '{' > *MacroEnt > '}';
		// PassBlock: pass \{ (BlockUse | BoolSet | MacroBlock | ShSet | ValueSet)* \}
		DEF_PUSHVAL(fnPushBu, blkL)
		DEF_PUSHVAL(fnPushBs, bsL)
		DEF_SETVAL(fnSetMacro, mcL)
		DEF_PUSHVAL(fnPushSh, shL)
		DEF_PUSHVAL(fnPushTp, tpL)
		DEF_PUSHVAL(fnPushVs, vsL)
		const auto PassBlock_def = lit("pass") > NameToken[fnSetName] > '{' >
				*(BlockUse[fnPushBu] | ValueSet[fnPushVs] | BoolSet[fnPushBs] |
				MacroBlock[fnSetMacro] | ShSet[fnPushSh]) > '}';
		// TechBlock: technique (: NameToken (, NameToken)*) \{ (PassBlock | BlockUse | BoolSet | MacroBlock | ShSet | ValueSet)* \}
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

		using x3::alnum;
		using x3::int_;
		using x3::float_;
		using x3::bool_;
		using x3::uint_;
		using x3::repeat;
		// String: "[^"]+"
		const auto String_def = lit('"') > +(char_ - '"') > '"';
		// NameToken: [:Alnum:_]+;
		const auto NameToken_def = lexeme[+(alnum | char_('_'))];
		// AttrEnt: GLPrecision? GLType NameToken : GLSem;
		const auto AttrEnt_def = (-(GLPrecision) >> GLType >> NameToken >> ':') > GLSem > ';';
		// VaryEnt: GLPrecision? GLType NameToken;
		const auto VaryEnt_def = -(GLPrecision) >> GLType >> NameToken >> -('[' > int_ > ']') > ';';
		// UnifEntry: Precision`GLPrecision` ValueName`rlNameToken` [SizeSem`rlNameToken`] = DefaultValue`rlVec|float|bool`
		const auto UnifEnt_def = (-(GLPrecision) >> GLType) > NameToken >
					-('[' > int_ > ']') >
					-(lit('=') > (Vec | float_ | bool_)) > ';';
		// Vector: [Float+]
		const auto Vec_def = '[' > +float_ > ']';
		// MacroEntry: NameToken(=NameToken)?;
		const auto MacroEnt_def = NameToken > -('=' > NameToken) > ';';
		// ConstEntry: GLPrecision? GLType = NameToken (Vector|Float|Bool);
		const auto ConstEnt_def = (-(GLPrecision) >> GLType >> NameToken >>
			lit('=')) > (Vec | float_ | bool_) > ';';

		// BoolSet: GLBoolsetting = Bool;
		const auto BoolSet_def = no_case[GLBoolsetting] > '=' > no_case[bool_] > ';';
		// ValueSet: GLSetting = (0xUint|GLFunc|GLStencilop|GLEq|GLBlend|GLFace|GLFacedir|GLColormask){1,4} | Float | Bool;
		const auto ValueSet_def = no_case[GLSetting] > '=' >
			repeat(1,4)[(lit("0x") > uint_) |
			no_case[GLFunc | GLStencilop | GLEq | GLBlend | GLFace | GLFacedir | GLColormask | GLFillMode]
			| float_ | bool_] > ';';
		// BlockUse: GLBlocktype (= | +=) NameToken (, NameToken)*;
		DEF_SETVAL(Bu_fnSetName, name)
		const auto Bu_fnSetFalse = [](auto& ctx){ _val(ctx).bAdd = false; };
		const auto Bu_fnSetTrue = [](auto& ctx){ _val(ctx).bAdd = true; };
		const auto BlockUse_def = no_case[GLBlocktype][fnSetType]
									> (lit('=')[Bu_fnSetFalse] | lit("+=")[Bu_fnSetTrue])
									> (NameToken % ',')[Bu_fnSetName] > ';';
		// ShSet: GLShadertype = NameToken \((Vector|Bool|Float)? (, Vector|Bool|Float)*\);
		DEF_SETVAL(Sh_fnSetName, shName)
		DEF_PUSHVAL(Sh_fnPushArgs, args)
		const auto ShSet_def = no_case[GLShadertype][fnSetType]
								> '=' > NameToken[Sh_fnSetName]
								> lit('(')
								> (-(Vec|bool_|float_)[Sh_fnPushArgs]
										> *(lit(',') > (Vec|bool_|float_)[Sh_fnPushArgs]))
								> lit(");");

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
							Vec,
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

#include <regex>
#include <iostream>
#include <boost/format.hpp>
namespace rev {
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
}
