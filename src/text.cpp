#include "text.hpp"
#include "lubee/src/bit.hpp"
#include <cstring>

namespace rev {
	namespace text {
		bool sjis_isMBChar(const char c) noexcept {
			const uint32_t subset = c & 0xff;
			if((subset >= 0x81 && subset <= 0x9F) || (subset >= 0xe0))
				return true;
			return false;
		}
		int sjis_strlen(const char* str) noexcept {
			// 文字列バイト数
			const int nStr = std::strlen(str);
			int result = nStr;
			for(int i=0 ; i<nStr ; i++) {
				if(sjis_isMBChar(str[i])) {
					result --;
					i ++;
				}
			}
			return result;
		}
		std::pair<CodeType, int> GetEncodeType(const void* ptr) noexcept {
			auto bom = *(reinterpret_cast<const uint32_t*>(ptr));
			if(bom == 0xfffe0000)
				return std::make_pair(CodeType::UTF_32BE, 4);
			if(bom == 0x0000feff)
				return std::make_pair(CodeType::UTF_32LE, 4);
			if((bom&0x00ffffff) == 0x00bfbbef)
				return std::make_pair(CodeType::UTF_8, 3);
			bom &= 0xffff;
			if(bom == 0xfffe)
				return std::make_pair(CodeType::UTF_16BE, 2);
			if(bom == 0xfeff)
				return std::make_pair(CodeType::UTF_16LE, 2);
			return std::make_pair(CodeType::ANSI, 0);
		}
		uint32_t CharToHex(const uint32_t ch) noexcept {
			if(ch >= 'a')
				return (ch-'a') + 10;
			else if(ch >= 'A')
				return (ch-'A') + 10;
			else
				return ch-'0';
		}
		uint32_t StrToHex4(const char* src) noexcept {
			return (CharToHex(src[0])<<12) |
					(CharToHex(src[1])<<8) |
					(CharToHex(src[2])<<4) |
					CharToHex(src[3]);
		}
		uint32_t HexToChar(const uint32_t hex) noexcept {
			if(hex >= 10)
				return (hex-10) + 'a';
			return hex + '0';
		}

		namespace {
			const char c_base64[64+1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
			const char c_base64_num[128] = {
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 62,0, 0, 0, 63,
				52,53,54,55,56,57,58,59,60,61,0, 0, 0, 127,0,0,
				0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,11,12,13,14,
				15,16,17,18,19,20,21,22,23,24,25,0, 0, 0, 0, 0,
				0, 26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
				41,42,43,44,45,46,47,48,49,50,51,0, 0, 0, 0, 0
			};
		}
		std::string BinaryToBase64(const std::string& src) {
			std::string ret;
			ret.resize(src.size() * 2);
			const auto nw = BinaryToBase64(&ret[0], ret.size(), src.data(), src.size());
			ret.resize(nw);
			return ret;
		}
		int BinaryToBase64(char* dst, const std::size_t n_dst, const char* src, int n) {
			uint32_t buff=0;
			int nlen=0;

			int rcur = 0,
			wcur = 0;
			for(;n>0;--n) {
				const unsigned char c = src[rcur++];

				buff <<= 8;
				buff |= c;
				nlen += 8;

				while(nlen >= 6) {
					dst[wcur++] = c_base64[buff >> (nlen-6)];
					nlen -= 6;
					buff &= (1<<nlen)-1;
				}
			}

			if(nlen > 0) {
				buff <<= (6-nlen);
				dst[wcur++] = c_base64[buff];
			}

			if(wcur & 0x03) {
				int n = 4 - (wcur&0x03);
				for(int i=0 ; i<n ; i++)
					dst[wcur++] = '=';
			}

			if(wcur >= int(n_dst))
				throw std::length_error("base64(): buffer overflow");

			dst[wcur] = 0;
			return wcur;
		}
		std::string Base64ToBinary(const std::string& src) {
			std::string ret;
			ret.resize(src.size());
			const auto nw = Base64ToBinary(&ret[0], ret.size(), src.data(), src.size());
			ret.resize(nw);
			return ret;
		}
		int Base64ToBinary(char* dst, const std::size_t n_dst, const char* src, int n) {
			uint32_t buff=0;
			int nlen=0;
			int wcur=0;

			for(;;) {
				const unsigned char c = *src++;
				if(--n < 0 || c == '=')
					break;

				buff <<= 6;
				buff |= c_base64_num[c];
				nlen += 6;

				while(nlen >= 8) {
					nlen -= 8;
					dst[wcur++] = (char)(buff>>nlen);
					buff &= (1<<nlen)-1;
				}
			}
			if(wcur >= int(n_dst))
				throw std::length_error("base64toNum(): buffer overflow");
			return wcur;
		}
		namespace {
			bool IsURLChar_OAUTH(const char c) noexcept {
				return (c>='A' && c<='Z') || (c>='0' && c<='9') || (c>='a' && c<='z') ||
				c=='.' || c=='-' || c=='_' || c=='~';
			}
			char Get16Char_OAUTH(const int num) noexcept {
				if(num >= 10)
					return 'A'+num-10;
				return '0'+num;
			}

