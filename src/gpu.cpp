#include "gpu.hpp"
#include "gl/if.hpp"
#include "lubee/src/meta/countof.hpp"
#include <regex>
#include <boost/lexical_cast.hpp>

namespace rev {
	namespace {
		//! Regex: OpenGL version
		/*!	major, minor, revision */
		const std::regex re_version(R"((\d+)(?:\.(\d+))*)");
		//! Regex: GLSL Version
		/*!	[1,2] = GLSL version (major, minor) */
		const std::regex re_glsl(R"((\d+)\.(\d+))");
		//! Regex: Extensions...
		/*!	[1] = GL-Extension */
		const std::regex re_ext(R"([\w\d_]+)");

		template <class Itr, std::size_t N>
		void LoadFromRegex(Itr itr0, Itr itr1, int (&ar)[N]) {
			std::size_t cur = 0;
			for(; itr0<itr1 && cur<countof(ar) ; ++itr0,++cur)
				ar[cur] = boost::lexical_cast<int>(itr0->str());
		}
	}
	// ---------------------- GPUInfo::Version ----------------------
	void GPUInfo::Version::clear() {
		for(auto& a : ar)
			a = 0;
	}
	template <class M>
	void GPUInfo::Version::loadFromRegex(const M& m) {
		LoadFromRegex(m.cbegin()+1, m.cend(), ar);
	}
	std::ostream& operator << (std::ostream& os, const GPUInfo::Version& ver) {
		return os << ver.major << '.' << ver.minor << '-' << ver.revision;
	}
	// ---------------------- GPUInfo ----------------------
	void GPUInfo::onDeviceReset() {
		_strVendor = reinterpret_cast<const char*>(GL.glGetString(GL_VENDOR));
		_strRenderer = reinterpret_cast<const char*>(GL.glGetString(GL_RENDERER));

		_verGL.clear();
		_verDriver.clear();
		// バージョンの特定
		std::smatch m;
		std::string ver(reinterpret_cast<const char*>(GL.glGetString(GL_VERSION)));
		std::transform(ver.cbegin(), ver.cend(), ver.begin(), ::tolower);
		if(std::regex_search(ver, m, re_version)) {
			_verGL.loadFromRegex(m);

			// ドライバーバージョンの特定
			if(std::regex_search(m[0].second, ver.cend(), m, re_version))
				_verDriver.loadFromRegex(m);
		}
		// プロファイルの特定
		if(ver.find("compatibility") != std::string::npos)
			_profile = Profile::Compatibility;
		else if(ver.find("core") != std::string::npos)
			_profile = Profile::Core;
		else
			_profile = Profile::Unknown;

		std::cmatch cm;
		const auto* cp = reinterpret_cast<const char*>(GL.glGetString(GL_SHADING_LANGUAGE_VERSION));
		_verSL.clear();
		if(std::regex_search(cp, cm, re_glsl))
			_verSL.loadFromRegex(cm);

		_capSet.clear();
		cp = reinterpret_cast<const char*>(GL.glGetString(GL_EXTENSIONS));
		while(std::regex_search(cp, cm, re_ext)) {
			_capSet.insert(cm.str(0));
			cp = cm.suffix().first;
		}
	}
	void GPUInfo::onDeviceLost() {}

	const GPUInfo::Version& GPUInfo::glslVersion() const {
		return _verSL;
	}
	const GPUInfo::Version& GPUInfo::version() const {
		return _verGL;
	}
	const GPUInfo::Version& GPUInfo::driverVersion() const {
		return _verDriver;
	}
	const std::string& GPUInfo::vendor() const {
		return _strVendor;
	}
	const std::string& GPUInfo::renderer() const {
		return _strRenderer;
	}
	const GPUInfo::CapSet& GPUInfo::refCapabilitySet() const {
		return _capSet;
	}
	std::ostream& operator << (std::ostream& os, const GPUInfo& info) {
		using std::endl;
		os << "OpenGL Version: " << info.version() << endl
				<< "OpenGL Vendor: " << info.vendor() << endl
				<< "OpenGL Renderer: " << info.renderer() << endl
				<< "GLSL Version: " << info.glslVersion() << endl
				<< "Driver Version: " << info.driverVersion() << endl
				<< "Extensions: " <<  endl;
		auto& cs = info.refCapabilitySet();
		for(auto& c : cs)
			os << c << endl;
		return os;
	}
#ifndef USE_OPENGLES2
	// ----------------------- GPUTime -----------------------
	GPUTime::GPUTime(): _idQuery{0,0}, _idSync{nullptr,nullptr}, _cursor(0), _prevTime(0) {}
	GPUTime::~GPUTime() {
		onDeviceLost();
	}
	void GPUTime::onFrameBegin() {
		GL.glBeginQuery(GL_TIME_ELAPSED, _idQuery[_cursor]);
	}
	void GPUTime::onFrameEnd() {
		GL.glEndQuery(GL_TIME_ELAPSED);
		_idSync[_cursor] = GL.glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		GL.glWaitSync(_idSync[_cursor], 0, GL_TIMEOUT_IGNORED);
		_cursor ^= 1;

		GLuint q = _idQuery[_cursor];
		GLsync sync = _idSync[_cursor];
		if(sync != nullptr) {
			GL.glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);
			GLuint bEnd = GL_FALSE;
			do {
				GL.glGetQueryObjectuiv(q, GL_QUERY_RESULT_AVAILABLE, &bEnd);
			} while(bEnd == GL_FALSE);
			GL.glGetQueryObjectui64v(q, GL_QUERY_RESULT, &_prevTime);
			GL.glDeleteSync(sync);
			_idSync[_cursor] = nullptr;
		}
	}
	GLuint64 GPUTime::getTime() const {
		return _prevTime;
	}
	void GPUTime::onDeviceReset() {
		if(_idQuery[0] == 0)
			GL.glGenQueries(countof(_idQuery), _idQuery);
	}
	void GPUTime::onDeviceLost() {
		if(_idQuery[0] != 0) {
			for(auto& sync : _idSync) {
				if(sync)
					GL.glDeleteSync(sync);
			}
			GL.glDeleteQueries(countof(_idQuery), _idQuery);

			_idQuery[0] = _idQuery[1] = 0;
			_idSync[0] = _idSync[1] = nullptr;
			_cursor = 0;
			_prevTime = 0;
		}
	}
#endif
}
