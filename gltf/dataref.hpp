#pragma once
#include "value_loader.hpp"

namespace rev::gltf {
	template <
		class T,
		class Tag_t,
		class Q,
		const T& (Q::*FP)(const Tag_t&) const
	>
	class DataRef {
		public:
			using data_t = T;
		private:
			const T*		_data;
			using Loader = loader::FindLoader_t<Tag_t>;
		public:
			DataRef(): _data(nullptr) {}
			DataRef(const JValue& v, const Q& q):
				_data(&(q.*FP)(Loader(v)))
			{}
			DataRef(const Tag_t& tag, const Q& q):
				_data(&(q.*FP)(tag))
			{}
			static bool CanLoad(const JValue& v) {
				return Loader::CanLoad(v);
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
}
