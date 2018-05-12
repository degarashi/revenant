#pragma once
#include "drawcmd/cmd_unif_if.hpp"
#include "drawcmd/queue_if.hpp"
#include "drawcmd/types/vector.hpp"
#include "drawcmd/types/matrix.hpp"
#include "drawcmd/cmd.hpp"
#include "gl_program.hpp"
#include "gl_texture.hpp"
#include "handle/opengl.hpp"
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
		struct VecSingle : ICmd_Uniform {
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
		struct VecArray : ICmd_Uniform {
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
		struct MatSingle : ICmd_Uniform {
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
		struct MatArray : ICmd_Uniform {
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
		struct TexSingle : ICmd_Uniform {
			HTexC	tex;

			TexSingle(const HTexC& t):
				tex(t)
			{}
			void dcmd_export(draw::IQueue& q, const int id, const int actId) const override {
				tex->dcmd_export(q, id, actId);
			}
		};
		struct TexArray : ICmd_Uniform {
			using Ar = std::vector<HTexC>;
			Ar		tex;

			template <class Itr>
			TexArray(const Itr itr, const Itr itrE):
				tex(itr, itrE)
			{}
			void dcmd_export(draw::IQueue& q, const int id, const int actId) const override {
				const auto len = tex.size();
				for(std::size_t i=0 ; i<len ; i++) {
					tex[i]->dcmd_export(q, id+i, actId+i);
				}
			}
		};
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
		public draw::CommandVec
	{
		private:
			HProg				_program;

		public:
			void setProgram(const HProg& p);
			const HProg& getProgram() const noexcept;

			template <class... Ts>
			void setUniform(const SName& name, Ts&&... ts) {
				if(const auto id = _program->getUniformId(name)) {
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
				draw::VecSingle<vec_t>(t).dcmd_export(*this, id, -1);
			}
			template <class V, ENABLE_IF(frea::is_vector<V>{})>
			void setUniformById(const GLint id, const V& v) {
				draw::VecSingle<V>(v).dcmd_export(*this, id, -1);
			}
			template <class M, ENABLE_IF(frea::is_matrix<M>{})>
			void setUniformById(const GLint id, const M& m) {
				draw::MatSingle<M>(m).dcmd_export(*this, id, -1);
			}
			template <class T>
			void setUniformById(const GLint id, const std::shared_ptr<T>& t) {
				// テクスチャユニット番号を検索
				const auto num = _program->getTexIndex(id);
				D_Assert0(num);
				draw::TexSingle(t).dcmd_export(*this, id, *num);
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
				draw::VecArray<vec_t>(itr, itrE).dcmd_export(*this, id, -1);
			}
			template <
				class Itr,
				class V = std::decay_t<decltype(*std::declval<Itr>())>,
				ENABLE_IF(frea::is_vector<V>{})
			>
			void setUniformById(const GLint id, const Itr itr, const Itr itrE) {
				draw::VecArray<V>(itr, itrE).dcmd_export(*this, id, -1);
			}
			template <
				class Itr,
				class V = std::decay_t<decltype(*std::declval<Itr>())>,
				ENABLE_IF(frea::is_matrix<V>{})
			>
			void setUniformById(const GLint id, const Itr itr, const Itr itrE) {
				draw::MatArray<V>(itr, itrE).dcmd_export(*this, id, -1);
			}
			template <
				class Itr,
				class V = std::decay_t<decltype(*std::declval<Itr>())>,
				ENABLE_IF(detail::Is_SP<V>{})
			>
			void setUniformById(const GLint id, const Itr itr, const Itr itrE) {
				// テクスチャユニット番号を検索
				const auto num = _program->getTexIndex(id);
				D_Assert0(num);
				draw::TexArray(itr, itrE).dcmd_export(*this, id, *num);
			}
			void clearValue();
	};
}
