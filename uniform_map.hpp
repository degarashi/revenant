#pragma once
#include "gl_header.hpp"
#include <unordered_map>

namespace rev {
	namespace draw {
		class TokenBuffer;
		class TokenML;
	}
	class UniformMap {
		private:
			//! [UniformId -> Token]
			using Map = std::unordered_map<GLint, draw::TokenBuffer*>;
			Map		_map;
		public:
			~UniformMap();
			// umに既にidが登録されてないかチェックし、されていればそれを、無ければpoolからメモリを確保し返す
			draw::TokenBuffer* makeTokenBuffer(GLint id);
			void copyFrom(const UniformMap& other);
			void moveTo(draw::TokenML& ml);
			void clear();
			bool empty() const noexcept;
	};
}
