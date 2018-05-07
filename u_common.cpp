#include "u_common.hpp"
#include "spine/flyweight_item.hpp"
#include "gl_program.hpp"
#include "uniform_ent.hpp"

namespace rev {
	namespace {
		using UniformName = spi::FlyweightItem<std::string>;
		namespace unif {
			const UniformName
						Alpha("u_alpha"),
						Color("u_color"),
						Depth("u_depth");
			namespace texture {
				const UniformName
						Diffuse("u_texDiffuse"),
						Specular("u_texSpecular"),
						Normal("u_texNormal"),
						Emissive("u_texEmissive");
			}
		}
	}

	U_Common::U_Common():
		alpha(1),
		color{1,1,1,1},
		depth(1)
	{}
	void U_Common::extractUniform(UniformSetF_V& dst, const GLProgram& prog) const {
		#define DEF_UNIF(name, cname) \
			if(const auto id = prog.getUniformId(cname)) { \
				dst.emplace_back([id=*id](const void* p, UniformEnt& u){ \
					auto& self = *static_cast<const U_Common*>(p); \
					u.setUniform(id, self.name); \
				}); \
			}
		DEF_UNIF(alpha, unif::Alpha)
		DEF_UNIF(color, unif::Color)
		DEF_UNIF(depth, unif::Depth)
		DEF_UNIF(texture.diffuse, unif::texture::Diffuse)
		DEF_UNIF(texture.specular, unif::texture::Specular)
		DEF_UNIF(texture.normal, unif::texture::Normal)
		DEF_UNIF(texture.emissive, unif::texture::Emissive)
		#undef DEF_UNIF
	}
}
