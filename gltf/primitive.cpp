#include "gltf/primitive.hpp"
#include "value_loader.hpp"
#include "gltf/check.hpp"
#include "../vdecl.hpp"
#include "../gl_resource.hpp"
#include "../primitive.hpp"

#include "gltf/v1/buffer.hpp"
#include "gltf/v1/accessor.hpp"
#include "gltf/v1/v_semantic.hpp"

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
	template <class D_Accessor, class Q, class V>
	Primitive<D_Accessor, Q, V>::Primitive(const JValue& v, const Q& q):
		index(L::Optional<D_Accessor>(v, "indices", q))
	{
		const auto m = L::OptionalDefault<L::Integer>(v, "mode", 4);
		mode = CheckEnum(
					c_mode, m,
					[](auto&& c, auto&& m){ return c.first == m; }
				).second;
		using LTag = L::FindLoader_t<typename Q::Tag_t>;
		const auto attr = L::OptionalDefault<L::Dictionary<LTag>>(v, "attributes", {});
		V vsem;
		for(auto& a : attr) {
			attribute.emplace_back(
				vsem(a.first.c_str()),
				D_Accessor(a.second, q)
			);
		}
	}
	template <class D_Accessor, class Q, class V>
	bool Primitive<D_Accessor, Q, V>::CanLoad(const JValue&) noexcept {
		return true;
	}
	namespace {
		struct Visitor : boost::static_visitor<> {
			HIb				_ib;
			std::size_t		_count;

			Visitor(const HIb& ib):
				_ib(ib),
				_count(0)
			{}
			template <class T, ENABLE_IF(std::is_integral_v<T>)>
			void operator()(const std::vector<T>& t) {
				_count = t.size();
				_ib->initData(t);
			}
			template <class T, ENABLE_IF(!std::is_integral_v<T>)>
			void operator()(const std::vector<T>&) {
				D_Assert0(false);
			}
			void operator()(boost::blank) {
				D_Assert0(false);
			}
		};
	}
	template <class D_Accessor, class Q, class V>
	const HPrim& Primitive<D_Accessor, Q, V>::getPrimitive() const {
		if(!primitive_cache) {
			FWVDecl vdecl;
			std::vector<HVb> vb;
			std::size_t nV = std::numeric_limits<std::size_t>::max();
			{
				std::size_t index=0;
				std::unordered_map<HVb, std::size_t> map;

				VDecl::VDInfoV	 vdinfo;
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
				}
				vdecl = FWVDecl(vdinfo);
				vb.resize(map.size());
				for(auto& m : map) {
					vb[m.second] = m.first;
				}
			}
			if(index) {
				// make Index-buffer
				const HIb ib = mgr_gl.makeIBuffer(DrawType::Static);
				auto& idata = *(*index);

				Visitor visitor(ib);
				boost::apply_visitor(visitor, idata.getData());
				primitive_cache = ::rev::Primitive::MakeWithIndex(vdecl, mode, ib, visitor._count, 0, vb.data(), vb.size());
			} else {
				primitive_cache = ::rev::Primitive::MakeWithoutIndex(vdecl, mode, 0, nV, vb.data(), vb.size());
			}
		}
		return primitive_cache;
	}
}

template struct rev::gltf::Primitive<rev::gltf::v1::DRef_Accessor, rev::gltf::v1::IDataQuery, rev::gltf::v1::V_Semantic>;
