#pragma once
#include "parse.hpp"
#include "../gl/error.hpp"

namespace rev {
	//! 引数の型チェック
	class ArgChecker : public boost::static_visitor<> {
		private:
			using ArgItemV = std::vector<parse::ArgItem>;
			const ArgItemV&		_arg;
			const std::string&	_shName;
			int					_cursor;
			static bool _TypeCheck(const int from, const int to);
			static parse::Value::Type _Detect(float);
			static parse::Value::Type _Detect(int32_t);
			static parse::Value::Type _Detect(bool);

		public:
			ArgChecker(const std::string& shName, const ArgItemV& arg);
			template <class Vec>
			void operator()(const Vec& v) {
				// 引数が多すぎたらエラー
				if(_cursor >= int(_arg.size()))
					throw GLE_InvalidArgument(_shName, "too many arguments");

				const auto& arg = _arg[_cursor];
				const auto& info = parse::Value_info[arg.type];
				++_cursor;
				// 型が違っていたらエラー
				if(!_TypeCheck(_Detect(v[0]), info.type))
					throw GLE_InvalidArgument(_shName, "(none)");
				// Dimensionが違っていたらエラー
				if(info.dim != Vec::size)
					throw GLE_InvalidArgument(_shName, arg.name);
			}
			void finalizeCheck();
	};
}
