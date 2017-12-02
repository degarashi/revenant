#include "gl_resource.hpp"
#include "gl_framebuffer.hpp"
#include "gl_if.hpp"
#include "gl_renderbuffer.hpp"
#include "gl_program.hpp"
#include "gl_buffer.hpp"
#include "gl_texture.hpp"
#include "gl_shader.hpp"
#include "glx.hpp"
#include "sdl_rw.hpp"
#include "systeminfo.hpp"

namespace rev {
	const char* IGLResource::getResourceName() const noexcept {
		return "IGLResource";
	}

	namespace {
		template <class Tex_t, class... Ts>
		auto MakeStaticTex(Ts&&... ts) {
			return [&](const URI& uri){
				return new Tex_t(uri, std::forward<Ts>(ts)...);
			};
		}
	}
	const std::string GLRes::cs_rtname[] = {
		"texture",
		"effect"
	};
	// 既にデバイスがアクティブだったらonDeviceResetを呼ぶ
	void GLRes::_resourceInit(IGLResource *const r) {
		if(_bInit)
			r->onDeviceReset();
	}
	GLRes::GLRes():
		ResMgrApp(cs_rtname)
	{
		_bInit = false;
		_bInDtor = false;
		_upFb.reset(new GLFBuffer());

		// EmptyTexture = 1x1の単色テクスチャ
		const uint32_t buff1 = 0xffffffff;
		_hEmptyTex = createTextureInit(
						lubee::SizeI(1,1),
						GL_RGBA,
						false,
						true,
						GL_UNSIGNED_BYTE,
						AB_Byte(&buff1, sizeof(buff1))
					);
	}
	GLRes::~GLRes() {
		// 破棄フラグを立ててからOnDeviceLost関数を呼ぶ
		_bInDtor = true;
		onDeviceLost();
	}
	const FBInfo_OP& GLRes::getDefaultDepth() const {
		return _defaultDepth;
	}
	const FBInfo_OP& GLRes::getDefaultColor() const {
		return _defaultColor;
	}
	HTex GLRes::loadTexture(const URI& uri, const MipState miplevel, const InCompressedFmt_OP fmt) {
		_setResourceTypeId(ResourceType::Texture);
		return loadResourceApp<Texture_URI>(
					uri,
					[this, miplevel, fmt](auto& uri, auto&& mk){
						mk(uri.uri, miplevel, fmt);
						_resourceInit(mk.pointer);
					}
				).first;
	}
	// 連番キューブ: Key=(Path+@, ext) URI=(Path, ext)
	HTex GLRes::loadCubeTexture(const MipState miplevel, const InCompressedFmt_OP fmt,
								const URI& uri0, const URI& uri1, const URI& uri2,
								const URI& uri3, const URI& uri4, const URI& uri5)
	{
		// 個別指定CubeTextureの場合はリソース名はUriを全部つなげた文字列とする
		std::string tmp(uri0.plain());
		const auto fn = [&tmp](const URI& u) {
			tmp.append(u.plain());
		};
		fn(uri1); fn(uri2); fn(uri3); fn(uri4); fn(uri5);

		return
			loadResourceApp<Texture_CubeURI>(
				FileURI(tmp),
				[&](auto& /*uri*/, auto&& mk){
					mk(uri0.clone(), uri1.clone(), uri2.clone(),
						uri3.clone(), uri4.clone(), uri5.clone(),
						miplevel,fmt);
					_resourceInit(mk.pointer);
				}
			).first;
	}
	HTex GLRes::_createTexture(bool bCube, const lubee::SizeI& size, GLInSizedFmt fmt, bool bStream, bool bRestore) {
		auto h = base_type::acquireA<Texture_Mem>(bCube, fmt, size, bStream, bRestore);
		_resourceInit(h.get());
		return h;
	}
	HTex GLRes::createCubeTexture(const lubee::SizeI& size, GLInSizedFmt fmt, bool bStream, bool bRestore) {
		return _createTexture(true, size, fmt, bStream, bRestore);
	}
	HTex GLRes::createTexture(const lubee::SizeI& size, GLInSizedFmt fmt, bool bStream, bool bRestore) {
		return _createTexture(false, size, fmt, bStream, bRestore);
	}
	HTex GLRes::createTextureInit(const lubee::SizeI& size, GLInSizedFmt fmt, bool bStream, bool bRestore, GLTypeFmt srcFmt, AB_Byte data) {
		auto h = base_type::acquireA<Texture_Mem>(false, fmt, size, bStream, bRestore);
		_resourceInit(h.get());
		h->writeData(data, srcFmt);
		return h;
	}
	HSh GLRes::makeShader(const ShType type, const std::string& src) {
		auto h = base_type::acquireA<GLShader>(type, src);
		_resourceInit(h.get());
		return h;
	}
	HVb GLRes::makeVBuffer(const DrawType dtype) {
		auto h = base_type::acquireA<GLVBuffer>(dtype);
		_resourceInit(h.get());
		return h;
	}
	HIb GLRes::makeIBuffer(const DrawType dtype) {
		auto h = base_type::acquireA<GLIBuffer>(dtype);
		_resourceInit(h.get());
		return h;
	}

