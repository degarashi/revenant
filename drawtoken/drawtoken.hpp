#pragma once
#include <vector>
#include <cstdint>
#include <list>
#include <array>

namespace rev {
	namespace draw {
		struct TokenDst {
			virtual ~TokenDst() {}
			virtual void* allocate_memory(std::size_t s, intptr_t ofs) = 0;
		};
		struct Token {
			virtual ~Token() {}
			virtual void exec() = 0;
			virtual void takeout(TokenDst& dst) = 0;
			virtual void clone(TokenDst& dst) const = 0;
			virtual std::size_t getSize() const = 0;
		};
		template <class From, class To>
		intptr_t CalcPointerOffset() {
			return reinterpret_cast<intptr_t>(static_cast<To*>(reinterpret_cast<From*>(1))) - 1;
		}
		template <class T>
		intptr_t CalcTokenOffset() {
			return CalcPointerOffset<T, Token>();
		}
		template <class T>
		struct TokenT : Token {
			// 引数のバッファへ中身をcopyする
			void clone(TokenDst& dst) const override {
				new(dst.allocate_memory(getSize(), CalcTokenOffset<T>())) T(static_cast<const T&>(*this));
			}
			// 引数のバッファへ中身をmoveする
			void takeout(TokenDst& dst) override {
				new(dst.allocate_memory(getSize(), CalcTokenOffset<T>())) T(std::move(static_cast<T&>(*this)));
			}
			std::size_t getSize() const override {
				return sizeof(T);
			}
		};

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
