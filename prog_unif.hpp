#pragma once
#include "drawtoken/matrix.hpp"
#include "drawtoken/tokenbuffer.hpp"
#include "uniform_map.hpp"
#include "gl_types.hpp"

namespace rev {
	class Prog_Unif {
		private:
			UniformMap	_uniValue;
			HProg		_program;

			//! ベクトルUniform変数
			template <
				class V,
				ENABLE_IF((
					frea::is_vector<V>{}
				))
			>
			void _makeUniformToken(draw::TokenDst& dst, const GLint id, const V* v, const int n, const bool) const {
				_MakeUniformToken<draw::Unif_Vec<typename V::value_t, V::size>>(dst, id, id, v, n);
			}
			//! 行列Uniform変数(非正方形)
			template <
				class M,
				ENABLE_IF((
					frea::is_matrix<M>{} &&
					(M::dim_m != M::dim_n)
				))
			>
			void _makeUniformToken(draw::TokenDst& dst, const GLint id, const M* m, const int n, const bool bT) const {
				constexpr int DIM = lubee::Arithmetic<M::dim_m, M::dim_n>::great;
				std::vector<frea::Mat_t<typename M::value_t,DIM,DIM,false>> tm(n);
				for(int i=0 ; i<n ; i++)
					tm[i] = m[i].template convert<DIM,DIM>();
				_makeUniformToken(dst, id, tm.data(), n, bT);
			}
			//! 行列Uniform変数(正方形)
			template <
				class M,
				ENABLE_IF((
					frea::is_matrix<M>{} &&
					(M::dim_m == M::dim_n)
				))
			>
			void _makeUniformToken(draw::TokenDst& dst, const GLint id, const M* m, const int n, const bool bT) const {
				_MakeUniformToken<draw::Unif_Mat<typename M::value_t, M::dim_m>>(dst, id, id, m, n, bT);
			}
			void _makeUniformToken(draw::TokenDst& dst, GLint id, const bool* b, int n, bool) const;
			void _makeUniformToken(draw::TokenDst& dst, GLint id, const float* fv, int n, bool) const;
			void _makeUniformToken(draw::TokenDst& dst, GLint id, const double* fv, int n, bool) const;
			void _makeUniformToken(draw::TokenDst& dst, GLint id, const int* iv, int n, bool) const;
			void _makeUniformToken(draw::TokenDst& dst, GLint id, const HTex* hTex, int n, bool) const;
			template <class UT, class... Ts>
			static void _MakeUniformToken(draw::TokenDst& dst, GLint /*id*/, Ts&&... ts) {
				new(dst.allocate_memory(sizeof(UT), draw::CalcTokenOffset<UT>())) UT(std::forward<Ts>(ts)...);
			}
			draw::TokenBuffer& _makeUniformTokenBuffer(GLint id);
		protected:
			UniformMap& _refUniformValue() noexcept;

		public:
			void setProgram(const HProg& p);
			const HProg& getProgram() const noexcept;
			void clearUniformValue();
			const UniformMap& getUniformValue() const noexcept;

			GLint_OP getUniformId(const Name& name) const;
			template <class... Ts>
			void setUniform(const Name& name, const Ts&... ts) {
				setUniform(*getUniformId(name), ts...);
			}
			//! 指定された名前の変数があればセットする(なくてもエラーにはしない)
			template <class... Ts>
			void setUniform_if(const Name& name, const Ts&... ts) {
				if(const auto id = getUniformId(name))
					setUniform(*id, ts...);
			}
			//! 単体Uniform変数セット
			template <
				class T,
				ENABLE_IF((
					!std::is_pointer<T>{}
				))
			>
			void setUniform(const GLint id, const T& t, const bool bT=false) {
				setUniform(id, &t, 1, bT);
			}
			//! 配列Uniform変数セット
			template <class T>
			void setUniform(const GLint id, const T* t, const int n, const bool bT=false) {
				_makeUniformToken(_makeUniformTokenBuffer(id), id, t, n, bT);
			}
	};
}
