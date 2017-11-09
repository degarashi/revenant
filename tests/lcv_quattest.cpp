#include "lcv_classtest.hpp"
#include "frea/constant.hpp"

namespace rev {
	namespace test {
		template <class T>
		struct LCV_Quat : LCV_ClassTest<T> {
			using base_t = LCV_ClassTest<T>;
			using value_t = typename base_t::value_t;
			using elem_t = typename value_t::value_t;
			using vec3_t = typename value_t::vec_t;
			using vec4_t = typename value_t::vec4_t;
			using mat3_t = typename value_t::mat3_t;

			void SetUp() override {
				base_t::SetUp();
				auto& lsc = *this->_lsp;
				LuaImport::RegisterClass<vec3_t>(lsc);
				LuaImport::RegisterClass<vec4_t>(lsc);
				LuaImport::RegisterClass<frea::RadF>(lsc);
				LuaImport::RegisterClass<Mat3>(lsc);
				LuaImport::RegisterClass<Mat4>(lsc);
				LuaImport::RegisterClass<frea::ExpQuat>(lsc);
			}
		};
		using TypesQ = ::testing::Types<
			LCVPair1<frea::Quat>, LCVPair2<frea::AQuat, frea::Quat>
		>;
		TYPED_TEST_CASE(LCV_Quat, TypesQ);
		TYPED_TEST(LCV_Quat, Push) {
			this->pushTest();
		}
		TYPED_TEST(LCV_Quat, Type) {
			this->typeTest();
		}
		TYPED_TEST(LCV_Quat, ReadMember) {
			this->readMemberTestXYZW();
		}
		TYPED_TEST(LCV_Quat, WriteMember) {
			this->writeMemberTestXYZW();
		}
		namespace {
			const std::string lua_StaticMethod =
				"return function(ax, ay, az, m3, ang0, ang1, ang2, targetFlag, baseFlag, vat, vpos)\n"
					"return {\n"
						// Identity
						"Quat.Identity(),\n"
						// FromAxisF
						"Quat.FromAxisF(ax.x, ay.x, az.x, ax.y, ay.y, az.y, ax.z, ay.z, az.z),\n"
						// FromMat
						"Quat.FromMat(m3),\n"
						// FromAxis
						"Quat.FromAxis(ax, ay, az),\n"
						// FromMatAxis
						"Quat.FromMatAxis(ax, ay, az),\n"
						// RotationYPR
						"Quat.RotationYPR(ang0, ang1, ang2),\n"
						// RotationX
						"Quat.RotationX(ang0),\n"
						// RotationY
						"Quat.RotationY(ang1),\n"
						// RotationZ
						"Quat.RotationZ(ang2),\n"
						// LookAt
						"Quat.LookAt(az, ay),\n"
						// SetLookAt
						"Quat.SetLookAt(targetFlag, baseFlag, ay, vat, vpos),\n"
						// Lua_Rotation
						"Quat.Lua_Rotation(ax, ang0),\n"
						// Lua_RotationFromTo
						"Quat.Lua_RotationFromTo(ax, ay)\n"
					"}\n"
				"end"
			;
			const std::string lua_ConstMethod =
				"return function(q0, q1, vdir0, vdir1, s, ang)\n"
					"return {\n"
						// scale
						"q0:scale(s),\n"
						// rotationX
						"q0:rotationX(ang),\n"
						// rotationY
						"q0:rotationY(ang),\n"
						// rotationZ
						"q0:rotationZ(ang),\n"
						// rotation
						"q0:rotation(vdir0, ang),\n"
						// asVec4
						"q0:asVec4(),\n"
						// normalization
						"q0:normalization(),\n"
						// conjugation
						"q0:conjugation(),\n"
						// inversion
						"q0:inversion(),\n"
						// len_sq
						"q0:len_sq(),\n"
						// length
						"q0:length(),\n"
						// angle
						"q0:angle(),\n"
						// getVector
						"q0:getVector(),\n"
						// getAxis
						"q0:getAxis(),\n"
						// dot
						"q0:dot(q1),\n"
						// slerp
						"q0:slerp(q1, s),\n"
						// getXAxis
						"q0:getXAxis(),\n"
						// getXAxisInv
						"q0:getXAxisInv(),\n"
						// getYAxis
						"q0:getYAxis(),\n"
						// getYAxisInv
						"q0:getYAxisInv(),\n"
						// getZAxis
						"q0:getZAxis(),\n"
						// getZAxisInv
						"q0:getZAxisInv(),\n"
						// getRight
						"q0:getRight(),\n"
						// getUp
						"q0:getUp(),\n"
						// getDir
						"q0:getDir(),\n"
						// asMat33
						"q0:asMat33(),\n"
						// asMat44
						"q0:asMat44(),\n"
						// distance
						"q0:distance(q1),\n"
						// asExpQuat
						"q0:asExpQuat(),\n"
						// luaRotation
						"q0:luaRotation(vdir0, ang),\n"
						// luaRotationFromTo
						"q0:luaRotationFromTo(vdir0, vdir1),\n"
						// luaAddQ
						"q0:luaAddQ(q1),\n"
						// luaSubQ
						"q0:luaSubQ(q1),\n"
						// luaMulQ
						"q0:luaMulQ(q1),\n"
						// luaMulF
						"q0:luaMulF(s),\n"
						// luaDivF
						"q0:luaDivF(s),\n"
						// luaEqual
						"q0:luaEqual(q1),\n"
						// luaToString
						"q0:luaToString()\n"
					"}\n"
				"end"
			;
			const std::string lua_Method =
				"return function(q1, vdir, ang, ...)\n"
					"local method = {\n"
						// rotateX
						"function(q0) q0:rotateX(ang) end,\n"
						// rotateY
						"function(q0) q0:rotateY(ang) end,\n"
						// rotateZ
						"function(q0) q0:rotateZ(ang) end,\n"
						// rotate
						"function(q0) q0:rotate(vdir, ang) end,\n"
						// identity
						"function(q0) q0:identity() end,\n"
						// normalize
						"function(q0) q0:normalize() end,\n"
						// conjugate
						"function(q0) q0:conjugate() end,\n"
						// invert
						"function(q0) q0:invert() end\n"
					"}\n"
					"local res = {}\n"
					"for i=1,#method do\n"
						"local q0 = q1.New(...)\n"
						"method[i](q0)\n"
						"res[i] = q0\n"
					"end\n"
					"return res\n"
				"end"
			;
		}
		// Luaでのstaticメンバ関数呼び出しチェック
		TYPED_TEST(LCV_Quat, StaticMethod) {
			// テストコードをロードすると1つのLua関数が返る
			this->loadTestSource_Ret1(lua_StaticMethod, LuaType::Function);

			USING(elem_t);
			USING(vec3_t);
			auto& mt = this->mt();
			auto mtf = mt.template getUniformF<elem_t>();

			// 方向ベクトル3つ
			constexpr elem_t Th = 1e-3;
			const auto axis = frea::random::GenVecUnitN<vec3_t>(mtf, 3, Th);

			// 角度3つ
			frea::RadF ang[3];
			for(auto& a : ang)
				a = frea::random::GenAngle<frea::RadF>(mtf);

			// 座標ベクトル2つ
			constexpr lubee::Range<elem_t>	PosRange{-1e3, 1e3};
			auto mtf_pos = mt.template getUniformF<elem_t>(PosRange);
			const auto pos = frea::random::GenVecN<vec3_t>(mtf_pos, 2, Th);

			// 軸フラグ2つ
			constexpr lubee::Range<int> FlagRange{0, frea::Axis::_Num-1};
			auto& rdi = this->getRDI();
			using Axis_e = frea::Axis::e;
			Axis_e flag[2] = {
				static_cast<Axis_e>(rdi(FlagRange)),
				static_cast<Axis_e>(rdi(FlagRange))
			};
			while(flag[0] == flag[1])
				flag[1] = static_cast<Axis_e>((flag[1]+1) % frea::Axis::_Num);

			// 任意の回転行列
			USING(mat3_t);
			auto mRot = mat3_t::RotationAxis(axis[0], ang[0]);

			const GetLCVType<vec3_t>		lcv_v;
			const GetLCVType<Mat3>			lcv_m;
			const GetLCVType<frea::RadF>	lcv_a;

			auto& lsp = this->_lsp;
			lua_State* ls = lsp->getLS();
			for(auto& a : axis)
				lcv_v(ls, a);
			lcv_m(ls, mRot);
			for(auto& a : ang)
				lcv_a(ls, a);
			for(auto& f : flag)
				lsp->push(f);
			for(auto& p : pos)
				lcv_v(ls, p);

			lsp->call(11,1);
			// テーブルが1つ返る
			Assert0(lsp->getTop() == 1 &&
					lsp->type(1) == LuaType::Table);
			auto res = lsp->toTable(-1);

			USING(value_t);
			USING(lua_type);
			ASSERT_EQ(value_t::Identity(), boost::get<lua_type>((*res)[1]));
			ASSERT_EQ(value_t::FromAxisF(axis[0].x, axis[1].x, axis[2].x,
										axis[0].y, axis[1].y, axis[2].y,
										axis[0].z, axis[1].z, axis[2].z),
						boost::get<lua_type>((*res)[2]));
			ASSERT_EQ(value_t::FromMat(mRot), boost::get<lua_type>((*res)[3]));
			ASSERT_EQ(value_t::FromAxis(axis[0], axis[1], axis[2]), boost::get<lua_type>((*res)[4]));
			ASSERT_EQ(value_t::FromMatAxis(axis[0], axis[1], axis[2]), boost::get<lua_type>((*res)[5]));
			ASSERT_EQ(value_t::RotationYPR(ang[0], ang[1], ang[2]), boost::get<lua_type>((*res)[6]));
			ASSERT_EQ(value_t::RotationX(ang[0]), boost::get<lua_type>((*res)[7]));
			ASSERT_EQ(value_t::RotationY(ang[1]), boost::get<lua_type>((*res)[8]));
			ASSERT_EQ(value_t::RotationZ(ang[2]), boost::get<lua_type>((*res)[9]));
			ASSERT_EQ(value_t::LookAt(axis[2], axis[1]), boost::get<lua_type>((*res)[10]));
			ASSERT_EQ(value_t::SetLookAt(flag[0], flag[1], axis[1], pos[0], pos[1]), boost::get<lua_type>((*res)[11]));
			ASSERT_EQ(value_t::Lua_Rotation(axis[0], ang[0]), boost::get<lua_type>((*res)[12]));
			ASSERT_EQ(value_t::Lua_RotationFromTo(axis[0], axis[1]), boost::get<lua_type>((*res)[13]));
		}
		// Luaでのメンバ関数呼び出しチェック
		TYPED_TEST(LCV_Quat, Method) {
			// テストコードをロードすると1つのLua関数が返る
			this->loadTestSource_Ret1(lua_Method, LuaType::Function);

			USING(value_t);
			USING(vec3_t);
			USING(lua_type);
			auto& lsp = this->_lsp;
			lua_State* ls = lsp->getLS();
			// 2つのQuat, 方向ベクトル、角度を生成
			const GenValue_t<value_t> gv_q;
			const value_t q[2] = {
				gv_q(*this),
				gv_q(*this)
			};
			GetLCVType<value_t>()(ls, q[1]);

			const vec3_t dir = this->template makeNonZeroVector<vec3_t>().normalization();
			GetLCVType<vec3_t>()(ls, dir);

			const frea::RadF ang = GenValue_t<frea::RadF>()(*this);
			GetLCVType<frea::RadF>()(ls, ang);
			for(int i=0 ; i<value_t::size ; i++) {
				lsp->push(q[0][i]);
			}
			lsp->call(3+4, 1);

			// テーブルが1つ返る
			Assert0(lsp->getTop() == 1 &&
					lsp->type(1) == LuaType::Table);
			auto res = lsp->toTable(-1);
			int cur = 0;
			const auto chk = [&q, &cur, &res](const auto& proc){
				value_t q2(q[0]);
				proc(q2);
				ASSERT_EQ(q2, boost::get<lua_type>((*res)[++cur]));
			};
			ASSERT_NO_FATAL_FAILURE(chk([&](auto& q){ q.rotateX(ang); }));
			ASSERT_NO_FATAL_FAILURE(chk([&](auto& q){ q.rotateY(ang); }));
			ASSERT_NO_FATAL_FAILURE(chk([&](auto& q){ q.rotateZ(ang); }));
			ASSERT_NO_FATAL_FAILURE(chk([&](auto& q){ q.rotate(dir, ang); }));
			ASSERT_NO_FATAL_FAILURE(chk([&](auto& q){ q.identity(); }));
			ASSERT_NO_FATAL_FAILURE(chk([&](auto& q){ q.normalize(); }));
			ASSERT_NO_FATAL_FAILURE(chk([&](auto& q){ q.conjugate(); }));
			ASSERT_NO_FATAL_FAILURE(chk([&](auto& q){ q.invert(); }));
		}
		// LuaでのConstメンバ関数呼び出しチェック
		TYPED_TEST(LCV_Quat, ConstMethod) {
			// テストコードをロードすると1つのLua関数が返る
			this->loadTestSource_Ret1(lua_ConstMethod, LuaType::Function);

			USING(vec3_t);
			USING(value_t);
			USING(elem_t);
			// 2つのQuatと2つの方向ベクトル、スカラー値、角度を生成
			const GenValue_t<value_t> gv_q;
			const value_t q[2] = {
				gv_q(*this),
				gv_q(*this)
			};
			const vec3_t dir[2] = {
				this->template makeNonZeroVector<vec3_t>().normalization(),
				this->template makeNonZeroVector<vec3_t>().normalization()
			};
			const elem_t s = GenValue_t<elem_t>()(*this);
			const frea::RadF ang = GenValue_t<frea::RadF>()(*this);

			// テスト引数を積む
			auto& lsp = this->_lsp;
			lua_State* ls = lsp->getLS();
			for(auto& qa : q)
				GetLCVType<value_t>()(ls, qa);
			for(auto& da : dir)
				GetLCVType<vec3_t>()(ls, da);
			GetLCVType<elem_t>()(ls, s);
			GetLCVType<frea::RadF>()(ls, ang);
			// 演算テストコードを実行
			lsp->call(6, 1);
			// テーブルが1つ返る
			Assert0(lsp->getTop() == 1 &&
					lsp->type(1) == LuaType::Table);

			USING(lua_type);
			// Lua内での演算結果をC++側と比較
			auto res = lsp->toTable(1);
			// scale
			ASSERT_EQ(lua_type(q[0].scale(s)), boost::get<lua_type>((*res)[1]));
			// rotationX
			ASSERT_EQ(lua_type(q[0].rotationX(ang)), boost::get<lua_type>((*res)[2]));
			// rotationY
			ASSERT_EQ(lua_type(q[0].rotationY(ang)), boost::get<lua_type>((*res)[3]));
			// rotationZ
			ASSERT_EQ(lua_type(q[0].rotationZ(ang)), boost::get<lua_type>((*res)[4]));
			// rotation
			ASSERT_EQ(lua_type(q[0].rotation(dir[0], ang)), boost::get<lua_type>((*res)[5]));
			// asVec4
			ASSERT_EQ(lua_type(q[0].asVec4()), (*res)[6].template toVector<4>());
			// normalization
			ASSERT_EQ(lua_type(q[0].normalization()), boost::get<lua_type>((*res)[7]));
			// conjugation
			ASSERT_EQ(lua_type(q[0].conjugation()), boost::get<lua_type>((*res)[8]));
			// inversion
			ASSERT_EQ(lua_type(q[0].inversion()), boost::get<lua_type>((*res)[9]));
			// len_sq
			ASSERT_EQ(lua_Number(q[0].len_sq()), boost::get<lua_Number>((*res)[10]));
			// length
			ASSERT_EQ(lua_Number(q[0].length()), boost::get<lua_Number>((*res)[11]));
			// angle
			ASSERT_EQ(q[0].angle(), boost::get<frea::RadF>((*res)[12]));
			// getVector
			ASSERT_EQ(frea::Vec3(q[0].getVector()), (*res)[13].template toVector<3>());
			// getAxis
			ASSERT_EQ(frea::Vec3(q[0].getAxis()), (*res)[14].template toVector<3>());
			// dot
			ASSERT_EQ(lua_Number(q[0].dot(q[1])), boost::get<lua_Number>((*res)[15]));
			// slerp
			ASSERT_EQ(lua_type(q[0].slerp(q[1], s)), boost::get<lua_type>((*res)[16]));
			// getXAxis
			ASSERT_EQ(frea::Vec3(q[0].getXAxis()), (*res)[17].template toVector<3>());
			// getXAxisInv
			ASSERT_EQ(frea::Vec3(q[0].getXAxisInv()), (*res)[18].template toVector<3>());
			// getYAxis
			ASSERT_EQ(frea::Vec3(q[0].getYAxis()), (*res)[19].template toVector<3>());
			// getYAxisInv
			ASSERT_EQ(frea::Vec3(q[0].getYAxisInv()), (*res)[20].template toVector<3>());
			// getZAxis
			ASSERT_EQ(frea::Vec3(q[0].getZAxis()), (*res)[21].template toVector<3>());
			// getZAxisInv
			ASSERT_EQ(frea::Vec3(q[0].getZAxisInv()), (*res)[22].template toVector<3>());
			// getRight
			ASSERT_EQ(frea::Vec3(q[0].getRight()), (*res)[23].template toVector<3>());
			// getUp
			ASSERT_EQ(frea::Vec3(q[0].getUp()), (*res)[24].template toVector<3>());
			// getDir
			ASSERT_EQ(frea::Vec3(q[0].getDir()), (*res)[25].template toVector<3>());
			// asMat33
			ASSERT_EQ(q[0].asMat33(), ((*res)[26].template toMatrix<3,3>()));
			// asMat44
			ASSERT_EQ(q[0].asMat44(), ((*res)[27].template toMatrix<4,4>()));
			// distance
			ASSERT_EQ(lua_Number(q[0].distance(q[1])), boost::get<lua_Number>((*res)[28]));
			// asExpQuat
			ASSERT_EQ(q[0].asExpQuat(), boost::get<frea::ExpQuat>((*res)[29]));
			// luaRotation
			ASSERT_EQ(lua_type(q[0].luaRotation(dir[0], ang)), boost::get<lua_type>((*res)[30]));
			// luaRotationFromTo
			ASSERT_EQ(lua_type(q[0].luaRotationFromTo(dir[0], dir[1])), boost::get<lua_type>((*res)[31]));
			// luaAddQ
			ASSERT_EQ(lua_type(q[0].luaAddQ(q[1])), boost::get<lua_type>((*res)[32]));
			// luaSubQ
			ASSERT_EQ(lua_type(q[0].luaSubQ(q[1])), boost::get<lua_type>((*res)[33]));
			// luaMulQ
			ASSERT_EQ(lua_type(q[0].luaMulQ(q[1])), boost::get<lua_type>((*res)[34]));
			// luaMulF
			ASSERT_EQ(lua_type(q[0].luaMulF(s)), boost::get<lua_type>((*res)[35]));
			// luaDivF
			ASSERT_EQ(lua_type(q[0].luaDivF(s)), boost::get<lua_type>((*res)[36]));
			// luaEqual
			ASSERT_EQ(q[0].luaEqual(q[1]), boost::get<bool>((*res)[37]));
			// luaToString
			ASSERT_EQ(q[0].luaToString(), (*res)[38].toString());
		}
	}
}
