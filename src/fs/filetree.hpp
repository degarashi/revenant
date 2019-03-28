//! ファイルツリー構造を保持するクラス
#pragma once
#include "spine/src/enum.hpp"
#include "../abstbuffer.hpp"
#include "time.hpp"
#include <unordered_map>

namespace rev {
	class Dir;
	class PathBlock;
	struct IFTFile;
	using File_UP = std::unique_ptr<IFTFile>;
	struct FRecvNotify;
	using String_SP = std::shared_ptr<std::string>;

	//! ファイルツリーの構築
	struct FileTree {
		DefineEnumPair(
			Type,
			((File)(0x01))
			((Directory)(0x02))
		);
		DefineEnumPair(
			Diff,
			((Created)(0x01))
			((Deleted)(0x02))
			((Accessed)(0x04))
			((Writed)(0x08))
		);

		static File_UP ConstructFTree(Dir& dir);
		static File_UP ConstructFTree(To32Str path);
		static void For_Each(const IFTFile* ft, const std::function<void(const std::string&, const IFTFile*)>& cb, PathBlock& pblock);
		//! 下層のエントリ全てに対して削除イベントを送出
		static void ThrowRemoveEvent(FRecvNotify& visitor, const String_SP& base, const IFTFile* ft, PathBlock& pblock);
		//! 下層のエントリ全てに対し作成イベントを送出
		static void ThrowCreateEvent(FRecvNotify& visitor, const String_SP& base, const IFTFile* ft, PathBlock& pblock);
		//! ファイルツリー差分を巡ってコールバック関数を呼ぶ
		static bool VisitDifference(FRecvNotify& visitor, const String_SP& base, const IFTFile* f0, const IFTFile* f1, PathBlock& pblock);
	};

	struct IFTFile {
		std::string		fname;
		FStatus			fstat;

		virtual bool isDirectory() const = 0;
		virtual FileTree::Type getType() const = 0;
		virtual bool equal(const IFTFile& f) const;

		IFTFile(const std::string& name, const FStatus& fs);
		const std::string& getName() const;
		const FStatus& getStatus() const;
		bool operator == (const IFTFile& f) const;
		bool operator != (const IFTFile& f) const;

		//! デバッグ用プリント関数
		virtual void print(std::ostream& os, int indent=0) const = 0;
		virtual ~IFTFile() {}
	};
	//! ファイル1つに相当
	struct FileInfo : IFTFile {
		using IFTFile::IFTFile;
		FileTree::Type getType() const override;
		bool isDirectory() const override;
		bool equal(const IFTFile& f) const override;
		void print(std::ostream& os, int indent) const override;
	};
	//! ディレクトリに該当
	struct DirInfo : IFTFile {
		using FileMap = std::unordered_map<std::string, File_UP>;
		FileMap		fmap;

		using IFTFile::IFTFile;
		FileTree::Type getType() const override;
		bool isDirectory() const override;
		bool equal(const IFTFile& f) const override;
		void print(std::ostream& os, int indent) const override;
	};
}
