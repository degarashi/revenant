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

	// ---------------- LCTable ----------------
	namespace {
		struct CompareVisitor : boost::static_visitor<bool> {
			template <class T0, class T1>
			bool operator()(const T0&, const T1&) const {
				return false;
			}
			template <class T>
			bool operator()(const T& t0, const T& t1) const {
				return t0 == t1;
			}
			bool operator()(const std::string& s, const char* c) const {
				return s == std::string(c);
			}
			bool operator()(const char* c, const std::string& s) const {
				return std::string(c) == s;
			}
			bool operator()(const LCTable_SP& t0, const LCTable_SP& t1) const {
				return t0->preciseCompare(*t1);
			}
		};
	}
	bool LCTable::preciseCompare(const LCTable& tbl) const {
		if(size() != tbl.size())
			return false;
		for(auto& ent : *this) {
			CompareVisitor cv;
			auto itr = std::find_if(tbl.begin(), tbl.end(),
				[&cv, &ent](const auto& p){
					return boost::apply_visitor(cv, ent.first, p.first);
				});
			if(itr == tbl.end())
				return false;
			if(!boost::apply_visitor(cv, ent.second, itr->second))
				return false;
		}
		return true;
	}
	// ---------------- LCValue ----------------
	bool LCValue::preciseCompare(const LCValue& c) const {
		return boost::apply_visitor(CompareVisitor(), *this, c);
	}
	LCValue::LCValue():
		LCVar(LuaNil())
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
		new(this) LCValue(lcv);
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
			LuaType operator()(const T& t) const {
				return LCV<T>()(t);
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
				*_os << t;
				return _os;
			}
		};
	}
	std::ostream& operator << (std::ostream& os, const LCValue& lcv) {
		return *boost::apply_visitor(PrintVisitor(os), lcv);
	}
}
