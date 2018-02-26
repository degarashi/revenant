#pragma once

namespace rev {
	class Camera2D;
	namespace debug {
		class Camera2DC {
			private:
				const ::rev::Camera2D& _c;
			public:
				Camera2DC(const ::rev::Camera2D& c);
				void show() const;
		};
		class Camera2D : public Camera2DC {
			private:
				::rev::Camera2D& _c;
			public:
				Camera2D(::rev::Camera2D& c);
				bool edit() const;
		};
	}
}
