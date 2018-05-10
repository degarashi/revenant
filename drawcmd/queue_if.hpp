#pragma once
#include <memory>
#include <functional>

namespace rev::draw {
	using VoidC_SP = std::shared_ptr<const void>;
	using CommandF = void (*)(const void*);
	class IQueue {
		protected:
			using DataP = std::pair<const void*, std::size_t>;
			virtual void* _allocateMemory(std::size_t s, CommandF f) = 0;
			virtual void _copyMemory(const DataP& src) = 0;
		public:
			virtual ~IQueue() {}
			template <class T>
			void add(const T& obj) {
				static_assert(
					std::is_trivially_destructible_v<T> &&
					std::is_trivially_copy_constructible_v<T>
				);
				new(_allocateMemory(sizeof(T), &T::Command)) T(obj);
			}
			virtual void stockResource(const VoidC_SP& r) = 0;

			using CBResource = std::function<void (const VoidC_SP&)>;
			virtual DataP getData() const = 0;
			virtual void getResource(const CBResource& cb) const = 0;

			void append(const IQueue& src) {
				_copyMemory(src.getData());
				src.getResource([this](const VoidC_SP& r){
					stockResource(r);
				});
			}
	};
}
