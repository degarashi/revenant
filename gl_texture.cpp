#include "gl_texture.hpp"
#include "gl_error.hpp"
#include "gl_if.hpp"
#include "sdl_surface.hpp"
#include "sdl_rw.hpp"
#include "gl_resource.hpp"
#include "gl_framebuffer.hpp"
#include "handler.hpp"
#include "drawcmd/queue_if.hpp"
#include "tls_data.hpp"

namespace rev {
	// ------------------------- TextureBase -------------------------
	const GLuint TextureBase::cs_Filter[3][2] = {
		{GL_NEAREST, GL_LINEAR},
		{GL_NEAREST_MIPMAP_NEAREST, GL_LINEAR_MIPMAP_NEAREST},
		{GL_NEAREST_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR}
	};
	void TextureBase::use_begin() const {
		D_GLAssert(glActiveTexture, GL_TEXTURE0 + _actId);
		D_GLAssert(glBindTexture, _texFlag, _idTex);
		D_GLAssert0();
	}
	GLuint TextureBase::getTexFlag() const {
		return _idTex;
	}
	void TextureBase::setActiveId(GLuint n) {
		_actId = n;
	}
	GLint TextureBase::getTextureId() const {
		return _idTex;
	}
	bool TextureBase::isMipmap() const {
		return  IsMipmap(_mipLevel);
	}
	bool TextureBase::isCubemap() const {
		return _texFlag != GL_TEXTURE_2D;
	}
	bool TextureBase::IsMipmap(const MipState level) {
		return level >= MipState::MipmapNear;
	}
	void TextureBase::setAnisotropicCoeff(float coeff) {
		_coeff = coeff;
	}
	void TextureBase::setFilter(bool bLinearMag, bool bLinearMin) {
		_iLinearMag = bLinearMag ? 1 : 0;
		_iLinearMin = bLinearMin ? 1 : 0;
	}
	void TextureBase::setMagMinFilter(bool bLinear) {
		setFilter(bLinear, bLinear);
	}
	void TextureBase::setUVWrap(WrapState s, WrapState t) {
		_wrapS = s;
		_wrapT = t;
	}
	void TextureBase::setWrap(WrapState st) {
		setUVWrap(st, st);
	}

	// ------------------------- IGLTexture -------------------------
	IGLTexture::IGLTexture(const MipState miplevel, const InCompressedFmt_OP fmt, const lubee::SizeI& sz, const bool bCube):
		TextureBase {
			._idTex = 0,
			._iLinearMag = 0,
			._iLinearMin = 0,
			._wrapS = WrapState::ClampToEdge,
			._wrapT = WrapState::ClampToEdge,
			._mipLevel = miplevel,
			._actId = 0,
			._texFlag = GLuint(bCube ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D),
			._coeff = 0,
		},
		_faceFlag(bCube ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : GL_TEXTURE_2D),
		_size(sz),
		_format(fmt)
	{
		Assert0(!bCube || (_size.width==_size.height));
	}

