#pragma once
#include "beat/pose3d.hpp"
#include "../../drawable.hpp"
#include "../../singleton_data.hpp"

namespace vertex {
	struct cube {
		frea::Vec3	pos;
		frea::Vec3	normal;

		const static rev::FWVDecl s_vdecl;
	};
}
class Cube : public beat::g3::Pose,
			public lubee::AAllocator<Cube>
{
	private:
		DefineEnum(Data,
			(Flat)
			(Gouraud)
			(Tech)
		);
		using Vec3 = frea::Vec3;
		using Flat = rev::SingletonData<rev::Primitive, Cube, Data::Flat>;
		using Gouraud = rev::SingletonData<rev::Primitive, Cube, Data::Gouraud>;
		using Tech = rev::SingletonData<rev::ITech, Cube, Data::Tech>;
		Flat	_p_flat;
		Gouraud	_p_gouraud;
		Tech	_tech;
		bool	_flat;
		const rev::HPrim& _getPrimitive() const noexcept;

	public:
		static rev::HPrim MakeData(lubee::IConst<Data::Flat>);
		static rev::HPrim MakeData(lubee::IConst<Data::Gouraud>);
		static rev::HTech MakeData(lubee::IConst<Data::Tech>);

		Cube();
		void setFlat(bool flat);
		void draw(rev::IEffect& e) const;
		void outputDrawTag(rev::DrawTag& d) const;
};
class CubeObj :
	public rev::DrawableObjT<CubeObj>,
	public Cube,
	public lubee::AAllocator<CubeObj>
{
	private:
		struct St;
	public:
		using AA = lubee::AAllocator<CubeObj>;
		using AA::NewS;
		CubeObj(const frea::Vec3& scale);

		DEF_DEBUGGUI_NAME
		DEF_DEBUGGUI_PROP
};
