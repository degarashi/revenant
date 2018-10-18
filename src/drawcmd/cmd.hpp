#pragma once
#include "queue_if.hpp"
#include <vector>
#include <unordered_set>

namespace rev::draw {
	class CommandVec : public IQueue {
		public:
			using Res = std::vector<VoidC_SP>;
			Res		_res;
			using Vec = std::vector<uint8_t>;
			Vec		_vec;
			void* _allocateMemory(std::size_t s, CommandF f) override;
			void _copyMemory(const DataP& src) override;
			void _copyResource(const ResP& src) override;
		public:
			// -------------- call from DrawThread --------------
			void exec();
			// -------------- call from MainThread --------------
			void clear();
			void stockResource(const VoidC_SP& r) override;
			DataP getData() const override;
			ResP getResource() const override;
	};
}
