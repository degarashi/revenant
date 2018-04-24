#include "dir.hpp"
#include "ovr_functor.hpp"

namespace rev {
	// ----------------- Dir::PathReset -----------------
	Dir::PathReset::PathReset(): cwd(DirDep::Getcwd()) {}
	Dir::PathReset::~PathReset() {
		if(!std::uncaught_exception())
			DirDep::Chdir(cwd);
	}

	// ----------------- Dir -----------------
	const char Dir::SC('/'),
				Dir::DOT('.'),
				Dir::EOS('\0'),
				*Dir::SC_P(u8"/"),
				Dir::LBK('['),
				Dir::RBK(']');
	Dir::Dir(Dir&& d) noexcept:
		PathBlock(std::move(d))
	{}
	Dir& Dir::operator = (Dir&& d) noexcept {
		static_cast<PathBlock&>(*this) = std::move(static_cast<PathBlock&>(d));
		return *this;
	}
	std::string Dir::GetCurrentDir() {
		return To8Str(DirDep::GetCurrentDir()).moveTo();
	}
	std::string Dir::GetProgramDir() {
		return To8Str(DirDep::GetProgramDir()).moveTo();
	}
	void Dir::SetCurrentDir(const std::string& path) {
		PathStr ps;
		// Windows環境において先頭のドライブ文字を削る
		if(path.length() > 1 &&
			::isalpha(path[0]) && path[1] == ':')
		{
			ps = ToPathStr(path.substr(2)).moveTo();
		} else
			ps = ToPathStr(path).moveTo();
		DirDep::SetCurrentDir(ps);
	}

	Dir::SearchL Dir::_WildcardToSegment(const char* s) {
		SearchL ret;
		_ExtractSegment(s, [&ret](const char* s0, const char* s1){
			ret.emplace_back(_SegmentRead(s0, s1));
		});
		return ret;
	}
	namespace {
		namespace seg {
			const std::string Dir(R"([-_ .\w*])"),
								Star(Dir + "+"),
								Question(Dir);
		}
	}
	Dir::SearchSeg Dir::_SegmentRead(const char* s0, const char* s1) {
		if(s0 == s1) {
			return {std::string(), false};
		}
		std::string s;
		bool recursive = false,
			 regex = false;
		do {
			const auto c = *s0++;
			if(c == '*') {
				regex = true;
				s += seg::Star;
				if(s0 != s1 && *s0 == '*') {
					++s0;
					recursive = true;
				}
			} else if(c == '?') {
				regex = true;
				s += seg::Question;
			} else
				s += c;
		} while(s0 != s1);

		if(regex)
			return {std::regex(s), recursive};
		return {s, false};
	}
	void Dir::_ExtractSegment(const char* s, const SegCB& cb) {
		const char* s0 = s;
		for(;;) {
			const auto c = *s;
			if(c == '\0')
				break;
			else if(c == SC) {
				cb(s0, s);
				s0 = ++s;
			}
			++s;
		}
		if(s0 != s)
			cb(s0, s);
	}
	std::string Dir::setCurrentDir() const {
		std::string prev = GetCurrentDir();
		SetCurrentDir(plain_utf8());
		return prev;
	}
	void Dir::_EnumEntry(SearchItr itr, const SearchItr itrE, std::string& lpath, const EnumCB& cb) {
		if(itr == itrE)
			return;

		const size_t pl = lpath.size();
		DirDep::EnumEntry(lpath, [=, &lpath, &cb](const PathCh* name, const bool dir_flag) {
			// . と .. は無視する
			if(name[0]==PathCh(DOT)) {
				if(name[1]==PathCh(EOS) || name[1]==PathCh(DOT))
					return;
			}
			const std::string seg(To8Str(name).moveTo());
			const auto pushSeg = [&lpath, &seg](){
				if(lpath.back() != SC)
					lpath += SC;
				lpath += seg;
			};
			const auto popSeg = [&lpath, pl](){
				lpath.resize(pl);
			};
			const auto procLower = [&](){
				// ファイルに付き当たった場合は終了
				if(!dir_flag && itr+1 != itrE)
					return;

				pushSeg();
				if(dir_flag) {
					if(itr+1 != itrE) {
						_EnumEntry(itr+1, itrE, lpath, cb);
						if(itr->recursive)
							_EnumEntry(itr, itrE, lpath, cb);
					} else
						cb(Dir(lpath));
				} else if(itr+1 == itrE)
					cb(Dir(lpath));
				popSeg();
			};
			boost::apply_visitor(OVR_Functor{
				[&](boost::blank){},
				[&](const std::string& s){
					if(s == seg) {
						procLower();
					}
				},
				[&](const std::regex& r){
					std::smatch m;
					if(std::regex_match(seg, m, r)) {
						procLower();
					}
				}
			}, itr->var);
		});
	}
	bool Dir::isFile() const noexcept {
		return DirDep::IsFile(plain_utf32());
	}
	bool Dir::isDirectory() const noexcept {
		return DirDep::IsDirectory(plain_utf32());
	}
	void Dir::remove() const {
		DirDep::Remove(plain_utf32());
	}
	void Dir::copy(const std::string& to) const {
		DirDep::Copy(plain_utf32(), to);
	}
	void Dir::move(const std::string& to) const {
		DirDep::Move(plain_utf32(), to);
	}
	void Dir::mkdir(uint32_t mode) const {
		PathReset preset;
		if(isAbsolute())
			DirDep::Chdir(SC_P);
		mode |= FStatus::UserRWX;

		const int nsg = segments();
		std::string ns;
		int i;
		// 最初のパスから1つずつ存在確認
		for(i=0 ; i<nsg ; i++) {
			ns = getSegment_utf8(i,i+1);
			if(!DirDep::Chdir_nt(ns))
				break;
		}
		if(i == nsg)
			return;

		// パスがファイルだったら失敗とする
		Assert(!DirDep::IsFile(ns), "there is file at the path");
		for(;;) {
			DirDep::Mkdir(ns, mode);
			DirDep::Chdir(ns);
			if(++i == nsg)
				break;
			ns = getSegment_utf8(i,i+1);
		}
	}
	void Dir::_chmod(PathBlock& lpath, const ModCB& cb) {
		DirDep::EnumEntry(lpath.plain_utf32(), [&lpath, this, cb](const PathCh* name, bool) {
			lpath <<= name;
			if(ChMod(lpath, cb))
				_chmod(lpath, cb);
			lpath.popBack();
		});
	}
	bool Dir::ChMod(const PathBlock& pb, ModCB cb) {
		ToPathStr path = pb.plain_utf32();
		FStatus fstat = DirDep::Status(path);
		const bool bDir = fstat.flag & FStatus::DirectoryType;
		if(bDir)
			fstat.flag |= FStatus::UserExec;
		const bool bRecr = cb(pb, fstat);
		DirDep::Chmod(path, fstat.flag);
		return bDir && bRecr;
	}
	void Dir::chmod(ModCB cb) {
		PathBlock pb(*this);
		if(ChMod(pb, cb))
			_chmod(pb, cb);
	}
	void Dir::chmod(uint32_t mode) {
		chmod([mode](const PathBlock&, FStatus& fs) {
			fs.flag = mode;
			return false;
		});
	}
	FILE* Dir::openAsFP(const char* mode) const {
		return std::fopen(To8Str(plain_utf32()).getStringPtr(), mode);
	}
	FStatus Dir::status() const {
		return DirDep::Status(plain_utf8());
	}
	FTime Dir::time() const {
		return DirDep::Filetime(plain_utf8());
	}
	void Dir::RemoveAll(const std::string& path) {
		if(DirDep::IsDirectory(path)) {
			auto prev = Dir::GetCurrentDir();
			Dir::SetCurrentDir(path);
			// 下層のファイルやディレクトリを削除
			EnumEntryWildCard("*", [](const Dir& d){
				RemoveAll(d.plain_utf8(true));
			});
			DirDep::Rmdir(path);
			Dir::SetCurrentDir(prev);
		} else
			DirDep::Remove(path);
	}
	std::string Dir::plain_utf8(const bool bAbs) const {
		if(bAbs) {
			if(!PathBlock::isAbsolute()) {
				PathBlock pb(GetCurrentDir());
				pb <<= static_cast<const PathBlock&>(*this);
				return pb.plain_utf8(true);
			}
		}
		return PathBlock::plain_utf8(true);
	}
	std::u32string Dir::plain_utf32(const bool bAbs) const {
		if(bAbs) {
			if(!PathBlock::isAbsolute()) {
				PathBlock pb(GetCurrentDir());
				pb <<= static_cast<const PathBlock&>(*this);
				return pb.plain_utf32(true);
			}
		}
		return PathBlock::plain_utf32(true);
	}

