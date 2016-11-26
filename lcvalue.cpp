#include "lcv.hpp"

namespace std {
	template <class T>
	bool operator == (const weak_ptr<T>& p0, const weak_ptr<T>& p1) noexcept {
		return p0.lock() == p1.lock();
	}
}
namespace rev {
	bool LuaNil::operator == (LuaNil) const { return true; }
	bool LuaNil::operator != (LuaNil) const { return false; }

	LCValue::LCValue():
		LCVar(boost::blank())
	{}
	LCValue::LCValue(const LCValue& lc):
		LCVar(static_cast<const LCVar&>(lc))
	{}
	LCValue::LCValue(LCValue&& lcv):
		LCVar(std::move(static_cast<LCVar&>(lcv)))
	{}
	LCValue::LCValue(const lua_OtherNumber num):
		LCValue(static_cast<lua_Number>(num))
	{}
	LCValue::LCValue(const lua_IntegerU num):
		LCValue(static_cast<lua_Integer>(num))
	{}
	LCValue::LCValue(const lua_OtherInteger num):
		LCValue(static_cast<lua_Integer>(num))
	{}
	LCValue::LCValue(const lua_OtherIntegerU num):
		LCValue(static_cast<lua_Integer>(num))
	{}
	LCValue::LCValue(const LCTable_SP& tbl):
		LCVar(tbl)
	{}
	LCValue::LCValue(const Lua_SP& sp):
		LCVar(sp)
	{}

	bool LCValue::operator == (const LCValue& lcv) const noexcept {
		return static_cast<const LCVar&>(*this) == static_cast<const LCVar&>(lcv);
	}
	bool LCValue::operator != (const LCValue& lcv) const noexcept {
		return !(this->operator == (lcv));
	}
	namespace {
		/*! shared_ptr -> shared_ptr
			weak_ptr -> shared_ptr */
		struct GetSPVisitor : boost::static_visitor<HRes> {
			HRes operator()(const HRes& p) const {
				return p;
			}
			HRes operator()(const WRes& p) const {
				return p.lock();
			}
			template <class T>
			HRes operator()[[noreturn]](const T&) const {
				AssertF0();
				throw 0;
			}
		};
	}
	HRes LCValue::_toSPtr() const {
		return boost::apply_visitor(GetSPVisitor(), *this);
	}
	namespace {
		struct ConvertBool : boost::static_visitor<bool> {
			bool operator()(boost::blank) const { return false; }
			bool operator()(LuaNil) const { return false; }
			bool operator()(bool b) const { return b; }
			template <class T>
			bool operator()(const T&) const { return true; }
		};
		const LCValue c_dummyLCValue;
		struct ReferenceIndex : boost::static_visitor<const LCValue&> {
			const int index;
			ReferenceIndex(int idx): index(idx) {}
			const LCValue& operator()(const LCTable_SP& sp) const {
				auto itr = sp->find(index);
				if(itr != sp->end())
					return itr->second;
				return c_dummyLCValue;
			}
			template <class T>
			const LCValue& operator()(const T&) const {
				Assert(false, "invalid LCValue type");
				return c_dummyLCValue;
			}
		};
	}
	LCValue::operator bool () const noexcept {
		return boost::apply_visitor(ConvertBool(), *this);
	}
	const LCValue& LCValue::operator [](const int s) const {
		// Luaの配列インデックスは1オリジンのため、1を加える
		return boost::apply_visitor(ReferenceIndex(s+1), *this);
	}
	LCValue::LCValue(std::tuple<>&):
		LCValue()
	{}
	LCValue::LCValue(std::tuple<>&&):
		LCValue()
	{}
	LCValue::LCValue(const std::tuple<>&):
		LCValue()
	{}
	LCValue& LCValue::operator = (const LCValue& lcv) {
		this->~LCValue();
		new(this) LCValue(static_cast<const LCVar&>(lcv));
		return *this;
	}
	LCValue& LCValue::operator = (LCValue&& lcv) {
		this->~LCValue();
		new(this) LCValue(std::move(lcv));
		return *this;
	}
	namespace {
		struct PushVisitor : boost::static_visitor<> {
			lua_State* _ls;
			PushVisitor(lua_State* ls):
				_ls(ls)
			{}
			template <class T>
			void operator()(const T& t) const {
				LCV<T>()(_ls, t);
			}
		};
	}
	void LCValue::push(lua_State* ls) const {
		boost::apply_visitor(PushVisitor(ls), *this);
	}
	namespace {
		struct TypeVisitor : boost::static_visitor<LuaType> {
			template <class T>
			LuaType operator()(const T&) const {
				return LCV<T>()();
			}
		};
	}
	LuaType LCValue::type() const {
		return boost::apply_visitor(TypeVisitor(), *this);
	}
	namespace {
		struct LCVisitor : boost::static_visitor<const char*> {
			template <class T>
			const char* operator()(T) const {
				return nullptr;
			}
			const char* operator()(const char* c) const {
				return c;
			}
			const char* operator()(const std::string& s) const {
				return s.c_str();
			}
		};
	}
	const char* LCValue::toCStr() const {
		return boost::apply_visitor(LCVisitor(), *this);
	}
	std::string LCValue::toString() const {
		auto* str = toCStr();
		if(str)
			return std::string(str);
		return std::string();
	}
	namespace {
		struct PrintVisitor : boost::static_visitor<std::ostream*> {
			std::ostream* _os;
			PrintVisitor(std::ostream& os):
				 _os(&os)
			{}
			template <class T>
			std::ostream* operator()(const T& t) const {
				 return &LCV<T>()(*_os, t);
			}
		};
	}
	std::ostream& operator << (std::ostream& os, const LCValue& lcv) {
		return *boost::apply_visitor(PrintVisitor(os), lcv);
	}
}