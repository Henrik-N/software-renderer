const std = @import("std");

const math = @import("math.zig");
const Vec3 = math.Vec3;
const FaceVertexIndices = @Vector(3, u16);

pub const Mesh = struct {
    vertices: std.ArrayList(Vec3),
    faces: std.ArrayList(FaceVertexIndices),

    pub fn deinit(mesh: Mesh) void {
        mesh.faces.deinit();
        mesh.vertices.deinit();
    }

    // Assumes the mesh uses triangle faces
    pub fn loadFromObj(ally: std.mem.Allocator, filename: []const u8) !Mesh {
        var mesh = Mesh{
            .vertices = std.ArrayList(Vec3).init(ally),
            .faces = std.ArrayList(FaceVertexIndices).init(ally),
        };
        errdefer mesh.deinit();

        var file = try std.fs.cwd().openFile(filename, .{}); // TODO: Don't use cwd
        defer file.close();

        var buffered_reader = std.io.bufferedReader(file.reader());
        const reader = buffered_reader.reader();

        var line = std.ArrayList(u8).init(ally);
        defer line.deinit();

        while (reader.streamUntilDelimiter(line.writer(), '\n', null)) {
            defer line.clearRetainingCapacity();

            var it: std.mem.SplitIterator(u8, .sequence) = std.mem.split(u8, line.items, " ");
            var word: []const u8 = it.next() orelse continue;

            const is_vertex_entry = std.mem.eql(u8, word, "v");
            if (is_vertex_entry) {
                var vertex: Vec3 = undefined;
                for (0..3) |k| {
                    word = it.next() orelse return error.ObjVertexEntry;
                    vertex[k] = try std.fmt.parseFloat(f32, word);
                }
                try mesh.vertices.append(vertex);
                continue;
            }

            const is_face_entry = std.mem.eql(u8, word, "f");
            if (is_face_entry) {
                var face: FaceVertexIndices = undefined;
                for (0..3) |k| {
                    word = it.next() orelse return error.ObjFaceCorner;
                    var face_entries_it = std.mem.split(u8, word, "/");
                    const vertex_index: []const u8 = face_entries_it.next() orelse return error.ObjFaceEntry;
                    const base = 10;
                    face[k] = try std.fmt.parseInt(u16, vertex_index, base);
                    face[k] -= 1; // NOTE obj files start vertex indices at 1
                }
                try mesh.faces.append(face);
                continue;
            }
        } else |err| switch (err) {
            error.EndOfStream => {},
            else => return err,
        }

        return mesh;
    }
};
