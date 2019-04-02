#pragma once
#include <vector>
#include <cstdint>

namespace rev {
	using ByteBuff = std::vector<uint8_t>;
	//! 1bitのモノクロビットマップを8bitのグレースケールへ変換
	ByteBuff Convert1Bit_8Bit(const void* src, int width, int pitch, int nrow);
	//! 8bitのグレースケールを24bitのPackedBitmapへ変換
	ByteBuff Convert8Bit_Packed24Bit(const void* src, int width, int pitch, int nrow);
	//! 8bitのグレースケールを32bitのPackedBitmapへ変換
	ByteBuff Convert8Bit_Packed32Bit(const void* src, int width, int pitch, int nrow);
}
