#include "../lc_visitor.hpp"

namespace rev {
	bool LCTable::preciseCompare(const LCTable& tbl) const {
		if(size() != tbl.size())
			return false;
		for(auto& ent : *this) {
			visitor::CompareVisitor cv;
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
}
