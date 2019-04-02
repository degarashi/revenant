#include "convert.hpp"

namespace rev {
	ByteBuff Convert1Bit_8Bit(const void* src, const int width, const int pitch, const int nrow) {
		auto* pSrc = reinterpret_cast<const uint8_t*>(src);
		ByteBuff buff(width * nrow);
		auto* dst = buff.data();
		for(int i=0 ; i<nrow ; i++) {
			for(int j=0 ; j<width ; j++)
				*dst++ = ((int32_t(0) - (pSrc[j/8] & (1<<(7-(j%8)))))>>8) & 0xff;
			pSrc += pitch;
		}
		return buff;
	}
	namespace {
		template <int NB>
		ByteBuff ExpandBits(const void* src, const int width, const int pitch, const int nrow) {
			auto* pSrc = reinterpret_cast<const uint8_t*>(src);
			ByteBuff buff(width*nrow*NB);
			auto* dst = reinterpret_cast<uint8_t*>(buff.data());
			for(int i=0 ; i<nrow ; i++) {
				for(int j=0 ; j<width ; j++) {
					auto tmp = pSrc[j];
					for(int k=0 ; k<NB ; k++)
						*dst++ = tmp;
				}
				pSrc += pitch;
			}
			return buff;
		}
	}
	ByteBuff Convert8Bit_Packed24Bit(const void* src, const int width, const int pitch, const int nrow) {
		return ExpandBits<3>(src, width, pitch, nrow);
	}
	ByteBuff Convert8Bit_Packed32Bit(const void* src, const int width, const int pitch, const int nrow) {
		return ExpandBits<4>(src, width, pitch, nrow);
	}
}
