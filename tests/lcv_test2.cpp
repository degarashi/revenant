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
		struct LCV_Vector : LCV_ClassTest<T> {
			using base_t = LCV_ClassTest<T>;
			using value_t = typename base_t::value_t;
			template <class V>
			static bool HasZero(const V& v, const typename V::value_t& threshold) {
				for(auto& val : v) {
					if(std::abs(val) < threshold)
						return true;
				}
				return false;
			}
			auto makeNonZero() {
				return GenValue_t<typename value_t::value_t>()(*this);
			}
			value_t makeNonZeroVector() {
				constexpr lua_Number th = 1e-5;
				value_t v;
				do {
					v = GenValue_t<value_t>()(*this);
				} while(this->HasZero(v, th));
				return v;
			}
			value_t makeVector() {
				return GenValue_t<value_t>()(*this);
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
		};
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
			const std::string lua_MemberRead =
				"return function(v, n)\n"
					"local member = {\"x\", \"y\", \"z\", \"w\"}\n"
					"local res = {}\n"
					"for i=1,n+1 do\n"
						"res[i] = v[member[i]]\n"
					"end\n"
					"return res\n"
				"end"
			;
			const std::string lua_MemberWrite =
				"return function(v, ...)\n"
					"local member = {\"x\", \"y\", \"z\", \"w\"}\n"
					"local arg = {...}\n"
					"for i=1,#arg do\n"
						"v[member[i]] = arg[i]\n"
					"end\n"
				"end"
			;
			const std::string lua_ConstMethod =
				"return function(v0, v1, sMin, sMax)\n"
					"return {\n"
						// dot
						"v0:dot(v1),\n"
						// average
						"v0:average(),\n"
						// distance
						"v0:distance(v1),\n"
						// dist_sq
						"v0:dist_sq(v1),\n"
						// getMin
						"v0:getMin(v1),\n"
						// getMax
						"v0:getMax(v1),\n"
						// normalization
						"v0:normalization(),\n"
						// length
						"v0:length(),\n"
						// len_sq
						"v0:len_sq(),\n"
						// isNaN
						"v0:isNaN(),\n"
						// isOutstanding
						"v0:isOutstanding(),\n"
						// saturation
						"v0:saturation(sMin, sMax),\n"
						// l_intp
						"v0:l_intp(v1, sMin),\n"
						// absolute
						"v0:absolute(),\n"
						// getMinValue
						"v0:getMinValue(),\n"
						// getMaxValue
						"v0:getMaxValue(),\n"
						// linearNormalization
						"v0:linearNormalization(),\n"
						// isZero
						"v0:isZero(sMin)\n"
					"}\n"
				"end"
			;
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
		}
		// Luaでのメンバ変数読み込みチェック
		TYPED_TEST(LCV_Vector, ReadMember) {
			USING(value_t);
			auto& lsp = this->_lsp;
			// テストコードをロードすると1つのLua関数が返る
			this->loadTestSource_Ret1(lua_MemberRead, LuaType::Function);

			const value_t v0 = this->makeVector();
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
		// Luaでのメンバ変数書き込みチェック
		TYPED_TEST(LCV_Vector, WriteMember) {
			USING(value_t);
			auto& lsp = this->_lsp;
			// テストコードをロードすると1つのLua関数が返る
			this->loadTestSource_Ret1(lua_MemberWrite, LuaType::Function);

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
		// LuaでのConstメンバ関数呼び出しチェック
		TYPED_TEST(LCV_Vector, ConstMethod) {
			// テストコードをロードすると1つのLua関数が返る
			this->loadTestSource_Ret1(lua_ConstMethod, LuaType::Function);

			USING(value_t);
			// 2つのベクトル値と2つのスカラー値を生成
			value_t v0 = this->makeNonZeroVector(),
					v1;
			// 一定の確率でv0とv1を同じ値にする
			if(this->getRDI()({0,1}) == 0)
				v1 = v0;
			else
				v1 = this->makeNonZeroVector();
			lua_Number sMin = this->makeNonZero(),
					   sMax = this->makeNonZero();
			if(sMin > sMax)
				std::swap(sMin, sMax);

			auto& lsp = this->_lsp;
			lua_State* ls = lsp->getLS();
			const LCV<value_t> lcv_v;
			lcv_v(ls, v0);
			lcv_v(ls, v1);
			lsp->push(sMin);
			lsp->push(sMax);
			lsp->call(4, 1);
			// テーブルが1つ返る
			Assert0(lsp->getTop() == 1 &&
					lsp->type(1) == LuaType::Table);

			USING(lua_type);
			auto res = lsp->toTable(-1);
			// dot
			ASSERT_EQ(v0.dot(v1), boost::get<lua_Number>((*res)[1]));
			// average
			ASSERT_EQ(v0.average(), boost::get<lua_Number>((*res)[2]));
			// distance
			ASSERT_EQ(v0.distance(v1), boost::get<lua_Number>((*res)[3]));
			// dist_sq
			ASSERT_EQ(v0.dist_sq(v1), boost::get<lua_Number>((*res)[4]));
			// getMin
			ASSERT_EQ(v0.getMin(v1), boost::get<lua_type>((*res)[5]));
			// getMax
			ASSERT_EQ(v0.getMax(v1), boost::get<lua_type>((*res)[6]));
			// normalization
			ASSERT_EQ(v0.normalization(), boost::get<lua_type>((*res)[7]));
			// length
			ASSERT_EQ(v0.length(), boost::get<lua_Number>((*res)[8]));
			// len_sq
			ASSERT_EQ(v0.len_sq(), boost::get<lua_Number>((*res)[9]));
			// isNaN
			ASSERT_EQ(v0.isNaN(), boost::get<bool>((*res)[10]));
			// isOutstanding
			ASSERT_EQ(v0.isOutstanding(), boost::get<bool>((*res)[11]));
			// saturation
			ASSERT_EQ(v0.saturation(sMin, sMax), boost::get<lua_type>((*res)[12]));
			// l_intp
			ASSERT_EQ(v0.l_intp(v1, sMin), boost::get<lua_type>((*res)[13]));
			// absolute
			ASSERT_EQ(v0.absolute(), boost::get<lua_type>((*res)[14]));
			// getMinValue
			ASSERT_EQ(v0.getMinValue(), boost::get<lua_Number>((*res)[15]));
			// getMaxValue
			ASSERT_EQ(v0.getMaxValue(), boost::get<lua_Number>((*res)[16]));
			// linearNormalization
			ASSERT_EQ(v0.linearNormalization(), boost::get<lua_type>((*res)[17]));
			// isZero
			ASSERT_EQ(v0.isZero(sMin), boost::get<bool>((*res)[18]));
		}
		// Luaでの演算チェック
		TYPED_TEST(LCV_Vector, Operator) {
			USING(lua_type);
			USING(value_t);

			// 2つのベクトル値と1つのスカラー値を生成
			value_t v0 = this->makeNonZeroVector(), v1;
			// 一定の確率でv0とv1を同じ値にする
			if(this->getRDI()({0,1}) == 0)
				v1 = v0;
			else
				v1 = this->makeNonZeroVector();

			const lua_Number s = this->makeNonZero();

			auto& lsp = this->_lsp;
			// テストコードをロードすると1つのLua関数が返る
			this->loadTestSource_Ret1(lua_OperatorCheck, LuaType::Function);

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
