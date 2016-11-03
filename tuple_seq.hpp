#pragma once

//! tupleのsequenceを定義
#define MAKE_SEQ(size, rel) MAKE_SEQ_D(size, rel)
#define MAKE_SEQ_D(size, rel) \
	BOOST_PP_CAT( \
		MAKE_SEQ_A_ ## size rel, \
		0X0 \
	)() \
	/**/

// size 2
#define MAKE_SEQ_A_2(x, y) ((x, y)) MAKE_SEQ_B_2
#define MAKE_SEQ_B_2(x, y) ((x, y)) MAKE_SEQ_A_2
// size 3
#define MAKE_SEQ_A_3(x, y, z) ((x, y, z)) MAKE_SEQ_B_3
#define MAKE_SEQ_B_3(x, y, z) ((x, y, z)) MAKE_SEQ_A_3
// size 4
#define MAKE_SEQ_A_4(x, y, z, w) ((x, y, z, w)) MAKE_SEQ_B_4
#define MAKE_SEQ_B_4(x, y, z, w) ((x, y, z, w)) MAKE_SEQ_A_4

#define MAKE_SEQ_A_20X0()
#define MAKE_SEQ_B_20X0()
#define MAKE_SEQ_A_30X0()
#define MAKE_SEQ_B_30X0()
#define MAKE_SEQ_A_40X0()
#define MAKE_SEQ_B_40X0()
