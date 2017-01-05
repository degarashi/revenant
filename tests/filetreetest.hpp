#pragma once
#include <string>
#include <unordered_set>
#include <memory>
#include <random>

namespace lubee {
	template <class MT>
	class Random;
	using RandomMT = Random<std::mt19937>;
}
namespace rev {
	class Dir;
	struct IFTFile;
	struct FStatus;
	using File_UP = std::unique_ptr<IFTFile>;
	struct FEv;
	using FEv_UP = std::unique_ptr<FEv>;
	namespace test {
		using RandomMT = lubee::RandomMT;
		extern const std::string c_testDir;
		using StringSet = std::unordered_set<std::string>;
		//! ランダムな以前の結果と重複しない名前を生成
		std::string RandomName(RandomMT& rd, StringSet& nameset);
		//! 指定されたファイル名でランダムデータを書き込む
		void PlaceRandomFile(RandomMT& rd, const Dir& d, const std::string& name);
		//! ランダムな名前、データでファイルを作成する
		std::string CreateRandomFile(RandomMT& rd, StringSet& nameset, const Dir& dir);
		void MakeRandomFileTree(const Dir& d, RandomMT& rd, int nFile, File_UP* pDst);
		//! テスト用にファイル・ディレクトリ構造を作る
		Dir MakeTestDir(RandomMT& rd, int nAction, File_UP* ft);

		//! FEvのハッシュ値を求める
		struct FEv_hash {
			std::size_t operator()(const FEv_UP& e) const;
			std::size_t operator()(const FEv* e) const;
		};
		//! FEvとFEv_UPの比較
		struct FEv_equal {
			bool operator()(const FEv_UP& fe, const FEv* fp) const;
			bool operator()(const FEv* fp, const FEv_UP& fe) const;
			bool operator()(const FEv_UP& fe0, const FEv_UP& fe1) const;
			bool operator()(const FEv* fp0, const FEv* fp1) const;
		};
		struct ActionNotify {
			virtual void onCreate(const Dir& /*path*/) {}
			virtual void onModify(const Dir& /*path*/, const FStatus& /*prev*/, const FStatus& /*cur*/) {}
			virtual void onModify_Size(const Dir& /*path*/, const FStatus& /*prev*/, const FStatus& /*cur*/) {}
			virtual void onDelete(const Dir& /*path*/) {}
			virtual void onMove_Pre(const Dir& /*from*/, const Dir& /*to*/) {}
			virtual void onMove_Post(const Dir& /*from*/, const Dir& /*to*/) {}
			virtual void onAccess(const Dir& /*path*/, const FStatus& /*prev*/, const FStatus& /*cur*/) {}
			virtual void onUpDir(const Dir& /*path*/) {}
			virtual void onDownDir(const Dir& /*path*/) {}
		};
		//! テスト用ディレクトリ構造に対してランダムなファイル操作を行う(ディレクトリ移動を含まない操作一回分)
		void MakeRandomActionSingle(RandomMT& rd, StringSet& orig_nameset, StringSet& nameset, const std::string& basePath, const Dir& path, ActionNotify& ntf);
		//! テスト用ディレクトリ構造に対してランダムなファイル操作を行う
		void MakeRandomAction(RandomMT& rd, StringSet& orig_nameset, StringSet& nameset, const Dir& path, int n, ActionNotify& ntf);
	}
}
