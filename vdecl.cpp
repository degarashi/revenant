#include "vdecl.hpp"
#include "gl_error.hpp"
#include "gl_format.hpp"
#include "gl_if.hpp"
#include "lubee/meta/countof.hpp"
#include "drawcmd/queue_if.hpp"

namespace rev {
	// ----------------- VDecl::VDInfo -----------------
	VDecl::VDInfo::VDInfo(const GLuint streamId, const GLuint offset, const GLuint elemFlag,
						const GLuint bNormalize, const GLuint elemSize,
						const VSemantic sem, const GLuint strideOvr):
		streamId(streamId),
		offset(offset),
		elemFlag(elemFlag),
		bNormalize(bNormalize),
		elemSize(elemSize),
		sem(sem),
		strideOvr(strideOvr)
	{}
	bool VDecl::VDInfo::operator == (const VDInfo& v) const {
		if( ((streamId ^ v.streamId)
			| (offset ^ v.offset)
			| (elemFlag ^ v.elemFlag)
			| (bNormalize ^ v.bNormalize)
			| (elemSize ^ v.elemSize)
			| (strideOvr ^ v.strideOvr)) == 0)
		{
			return sem == v.sem;
		}
		return false;
	}
	bool VDecl::VDInfo::operator != (const VDInfo& v) const {
		return !(this->operator == (v));
	}
	// ----------------- VDecl -----------------
	VDecl::VDecl(const VDInfoV& vl) {
		_vdInfo = vl;
		_init();
	}
	// 一旦Vectorに変換
	VDecl::VDecl(const std::initializer_list<VDInfo> il):
		VDecl(VDInfoV(il.begin(), il.end()))
	{}
	void VDecl::_init() {
		// StreamId毎に集計
		VDInfoV stream[MaxVStream];
		for(auto& v : _vdInfo)
			stream[v.streamId].push_back(v);

		// 頂点定義のダブり確認
		for(auto& t : stream) {
			if(!t.empty()) {
				// オフセットでソート
				std::sort(t.begin(), t.end(), [](const auto& v0, const auto& v1) { return v0.offset < v1.offset; });

				unsigned int tail = 0;
				for(auto& t2 : t) {
					if(tail > t2.offset)
						throw GLE_Error("invalid vertex offset");
					tail = t2.offset + GLFormat::QuerySize(t2.elemFlag) * t2.elemSize;
				}
			}
		}

		_setter.resize(_vdInfo.size());
		std::size_t cur = 0;
		for(std::size_t i=0 ; i<countof(stream) ; i++) {
			_streamOfs[i] = cur;
			for(auto& t2 : stream[i]) {
				_setter[cur] = [t2](draw::IQueue& q, const GLuint vb_stride, const VSem_AttrV& attr) {
					// Stride-Overrideが0の時はVBufferから提供されたStrideを使う
					const auto stride = (t2.strideOvr==0) ? vb_stride : t2.strideOvr;
					D_Assert0(stride > 0);
					// Semanticを線形探索
					const auto itr =
						std::find_if(
							attr.cbegin(),
							attr.cend(),
							[s=t2.sem](const auto& a){
								return a.sem == s;
							}
						);
					if(itr == attr.cend())
						return;
					const auto attrId = itr->attrId;
					const auto* ptr = reinterpret_cast<const GLvoid*>(t2.offset);
					bool bInteger = false;
					#ifndef USE_OPENGLES2
					{
						const auto typ = GLFormat::QueryGLSLInfo(t2.elemFlag)->type;
						if(typ == GLSLType::BoolT || typ == GLSLType::IntT) {
							// PCにおけるAMDドライバの場合、Int値はIPointerでセットしないと値が化けてしまう為
							bInteger = true;
						}
					}
					#endif
					q.add(DCmd_VPtr{
						.attrId = attrId,
						.elemSize = t2.elemSize,
						.elemFlag = t2.elemFlag,
						.stride = stride,
						.offset = ptr,
						.bInteger = bInteger,
						.bNormalize = t2.bNormalize,
					});
				};
				++cur;
			}
		}
		_streamOfs[MaxVStream] = _streamOfs[MaxVStream-1];
	}
	void VDecl::DCmd_VPtr::Command(const void* p) {
		auto& self = *static_cast<const DCmd_VPtr*>(p);
		GL.glEnableVertexAttribArray(self.attrId);
		if(self.bInteger) {
			GL.glVertexAttribIPointer(
				self.attrId,
				self.elemSize,
				self.elemFlag,
				self.stride,
				self.offset
			);
		} else {
			GL.glVertexAttribPointer(
				self.attrId,
				self.elemSize,
				self.elemFlag,
				self.bNormalize,
				self.stride,
				self.offset
			);
		}
	}
	void VDecl::dcmd_apply(draw::IQueue& q, const HVb (&stream)[MaxVStream], const VSem_AttrV& attr) const {
		for(std::size_t i=0 ; i<countof(stream) ; i++) {
			// VStreamが設定されていればBindする
			auto& vb = stream[i];
			if(vb) {
				const GLuint stride = vb->getStride();
				const auto from = _streamOfs[i],
							to = _streamOfs[i+1];
				vb->dcmd_use(q);
				for(std::size_t j=from ; j<to ; j++)
					_setter[j](q, stride, attr);
				vb->dcmd_useEnd(q);
			}
		}
	}
	bool VDecl::operator == (const VDecl& vd) const {
		// VDInfoの比較
		return _vdInfo == vd._vdInfo;
	}
	bool VDecl::operator != (const VDecl& vd) const {
		return !(this->operator == (vd));
	}
}
