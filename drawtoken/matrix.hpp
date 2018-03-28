#pragma once
#include "vector.hpp"
#include "frea/matrix.hpp"

namespace rev::draw {
	void Unif_Mat_Exec(std::size_t idx, GLint id, const void* ptr, std::size_t n, bool bT);

	// 要素Tによる行の幅がDimの行列
	template <class T, std::size_t Dim>
	class Unif_Mat : public Unif_Vec<T,Dim> {
		private:
			using base_t = Unif_Vec<T,Dim>;
			bool	_bT;
		public:
			// Matrix単体
			template <class M>
			Unif_Mat(const M& m, const bool bT):
				Unif_Mat(&m, 1, bT)
			{}
			template <
				class M,
				ENABLE_IF(
					frea::is_matrix<M>{} &&
					M::dim_m == Dim &&
					M::dim_n == Dim
				)
			>
			Unif_Mat(const M* mp, const std::size_t n, const bool bT):
				base_t(mp->m, n*Dim),
				_bT(bT)
			{}
			void exec() override {
				D_Assert0(base_t::_nAr % Dim == 0);
				Unif_Mat_Exec(Dim-2, base_t::idUnif, base_t::_data.get(), base_t::_nAr/Dim, _bT);
			}
			void clone(TokenDst& dst) const override {
				new(dst.allocate_memory(getSize(), draw::CalcTokenOffset<Unif_Mat>())) Unif_Mat(*this);
			}
			void takeout(TokenDst& dst) override {
				new(dst.allocate_memory(getSize(), draw::CalcTokenOffset<Unif_Mat>())) Unif_Mat(std::move(*this));
			}
			std::size_t getSize() const override {
				return sizeof(*this);
			}
	};
}
