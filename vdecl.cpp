#include "glx.hpp"
#include "gl_error.hpp"
#include "gl_format.hpp"
#include "drawtoken/buffer.hpp"

namespace rev {
	// ----------------- VDecl::VDInfo -----------------
	bool VDecl::VDInfo::operator == (const VDInfo& v) const {
		return ((streamId ^ v.streamId)
				| (offset ^ v.offset)
				| (elemFlag ^ v.elemFlag)
				| (bNormalize ^ v.bNormalize)
				| (elemSize ^ v.elemSize)
				| (semId ^ v.semId)) == 0;
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
	VDecl::VDecl(std::initializer_list<VDInfo> il):
		VDecl(VDInfoV(il.begin(), il.end()))
	{}
	void VDecl::_init() {
		// StreamId毎に集計
		std::vector<VDInfo> stream[VData::MaxVStream];
		for(auto& v : _vdInfo)
			stream[v.streamId].push_back(v);

		// 頂点定義のダブり確認
		for(auto& t : stream) {
			if(!t.empty()) {
				// オフセットでソート
				std::sort(t.begin(), t.end(), [](const VDInfo& v0, const VDInfo& v1) { return v0.offset < v1.offset; });

				unsigned int tail = 0;
				for(auto& t2 : t) {
					if(tail > t2.offset)
						throw GLE_Error("invalid vertex offset");
					tail = t2.offset + GLFormat::QuerySize(t2.elemFlag) * t2.elemSize;
				}
			}
		}

		_func.resize(_vdInfo.size());
		int cur = 0;
		for(int i=0 ; i<static_cast<int>(countof(stream)) ; i++) {
			_nEnt[i] = cur;
			for(auto& t2 : stream[i]) {
				_func[cur] = [t2](GLuint stride, const VData::AttrA& attr) {
					auto attrId = attr[t2.semId];
					if(attrId < 0)
						return;
					GL.glEnableVertexAttribArray(attrId);
					#ifndef USE_OPENGLES2
						auto typ = GLFormat::QueryGLSLInfo(t2.elemFlag)->type;
						if(typ == GLSLType::BoolT || typ == GLSLType::IntT) {
							// PCにおけるAMDドライバの場合、Int値はIPointerでセットしないと値が化けてしまう為
							GL.glVertexAttribIPointer(attrId, t2.elemSize, t2.elemFlag, stride, reinterpret_cast<const GLvoid*>(t2.offset));
						} else
					#endif
							GL.glVertexAttribPointer(attrId, t2.elemSize, t2.elemFlag, t2.bNormalize, stride, reinterpret_cast<const GLvoid*>(t2.offset));
				};
				++cur;
			}
		}
		_nEnt[VData::MaxVStream] = _nEnt[VData::MaxVStream-1];
	}
	void VDecl::apply(const VData& vdata) const {
		for(int i=0 ; i<VData::MaxVStream ; i++) {
			auto& ovb = vdata.buff[i];
			// VStreamが設定されていればBindする
			if(ovb) {
				auto& vb = *ovb;
				auto u = vb.use();
				GLuint stride = vb.getStride();
				for(int j=_nEnt[i] ; j<_nEnt[i+1] ; j++)
					_func[j](stride, vdata.attrId);
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