	bool IGLTexture::_onDeviceReset() {
		if(_idTex == 0) {
			GL.glGenTextures(1, &_idTex);
			return true;
		}
		return false;
	}
	GLenum IGLTexture::getFaceFlag(CubeFace face) const {
		if(isCubemap())
			return _faceFlag + static_cast<int>(face) - static_cast<int>(CubeFace::PositiveX);
		return _faceFlag;
	}
	const InCompressedFmt_OP& IGLTexture::getFormat() const {
		return _format;
	}
	IGLTexture::~IGLTexture() {
		onDeviceLost();
	}
	const lubee::SizeI& IGLTexture::getSize() const {
		return _size;
	}
	void IGLTexture::save(const PathBlock& path, CubeFace face) {
		auto buff = readData(GL_BGRA, GL_UNSIGNED_BYTE, 0, face);
		auto sfc = rev::Surface::Create(buff, sizeof(uint32_t)*_size.width, _size.width, _size.height, SDL_PIXELFORMAT_ARGB8888);
		// OpenGLテクスチャは左下が原点なので…
		auto sfcVf = sfc->flipVertical();
		auto hRW = mgr_rw.fromFile(path, Access::Write);
		sfcVf->saveAsPNG(hRW);
	}
	ByteBuff IGLTexture::readData(GLInFmt internalFmt, GLTypeFmt elem, int level, CubeFace face) const {
		auto size = getSize();
		const size_t sz = size.width * size.height * GLFormat::QueryByteSize(internalFmt, elem);
		ByteBuff buff(sz);
		#ifndef USE_OPENGLES2
		{
			use_begin();
			GL.glGetTexImage(getFaceFlag(face), level, internalFmt, elem, buff.data());
		}
		#elif
		{
			//	OpenGL ES2ではglGetTexImageが実装されていないのでFramebufferにセットしてglReadPixelsで取得
			GLint id;
			GL.glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &id);
			GLFBufferTmp& tmp = mgr_gl.getTmpFramebuffer();
			use_begin();
			if(isCubemap())
				tmp.attachCubeTexture(GLFBuffer::Att::COLOR0, getTextureId(), getFaceFlag(face));
			else
				tmp.attachTexture(GLFBuffer::Att::COLOR0, getTextureId());
			GL.glReadPixels(0, 0, size.width, size.height, internalFmt, elem, buff.data());
			tmp.attachTexture(GLFBuffer::Att::COLOR0, 0);
			GL.glBindFramebuffer(GL_READ_FRAMEBUFFER_BINDING, id);
		}
		#endif
		return buff;
	}
	ByteBuff IGLTexture::readRect(GLInFmt internalFmt, GLTypeFmt elem, const lubee::RectI& rect, CubeFace face) const {
		const size_t sz = rect.width() * rect.height() * GLFormat::QueryByteSize(internalFmt, elem);

		GLint id;
		GLenum flag;
		#ifndef USE_OPENGLES2
			flag = GL_READ_FRAMEBUFFER_BINDING;
		#elif
			flag = GL_FRAMEBUFFER_BINDING;
		#endif
		GL.glGetIntegerv(flag, &id);

		GLFBufferTmp& tmp = mgr_gl.getTmpFramebuffer();
		ByteBuff buff(sz);
		tmp.use_begin();
		if(isCubemap())
			tmp.attachCubeTexture(GLFBuffer::Att::COLOR0, getTextureId(), getFaceFlag(face));
		else
			tmp.attachTexture(GLFBuffer::Att::COLOR0, getTextureId());
		GL.glReadPixels(rect.x0, rect.y0, rect.width(), rect.height(), internalFmt, elem, buff.data());
		tmp.attachTexture(GLFBuffer::Att::COLOR0, 0);

		GL.glBindFramebuffer(flag, id);
		return buff;
	}
	void IGLTexture::onDeviceLost() {
		if(_idTex != 0) {
			GLW.getDrawHandler().postExecNoWait([buffId=getTextureId()](){
				GLuint id = buffId;
				GL.glDeleteTextures(1, &id);
			});
			_idTex = 0;
			D_GLWarn0();
		}
	}
	bool IGLTexture::operator == (const IGLTexture& t) const {
		return getTextureId() == t.getTextureId();
	}
	void IGLTexture::dcmd_uniform(draw::IQueue& q, const GLint id, const int actId) const {
		auto cmd = DCmd_Uniform{
			static_cast<const TextureBase&>(*this),
			.unifId = id
		};
		cmd.setActiveId(actId);
		q.add(cmd);
		q.stockResource(shared_from_this());
	}
	const char* IGLTexture::getResourceName() const noexcept {
		return "IGLTexture";
	}

	// ------------------------- Texture_Mem -------------------------
	Texture_Mem::Texture_Mem(const bool bCube, GLInSizedFmt fmt, const lubee::SizeI& sz, const bool /*bStream*/, const bool bRestore):
		IGLTexture(MipState::NoMipmap, fmt, sz, bCube),
		// _bStream(bStream),
		_bRestore(bRestore)
	{}
	const GLFormatDesc& Texture_Mem::_prepareBuffer() {
		auto& info = *GLFormat::QueryInfo(*_format);
		_typeFormat = info.elementType;
		_buff = ByteBuff(_size.width * _size.height * GLFormat::QuerySize(info.elementType) * info.numElem);
		return info;
	}
	void Texture_Mem::onDeviceLost() {
		if(_idTex != 0) {
			if(!mgr_gl.isInDtor() && _bRestore) {
				auto& info = _prepareBuffer();
				_buff = readData(info.baseType, info.elementType, 0);
			}
			IGLTexture::onDeviceLost();
		}
	}
	void Texture_Mem::onDeviceReset() {
		if(_onDeviceReset()) {
			use_begin();
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			if(_bRestore && _buff) {
				// バッファの内容から復元
				GLenum baseFormat = GLFormat::QueryInfo(_format.get())->baseType;
				GLWarn(glTexImage2D, getFaceFlag(), 0, _format.get(), _size.width, _size.height, 0, baseFormat, _typeFormat.get(), &_buff->operator [](0));
				// DeviceがActiveな時はバッファを空にしておく
				_buff = spi::none;
				_typeFormat = spi::none;
			} else {
				// とりあえず領域だけ確保しておく
				if(isCubemap()) {
					for(int i=0 ; i<=static_cast<int>(CubeFace::NegativeZ) ; i++)
						GLWarn(glTexImage2D, getFaceFlag(static_cast<CubeFace::e>(i)), 0, _format.get(), _size.width, _size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
				} else
					GLWarn(glTexImage2D, getFaceFlag(), 0, _format.get(), _size.width, _size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			}
		}
	}
	// DeviceLostの時にこのメソッドを読んでも無意味
	void Texture_Mem::writeData(AB_Byte buff, GLTypeFmt srcFmt, CubeFace face) {
		// バッファ容量がサイズ以上かチェック
		const auto szInput = GLFormat::QuerySize(srcFmt);
		Assert0(buff.getLength() >= _size.width*_size.height*szInput);
		// DeviceLost中でなければすぐにテクスチャを作成するが、そうでなければ内部バッファにコピーするのみ
		if(_idTex != 0) {
			// テクスチャに転送
			auto& tfm = getFormat();
			auto& sz = getSize();
			use_begin();
			GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			GL.glTexImage2D(getFaceFlag(face), 0, tfm.get(), sz.width, sz.height,
							0, GLFormat::QueryInfo(*tfm)->baseType, srcFmt.get(), buff.getPtr());
			GLAssert0();
		} else {
			if(_bRestore) {
				// 内部バッファへmove
				_buff = buff.moveTo();
				_typeFormat = srcFmt;
			}
		}
	}
	void Texture_Mem::writeRect(AB_Byte buff, const lubee::RectI& rect, const GLTypeFmt srcFmt, const CubeFace face) {
		#ifdef DEBUG
			const size_t bs = GLFormat::QueryByteSize(_format.get(), srcFmt);
			const auto sz = buff.getLength();
			D_Assert0(sz >= bs*rect.width()*rect.height());
		#endif
		if(_idTex != 0) {
			auto& fmt = getFormat();
			use_begin();
			// GLテクスチャに転送
			const GLenum baseFormat = GLFormat::QueryInfo(fmt.get())->baseType;
			GL.glTexSubImage2D(getFaceFlag(face), 0, rect.x0, rect.y0, rect.width(), rect.height(), baseFormat, srcFmt.get(), buff.getPtr());
		} else {
			// 内部バッファが存在すればそこに書き込んでおく
			if(_buff) {
				// でもフォーマットが違う時は警告だけ出して何もしない
				if(*_typeFormat != srcFmt) {
					Expect(false, u8"テクスチャのフォーマットが違うので部分的に書き込めない");
				} else {
					auto& b = *_buff;
					auto* dst = &b[_size.width * rect.y0 + rect.x0];
					auto* src = buff.getPtr();
					// 1画素のバイト数
					const size_t sz = GLFormat::QueryByteSize(_format.get(), _typeFormat.get());
					for(int i=0 ; i<rect.height() ; i++) {
						std::memcpy(dst, src, rect.width());
						dst += _size.width;
						src += sz;
					}
				}
			}
		}
	}
	namespace {
		/*
			\param[in]	tflag	GL_TEXTURE_2D or GL_TEXTURE_CUBE_MAP_(POSITIVE|NEGATIVE)_(X|Y|Z)
			\param[in]	sfc		ピクセルデータが入ったサーフェスクラス
			\param[in]	fmt		希望するフォーマット(none可)
			\param[in]	bP2		trueなら2の乗数サイズへ変換
			\param[in]	bMip	trueならミップマップ生成
		*/
		Size_Fmt WritePixelData(const GLenum tflag, const HSfc& sfc, const InCompressedFmt_OP fmt, const bool bP2, const bool bMip) {
			// SDLフォーマットから適したOpenGLフォーマットへ変換
			HSfc tsfc = sfc;
			GLFormatDesc desc;
			if(fmt) {
				desc = *GLFormat::QueryInfo(*fmt);
			} else {
				// 希望するフォーマットが無ければSurfaceから決定
				auto info = GLFormat::QuerySDLtoGL(tsfc->getFormat().format);
				if(!info) {
					// INDEXEDなフォーマット等は該当が無いのでRGB24として扱う
					info = GLFormat::QuerySDLtoGL(SDL_PIXELFORMAT_RGB24);
					D_Assert0(info);
				}
				desc = *info;
			}
			const auto sdlFmt = desc.sdlFormat!=SDL_PIXELFORMAT_UNKNOWN ? desc.sdlFormat : desc.sdlLossFormat;
			tsfc->convert(sdlFmt);
			// テクスチャ用のサイズ調整
			auto size = tsfc->getSize();
			const lubee::PowSize n2size{size.width, size.height};
			using CB = std::function<void (const void*)>;
			std::function<void (CB)>	func;
			// 2乗サイズ合わせではなくpitchが詰めてある場合は変換しなくていい
			if(!bP2 && tsfc->isContinuous()) {
				func = [&tsfc](CB cb) {
					auto lk = tsfc->lock();
					cb(lk.getBits());
				};
			} else {
				// 2乗サイズ合わせ
				if(bP2 && size != n2size)
					tsfc = tsfc->resize(n2size);
				func = [&tsfc, sdlFmt](CB cb) {
					auto buff = tsfc->extractAsContinuous(sdlFmt);
					cb(&buff[0]);
				};
			}
			// ミップマップの場合はサイズを縮小しながらテクスチャを書き込む
			const auto tsize = tsfc->getSize();
			size = tsize;
			int layer = 0;
			const auto make = [tflag, &layer, &desc, &size](const void* data) {
				GL.glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				GL.glTexImage2D(tflag, layer++, desc.format, size.width, size.height, 0, desc.baseType, desc.elementType, data);
			};
			if(!bMip)
				func(make);
			else {
				for(;;) {
					func(make);
					if(size.width==1 && size.height==1)
						break;
					size.shiftR_one(1);
					tsfc = tsfc->resize(size);
				}
			}
			return std::make_pair(tsize, desc.format);
		}
		//! texのfaceにhRWのピクセルデータを書き込む
		Size_Fmt LoadTexture(const IGLTexture& tex, const HRW& hRW, const CubeFace face) {
			const HSfc sfc = Surface::Load(hRW);
			const GLenum tflag = tex.getFaceFlag(face);
			tex.use_begin();
			return WritePixelData(tflag, sfc, tex.getFormat(), true, tex.isMipmap());
		}
	}
	Size_Fmt LoadTextureFromBuffer(const IGLTexture& tex, const GLenum tflag, const GLenum format, const lubee::SizeI& size, const ByteBuff& buff, const bool bP2, const bool bMip) {
		// 簡単の為に一旦SDL_Surfaceに変換
		const auto info = GLFormat::QueryInfo(format);
		const int pixelsize = info->numElem* GLFormat::QuerySize(info->baseType);
		const HSfc sfc = Surface::Create(buff, pixelsize*size.width, size.width, size.height, info->sdlFormat);
		tex.use_begin();
		return WritePixelData(tflag, sfc, spi::none, bP2, bMip);
	}
	// ------------------------- Texture_URI -------------------------
	Texture_URI::Texture_URI(const HURI& uri, const MipState miplevel, const InCompressedFmt_OP fmt):
		IGLTexture(miplevel, fmt, lubee::SizeI(0,0), false),
		_uri(uri)
	{}
	void Texture_URI::onDeviceReset() {
		if(_onDeviceReset())
			std::tie(_size, _format) = LoadTexture(*this, mgr_rw.fromURI(*_uri, Access::Read), CubeFace::PositiveX);
	}

	// ------------------------- Texture_CubeURI -------------------------
	Texture_CubeURI::Texture_CubeURI(
		const HURI& uri0, const HURI& uri1, const HURI& uri2,
		const HURI& uri3, const HURI& uri4, const HURI& uri5,
		const MipState miplevel, const InCompressedFmt_OP fmt
	):
		IGLTexture(miplevel, fmt, lubee::SizeI(0,0), true),
		_uri{uri0, uri1, uri2, uri3, uri4, uri5}
	{}
	void Texture_CubeURI::onDeviceReset() {
		if(_onDeviceReset()) {
			for(int i=0 ; i<6 ; i++) {
				const auto size_fmt = LoadTexture(
					*this,
					mgr_rw.fromURI(*_uri[i], Access::Read),
					static_cast<CubeFace::e>(i)
				);
				if(i==0)
					std::tie(_size, _format) = size_fmt;
			}
		}
	}

	namespace {
		const GLenum cs_wrap[WrapState::_Num] = {
			GL_CLAMP_TO_EDGE,
			GL_CLAMP_TO_BORDER,
			GL_MIRRORED_REPEAT,
			GL_REPEAT,
			GL_MIRROR_CLAMP_TO_EDGE
		};
	}
	void IGLTexture::DCmd_Uniform::Command(const void* p) {
		auto& self = *static_cast<const DCmd_Uniform*>(p);
		// 最後にBindは解除しない
		self.use_begin();
		{
			if(tls_videoParam.get().bAnisotropic) {
				// setAnisotropic
				GLfloat aMax;
				GL.glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aMax);
				GL.glTexParameteri(self._texFlag, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::max(aMax * self._coeff, 1.f));
			}
			// setUVWrap
			GL.glTexParameteri(self._texFlag, GL_TEXTURE_WRAP_S, cs_wrap[self._wrapS]);
			GL.glTexParameteri(self._texFlag, GL_TEXTURE_WRAP_T, cs_wrap[self._wrapT]);
			// setFilter
			GL.glTexParameteri(self._texFlag, GL_TEXTURE_MAG_FILTER, cs_Filter[0][self._iLinearMag]);
			GL.glTexParameteri(self._texFlag, GL_TEXTURE_MIN_FILTER, cs_Filter[self._mipLevel][self._iLinearMin]);
		}
		GL.glUniform1i(self.unifId, self._actId);
	}
}
