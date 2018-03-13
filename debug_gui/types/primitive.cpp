#include "../../primitive.hpp"
#include "../entry_field.hpp"
#include "../group.hpp"
#include "../id.hpp"
#include "../../gl_buffer.hpp"
#include "../print.hpp"
#include "../../vdecl.hpp"
#include "../tree.hpp"
#include "../child.hpp"
#include "../sstream.hpp"
#include "../indent.hpp"

namespace rev {
	bool Primitive::property(const bool edit) {
		auto f = debug::EntryField("Primitive", edit);
		f.show("DrawMode", drawMode);
		ImGui::Columns(1);
		bool mod = false;
		if(vdecl) {
			if(ImGui::CollapsingHeader("Vertex-Decl")) {
				const auto _ = debug::IndentPush();
				vdecl->property();
			}
		}
		StringStream s;
		for(int i=0 ; i<MaxVStream ; i++) {
			if(vb[i]) {
				s << "VertexBuffer[" << i << "]";
				if(const auto _ = debug::TreePush(s.output().c_str())) {
					mod |= vb[i]->property(edit);
				}
			}
		}
		if(ib) {
			if(const auto _ = debug::TreePush("IndexBuffer")) {
				mod |= ib->property(edit);
			}
		}
		return mod | f.modified();
	}
}
