#pragma once
#include "drawtoken_t.hpp"
#include "frea/matrix.hpp"

namespace rev::draw {
	void Unif_Mat_Exec(std::size_t idx, GLint id, const void* ptr, std::size_t n, bool bT);

	// Matrix配列
	template <class T, std::size_t Dim>
	class Unif_MatA : public Uniform<Unif_MatA<T,Dim>> {
		private:
			using base_t = Uniform<Unif_MatA<T,Dim>>;
			using mat_t = frea::Mat_t<T, Dim, Dim, false>;
			using mat_v = std::vector<mat_t>;
			mat_v		_data;

		public:
			template <class Itr>
			Unif_MatA(const Itr itr, const Itr itrE):
				_data(itr, itrE)
			{}
			template <class M>
			Unif_MatA(const std::vector<M>& src):
				Unif_MatA(src.cbegin(), src.cend())
			{}
			Unif_MatA(std::vector<mat_t>&& src):
				_data(std::move(src))
			{}
			void exec() override {
				Unif_Mat_Exec(Dim-2, base_t::idUnif, _data.data(), _data.size(), true);
			}
			bool isArray() const noexcept override {
				return true;
			}
	};
	// Matrix単体
	template <class T, std::size_t Dim>
	class Unif_Mat : public Uniform<Unif_Mat<T,Dim>> {
		private:
			using base_t = Uniform<Unif_Mat<T,Dim>>;
			using mat_t = frea::Mat_t<T, Dim, Dim, false>;
			mat_t		_data;

		public:
			template <class M>
			Unif_Mat(const M& m):
				_data(m)
			{}
			void exec() override {
				Unif_Mat_Exec(Dim-2, base_t::idUnif, &_data.m[0][0], 1, true);
			}
			bool isArray() const noexcept override {
				return false;
			}
	};
}
			// constexpr int DIM = lubee::Arithmetic<M::dim_m, M::dim_n>::great;
			// std::vector<frea::Mat_t<typename M::value_t,DIM,DIM,false>> tm(nvalue);
			// for(std::size_t i=0 ; i<nvalue ; i++)
				// tm[i] = m[i].template convert<DIM,DIM>();
			// return MakeUniform(tm.data(), nvalue);
