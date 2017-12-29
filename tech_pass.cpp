#include "tech_pass.hpp"
#include "glx_block.hpp"

namespace rev {
	TechPass::TechPass(const std::string& path):
		_tech(MakeGLXMaterial(parse::LoadGLXStructSet(path)))
	{
		for(auto& t : _tech) {
			const Name techName(t.name);
			for(auto& p : t.pass) {
				Name tp(techName);
				tp.append("|");
				tp.append(p->getName());
				_nameToId[tp] = std::make_pair(&t - _tech.data(),
												&p - t.pass.data());
			}
		}
	}
	Tech_SP TechPass::getTechnique(const Name& techpass) const {
		const auto itr = _nameToId.find(techpass);
		if(itr != _nameToId.end()) {
			const auto idx = itr->second;
			return _tech[idx.first].pass[idx.second];
		}
		return nullptr;
	}
	Tech_SP TechPass::getTechnique(const Name& tech, const Name& pass) const {
		Name tp(tech);
		tp.append(pass);
		return getTechnique(tp);
	}
}
#include <boost/format.hpp>
namespace rev {
	TechPass::EC_FileNotFound::EC_FileNotFound(const std::string& fPath):
		std::runtime_error((boost::format("file path: \"%1%\" was not found.") % fPath).str())
	{}
}
