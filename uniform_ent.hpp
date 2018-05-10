#pragma once
#include "drawcmd/cmd_if.hpp"
#include "drawcmd/queue_if.hpp"
#include "drawcmd/types/vector.hpp"
#include "drawcmd/types/matrix.hpp"
#include "gl_program.hpp"
#include "gl_texture.hpp"
#include "handle/opengl.hpp"
#include "spine/rflag.hpp"
#include "frea/detect_type.hpp"
#include "lubee/compare.hpp"

namespace rev {
	namespace draw {
		template <class CB>
		void GetMaxN(const std::size_t n, const CB& cb) {
			D_Assert0(lubee::IsInRange<std::size_t>(n, 1, 32));
			if(n == 1)
				cb(lubee::SZConst<1>{});
			else if(n <= 4)
				cb(lubee::SZConst<4>{});
			else if(n <= 8)
				cb(lubee::SZConst<8>{});
			else if(n <= 16)
				cb(lubee::SZConst<16>{});
			else if(n <= 32)
				cb(lubee::SZConst<32>{});
		}
		template <class V>
		struct VecSingle : ICmd {
			V			value;

			VecSingle(const V& v):
				value(v)
			{
				static_assert(frea::is_vector<V>{});
			}
			void dcmd_export(IQueue& q, const int id, const int) const override {
				q.add(MakeVector(value, id));
			}
		};
		template <class V>
		struct VecArray : ICmd {
			using Ar = std::vector<V>;
			Ar			value;

			template <class Itr>
			VecArray(Itr itr, const Itr itrE):
				value(itr, itrE)
			{
				static_assert(frea::is_vector<V>{});
			}
			void dcmd_export(IQueue& q, const int id, const int) const override {
				GetMaxN(value.size(), [this, &q, id](auto n){
					q.add(MakeVectorArray<decltype(n)::value>(value.begin(), value.end(), id));
				});
			}
		};
		template <class M>
		struct MatSingle : ICmd {
			M			value;

			MatSingle(const M& m):
				value(m)
			{
				static_assert(frea::is_matrix<M>{});
			}
			void dcmd_export(draw::IQueue& q, const int id, const int) const override {
				q.add(draw::MakeMatrix(value, id));
			}
		};
		template <class M>
		struct MatArray : ICmd {
			using Ar = std::vector<M>;
			Ar			value;

			template <class Itr>
			MatArray(const Itr itr, const Itr itrE):
				value(itr, itrE)
			{
				static_assert(frea::is_matrix<M>{});
			}
			void dcmd_export(draw::IQueue& q, const int id, const int) const override {
				GetMaxN(value.size(), [this, &q, id](auto n){
					q.add(MakeMatrixArray<decltype(n)::value>(value.begin(), value.end(), id));
				});
			}
		};
		struct TexSingle : ICmd {
			HTexC	tex;

			TexSingle(const HTexC& t):
				tex(t)
			{}
			void dcmd_export(draw::IQueue& q, const int id, const int actId) const override {
				tex->dcmd_uniform(q, id, actId);
			}
		};
		struct TexArray : ICmd {
			using Ar = std::vector<HTexC>;
			Ar		tex;

			template <class Itr>
			TexArray(const Itr itr, const Itr itrE):
				tex(itr, itrE)
			{}
			void dcmd_export(draw::IQueue& q, const int id, const int actId) const override {
				const auto len = tex.size();
				for(std::size_t i=0 ; i<len ; i++) {
					tex[i]->dcmd_uniform(q, id+i, actId+i);
				}
			}
		};
		using Cmd_SP = std::shared_ptr<ICmd>;
	}
	namespace detail {
		template <class T, ENABLE_IF(std::is_floating_point_v<T>)>
		GLfloat NumberCnv(T);
		template <class T, ENABLE_IF(std::is_integral_v<T> && std::is_unsigned_v<T>)>
		GLuint NumberCnv(T);
		template <class T, ENABLE_IF(std::is_integral_v<T> && std::is_signed_v<T>)>
		GLint NumberCnv(T);
		template <class T>
		using NumberCnv_t = decltype(NumberCnv(std::declval<T>()));

