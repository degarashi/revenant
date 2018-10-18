#pragma once
#include "luaimport.hpp"
#include "frea/src/matrix.hpp"
#include "frea/src/expquat.hpp"
#include "frea/src/fwd.hpp"

DEF_LUAIMPORT(frea::Vec2)
DEF_LUAIMPORT(frea::Vec3)
DEF_LUAIMPORT(frea::Vec4)
DEF_LUAIMPORT(frea::AVec2)
DEF_LUAIMPORT(frea::AVec3)
DEF_LUAIMPORT(frea::AVec4)

DEF_LUAIMPORT(frea::Mat2)
DEF_LUAIMPORT(frea::Mat3)
DEF_LUAIMPORT(frea::Mat4)
DEF_LUAIMPORT(frea::AMat2)
DEF_LUAIMPORT(frea::AMat3)
DEF_LUAIMPORT(frea::AMat4)

DEF_LUAIMPORT(frea::Quat)
DEF_LUAIMPORT(frea::AQuat)

DEF_LUAIMPORT(frea::Plane)
DEF_LUAIMPORT(frea::APlane)

DEF_LUAIMPORT(frea::DegF)
DEF_LUAIMPORT(frea::RadF)

DEF_LUAIMPORT(frea::ExpQuat)
DEF_LUAIMPORT(frea::AExpQuat)

#include "lubee/src/random.hpp"
DEF_LUAIMPORT(lubee::RandomMT)
