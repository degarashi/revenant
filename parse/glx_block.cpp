#include "glx_block.hpp"
#include "sdl_rw.hpp"

namespace rev {
	namespace parse {
		namespace {
			const std::string cs_rtname[] = {
				"effect"
			};
		}
		BlockSet_SP LoadGLXStructSet(const std::string& path) {
			BlockSet_SP bs = std::make_shared<BlockSet>();
			std::unordered_set<std::string>		loaded,
												toLoad{path};
			// 現状はファイルパスのみ対応
			// toLoad = まだ読み込んでないファイル名
			while(!toLoad.empty()) {
				// toLoadから1つ取り出して読み込み
				const auto itr = toLoad.begin();
				auto hdl = mgr_block.loadResourceApp<GLXStruct>(
								UserURI(*itr),
								[](auto& uri, auto&& mk){
									auto s = mgr_rw.fromURI(*uri.uri, Access::Read);
									mk();
									ParseGlx(*mk.pointer, s->readAllAsString());
								}
							).first;
				loaded.emplace(*itr);
				toLoad.erase(itr);
				// まだ読み込んでないファイルをtoLoadに加える
				for(auto& inc : hdl->incl) {
					if(loaded.count(inc) == 0)
						toLoad.emplace(inc);
				}
				bs->emplace(std::move(hdl));
			}
			return bs;
		}
		// ----------------- FxBlock -----------------
		FxBlock::FxBlock():
			ResMgrApp(cs_rtname)
		{}
		// ----------------- BlockSet -----------------
		template <class RET, class GETM>
		auto FindBlock(const BlockSet& bs, GETM getM, const std::string& s) -> spi::Optional<const RET&> {
			for(auto& b : bs) {
				const auto& m = getM(*b).get();
				auto itr = m.find(s);
				if(itr != m.cend())
					return itr->second;
			}
			return spi::none;
		}
		spi::Optional<const AttrStruct&> BlockSet::findAttribute(const std::string& s) const {
			return FindBlock<AttrStruct>(*this, [](auto&& str) { return std::cref(str.atM); }, s);
		}
		spi::Optional<const ConstStruct&> BlockSet::findConst(const std::string& s) const {
			return FindBlock<ConstStruct>(*this, [](auto&& str) { return std::cref(str.csM); }, s);
		}
		spi::Optional<const UnifStruct&> BlockSet::findUniform(const std::string& s) const {
			return FindBlock<UnifStruct>(*this, [](auto&& str) { return std::cref(str.uniM); }, s);
		}
		spi::Optional<const VaryStruct&> BlockSet::findVarying(const std::string& s) const {
			return FindBlock<VaryStruct>(*this, [](auto&& str) { return std::cref(str.varM); }, s);
		}
		spi::Optional<const ShStruct&> BlockSet::findShader(const std::string& s) const {
			return FindBlock<ShStruct>(*this, [](auto&& str) { return std::cref(str.shM); }, s);
		}
		spi::Optional<const TPStruct&> BlockSet::findTechPass(const std::string& s) const {
			for(auto& b : *this) {
				auto& tpL = b->tpL;
				auto itr = std::find_if(tpL.cbegin(), tpL.cend(), [&](auto& tp){ return tp.name == s; });
				if(itr != tpL.cend())
					return *itr;
			}
			return spi::none;
		}
		spi::Optional<const CodeStruct&> BlockSet::findCode(const std::string& s) const {
			return FindBlock<CodeStruct>(*this, [](auto&& str) { return std::cref(str.codeM); }, s);
		}
	}
}
