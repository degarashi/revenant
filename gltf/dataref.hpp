#pragma once
#include "gltf/dataquery_if.hpp"
#include "gltf/value_loader.hpp"

namespace rev::gltf {
	template <
		class T,
		const T& (IDataQuery::*FP)(const Tag&) const
	>
	class DataRef {
		private:
			const T*		_data;
		public:
			DataRef(): _data(nullptr) {}
			DataRef(const JValue& v, const IDataQuery& q):
				_data(&(q.*FP)(loader::StdString(v)))
			{}
			DataRef(const Tag& tag, const IDataQuery& q):
				_data(&(q.*FP)(tag))
			{}
			static bool CanLoad(const JValue& v) {
				return loader::String::CanLoad(v);
			}
			const T* data() const noexcept {
				return _data;
			}
			auto operator * () const noexcept -> decltype(*_data) {
				return *_data;
			}
			auto operator ->() const -> decltype(_data) {
				return _data;
			}
			explicit operator bool () const noexcept {
				return static_cast<bool>(_data);
			}
	};
	#define DEF_REF(z, ign, name) \
		using BOOST_PP_CAT(DRef_, name) = DataRef<name, &IDataQuery::BOOST_PP_CAT(get, name)>;
	BOOST_PP_SEQ_FOR_EACH(DEF_REF, EMPTY, SEQ_RES)
	#undef DEF_REF
}
