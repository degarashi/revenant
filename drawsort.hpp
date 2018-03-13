#pragma once
#include "handle.hpp"
#include "drawtag.hpp"
#include "debuggui_if.hpp"
#include "spine/enum.hpp"
#include <vector>

namespace rev {
	struct DSort;
	using DSort_SP = std::shared_ptr<DSort>;
	using DSort_V = std::vector<DSort_SP>;
	using DLObj_P = std::pair<const DrawTag*, HDObj>;
	using DLObj_V = std::vector<DLObj_P>;

	class IEffect;
	// ---- Draw sort algorithms ----
	struct DSort : IDebugGui {
		//! ソートに必要な情報が記録されているか(デバッグ用)
		virtual bool hasInfo(const DrawTag& d) const = 0;
		virtual bool compare(const DrawTag& d0, const DrawTag& d1) const = 0;
		virtual void apply(const DrawTag& d, IEffect& e);
		static void DoSort(const DSort_V& alg, int cursor, typename DLObj_V::iterator itr0, typename DLObj_V::iterator itr1);
	};
	//! 描画ソート: Z距離の昇順
	struct DSort_Z_Asc : DSort {
		//! 無効とされる深度値のボーダー (これ以下は無効)
		const static float cs_border;
		bool hasInfo(const DrawTag& d) const override;
		bool compare(const DrawTag& d0, const DrawTag& d1) const override;
		DEF_DEBUGGUI_NAME
	};
	//! 描画ソート: Z距離の降順
	struct DSort_Z_Desc : DSort_Z_Asc {
		bool compare(const DrawTag& d0, const DrawTag& d1) const override;
		DEF_DEBUGGUI_NAME
	};
	//! 描画ソート: ユーザー任意の優先度値 昇順
	struct DSort_Priority_Asc : DSort {
		bool hasInfo(const DrawTag& d) const override;
		bool compare(const DrawTag& d0, const DrawTag& d1) const override;
		DEF_DEBUGGUI_NAME
	};
	//! 描画ソート: ユーザー任意の優先度値 降順
	struct DSort_Priority_Desc : DSort_Priority_Asc {
		bool compare(const DrawTag& d0, const DrawTag& d1) const override;
		DEF_DEBUGGUI_NAME
	};
	//! 描画ソート: Tech&Pass Id
	struct DSort_TechPass : DSort {
		bool hasInfo(const DrawTag& d) const override;
		bool compare(const DrawTag& d0, const DrawTag& d1) const override;
		void apply(const DrawTag& d, IEffect& e) override;
		DEF_DEBUGGUI_NAME
	};
	namespace detail {
		class DSort_UniformPairBase {
			private:
				IEffect*	_pFx = nullptr;
			protected:
				//! UniformIdがまだ取得されて無ければ or 前回と違うEffectの時にIdを更新
				void _refreshUniformId(IEffect& e, const std::string* name, int* id, std::size_t length);
		};
		template <std::size_t N>
		class DSort_UniformPair : public DSort_UniformPairBase {
			private:
				using ArStr = std::array<std::string, N>;
				using ArId = std::array<int, N>;
				//! Index -> Uniform名の対応表
				ArStr	_strUniform;
				ArId	_unifId;

				void _init(int) {}
				template <class T, class... Ts>
				void _init(int cursor, T&& t, Ts&&... ts) {
					Assert0(cursor < countof(_strUniform));
					_strUniform[cursor] = std::forward<T>(t);
					_init(++cursor, std::forward<Ts>(ts)...);
				}
			protected:
				constexpr static int length = N;
				const ArId& _getUniformId(IEffect& e) {
					_refreshUniformId(e, _strUniform.data(), _unifId.data(), N);
					return _unifId;
				}
				template <class... Ts>
				DSort_UniformPair(Ts&&... ts) {
					_init(0, std::forward<Ts>(ts)...);
				}
		};
	}
	//! 描画ソート: Texture
	class DSort_Texture :
		public DSort,
		public detail::DSort_UniformPair<std::tuple_size<DrawTag::TexAr>::value>
	{
		private:
			using base_t = detail::DSort_UniformPair<std::tuple_size<DrawTag::TexAr>::value>;
		public:
			using base_t::base_t;
			bool hasInfo(const DrawTag& d) const override;
			bool compare(const DrawTag& d0, const DrawTag& d1) const override;
			void apply(const DrawTag& d, IEffect& e) override;
			DEF_DEBUGGUI_NAME
	};
	//! 描画ソート: Primitive
	struct DSort_Primitive : DSort {
		bool hasInfo(const DrawTag& d) const override;
		bool compare(const DrawTag& d0, const DrawTag& d1) const override;
		void apply(const DrawTag& d, IEffect& e) override;
		DEF_DEBUGGUI_NAME
	};
	extern const DSort_SP	cs_dsort_z_asc,
							cs_dsort_z_desc,
							cs_dsort_priority_asc,
							cs_dsort_priority_desc,
							cs_dsort_techpass,
							cs_dsort_texture,
							cs_dsort_primitive;
	DefineEnum(
		SortAlg,
		(Z_Asc)
		(Z_Desc)
		(Priority_Asc)
		(Priority_Desc)
		(TechPass)
		(Texture)
		(Primitive)
	);
	using SortAlgList = std::vector<SortAlg>;
}
