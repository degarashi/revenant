#include "apppath.hpp"
#include "sdl_rw.hpp"
#include <boost/format.hpp>

namespace rev {
	// ------------------- E_SyntaxError -------------------
	AppPath::E_SyntaxError::E_SyntaxError(const int nline, const char expect):
		std::invalid_argument("")
	{
		boost::format fmt("in line %1%:\n AppPath source(text) syntax error: expected '%2%'");
		static_cast<std::invalid_argument&>(*this) = std::invalid_argument((fmt % nline % expect).str());
	}
	AppPath::E_SyntaxError::E_SyntaxError(const int nline, const std::string& msg):
		std::invalid_argument("")
	{
		boost::format fmt("in line %1%:\n AppPath source(text) syntax error: %2%");
		static_cast<std::invalid_argument&>(*this) = std::invalid_argument((fmt % nline % msg).str());
	}
	// ------------------- AppPath -------------------
	AppPath::AppPath(const PathBlock& apppath):
		_pbApp(apppath),
		_pbAppDir(_pbApp)
	{
		Assert0(_pbApp.isAbsolute());
	}
	void AppPath::setFromText(HRW hRW, const bool bAdd) {
		if(!bAdd)
			_path.clear();

		RWops& ops = *hRW;
		ByteBuff buff = ops.readAll();
		const char *ptr = reinterpret_cast<const char*>(buff.data()),
					*ptrE = ptr + buff.size();
		char tmp[256];
		PathV* pEnt = nullptr;
		int line = 1;
		const auto fnRead = [&tmp](int line, const char* cur, const char* to, char cEnd) {
			int wcur = 0;
			while(cur != to) {
				char c = *cur++;
				if(c == cEnd) {
					tmp[wcur] = '\0';
					return cur;
				} else
					tmp[wcur++] = c;
			}
			throw E_SyntaxError(line, cEnd);
		};
		while(ptr < ptrE) {
			if(*ptr == '[') {
				// ---- Entry ----
				++ptr;
				// Read until ']'
				ptr = fnRead(line, ptr, ptrE, ']');
				pEnt = &_path[tmp];
				ptr = fnRead(line++, ptr, ptrE, '\n');
			} else {
				// ---- Path ----
				// Read until '\n'
				ptr = fnRead(line++, ptr, ptrE, '\n');
				PathBlock pbtmp(_pbAppDir);
				pbtmp <<= tmp;
				if(!pEnt)
					throw E_SyntaxError(line, "no active entry.");
				pEnt->emplace_back(std::move(pbtmp));
			}
		}
	}
	HRW AppPath::getRW(const std::string& resname, const PathBlock& pattern, const int access, PathBlock* opt) const {
		HRW hRet;
		enumPath(resname, pattern, [&hRet, &opt, access](const Dir& d){
			if(auto lh = mgr_rw.fromFile(d, access)) {
				if(opt)
					*opt = d;
				hRet = std::move(lh);
				return false;
			}
			return true;
		});
		return hRet;
	}
	void AppPath::enumPath(const std::string& resname, const PathBlock& pattern, CBEnum cb) const {
		const auto fnCheck = [&cb](const std::string& pat){
			bool bContinue = true;
			Dir::EnumEntryWildCard(pat, [&bContinue, &cb](const Dir& d){
				if(bContinue)
					bContinue = cb(d);
			});
		};
		// 絶対パスならそれで検索
		if(pattern.isAbsolute())
			fnCheck(pattern.plain_utf8());

		// リソース名に関連付けされたベースパスを付加して検索
		const auto itr = _path.find(resname);
		if(itr != _path.end()) {
			auto& pathv = itr->second;
			for(auto& path : pathv) {
				PathBlock pb = path;
				pb <<= pattern;
				fnCheck(pb.plain_utf8());
			}
		}
	}
	const PathBlock& AppPath::getAppPath() const {
		return _pbApp;
	}
	const PathBlock& AppPath::getAppDir() const {
		return _pbAppDir;
	}

	// ------------------- AppPathCache -------------------
	void AppPathCache::init(const std::string rtname[], const std::size_t n) {
		_cache.resize(n);
		for(std::size_t i=0 ; i<n ; i++)
			_cache[i].first = rtname[i];
	}
	const URI& AppPathCache::uriFromResourceName(const int n, const std::string& name) const {
		auto& cache = _cache[n];
		auto itr = cache.second.find(name);
		if(itr != cache.second.end())
			return itr->second;

		URI uri;
		uri.setType("file");
		mgr_path.enumPath(cache.first, PathBlock(name), [&uri](const Dir& d){
			static_cast<PathBlock&>(uri) = static_cast<const PathBlock&>(d);
			return false;
		});
		if(uri.path().empty())
			throw ResourceNotFound(name);
		auto& ent = cache.second[name];
		ent = std::move(uri);
		return ent;
	}
}
