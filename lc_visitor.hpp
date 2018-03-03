#pragma once
#include "lcv.hpp"
#include "lctable.hpp"

namespace rev {
	namespace visitor {
		struct Compare {
			template <class T0, class T1>
			bool operator()(const T0&, const T1&) const {
				return false;
			}
			template <class T>
			bool operator()(const T& t0, const T& t1) const {
				return t0 == t1;
			}
		};
		struct CompareVisitor : boost::static_visitor<bool> {
			template <class T0, class T1>
			bool operator()(const T0& t0, const T1& t1) const {
				using D0 = StringTypeCnv_t<typename LCV<T0>::value_t>;
				using D1 = StringTypeCnv_t<typename LCV<T1>::value_t>;
				return Compare()(D0(t0), D1(t1));
			}
			template <class T>
			bool operator()(const T& t0, const T& t1) const {
				return t0 == t1;
			}
			template <class T>
			bool operator()(const std::weak_ptr<T>& t0, const std::weak_ptr<T>& t1) const {
				return t0.lock() == t1.lock();
			}
			bool operator()(const char* c0, const char* c1) const {
				return std::strcmp(c0, c1) == 0;
			}
			bool operator()(const std::string& s, const char* c) const {
				return s == std::string(c);
			}
			bool operator()(const char* c, const std::string& s) const {
				return std::string(c) == s;
			}
			bool operator()(const LCTable_SP& t0, const LCTable_SP& t1) const {
				return t0->preciseCompare(*t1);
			}
		};
	}
}
