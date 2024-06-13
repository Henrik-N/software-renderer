#include "_io.h"

#include <filesystem>
#include <optional>


namespace io {


namespace fs = std::filesystem;


static std::optional<fs::path> cached_assets_folder_path = {};


fs::path find_assets_folder_path() {
    if (cached_assets_folder_path.has_value()) {
        return cached_assets_folder_path.value();
    }

    fs::path assets_folder_path = fs::current_path();
    bool found_asset_folder_path = false;

    while (assets_folder_path.has_stem()) {
        for (const fs::directory_entry& sub_dir : fs::directory_iterator(assets_folder_path)) {
            if (sub_dir.path().has_stem() &&
                sub_dir.path().stem() == "assets") {
                assets_folder_path = sub_dir.path();
                found_asset_folder_path = true;
                break;
            }
        }

        if (found_asset_folder_path) {
            break;
        }

        assets_folder_path = assets_folder_path.parent_path();
    }

    if (!found_asset_folder_path) {
        ERR(": failed to find asset folder path");
        return fs::path{};
    }

    cached_assets_folder_path = assets_folder_path;

    return assets_folder_path;
}


fs::path find_full_asset_path(const fs::path& subpath) {
    const fs::path assets_folder_path = io::find_assets_folder_path();
    if (assets_folder_path.empty()) {
        return fs::path{};
    }

    return assets_folder_path / subpath;
}


bool read_binary(const fs::path& file_path, std::vector<u8>& data)
{
    if (!fs::exists(file_path))
    {
        ERR("no file " << file_path);
        return false;
    }

    std::FILE* file = std::fopen(file_path.c_str(), "rb");
    if (!file)
    {
        ERR("failed to open " << file_path);
        return false;
    }

    const usize file_size = fs::file_size(file_path);
    data.resize(file_size);

    const usize num_bytes_read = std::fread(data.data(), sizeof(u8), file_size, file);
    const bool success = num_bytes_read == file_size;
    if (!success)
    {
        ERR("failed to read file" << file_path);
    }

    std::fclose(file);

    return success;
}


bool write_binary(const fs::path& file_path, const std::span<u8> data)
{
    std::FILE* file = std::fopen(file_path.c_str(), "wb");
    if (!file)
    {
        ERR("failed to open " << file_path);
        return false;
    }

    const usize num_bytes_written = std::fwrite(data.data(), sizeof(u8), data.size(), file);
    const bool success = num_bytes_written == data.size();
    if (!success)
    {
        ERR("failed to write to file " << file_path << ". Had " << data.size() << " bytes to write, only wrote " << num_bytes_written);
    }

    std::fclose(file);

    return success;
}


}
