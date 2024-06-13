#pragma once

#include <filesystem>
#include <span>

namespace io {
    namespace fs = std::filesystem;

    fs::path find_assets_folder_path();
    fs::path find_full_asset_path(const fs::path& sub_path); // subpath is a subpath in the assets folde

    bool read_binary(const fs::path& file_path, std::vector<u8>& data);
    bool write_binary(const fs::path& file_path, std::span<u8> data);
}