	std::pair<std::string, int> Dir::_CalcOfs(const std::string& r) {
		std::string path;
		// 絶対パスの時は内部パスを無視する
		int ofs = 0;
		bool bAbs = false;
		if(r[0] == '/') {
			// Linux環境でのAbsolute path
			#ifdef WIN32
				AssertF("invalid absolute path");
			#endif
			path += '/';
			ofs = 1;
			bAbs = true;
		} else if(const auto letter = PathBlock::_GetDriveLetter(r.data(), r.data()+r.length())) {
			// Windows環境でのAbsolute path
			#ifdef WIN32
				path += *letter;
				path += ":\\";
			#else
				AssertF("invalid absolute path");
			#endif
			ofs = 3;
			bAbs = true;
		}
		if(!bAbs)
			path += "./";

		return {path, ofs};
	}
	void Dir::EnumEntryWildCard(const std::string& r, const EnumCB& cb) {
		if(r.empty())
			return;

		auto path_ofs = _CalcOfs(r);
		const auto sl = _WildcardToSegment(r.data() + path_ofs.second);
		_EnumEntry(sl.begin(), sl.end(), path_ofs.first, cb);
	}
	Dir::StrList Dir::EnumEntryWildCard(const std::string& s) {
		StrList ret;
		EnumEntryWildCard(s, [&ret](const Dir& dir){
			ret.push_back(dir.plain_utf8());
		});
		return ret;
	}
	void Dir::EnumEntry(const SearchL& s, const EnumCB& cb) {
		std::string path;
		_EnumEntry(s.begin(), s.end(), path, cb);
	}
	Dir::StrList Dir::EnumEntry(const SearchL& s) {
		StrList res;
		EnumEntry(s, [&res](const Dir& dir){
			res.push_back(dir.plain_utf8());
		});
		return res;
	}
}
