#include "gltf/primitive.hpp"
#include "value_loader.hpp"
#include "gltf/check.hpp"
#include "../glx_const.hpp"
#include "../vdecl.hpp"
#include "../gl_resource.hpp"
#include "../primitive.hpp"

#include "gltf/v1/buffer.hpp"
#include "gltf/v1/accessor.hpp"

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
	template <class D_Accessor, class Q>
	Primitive<D_Accessor, Q>::Primitive(const JValue& v, const Q& q):
		index(L::Optional<D_Accessor>(v, "indices", q))
	{
		const auto m = L::OptionalDefault<L::Integer>(v, "mode", 4);
		mode = CheckEnum(
					c_mode, m,
					[](auto&& c, auto&& m){ return c.first == m; }
				).second;
		const auto attr = L::OptionalDefault<L::Dictionary<L::String>>(v, "attributes", {});
		for(auto& a : attr) {
			attribute.emplace_back(
				*V_Semantic::FromString(a.first.c_str()),
				D_Accessor(a.second, q)
			);
		}
	}
	template <class D_Accessor, class Q>
	bool Primitive<D_Accessor, Q>::CanLoad(const JValue&) noexcept {
		return true;
	}
	template <class D_Accessor, class Q>
	const HPrim& Primitive<D_Accessor, Q>::getPrimitive() const {
		if(!primitive_cache) {
			FWVDecl vdecl;
			HVb vb[MaxVStream];
			std::size_t nVb;
			std::size_t nV = std::numeric_limits<std::size_t>::max();
			{
				std::size_t index=0;
				using BView = decltype(attribute[0].second->bufferView.data());
				std::unordered_map<BView, std::size_t> map;

				// BufferViewがそのままVertexBufferになる
				VDecl::VDInfoV	 vdinfo;
				for(auto& a : attribute) {
					auto& acc = *a.second;
					std::size_t idx;
					{
						BView key = acc.bufferView.data();
						if(const auto itr = map.find(key);
								itr != map.end())
							idx = itr->second;
						else {
							idx = index;
							map.emplace(key, index);
							++index;

							nV = std::min(nV, acc.count);
						}
					}
					vdinfo.emplace_back(idx, acc.byteOffset + acc.bufferView->byteOffset, acc.componentType, GL_FALSE, acc.nElem, a.first, acc.byteStride);
				}
				vdecl = FWVDecl(vdinfo);
				D_Assert(index <= MaxVStream, "too many vertex streams");
				for(auto& m : map) {
					vb[m.second] = m.first->src->getAsVb();
				}
				nVb = map.size();
			}
			if(index) {
				// make Index-buffer
				const HIb ib = mgr_gl.makeIBuffer(DrawType::Static);
				auto& idata = *(*index);
				std::size_t count;
				idata.getData([&ib, &count](const auto& p){
					count = p->size();
					ib->initData(*p);
				});
				primitive_cache = ::rev::Primitive::MakeWithIndex(vdecl, mode, ib, count, 0, vb, nVb);
			} else {
				primitive_cache = ::rev::Primitive::MakeWithoutIndex(vdecl, mode, 0, nV, vb, nVb);
			}
		}
		return primitive_cache;
	}
}

template struct rev::gltf::Primitive<rev::gltf::v1::DRef_Accessor, rev::gltf::v1::IDataQuery>;