			bool IsURLChar(const char c) noexcept {
				return (c>='A' && c<='Z') || (c>='0' && c<='9') || (c>='a' && c<='z') ||
				c=='\'' || c=='.' || c=='-' || c=='*' || c==')' || c=='(' || c=='_';
			}
			char Get16Char(const int num) noexcept {
				if(num >= 10)
					return 'a'+num-10;
				return '0'+num;
			}
		}
		int url_encode_OAUTH(char* dst, const std::size_t n_dst, const char* src, int n) {
			int wcur = 0,
			rcur = 0;
			for(;;) {
				const unsigned char c = src[rcur++];
				if(--n < 0)
					break;
				else if(IsURLChar_OAUTH(c))
					dst[wcur++] = c;
				else {
					dst[wcur++] = '%';
					dst[wcur++] = Get16Char_OAUTH(c>>4);
					dst[wcur++] = Get16Char_OAUTH(c&0x0f);
				}
			}
			dst[wcur] = '\0';
			if(wcur >= int(n_dst))
				throw std::length_error("url_encode_OAUTH(): buffer overflow");
			return wcur;
		}
		int url_encode(char* dst, const std::size_t n_dst, const char* src, int n) {
			int wcur = 0,
			rcur = 0;
			for(;;) {
				const unsigned char c = src[rcur++];
				if(--n < 0)
					break;
				else if(IsURLChar(c))
					dst[wcur++] = c;
				else if(c == ' ')
					dst[wcur++] = '+';
				else {
					dst[wcur++] = '%';
					dst[wcur++] = Get16Char(c>>4);
					dst[wcur++] = Get16Char(c&0x0f);
				}
			}
			dst[wcur] = '\0';
			if(wcur >= int(n_dst))
				throw std::length_error("url_encode(): buffer overflow");
			return wcur;
		}
		bool utf16_isSurrogate(const char16_t c) noexcept {
			return (c & 0xdc00) == 0xd800;
		}
		bool utf16_isSpace(const char16_t c) noexcept {
			return (c==u' ') || (c==u'　');
		}
		bool utf16_isLF(const char16_t c) noexcept {
			return c==u'\n';
		}
		bool utf16_isPrivate(const char16_t c) noexcept {
			return c>=0xe000 && c<0xf900;
		}
		void WriteData(void* pDst, const char32_t val, const int n) NOEXCEPT_IF_RELEASE {
			switch(n) {
				case 1:
					*reinterpret_cast<uint8_t*>(pDst) = val&0xff; return;
				case 2:
					*reinterpret_cast<uint16_t*>(pDst) = val&0xffff; return;
				case 3:
					*reinterpret_cast<uint16_t*>(pDst) = val&0xffff;
					*reinterpret_cast<uint8_t*>(((uintptr_t)pDst)+2) = (val>>16)&0xff; return;
				case 4:
					*reinterpret_cast<uint32_t*>(pDst) = val; return;
				default:
					D_Assert0(false);
			}
		}
		Code UTF16To32(const char32_t src) noexcept {
			Code ret;
			ret.nwrite = 1;
			if((src & 0xdc00dc00) == 0xdc00d800) {
				// サロゲートペア
				const char32_t l_bit = src & 0x03ff0000,
				h_bit = (src & 0x000003ff) + 0x40;

				ret.code = (l_bit>>16)|(h_bit<<10);
				ret.nread = 2;
			} else {
				// 16bit数値
				ret.code = src&0xffff;
				ret.nread = 1;
			}
			return ret;
		}
		Code UTF32To16(const char32_t src) noexcept {
			Code ret;
			ret.nread = 1;
			if(src >= 0x10000) {
				// サロゲートペア
				const char32_t l_bit = (src & 0x3ff)<<16,
				h_bit = ((src>>10)&0x3ff) - 0x40;
				ret.code = h_bit|l_bit|0xdc00d800;
				ret.nwrite = 2;
			} else {
				// 16bit数値
				ret.code = src;
				ret.nwrite = 1;
			}
			return ret;
		}
		Code UTF8To32(const char32_t src) {
			Code ret;
			ret.nwrite = 1;
			if((src&0x80) == 0) {
				// 1バイト文字
				ret.code = src&0xff;
				ret.nread = 1;
			} else if((src&0xc0e0) == 0x80c0) {
				// 2バイト文字
				ret.code = ((src&0x1f)<<6) |
						((src&0x3f00)>>8);
				ret.nread = 2;
			} else if((src&0xc0c0f0) == 0x8080e0) {
				// 3バイト文字
				ret.code = ((src&0x0f)<<12) |
						((src&0x3f00)>>2) |
						((src&0x3f0000)>>16);
				ret.nread = 3;
			} else if((src&0xc0c0c0f8) == 0x808080f0) {
				// 4バイト文字
				ret.code = ((src&0x07)<<18) |
						((src&0x3f00)<<4) |
						((src&0x3f0000)>>10) |
						((src&0x3f000000)>>24);
				ret.nread = 4;
			} else
				throw InvalidUnicode("UTF8To32()");
			return ret;
		}
		Code UTF8To32_s(const char32_t src) {
			Code ret = UTF8To32(src);
			const char32_t mask[5] = {0, 0,0x1e00,0x0f2000,0x07300000};
			const char32_t ormask[5] = {0, 1,0,0,0};

			if((ret.code & mask[ret.nread]) | ormask[ret.nread]) {}
			else
				throw InvalidUnicode("UTF8To32_s()");
			return ret;
		}
		Code UTF32To8(const char32_t src) noexcept {
			Code ret;
			ret.nread = 1;
			if(src < 0x80) {
				// 1バイト文字
				ret.code = src;
				ret.nwrite = 1;
			} else if(src < 0x0400) {
				// 2バイト文字
				ret.code = ((0x80 | (src&0x3f))<<8) |
						(0xc0 | ((src&0x07c0)>>6));
						ret.nwrite = 2;
			} else if(src < 0x10000) {
				// 3バイト文字
				ret.code = ((0x80 | (src&0x3f))<<16) |
						((0x80 | ((src&0x0fc0)>>6))<<8) |
						(0xe0 | ((src&0xf000)>>12));
				ret.nwrite = 3;
			} else {
				// 4バイト文字
				ret.code = ((0x80 | (src&0x3f))<<24) |
						((0x80 | ((src&0x0fc0)>>6))<<16) |
						((0x80 | ((src&0x3f000)>>12))<<8) |
						(0xf0 | ((src&0x1c0000)>>18));
				ret.nwrite = 4;
			}
			return ret;
		}
		Code UTF8To16(const char32_t src) {
			const Code ret = UTF8To32(src),
						ret2 = UTF32To16(ret.code);
			return Code{ret2.code, ret.nread, ret2.nwrite};
		}
		// 変換後の数値, 読み取ったバイト数, 書き込んだバイト数
		Code UTF16To8(const char32_t src) noexcept {
			const Code ret = UTF16To32(src),
						ret2 = UTF32To8(ret.code);
			return Code{ret2.code, ret.nread, ret2.nwrite};
		}

