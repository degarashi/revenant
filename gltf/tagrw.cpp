#include "gltf/idtag.hpp"
#include "../uri.hpp"
#include "../sdl_rw.hpp"

namespace rev {
	namespace gltf {
		namespace {
			HURI MakeURI_gltf(const char* s) {
				if(auto ret = DataURI::Interpret(s))
					return ret;
				if(auto ret = FileURI::Interpret(s))
					return ret;
				return std::make_shared<FileURI>(s);
			}
		}
		TagRW::TagRW(const JValue& v):
			_uri(MakeURI_gltf(loader::String(v)))
		{}
		HRW TagRW::getRW() const {
			return mgr_rw.fromURI(*_uri, Access::Read);
		}
		const HURI& TagRW::getUri() const noexcept {
			return _uri;
		}
		// ITagQueryを通してローカルファイル等を参照
		void TagRW::resolve(const ITagQuery& q) {
			// gltfローカルパスをグローバルへ変換
			if(_uri->getType() == URI::Type::File) {
				auto& fu = static_cast<FileURI&>(*_uri);
				fu.pathblock() = *std::static_pointer_cast<const PathBlock>(
					q.query(Resource::Type::Localfile, fu.path())
				);
			}
		}
		const ByteV& TagRW::getBuffer() const {
			if(!_buffer_cached)
				_buffer_cached = getRW()->readAll();
			return *_buffer_cached;
		}
	}
}
