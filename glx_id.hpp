//! GLEffectで使うUniformIdやTech&PassId等, 定数の登録支援
#pragma once
#include <cstdint>
#include <algorithm>
#include "lubee/operators.hpp"

namespace rev {
	//! Tech:Pass の組み合わせを表す
	struct GL16Id : lubee::op::Cmp<GL16Id> {
		uint8_t		tech,
					pass;

		GL16Id(const uint8_t t, const uint8_t p) noexcept:
			tech(t), pass(p)
		{}
		bool operator == (const GL16Id& id) const noexcept {
			return tech==id.tech && pass==id.pass;
		}
	};
	//! ある(Tech:Pass)から別の(Tech:Pass)への遷移を表す
	struct GLDiffId : lubee::op::Cmp<GLDiffId> {
		GL16Id	from,
				to;

		GLDiffId(const GL16Id f, const GL16Id t) noexcept:
			from(f), to(t)
		{}
		bool operator == (const GLDiffId& id) const noexcept {
			return from==id.from && to==id.to;
		}
	};
}
namespace std {
	template <> struct hash<::rev::GL16Id> {
		size_t operator() (const ::rev::GL16Id& id) const {
			return (id.tech << 8) | id.pass;
		}
	};
	template <> struct hash<::rev::GLDiffId> {
		size_t operator() (const ::rev::GLDiffId& id) const {
			return (hash<::rev::GL16Id>()(id.from) << 16)
					| (hash<::rev::GL16Id>()(id.to));
		}
	};
}

#include "lubee/wrapper.hpp"
namespace rev {
	inline std::string ConvertToStr(const std::string& s) { return s; }
	template <class T>
	std::string ConvertToStr(const T& t) {
		return std::to_string(t);
	}
	template <class T0, class T1>
	std::string ConvertToStr(const std::pair<T0,T1>& t) {
		return ConvertToStr(t.first) + ":" + ConvertToStr(t.second);
	}
	using StrV = std::vector<std::string>;
	using StrPair = std::pair<std::string, std::string>;
	using StrPairV = std::vector<StrPair>;
	using IdValue = lubee::Wrapper<int>;
	template <class Key>
	class IdMgr {
		private:
			using EntryV = std::vector<Key>;
			EntryV _entry;

		public:
			IdValue genId(const Key& key) {
				auto itr = std::find(_entry.begin(), _entry.end(), key);
				D_Expect(itr==_entry.end(), "Idの重複生成 %1%", ConvertToStr(key));
				if(itr == _entry.end()) {
					_entry.emplace_back(key);
					return IdValue(_entry.size()-1);
				}
				return IdValue(itr - _entry.begin());
			}
			const EntryV& getList() {
				return _entry;
			}
	};

	template <class Tag>
	class IdMgr_Glx {
		private:
			using UnifIdM = IdMgr<std::string>;
			using TechIdM = IdMgr<StrPair>;
			static UnifIdM		s_unifIdM;
			static TechIdM		s_techIdM;
		public:
			static IdValue GenUnifId(const std::string& name) {
				return s_unifIdM.genId(name);
			}
			static IdValue GenTechId(const std::string& tech, const std::string& pass) {
				return s_techIdM.genId({tech, pass});
			}
			static decltype(auto) GetUnifList() {
				return s_unifIdM.getList();
			}
			static decltype(auto) GetTechList() {
				return s_techIdM.getList();
			}
	};
	template <class Tag>
	typename IdMgr_Glx<Tag>::UnifIdM IdMgr_Glx<Tag>::s_unifIdM;
	template <class Tag>
	typename IdMgr_Glx<Tag>::TechIdM IdMgr_Glx<Tag>::s_techIdM;
}