		Length::Length(const std::size_t sz) noexcept:
			dataLen(sz),
			strLen(sz)
		{}
		Length::Length(const std::size_t dLen, const std::size_t sLen) noexcept:
			dataLen(dLen),
			strLen(sLen)
		{}

		Length GetLength(const char32_t* str) noexcept {
			const auto* ts = str;
			while(*ts != U'\0')
				++ts;
			return ts - str;
		}
		Length GetLength(const char32_t* /*str*/, const size_t len) noexcept {
			// 1ワード = 1文字と決まってるのでlenをそのまま返す
			return len;
		}
		namespace {
			constexpr int utf8invalid = 0xffff;
			const int c_utf8size[] = {1, utf8invalid, 2, 3, 4, 5, 6, utf8invalid};
		}
		Length GetLength(const char* str) noexcept {
			auto *cur0 = reinterpret_cast<const uint8_t*>(str),
			*cur = cur0;
			size_t count = 0;

			uint8_t c = *cur;
			while(c != '\0') {
				cur += c_utf8size[7 - lubee::bit::MSB(uint8_t(~c))];
				++count;
				c = *cur;
			}
			return Length(cur-cur0, count);
		}
		Length GetLength(const char* str, const size_t len) NOEXCEPT_IF_RELEASE {
			size_t count = 0;
			auto *cur0 = reinterpret_cast<const uint8_t*>(str),
			*cur = cur0,
			*curE = cur + len;
			while(cur < curE) {
				cur += c_utf8size[7 - lubee::bit::MSB(uint8_t(~(*cur)))];
				++count;
			}
			D_Assert0(cur == curE);
			return Length(len, count);
		}
		Length GetLength(const char16_t* str) noexcept {
			int cur = 0;
			size_t count = 0;
			char16_t c = str[cur];
			while(c != u'\0') {
				++cur;
				++count;
				if(text::utf16_isSurrogate(c))
					++cur;
				c = str[cur];
			}
			return Length(cur, count);
		}
		Length GetLength(const char16_t* str, const size_t len) NOEXCEPT_IF_RELEASE {
			auto *strE = str + len;
			size_t count = 0;
			while(str < strE) {
				if(text::utf16_isSurrogate(*str))
					++str;
				++str;
				++count;
			}
			D_Assert0(str == strE);
			return Length(len, count);
		}
	}
}
