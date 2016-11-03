#pragma once
#include "vector.hpp"
#include "frea/matrix.hpp"

namespace rev {
	namespace draw {
		void Unif_Mat_Exec(int idx, GLint id, const void* ptr, int n, bool bT);
		template <class T, int DN>
		class Unif_Mat : public Unif_Vec<T,DN> {
			private:
				using base_t = Unif_Vec<T,DN>;
				bool	_bT;
			public:
				template <class M, ENABLE_IF(frea::is_matrix<M>{})>
				Unif_Mat(const GLint id, const M& m, const bool bT):
					Unif_Mat(id, &m, 1, bT)
				{}
				template <class M, ENABLE_IF(frea::is_matrix<M>{} && M::dim_m==M::dim_n)>
				Unif_Mat(const GLint id, const M* mp, const int n, const bool bT):
					base_t(id, mp->data, M::dim_m*M::dim_m, n),
					_bT(bT)
				{}
				void exec() override {
					Unif_Mat_Exec(DN-2, base_t::idUnif, base_t::_data.get(), base_t::_nAr, _bT);
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
}
