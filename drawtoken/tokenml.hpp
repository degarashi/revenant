#pragma once
#include "token.hpp"
#include <list>
#include <vector>

namespace rev {
	namespace draw {
		namespace detail {
			/*
				getDrawTokenの役割:
				描画時にしか必要ないAPI呼び出しを纏める
				ただしDrawThreadからはリソースハンドルの参照が出来ないのでOpenGLの番号をそのまま格納
			*/
			class TokenMemory {
				private:
					using Buffer = std::vector<uint8_t>;
					Buffer		_buffer;
					std::size_t	_used;

				public:
					TokenMemory(TokenMemory&& t) noexcept;
					TokenMemory(const TokenMemory&) = delete;
					TokenMemory(std::size_t s);
					~TokenMemory();
					void* getMemory(std::size_t s, intptr_t ofs);
					void exec();
					void clear() noexcept;

					TokenMemory& operator = (TokenMemory&& t) noexcept;
					TokenMemory& operator = (const TokenMemory&) = delete;
			};
		}
		//! 複数のTokenMemoryを統合(TokenMultiLane)
		class TokenML : public TokenDst {
			private:
				using TokenML_t = std::list<detail::TokenMemory>;
				TokenML_t	_buffer;
				std::size_t	_laneSize;

			public:
				TokenML(std::size_t s=4096);
				void setLaneSize(std::size_t s);
				template <class T, class... Ts>
				T* allocate(Ts&&... ts) {
					return new(allocate_memory(sizeof(T), CalcTokenOffset<T>())) T(std::forward<Ts>(ts)...);
				}
				void* allocate_memory(std::size_t s, intptr_t ofs) override;
				void exec();
				void clear();
				void append(TokenML&& ml);
		};
	}
}
