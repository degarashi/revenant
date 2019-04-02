#pragma once
#include "spine/src/singleton.hpp"
#include "spine/src/enum.hpp"

namespace rev::info {
	//! 実行環境に関する情報を取得
	class Spec : public spi::Singleton<Spec> {
		public:
			DefineEnumPair(
				Feature,
				((F_3DNow)(0x01))
				((F_AltiVec)(0x02))
				((F_MMX)(0x04))
				((F_RDTSC)(0x08))
				((F_SSE)(0x10))
				((F_SSE2)(0x11))
				((F_SSE3)(0x12))
				((F_SSE41)(0x14))
				((F_SSE42)(0x18))
			);
			struct PowerStatus {
				DefineEnum(
					State,
					(Unknown)
					(OnBattery)
					(NoBattery)
					(Charging)
					(Charged)
				);
				State	state;
				int		seconds,
						percentage;

				void output(std::ostream& os) const;
			};
		private:
			uint32_t		_feature;
			std::string		_platform;
			int				_nCacheLine,
							_nCpu;
		public:
			Spec() noexcept;
			const std::string& getPlatform() const noexcept;

			int cpuCacheLineSize() const noexcept;
			int cpuCount() const noexcept;
			bool hasFeature(PowerStatus::State flag) const noexcept;
			PowerStatus powerStatus() const noexcept;
	};
}
