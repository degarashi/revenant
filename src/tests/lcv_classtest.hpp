#pragma once
#include "lcvtest.hpp"
#include "../luaimpl.hpp"
#include "lubee/src/meta/countof.hpp"

namespace rev {
	namespace test {
		struct LCVNoImplement {};
		template <class T>
		using LCVPair0 = std::pair<T, LCVNoImplement>;
		template <class T>
		using LCVPair1 = std::pair<T,T>;
		template <class T0, class T1>
		using LCVPair2 = std::pair<T0,T1>;

		namespace code {
			extern const std::string lua_MemberReadXYZW;
			extern const std::string lua_MemberWriteXYZW;
		}

		template <class T>
		struct LCV_ClassTest : LCV_TestRW {
			using base_t = LCV_TestRW;
			using value_t = typename T::first_type;
			using lua_type = typename T::second_type;

			template <class TR>
			void _luaRegisterClass(TR*) {
				LuaImport::RegisterClass<TR>(*this->_lsp);
			}
			void _luaRegisterClass(LCVNoImplement*) {}

			void SetUp() override {
				base_t::SetUp();
				this->loadSharedPtrModule();
				this->_luaRegisterClass((lua_type*)nullptr);
			}

			template <class V, class Th>
			static bool HasZero(const V& v, const Th& threshold) {
				for(auto& val : v) {
					if(std::abs(val) < threshold)
						return true;
				}
				return false;
			}
			template <class V>
			V makeNonZero() {
				constexpr V Th = 1e-5;
				V ret = GenValue_t<V>()(*this);
				if(ret < Th)
					ret += ret*2;
				return ret;
			}
			template <class V>
			V makeVector() {
				return GenValue_t<V>()(*this);
			}
			template <class V>
			V makeNonZeroVector() {
				using elem_t = typename V::value_t;
				V v;
				for(auto& va : v)
					va = makeNonZero<elem_t>();
				return v;
			}
			void loadTestSource_Ret1(const std::string& src, const LuaType retT) {
				auto& lsp = this->_lsp;
				const int idx = lsp->getTop();
				loadTestSource(src);
				Assert0(lsp->getTop() == idx+1 &&
						lsp->type(-1) == retT);
			}
			void loadTestSource(const std::string& src) {
				this->_lsp->loadFromSource(mgr_rw.fromConstTemporal(src.c_str(), src.length()));
			}
			// Push/Pop等値チェック
			void pushTest() {
				ASSERT_NO_FATAL_FAILURE(base_t::pushTest<value_t>());
			}
			// LCVでLuaへ渡した時の型チェック
			void typeTest() {
				ASSERT_NO_FATAL_FAILURE(base_t::typeTest<value_t>());
			}
			void _readMemberTestXYZW() {
				auto& lsp = this->_lsp;
				// テストコードをロードすると1つのLua関数が返る
				this->loadTestSource_Ret1(code::lua_MemberReadXYZW, LuaType::Function);

				const value_t v0 = this->template makeVector<value_t>();
				// 1つのベクトル値と、
				LCV<value_t>()(lsp->getLS(), v0);
				// 要素数を積み
				lsp->push(value_t::size);
				// 演算テストコードを実行
				lsp->call(2, 1);
				// テーブルが1つ返る
				Assert0(lsp->getTop() == 1 &&
						lsp->type(1) == LuaType::Table);

				// Lua内での取得結果をC++側と比較
				auto res = lsp->toTable(1);
				for(int i=0 ; i<value_t::size ; i++) {
					ASSERT_EQ(lua_Number(v0[i]), boost::get<lua_Number>((*res)[i+1]));
				}
			}
			void _writeMemberTestXYZW() {
				auto& lsp = this->_lsp;
				// テストコードをロードすると1つのLua関数が返る
				this->loadTestSource_Ret1(code::lua_MemberWriteXYZW, LuaType::Function);

				const value_t v0 = GenValue_t<value_t>()(*this);
				const LCV<value_t&> lcv;
				lcv(lsp->getLS(), v0);

				const GenValue_t<lua_Number>	gv_f;
				lua_Number arg[value_t::size];
				for(auto& a : arg) {
					a = gv_f(*this);
					lsp->push(a);
				}
				lsp->call(countof(arg)+1, 0);

				// Luaコードでのメンバ書き換え結果をC++側と比較
				for(int i=0 ; i<value_t::size ; i++) {
					ASSERT_EQ(v0[i], (typename value_t::value_t)(arg[i]));
				}
			}
			void readMemberTestXYZW() {
				ASSERT_NO_FATAL_FAILURE(_readMemberTestXYZW());
			}
			void writeMemberTestXYZW() {
				ASSERT_NO_FATAL_FAILURE(_writeMemberTestXYZW());
			}
		};
	}
}

