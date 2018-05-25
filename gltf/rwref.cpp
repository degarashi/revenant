#include "rwref.hpp"
#include "dataquery_if.hpp"
#include "value_loader.hpp"
#include "../uri.hpp"
#include "../sdl_rw.hpp"

namespace rev::gltf {
	namespace {
		HURI MakeURI_gltf(const IDataQuery& q, const char* s) {
			if(auto ret = DataURI::Interpret(s))
				return ret;
			if(auto ret = FileURI::Interpret(s))
				return ret;
			// ローカルパスをグローバルへ変換
			const auto path = q.getFilePath(s);
			return std::make_shared<FileURI>(path.plain_utf8());
		}
	}
	RWRef::RWRef(const JValue& v, const IDataQuery& q):
		_uri(MakeURI_gltf(q, loader::String(v)))
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
