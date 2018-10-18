#pragma once
#include "../../handle/camera.hpp"
#include "lubee/src/rect.hpp"
#include "qm_usem_if.hpp"
#include "../../cache.hpp"

namespace rev::gltf::v1 {
	class QueryMatrix_USemCached :
		public IQueryMatrix_USem
	{
		private:
			const dc::IQueryMatrix&		_qm;

			using Mat4 = frea::Mat4;
			using Vec4 = frea::Vec4;
			struct Hash {
				template <class T>
				std::size_t operator()(const T& t) const noexcept {
					return t.getHash();
				}
			};
			// ----------------- for USemantics -----------------
			struct USemKey {
				dc::JointId		jointId;
				USemantic		sem;

				std::size_t getHash() const noexcept;
				bool operator == (const USemKey& k) const noexcept;
			};
			using USemM = Cache<USemKey, Mat4, Hash>;
			mutable USemM	_matrix;

			HCam3			_camera;
			Mat4			_view,
							_proj,
							_viewProj;
			// ----------------- for Viewport -----------------
			// Viewport
			Vec4			_viewport;

			struct SkinKey {
				dc::JointId		jointId;
				SkinBindSet_SP	bind;

				std::size_t getHash() const noexcept;
				bool operator == (const SkinKey& k) const noexcept;
			};
			using SkinM = Cache<SkinKey, Mat4V, Hash>;
			mutable SkinM	_skin;

		public:
			QueryMatrix_USemCached(const HCam3& cam, const lubee::RectF& vp, const dc::IQueryMatrix& qm);

			void exportSemantic(ISemanticSet& s, JointId id, const SkinBindSet_SP& bind, USemantic sem) const override;

			dc::Mat4 getLocal(JointId id) const override;
			dc::Mat4 getGlobal(JointId id) const override;
			dc::Mat4 getLocal(const dc::SName& name) const override;
			dc::Mat4 getGlobal(const dc::SName& name) const override;
	};
}
