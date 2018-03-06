#pragma once
#include "font_common.hpp"
#include "lubee/error.hpp"
#include "lubee/bit.hpp"

namespace rev {
	/*!	Face毎に1つ用意
		空き領域の管理だけする。テクスチャの確保などは行わない */
	template <int NLayer0, int NLayer1B, int MinSizeB>
	class LaneAlloc : public ILaneAlloc {
		private:
			using FlagInt = uint32_t;
			constexpr static int NLayer1 = 1 << NLayer1B,
								MinSize = 1 << MinSizeB,
								L0Base = MinSize << 1,
								MaxBit = sizeof(FlagInt)*8-1;

			// sizebits = [Layer0 : Layer1 : MinSizeB]
			//! サイズごとのLane線形リスト先端
			Lane*		_lane[NLayer0][NLayer1] = {};
			FlagInt		_flagL0 = 0;
			FlagInt		_flagL1[NLayer0] = {};

			void _addFreeLane(Lane* lane) {
				auto id = _GetLayerID(lane->rect.width());
				// 先頭に追加
				lane->pNext = _lane[id.first][id.second];
				_lane[id.first][id.second] = lane;
				_setFlag(id.first, id.second);
			}
			Lane* _popLaneFront(const int nl0, const int nl1) {
				Lane* p = _lane[nl0][nl1];
				Assert0(p);
				if(!(_lane[nl0][nl1] = p->pNext))
					_clearFlag(nl0, nl1);
				return p;
			}
			void _setFlag(const int nl0, const int nl1) {
				_flagL1[nl0] |= 1<<nl1;
				_flagL0 |= 1<<nl0;
			}
			void _clearFlag(const int nl0, const int nl1) {
				_flagL1[nl0] &= ~(1<<nl1);
				if(_flagL1[nl0] == 0)
					_flagL0 &= ~(1<<nl0);
			}

			//! Laneから容量を切り分ける
			/*! 余りがMinSize未満だったらfreeBlkをnullにする */
			bool _dispenseFromLane(LaneRaw& dst, const int nl0, const int nl1, const std::size_t w) {
				Lane* pl = _lane[nl0][nl1];
				if(!pl)
					return false;
				Assert0(pl->rect.width() >= static_cast<int>(w));

				dst.hTex = pl->hTex;
				dst.rect = lubee::RectI(pl->rect.x0, pl->rect.x0+w, pl->rect.y0, pl->rect.y1);
				// 使った容量分縮める
				pl->rect.shrinkLeft(w);
				if(pl->rect.width() < MinSize) {
					_popLaneFront(nl0, nl1);
					delete pl;
				} else {
					// 適したリストに格納
					int width = pl->rect.width();
					auto id = _GetLayerID(width);
					if(nl0 != id.first || nl1 != id.second) {
						_popLaneFront(nl0, nl1);
						_addFreeLane(pl);
					}
				}
				return true;
			}

			constexpr static int SizeL0(const int nl0) {
				return MinSize<<nl0;
			}
			constexpr static int SizeL1(const int nl0, const int nl1) {
				return SizeL0(nl0) + (SizeL0(nl0) / (1<<NLayer1)) * nl1;
			}
			static std::pair<int,int> _GetLayerID(const std::size_t num) {
				int msb = lubee::bit::MSB(num);
				int l0 = std::max(0, msb - MinSizeB - NLayer1B);
				int l1 = (num >> NLayer1B) & ((1<<NLayer1B)-1);
				return std::make_pair(l0,l1);
			}

		public:
			~LaneAlloc() {
				clear();
			}
			void clear() override {
				for(auto& a0 : _lane) {
					for(auto& p : a0) {
						auto& p3 = p;
						while(p) {
							auto* p2 = p->pNext;
							delete p;
							p = p2;
						}
						p3 = nullptr;
					}
				}
			}
			void addFreeLane(const HTexMem& hTex, const lubee::RectI& rect) override {
				_addFreeLane(new Lane{hTex, rect});
			}
			/*! \return 容量不足で確保できない時はfalse */
			bool alloc(LaneRaw& dst, const std::size_t w) override {
				auto id = _GetLayerID(w);
				if(++id.second >= NLayer1) {
					++id.first;
					id.second = 0;
				}
				if(_dispenseFromLane(dst, id.first, id.second, w))
					return true;

				// Layer1から探す
				uint32_t flag1 = _flagL1[id.first] & ~((1 << id.second) - 1);
				uint32_t lsb1 = lubee::bit::LSB(flag1);
				if(lsb1 == MaxBit) {
					// Layer1レベルでは空きが無いのでLayer0探索
					uint32_t flag0 = _flagL0 & ~((1 << (id.first+1)) - 1);
					int lsb0 = lubee::bit::LSB(flag0);
					if(lsb0 == MaxBit) {
						// 空き容量不足
						return false;
					}
					Assert0(_flagL1[lsb0] != 0);
					lsb1 = lubee::bit::LSB(_flagL1[lsb0]);

					id.first = lsb0;
					id.second = lsb1;
				} else {
					id.second = lsb1;
				}
				const bool b = _dispenseFromLane(dst, id.first, id.second, w);
				Assert0(b);
				return true;
			}
	};
}
