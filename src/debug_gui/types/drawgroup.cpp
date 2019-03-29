#include "../../object/drawgroup.hpp"
#include "../indent.hpp"
#include "../tree.hpp"
#include "../listview.hpp"

namespace rev {
	const char* DrawGroup::getDebugName() const noexcept {
		return "DrawGroup";
	}
	bool DrawGroup::property(const bool edit) {
		bool mod = false;
		if(const auto _ = debug::TreePush(DrawableObj::getDebugName())) {
			mod |= DrawableObj::property(edit);
		}
		ImGui::Separator();
		{
			ImGui::TextUnformatted("Sort order: ");
			const auto indent = debug::IndentPush();
			for(auto& s : _dsort) {
				ImGui::TextUnformatted(s->getDebugName());
			}
		}
		if(ImGui::Checkbox("Dynamic sort", &_bDynamic)) {
			if(_bDynamic) {
				// リスト中のオブジェクトをソートし直す
				_doDrawSort();
			}
		}
		ImGui::Separator();

		struct GetSecond {
			using Itr = std::decay_t<decltype(_dobj.cbegin())>;
			Itr		itr;
			decltype(auto) operator * () const {
				return itr->second;
			}
			decltype(auto) operator ++ () {
				++itr;
				return *this;
			}
			bool operator != (const  GetSecond& s) const noexcept {
				return itr != s.itr;
			}
		};
		debug::ListView(GetSecond{_dobj.cbegin()}, GetSecond{_dobj.cend()},
			true,
			debug::ListViewFlag::NarrowColumn | debug::ListViewFlag::Filter);
		return mod;
	}
}
