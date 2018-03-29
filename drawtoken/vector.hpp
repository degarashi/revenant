#pragma once
#include "uniform.hpp"
#include "frea/vector.hpp"

#ifdef DEBUGGUI_ENABLED
	#include "../debug_gui/print.hpp"
#endif

namespace rev::draw {
	void Unif_Vec_Exec(std::size_t idx, GLint id, const void* ptr, std::size_t n);

	namespace detail {
		template <class T, std::size_t Dim>
		constexpr std::size_t CalcIndex() {
			std::size_t idx=0;
			if constexpr (std::is_integral<T>{}) {
				if constexpr (std::is_unsigned<T>{}) {
					idx = 8;
				} else
					idx = 4;
			} else
				idx = 0;

			idx += Dim-1;
			return idx;
		}
	}
	// Vector配列
	template <class T, std::size_t Dim>
	class Unif_VecA : public Uniform<Unif_VecA<T,Dim>> {
		private:
			using base_t = Uniform<Unif_VecA<T,Dim>>;
			using vec_t = frea::Vec_t<T, Dim, false>;
			using vec_v = std::vector<vec_t>;
			vec_v	_data;

		public:
			template <class Itr>
			Unif_VecA(const Itr itr, const Itr itrE):
				_data(itr, itrE)
			{}
			template <class V>
			Unif_VecA(const std::vector<V>& src):
				Unif_VecA(src.cbegin(), src.cend())
			{}
			Unif_VecA(std::vector<vec_t>&& src):
				_data(std::move(src))
			{}
			void exec() override {
				constexpr auto idx = detail::CalcIndex<T,Dim>();
				Unif_Vec_Exec(idx, base_t::idUnif, _data.data(), _data.size());
			}
			bool isArray() const noexcept override {
				return true;
			}
			#ifdef DEBUGGUI_ENABLED
				void printValue() const override {
					debug::Show(nullptr, _data.cbegin(), _data.cend());
				}
			#endif
	};
	template <class T>
	class Unif_VecA<T, 1> : public Uniform<Unif_VecA<T, 1>> {
		private:
			using base_t = Uniform<Unif_VecA<T, 1>>;
			using Data = std::vector<T>;
			Data	_data;

		public:
			template <class Itr>
			Unif_VecA(const Itr itr, const Itr itrE):
				_data(itr, itrE)
			{}
			template <class V>
			Unif_VecA(const std::vector<V>& src):
				Unif_VecA(src.cbegin(), src.cend())
			{}
			Unif_VecA(std::vector<T>&& src):
				_data(std::move(src))
			{}

			void exec() override {
				constexpr auto idx = detail::CalcIndex<T,1>();
				Unif_Vec_Exec(idx, base_t::idUnif, _data.data(), _data.size());
			}
			bool isArray() const noexcept override {
				return true;
			}
			#ifdef DEBUGGUI_ENABLED
				void printValue() const override {
					debug::Show(nullptr, _data.cbegin(), _data.cend());
				}
			#endif
	};
	// Vector単体
	template <class T, std::size_t Dim>
	class Unif_Vec : public Uniform<Unif_Vec<T,Dim>> {
		private:
			using base_t = Uniform<Unif_Vec<T,Dim>>;
			using vec_t = frea::Vec_t<T, Dim, false>;
			vec_t	_data;

		public:
			template <class V>
			Unif_Vec(const V& v):
				_data(v)
			{}
			void exec() override {
				constexpr auto idx = detail::CalcIndex<T,Dim>();
				Unif_Vec_Exec(idx, base_t::idUnif, &_data, 1);
			}
			bool isArray() const noexcept override {
				return false;
			}
			#ifdef DEBUGGUI_ENABLED
				void printValue() const override {
					debug::Show(nullptr, _data);
				}
			#endif
	};
	template <class T>
	class Unif_Vec<T,1> : public Uniform<Unif_Vec<T,1>> {
		private:
			using base_t = Uniform<Unif_Vec<T,1>>;
			T		_data;
		public:
			template <class V>
			Unif_Vec(const V& t):
				_data(t.x)
			{}
			Unif_Vec(const T& t):
				_data(t)
			{}
			void exec() override {
				constexpr auto idx = detail::CalcIndex<T,1>();
				Unif_Vec_Exec(idx, base_t::idUnif, &_data, 1);
			}
			bool isArray() const noexcept override {
				return false;
			}
			#ifdef DEBUGGUI_ENABLED
				void printValue() const override {
					debug::Show(nullptr, _data);
				}
			#endif
	};
}
