#include "../filetreetest.hpp"
#include "../../filetree.hpp"
#include "../../watch.hpp"
#include "../../dir.hpp"
#include "lubee/src/range.hpp"
#include "lubee/src/random.hpp"
#include <fstream>
#include <gtest/gtest.h>

namespace rev {
	namespace test {
		const std::string c_testDir = "test_dir";
		Dir MakeTestDir(RandomMT& rd, const int nAction, File_UP* ft) {
			Dir dir(Dir::GetProgramDir());
			dir <<= c_testDir;
			if(dir.isFile() || dir.isDirectory())
				Dir::RemoveAll(dir.plain_utf8());
			MakeRandomFileTree(dir, rd, nAction, ft);
			return dir;
		}

		namespace {
			const lubee::RangeI c_nameLen{3,16},
								c_sizeLen{8, 256},
								c_wordRange{'A', 'Z'};
		}
		// -------------- FEv_hash --------------
		std::size_t FEv_hash::operator()(const FEv_UP& e) const {
			return operator()(e.get());
		}
		std::size_t FEv_hash::operator()(const FEv* e) const {
			return std::hash<std::string>()(*e->basePath) -
				std::hash<std::string>()(e->name) *
				std::hash<bool>()(e->isDir) +
				std::hash<uint32_t>()(e->getType());
		}
		// -------------- FEv_equal --------------
		bool FEv_equal::operator()(const FEv_UP& fe, const FEv* fp) const {
			return (*this)(fe.get(), fp);
		}
		bool FEv_equal::operator()(const FEv* fp, const FEv_UP& fe) const {
			return (*this)(fp, fe.get());
		}
		bool FEv_equal::operator()(const FEv_UP& fe0, const FEv_UP& fe1) const {
			return (*this)(fe0.get(), fe1.get());
		}
		bool FEv_equal::operator()(const FEv* fp0, const FEv* fp1) const {
			return *fp0 == *fp1;
		}
		std::string RandomName(RandomMT& rd, StringSet& nameset) {
			for(;;) {
				char ftmp[c_nameLen.to+1];
				int fname = rd.getUniform<int>(c_nameLen);
				for(int k=0 ; k<fname ; k++)
					ftmp[k] = rd.getUniform<int>(c_wordRange);
				ftmp[fname] = '\0';
		
				std::string str(ftmp);
				if(nameset.count(str) == 0) {
					nameset.insert(str);
					return str;
				}
			}
		}
		void PlaceRandomFile(RandomMT& rd, const Dir& d, const std::string& name) {
			Dir dir(d);
			dir <<= name;
			auto path = dir.plain_utf8();
			std::ofstream ofs(path.c_str(), std::ios::binary);
			const int fsize = rd.getUniform<int>(c_sizeLen);
			char tmp[c_sizeLen.to];
			for(int k=0 ; k<fsize ; k++)
				tmp[k] = rd.getUniform<int>({0, 0xff});
			ofs.write(tmp, fsize);
		}
		std::string CreateRandomFile(RandomMT& rd, StringSet& nameset, const Dir& dir) {
			const std::string name = RandomName(rd, nameset);
			PlaceRandomFile(rd, dir, name);
			return name;
		}
		void MakeRandomFileTree(const Dir& d, RandomMT& rd, const int nFile, File_UP* pDst) {
			Dir dir(d);
			dir.mkdir(FStatus::AllRead);
			std::vector<DirInfo*>	pDi;
			if(pDst) {
				DirInfo* tmp;
				pDst->reset(tmp = new DirInfo(d.getLast_utf8(), DirDep::Status(d.plain_utf8())));
				pDi.emplace_back(tmp);
			}

			// 一度使ったファイル名は使わない
			StringSet nameset;
			int layer = 0;
			for(int j=0 ; j<nFile ; j++) {
				switch(rd.getUniform<int>({0, 2})) {
					case 0:
						// ディレクトリを1つ上がる
						if(layer > 0) {
							const FStatus stat = DirDep::Status(dir.plain_utf8());
							dir.popBack();
							if(!pDi.empty()) {
								pDi.back()->fstat = stat;
								pDi.pop_back();
							}
							--layer;
						}
						break;
					case 1: {
						// 下層にディレクトリ作製 & 移動
						bool bRetry = true;
						while(bRetry) {
							try {
								std::string str = RandomName(rd, nameset);
								dir <<= str;
								dir.mkdir(FStatus::AllRead);
								if(!pDi.empty()) {
									DirInfo* tmp;
									pDi.back()->fmap.emplace(str, File_UP(tmp = new DirInfo(str, FStatus())));
									pDi.emplace_back(tmp);
								}
								bRetry = false;
							} catch(const std::string& s) {
								// 既に同名のファイルがあったら再トライ
								//TODO: 他のエラーと区別がつかないのを修正
								dir.popBack();
							}
						}
						++layer;
						break; }
					case 2: {
						// カレントにファイルを作成 (ファイル名と内容はランダム)
						const std::string name = CreateRandomFile(rd, nameset, dir);
						if(!pDi.empty()) {
							dir <<= name;
							const FStatus stat = DirDep::Status(dir.plain_utf8());
							dir.popBack();
							pDi.back()->fmap.emplace(name, File_UP(new FileInfo(name, stat)));
						}
						break; }
				}
			}
			// ディレクトリを1つ上がる
			while(!pDi.empty()) {
				const FStatus stat = DirDep::Status(dir.plain_utf8());
				dir.popBack();
				pDi.back()->fstat = stat;
				pDi.pop_back();
			}
		}
		namespace {
			DefineEnum(FileAction,
				(Create)		// ファイル作成
				(Modify)		// ファイルデータ変更(サイズはそのまま)
				(Modify_Size)	// ファイルのサイズを変更
				(Delete)		// ファイル又はディレクトリ削除
				(Move)			// ファイル移動
				(Access)		// アクセスタイムを更新
			);
			DefineEnum(FileTreeAction,
				(Manipulate)	// 何かファイル操作する
				(UpDir)			// ディレクトリを1つ上がる
				(DownDir)		// ディレクトリを1つ降りる(現階層にディレクトリがない場合は何もしない)
			);
			using StringV = std::vector<std::string>;
			auto Select(RandomMT& rd, StringSet& nameset, const Dir& dir, const uint32_t flag, const bool bAdd) {
				StringV ps;
				auto path = dir.plain_utf8() + "/*";
				Dir::EnumEntryWildCard(path, [&nameset, &ps, flag](const Dir& dir) {
					FStatus fs = dir.status();
					if(fs.flag & flag) {
						std::string name = dir.getLast_utf8();
						if(nameset.count(name) == 0)
							ps.push_back(std::move(name));
					}
				});
				if(ps.empty())
					return std::string();
				const int idx = rd.getUniform<int>({0, ps.size()-1});
				if(bAdd)
					nameset.emplace(ps[idx]);
				return std::move(ps[idx]);
			}
			//! ランダムにファイルやディレクトリを選択(一度選んだファイルは二度と選ばない)
			auto SelectFile(RandomMT& rd, StringSet& nameset, const Dir& dir, const bool bAdd) {
				return Select(rd, nameset, dir, FStatus::FileType, bAdd);
			}
			auto SelectBoth(RandomMT& rd, StringSet& nameset, const Dir& dir, const bool bAdd) {
				return Select(rd, nameset, dir, FStatus::FileType | FStatus::DirectoryType, bAdd);
			}
			auto SelectDir(RandomMT& rd, StringSet& nameset, const Dir& dir, const bool bAdd) {
				return Select(rd, nameset, dir, FStatus::DirectoryType, bAdd);
			}
			// ファイルツリーのランダムな場所へのパスを作成
			auto GenerateRandomPath(RandomMT& rd, StringSet& orig_nameset, StringSet& nameset, const Dir& dir) {
				Dir tdir(dir);
				for(;;) {
					// 階層を降りる or 留まる
					if(rd.getUniform<int>({0, 1}) == 0) {
						auto ps = SelectDir(rd, nameset, tdir, false);
						if(!ps.empty()) {
							tdir <<= ps;
							continue;
						}
					}
					for(;;) {
						auto name = RandomName(rd, nameset);
						if(orig_nameset.count(name) == 0) {
							tdir <<= name;
							break;
						}
					}
					return tdir.getSegment_utf8(dir.segments(), PathBlock::End);
				}
			}
		}
		void MakeRandomActionSingle(RandomMT& rd, StringSet& orig_nameset, StringSet& nameset, const std::string& basePath, const Dir& path, ActionNotify& ntf)
		{
			Dir dir(path);
			switch(rd.getUniform<int>({0, FileAction::_Num})) {
				case FileAction::Create: {
					std::string str = RandomName(rd, nameset);
					PlaceRandomFile(rd, dir, str);
					dir <<= str;
					ntf.onCreate(dir);
					dir.popBack();
					break;
				}
				case FileAction::Modify: {
					auto ps = SelectFile(rd, nameset, dir, true);
					if(!ps.empty()) {
						dir <<= ps;
		
						auto st = dir.status();
						// 先頭1バイトを書き換え
						{
							char ch;
							FILE* fp = ::fopen(dir.plain_utf8().c_str(), "r+");
							::fread(&ch, 1, 1, fp);
							++ch;
							::fseek(fp, 0, SEEK_SET);
							::fwrite(&ch, 1, 1, fp);
							#ifdef UNIX
								::fsync(::fileno(fp));
							#endif
							::fclose(fp);
						}
						ntf.onModify(dir, st, dir.status());
						dir.popBack();
					}
					break;
				}
				case FileAction::Modify_Size: {
					auto ps = SelectFile(rd, nameset, dir, true);
					if(!ps.empty()) {
						dir <<= ps;
						EXPECT_FALSE(dir.isDirectory());
						auto prev = dir.status();
						// 末尾1バイト追加
						{
							char ch = 0xfe;
							FILE* fp = ::fopen(dir.plain_utf8().c_str(), "a");
							::fwrite(&ch, 1, 1, fp);
							#ifdef UNIX
								::fsync(::fileno(fp));
							#endif
							::fclose(fp);
						}
		
						ntf.onModify_Size(dir, prev, dir.status());
						dir.popBack();
					}
					break;
				}
				case FileAction::Delete: {
					auto ps = SelectFile(rd, nameset, dir, true);
					if(!ps.empty()) {
						dir <<= ps;
						EXPECT_FALSE(dir.isDirectory());
						ntf.onDelete(dir);
						dir.remove();
						dir.popBack();
					}
					break;
				}
				case FileAction::Move: {
					auto ps = SelectBoth(rd, nameset, dir, true);
					if(!ps.empty()) {
						dir <<= ps;
						std::string pathTo = GenerateRandomPath(rd, orig_nameset, nameset, Dir(basePath));
						Dir dirTo(basePath + '/' + pathTo);
		
						EXPECT_NE(dir, dirTo);
						ntf.onMove_Pre(dir, dirTo);
						dir.move(dirTo.plain_utf8());
						ntf.onMove_Post(dir, dirTo);
						dir.popBack();
					}
					break;
				}
				case FileAction::Access: {
					auto ps = SelectFile(rd, nameset, dir, true);
					if(!ps.empty()) {
						dir <<= ps;
						auto st = dir.status();
						{
							// fsyncを使いたいのでfopenでファイルを開く
							char c;
							FILE* fp = ::fopen(dir.plain_utf8().c_str(), "r");
							::fread(&c, 1, 1, fp);
							#ifdef UNIX
								::fsync(::fileno(fp));
							#endif
							::fclose(fp);
						}
						ntf.onAccess(dir, st, dir.status());
						dir.popBack();
					}
					break;
				}
			}
		}
		void MakeRandomAction(RandomMT& rd, StringSet& orig_nameset, StringSet& nameset, const Dir& path, int n, ActionNotify& ntf)
		{
			Dir dir(path);
			auto basePath = dir.plain_utf8();
			int layer = 0;
			for(int i=0 ; i<n ; i++) {
				switch(rd.getUniform<int>({0, FileTreeAction::_Num})) {
					case FileTreeAction::Manipulate:
						MakeRandomActionSingle(rd, orig_nameset, nameset, basePath, dir, ntf);
						break;
					case FileTreeAction::UpDir: {
						if(layer > 0) {
							dir.popBack();
							--layer;
		
							ntf.onUpDir(dir);
						}
						break;
					}
					case FileTreeAction::DownDir: {
						auto ps = SelectDir(rd, nameset, dir, false);
						if(!ps.empty()) {
							dir <<= ps;
							++layer;
		
							ntf.onDownDir(dir);
						}
						break;
					}
				}
			}
		}
	}
}
