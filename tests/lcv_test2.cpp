#include "lcvtest.hpp"
#include "lubee/rect.hpp"

namespace rev {
	namespace test {
		struct LCVNoImplement {};
		template <class T>
		using LCVPair0 = std::pair<T, LCVNoImplement>;
		template <class T>
		using LCVPair1 = std::pair<T,T>;
		template <class T0, class T1>
		using LCVPair2 = std::pair<T0,T1>;

		using TypesL = ::testing::Types<
			LCVPair0<lubee::Range<int>>, LCVPair0<lubee::Range<float>>, LCVPair0<lubee::Range<double>>,
			LCVPair0<lubee::SizeI>, LCVPair0<lubee::SizeF>,
			LCVPair0<lubee::RectI>, LCVPair0<lubee::RectF>,
			LCVPair1<frea::DegF>, LCVPair2<frea::DegD, frea::DegF>, LCVPair1<frea::RadF>, LCVPair2<frea::RadD, frea::RadF>
		>;
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
		};
		TYPED_TEST_CASE(LCV_ClassTest, TypesL);
		TYPED_TEST(LCV_ClassTest, Push) {
			USING(value_t);
			ASSERT_NO_FATAL_FAILURE(this->template pushTest<value_t>());
		}
		TYPED_TEST(LCV_ClassTest, Type) {
			USING(value_t);
			ASSERT_NO_FATAL_FAILURE(this->template typeTest<value_t>());
		}

		template <class T>
		using LCV_Vector = LCV_ClassTest<T>;
		using TypesV = ::testing::Types<
			LCVPair1<frea::Vec2>, LCVPair2<frea::AVec2, frea::Vec2>,
			LCVPair1<frea::Vec3>, LCVPair2<frea::AVec3, frea::Vec3>,
			LCVPair1<frea::Vec4>, LCVPair2<frea::AVec4, frea::Vec4>
		>;
		TYPED_TEST_CASE(LCV_Vector, TypesV);
		// Push/Pop等値チェック
		TYPED_TEST(LCV_Vector, Push) {
			USING(value_t);
			ASSERT_NO_FATAL_FAILURE(this->template pushTest<value_t>());
		}
		// LCVでLuaへ渡した時の型チェック
		TYPED_TEST(LCV_Vector, Type) {
			USING(value_t);
			ASSERT_NO_FATAL_FAILURE(this->template typeTest<value_t>());
		}
		// 一旦LCValueに変換する
		TYPED_TEST(LCV_Vector, LCValue) {
			USING(lua_type);
			USING(value_t);
			lua_State* ls = this->_lsp->getLS();
			const auto val0 = GenValue<value_t>()(*this);
			LCV<value_t>()(ls, val0);
			const auto lc = LCV<LCValue>()(1, ls, nullptr);
			const auto val1 = boost::get<lua_type>(lc);
			ASSERT_EQ(val0, val1);
		}
		namespace {
			const std::string lua_OperatorCheck =
				"return function(v0, v1, s)\n"
					"return {\n"
						// V + V
						"v0 + v1,\n"
						// V - V
						"v0 - v1,\n"
						// V * V
						"v0 * v1,\n"
						"v0 * s,\n"
						// V / V
						"v0 / v1,\n"
						"v0 / s,\n"
						// #V
						"#v0,\n"
						// -V
						"-v0,\n"
						// V == V
						"v0 == v1,\n"
						// tostring(V)
						"tostring(v0)"
				"	}\n"
				"end";
			template <class V>
			bool HasZero(const V& v, const typename V::value_t& threshold) {
				for(auto& val : v) {
					if(std::abs(val) < threshold)
						return true;
				}
				return false;
			}
		}
		// Luaでの演算チェック
		TYPED_TEST(LCV_Vector, Operator) {
			USING(lua_type);
			USING(value_t);

			// 2つのベクトル値と1つのスカラー値を生成
			const GenValue_t<value_t>		gv_v;
			const GenValue_t<lua_Number>	gv_s;
			constexpr lua_Number th = 1e-5;
			value_t v0, v1;
			do {
				v0 = gv_v(*this);
			} while(HasZero(v0, th));
			// 一定の確率でv0とv1を同じ値にする
			if(this->getRDI()({0,1}) == 0)
				v1 = v0;
			else {
				do {
					v1 = gv_v(*this);
				} while(HasZero(v1, th));
			}
			lua_Number s;
			do {
				s = gv_s(*this);
			} while(std::abs(s) < th);

			auto& lsp = this->_lsp;
			lsp->loadFromSource(mgr_rw.fromConstTemporal(lua_OperatorCheck.c_str(), lua_OperatorCheck.length()));
			// テストコードをロードすると1つのLua関数が返る
			Assert0(lsp->getTop() == 1 &&
					lsp->type(1) == LuaType::Function);

			lua_State* ls = lsp->getLS();
			const LCV<value_t> lcv;
			// 2つのベクトル値と、
			lcv(ls, v0);
			lcv(ls, v1);
			// 何かスカラー値を積み
			lsp->push(s);
			// 演算テストコードを実行
			lsp->call(3, 1);
			// テーブルが1つ返る
			Assert0(lsp->getTop() == 1 &&
					lsp->type(1) == LuaType::Table);

			// Lua内での演算結果をC++側と比較
			auto res = lsp->toTable(1);
			// V + V
			ASSERT_EQ(lua_type(v0+v1), boost::get<lua_type>((*res)[1]));
			// V - V
			ASSERT_EQ(lua_type(v0-v1), boost::get<lua_type>((*res)[2]));
			// V * V
			ASSERT_EQ(lua_type(v0*v1), boost::get<lua_type>((*res)[3]));
			// V * s
			ASSERT_EQ(lua_type(v0*s), boost::get<lua_type>((*res)[4]));
			// V / V
			ASSERT_EQ(lua_type(v0/v1), boost::get<lua_type>((*res)[5]));
			// V / s
			ASSERT_EQ(lua_type(v0/s), boost::get<lua_type>((*res)[6]));
			// #V
			ASSERT_EQ(lua_type(v0).length(), boost::get<lua_Number>((*res)[7]));
			// -V
			ASSERT_EQ(-lua_type(v0), boost::get<lua_type>((*res)[8]));
			// V == V
			ASSERT_EQ(v0==v1, boost::get<bool>((*res)[9]));
			// tostring(V)
			ASSERT_EQ(lubee::ToString(v0), (*res)[10].toString());
		}
	}
}
