#include "matrix.hpp"

namespace rev::debug::inner {
	template <class M>
	void _ShowMatrix(const M& m) {
		MatrixC<M>(m).show();
	}
	template void _ShowMatrix(const frea::Mat2& v);
	template void _ShowMatrix(const frea::Mat3& v);
	template void _ShowMatrix(const frea::Mat4& v);

	template <class M>
	bool _EditMatrix(M& m) {
		return FMatrix<M>(m).edit();
	}
	template bool _EditMatrix<frea::Mat2>(frea::Mat2& v);
	template bool _EditMatrix<frea::Mat3>(frea::Mat3& v);
	template bool _EditMatrix<frea::Mat4>(frea::Mat4& v);
}
