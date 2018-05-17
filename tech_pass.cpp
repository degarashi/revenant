#include "tech_pass.hpp"
#include "glx_block.hpp"
#include "tech_if.hpp"

namespace rev {
	TechPass::TechPass(const std::string& path):
		_tech(MakeTechPair(parse::LoadGLXStructSet(path)))
	{
		for(auto& t : _tech) {
			const Name techName(t.name);
			for(auto& p : t.pass) {
				const auto tp = MakeName(techName, p->getName());
				_nameToId[tp] = std::make_pair(&t - _tech.data(),
												&p - t.pass.data());
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
		const auto itr = _nameToId.find(techpass);
		if(itr != _nameToId.end()) {
			const auto idx = itr->second;
			return _tech[idx.first].pass[idx.second];
		}
		return nullptr;
	}
	HTech TechPass::getTechnique(const Name& tech, const Name& pass) const {
		return getTechnique(MakeName(tech, pass));
	}
	const char* TechPass::getResourceName() const noexcept {
		return "TechPass";
	}
}
#include <boost/format.hpp>
namespace rev {
	TechPass::EC_FileNotFound::EC_FileNotFound(const std::string& fPath):
		std::runtime_error((boost::format("file path: \"%1%\" was not found.") % fPath).str())
	{}
}
