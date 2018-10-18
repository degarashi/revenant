#include <string>
#include <memory>

using ImGuiID = unsigned int;
namespace rev::debug {
	template <class P>
	class StateStorage;
	class TextBuffer {
		private:
			using CBuff = std::string;
			using St = StateStorage<std::shared_ptr<CBuff>>;
			CBuff&	_buff;
		public:
			TextBuffer(const ImGuiID id, const std::size_t size);
			const CBuff& buffer() const noexcept;
			const char* pointer() const noexcept;
			CBuff& buffer() noexcept;
			char* pointer() noexcept;
			std::size_t size() const noexcept;
	};
}
