#include "dir_depWin.hpp"
#include "lubee/error.hpp"
#include "spine/optional.hpp"

#define AS_LPCWSTR_PATH(path)	reinterpret_cast<const wchar_t*>(path.getStringPtr())
#define AS_LPWSTR(path)			reinterpret_cast<wchar_t*>(path)
// ファイルのユーザーアクセス権限関係はとりあえず無視
namespace rev {
	WError::WError(const char* name):
		std::runtime_error("")
	{
		LPVOID lpMsg;
		DWORD errID = GetLastError();
		FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					nullptr,
					errID,
					LANG_USER_DEFAULT,
					reinterpret_cast<LPWSTR>(&lpMsg),
					128, nullptr);
		spn::PathStr pstr(reinterpret_cast<const char16_t*>(lpMsg));
		std::cout << Text::UTFConvertTo8(pstr) << std::endl;
		LocalFree(lpMsg);

		std::stringstream ss;
		ss << "winapi-error at " << name << std::endl;
		ss << "ErrorID: " << errID << std::endl;
		std::string msg = spn::Text::UTFConvertTo8(pstr);
		ss.write(msg.c_str(), msg.length());
		static_cast<std::runtime_error&>(*this) = std::runtime_error(ss.str());
	}

	PathStr Dir_depWin::Getcwd() {
		PathCh tmp[512];
		GetCurrentDirectoryW(sizeof(tmp)/sizeof(tmp[0]), AS_LPWSTR(tmp));
		return PathStr(tmp);
	}
	void Dir_depWin::Chdir(ToPathStr path) {
		if(!SetCurrentDirectoryW(AS_LPCWSTR_PATH(path)))
			throw WError("chdir");
	}
	bool Dir_depWin::Chdir_nt(ToPathStr path) noexcept {
		return SetCurrentDirectoryW(AS_LPCWSTR_PATH(path)) != FALSE;
	}
	void Dir_depWin::Mkdir(ToPathStr path, const uint32_t mode) {
		if(CreateDirectoryW(AS_LPCWSTR_PATH(path), nullptr) == 0)
			throw WError("mkdir");
	}
	void Dir_depWin::Chmod(ToPathStr /*path*/, const uint32_t /*mode*/) noexcept {
		// Windowsだとファイルにデフォルトで実行権限がついてるので何もしない
	}
	void Dir_depWin::Rmdir(ToPathStr path) {
		if(RemoveDirectoryW(AS_LPCWSTR_PATH(path)) == 0)
			throw WError("rmdir");
	}
	void Dir_depWin::Remove(ToPathStr path) {
		if(DeleteFileW(AS_LPCWSTR_PATH(path)) == 0)
			throw WError("remove");
	}
	void Dir_depWin::Move(ToPathStr from, ToPathStr to) {
		if(MoveFileW(AS_LPCWSTR_PATH(from), AS_LPCWSTR_PATH(to)) == 0)
			throw WError("move");
	}
	void Dir_depWin::Copy(ToPathStr from, ToPathStr to) {
		if(CopyFileW(AS_LPCWSTR_PATH(from), AS_LPCWSTR_PATH(to), TRUE) == 0)
			throw WError("copy");
	}
	void Dir_depWin::EnumEntry(ToPathStr path, EnumCBD cb) {
		WIN32_FIND_DATAW tfd;
		PathStr ps = path.moveTo();
		ps.append(u"/*");
		UPWinH fh(FindFirstFileW(reinterpret_cast<const wchar_t*>(ps.c_str()), &tfd));
		if(fh.get() == INVALID_HANDLE_VALUE)
			return;
		do {
			cb(reinterpret_cast<const char16_t*>(tfd.cFileName), tfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		} while(FindNextFileW(fh.get(), &tfd));
	}
	FStatus Dir_depWin::Status(ToPathStr path) {
		const uint32_t attr = GetFileAttributesW(AS_LPCWSTR_PATH(path));
		if(attr == INVALID_FILE_ATTRIBUTES)
			return FStatus(FStatus::NotAvailable);

		FStatus st;
		// Windowsの場合はユーザーIDやグループIDが無効 (=0)
		st.userId = 0;
		st.groupId = 0;
		UPWinH fh(CreateFileW(AS_LPCWSTR_PATH(path), GENERIC_READ, FILE_SHARE_READ,
								nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
		LARGE_INTEGER sz;
		GetFileSizeEx(fh.get(), &sz);
		st.size = sz.QuadPart;
		st.flag = ConvertFlag_W2S(attr);
		FILETIME tmC, tmA, tmW;
		GetFileTime(fh.get(), &tmC, &tmA, &tmW);

		st.ftime.tmCreated = WinFT2UnixTime(tmC);
		st.ftime.tmAccess = WinFT2UnixTime(tmA);
		st.ftime.tmModify = WinFT2UnixTime(tmW);
		return st;
	}
	uint32_t Dir_depWin::ConvertFlag_S2W(const uint32_t flag) noexcept {
		uint32_t res = 0;
		if(flag & FStatus::DirectoryType)
			res |= FILE_ATTRIBUTE_DIRECTORY;
		constexpr uint32_t AllRWE = (FStatus::AllExec|FStatus::AllWrite);
		if((flag & AllRWE) != AllRWE)
			res |= FILE_ATTRIBUTE_READONLY;
		else
			res = FILE_ATTRIBUTE_NORMAL;
		return res;
	}
	uint32_t Dir_depWin::ConvertFlag_W2S(const uint32_t flag) noexcept {
		// ユーザー書き込み権限を持っていなければReadOnlyとする
		uint32_t res = 0;
		if(flag == FILE_ATTRIBUTE_NORMAL)
			return FStatus::FileType;
		res |= (flag & (FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_READONLY)) ? FStatus::AllRead : (FStatus::AllRW|FStatus::AllExec);
		res |= (flag & FILE_ATTRIBUTE_DIRECTORY) ? FStatus::DirectoryType : FStatus::FileType;
		return res;
	}
	namespace {
		spi::Optional<bool> SingleCheck(const ToPathStr& path, const uint32_t flag) noexcept {
			const uint32_t attr = GetFileAttributesW(AS_LPCWSTR_PATH(path));
			if(attr == INVALID_FILE_ATTRIBUTES)
				return spi::none;
			return attr & flag;
		}
	}
	bool Dir_depWin::IsFile(ToPathStr path) noexcept {
		const auto res = SingleCheck(path, FILE_ATTRIBUTE_DIRECTORY);
		if(!res)
			return false;
		return !(*res);
	}
	bool Dir_depWin::IsDirectory(ToPathStr path) noexcept {
		const auto res = SingleCheck(path, FILE_ATTRIBUTE_DIRECTORY);
		return res && *res;
	}
	PathStr Dir_depWin::GetCurrentDir() {
		wchar_t buff[512];
		GetCurrentDirectoryW(sizeof(buff)/sizeof(buff[0]), buff);
		return PathStr(reinterpret_cast<const char16_t*>(buff));
	}
	void Dir_depWin::SetCurrentDir(const PathStr& path) {
		Chdir(path);
	}
	PathStr Dir_depWin::GetProgramDir() {
		PathCh buff[512];
		DWORD ret = GetModuleFileNameW(NULL, reinterpret_cast<wchar_t*>(buff), sizeof(buff)/sizeof(buff[0]));
		if(ret == 0)
			throw WError("GetProgramDir");
		PathBlock pb(buff);
		pb.popBack();
		return PathStr(To16Str(pb.plain_utf32()).getStringPtr());
	}
}

