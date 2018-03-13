#include "bsprite.hpp"
#include "../../drawgroup.hpp"

// ----------------------- BoundingSprite -----------------------
BoundingSprite::BoundingSprite(const rev::HDGroup& dg, const rev::HTex& hTex, const frea::Vec2& pos, const frea::Vec2& svec):
	Sprite2D(hTex, 0.f),
	_dg(dg),
	_svec(svec)
{
	setScaling(frea::Vec2{0.1f, 0.1f});
	setOffset(pos);
	setZRange({-1.f, 1.f});
	setStateNew<St>();
}
struct BoundingSprite::St : StateT<St> {
	void onUpdate(BoundingSprite& self) override {
		auto& sc = self.getScaling();
		auto& ofs = self.refOffset();
		auto& svec = self._svec;
		ofs += svec;
		// 境界チェック
		// Left
		if(ofs.x < -1.f) {
			svec.x *= -1.f;
			ofs.x = -1.f;
		}
		// Right
		if(ofs.x + sc.x > 1.f) {
			svec.x *= -1.f;
			ofs.x = 1.f - sc.x;
		}
		// Top
		if(ofs.y + sc.y > 1.f) {
			svec.y *= -1.f;
			ofs.y = 1.f - sc.y;
		}
		// Bottom
		if(ofs.y < -1.f) {
			svec.y *= -1.f;
			ofs.y = -1.f;
		}

		// ZOffset = y
		self.setZOffset(ofs.y);
		self.outputDrawTag(self._dtag);
		self._dtag.technique = Sprite2D::GetDefaultTech();
	}
	void onConnected(BoundingSprite& self, const rev::HGroup&) override {
		self._dg->addObj(self.shared_from_this());
	}
	void onDisconnected(BoundingSprite& self, const rev::HGroup&) override {
		self._dg->remObj(self.shared_from_this());
	}
	void onDraw(const BoundingSprite& self, rev::IEffect& e) const override {
		self.draw(e);
	}
};

#ifdef DEBUGGUI_ENABLED
const char* BoundingSprite::getDebugName() const noexcept {
	return "BoundingSprite";
}
#endif
