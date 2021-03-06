#include "tech_pass.hpp"
#include "techmgr.hpp"
#include "tech_if.hpp"

namespace rev {
	TechPass::TechPass(const parse::BlockSet& bs) {
		const auto tpv = MakeTechPair(bs);
		for(auto& t : tpv) {
			for(auto& p : t.pass) {
				_nameToTech[MakeName(t.name, p->getName())] = p;
			}
		}
	}
	Name TechPass::MakeName(const Name& tech, const Name& pass) {
		Name tp(tech);
		tp.append("|");
		tp.append(pass);
		return tp;
	}
	HTech TechPass::getTechnique(const Name& techpass) const {
		const auto itr = _nameToTech.find(techpass);
		if(itr != _nameToTech.end()) {
			return itr->second;
		}
		return nullptr;
	}
	HTech TechPass::getTechnique(const Name& tech, const Name& pass) const {
		return getTechnique(MakeName(tech, pass));
	}
	const char* TechPass::getResourceName() const noexcept {
		return "TechPass";
	}
	TechPass::NameToTech& TechPass::refTech() noexcept {
		return _nameToTech;
	}
}
#include <boost/format.hpp>
namespace rev {
	TechPass::EC_FileNotFound::EC_FileNotFound(const std::string& fPath):
		std::runtime_error((boost::format("file path: \"%1%\" was not found.") % fPath).str())
	{}
}
