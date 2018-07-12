#include "gl_resource.hpp"
#include "gl_framebuffer.hpp"
#include "gl_if.hpp"
#include "gl_renderbuffer.hpp"
#include "gl_program.hpp"
#include "gl_buffer.hpp"
#include "texturesrc_uri.hpp"
#include "texturesrc_mem.hpp"
#include "gl_shader.hpp"
#include "glx.hpp"
#include "sdl_surface.hpp"
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
		"texture"
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
		auto filter = mgr_gl.createTexFilter();
		auto src = createTextureInit(
						lubee::SizeI(1,1),
						GL_RGBA,
						false,
						true,
						GL_UNSIGNED_BYTE,
						AB_Byte(&buff1, sizeof(buff1))
					);
		_hEmptyTex = attachTexFilter(src, filter);
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
	HTexF GLRes::createTexFilter() {
		return std::make_shared<TextureFilter>();
	}
	HTex GLRes::attachTexFilter(const HTexSrcC& src, const HTexF& filter) {
		return makeResource<GLTexture>(src, filter);
	}
	HTexSrc GLRes::loadTexture(const URI& uri, const MipState miplevel, const InCompressedFmt_OP fmt) {
		_setResourceTypeId(ResourceType::Texture);
		return loadResourceApp<TextureSrc_URI>(
			uri,
			[this, &miplevel, fmt](auto& uri, auto&& mk){
				mk(uri.uri, TextureFilter::IsMipmap(miplevel), fmt);
				_resourceInit(mk.pointer);
			}
		).first;
	}
	HTexMem2D GLRes::loadTextureFromRW(const HRW& rw) {
		const auto sfc = Surface::Load(rw);
		const auto buff = sfc->extractAsContinuous(SDL_PIXELFORMAT_RGBA32);
		return makeResource<TextureSrc_Mem2D>(GL_RGBA, sfc->getSize(), false, true);
	}
	// 連番キューブ: Key=(Path+@, ext) URI=(Path, ext)
	HTexSrc GLRes::loadCubeTexture(const MipState miplevel, const InCompressedFmt_OP fmt,
								const URI& uri0, const URI& uri1, const URI& uri2,
								const URI& uri3, const URI& uri4, const URI& uri5)
	{
		// リソース名はmiplevel, fmt, Uriを全部つなげた文字列とする
		std::string tmp;
		tmp.append(miplevel.toStr());
		if(fmt)
			tmp.append(std::to_string(uint32_t(*fmt)));
		const auto fn = [&tmp](const URI& u) {
			tmp.append(u.plain());
		};
		fn(uri0); fn(uri1); fn(uri2); fn(uri3); fn(uri4); fn(uri5);

		return
			loadResourceApp<TextureSrc_CubeURI>(
				FileURI(tmp),
				[&](auto& /*uri*/, auto&& mk){
					// UserURIをFileURIへ変換
					const auto cnvURI = [this](const URI& u){
						URIWrap wrap(u);
						_modifyResourceName(wrap);
						return wrap.uri;
					};
					mk(
						cnvURI(uri0), cnvURI(uri1), cnvURI(uri2),
						cnvURI(uri3), cnvURI(uri4), cnvURI(uri5),
						miplevel,
						fmt
					);
					_resourceInit(mk.pointer);
				}
			).first;
	}
	HTexMemCube GLRes::createCubeTexture(const lubee::SizeI& size, const GLInSizedFmt fmt, const bool mip, const  bool bRestore) {
		return makeResource<TextureSrc_MemCube>(fmt, size, mip, bRestore);
	}
	HTexMem2D GLRes::createTexture(const lubee::SizeI& size, const GLInSizedFmt fmt, const bool mip, const bool bRestore) {
		return makeResource<TextureSrc_Mem2D>(fmt, size, mip, bRestore);
	}
	HTexMem2D GLRes::createTextureInit(
		const lubee::SizeI& size,
		const GLInSizedFmt fmt,
		const bool mip,
		const bool bRestore,
		const GLTypeFmt srcFmt,
		const AB_Byte data
	) {
		auto h = std::static_pointer_cast<TextureSrc_Mem2D>(createTexture(size, fmt, mip, bRestore));
		h->writeData(data, srcFmt);
		return h;
	}
	HSh GLRes::makeShader(const ShType type, const std::string& src) {
		return makeResource<GLShader>(type, src);
	}
	HVb GLRes::makeVBuffer(const DrawType dtype) {
		return makeResource<GLVBuffer>(dtype);
	}
	HIb GLRes::makeIBuffer(const DrawType dtype) {
		return makeResource<GLIBuffer>(dtype);
	}

	HProg GLRes::makeProgram(const HSh& vsh, const HSh& psh) {
		return makeResource<GLProgram>(vsh, psh);
	}
	HProg GLRes::makeProgram(const HSh& vsh, const HSh& gsh, const HSh& psh) {
		return makeResource<GLProgram>(vsh, gsh, psh);
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
		_defaultColor = GLFBufferCore::GetCurrentInfo(Att::Color0);
		_defaultDepth = GLFBufferCore::GetCurrentInfo(Att::Depth);
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
	HFb GLRes::makeFBuffer() {
		return makeResource<GLFBuffer>();
	}
	HRb GLRes::makeRBuffer(const int w, const int h, const GLInRenderFmt fmt) {
		return makeResource<GLRBuffer>(w, h, fmt);
	}
}
