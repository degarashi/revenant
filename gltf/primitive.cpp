#include "gltf/primitive.hpp"
#include "value_loader.hpp"
#include "gltf/check.hpp"
#include "../vdecl.hpp"
#include "../gl_resource.hpp"
#include "../primitive.hpp"

#include "gltf/v1/buffer.hpp"
#include "gltf/v1/accessor.hpp"
#include "gltf/v1/v_semantic.hpp"
#include "gltf/v1/mesh.hpp"

namespace rev::gltf {
	namespace {
		const std::pair<GLenum, DrawMode::e> c_mode[] = {
			{GL_POINTS,			DrawMode::Points},
			{GL_LINES,			DrawMode::Lines},
			{GL_LINE_LOOP,		DrawMode::LineLoop},
			{GL_LINE_STRIP,		DrawMode::LineStrip},
			{GL_TRIANGLES,		DrawMode::Triangles},
			{GL_TRIANGLE_STRIP,	DrawMode::TriangleStrip},
			{GL_TRIANGLE_FAN,	DrawMode::TriangleFan},
		};
	}
	namespace L = gltf::loader;
	template <class P>
	Primitive<P>::Primitive(const JValue& v, const Query_t& q):
		index(L::Optional<Accessor_t>(v, "indices", q))
	{
		const auto m = L::OptionalDefault<L::Integer>(v, "mode", 4);
		mode = CheckEnum(
					c_mode, m,
					[](auto&& c, auto&& m){ return c.first == m; }
				).second;
		using LTag = L::FindLoader_t<typename Query_t::Tag_t>;
		const auto attr = L::OptionalDefault<L::Dictionary<LTag>>(v, "attributes", {});
		VSem_t vsem;
		for(auto& a : attr) {
			attribute.emplace_back(
				vsem(a.first.c_str()),
				Accessor_t(a.second, q)
			);
		}
	}
	template <class P>
	bool Primitive<P>::CanLoad(const JValue&) noexcept {
		return true;
	}
	namespace {
		struct Visitor {
			HIb				_ib;
			std::size_t		_count;

			Visitor(const HIb& ib):
				_ib(ib),
				_count(0)
			{}
			template <class T, ENABLE_IF(std::is_integral_v<T>)>
			void operator()(const T* t, const std::size_t len) {
				_count = len;
				_ib->initData(t, len);
			}
			template <class T, ENABLE_IF(!std::is_integral_v<T>)>
			void operator()(const T*, std::size_t) {
				D_Assert0(false);
			}
		};
	}
	template <class P>
	const HPrim& Primitive<P>::getPrimitive() const {
		if(!primitive_cache) {
			FWVDecl vdecl;
			std::vector<HVb> vb;
			std::size_t nV = std::numeric_limits<std::size_t>::max();
			{
				std::size_t index=0;
				std::unordered_map<HVb, std::size_t> map;

				VDecl::VDInfoV		vdinfo;
				VSemCount			vc = {};
				for(auto& a : attribute) {
					auto& acc = *a.second;
					std::size_t idx;
					const auto vbp = acc.getAsVb();
					{
						HVb key = vbp.vb;
						if(const auto itr = map.find(key);
								itr != map.end())
							idx = itr->second;
						else {
							idx = index;
							map.emplace(key, index);
							++index;

							nV = std::min(nV, acc._count);
						}
					}
					vdinfo.emplace_back(idx, vbp.offset, acc._componentType, GL_FALSE, acc._nElem, a.first, acc.getByteStride());
					++vc[a.first.sem];
				}
				vb.resize(map.size());
				for(auto& m : map) {
					vb[m.second] = m.first;
				}

				std::size_t vdIdx = vb.size();
				P::VBuffModify(vc, [&vdIdx, &vdinfo, &vb, nV](const PrimitiveVertex& v) {
					HVb vb0 = mgr_gl.makeVBuffer(DrawType::Static);
					const auto unit = v.stride;
					std::vector<uint8_t> buff(unit * nV);
					auto* dst = buff.data();
					for(std::size_t i=0 ; i<nV ; i++) {
						for(std::size_t j=0 ; j<unit ; j++) {
							*dst++ = v.value[j];
						}
					}
					D_Assert0(dst == buff.data() + unit*nV);
					vdinfo.emplace_back(vdIdx++, 0, v.type, v.normalized, v.nElem, v.vsem, unit);
					vb0->initData(std::move(buff), unit);
					vb.emplace_back(vb0);
				});
				vdecl = FWVDecl(vdinfo);
			}
			if(index) {
				// make Index-buffer
				const HIb ib = mgr_gl.makeIBuffer(DrawType::Static);
				auto& idata = *(*index);

				Visitor visitor(ib);
				idata.getData(visitor);
				primitive_cache = ::rev::Primitive::MakeWithIndex(vdecl, mode, ib, visitor._count, 0, vb.data(), vb.size());
			} else {
				primitive_cache = ::rev::Primitive::MakeWithoutIndex(vdecl, mode, 0, nV, vb.data(), vb.size());
			}
		}
		return primitive_cache;
	}
}

template struct rev::gltf::Primitive<rev::gltf::v1::PrimitivePolicy>;
