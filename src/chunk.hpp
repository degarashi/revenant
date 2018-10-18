#pragma once
#include <cstdint>

namespace rev {
	//! 32bitの4文字チャンクを生成
	#define MAKECHUNK(c0,c1,c2,c3) ((c3<<24) | (c2<<16) | (c1<<8) | c0)
	//! 4つの8bit値から32bitチャンクを生成
	inline uint32_t MakeChunk(const uint8_t c0, const uint8_t c1, const uint8_t c2, const uint8_t c3) noexcept {
		return (c3 << 24) | (c2 << 16) | (c1 << 8) | c0;
	}
	//! 文字列の先頭4文字から32bitチャンクを生成
	inline uint32_t MakeChunk(const char* str) noexcept {
		return MakeChunk(str[0], str[1], str[2], str[3]);
	}
	//! 2つの32bit値を64bit値にパック
	inline uint64_t U64FromU32(const uint32_t hv, const uint32_t lv) noexcept {
		uint64_t ret(hv);
		ret <<= 32;
		ret |= lv;
		return ret;
	}
}
