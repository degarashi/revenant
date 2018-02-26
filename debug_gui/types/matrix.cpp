#include "matrix.hpp"

namespace rev {
	namespace debug {
		namespace inner {
			template <class M, ENABLE_IF(frea::is_matrix<M>{})>
			void _Show(const M& m) {
				MatrixC<M>(m).show();
			}
			template void _Show(const frea::Mat2& v);
			template void _Show(const frea::Mat3& v);
			template void _Show(const frea::Mat4& v);
			template void _Show(const frea::AMat2& v);
			template void _Show(const frea::AMat3& v);
			template void _Show(const frea::AMat4& v);

			template <class M, ENABLE_IF(frea::is_matrix<M>{})>
			bool _Edit(M& m) {
				return FMatrix<M>(m).edit();
			}
			template bool _Edit<frea::Mat2>(frea::Mat2& v);
			template bool _Edit<frea::Mat3>(frea::Mat3& v);
			template bool _Edit<frea::Mat4>(frea::Mat4& v);
			template bool _Edit<frea::AMat2>(frea::AMat2& v);
			template bool _Edit<frea::AMat3>(frea::AMat3& v);
			template bool _Edit<frea::AMat4>(frea::AMat4& v);
		}
	}
}
