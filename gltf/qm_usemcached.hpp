#pragma once
#include "../handle/camera.hpp"
#include "lubee/rect.hpp"
#include "qm_usem_if.hpp"

namespace rev::gltf {
	class QueryMatrix_USemCached :
		public IQueryMatrix_USem
	{
		private:
			const dc::IQueryMatrix&		_qm;

			using Mat4 = frea::Mat4;
			using Vec4 = frea::Vec4;
			// ----------------- for USemantics -----------------
			struct USemKey {
				dc::JointId		jointId;
				USemantic		sem;

				std::size_t operator()(const USemKey& k) const noexcept;
				bool operator == (const USemKey& k) const noexcept;
			};
			using USemM = std::unordered_map<USemKey, Mat4, USemKey>;
			mutable USemM	_matrix;

			HCam3			_camera;
			Mat4			_view,
							_proj,
							_viewProj;
			// ----------------- for Viewport -----------------
			// Viewport
			Vec4			_viewport;
			Mat4 _calcMat(JointId id, USemantic sem) const;
			mutable Mat4V _jointMat;
			const Mat4V& _getJointMat(JointId id, const SkinBindSet_SP& bind) const;

		public:
			QueryMatrix_USemCached(const HCam3& cam, const lubee::RectF& vp, const dc::IQueryMatrix& qm);

			void exportSemantic(ISemanticSet& s, JointId id, const SkinBindSet_SP& bind, USemantic sem) const override;

			dc::Mat4 getLocal(JointId id) const override;
			dc::Mat4 getGlobal(JointId id) const override;
			dc::Mat4 getLocal(const dc::SName& name) const override;
			dc::Mat4 getGlobal(const dc::SName& name) const override;
	};
}
