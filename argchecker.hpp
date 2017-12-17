#pragma once
#include "glx_parse.hpp"

namespace rev {
	//! 引数の型チェックと同時に出力
	struct ArgChecker : boost::static_visitor<> {
		DefineEnum(Target,
			(Boolean)
			(Scalar)
			(Vector)
			(None)
		);
		const static int N_Target = 4;
		Target _target[N_Target];
		const ArgItem* _arg[N_Target];
		const std::string& _shName;
		std::ostream& _ost;
		int _cursor = 0;

		ArgChecker(std::ostream& ost, const std::string& shName, const std::vector<ArgItem>& args);
		static Target Detect(int type);
		void _checkAndSet(Target tgt);
		void operator()(const std::vector<float>& v);
		void operator()(float v);
		void operator()(bool b);
		void finalizeCheck();
	};
}
