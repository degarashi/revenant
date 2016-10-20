#pragma once
#include <cstdint>
#include <ctime>

namespace rev {
	#ifdef WIN32
		FILETIME UnixTime2WinFT(time_t t) noexcept;
		time_t WinFT2UnixTime(const FILETIME& ft) noexcept;
	#endif
	uint64_t UnixTime2WinTime(time_t t) noexcept;
	time_t WinTime2UnixTime(uint64_t ft) noexcept;
	uint64_t u32_u64(uint32_t valH, uint32_t valL) noexcept;

	//! ファイルアクセス時間比較用
	struct FTime {
		uint64_t	tmAccess,
					tmModify,
					tmCreated;

		bool operator == (const FTime& ft) const noexcept;
		bool operator != (const FTime& ft) const noexcept;
	};
	struct FStatus {
		enum Flag : uint32_t {
			UserRead = 0x100,
			UserWrite = 0x80,
			UserExec = 0x40,
			UserRWX = UserRead | UserWrite | UserExec,

			GroupRead = 0x20,
			GroupWrite = 0x10,
			GroupExec = 0x08,
			GroupRWX = GroupRead | GroupWrite | GroupExec,

			OtherRead = 0x04,
			OtherWrite = 0x02,
			OtherExec = 0x01,
			OtherRWX = OtherRead | OtherWrite | OtherExec,

			AllRead = UserRead | GroupRead | OtherRead,
			AllWrite = UserWrite | GroupWrite | OtherWrite,
			AllExec = UserExec | GroupExec | OtherExec,
			AllRW = AllRead | AllWrite,

			FileType = 0x200,
			DirectoryType = 0x400,
			NotAvailable = 0x800
		};

		uint32_t	flag;
		uint32_t	userId,
					groupId;
		uint64_t	size;
		FTime		ftime;

		FStatus() = default;
		FStatus(uint32_t flag) noexcept;
		bool operator == (const FStatus& fs) const noexcept;
		bool operator != (const FStatus& fs) const noexcept;
	};
}
