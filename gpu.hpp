#pragma once
#include <unordered_set>
#include "gl_types.hpp"

namespace std {
	template <class>
	class sub_match;
	template <class, class>
	class match_results;
	using cmatch = match_results<
						const char*,
						std::allocator<std::sub_match<const char*>>>;
	using smatch = match_results<
						std::string::const_iterator,
						std::allocator<std::sub_match<std::string::const_iterator>>>;
}
namespace rev {
	// OpenGLES2では同期オブジェクトが無いのでGPUTimeは無効
	#ifndef USE_OPENGLES2
		//! フレームレート表示
		/*! 毎フレームテキストハンドルを作り直す */
		class GPUTime : public IGLResource {
			private:
				GLuint		_idQuery[2];
				GLsync		_idSync[2];
				int			_cursor;
				GLuint64	_prevTime;
			public:
				GPUTime();
				~GPUTime();
				void onFrameBegin();
				void onFrameEnd();
				GLuint64 getTime() const;

				void onDeviceLost() override;
				void onDeviceReset() override;
		};
	#endif
	//! GPUの情報表示
	class GPUInfo : public IGLResource {
		private:
			struct Version {
				union {
					struct {
						int major,
							minor,
							revision;
					};
					int ar[3];
				};
				void clear();
				void loadFromRegex(const std::cmatch& m);
				void loadFromRegex(const std::smatch& m);
			};
			friend std::ostream& operator << (std::ostream& os, const GPUInfo::Version& ver);
			enum class Profile {
				Core,
				Compatibility,
				Unknown
			};
			using CapSet = std::unordered_set<std::string>;
			CapSet		_capSet;
			Version		_verGL,		//!< OpenGL version
						_verSL,		//!< GLSL version
						_verDriver;
			std::string	_strVendor,
						_strRenderer;
			Profile		_profile;

		public:
			const Version& glslVersion() const;
			const Version& version() const;
			const Version& driverVersion() const;
			const std::string& vendor() const;
			const std::string& renderer() const;
			const CapSet& refCapabilitySet() const;

			void onDeviceLost() override;
			void onDeviceReset() override;
			friend std::ostream& operator << (std::ostream& os, const GPUInfo& info);
	};
	std::ostream& operator << (std::ostream& os, const GPUInfo::Version& ver);
	std::ostream& operator << (std::ostream& os, const GPUInfo& info);
}
