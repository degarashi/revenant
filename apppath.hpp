#pragma once
#include "dir.hpp"
#include "handle.hpp"
#include "spine/singleton.hpp"
#include "uri.hpp"

namespace rev {
	#define mgr_path (::rev::AppPath::ref())
	//! アプリケーションのパスや引数、その他システムパスを登録
	/*! 将来的にはLuaによる変数定義で置き換え */
	class AppPath : public spi::Singleton<AppPath> {
		private:
			//! アプリケーション本体のパス
			PathBlock	_pbApp,
						_pbAppDir;
			using PathV = std::vector<PathBlock>;
			// AppPathを先頭に付加した状態で格納
			using PathM = std::unordered_map<std::string, PathV>;
			PathM			_path;
			struct E_SyntaxError : public std::invalid_argument {
				E_SyntaxError(int nline, char expect);
				E_SyntaxError(int nline, const std::string& msg);
			};

		public:
			AppPath(const PathBlock& apppath);
			//! シンプルなテキスト記述でシステムパスを設定
			/*! [ResourceType]\n
				path0\n
				path1\n
				...
			*/
			void setFromText(HRW hRW, bool bAdd);

			const PathBlock& getAppPath() const;
			const PathBlock& getAppDir() const;
			HRW getRW(const std::string& resname, const PathBlock& pattern, int access, PathBlock* opt) const;
			using CBEnum = std::function<bool (const Dir&)>;
			void enumPath(const std::string& resname, const PathBlock& pattern, CBEnum cb) const;
	};
	class URI;
	//! リソースパスのキャッシュと参照
	/*! リソースマネージャクラスに内包して使う */
	class AppPathCache {
		private:
			// リソース名 -> URIのキャッシュ
			using Cache = std::unordered_map<std::string, URI>;
			// リソース番号 -> リソースマップ
			using CacheV = std::vector<std::pair<std::string, Cache>>;
			mutable CacheV	_cache;

			using SV = std::vector<std::string>;
			friend class cereal::access;
			template <class Ar>
			void save(Ar& ar) const {
				// リソース名だけシリアライズ
				SV sv;
				for(auto& s : _cache)
					sv.emplace_back(s.first);
				ar(sv);
			}
			template <class Ar>
			void load(Ar& ar) {
				SV sv;
				ar(sv);

				const int n = sv.size();
				_cache.clear();
				_cache.resize(n);
				for(int i=0 ; i<n ; i++)
					_cache[i].first = sv[i];
			}

		public:
			AppPathCache() = default;
			void init(const std::string rtname[], std::size_t n);
			template <std::size_t N>
			AppPathCache(const std::string (&rtname)[N]):
				_cache(N)
			{
				init(rtname, N);
			}
			const URI& uriFromResourceName(int n, const std::string& name) const;
	};
}
