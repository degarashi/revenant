#include "dir.hpp"
#include "ovr_functor.hpp"

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
	namespace {
		#define DIR_SEGMENT R"([\-_ \.\w/])"
		// ワイルドカード記述の置き換え
		const std::pair<std::regex, std::string> RE[4] = {
			{std::regex(R"(\\)"), R"(/)"},					// \ -> /
			{std::regex(R"(\*)"), DIR_SEGMENT R"(+?)"},		// * -> [_ \.\-\w/]+?
			{std::regex(R"(\?)"), DIR_SEGMENT},				// ? -> [_ \.\-\w]
			{std::regex(R"(\.)"), R"(\.)"}					// . -> \.
		};
		#undef DIR_SEGMENT
	}
	std::string Dir::ToRegEx(const std::string& s) {
		std::string s2 = std::regex_replace(s, RE[0].first, RE[0].second);
		s2 = std::regex_replace(s2, RE[3].first, RE[3].second);
		s2 = std::regex_replace(s2, RE[2].first, RE[2].second);
		s2 = std::regex_replace(s2, RE[1].first, RE[1].second);
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
	namespace {
		template <class Itr>
		bool FindChar(Itr itr, const Itr itrE, const char target, const std::size_t n) {
			D_Assert0(n >= 1);
			std::size_t cur = 0;
			std::size_t remain = itrE-itr;
			while(remain >= n) {
				const auto c = *itr;
				if(c == target) {
					if(++cur == n)
						return true;
				} else
					cur = 0;
				++itr;
				--remain;
			}
			return false;
		}
		// とりあえず確実に非Regexだろうと言える物はfalse, 他はtrue
		template <class Itr>
		bool HasRegex(Itr itr, const Itr itrE) {
			while(itr != itrE) {
				const auto c = *itr;
				if(c == '[') {
					if(itrE-itr >= 4) {
						if(*(itr+1) == '[') {
							if(FindChar(itr+2, itrE, ']', 2)) {
								// found [[ ]]
								return true;
							}
						}
					}
					if(FindChar(itr+1, itrE, ']', 1)) {
						// found [ ]
						return true;
					}
				} else if(c == '+')
					return true;
				else if(c == '*')
					return true;
				else if(c == '|')
					return true;
				else if(c == '\\')
					return true;
				++itr;
			}
			return false;
		}
	}
	// TODO Assertを例外送出に置き替え
	Dir::RegexL Dir::_ParseRegEx(const std::string& r) {
		Log(Error, r.c_str());
		RegexL rl;
		const auto push = [&rl](const auto from, const auto to) {
			Log(Error, std::string(from, to).c_str());
			if(HasRegex(from, to)) {
				rl.emplace_back(std::regex(from, to));
				Log(Error, "RegEx");
			} else {
				rl.emplace_back(std::string(from, to));
				Log(Error, "String");
			}
		};
		auto segEnd = r.begin(),
			segBegin = segEnd;
		const auto itrE = r.end();
		bool lb_skip = false;
		while(segEnd != itrE) {
			const auto c = *segEnd;
			if(lb_skip) {
				// [ があったら次の ] まで読み飛ばす
				if(c == RBK)
					lb_skip = false;
			} else {
				if(c == LBK)
					lb_skip = true;
				else if(c == SC) {
					const auto len = segEnd - segBegin;
					bool ignore = false;
					if(len == 0) {
						// スラッシュが2つ続けてあった場合
						ignore = true;
					} else if(len >= 2) {
						if(*segBegin == '\\' && *(segBegin+1) == '.') {
							if(len == 2) {
								// [. である場合]
								// セグメントをスキップ
								ignore = true;
							} else if(len == 4 && (*(segBegin+2) == '\\' && *(segBegin+3) == '.')) {
								// [.. である場合]
								// セグメントを1つ戻す
								Assert0(!rl.empty());
								rl.pop_back();
								ignore = true;
							}
						}
					}
					if(!ignore) {
						// 正常なセグメントが取得できたのでpush_back
						push(segBegin, segEnd);
					}
					segBegin = ++segEnd;
					continue;
				}
			}
			++segEnd;
		}
		if(segBegin != segEnd) {
			push(segBegin, segEnd);
		}
		Assert0(!lb_skip);
		return rl;
	}
	void Dir::_EnumEntryRegEx(RegexItr itr, const RegexItr itrE, std::string& lpath, const std::size_t baseLen, EnumCB cb) {
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
				if(!dir_flag && itr+1 != itrE)
					return;

				pushSeg();
				if(dir_flag) {
					if(itr+1 != itrE)
						_EnumEntryRegEx(itr+1, itrE, lpath, baseLen, cb);
					else
						cb(Dir(lpath));
				} else if(itr+1 == itrE)
					cb(Dir(lpath));
				popSeg();
			};
			boost::apply_visitor(OVR_Functor{
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
			}, *itr);
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