		template <class T>
		struct Is_SP : std::false_type {};
		template <class T>
		struct Is_SP<std::shared_ptr<T>> : std::true_type {};
	}
	class UniformEnt :
		public IDebugGui
	{
		private:
			using UniformIdMap_t = std::unordered_map<GLint, draw::Cmd_SP>;

			#define SEQ \
				((Program)(HProg)) \
				((IdEntry)(UniformIdMap_t)(Program))
			RFLAG_DEFINE(UniformEnt, SEQ)
			RFLAG_SETMETHOD(IdEntry)
			RFLAG_REFMETHOD(IdEntry)
		public:
			RFLAG_GETMETHOD_DEFINE(SEQ)
			RFLAG_SETMETHOD_DEFINE(SEQ)
			RFLAG_REFMETHOD_DEFINE(SEQ)
			#undef SEQ

			void dcmd_output(draw::IQueue& q);

			void _setUniform(GLint id, const draw::Cmd_SP& cmd);
			template <class... Ts>
			void setUniform(const SName& name, Ts&&... ts) {
				if(const auto id = getProgram()->getUniformId(name)) {
					setUniformById(*id, std::forward<Ts>(ts)...);
				}
			}
			template <
				class T,
				ENABLE_IF(
					!frea::is_vector<T>{} &&
					!frea::is_matrix<T>{} &&
					!detail::Is_SP<T>{}
				)
			>
			void setUniformById(const GLint id, const T& t) {
				using vec_t = frea::Vec_t<detail::NumberCnv_t<T>, 1, false>;
				_setUniform(id, draw::Cmd_SP(new draw::VecSingle<vec_t>(t)));
			}
			template <class V, ENABLE_IF(frea::is_vector<V>{})>
			void setUniformById(const GLint id, const V& v) {
				_setUniform(id, draw::Cmd_SP(new draw::VecSingle<V>(v)));
			}
			template <class M, ENABLE_IF(frea::is_matrix<M>{})>
			void setUniformById(const GLint id, const M& m) {
				_setUniform(id, draw::Cmd_SP(new draw::MatSingle<M>(m)));
			}
			template <class T>
			void setUniformById(const GLint id, const std::shared_ptr<T>& t) {
				_setUniform(id, std::make_shared<draw::TexSingle>(t));
			}

			template <class T>
			void setUniformById(const GLint id, const std::vector<T>& t) {
				setUniformById(id, t.begin(), t.end());
			}

			template <
				class Itr,
				class T = std::decay_t<decltype(*std::declval<Itr>())>,
				ENABLE_IF(
					!detail::Is_SP<T>{} &&
					!frea::is_matrix<T>{} &&
					!frea::is_vector<T>{}
				)
			>
			void setUniformById(const GLint id, const Itr itr, const Itr itrE) {
				using vec_t = frea::Vec_t<detail::NumberCnv_t<T>, 1, false>;
				_setUniform(id, std::make_shared<draw::VecArray<vec_t>>(itr, itrE));
			}
			template <
				class Itr,
				class V = std::decay_t<decltype(*std::declval<Itr>())>,
				ENABLE_IF(frea::is_vector<V>{})
			>
			void setUniformById(const GLint id, const Itr itr, const Itr itrE) {
				_setUniform(id, std::make_shared<draw::VecArray<V>>(itr, itrE));
			}
			template <
				class Itr,
				class V = std::decay_t<decltype(*std::declval<Itr>())>,
				ENABLE_IF(frea::is_matrix<V>{})
			>
			void setUniformById(const GLint id, const Itr itr, const Itr itrE) {
				_setUniform(id, std::make_shared<draw::MatArray<V>>(itr, itrE));
			}
			template <
				class Itr,
				class V = std::decay_t<decltype(*std::declval<Itr>())>,
				ENABLE_IF(detail::Is_SP<V>{})
			>
			void setUniformById(const GLint id, const Itr itr, const Itr itrE) {
				_setUniform(id, std::make_shared<draw::TexArray>(itr, itrE));
			}
			void assign(const UniformEnt& e);
			void clearValue();

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
