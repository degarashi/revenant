#include "gltf/primitive.hpp"
#include "value_loader.hpp"
#include "gltf/check.hpp"
#include "../gl_resource.hpp"
#include "../primitive.hpp"

#include "gltf/v1/buffer.hpp"
#include "gltf/v1/accessor.hpp"
#include "gltf/v1/v_semantic.hpp"
#include "gltf/v1/mesh.hpp"

#include "mk.hpp"
#include <numeric>

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
		struct IB_Visitor {
			HIb				_ib;
			std::size_t		_count;

			IB_Visitor(const HIb& ib):
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
		std::vector<int32_t> MakeSequence(const std::size_t n) {
			std::vector<int32_t> ret(n);
			std::iota(ret.begin(), ret.end(), 0);
			return ret;
		}
	}
	template <class P>
	const HPrim& Primitive<P>::getPrimitiveWithTangent() const {
		if(!cache.tangent) {
			struct TangentInput {
				const Accessor_t	*pos,
									*normal,
									*uv;

				bool canGenerate() const noexcept {
					return pos && normal && uv;
				}
			} t_input = {};
			VSemCount	vc = {};
			bool hasTangent = false;
			for(auto& a : attribute) {
				++vc[a.first.sem];
				if(a.first == VSemantic{VSemEnum::POSITION, 0})
					t_input.pos = &a.second;
				else if(a.first == VSemantic{VSemEnum::NORMAL, 0})
					t_input.normal = &a.second;
				else if(a.first == VSemantic{VSemEnum::TEXCOORD, 0})
					t_input.uv = &a.second;
				else if(a.first == VSemantic{VSemEnum::TANGENT, 0})
					hasTangent = true;
			}
			// 既にtangentベクトルを持っていたら通常のprimitive生成
			if(hasTangent) {
				cache.tangent = getPrimitive();
				return *cache.tangent;
			}

			if(!(mode == DrawMode::Triangles) ||
				!t_input.canGenerate())
			{
				// 生成できないのでnullをセット
				cache.tangent = nullptr;
				return *cache.tangent;
			}

			MKInput mki(
				index ? (*index)->cnvToInt32() : MakeSequence((*t_input.pos)->_count),
				(*t_input.pos)->cnvToFloat(),
				(*t_input.normal)->cnvToFloat(),
				(*t_input.uv)->cnvToFloat()
			);
			mki.calcResult();
			const auto& dupl = mki.getDuplInfo();
			const auto nV = dupl.postLen;
			ByteV	vstream;
			struct VEnt {
				VSemantic		vsem;
				std::size_t		offset,
								nElem;
				GLenum			type;
				bool			normalize;
			};
			using VData = std::vector<VEnt>;
			VData vdata;
			// 全ての頂点情報を列挙して再構築(一本のストリームにする)
			for(auto& a : attribute) {
				auto& acc = *a.second;
				const auto unit = acc.getUnitSize();
				const auto nelem = acc.getActualNElem();
				const auto elemSize = unit * nelem;

				ByteV ar(acc._count * elemSize);
				auto dst = reinterpret_cast<uintptr_t>(ar.data());
				auto itr = acc.begin(),
					 itrE = acc.end();
				while(*itr != *itrE) {
					dst += itr->readAndIncrement(dst);
				}
				D_Assert0(dst == reinterpret_cast<uintptr_t>(ar.data())+ar.size());

				ar.resize(nV * elemSize);
				for(auto& cp : dupl.copy) {
					std::memcpy(ar.data() + elemSize * cp.to,
								ar.data() + elemSize * cp.from,
								elemSize);
				}
				const auto prevLen = vstream.size();
				vdata.emplace_back(VEnt{
					.vsem = a.first,
					.offset = prevLen,
					.nElem = nelem,
					.type = acc._componentType,
					.normalize = acc.isNormalized()
				});
				vstream.resize(prevLen + ar.size());
				std::memcpy(vstream.data()+prevLen,
							ar.data(),
							ar.size());
			}
			// tangentを付加
			{
				const auto& tan_v = mki.getTangent();
				constexpr auto TSize = frea::Vec4::size;
				std::vector<float>	tanV(nV*TSize);
				const auto copyLen = dupl.copy.size();
				D_Assert0(tan_v.size() == copyLen);
				for(std::size_t i=0 ; i<copyLen ; i++) {
					auto& cp = dupl.copy[i];
					D_Assert0(cp.from < cp.to || cp.from < dupl.prevLen);
					auto& tan = tan_v[i];
					for(std::size_t j=0 ; j<TSize ; j++)
						tanV[cp.to*TSize+j] = tan[j];
				}
				vdata.emplace_back(VEnt{
					.vsem = VSemantic{VSemEnum::TANGENT, 0},
					.offset = vstream.size(),
					.nElem = TSize,
					.type = GL_FLOAT,
					.normalize = false
				});
				const auto prevLen = vstream.size();
				vstream.resize(prevLen + tanV.size()*sizeof(float));
				std::memcpy(vstream.data()+prevLen,
							tanV.data(),
							tanV.size()*sizeof(float));
			}
			// make vertex buffer
			VDecl::VDInfoV		vdinfo;
			for(auto& v : vdata) {
				vdinfo.emplace_back(0, v.offset, v.type, v.normalize, v.nElem, v.vsem, v.nElem*GLFormat::QuerySize(v.type));
			}
			HVb vb[2];
			vb[0] = mgr_gl.makeVBuffer(DrawType::Static);
			vb[0]->initData(std::move(vstream), 0);
			P::VBuffDummy(vc, [&vdinfo, &vb=vb[1], nV](const DummyVertexV& v) {
				vb = ProcDummyVertex(vdinfo, 1, nV, v);
			});
			// make index buffer
			const HIb ib = mgr_gl.makeIBuffer(DrawType::Static);
			const auto idxLen = dupl.index.size();
			ib->initData(std::move(dupl.index));
			cache.tangent = ::rev::Primitive::MakeWithIndex(
								FWVDecl(vdinfo),
								DrawMode::Triangles,
								ib,
								idxLen,
								0,
								vb,
								vb[1] ? 2 : 1
							);
		}
		return *cache.tangent;
	}
	template <class P>
	HVb Primitive<P>::ProcDummyVertex(VDecl::VDInfoV& vdinfo, const std::size_t streamId, const std::size_t nV, const DummyVertexV& v) {
		if(v.empty())
			return nullptr;
		std::size_t stride = 0;
		for(auto& v0 : v)
			stride += v0.stride;

		std::vector<uint8_t> unit(stride);
		auto* dst = unit.data();
		std::size_t ofs = 0;
		for(auto& v0 : v) {
			const std::size_t size = v0.stride;
			for(std::size_t i=0 ; i<size ; i++)
				*dst++ = v0.value[i];
			vdinfo.emplace_back(streamId, ofs, v0.type, v0.normalized, v0.nElem, v0.vsem, stride);
			ofs += size;
		}
		D_Assert0(dst == unit.data() + stride);

		std::vector<uint8_t> buff(stride * nV);
		for(std::size_t i=0 ; i<nV ; i++) {
			std::memcpy(buff.data()+i*stride, unit.data(), stride);
		}

		HVb vb = mgr_gl.makeVBuffer(DrawType::Static);
		vb->initData(std::move(buff), stride);
		return vb;
	}
	template <class P>
	dc::BBox_Op Primitive<P>::getBBox() const {
		if(!cache.bbox) {
			const auto itr = std::find_if(attribute.begin(), attribute.end(), [](auto& a){
				return a.first==VSemantic{VSemEnum::POSITION, 0};
			});
			if(itr == attribute.end())
				throw std::runtime_error("can't calculate bounding-box (has not POSITION)");
			auto& acc = itr->second;
			if(acc->getActualNElem() != 3)
				throw std::runtime_error("position accessor's: nElem is not 3");

			cache.bbox = acc->getBBox();
			if(!cache.bbox) {
				cache.bbox = dc::BBox{};
				cache.noBBox = true;
			}
			cache.noBBox = false;
		}
		if(cache.noBBox)
			return spi::none;
		return cache.bbox;
	}
	template <class P>
	const HPrim& Primitive<P>::getPrimitive() const {
		if(!cache.normal) {
			FWVDecl vdecl;
			std::vector<HVb> vb;
			std::size_t nV = std::numeric_limits<std::size_t>::max();
			{
				std::size_t attrIndex=0;
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
							idx = attrIndex;
							map.emplace(key, attrIndex);
							++attrIndex;

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

				P::VBuffDummy(vc, [&vdinfo, &vb, nV](const DummyVertexV& v) {
					if(auto vb0 = ProcDummyVertex(vdinfo, vb.size(), nV, v))
						vb.emplace_back(vb0);
				});
				vdecl = FWVDecl(vdinfo);
			}
			if(index) {
				// make Index-buffer
				const HIb ib = mgr_gl.makeIBuffer(DrawType::Static);
				auto& idata = *(*index);

				IB_Visitor visitor(ib);
				idata.getData(visitor);
				cache.normal = ::rev::Primitive::MakeWithIndex(vdecl, mode, ib, visitor._count, 0, vb.data(), vb.size());
			} else {
				cache.normal = ::rev::Primitive::MakeWithoutIndex(vdecl, mode, 0, nV, vb.data(), vb.size());
			}
		}
		return cache.normal;
	}
}

template struct rev::gltf::Primitive<rev::gltf::v1::PrimitivePolicy>;
