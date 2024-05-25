#include <filesystem>

namespace FilesystemHelper {
	std::filesystem::path revertPath = std::filesystem::current_path();

	void SetCurrentPath(size_t levels) {
		revertPath = std::filesystem::current_path();
		std::filesystem::path newPath = std::filesystem::current_path();
		for (size_t i = 0; i < levels; i++) {
			newPath = newPath.parent_path();
		}
		std::filesystem::current_path(newPath);
	}

	void SetCurrentPath(std::filesystem::path path) {
		revertPath = std::filesystem::current_path();
		std::filesystem::current_path(path);
	}

	void RevertCurrentPath() {
		std::filesystem::current_path(revertPath);
	}
}
