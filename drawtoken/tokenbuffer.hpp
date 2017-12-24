#pragma once
#include "token.hpp"
#include <array>

namespace rev {
	namespace draw {
		// (全てのトークンサイズは128byte以下と仮定)
		constexpr int MaxTokenSize = 128;
		// トークン1つ分を格納
		class TokenBuffer : public TokenDst, public Token {
			private:
				using Data = std::array<uint8_t, MaxTokenSize>;
				Data		_data;
				intptr_t	_offset;
				bool		_bInit;
			public:
				TokenBuffer();
				TokenBuffer(TokenBuffer&& t);
				~TokenBuffer();
				Token* asToken();
				const Token* asToken() const;
				void* allocate_memory(std::size_t s, intptr_t ofs) override;
				void exec() override;
				void takeout(TokenDst& dst) override;
				void clone(TokenDst& dst) const override;
				std::size_t getSize() const override;
		};
	}
}
