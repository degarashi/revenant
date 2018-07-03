#include "vdecl.hpp"
#include "gl_error.hpp"
#include "gl_format.hpp"
#include "gl_if.hpp"
#include "lubee/meta/countof.hpp"
#include "drawcmd/queue_if.hpp"
#include "vertex_map.hpp"

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
		std::size_t maxStr = 0;
		for(auto& v : _vdInfo)
			maxStr = std::max<std::size_t>(maxStr, v.streamId+1);
		using VDInfoV2 = std::vector<VDInfoV>;
		VDInfoV2 stream(maxStr);
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
		_streamOfs.resize(maxStr+1);
		for(std::size_t i=0 ; i<maxStr ; i++) {
			_streamOfs[i] = cur;
			for(auto& t2 : stream[i]) {
				_setter[cur] = [t2](draw::IQueue& q, const GLuint vb_stride, const VSemAttrMap& attr) {
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
					const auto bInteger =
					#ifndef USE_OPENGLES2
						// PCにおけるAMDドライバの場合、Int値はIPointerでセットしないと値が化けてしまう為
						itr->bInteger;
					#else
						false;
					#endif
					const auto* ptr = reinterpret_cast<const GLvoid*>(t2.offset);
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
		_streamOfs[maxStr] = cur;
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
	void VDecl::DCmd_Disable::Command(const void* p) {
		auto& self = *static_cast<const DCmd_Disable*>(p);
		for(uint_fast8_t i=0 ; i<self.nAttr ; i++)
			GL.glDisableVertexAttribArray(i);
	}
	void VDecl::dcmd_export(draw::IQueue& q, const std::vector<HVb> &stream, const VSemAttrMap& vmap) const {
		q.add(DCmd_Disable{uint_fast8_t(16)});

		const auto len = _streamOfs.size();
		for(std::size_t i=0 ; i<len-1 ; i++) {
			// VStreamが設定されていればBindする
			if(i < stream.size()) {
				if(auto& vb = stream[i]) {
					const GLuint stride = vb->getStride();
					const auto from = _streamOfs[i],
								to = _streamOfs[i+1];
					vb->dcmd_export(q);
					for(std::size_t j=from ; j<to ; j++)
						_setter[j](q, stride, vmap);
				}
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
	bool VDecl::hasAttribute(const VSemantic v) const {
		for(auto& vd : _vdInfo) {
			if(vd.sem == v)
				return true;
		}
		return false;
	}
}
