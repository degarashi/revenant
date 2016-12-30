#include "lcv_classtest.hpp"
#include "lubee/meta/countof.hpp"

namespace rev {
	namespace test {
		template <class T>
		struct LCV_Vector : LCV_ClassTest<T> {
			using base_t = LCV_ClassTest<T>;
			using value_t = typename base_t::value_t;
			using elem_t = typename value_t::value_t;
		};
		using TypesV = ::testing::Types<
			LCVPair1<frea::Vec2>, LCVPair2<frea::AVec2, frea::Vec2>,
			LCVPair1<frea::Vec3>, LCVPair2<frea::AVec3, frea::Vec3>,
			LCVPair1<frea::Vec4>, LCVPair2<frea::AVec4, frea::Vec4>
		>;
		TYPED_TEST_CASE(LCV_Vector, TypesV);
		TYPED_TEST(LCV_Vector, Push) {
			this->pushTest();
		}
		TYPED_TEST(LCV_Vector, Type) {
			this->typeTest();
		}
		// 一旦LCValueに変換する
		TYPED_TEST(LCV_Vector, LCValue) {
			USING(lua_type);
			USING(value_t);
			lua_State* ls = this->_lsp->getLS();
			const auto val0 = GenValue<value_t>()(*this);
			LCV<value_t>()(ls, val0);
			const auto lc = LCV<LCValue>()(1, ls, nullptr);
			const auto val1 = lc.toVector<lua_type::size>();
			ASSERT_EQ(val0, val1);
		}
		// Luaでのメンバ変数読み込みチェック
		TYPED_TEST(LCV_Vector, ReadMember) {
			this->readMemberTestXYZW();
		}
		// Luaでのメンバ変数書き込みチェック
		TYPED_TEST(LCV_Vector, WriteMember) {
			this->writeMemberTestXYZW();
		}
		namespace {
			const std::string lua_Method =
				"return function(v1, ...)\n"
					"local method = {\n"
						// selectMax
						"function(v0,v1) v0:selectMax(v1) end,\n"
						// normalize
						"function(v0,v1) v0:normalize() end,\n"
						// selectMin
						"function(v0,v1) v0:selectMin(v1) end,\n"
						// linearNormalize
						"function(v0,v1) v0:linearNormalize() end\n"
					"}\n"
					"local res = {}\n"
					"for i=1,#method do\n"
						"local v0 = v1.New(...)\n"
						"method[i](v0,v1)\n"
						"res[i] = v0\n"
					"end\n"
					"return res\n"
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
		// Luaでのメンバ関数呼び出しチェック
		TYPED_TEST(LCV_Vector, Method) {
			// テストコードをロードすると1つのLua関数が返る
			this->loadTestSource_Ret1(lua_Method, LuaType::Function);

			USING(value_t);
			auto& lsp = this->_lsp;
			lua_State* ls = lsp->getLS();

			// 2つのベクトル値を生成
			const value_t v0 = this->template makeNonZeroVector<value_t>(),
							v1 = this->template makeNonZeroVector<value_t>();
			LCV<value_t>()(ls, v1);
			for(int i=0 ; i<value_t::size ; i++) {
				lsp->push(v0[i]);
			}
			lsp->call(value_t::size+1, 1);
			// テーブルが1つ返る
			Assert0(lsp->getTop() == 1 &&
					lsp->type(1) == LuaType::Table);

			USING(lua_type);
			auto res = lsp->toTable(-1);
			// selectMax
			{
				value_t v2(v0);
				v2.selectMax(v1);
				ASSERT_EQ(v2, (*res)[1].template toVector<lua_type::size>());
			}
			// normalize
			{
				value_t v2(v0);
				v2.normalize();
				ASSERT_EQ(v2, (*res)[2].template toVector<lua_type::size>());
			}
			// selectMin
			{
				value_t v2(v0);
				v2.selectMin(v1);
				ASSERT_EQ(v2, (*res)[3].template toVector<lua_type::size>());
			}
			// linearNormalize
			{
				value_t v2(v0);
				v2.linearNormalize();
				ASSERT_EQ(v2, (*res)[4].template toVector<lua_type::size>());
			}
		}
		// LuaでのConstメンバ関数呼び出しチェック
		TYPED_TEST(LCV_Vector, ConstMethod) {
			// テストコードをロードすると1つのLua関数が返る
			this->loadTestSource_Ret1(lua_ConstMethod, LuaType::Function);

			USING(value_t);
			USING(elem_t);
			// 2つのベクトル値と2つのスカラー値を生成
			value_t v0 = this->template makeNonZeroVector<value_t>(),
					v1;
			// 一定の確率でv0とv1を同じ値にする
			if(this->getRDI()({0,1}) == 0)
				v1 = v0;
			else
				v1 = this->template makeNonZeroVector<value_t>();
			lua_Number sMin = this->template makeNonZero<elem_t>(),
					   sMax = this->template makeNonZero<elem_t>();
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
			ASSERT_EQ(v0.getMin(v1), (*res)[5].template toVector<lua_type::size>());
			// getMax
			ASSERT_EQ(v0.getMax(v1), (*res)[6].template toVector<lua_type::size>());
			// normalization
			ASSERT_EQ(v0.normalization(), (*res)[7].template toVector<lua_type::size>());
			// length
			ASSERT_EQ(v0.length(), boost::get<lua_Number>((*res)[8]));
			// len_sq
			ASSERT_EQ(v0.len_sq(), boost::get<lua_Number>((*res)[9]));
			// isNaN
			ASSERT_EQ(v0.isNaN(), boost::get<bool>((*res)[10]));
			// isOutstanding
			ASSERT_EQ(v0.isOutstanding(), boost::get<bool>((*res)[11]));
			// saturation
			ASSERT_EQ(v0.saturation(sMin, sMax), (*res)[12].template toVector<lua_type::size>());
			// l_intp
			ASSERT_EQ(v0.l_intp(v1, sMin), (*res)[13].template toVector<lua_type::size>());
			// absolute
			ASSERT_EQ(v0.absolute(), (*res)[14].template toVector<lua_type::size>());
			// getMinValue
			ASSERT_EQ(v0.getMinValue(), boost::get<lua_Number>((*res)[15]));
			// getMaxValue
			ASSERT_EQ(v0.getMaxValue(), boost::get<lua_Number>((*res)[16]));
			// linearNormalization
			ASSERT_EQ(v0.linearNormalization(), (*res)[17].template toVector<lua_type::size>());
			// isZero
			ASSERT_EQ(v0.isZero(sMin), boost::get<bool>((*res)[18]));
		}
		// Luaでの演算チェック
		TYPED_TEST(LCV_Vector, Operator) {
			USING(lua_type);
			USING(value_t);
			USING(elem_t);

			// 2つのベクトル値と1つのスカラー値を生成
			value_t v0 = this->template makeNonZeroVector<value_t>(), v1;
			// 一定の確率でv0とv1を同じ値にする
			if(this->getRDI()({0,1}) == 0)
				v1 = v0;
			else
				v1 = this->template makeNonZeroVector<value_t>();

			const lua_Number s = this->template makeNonZero<elem_t>();

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
			ASSERT_EQ(lua_type(v0+v1), (*res)[1].template toVector<lua_type::size>());
			// V - V
			ASSERT_EQ(lua_type(v0-v1), (*res)[2].template toVector<lua_type::size>());
			// V * V
			ASSERT_EQ(lua_type(v0*v1), (*res)[3].template toVector<lua_type::size>());
			// V * s
			ASSERT_EQ(lua_type(v0*s), (*res)[4].template toVector<lua_type::size>());
			// V / V
			ASSERT_EQ(lua_type(v0/v1), (*res)[5].template toVector<lua_type::size>());
			// V / s
			ASSERT_EQ(lua_type(v0/s), (*res)[6].template toVector<lua_type::size>());
			// #V
			ASSERT_EQ(lua_type(v0).length(), boost::get<lua_Number>((*res)[7]));
			// -V
			ASSERT_EQ(-lua_type(v0), (*res)[8].template toVector<lua_type::size>());
			// V == V
			ASSERT_EQ(v0==v1, boost::get<bool>((*res)[9]));
			// tostring(V)
			ASSERT_EQ(lubee::ToString(v0), (*res)[10].toString());
		}
	}
}
