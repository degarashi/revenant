#include "time.hpp"
#include "lubee/arithmetic.hpp"

namespace rev {
	#ifdef WIN32
		FILETIME UnixTime2WinFT(const time_t t) noexcept {
			uint64_t wt = UnixTime2WinTime(t);
			FILETIME ft;
			ft.dwLowDateTime = wt & 0xffffffff;
			wt >>= 32;
			ft.dwHighDateTime = wt;
			return ft;
		}
		time_t WinFT2UnixTime(const FILETIME& ft) noexcept {
			uint64_t tmp = ft.dwHighDateTime;
			tmp <<= 32;
			tmp |= ft.dwLowDateTime;
			return WinTime2UnixTime(tmp);
		}
	#endif
	namespace {
		constexpr int UnixEpocYear = 1970,
					WinEpocYear = 1601;
		constexpr uint64_t Second = uint64_t(100 * 1000000),
							Minute = Second * 60,
							Hour = Minute * 60,
							Day = Hour * 24,
							Year = Day * 365;
	}
	uint64_t UnixTime2WinTime(const time_t t) noexcept {
		uint64_t tmp = t * Second;
		tmp -= (UnixEpocYear - WinEpocYear) * Year;
		return tmp;
	}
	time_t WinTime2UnixTime(uint64_t t) noexcept {
		t += (UnixEpocYear - WinEpocYear) * Year;
		t /= Second;
		return static_cast<time_t>(t);
	}
	uint64_t u32_u64(const uint32_t valH, const uint32_t valL) noexcept {
		uint64_t ret = valH;
		ret <<= 32;
		ret |= valL;
		return ret;
	}

	// ------------------- FStatus -------------------
	FStatus::FStatus(const uint32_t f) noexcept:
		flag(f)
	{}
	bool FStatus::operator == (const FStatus& f) const noexcept {
		return flag == f.flag &&
			userId == f.userId &&
			groupId == f.groupId &&
			size == f.size &&
			ftime == f.ftime;
	}
	bool FStatus::operator != (const FStatus& f) const noexcept {
		return !(*this == f);
	}
	// ------------------- FTime -------------------
	bool FTime::operator == (const FTime& ft) const noexcept {
		return lubee::X_OrArgs(tmCreated, ft.tmCreated,
								tmAccess, ft.tmAccess,
								tmModify, ft.tmModify) == 0;
	}
	bool FTime::operator != (const FTime& ft) const noexcept {
		return !(*this == ft);
	}
}
