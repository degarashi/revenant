#include "gltf/v2/mesh.hpp"
#include "gltf/check.hpp"

namespace rev::gltf::v2 {
	namespace {
		struct TargetC {
			std::string		name;
			uint32_t		mask,
							index;
		};
		const TargetC c_target[] = {
			{"POSITION", 0x01, MorphTarget::Position},
			{"NORMAL", 0x02, MorphTarget::Normal},
			{"TANGENT", 0x04, MorphTarget::Tangent},
		};
	}
	namespace L = gltf::loader;
	void Primitive::_getDummyVertex(const VSemCount& vc, DummyVertexV& v) const {
		const auto add = [&v](const VSemantic vsem, auto Num){
			constexpr auto N = decltype(Num)::value;
			uint8_t data[N];
			for(auto& d : data)
				d = 255;
			v.emplace_back(
				GL_UNSIGNED_BYTE,
				vsem,
				true,
				N,
				data
			);
		};
		if(vc[VSemEnum::COLOR] == 0)
			add({VSemEnum::COLOR, 0}, lubee::SZConst<4>{});
		if(vc[VSemEnum::NORMAL] == 0)
			add({VSemEnum::NORMAL, 0}, lubee::SZConst<3>{});
		if(vc[VSemEnum::TEXCOORD] < 1)
			add({VSemEnum::TEXCOORD, 0}, lubee::SZConst<2>{});
		if(vc[VSemEnum::TEXCOORD] < 2)
			add({VSemEnum::TEXCOORD, 1}, lubee::SZConst<2>{});
	}
	// ------------------- Primitive -------------------
	Primitive::Primitive(const JValue& v, const IDataQuery& q):
		gltf::Primitive<PrimitivePolicy>(v, q),
		material(L::Optional<DRef_Material>(v, "material", q))
	{
		using u8 = uint_fast8_t;
		using s8 = int_fast8_t;
		if(const auto t_op = L::Optional<L::Array<L::Dictionary<L::Integer>>>(v, "targets")) {
			auto& t = *t_op;

			const auto nTarget = t.size();
			// [TargetFlag + Index -> DataId]
			std::vector<spi::Optional<DataId>> acc_v(nTarget * MorphTarget::_Num);
			u8 mask = 0;
			for(u8 i=0 ; i<nTarget; i++) {
				for(auto& t0 : t[i]) {
					// TargetFlagの特定
					const auto en = CheckEnum(c_target, t0.first, [](auto& c, auto& t){
						return c.name == t;
					});
					if(i==0)
						mask |= en.mask;
					acc_v[MorphTarget::_Num * i + en.index] = t0.second;
				}
			}
			{
				// Pos, Normal, Tangentの順でAttributeを割り当て
				s8 sem_cur = 0;
				for(auto& dataId : acc_v) {
					if(dataId) {
						attribute.emplace_back(
							VSemantic{VSemEnum::MORPH, int(sem_cur++)},
							Accessor_t(*dataId, q)
						);
					}
				}
			}
			Morph m;
			m.width = lubee::bit::Count(mask);
			// 存在するTargetを隙間なく敷き詰めた配列で当該Targetが何番目に位置するか
			{
				// targetIndexを決める上で使用するカウンタ
				s8 cur = 0;
				for(u8 i=0 ; i<MorphTarget::_Num ; i++) {
					if((1<<i) & mask) {
						m.targetIndex[i] = cur++;
					} else
						m.targetIndex[i] = -1;
				}
			}
			morph = m;
		}
	}
	// ------------------- Mesh -------------------
	Mesh::Mesh(const JValue& v, const IDataQuery& q):
		Resource(v),
		primitives(L::Required<L::Array<Primitive>>(v, "primitives", q))
	{
		if(const auto w = L::Optional<L::Array<L::Float>>(v, "weights")) {
			weight = std::make_shared<WeightV>(*w);
		}
	}
	Resource::Type Mesh::getType() const noexcept {
		return Type::Mesh;
	}
}
