#pragma once
#include "types.hpp"
#include "debug_gui/if.hpp"

namespace rev {
	//! GLシェーダークラス
	class GLShader : public IGLResource {
		private:
			GLuint				_idSh;
			ShType				_flag;
			const std::string	_source;
			void _initShader();
		public:
			//! 空シェーダーの初期化
			GLShader();
			GLShader(ShType flag, const std::string& src);
			~GLShader() override;
			bool isEmpty() const;
			int getShaderId() const;
			ShType getShaderType() const;
			void onDeviceLost() override;
			void onDeviceReset() override;

			DEF_DEBUGGUI_NAME
			DEF_DEBUGGUI_PROP
	};
}
