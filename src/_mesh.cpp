#include "_mesh.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>


// =====================================================================================================================
// == Util =============================================================================================================
// =====================================================================================================================

namespace fs = std::filesystem;


static std::optional<fs::path> cached_asset_folder_path = {};


static fs::path find_asset_folder_path() {
    if (cached_asset_folder_path.has_value()) {
        return cached_asset_folder_path.value();
    }

    fs::path asset_folder_path = fs::current_path();
    bool found_asset_folder_path = false;

    while (asset_folder_path.has_stem()) {
        for (const fs::directory_entry& sub_dir : fs::directory_iterator(asset_folder_path)) {
            if (sub_dir.path().has_stem() &&
                sub_dir.path().stem() == "assets") {
                asset_folder_path = sub_dir.path();
                found_asset_folder_path = true;
                break;
            }
        }

        if (found_asset_folder_path) {
            break;
        }

        asset_folder_path = asset_folder_path.parent_path();
    }

    if (!found_asset_folder_path) {
        std::cerr << __FUNCTION__ << ": failed to find asset folder path" << std::endl;
        return fs::path{};
    }

    cached_asset_folder_path = asset_folder_path;

    return asset_folder_path;
}


// =====================================================================================================================
// == Mesh =============================================================================================================
// =====================================================================================================================

bool Mesh::load_from_obj(Mesh& mesh, std::string_view filename) {
    if (filename.empty()) {
        std::cerr << __FUNCTION__ << ": filename nullptr" << std::endl;
        return false;
    }

    const fs::path asset_folder_path = find_asset_folder_path();
    if (asset_folder_path.empty()) {
        return false;
    }

    const std::string asset_path = asset_folder_path / fs::path(filename);

    std::fstream file{};
    {
        file.open(asset_path, std::ios::in);
        if (file.fail()) {
            printf("%s: %s\n", __FUNCTION__, ": file failed to open");
            return false;
        }
    }

    mesh.vertices.clear();
    mesh.faces.clear();

    mesh.vertices.reserve(1000); // TODO: Find a decent reserve size
    mesh.faces.reserve(1000 * 3);


    std::string line, buffer;
    line.reserve(1024);
    buffer.reserve(30);

    while (!file.eof()) {
        std::getline(file, line);
        std::istringstream line_stream{line};
        line_stream >> buffer;

        if (buffer == "v") {
            Vec3 vertex{};
            line_stream >> vertex.x >> vertex.y >> vertex.z;

            mesh.vertices.emplace_back(vertex);
            continue;
        }

        if (buffer == "f") {
            auto next_number_before_slash = [&]() -> i32 {
                line_stream >> buffer;
                std::istringstream temp_stream{buffer};
                std::getline(temp_stream, buffer, '/');
                // TODO: Maybe use something that doesn't throw here in case the obj-file has an invalid format.
                return std::stoi(buffer);
            };

            Face_Vertex_Indices face_vertex_indices;
            face_vertex_indices[0] = next_number_before_slash() - 1; // obj files start vertices at index 1
            face_vertex_indices[1] = next_number_before_slash() - 1;
            face_vertex_indices[2] = next_number_before_slash() - 1;

            mesh.faces.emplace_back(face_vertex_indices);
            continue;
        }
    }

    file.close();

    mesh.vertices.shrink_to_fit();
    mesh.faces.shrink_to_fit();

    return true;
}


void Mesh::log() {
    for (const Vec3& vertex : vertices) {
        std::cout << "v " << vertex.x << " " << vertex.y << " " << vertex.z << "\n";
    }

    for (const Face_Vertex_Indices& face_indices : faces) {
        std::cout << "f " << face_indices[0] << " " << face_indices[1] << " " << face_indices[2] << "\n";
    }

    std::cout << std::flush;
}
