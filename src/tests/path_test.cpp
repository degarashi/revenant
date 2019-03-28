#ifdef WIN32
	#include <intrin.h>
	namespace {
		const char* c_DriveLetter = "C:";
	}
#else
	namespace {
		const char* c_DriveLetter = "/";
	}
#endif
#include "test.hpp"
#include "../fs/path.hpp"
#include "../random/path.hpp"
#include "lubee/src/random/string.hpp"
#include "../serialization/path.hpp"
#include "lubee/src/check_serialization.hpp"

namespace rev {
	namespace test {
		class PathTest : public Random {};

		TEST_F(PathTest, Serialize) {
			// ランダムなパス生成
			auto& mt = this->mt();

			PathBlock pb = random::GenPath(mt.getUniformF<int>(), {1,20}, {1,16}, c_DriveLetter);
			lubee::CheckSerialization(pb);
		}
		// 拡張子の抽出テスト
		TEST_F(PathTest, Extension) {
			// ランダムなパス生成
			auto& rd = this->mt();
			PathBlock pb = random::GenPath(rd.getUniformF<int>(), {1,20}, {1,16}, c_DriveLetter);
			// 拡張子が無ければ空文字列が返る
			ASSERT_TRUE(pb.getExtension().empty());

			const std::string ext("the_extension"),
							ext2("another_extension");
			auto tmp = pb.getLast_utf8();
			tmp += ".";
			tmp += ext;
			pb.popBack();
			pb.pushBack(tmp);
			ASSERT_EQ(ext, pb.getExtension());

			pb.setExtension(ext2);
			ASSERT_EQ(ext2, pb.getExtension());
		}
		// ファイルパス末尾の数値に対するテスト
		TEST_F(PathTest, PathNumber) {
			// ランダムなパス生成
			auto& rd = this->mt();
			PathBlock pb = random::GenPath(rd.getUniformF<int>(), {1,20}, {1,16}, c_DriveLetter);

			// 余計な拡張子を付加
			pb.setExtension("foo");

			// パスの末尾に数値が付加されていなければnoneが返る
			ASSERT_FALSE(pb.getPathNum());

			// パスの末尾に数値を付加
			pb.popBack();
			pb.pushBack("bar123.foo64");
			auto num = pb.getPathNum();
			ASSERT_TRUE(static_cast<bool>(num));
			ASSERT_EQ(123, *num);

			// 数値の改変テスト
			pb.setPathNum([](Int_OP n) -> Int_OP{
				if(n)
					return *n * 2;
				return spi::none;
			});
			num = pb.getPathNum();
			ASSERT_TRUE(static_cast<bool>(num));
			ASSERT_EQ(123*2, *num);

			// 数値の削除テスト
			pb.setPathNum([](auto)-> Int_OP{ return spi::none; });
			ASSERT_FALSE(pb.getPathNum());
		}
		// 拡張子末尾の数値に対するテスト
		TEST_F(PathTest, ExtNumber) {
			// ランダムなパス生成
			auto& rd = this->mt();
			PathBlock pb = random::GenPath(rd.getUniformF<int>(), {1,20}, {1,16}, c_DriveLetter);

			// 拡張子が無ければnoneが返る
			ASSERT_FALSE(pb.getExtNum());

			// 拡張子の末尾に数値が無ければnoneが返る
			pb.setExtension("foo");
			ASSERT_FALSE(pb.getExtNum());

			// 指定した数値の取得テスト
			pb.setExtension("bar123");
			auto num = pb.getExtNum();
			ASSERT_TRUE(static_cast<bool>(num));
			ASSERT_EQ(123, *num);

			// 数値の改変テスト
			pb.setExtNum([](Int_OP n) -> Int_OP{
				if(n)
					return *n * 2;
				return spi::none;
			});
			num = pb.getExtNum();
			ASSERT_TRUE(static_cast<bool>(num));
			ASSERT_EQ(123*2, *num);

			// 数値の削除テスト
			pb.setExtNum([](auto)-> Int_OP{ return spi::none; });
			ASSERT_FALSE(pb.getExtNum());
		}
	}
}
