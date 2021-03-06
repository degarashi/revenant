#include "dir_depLinux.hpp"
#ifdef ANDROID
	#include <dirent.h>
	#include <errno.h>
#else
	#include <sys/dir.h>
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <memory>

namespace {
	class PError : public std::runtime_error {
		public:
			PError(const PError&) = default;
			PError(const std::string& name): std::runtime_error("") {
				perror(name.c_str());
				std::stringstream ss;
				ss << "posix-error at " << name << ' ' << errno << std::endl;
				(std::runtime_error&)(*this) = std::runtime_error(ss.str());
			}
	};
	const int c_buffSize = 2048;
	template <class... Ts>
	std::string MakeErrorMessage(const char* fmt, Ts&&... ts) noexcept {
		try {
			constexpr std::size_t BuffSize = 2048;
			char buff[BuffSize];
			std::snprintf(buff, BuffSize, fmt, std::forward<Ts>(ts)...);
			std::string ret("---<assertion failed!>---\nexpression: ");
			ret.append("\n");
			ret.append(buff);
			return ret;
		} catch(...) {
			// 何か例外が発生したら空のメッセージを返す
			return std::string();
		}
	}
}
namespace rev {
	std::string Dir_depLinux::Getcwd() {
		char tmp[c_buffSize];
		::getcwd(tmp, sizeof(tmp));
		return std::string(tmp);
	}
	void Dir_depLinux::Chdir(ToPathStr path) {
		if(::chdir(path.getStringPtr()) < 0)
			throw PError(MakeErrorMessage("chdir: %s", path.getStringPtr()));
	}
	bool Dir_depLinux::Chdir_nt(ToPathStr path) noexcept {
		return ::chdir(path.getStringPtr()) >= 0;
	}
	void Dir_depLinux::Mkdir(ToPathStr path, const uint32_t mode) {
		if(::mkdir(path.getStringPtr(), ConvertFlag_S2L(mode)) < 0)
			throw PError(MakeErrorMessage("mkdir: %s", path.getStringPtr()));
	}
	void Dir_depLinux::Chmod(ToPathStr path, const uint32_t mode) {
		if(::chmod(path.getStringPtr(), ConvertFlag_S2L(mode)) < 0)
			throw PError(MakeErrorMessage("chmod: %s", path.getStringPtr()));
	}
	void Dir_depLinux::Rmdir(ToPathStr path) {
		if(::rmdir(path.getStringPtr()) < 0)
			throw PError(MakeErrorMessage("rmdir: %s", path.getStringPtr()));
	}
	void Dir_depLinux::Remove(ToPathStr path) {
		if(::unlink(path.getStringPtr()) < 0)
			throw PError(MakeErrorMessage("remove: %s", path.getStringPtr()));
	}
	void Dir_depLinux::Move(ToPathStr from, ToPathStr to) {
		if(::rename(from.getStringPtr(), to.getStringPtr()) < 0)
			throw PError(MakeErrorMessage("move: %s -> %s", from.getStringPtr(), to.getStringPtr()));
	}
	void Dir_depLinux::Copy(ToPathStr from, ToPathStr to) {
		using Size = std::streamsize;
		std::ifstream ifs(from.getStringPtr(), std::ios::binary);
		std::ofstream ofs(to.getStringPtr(), std::ios::binary|std::ios::trunc);
		ifs.seekg(std::ios::end);
		Size sz = ifs.tellg();
		ifs.seekg(std::ios::beg);

		char buff[c_buffSize];
		for(;;) {
			Size nt = std::min(Size(sizeof(buff)), sz);
			ifs.read(buff, nt);
			ofs.write(buff, nt);
			if(nt != Size(sizeof(buff)))
				break;
		}
	}
	void Dir_depLinux::EnumEntry(ToPathStr path, EnumCBD cb) {
		struct DIR_Rel {
			void operator()(DIR* dir) const {
				::closedir(dir);
			}
		};
		PathStr tpath = path.moveTo();
		std::unique_ptr<DIR, DIR_Rel> dir(::opendir(tpath.c_str()));
		if(dir) {
			while(dirent* ent = ::readdir(dir.get())) {
				const std::size_t plen = tpath.size();
				tpath.append("/");
				tpath.append(ent->d_name);
				cb(ent->d_name, IsDirectory(tpath));
				tpath.resize(plen);
			}
		}
	}
	FStatus Dir_depLinux::Status(ToPathStr path) {
		struct stat st;
		if(::stat(path.getStringPtr(), &st) < 0)
			return FStatus(FStatus::NotAvailable);
		return CreateFStatus(st);
	}
	FTime Dir_depLinux::Filetime(ToPathStr path) {
		struct stat st;
		if(::stat(path.getStringPtr(), &st) < 0)
			throw PError(MakeErrorMessage("filetime: %s", path.getStringPtr()));
		FTime ft;
		ft.tmAccess = UnixTime2WinTime(st.st_atime);
		ft.tmCreated = UnixTime2WinTime(st.st_ctime);
		ft.tmModify = UnixTime2WinTime(st.st_mtime);
		return ft;
	}

