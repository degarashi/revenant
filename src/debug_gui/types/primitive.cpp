#include "../../effect/primitive.hpp"
#include "../entry_field.hpp"
#include "../group.hpp"
#include "../../gl/buffer.hpp"
#include "../print.hpp"
#include "../../effect/vdecl.hpp"
#include "../tree.hpp"
#include "../child.hpp"
#include "../sstream.hpp"
#include "lubee/src/meta/countof.hpp"
#include "../header.hpp"
#include "../resource_window.hpp"
#include "lubee/src/meta/constant_t.hpp"
#include "../print.hpp"

namespace rev {
	namespace debug {
		namespace {
			template <class CB>
			void SelectByType(const GLuint elemFlag, CB&& cb) {
				switch(elemFlag) {
					case GL_BYTE:
						cb(GLubyte{});
						break;
					case GL_UNSIGNED_BYTE:
						cb(GLubyte{});
						break;
					case GL_SHORT:
						cb(GLshort{});
						break;
					case GL_UNSIGNED_SHORT:
						cb(GLushort{});
						break;
					case GL_FLOAT:
						cb(GLfloat{});
						break;
					default:
						Assert0(false);
				}
			}
			using lubee::SZConst;
			template <class CB>
			void SelectByN(const std::size_t n, CB&& cb) {
				switch(n) {
					case 1:
						cb(SZConst<1>{});
						break;
					case 2:
						cb(SZConst<2>{});
						break;
					case 3:
						cb(SZConst<3>{});
						break;
					case 4:
						cb(SZConst<4>{});
						break;
					default:
						Assert0(false);
				}
			}
			template <class T, class Unit, std::size_t... Idx>
			T _MakeValue(const Unit* ptr, std::index_sequence<Idx...>) {
				return T{*(ptr+Idx)...};
			}
			template <class T, class Unit, std::size_t N>
			T MakeValue(const void* ptr) {
				return _MakeValue<T>(reinterpret_cast<const Unit*>(ptr), std::make_index_sequence<N>{});
			}
		}
		class VBView : public IDebugGui {
			private:
				WVbC			_vb;
				VDecl::VDInfo	_vd;

			public:
				VBView(const HVb& vb, const VDecl::VDInfo& vd):
					_vb(vb),
					_vd(vd)
				{}
				bool property(const bool) override {
					if(auto vb = _vb.lock()) {
						std::size_t stride = vb->getStride();
						if(_vd.strideOvr != 0)
							stride = _vd.strideOvr;

						const auto _ = debug::ColumnPush(2);
						std::size_t idx = 0;
						auto ptr = reinterpret_cast<uintptr_t>(vb->_getBufferPtr());
						const auto ptrE = ptr + vb->_getBufferSize();
						ptr += _vd.offset;
						while(ptrE-stride >= ptr) {
							SelectByType(_vd.elemFlag, [this, &idx, ptr](auto typ){
								using Unit = decltype(typ);
								SelectByN(_vd.elemSize, [&idx, ptr](auto n){
									constexpr std::size_t N = decltype(n)::value;
									const auto value = MakeValue<frea::Vec_t<Unit, N, false>, Unit, N>(reinterpret_cast<const void*>(ptr));
									debug::Show(nullptr, idx++);
									ImGui::NextColumn();
									debug::Show(nullptr, value);
									ImGui::NextColumn();
								});
							});
							ptr += stride;
						}
					} else {
						debug::Show(nullptr, "Vertex buffer deleted");
					}
					return false;
				}
				const char* getDebugName() const noexcept override {
					return "VertexBuffer-View";
				}
		};
		using VBView_SP = std::shared_ptr<VBView>;
	}
	bool Primitive::property(const bool edit) {
		static debug::VBView_SP s_vbview;
		auto f = debug::EntryField("Primitive", edit);
		f.show("DrawMode", drawMode);
		ImGui::Columns(1);
		bool mod = false;
		if(const auto _ = debug::Header("Vertex-Decl", !vdecl)) {
			vdecl->property([this](const auto& vd){
				if(ImGui::Selectable("show vertex data...")) {
					s_vbview = std::make_shared<debug::VBView>(vb[vd.streamId], vd);
					debug::ResourceWindow::Add(s_vbview);
				}
			});
		}
		{
			StringStream s;
			const auto len = vb.size();
			for(std::size_t i=0 ; i<len ; i++) {
				if(vb[i]) {
					s << "VertexBuffer[" << i << "]";
					const auto name = s.output();
					if(const auto _ = debug::TreePush(name.c_str())) {
						mod |= vb[i]->property(edit);
					}
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
