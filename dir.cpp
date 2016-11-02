#include "dir.hpp"

namespace rev {
	Dir::PathReset::PathReset(): cwd(DirDep::Getcwd()) {}
	Dir::PathReset::~PathReset() {
		if(!std::uncaught_exception())
			DirDep::Chdir(cwd);
	}

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
	std::string Dir::ToRegEx(const std::string& s) {
		// ワイルドカード記述の置き換え
		// * -> ([_ \.\-\w]+?)
		// ? -> ([_ \.\-\w])
		// . -> (\.)

		// バックスラッシュをスラッシュに置き換え
		// \ -> /
		std::regex re[4] = {std::regex(R"(\\)"), std::regex(R"(\*)"), std::regex(R"(\?)"), std::regex(R"(\.)")};
		std::string s2 = std::regex_replace(s, re[0], R"(/)");
		s2 = std::regex_replace(s2, re[3], R"(\\.)");
		s2 = std::regex_replace(s2, re[2], R"([_ \\.\\-\\w])");
		s2 = std::regex_replace(s2, re[1], R"([_ \\.\\-\\w]+?)");
		return s2;
	}
	std::string Dir::setCurrentDir() const {
		std::string prev = GetCurrentDir();
		SetCurrentDir(plain_utf8());
		return prev;
	}
	Dir::StrList Dir::EnumEntryRegEx(const std::string& r) {
		StrList res;
		EnumEntryRegEx(r, [&res](const Dir& dir){
			res.push_back(dir.plain_utf8());
		});
		return res;
	}
	Dir::StrList Dir::EnumEntryWildCard(const std::string& s) {
		return EnumEntryRegEx(ToRegEx(s));
	}
	void Dir::EnumEntryWildCard(const std::string& s, EnumCB cb) {
		EnumEntryRegEx(ToRegEx(s), cb);
	}
	Dir::RegexL Dir::_ParseRegEx(const std::string& r) {
		RegexL rl;
		auto itr = r.begin(),
			itrE = r.end(),
			itr0 = itr;
		bool bSkip = false;
		while(itr != itrE) {
			auto c = *itr;
			if(bSkip) {
				if(c == RBK)
					bSkip = false;
			} else {
				if(c == LBK)
					bSkip = true;
				else if(c == SC) {
					auto diff = itr - itr0;
					bool bIgnore = false;
					if(diff == 0)
						bIgnore = true;
					else if(diff >= 2) {
						if(*itr0 == '\\' && *(itr0+1) == '.') {
							if(diff == 2) {
								// セグメントをスキップ
								bIgnore = true;
							} else if(diff == 4 && (*(itr0+2) == '\\' && *(itr0+3) == '.')) {
								// セグメントを1つ戻す
								Assert0(!rl.empty());
								rl.pop_back();
								bIgnore = true;
							}
						}
					}
					if(!bIgnore)
						rl.emplace_back(itr0, itr);
					itr0 = ++itr;
					continue;
				}
			}
			++itr;
		}
		if(itr0 != itr)
			rl.emplace_back(itr0, itr);
		return rl;
	}
	void Dir::_EnumEntryRegEx(RegexItr itr, RegexItr itrE, std::string& lpath, const std::size_t baseLen, EnumCB cb) {
		if(itr == itrE)
			return;

		size_t pl = lpath.size();
		DirDep::EnumEntry(lpath, [=, &lpath, &cb](const PathCh* name, bool) {
			if(name[0]==PathCh(DOT)) {
				if(name[1]==PathCh(EOS) || name[1]==PathCh(DOT))
					return;
			}
			std::string s(To8Str(name).moveTo());
			std::smatch m;
			if(std::regex_match(s, m, *itr)) {
				if(lpath.back() != SC)
					lpath += SC;
				lpath += s;
				if(DirDep::IsDirectory(ToPathStr(lpath))) {
					if(itr+1 != itrE)
						_EnumEntryRegEx(itr+1, itrE, lpath, baseLen, cb);
					else
						cb(Dir(lpath));
				} else
					cb(Dir(lpath));
				lpath.resize(pl);
			}
		});
	}
	void Dir::_EnumEntry(const std::string& /*s*/, const std::string& path, EnumCB cb) {
		DirDep::EnumEntry(path, [&cb](const PathCh* name, bool /*bDir*/) {
			PathStr s(ToPathStr(name).moveTo());
			if(s == name)
				cb(Dir(s));
		});
	}
	void Dir::EnumEntryRegEx(const std::string& r, EnumCB cb) {
		if(r.empty())
			return;
		std::string path;
		// 絶対パスの時は内部パスを無視する
		int ofs = 0;
		bool bAbs = false;
		if(r[0] == '/') {
			#ifdef WIN32
				Assert(false, "invalid absolute path");
			#endif
			path += '/';
			ofs = 1;
			bAbs = true;
		} else if(auto letter = _GetDriveLetter(&r[0], &r[0] + r.length())) {
			// windowsの場合のみドライブ文字を出力
			#ifdef WIN32
				path += *letter;
				path += ':';
			#else
				path += '/';
			#endif
			ofs = 2;
			bAbs = true;
		}
		if(!bAbs)
			path += "./";

		try {
			RegexL rl = _ParseRegEx(r.substr(ofs));
			_EnumEntryRegEx(rl.begin(), rl.end(), path, path.size()+1, cb);
		} catch(const std::regex_error& e) {
			// 正規表現に何かエラーがある時は単純に文字列比較とする
			_EnumEntry(r, path, cb);
		}
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

		int nsg = segments();
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
	void Dir::_chmod(PathBlock& lpath, ModCB cb) {
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
}
