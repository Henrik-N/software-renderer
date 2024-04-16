#pragma once

#include "_common.h"
#include "_math.h"


struct Mesh {
    using Face_Vertex_Indices = std::array<u16, 3>;

    std::vector<Vec3> vertices;
    std::vector<Face_Vertex_Indices> faces;

    static bool load_from_obj(Mesh& mesh, std::string_view filename);

    void log();
};
