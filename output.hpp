#pragma once
#include "lubee/output.hpp"
#include <boost/format.hpp>

namespace rev {
	namespace log {
		struct RevOutput : lubee::log::DefaultOutput {
			using base = lubee::log::DefaultOutput;
			void print(const lubee::log::Type::e type, const std::string& s) override;
		};

		template <class... Ts>
		std::string ConcatMessage(boost::format& fmt, Ts&&... /*t*/) {
			return fmt.str();
		}
		template <class T, class... Ts>
		std::string ConcatMessage(boost::format& fmt, T&& t, Ts&&... ts) {
			fmt % std::forward<T>(t);
			return ConcatMessage(fmt, std::forward<Ts>(ts)...);
		}
		template <class... Ts>
		std::string MakeMessage(const char* fmt, Ts&&... ts) noexcept {
			try {
				boost::format bf(fmt);
				return ConcatMessage(bf, std::forward<Ts>(ts)...);
			} catch(...) {
				// 何か例外が発生したら空のメッセージを返す
				return std::string();
			}
		}
	}
}
// lubeeのsnprintfベースのメッセージ定義をboost::formatに対応させたバージョン
#define LogR(type, ...)			::lubee::log::Log::Output(::lubee::log::Type::type, ::rev::log::MakeMessage(__VA_ARGS__))
#define LogRHere(type, ...)		::lubee::log::Log::Output(::lubee::log::Type::type, SOURCEPOS, ::rev::log::MakeMessage(__VA_ARGS__))