	namespace {
		const std::pair<FStatus::Flag, uint32_t> c_f2s[] = {
			{FStatus::UserRead, S_IRUSR},
			{FStatus::UserWrite, S_IWUSR},
			{FStatus::UserExec, S_IXUSR},
			{FStatus::GroupRead, S_IRGRP},
			{FStatus::GroupWrite, S_IWGRP},
			{FStatus::GroupExec, S_IXGRP},
			{FStatus::OtherRead, S_IROTH},
			{FStatus::OtherWrite, S_IWOTH},
			{FStatus::OtherExec, S_IXOTH}
		};
	}
	FStatus Dir_depLinux::CreateFStatus(const struct stat& st) noexcept {
		FStatus fs;
		fs.flag = ConvertFlag_L2S(st.st_mode);
		fs.userId = st.st_uid;
		fs.groupId = st.st_gid;
		fs.size = st.st_size;
		fs.ftime.tmAccess = UnixTime2WinTime(st.st_atime);
		fs.ftime.tmModify = UnixTime2WinTime(st.st_mtime);
		fs.ftime.tmCreated = UnixTime2WinTime(st.st_ctime);
		return fs;
	}
	uint32_t Dir_depLinux::ConvertFlag_L2S(const uint32_t flag) noexcept {
		uint32_t res = 0;
		if(S_ISREG(flag))
			res |= FStatus::FileType;
		else if(S_ISDIR(flag))
			res |= FStatus::DirectoryType;

		for(auto& p : c_f2s) {
			if(p.second & flag)
				res |= p.first;
		}
		return res;
	}
	uint32_t Dir_depLinux::ConvertFlag_S2L(const uint32_t flag) noexcept {
		uint32_t res = 0;
		if(flag & FStatus::FileType)
			res |= S_IFREG;
		else if(flag & FStatus::DirectoryType)
			res |= S_IFDIR;

		for(auto& p : c_f2s) {
			if(p.first & flag)
				res |= p.second;
		}
		return res;
	}
	bool Dir_depLinux::IsFile(ToPathStr path) noexcept {
		struct stat st;
		return ::stat(path.getStringPtr(), &st) >= 0 && S_ISREG(st.st_mode);
	}
	bool Dir_depLinux::IsDirectory(ToPathStr path) noexcept {
		struct stat st;
		return ::stat(path.getStringPtr(), &st) >= 0 && S_ISDIR(st.st_mode);
	}
	PathStr Dir_depLinux::GetCurrentDir() {
		char buff[c_buffSize];
		buff[0] = '\0';
		if(!::getcwd(buff, sizeof(buff)))
			throw PError("GetCurrentDir");
		return PathStr(buff);
	}
	void Dir_depLinux::SetCurrentDir(const PathStr& path) {
		Chdir(path.c_str());
	}
	PathStr Dir_depLinux::GetProgramDir() {
		char buff[c_buffSize];
		buff[0] = '\0';
		const int n = ::readlink("/proc/self/exe", buff, sizeof(buff)-4);
		if(n == -1)
			throw PError("GetProgramDir");
		buff[n] = '\0';
		buff[n+1] = '\0';
		buff[n+2] = '\0';
		PathBlock pb(buff);
		pb.popBack();
		return PathStr(pb.plain_utf8());
	}
}
