#pragma once
#include "spine/rflag.hpp"
#include "drawtoken/tokenml.hpp"
#include "handle.hpp"
#include <unordered_map>
#include <unordered_set>

namespace rev {
	using Name = std::string;
	namespace draw {
		struct Token;
		using Token_SP = std::shared_ptr<Token>;
	}
	using UniformMap_t = std::unordered_map<Name, draw::Token_SP>;
	using UniformIdMap_t = std::unordered_map<int, draw::Token_SP>;
	class UniformEnt {
		private:
			struct Result_t {
				std::unordered_set<GLint>	idset;
				draw::TokenML				token;
			};
			#define SEQ \
				((Program)(HProg)) \
				((Entry)(UniformMap_t)) \
				((IdEntry)(UniformIdMap_t)) \
				((Result)(Result_t)(Program)(Entry)(IdEntry))
			RFLAG_DEFINE(UniformEnt, SEQ)
		public:
			RFLAG_GETMETHOD_DEFINE(SEQ)
			RFLAG_SETMETHOD_DEFINE(SEQ)
			RFLAG_REFMETHOD_DEFINE(SEQ)
			#undef SEQ
			void copyFrom(const UniformEnt& e);
			void clearValue();
	};
}
