#pragma once
#include "gl_types.hpp"
#include "glx_id.hpp"
#include "differential.hpp"
#include "drawtoken/matrix.hpp"

namespace lubee {
	template <class T>
	class Rect;
	using RectI = Rect<int32_t>;
	using RectF = Rect<float>;
}
namespace rev {
	class VDecl;
	using VDecl_SP = std::shared_ptr<VDecl>;
	namespace draw {
		struct ClearParam;
	}

	class SystemUniform2D;
	class SystemUniform3D;
	struct IEffect : IGLResource {
		struct tagConstant {};
		using GlxId = rev::IdMgr_Glx<tagConstant>;
		// //! Uniform & TechPass 定数にIdを割り当てるクラス
		// static GlxId	s_myId;

		virtual GLint_OP getTechId(const std::string& tech) const = 0;
		virtual GLint_OP getPassId(const std::string& pass) const = 0;
		virtual GLint_OP getPassId(const std::string& tech, const std::string& pass) const = 0;
		virtual GLint_OP getCurTechId() const = 0;
		virtual GLint_OP getCurPassId() const = 0;
		virtual void setTechPassId(IdValue id) = 0;
		virtual HProg getProgram(int techId=-1, int passId=-1) const = 0;
		virtual void setTechnique(int id, bool bReset) = 0;
		virtual void setPass(int id) = 0;
		virtual void setFramebuffer(const HFb& fb) = 0;
		virtual HFb getFramebuffer() const = 0;
		virtual void setViewport(bool bPixel, const lubee::RectF& r) = 0;
		virtual void resetFramebuffer() = 0;
		virtual void setVDecl(const VDecl_SP& decl) = 0;
		virtual void setVStream(const HVb& vb, int n) = 0;
		virtual void setIStream(const HIb& ib) = 0;
		virtual GLint_OP getUniformId(const std::string& name) const = 0;
		virtual GLint_OP getUnifId(IdValue id) const = 0;
		virtual void clearFramebuffer(const draw::ClearParam& param) = 0;
		virtual void drawIndexed(GLenum mode, GLsizei count, GLuint offsetElem=0) = 0;
		virtual void draw(GLenum mode, GLint first, GLsizei count) = 0;
		virtual void beginTask() = 0;
		virtual void endTask() = 0;
		virtual void execTask() = 0;
		virtual diff::Effect getDifference() const = 0;

		virtual draw::TokenBuffer& _makeUniformTokenBuffer(GLint id) = 0;
		template <class UT, class... Ts>
		static void MakeUniformToken(draw::TokenDst& dst, GLint /*id*/, Ts&&... ts) {
			new(dst.allocate_memory(sizeof(UT), draw::CalcTokenOffset<UT>())) UT(std::forward<Ts>(ts)...);
		}
		//! 定数値を使ったUniform変数設定。Uniform値が存在しなくてもエラーにならない
		template <bool Check, class... Ts>
		void _setUniformById(const IdValue id, Ts&&... ts) {
			if(auto idv = getUnifId(id))
				setUniform(*idv, std::forward<Ts>(ts)...);
			else {
				// 定数値に対応するUniform変数が見つからない時は警告を出す
				Expect(!Check, "Uniform-ConstantId: %s not found", std::to_string(id._value).c_str());
			}
		}

		//! 定数値を使ったUniform変数設定
		/*! clang補完の為の引数明示 -> _setUniform(...) */
		template <bool Check=true, class T>
		void setUniform(const IdValue id, T&& t, const bool bT=false) {
			_setUniformById<Check>(id, std::forward<T>(t), bT); }
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
		// clang補完の為の引数明示 -> _setUniform(...)
		template <bool Check=true, class T>
		void setUniform(const IdValue id, const T* t, const int n, const bool bT=false) {
			_setUniformById<Check>(id, t, n, bT);
		}
		//! ベクトルUniform変数
		template <
			class V,
			ENABLE_IF((
				frea::is_vector<V>{}
			))
		>
		void _makeUniformToken(draw::TokenDst& dst, const GLint id, const V* v, const int n, const bool) const {
			MakeUniformToken<draw::Unif_Vec<typename V::value_t, V::size>>(dst, id, id, v, n);
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
			MakeUniformToken<draw::Unif_Mat<typename M::value_t, M::dim_m>>(dst, id, id, m, n, bT);
		}

		virtual void _makeUniformToken(draw::TokenDst& dst, GLint id, const bool* b, int n, bool) const = 0;
		virtual void _makeUniformToken(draw::TokenDst& dst, GLint id, const float* fv, int n, bool) const = 0;
		virtual void _makeUniformToken(draw::TokenDst& dst, GLint id, const double* fv, int n, bool) const = 0;
		virtual void _makeUniformToken(draw::TokenDst& dst, GLint id, const int* iv, int n, bool) const = 0;
		virtual void _makeUniformToken(draw::TokenDst& dst, GLint id, const HTex* hTex, int n, bool) const = 0;

		virtual SystemUniform2D& ref2D() {
			AssertF("this class has no SystemUniform2D interface");
		}
		virtual SystemUniform3D& ref3D() {
			AssertF("this class has no SystemUniform3D interface");
		}
		const SystemUniform2D& ref2D() const {
			return const_cast<IEffect*>(this)->ref2D();
		}
		const SystemUniform3D& ref3D() const {
			return const_cast<IEffect*>(this)->ref3D();
		}
		virtual void moveFrom(IEffect& /*prev*/) {}
	};
}
