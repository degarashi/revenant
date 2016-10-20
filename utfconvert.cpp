#include "abstbuffer.hpp"

namespace rev {
	namespace text {
		namespace {
			template <class DST, class SRC>
			std::basic_string<DST> ConvertToString(const std::vector<SRC>& data, const SRC* pCur) {
				auto* pSrcC = reinterpret_cast<const DST*>(&data[0]);
				return std::basic_string<DST>(pSrcC, reinterpret_cast<const DST*>(pCur) - pSrcC);
			}
			template <class DST, class SRC, class Conv>
			std::basic_string<DST> UTFConvert(const SRC* pSrc, const std::size_t len, const int ratio, Conv cnv) {
				std::vector<DST> ret(len*ratio);
				if(ret.empty())
					return std::basic_string<DST>();
				auto* pDst = &ret[0];
				auto* pSrcEnd = pSrc + len;
				while(pSrc != pSrcEnd) {
					Code ret = cnv(*reinterpret_cast<const char32_t*>(pSrc));
					pSrc += ret.nread;
					WriteData(pDst, ret.code, ret.nwrite * sizeof(decltype(*pDst)));
					pDst += ret.nwrite;
				}
				// 実際に使ったサイズに合わせる
				return ConvertToString<DST>(ret, pDst);
			}
		}
		std::string UTFConvertTo8(c16Str src) {
			return UTFConvert<char>(src.getPtr(), src.getLength(), 2, &UTF16To8);
		}
		std::string UTFConvertTo8(c32Str src) {
			return UTFConvert<char>(src.getPtr(), src.getLength(), 4, &UTF32To8);
		}
		std::u16string UTFConvertTo16(c8Str src) {
			return UTFConvert<char16_t>(src.getPtr(), src.getLength(), 1, &UTF8To16);
		}
		std::u16string UTFConvertTo16(c32Str src) {
			return UTFConvert<char16_t>(src.getPtr(), src.getLength(), 2, &UTF32To16);
		}
		std::u32string UTFConvertTo32(c16Str src) {
			return UTFConvert<char32_t>(src.getPtr(), src.getLength(), 2, &UTF16To32);
		}
		std::u32string UTFConvertTo32(c8Str src) {
			return UTFConvert<char32_t>(src.getPtr(), src.getLength(), 1, &UTF8To32);
		}
	}
	template class ToNStrT<char>;
	template class ToNStrT<char16_t>;
	template class ToNStrT<char32_t>;
}
