#include "_mesh.h"
#include "_io.h"

#include <filesystem>
#include <fstream>
#include <iostream>


namespace fs = std::filesystem;


// =====================================================================================================================
// == Mesh =============================================================================================================
// =====================================================================================================================

bool Mesh::load_from_obj(Mesh& mesh, std::string_view filename) {
    if (filename.empty()) {
        ERR("filename nullptr");
        return false;
    }

    const fs::path asset_path = io::find_full_asset_path(filename);

    std::fstream file{};
    {
        file.open(asset_path, std::ios::in);
        if (file.fail()) {
            ERR("failed to open file " << asset_path);
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
