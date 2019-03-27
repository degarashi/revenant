#include "rwref.hpp"
#include "dataquery_if_base.hpp"
#include "value_loader.hpp"
#include "../uri.hpp"
#include "../sdl/rw.hpp"

namespace rev::gltf {
	RWRef::RWRef(const JValue& v, const IDataQueryBase& q):
		_uri([](const IDataQueryBase& q, const char* s) -> HURI {
			if(auto ret = DataURI::Interpret(s))
				return ret;
			if(auto ret = FileURI::Interpret(s))
				return ret;
			// ローカルパスをグローバルへ変換
			const auto path = q.getFilePath(s);
			return std::make_shared<FileURI>(path.plain_utf8());
		}(q, loader::String(v)))
	{}
	HRW RWRef::getRW() const {
		return mgr_rw.fromURI(*_uri, Access::Read);
	}
	const HURI& RWRef::getUri() const noexcept {
		return _uri;
	}
	const ByteV& RWRef::getBuffer() const {
		if(!_buffer_cached)
			_buffer_cached = getRW()->readAll();
		return *_buffer_cached;
	}
}
