#pragma once
#include "spine/src/singleton.hpp"
#include "spine/src/enum.hpp"
#include "lubee/src/rect.hpp"

namespace rev::info {
	#define rev_spec	(::rev::info::Spec::ref())
	//! 実行環境に関する情報を取得
	class Spec : public spi::Singleton<Spec> {
		public:
			using Name = std::string;
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
			struct Display {
				struct Mode {
					uint32_t		format;
					lubee::SizeI	size;
					int				rate;

					Mode(const void* m);
					bool operator == (const Mode& m) const noexcept;
					bool equals(uint32_t format, const lubee::SizeI &size) const noexcept;
				};
				using ModeV = std::vector<Mode>;
				struct DPI {
					float		diagonal,
								horizontal,
								vertical;

					DPI() = default;
					DPI(int index);
				};

				Name			name;
				ModeV			mode;
				int				currentModeIndex,
								desktopModeIndex;
				lubee::RectI	rect,
								usableRect;
				DPI				dpi;

				std::optional<int> findModeIndex(const Mode &m, bool ignoreRate) const noexcept;
				static int NDisplay() noexcept;
				static Display LoadInfo(int index);
			};
			using DisplayV = std::vector<Display>;
		private:
			Feature::value_t	 _feature;
			Name			_platform;
			int				_nCacheLine,
							_nCpu,
							_ramMB;
			DisplayV		_display;
		public:
			Spec() noexcept;
			const Name& getPlatform() const noexcept;

			int cpuCacheLineSize() const noexcept;
			int cpuCount() const noexcept;
			int systemRAM() const noexcept;
			bool hasFeature(PowerStatus::State flag) const noexcept;
			PowerStatus powerStatus() const noexcept;
			const DisplayV& display() const noexcept;
	};
}
