#pragma once
#include "fs/dir.hpp"
#include "spine/src/singleton.hpp"
#include "handle/sdl.hpp"
#include "uri.hpp"
#include <unordered_map>

namespace cereal {
	class access;
}
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
			const PathV& getPath(const std::string& resname) const;
	};
	class FileURI;
	//! リソースパスのキャッシュと参照
	/*! リソースマネージャクラスに内包して使う */
	class AppPathCache {
		private:
			using FileURI_SP = std::shared_ptr<FileURI>;
			// リソース名 -> URIのキャッシュ
			using Cache = std::unordered_map<std::string, FileURI_SP>;
			// リソース番号 -> リソースマップ
			using CacheV = std::vector<std::pair<std::string, Cache>>;
			mutable CacheV	_cache;

			void _init(const std::string rtname[], std::size_t n);
			friend class cereal::access;
			AppPathCache() = default;
			template <class Ar>
			friend void save(Ar&, const AppPathCache&);
			template <class Ar>
			friend void load(Ar&, AppPathCache&);

		public:
			bool operator == (const AppPathCache& a) const noexcept;
			bool operator != (const AppPathCache& a) const noexcept;

			template <std::size_t N>
			AppPathCache(const std::string (&rtname)[N]):
				_cache(N)
			{
				_init(rtname, N);
			}
			const FileURI_SP& uriFromResourceName(int n, const std::string& name) const;
	};
}
