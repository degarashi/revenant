#pragma once
#include "drawcmd/queue_if.hpp"
#include "drawcmd/types/vector.hpp"
#include "drawcmd/types/matrix.hpp"
#include "gl_program.hpp"
#include "gl_texture.hpp"
#include "handle/opengl.hpp"
#include "frea/detect_type.hpp"
#include "lubee/compare.hpp"

namespace rev {
	namespace detail {
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
		struct VecSingle {
			static void DCmd(draw::IQueue& q, const V& value, const int id) {
				static_assert(frea::is_vector<V>{});
				q.add(draw::MakeVector(value, id));
			}
		};
		template <class V>
		struct VecArray {
			template <class Itr>
			static void DCmd(draw::IQueue& q, const Itr itr, const Itr itrE, const int id) {
				static_assert(frea::is_vector<V>{});
				GetMaxN(itrE-itr, [itr, itrE, &q, id](auto n){
					q.add(draw::MakeVectorArray<decltype(n)::value>(itr, itrE, id));
				});
			}
		};
		template <class M>
		struct MatSingle {
			static void DCmd(draw::IQueue& q, const M& value, const int id, const bool transpose) {
				static_assert(frea::is_matrix<M>{});
				q.add(draw::MakeMatrix(value, id, transpose));
			}
		};
		template <class M>
		struct MatArray {
			template <class Itr>
			static void DCmd(draw::IQueue& q, const Itr itr, const Itr itrE, const int id, const bool transpose) {
				static_assert(frea::is_matrix<M>{});
				GetMaxN(itrE-itr, [=, &q](auto n){
					q.add(draw::MakeMatrixArray<decltype(n)::value>(itr, itrE, id, transpose));
				});
			}
		};
		struct TexSingle {
			static void DCmd(draw::IQueue& q, const HTexC& tex, const int id, const int actId) {
				tex->dcmd_export(q, id, actId);
			}
		};
		struct TexArray {
			template <class Itr>
			static void DCmd(draw::IQueue& q, Itr itr, const Itr itrE, const int id, const int actId) {
				std::size_t idx = 0;
				while(itr != itrE) {
					(*itr)->dcmd_export(q, id+idx, actId+idx);
					++idx;
					++itr;
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
	class UniformEnt {
		private:
			const GLProgram&	_program;
			draw::IQueue&		_q;

		public:
			UniformEnt(const GLProgram& p, draw::IQueue& q);
			const GLProgram& getProgram() const noexcept;

			template <class... Ts>
			void setUniform(const SName& name, Ts&&... ts) {
				if(const auto id = _program.getUniformId(name)) {
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
				detail::VecSingle<vec_t>::DCmd(_q, t, id);
			}
			template <class V, ENABLE_IF(frea::is_vector<V>{})>
			void setUniformById(const GLint id, const V& v) {
				detail::VecSingle<V>::DCmd(_q, v, id);
			}
			template <class M, ENABLE_IF(frea::is_matrix<M>{})>
			void setUniformById(const GLint id, const M& m, const bool transpose=true) {
				detail::MatSingle<M>::DCmd(_q, m, id, transpose);
			}
			template <class T>
			void setUniformById(const GLint id, const std::shared_ptr<T>& t) {
				// テクスチャユニット番号を検索
				const auto num = _program.getTexIndex(id);
				D_Assert0(num);
				detail::TexSingle::DCmd(_q, t, id, *num);
			}

			template <class T, class... Ts>
			void setUniformById(const GLint id, const std::vector<T>& t, Ts&&... ts) {
				setUniformById(id, t.begin(), t.end(), std::forward<Ts>(ts)...);
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
				std::vector<vec_t> tmp(itr, itrE);
				detail::VecArray<vec_t>::DCmd(_q, tmp.begin(), tmp.end(), id);
			}
			template <
				class Itr,
				class V = std::decay_t<decltype(*std::declval<Itr>())>,
				ENABLE_IF(frea::is_vector<V>{})
			>
			void setUniformById(const GLint id, const Itr itr, const Itr itrE) {
				detail::VecArray<V>::DCmd(_q, itr, itrE, id);
			}
			template <
				class Itr,
				class V = std::decay_t<decltype(*std::declval<Itr>())>,
				ENABLE_IF(frea::is_matrix<V>{})
			>
			void setUniformById(const GLint id, const Itr itr, const Itr itrE, const bool transpose=true) {
				detail::MatArray<V>::DCmd(_q, itr, itrE, id, transpose);
			}
			template <
				class Itr,
				class V = std::decay_t<decltype(*std::declval<Itr>())>,
				ENABLE_IF(detail::Is_SP<V>{})
			>
			void setUniformById(const GLint id, const Itr itr, const Itr itrE) {
				// テクスチャユニット番号を検索
				const auto num = _program.getTexIndex(id);
				D_Assert0(num);
				detail::TexArray::DCmd(_q, itr, itrE, id, *num);
			}
	};
}
