#pragma once
#include "lubee/src/random/string.hpp"
#include "lubee/src/range.hpp"
#include "../fs/path.hpp"

namespace rev {
	namespace random {
		//! ランダムなパスセグメント列(utf-8)
		template <class RD>
		auto GenPathSegment(RD&& rd, const std::size_t len) {
			const static std::string c_segment("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-_ ");
			return lubee::random::GenString(rd, len, c_segment);
		}
		//! ランダムなパス(utf-8)
		template <class RD>
		auto GenPath(RD&& rd, const lubee::RangeI& nSeg, const lubee::RangeI& nLen, const char* abs=nullptr) {
			PathBlock pb;
			if(abs)
				pb.setPath(abs);
			const int nsegment = rd({nSeg.from, nSeg.to-1});
			for(int i=0 ; i<nsegment ; i++) {
				// ランダムsegment名
				pb.pushBack(
					GenPathSegment(
						rd,
						rd({nLen.from, nLen.to})
					)
				);
			}
			return pb;
		}
	}
}
