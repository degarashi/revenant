#pragma once
#include "lane.hpp"
#include "lubee/src/error.hpp"
#include "lubee/src/bit.hpp"

namespace rev {
	/*!
		CharPlane毎に1つ用意
		行ベースの空き領域管理
	*/
	template <size_t NLayer0, size_t NLayer1B, size_t MinSizeB>
	class LaneAlloc :
		public ILaneAlloc
	{
		private:
			using UInt = unsigned int;
			using FlagInt = uint32_t;
			constexpr static UInt
								NLayer1 = 1 << NLayer1B,
								MinSize = 1 << MinSizeB,
								L0Base = MinSize << 1,
								MaxBit = sizeof(FlagInt)*8-1;

			// sizebits = [Layer0 : Layer1 : MinSizeB]
			//! サイズごとのLane線形リスト先端
			Lane*		_lane[NLayer0][NLayer1] = {};
			FlagInt		_flagL0 = 0;
			FlagInt		_flagL1[NLayer0] = {};

			void _addFreeLane(Lane* lane) {
				D_Assert0(!lane->pNext);
				const auto [id0,id1] = _GetLayerID(lane->rect.width());
				// 先頭に追加
				lane->pNext = _lane[id0][id1];
				_lane[id0][id1] = lane;
				_setFlag(id0, id1);
			}
			Lane* _popLaneFront(const UInt nl0, const UInt nl1) {
				D_Assert0(nl0 < NLayer0
						&& nl1 < NLayer1);
				Lane *const p = _lane[nl0][nl1];
				Assert0(p);
				if(!(_lane[nl0][nl1] = p->pNext)) {
					_clearFlag(nl0, nl1);
				} else
					p->pNext = nullptr;
				return p;
			}
			void _setFlag(const UInt nl0, const UInt nl1) {
				D_Assert0(nl0 < NLayer0
						&& nl1 < NLayer1);
				_flagL1[nl0] |= 1<<nl1;
				_flagL0 |= 1<<nl0;
			}
			void _clearFlag(const UInt nl0, const UInt nl1) {
				D_Assert0(nl0 < NLayer0
						&& nl1 < NLayer1);
				_flagL1[nl0] &= ~(1<<nl1);
				if(_flagL1[nl0] == 0)
					_flagL0 &= ~(1<<nl0);
			}

			//! Laneから容量を切り分ける
			/*! 余りがMinSize未満だったらfreeBlkをnullにする */
			bool _dispenseFromLane(LaneRaw& dst, const UInt nl0, const UInt nl1, const std::size_t w) {
				D_Assert0(nl0 < NLayer0
						&& nl1 < NLayer1);
				Lane *const pl = _lane[nl0][nl1];
				if(!pl)
					return false;
				Assert0(pl->rect.width() >= w);

				dst.hTex = pl->hTex;
				dst.rect = lubee::RectI(pl->rect.x0, pl->rect.x0+w, pl->rect.y0, pl->rect.y1);
				// 使った容量分縮める
				pl->rect.shrinkLeft(w);
				if(pl->rect.width() < MinSize) {
					const auto* p2 = _popLaneFront(nl0, nl1);
					D_Assert0(p2 == pl);
					delete pl;
				} else {
					// 適したリストに格納
					const auto width = pl->rect.width();
					const auto [id0,id1] = _GetLayerID(width);
					if(nl0 != id0 || nl1 != id1) {
						const auto* p2 = _popLaneFront(nl0, nl1);
						D_Assert0(p2 == pl);
						_addFreeLane(pl);
					}
				}
				return true;
			}

			constexpr static UInt SizeL0(const UInt nl0) {
				return MinSize<<nl0;
			}
			constexpr static UInt SizeL1(const UInt nl0, const UInt nl1) {
				return SizeL0(nl0) + (SizeL0(nl0) / (1<<NLayer1)) * nl1;
			}
			static std::pair<UInt,UInt> _GetLayerID(const std::size_t num) {
				const int msb = lubee::bit::MSB(num);
				const UInt l0 = std::max(0, msb - int(MinSizeB) - int(NLayer1B));
				const UInt l1 = (num >> NLayer1B) & ((1<<NLayer1B)-1);
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
			void addFreeLane(const HTexMem2D& hTex, const lubee::RectI& rect) override {
				_addFreeLane(new Lane{hTex, rect});
			}
			/*! \return 容量不足で確保できない時はfalse */
			bool alloc(LaneRaw& dst, const size_t w) override {
				auto [id0,id1] = _GetLayerID(w);
				if(++id1 >= NLayer1) {
					++id0;
					id1 = 0;
				}
				if(_dispenseFromLane(dst, id0, id1, w)) {
					D_Assert0(dst.rect.width() == w);
					return true;
				}

				// Layer1から探す
				const FlagInt flag1 = _flagL1[id0] & ~((1 << id1) - 1);
				UInt lsb1 = lubee::bit::LSB(flag1);
				if(lsb1 == MaxBit) {
					// Layer1レベルでは空きが無いのでLayer0探索
					const FlagInt flag0 = _flagL0 & ~((1 << (id0+1)) - 1);
					const UInt lsb0 = lubee::bit::LSB(flag0);
					if(lsb0 == MaxBit) {
						// 空き容量不足
						return false;
					}
					Assert0(_flagL1[lsb0] != 0);
					lsb1 = lubee::bit::LSB(_flagL1[lsb0]);

					id0 = lsb0;
					id1 = lsb1;
				} else {
					id1 = lsb1;
				}
				const bool b = _dispenseFromLane(dst, id0, id1, w);
				Assert0(b);
				D_Assert0(dst.rect.width() == w);
				return true;
			}
	};
}
