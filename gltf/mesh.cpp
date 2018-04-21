#include "gltf/mesh.hpp"
#include "gltf/check.hpp"
#include "../primitive.hpp"
#include "gltf/accessor.hpp"
#include "../gl_resource.hpp"
#include "../gl_buffer.hpp"
#include "../vdecl.hpp"
#include "gltf/buffer.hpp"

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
	using namespace loader;
	// --------------------------- Mesh::Primitive ---------------------------
	Mesh::Primitive::Primitive(const JValue& v):
		index(Optional<String>(v, "indices")),
		material(Required<String>(v, "material"))
	{
		const auto m = Optional<Integer>(v, "mode", 4);
		mode = CheckEnum(
					c_mode, m,
					[](auto&& c, auto&& m){ return c.first == m; }
				).second;
		const auto attr = Optional<Dictionary<String>>(v, "attributes", {});
		for(auto& a : attr) {
			attribute.emplace_back(
				*V_Semantic::FromString(a.first.c_str()),
				TagAccessor(a.second)
			);
		}
	}
	void Mesh::Primitive::resolve(const ITagQuery& q) {
		for(auto& a : attribute)
			a.second.resolve(q);
		material.resolve(q);
		if(index)
			index->resolve(q);
	}
	bool Mesh::Primitive::CanLoad(const JValue&) noexcept {
		return true;
	}
	const HPrim& Mesh::Primitive::getPrimitive() const {
		if(!primitive_cache) {
			primitive_cache = std::make_shared<::rev::Primitive>();
			auto& p = *primitive_cache;
			p.drawMode = mode;
			std::size_t nV = std::numeric_limits<std::size_t>::max();
			{
				std::size_t index=0;
				std::unordered_map<const BufferView*, std::size_t> map;

				// BufferViewがそのままVertexBufferになる
				VDecl::VDInfoV	 vdinfo;
				for(auto& a : attribute) {
					auto& acc = *(*a.second);
					std::size_t idx;
					{
						const BufferView* key = acc.bufferView.data().get();
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
				p.vdecl = std::make_shared<VDecl>(vdinfo);
				D_Assert(index <= MaxVStream, "too many vertex streams");
				for(auto& m : map) {
					p.vb[m.second] = m.first->src->getAsVb();
				}
			}
			if(index) {
				// make Index-buffer
				const HIb ib = p.ib = mgr_gl.makeIBuffer(DrawType::Static);
				auto& idata = *(*(*index));
				std::size_t count;
				idata.getData([&ib, &count](const auto& p){
					count = p->size();
					ib->initData(*p);
				});
				auto& wi = p.withIndex;
				wi.offsetElem = 0;
				wi.count = count;
			} else {
				auto& wo = p.withoutIndex;
				wo.first = 0;
				wo.count = nV;
			}
		}
		return primitive_cache;
	}
	// --------------------------- Mesh ---------------------------
	Mesh::Mesh(const JValue& v):
		Resource(v),
		primitive(Optional<Array<Primitive>>(v, "primitives", {}))
	{}
	Resource::Type Mesh::getType() const noexcept {
		return Type::Mesh;
	}
	void Mesh::resolve(const ITagQuery& q) {
		for(auto& p : primitive)
			p.resolve(q);
	}
}
