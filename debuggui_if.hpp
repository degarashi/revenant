#pragma once
#include <functional>

namespace rev {
	struct IDebugGui {
		virtual ~IDebugGui() {}
		virtual const char* getDebugName() const noexcept;
		virtual bool property(bool edit);
		// 内容を簡潔に表示
		virtual void summary() const;
		// 内容を示す文字列を返す
		virtual std::string summary_str() const;

		using FilterCB = std::function<bool (const char*)>;
		bool summary(const FilterCB& filter) const;
	};
}
#ifdef DEBUGGUI_ENABLED
	#define DEF_DEBUGGUI_PROP \
		bool property(bool edit) override;
	#define DEF_DEBUGGUI_NAME \
		const char* getDebugName() const noexcept override;
	#define DEF_DEBUGGUI_SUMMARY \
		void summary() const override;
	#define DEF_DEBUGGUI_SUMMARYSTR \
		std::string summary_str() const override;

	#define DEF_DEBUGGUI_ALL\
		DEF_DEBUGGUI_PROP \
		DEF_DEBUGGUI_NAME \
		DEF_DEBUGGUI_SUMMARY \
		DEF_DEBUGGUI_SUMMARYSTR
#else
	#define DEF_DEBUGGUI_PROP
	#define DEF_DEBUGGUI_NAME
	#define DEF_DEBUGGUI_SUMMARY
	#define DEF_DEBUGGUI_SUMMARYSTR
	#define DEF_DEBUGGUI_ALL
#endif
