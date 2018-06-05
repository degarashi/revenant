#include "value_loader.hpp"

namespace rev::gltf {
	namespace loader {
		const JValue& GetRequiredEntry(const JValue& v, const char* key) {
			if(const auto r = GetOptionalEntry(v, key))
				return *r;
			throw LackOfPrerequisite(key);
		}
		const JValue& GetOptionalEntryDefault(const JValue& v, const char* key, const JValue& def) {
			if(const auto r = GetOptionalEntry(v, key))
				return *r;
			return def;
		}
		spi::Optional<const JValue&> GetOptionalEntry(const JValue& v, const char* key) {
			const auto itr = v.FindMember(key);
			if(itr != v.MemberEnd())
				return itr->value;
			return spi::none;
		}
		Null::Null(const JValue& v) {
			if(!CanLoad(v))
				throw InvalidProperty("can't read as null");
		}
		bool Null::CanLoad(const JValue& v) noexcept {
			return v.IsNull();
		}
		Bool::Bool(const JValue& v) {
			if(!CanLoad(v))
				throw InvalidProperty("can't read as boolean");
			this->_value = v.GetBool();
		}
		bool Bool::CanLoad(const JValue& v) noexcept {
			return v.IsBool();
		}
		Number::Number(const JValue& v) {
			if(!CanLoad(v))
				throw InvalidProperty("can't read as number");
			this->_value = v.GetDouble();
		}
		bool Number::CanLoad(const JValue& v) noexcept {
			return v.IsNumber();
		}
		Float::Float(const JValue& v) {
			if(!CanLoad(v))
				throw InvalidProperty("can't read as number");
			this->_value = v.GetFloat();
		}
		bool Float::CanLoad(const JValue& v) noexcept {
			return v.IsNumber();
		}
		Integer::Integer(const JValue& v) {
			if(!CanLoad(v))
				throw InvalidProperty("can't read as integer");
			this->_value = v.GetInt64();
		}
		bool Integer::CanLoad(const JValue& v) noexcept {
			return v.IsInt64();
		}
		GLEnum::GLEnum(const JValue& v) {
			this->_value = static_cast<GLEnum>(Integer(v));
		}
		bool GLEnum::CanLoad(const JValue& v) noexcept {
			return Integer::CanLoad(v);
		}
		String::String(const JValue& v) {
			if(!CanLoad(v))
				throw InvalidProperty("can't read as string");
			this->_value = v.GetString();
		}
		bool String::CanLoad(const JValue& v) noexcept {
			return v.IsString();
		}
		StdString::StdString(const JValue& v) {
			this->_value = String(v);
		}
		bool StdString::CanLoad(const JValue& v) noexcept {
			return String::CanLoad(v);
		}
		Quat::Quat(const JValue& v) {
			if(!CanLoad(v))
				throw InvalidProperty("can't read as quat");
			const Vec4 vec(v);
			x = vec.x;
			y = vec.y;
			z = vec.z;
			w = vec.w;
			if(std::abs(length() - 1.f) > 1e-3f)
				throw InvalidProperty("non-normalized quaternion");
		}
		bool Quat::CanLoad(const JValue& v) noexcept {
			return Vec4::CanLoad(v);
		}
		Pose3::Pose3(const JValue& v) {
			if(!CanLoad(v))
				throw InvalidProperty("can't read as pose");
			if(auto mat = Optional<gltf::loader::Mat4>(v, "matrix")) {
				// 読み取った値は右からベクトルを掛けるタイプなので転置する
				mat->transpose();
				static_cast<value_t&>(*this) = value_t(mat->convert<4,3>());
			}
			else if(v.HasMember("translation")) {
				static_cast<value_t&>(*this) = value_t(
					OptionalDefault<loader::Vec3>(v, "translation", {0,0,0}),
					OptionalDefault<loader::Quat>(v, "rotation", Quat::Identity()),
					OptionalDefault<loader::Vec3>(v, "scale", {0,0,0})
				);
			} else
				identity();
		}
		bool Pose3::CanLoad(const JValue& /*v*/) noexcept {
			return true;
		}
	}
}
