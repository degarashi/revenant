#pragma once

#ifdef USE_OPENGLES2
	#define GLDEPTHRANGE glDepthRangef
#else
	#define GLDEPTHRANGE glDepthRange
#endif
