#include "tokenml.hpp"
#include "lubee/error.hpp"

namespace rev {
	namespace draw {
		namespace detail {
			// ------------------- TokenMemory -------------------
			TokenMemory::TokenMemory(const std::size_t s):
				_buffer(s),
				_used(0)
			{}
			TokenMemory::~TokenMemory() {
				iterate([](auto* token){
					token->~Token();
				});
			}
			void TokenMemory::iterate(const IterCB& f) {
				uint8_t* data = _buffer.data();
				const auto used = _used;
				std::size_t cur = 0;
				while(cur < used) {
					auto ptr = reinterpret_cast<intptr_t>(data + cur);
					auto ofs = *reinterpret_cast<intptr_t*>(ptr);
					ptr += ofs+sizeof(intptr_t);
					auto* token = reinterpret_cast<Token*>(ptr);
					auto cur_ofs = token->getSize();
					f(token);
					cur += cur_ofs + sizeof(intptr_t);
				}
			}
			void* TokenMemory::getMemory(const std::size_t s, const intptr_t ofs) {
				D_Assert0(ofs >= 0);
				if(_buffer.size() < s+_used+sizeof(intptr_t))
					return nullptr;
				auto ptr = reinterpret_cast<intptr_t>(_buffer.data() + _used);
				*reinterpret_cast<intptr_t*>(ptr) = ofs;
				ptr += sizeof(intptr_t);
				_used += s+sizeof(intptr_t);
				return reinterpret_cast<void*>(ptr);
			}
			void TokenMemory::exec() {
				iterate([](auto* token){
					token->exec();
				});
			}
			void TokenMemory::clear() noexcept {
				_used = 0;
			}
			TokenMemory::TokenMemory(TokenMemory&& t) noexcept:
				_buffer(std::move(t._buffer)),
				_used(t._used)
			{
				t.clear();
			}
			TokenMemory& TokenMemory::operator = (TokenMemory&& t) noexcept {
				_buffer = std::move(t._buffer);
				_used = t._used;
				t.clear();
				return *this;
			}
		}
		// ------------------- TokenML -------------------
		TokenML::TokenML(const std::size_t s):
			_laneSize(s)
		{}
		void* TokenML::allocate_memory(const std::size_t s, const intptr_t ofs) {
			if(_buffer.empty())
				_buffer.emplace_back(_laneSize);
			else {
				if(void* ret = _buffer.back().getMemory(s, ofs))
					return ret;
				_buffer.emplace_back(_laneSize);
			}
			void* ret = _buffer.back().getMemory(s, ofs);
			D_Assert0(ret);
			return ret;
		}
		void TokenML::setLaneSize(const std::size_t s) {
			_laneSize = s;
		}
		void TokenML::iterate(const detail::TokenMemory::IterCB& cb) {
			for(auto& b : _buffer)
				b.iterate(cb);
		}
		void TokenML::exec() {
			for(auto& b : _buffer)
				b.exec();
		}
		void TokenML::clear() {
			_buffer.clear();
		}
		void TokenML::append(TokenML&& ml) {
			std::move(ml._buffer.begin(), ml._buffer.end(), std::back_inserter(_buffer));
			ml._buffer.clear();
		}
	}
}
