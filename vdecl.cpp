#include "vdecl.hpp"
#include "gl_error.hpp"
#include "gl_format.hpp"
#include "gl_if.hpp"
#include "lubee/meta/countof.hpp"
#include "drawtoken/buffer.hpp"

namespace rev {
	// ----------------- VDecl::VDInfo -----------------
	VDecl::VDInfo::VDInfo(const GLuint streamId, const GLuint offset, const GLuint elemFlag,
						const GLuint bNormalize, const GLuint elemSize, const VSemantic sem):
		streamId(streamId),
		offset(offset),
		elemFlag(elemFlag),
		bNormalize(bNormalize),
		elemSize(elemSize),
		sem(sem)
	{}
	bool VDecl::VDInfo::operator == (const VDInfo& v) const {
		if( ((streamId ^ v.streamId)
			| (offset ^ v.offset)
			| (elemFlag ^ v.elemFlag)
			| (bNormalize ^ v.bNormalize)
			| (elemSize ^ v.elemSize)) == 0)
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
				// 描画スレッドからの呼び出しとなるのでt2はコピー
				_setter[cur] = [t2](const GLuint stride, const VSem_AttrV& attr) {
					const auto itr = std::find_if(attr.cbegin(), attr.cend(),
						[s=t2.sem](const auto& a){
							return a.sem == s;
						}
					);
					if(itr == attr.cend())
						return;
					const auto attrId = itr->attrId;
					GL.glEnableVertexAttribArray(attrId);
					const auto* ptr = reinterpret_cast<const GLvoid*>(t2.offset);
					#ifndef USE_OPENGLES2
						const auto typ = GLFormat::QueryGLSLInfo(t2.elemFlag)->type;
						if(typ == GLSLType::BoolT || typ == GLSLType::IntT) {
							// PCにおけるAMDドライバの場合、Int値はIPointerでセットしないと値が化けてしまう為
							GL.glVertexAttribIPointer(attrId, t2.elemSize, t2.elemFlag, stride, ptr);
						} else
					#endif
							GL.glVertexAttribPointer(attrId, t2.elemSize, t2.elemFlag, t2.bNormalize, stride, ptr);
				};
				++cur;
			}
		}
		_streamOfs[MaxVStream] = _streamOfs[MaxVStream-1];
	}
	void VDecl::apply(const GLBufferCore* (&stream)[MaxVStream], const VSem_AttrV& attr) const {
		for(std::size_t i=0 ; i<countof(stream) ; i++) {
			// VStreamが設定されていればBindする
			if(const auto* vb = stream[i]) {
				const RUser _(*vb);
				const GLuint stride = vb->getStride();
				D_Assert0(stride > 0);

				const auto from = _streamOfs[i],
							to = _streamOfs[i+1];
				for(std::size_t j=from ; j<to ; j++)
					_setter[j](stride, attr);
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
