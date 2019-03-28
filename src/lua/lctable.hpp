#pragma once
#include "lcvalue.hpp"
#include <unordered_map>

namespace rev {
	class LCTable : public std::unordered_map<LCValue, LCValue> {
		public:
			using base = std::unordered_map<LCValue, LCValue>;
			using base::base;

			// 文字列(const char*)とstd::stringの同一視
			// テーブルはポインタではなく中身を全て比較
			bool preciseCompare(const LCTable& tbl) const;
	};

	template <class... Args, int N, ENABLE_IF_I((N != sizeof...(Args)))>
	void LCValue::_TupleAsTable(LCTable_SP& tbl, const std::tuple<Args...>& t, IConst<N>) {
		tbl->emplace(N+1, std::get<N>(t));
		_TupleAsTable(tbl, t, IConst<N+1>());
	}
	template <class... Args>
	LCTable_SP LCValue::_TupleAsTable(const std::tuple<Args...>& t) {
		auto ret = std::make_shared<LCTable>();
		LCValue::_TupleAsTable(ret, t, IConst<0>());
		return ret;
	}
	template <class... Args>
	LCValue::LCValue(std::tuple<Args...>& t):
		LCValue(static_cast<const std::tuple<Args...>&>(t))
	{}
	template <class... Args>
	LCValue::LCValue(std::tuple<Args...>&& t):
		LCValue(static_cast<const std::tuple<Args...>&>(t))
	{}
	template <class... Args>
	LCValue::LCValue(const std::tuple<Args...>& t):
		LCVar(_TupleAsTable(t))
	{}
}
