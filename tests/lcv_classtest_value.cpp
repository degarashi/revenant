#include "lcv_classtest.hpp"

namespace rev {
	namespace test {
		namespace code {
			const std::string lua_MemberReadXYZW =
				"return function(v, n)\n"
					"local member = {\"x\", \"y\", \"z\", \"w\"}\n"
					"local res = {}\n"
					"for i=1,n+1 do\n"
						"res[i] = v[member[i]]\n"
					"end\n"
					"return res\n"
				"end"
			;
			const std::string lua_MemberWriteXYZW =
				"return function(v, ...)\n"
					"local member = {\"x\", \"y\", \"z\", \"w\"}\n"
					"local arg = {...}\n"
					"for i=1,#arg do\n"
						"v[member[i]] = arg[i]\n"
					"end\n"
				"end"
			;
		}
	}
}
