#include "cube.hpp"
#include "../../vdecl.hpp"
#include "../../gl_resource.hpp"
#include "lubee/meta/countof.hpp"
#include "../../tech_pass.hpp"
#include "../../glx.hpp"
#include "../../sys_uniform.hpp"

// ---------------------- Cube頂点宣言 ----------------------
rev::HVDecl vertex::cube::MakeData(lubee::IConst<0>) {
	return rev::HVDecl{
		new rev::VDecl({
			{0,0, GL_FLOAT, GL_FALSE, 3, {rev::VSemEnum::POSITION, 0}},
			{1,0, GL_FLOAT, GL_FALSE, 3, {rev::VSemEnum::NORMAL, 0}}
		})
	};
}
const rev::SingletonData<rev::VDecl, vertex::cube, 0> vertex::cube_vdecl;

namespace {
	const static struct Pos_t {
		frea::Vec3 pos[2*2*2];
		Pos_t (){
			auto* p_pos = pos;
			for(std::size_t i=0 ; i<2 ; ++i) {
				const float y = i;
				for(std::size_t j=0 ; j<2 ; ++j) {
					const float x = j;
					for(std::size_t k=0 ; k<2 ; ++k) {
						const float z = k;
						*p_pos++ = frea::Vec3{x,y,z};
					}
				}
			}
		}
	} Pos;

	template <class T, std::size_t N>
	rev::HVb MakeVB(const T (&ar)[N]) {
		auto vb = mgr_gl.makeVBuffer(rev::DrawType::Static);
		vb->initData(ar, countof(ar), sizeof(T));
		return vb;
	}
	template <class T, std::size_t N>
	rev::HIb MakeIB(const T (&ar)[N]) {
		auto ib = mgr_gl.makeIBuffer(rev::DrawType::Static);
		ib->initData(ar, countof(ar));
		return ib;
	}

	// 大きさ1のキューブを定義
	rev::HVb MakeGouraudPosVB() {
		return MakeVB(Pos.pos);
	}
	rev::HVb MakeGouraudNormalVB() {
		frea::Vec3	nml[2*2*2];
		{
			auto* p_nml = nml;
			for(int i=0 ; i<2 ; ++i) {
				const float y = i*2-1;
				for(int j=0 ; j<2 ; ++j) {
					const float x = j*2-1;
					for(int k=0 ; k<2 ; ++k) {
						const float z = k*2-1;
						*p_nml++ = frea::Vec3{x,y,z}.normalization();
					}
				}
			}
		}
		return MakeVB(nml);
	}

	constexpr std::size_t NFace = 6;
	const uint16_t FlatIndex[NFace * 4] = {
		// Z-
		0,4,6,2,
		// X-
		1,5,4,0,
		// X+
		2,6,7,3,
		// Y+
		4,5,7,6,
		// Y-
		1,0,2,3,
		// Z+
		3,7,5,1,
	};
	rev::HVb MakeFlatPosVB() {
		frea::Vec3 vtx[NFace * 4];
		for(std::size_t i=0 ; i<NFace ; i++) {
			for(std::size_t j=0 ; j<4 ; j++) {
				vtx[i*4+j] = Pos.pos[FlatIndex[i*4+j]];
			}
		}
		return MakeVB(vtx);
	}
	rev::HVb MakeFlatNormalVB() {
		// [X+ Y+ Z+]
		const frea::Vec3 normal[6] = {
			{1,0,0}, {0,1,0}, {0,0,1},
			{-1,0,0}, {0,-1,0}, {0,0,-1}
		};
		const int index[NFace * 4] = {
			5,5,5,5,
			3,3,3,3,
			0,0,0,0,
			1,1,1,1,
			4,4,4,4,
			2,2,2,2,
		};

		frea::Vec3 nml[NFace * 4];
		for(std::size_t i=0 ; i<countof(index) ; i++) {
			nml[i] = normal[index[i]];
		}
		return MakeVB(nml);
	}
	const uint16_t FaceIndex[6] = {
		0,1,2, 2,3,0
	};
	rev::HIb MakeGouraudIB() {
		uint16_t index[countof(FaceIndex) * NFace];
		{
			auto* p_index = index;
			for(std::size_t i=0 ; i<NFace ; ++i) {
				const uint16_t* base = FlatIndex + i*4;
				for(std::size_t j=0 ; j<countof(FaceIndex) ; ++j) {
					*p_index++ = base[FaceIndex[j]];
				}
			}
		}
		return MakeIB(index);
	}
	rev::HIb MakeFlatIB() {
		uint16_t index[countof(FaceIndex) * NFace];
		{
			auto* p_index = index;
			for(std::size_t i=0 ; i<NFace ; ++i) {
				const uint16_t ofs = i*4;
				for(std::size_t j=0 ; j<countof(FaceIndex) ; ++j) {
					*p_index++ = FaceIndex[j] + ofs;
				}
			}
		}
		return MakeIB(index);
	}
}
// ----------------------- Cube -----------------------
Cube::Cube():
	_flat(false)
{}
rev::HPrim Cube::MakeData(lubee::IConst<Data::Flat>) {
	auto ret = std::make_shared<rev::Primitive>();
	ret->vdecl = vertex::cube_vdecl;
	ret->drawMode = rev::DrawMode::Triangles;

	ret->vb[0] = MakeFlatPosVB();
	ret->vb[1] = MakeFlatNormalVB();
	ret->ib = MakeFlatIB();

	auto& info = ret->withIndex;
	info.offsetElem = 0;
	info.count = NFace * countof(FaceIndex);
	return ret;
}
rev::HPrim Cube::MakeData(lubee::IConst<Data::Gouraud>) {
	auto ret = std::make_shared<rev::Primitive>();
	ret->vdecl = vertex::cube_vdecl;
	ret->drawMode = rev::DrawMode::Triangles;

	ret->vb[0] = MakeGouraudPosVB();
	ret->vb[1] = MakeGouraudNormalVB();
	ret->ib = MakeGouraudIB();

	auto& info = ret->withIndex;
	info.offsetElem = 0;
	info.count = NFace * countof(FaceIndex);
	return ret;
}
rev::HTech Cube::MakeData(lubee::IConst<Data::Tech>) {
	return mgr_gl.loadTechPass("cube.glx")->getTechnique("Cube|Default");
}
void Cube::outputDrawTag(rev::DrawTag& d) const {
	d.primitive = _getPrimitive();
}
void Cube::draw(rev::IEffect& e) const {
	e.setTechnique(_tech);
	dynamic_cast<rev::SystemUniform3D&>(e).setWorld(getToWorld().convertI<4,4>(1));
	e.setPrimitive(_getPrimitive());
	e.draw();
}
const rev::HPrim& Cube::_getPrimitive() const noexcept {
	if(_flat)
		return _p_flat;
	return _p_gouraud;
}

// ----------------------- CubeObj  -----------------------
struct CubeObj::St : StateT<St> {
	void onUpdate(CubeObj& self) override {
		self.outputDrawTag(self._dtag);
	}
	void onDraw(const CubeObj& self, rev::IEffect& e) const override {
		self.draw(e);
	}
};
CubeObj::CubeObj(const frea::Vec3& scale) {
	setAll({0,0,0}, frea::AQuat::Identity(), scale);
	setStateNew<St>();
}
#ifdef DEBUGGUI_ENABLED
const char* CubeObj::getDebugName() const noexcept {
	return "CubeObj";
}
bool CubeObj::property(const bool) {
	return false;
}
#endif
