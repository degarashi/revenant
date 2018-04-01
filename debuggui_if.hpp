#pragma once

namespace rev {
	struct IDebugGui {
		virtual ~IDebugGui() {}
		virtual const char* getDebugName() const noexcept;
		virtual bool property(bool edit);
		virtual void summary() const;
	};
}
#ifdef DEBUGGUI_ENABLED
	#define DEF_DEBUGGUI_PROP \
		bool property(bool edit) override;
	#define DEF_DEBUGGUI_NAME \
		const char* getDebugName() const noexcept override;
	#define DEF_DEBUGGUI_SUMMARY \
		void summary() const override;

	#define DEF_DEBUGGUI_ALL\
		DEF_DEBUGGUI_PROP \
		DEF_DEBUGGUI_NAME \
		DEF_DEBUGGUI_SUMMARY
#else
	#define DEF_DEBUGGUI_PROP
	#define DEF_DEBUGGUI_NAME
	#define DEF_DEBUGGUI_SUMMARY
	#define DEF_DEBUGGUI_ALL
#endif
