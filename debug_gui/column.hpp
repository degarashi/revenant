#pragma once

namespace rev {
	namespace debug {
		class ColumnPush {
			private:
				int		_prev,
						_n;
			public:
				ColumnPush(int n, bool border=true);
				ColumnPush(ColumnPush&& c);
				~ColumnPush();
		};
	}
}
