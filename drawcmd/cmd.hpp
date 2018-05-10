#pragma once
#include "queue_if.hpp"
#include <vector>
#include <unordered_set>

namespace rev::draw {
	class CommandVec : public IQueue {
		private:
			using Set = std::unordered_set<VoidC_SP>;
			Set		_set;
			using Vec = std::vector<uint8_t>;
			Vec		_vec;
			void* _allocateMemory(std::size_t s, CommandF f) override;
			void _copyMemory(const DataP& src) override;
		public:
			// -------------- call from DrawThread --------------
			void exec();
			// -------------- call from MainThread --------------
			void clear();
			void stockResource(const VoidC_SP& r) override;
			DataP getData() const override;
			void getResource(const CBResource& cb) const override;
	};
}
