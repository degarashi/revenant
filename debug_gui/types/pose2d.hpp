#pragma once

namespace beat {
	namespace g2 {
		class Pose;
	}
}
namespace rev {
	namespace debug {
		class Pose2DC {
			private:
				const beat::g2::Pose& _p;
			public:
				Pose2DC(const beat::g2::Pose& p);
				void show() const;
		};
		class Pose2D : public Pose2DC {
			private:
				beat::g2::Pose& _p;
			public:
				Pose2D(beat::g2::Pose& p);
				bool edit() const;
		};
	}
}
