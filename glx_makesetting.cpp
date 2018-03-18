#include "glx_makesetting.hpp"
#include "gl_vstate.hpp"
#include "gl_bstate.hpp"
#include "glx_parse.hpp"
#include <boost/format.hpp>

namespace rev {
	namespace {
		template <class To>
		struct Visitor : boost::static_visitor<To> {
			To operator()(boost::blank) const {
				D_Assert0(false);
				return {};
			}
			template <class T>
			To operator()(const T& t) const {
				return static_cast<To>(t);
			}
		};
		template <class To, class Value>
		To Convert(const Value& v) {
			return boost::apply_visitor(Visitor<To>(), v);
		}
		template <class... Args, class Value, std::size_t... Idx>
		HGLState Make(void (IGL::*func)(Args...), const Value& value, std::index_sequence<Idx...>) {
			// 引数の数が合わなかったらエラー
			const int nV = value.size();
			constexpr int NArgs = sizeof...(Args);
			if(NArgs != nV) {
				throw std::runtime_error(
					(
						boost::format("amount of argument(s) is not valid(func=%1%, required=%2%, actual=%3%)")
							% GLWrap::GetFunctionName(func)
							% NArgs
							% nV
					).str()
				);
			}
			return MakeGL_VState(func, Convert<Args>(value[Idx])...);
		}
		using Func = std::function<HGLState (const std::vector<parse::ValueSetting::ValueT>&)>;
		#define CONCAT_SCOPE(a,b)	a::b
		#define PPFUNC_MAKEFUNC(ign,data,elem) \
			[](const auto& value){ \
				return Make( \
					CONCAT_SCOPE( \
						&IGL, \
						BOOST_PP_TUPLE_ELEM(1,elem) \
					), \
					value, \
					std::make_index_sequence<BOOST_PP_SUB(BOOST_PP_TUPLE_SIZE(elem), 2)>{} \
				); \
			},
		const Func cs_make[] = {
			BOOST_PP_SEQ_FOR_EACH(PPFUNC_MAKEFUNC, EMPTY, SEQ_GLSETTING)
		};
	}
	HGLState MakeValueSetting(const parse::ValueSetting& s) {
		return cs_make[s.type](s.value);
	}
	HGLState MakeBoolSetting(const parse::BoolSetting& s) {
		return std::make_shared<GL_BState>(s.value, s.type);
	}
}
