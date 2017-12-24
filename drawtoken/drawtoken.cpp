#include "tokenbuffer.hpp"
#include "lubee/error.hpp"

namespace rev {
	namespace draw {
		// ------------------- TokenBuffer -------------------
		TokenBuffer::TokenBuffer():
			_bInit(false)
		{}
		TokenBuffer::TokenBuffer(TokenBuffer&& t):
			TokenBuffer()
		{
			if(t._bInit)
				t.asToken()->takeout(*this);
			t._bInit = false;
		}
		TokenBuffer::~TokenBuffer() {
			if(_bInit) {
				asToken()->~Token();
				_bInit = false;
			}
		}
		Token* TokenBuffer::asToken() {
			D_Assert0(_bInit);
			auto p = reinterpret_cast<intptr_t>(_data.data()) + _offset;
			return reinterpret_cast<Token*>(p);
		}
		const Token* TokenBuffer::asToken() const {
			return const_cast<TokenBuffer*>(this)->asToken();
		}
#ifdef DEBUG
		void* TokenBuffer::allocate_memory(const std::size_t s, const intptr_t ofs) {
#else
		void* TokenBuffer::allocate_memory(std::size_t /*s*/, const intptr_t ofs) {
#endif
			D_Assert0(_data.size() >= s);
			D_Assert0(ofs >= 0);
			if(_bInit)
				asToken()->~Token();
			else
				_bInit = true;
			_offset = ofs;
			return _data.data();
		}
		void TokenBuffer::exec() {
			asToken()->exec();
		}
		void TokenBuffer::clone(TokenDst& dst) const {
			asToken()->clone(dst);
		}
		void TokenBuffer::takeout(TokenDst& dst) {
			asToken()->takeout(dst);
		}
		std::size_t TokenBuffer::getSize() const {
			return asToken()->getSize();
		}
	}
}
