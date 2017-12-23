#include "glx_parse.hpp"
#include "tuple_seq.hpp"
#include <boost/format.hpp>

namespace rev {
	void OutputCommentBlock(std::ostream& os, const std::string& msg) {
		const char* c_separate = "-------------";
		os << "// " << c_separate << ' ' << msg << ' ' << c_separate << std::endl;
	}
	namespace parse {
		namespace {
			const static char* c_shType[] = {
				"VertexShader",
				"GeometryShader",
				"FragmentShader"
			};
		}
		// (prec) type name の順に出力
		std::ostream& operator << (std::ostream& os, const EntryBase& e) {
			if(e.prec)
				os << GLPrecision_::cs_typeStr[*e.prec] << ' ';
			os << GLType_::cs_typeStr[e.type] << ' ';
			os.write(e.name.c_str(), e.name.length());
			return os;
		}

		std::ostream& operator << (std::ostream& os, const AttrEntry& e) {
			return os << "attribute " << static_cast<const EntryBase&>(e) << ';';
		}
		std::ostream& operator << (std::ostream& os, const VaryEntry& e) {
			os << "varying " << static_cast<const EntryBase&>(e);
			if(e.arraySize) {
				os << '[' << *e.arraySize << ']';
			}
			return os << ';';
		}
		// nameの代わりにsemanticを出力
		std::ostream& operator << (std::ostream& os, const UnifEntry& e) {
			os << "uniform " << static_cast<const EntryBase&>(e);
			if(e.arraySize) {
				os << '[' << *e.arraySize << ']';
			}
			return os << ';';
		}

		namespace {
			struct PrintVisitor : boost::static_visitor<> {
				std::ostream& _dst;
				PrintVisitor(std::ostream& dst): _dst(dst) {}

				template <class T>
				void operator()(const T& t) const {
					_dst << t;
				}
			};
		}
		std::ostream& operator << (std::ostream& os, const ConstEntry& e) {
			os << "const " << static_cast<const EntryBase&>(e) << '=';
			boost::apply_visitor(PrintVisitor(os), e.defVal);
			return os << ';';
		}

		std::ostream& operator << (std::ostream& os, const CodeStruct& s) {
			OutputCommentBlock(os, (boost::format("code block \"%1%\"") % s.name).str());
			os << s.info << std::endl;
			return os;
		}
		std::string ShStruct::versionString() const {
			std::string str(version_str);
			if(bES)
				str.append(" es");
			return str;
		}
		std::ostream& operator << (std::ostream& os, const ShStruct& s) {
			using std::endl;

			os << '"' << s.name << '"' << endl;
			os << "version: " << s.versionString() << endl;
			os << "type: " << c_shType[s.type] << endl;
			os << "codeblock: ";
			for(auto& c : s.code)
				os << c << ' ';
			os << endl;
			os << "args: ";
			for(auto& a : s.args)
				os << GLType_::cs_typeStr[a.type] << ' ' << a.name << ", ";
			return os << endl;
		}

		std::ostream& operator << (std::ostream& os, const BlockUse& b) {
			using std::endl;

			os << GLBlocktype_::cs_typeStr[b.type] << endl;
			os << "derives: ";
			for(auto& a : b.name)
				os << a << ", ";
			return os << endl;
		}
		std::ostream& operator << (std::ostream& os, const BoolSetting& s) {
			return os << s.type << ": value=" << s.value << std::endl;
		}
		std::ostream& operator << (std::ostream& os, const MacroEntry& e) {
			// Entry オンリーなら #define Entry
			// Entry=Value なら #define Entry Value とする
			os << "#define " << e.fromStr;
			if(e.toStr)
				os << ' ' << *e.toStr;
			return os << std::endl;
		}
		std::ostream& operator << (std::ostream& os, const ValueSetting& s) {
			os << GLSetting_::cs_typeStr[s.type] << "= ";
			for(auto& a : s.value)
				os << a << ' ';
			return os << std::endl;
		}

		#define SEQ_TPS_ (blkL, BlockUse)(bsL, BoolSetting)(mcL, MacroEntry)(shL, ShSetting)(vsL, ValueSetting)(tpL, Pass)
		#define SEQ_TPS MAKE_SEQ(2, SEQ_TPS_)
		#define SEQ_GLX_ (atM, Attribute)(csM, Const)(shM, Shader)(uniM, Uniform)(varM, Varying)(codeM, Code)
		#define SEQ_GLX MAKE_SEQ(2, SEQ_GLX_)
		#define PRINTIT(ign,data,elem) for(auto& a : data.BOOST_PP_TUPLE_ELEM(0,elem)) {PrintIt(os, BOOST_PP_STRINGIZE(BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(1,elem), _array)), a);}
		#define PRINTITM(ign,data,elem) for(auto& a : data.BOOST_PP_TUPLE_ELEM(0,elem)) {PrintIt(os, BOOST_PP_STRINGIZE(BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(1,elem), _map)), a.second);}
		namespace {
			template <class T>
			void PrintIt(std::ostream& os, const T& t) {
				os << t;
			}
			void PrintIt(std::ostream& os, const boost::recursive_wrapper<TPStruct>& tp) {
				os << tp.get();
			}
			template <class T>
			void PrintIt(std::ostream& os, const char* msg, const T& t) {
				os << msg << std::endl;
				PrintIt(os, t);
				os << "-------------" << std::endl;
			}
		}

		std::ostream& operator << (std::ostream& os, const ShSetting& s) {
			os << c_shType[s.type] << "= " << s.shName << std::endl << "args: ";
			for(auto& a : s.args) {
				boost::apply_visitor(PrintVisitor(os), a);
				os << ", ";
			}
			return os << std::endl;
		}
		std::ostream& operator << (std::ostream& os, const TPStruct& t) {
			BOOST_PP_SEQ_FOR_EACH(PRINTIT, t, SEQ_TPS)
			return os;
		}
		std::ostream& operator << (std::ostream& os, const GLXStruct& glx) {
			BOOST_PP_SEQ_FOR_EACH(PRINTITM, glx, SEQ_GLX)
			for(auto& a : glx.tpL)
				PrintIt(os, "Tech: ", a);
			return os;
		}
	}
}
