#pragma once
#include <cstdint>
#include <string>
#include "lubee/error.hpp"

namespace rev {
	template <class T>
	class AbstString;
	using c8Str = AbstString<char>;
	using c16Str = AbstString<char16_t>;
	using c32Str = AbstString<char32_t>;

	struct InvalidUnicode : std::invalid_argument {
		using std::invalid_argument::invalid_argument;
	};
	namespace text {
		// エンコードタイプ
		enum class CodeType {
			UTF_8,
			UTF_16LE,
			UTF_16BE,
			UTF_32LE,
			UTF_32BE,
			ANSI
		};
		//! SJISコードか判定する
		bool sjis_isMBChar(char c) noexcept;
		//! SJISコードにおける文字数判定
		int sjis_strlen(const char* str) noexcept;
		//! エンコードタイプと判定に使った文字数を返す
		std::pair<CodeType, int> GetEncodeType(const void* ptr) noexcept;
		//! "ASCII文字の16進数値"を数値へ
		uint32_t CharToHex(uint32_t ch) noexcept;
		//! ASCII文字の4桁の16進数値を数値へ
		uint32_t StrToHex4(const char* src) noexcept;
		uint32_t HexToChar(uint32_t hex) noexcept;
		//! Base64変換
		int base64(char* dst, std::size_t n_dst, const char* src, int n);
		int base64toNum(char* dst, std::size_t n_dst, const char* src, int n);
		// URL変換
		int url_encode_OAUTH(char* dst, std::size_t n_dst, const char* src, int n);
		int url_encode(char* dst, std::size_t n_dst, const char* src, int n);
		// UTF-16関連
		bool utf16_isSurrogate(char16_t c) noexcept;
		bool utf16_isSpace(char16_t c) noexcept;
		bool utf16_isLF(char16_t c) noexcept;
		/*! privare領域なら1，サロゲート私用領域なら2を返す(予定) */
		bool utf16_isPrivate(char16_t c) noexcept;
		// UTF16 <-> UTF8 相互変換
		std::u16string UTFConvertTo16(c8Str src);
		std::u16string UTFConvertTo16(c32Str src);
		std::u32string UTFConvertTo32(c16Str src);
		std::u32string UTFConvertTo32(c8Str src);
		std::string UTFConvertTo8(c16Str src);
		std::string UTFConvertTo8(c32Str src);
		//! 32bitデータの内、(1〜4)までの任意のバイト数をpDstに書き込む
		void WriteData(void* pDst, char32_t val, int n) NOEXCEPT_IF_RELEASE;

		struct Code {
			char32_t	code;
			int			nread, nwrite;
		};
		// nread, nwriteはバイト数ではなく文字数を表す
		// UTF変換(主に内部用)
		Code UTF16To32(char32_t src) noexcept;
		Code UTF32To16(char32_t src) noexcept;
		Code UTF8To32(char32_t src);
		//! 不正なシーケンスを検出すると例外を発生させる
		Code UTF8To32_s(char32_t src);
		Code UTF32To8(char32_t src) noexcept;
		Code UTF8To16(char32_t src);
		Code UTF16To8(char32_t src) noexcept;

		struct Length {
			std::size_t	dataLen,	//!< 単純なバイト長
						strLen;		//!< 多バイト文字を考慮した文字列長
			Length() = default;
			Length(std::size_t sz) noexcept;
			Length(std::size_t dLen, std::size_t sLen) noexcept;
		};
		// ---- 文字列バイトの長さが不明な場合に文字数と一緒に計算 ----
		Length GetLength(const char* str) noexcept;
		Length GetLength(const char16_t* str) noexcept;
		Length GetLength(const char32_t* str) noexcept;
		// ---- 文字列バイトの長さが分かっている場合の処理 ----
		Length GetLength(const char* str, std::size_t len) NOEXCEPT_IF_RELEASE;
		Length GetLength(const char16_t* str, std::size_t len) NOEXCEPT_IF_RELEASE;
		Length GetLength(const char32_t* str, std::size_t len) noexcept;
	};
}
