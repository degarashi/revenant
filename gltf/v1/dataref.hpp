#pragma once
#include "gltf/v1/dataquery_if.hpp"
#include "../value_loader.hpp"

namespace rev::gltf::v1 {
	template <
		class T,
		class Tag_t,
		const T& (IDataQuery::*FP)(const Tag_t&) const
	>
	class DataRef {
		private:
			const T*		_data;
		public:
			DataRef(): _data(nullptr) {}
			DataRef(const JValue& v, const IDataQuery& q):
				_data(&(q.*FP)(loader::StdString(v)))
			{}
			DataRef(const Tag_t& tag, const IDataQuery& q):
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
		using BOOST_PP_CAT(DRef_, name) = DataRef<name, Tag, &IDataQuery::BOOST_PP_CAT(get, name)>;
	BOOST_PP_SEQ_FOR_EACH(DEF_REF, EMPTY, SEQ_RES)
	#undef DEF_REF
}
