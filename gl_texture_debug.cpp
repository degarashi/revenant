#include "gl_texture_debug.hpp"
#include "sdl_format.hpp"

namespace rev {
	// ------------------------- Texture_Debug -------------------------
	Texture_Debug::Texture_Debug(ITDGen* gen, const lubee::SizeI& size, bool bCube, MipState miplevel):
		IGLTexture(miplevel, GLFormat::QuerySDLtoGL(gen->getFormat())->format, size, bCube),
		_gen(gen)
	{}
	void Texture_Debug::onDeviceReset() {
		if(_onDeviceReset()) {
			const GLenum fmt = GLFormat::QuerySDLtoGL(this->_gen->getFormat())->format;
			const auto size = getSize();
			auto u = use();
			if(isCubemap()) {
				if(_gen->isSingle()) {
					auto buff = _gen->generate(getSize());
					for(int i=0 ; i<6 ; i++)
						MakeMip(getFaceFlag(static_cast<CubeFace::e>(i)), fmt, size, buff, true, isMipmap());
				}
				for(int i=0 ; i<6 ; i++)
					MakeMip(getFaceFlag(static_cast<CubeFace::e>(i)), fmt, size, _gen->generate(getSize(), static_cast<CubeFace::e>(i)), true, isMipmap());
			} else
				MakeMip(getFaceFlag(), fmt, size, _gen->generate(getSize(), CubeFace::PositiveX), true, isMipmap());
		}
	}

	namespace {
		// ブレゼンハムアルゴリズム
		struct Bresen {
			int _width, _tgt;
			int _cur, _error;

			Bresen(const int width, const int tgt) noexcept:
				_width(width),
				_tgt(tgt),
				_cur(0),
				_error(0/*width/2*/)
			{}
			int current() const noexcept {
				return _cur;
			}
			void advance() noexcept {
				_error += _tgt;
				if(_error >= _width) {
					_error -= _width;
					++_cur;
				}
			}
		};
	}
	// ------------------------- TDChecker -------------------------
	TDChecker::TDChecker(const frea::Vec4& col0, const frea::Vec4& col1, const int nDivW, const int nDivH):
		_col{col0,col1},
		_nDivW(nDivW),
		_nDivH(nDivH)
	{}
	uint32_t TDChecker::getFormat() const {
		return SDL_PIXELFORMAT_RGBA8888;
	}
	bool TDChecker::isSingle() const {
		return true;
	}
	ByteBuff TDChecker::generate(const lubee::SizeI& size, CubeFace /*face*/) const {
		GLubyte pack[2][4];
		for(int k=0 ; k<2 ; k++) {
			for(int i=0 ; i<4 ; i++)
				pack[k][i] = static_cast<GLubyte>(_col[k].m[i] * 255 + 0.5f);
		}

		int w = size.width,
			h = size.height;
		// データの生成
		Bresen brY(h, _nDivH);
		ByteBuff buff(w*h*sizeof(GLuint));
		auto* ptr = reinterpret_cast<GLuint*>(&buff[0]);
		for(int i=0 ; i<h ; i++) {
			int y = brY.current();
			Bresen brX(w, _nDivW);
			for(int j=0 ; j<w ; j++) {
				int x = brX.current();
				*ptr++ = *reinterpret_cast<GLuint*>(pack[(x^y) & 1]);
				brX.advance();
			}
			brY.advance();
		}
		return buff;
	}
	// ------------------------- TDCChecker -------------------------
	uint32_t TDCChecker::getFormat() const {
		return SDL_PIXELFORMAT_RGBA8888;
	}
	bool TDCChecker::isSingle() const {
		return true;
	}
	ByteBuff TDCChecker::generate(const lubee::SizeI& /*size*/, CubeFace /*face*/) const {
		const static GLubyte tex[] = {
			255, 255, 255, 255,     0,   0,   0, 255,   255, 255, 255 ,255,     0,   0,   0, 255,
			255,   0,   0, 255,     0, 255,   0, 255,     0,   0, 255 ,255,   255, 255, 255, 255,
			128,   0,   0, 255,     0, 128,   0, 255,     0,   0, 128 ,255,   128, 128, 128, 255,
			255, 255,   0, 255,   255,   0, 255, 255,     0, 255, 255 ,255,   255, 255, 255, 255,
		};
		ByteBuff buff(sizeof(tex));
		std::memcpy(&buff[0], tex, sizeof(tex));
		return buff;
	}
	// ------------------------- TDBorder -------------------------
	TDBorder::TDBorder(const frea::Vec4&, const frea::Vec4&) {
		Assert(false, "not implemented yet");
	}
	uint32_t TDBorder::getFormat() const {
		return SDL_PIXELFORMAT_RGBA8888;
	}
	bool TDBorder::isSingle() const {
		return true;
	}
	ByteBuff TDBorder::generate(const lubee::SizeI& /*size*/, CubeFace /*face*/) const {
		Assert(false, "not implemented yet") throw 0;
	}
}