	HProg GLRes::makeProgram(const HSh& vsh, const HSh& psh) {
		auto h = base_type::acquireA<GLProgram>(vsh,psh);
		_resourceInit(h.get());
		return h;
	}
	HProg GLRes::makeProgram(const HSh& vsh, const HSh& gsh, const HSh& psh) {
		auto h = base_type::acquireA<GLProgram>(vsh,gsh,psh);
		_resourceInit(h.get());
		return h;
	}
	GLFBufferTmp& GLRes::getTmpFramebuffer() const {
		return *_tmpFb;
	}
	HTex GLRes::getEmptyTexture() const {
		return _hEmptyTex;
	}
	bool GLRes::deviceStatus() const {
		return _bInit;
	}
	bool GLRes::isInDtor() const {
		return _bInDtor;
	}
	void GLRes::onDeviceLost() {
		if(_bInit) {
			_bInit = false;
			for(auto r : *this)
				r->onDeviceLost();
			// 一旦DeviceLostしたらフレームバッファはデフォルトに戻る(in GUIThread)
			_tmpFb->use_begin();
			_tmpFb.reset(nullptr);

			_upFb->onDeviceLost();
			_clearDefaultInfo();
		}
	}
	void GLRes::_initDefaultInfo() {
		GL.glBindFramebuffer(GL_FRAMEBUFFER, 0);
		using Att = GLFBufferCore::Att;
		_defaultColor = GLFBufferCore::GetCurrentInfo(Att::COLOR0);
		_defaultDepth = GLFBufferCore::GetCurrentInfo(Att::DEPTH);
	}
	void GLRes::_clearDefaultInfo() {
		_defaultDepth = spi::none;
		_defaultColor = spi::none;
	}
	void GLRes::onDeviceReset() {
		if(!_bInit) {
			_bInit = true;
			_upFb->onDeviceReset();
			_tmpFb.reset(new GLFBufferTmp(_upFb->getBufferId(), mgr_info.getScreenSize()));
			_initDefaultInfo();
			for(auto r : *this)
				r->onDeviceReset();
		}
	}
	HRes GLRes::loadResource(const URI& uri) {
		HRes ret;
		if(uri.getType() == URI::Type::File) {
			// とりあえず今はファイルのみ対応
			auto& fu = static_cast<const FileURI&>(uri);
			const auto ext = fu.pathblock().getExtension();
			// is it Texture?
			if(ext=="png" || ext=="jpg" || ext=="bmp")
				ret = loadTexture(uri);
			// is it Effect(Shader)?
			else if(ext == "glx") {
				ret = loadEffect<GLEffect>(fu.pathblock().plain_utf8());
			}
		}
		return ret;
	}
	HFb GLRes::makeFBuffer() {
		auto h = base_type::acquireA<GLFBuffer>();
		_resourceInit(h.get());
		return h;
	}
	HRb GLRes::makeRBuffer(const int w, const int h, const GLInRenderFmt fmt) {
		auto hRb = base_type::acquireA<GLRBuffer>(w, h, fmt);
		_resourceInit(hRb.get());
		return hRb;
	}
}
