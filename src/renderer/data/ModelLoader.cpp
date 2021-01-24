#include "ModelLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION

#include <tiny_obj_loader.h>

#define TINYPLY_IMPLEMENTATION

#include <tinyply.h>

#include <iostream>
#include <optional>
#include <memory>
#include <stdexcept>
#include <unordered_map>

// For debugging prints
// #define M_DEBUG_MODELLOADER

///////////////////////////////// HELPER //////////////////////////////////////

inline std::vector<uint8_t> read_file_binary(const std::string &pathToFile) {
    std::ifstream file(pathToFile, std::ios::binary);
    std::vector<uint8_t> fileBufferBytes;

    if (file.is_open()) {
        file.seekg(0, std::ios::end);
        size_t sizeBytes = file.tellg();
        file.seekg(0, std::ios::beg);
        fileBufferBytes.resize(sizeBytes);
        if (file.read((char *) fileBufferBytes.data(), sizeBytes)) return fileBufferBytes;
    } else throw std::runtime_error("could not open binary ifstream to path " + pathToFile);
    return fileBufferBytes;
}

struct memory_buffer : public std::streambuf {
    char *p_start{nullptr};
    char *p_end{nullptr};
    size_t size;

    memory_buffer(char const *first_elem, size_t size)
            : p_start(const_cast<char *>(first_elem)), p_end(p_start + size), size(size) {
        setg(p_start, p_start, p_end);
    }

    pos_type seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) override {
        if (dir == std::ios_base::cur) gbump(static_cast<int>(off));
        else setg(p_start, (dir == std::ios_base::beg ? p_start : p_end) + off, p_end);
        return gptr() - p_start;
    }

    pos_type seekpos(pos_type pos, std::ios_base::openmode which) override {
        return seekoff(pos, std::ios_base::beg, which);
    }
};

struct memory_stream : virtual memory_buffer, public std::istream {
    memory_stream(char const *first_elem, size_t size)
            : memory_buffer(first_elem, size), std::istream(static_cast<std::streambuf *>(this)) {}
};

///////////////////////////////// CLASS ///////////////////////////////////////

void ModelLoader::cleanup() {
}

std::optional<std::unique_ptr<Mesh>> ModelLoader::loadMeshFromOBJ(const std::string &filename) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    auto ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str());

    if (!warn.empty()) {
        std::cerr << "TinyObjloader: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "TinyObjloader: " << err << std::endl;
    }

    if (!ret) {
        throw std::runtime_error("TinyObjloader: " + err);
    }

    auto mesh = std::make_unique<Mesh>();
    std::unordered_map<Vertex, uint32_t> verticesMap = {};

    for (size_t s = 0; s < shapes.size(); s++) {
        for (auto &index : shapes[s].mesh.indices) {
            Vertex vertex;
            vertex.pos = glm::vec3(
                    attrib.vertices[index.vertex_index * 3 + 0],
                    attrib.vertices[index.vertex_index * 3 + 1],
                    attrib.vertices[index.vertex_index * 3 + 2]
            );
            vertex.color = glm::vec3(
                    attrib.colors[index.vertex_index * 3 + 0],
                    attrib.colors[index.vertex_index * 3 + 1],
                    attrib.colors[index.vertex_index * 3 + 2]
            );
            vertex.normal = glm::normalize(glm::vec3(
                    attrib.normals[index.normal_index * 3 + 0],
                    attrib.normals[index.normal_index * 3 + 1],
                    attrib.normals[index.normal_index * 3 + 2]
            ));
            vertex.uv = glm::vec2(
                    attrib.texcoords[index.texcoord_index * 2 + 0],
                    attrib.texcoords[index.texcoord_index * 2 + 1]
            );

            if (verticesMap.count(vertex) == 0) {
                verticesMap[vertex] = static_cast<uint32_t>(mesh->vertices.size());
                mesh->vertices.push_back(vertex);
            }
            mesh->indices.push_back(verticesMap[vertex]);
        }
    }

    std::cout << "Loaded mesh(" << filename << ") with "
              << mesh->vertices.size() << " vertices" << std::endl;

    return std::make_optional(std::move(mesh));
}

std::optional<std::unique_ptr<Mesh>> ModelLoader::loadMeshFromPLY(const std::string &filename) {
    using namespace tinyply;
#ifdef M_DEBUG_MODELLOADER
    std::cout << "........................................................................\n";
    std::cout << "Now Reading: " << filename << std::endl;
#endif
    std::unique_ptr<std::istream> file_stream;
    std::vector<uint8_t> byte_buffer;

    try {
        // For most files < 1gb, pre-loading the entire file upfront and wrapping it into a 
        // stream is a net win for parsing speed, about 40% faster. 
        byte_buffer = read_file_binary(filename);
        file_stream.reset(new memory_stream((char *) byte_buffer.data(), byte_buffer.size()));

        if (!file_stream || file_stream->fail())
            throw std::runtime_error("file_stream failed to open " + filename);

        file_stream->seekg(0, std::ios::end);
        const float size_mb = file_stream->tellg() * float(1e-6);
        file_stream->seekg(0, std::ios::beg);

        PlyFile file;
        file.parse_header(*file_stream);

#ifdef M_DEBUG_MODELLOADER
        std::cout << "\t[ply_header] Type: " << (file.is_binary_file() ? "binary" : "ascii") << std::endl;
        for (const auto & c : file.get_comments()) 
            std::cout << "\t[ply_header] Comment: " << c << std::endl;
        for (const auto & c : file.get_info())
            std::cout << "\t[ply_header] Info: " << c << std::endl;

        for (const auto & e : file.get_elements()) {
            std::cout << "\t[ply_header] element: " << e.name << " (" << e.size << ")" << std::endl;
            for (const auto & p : e.properties) {
                std::cout << "\t[ply_header] \tproperty: " << p.name << " (type=" << tinyply::PropertyTable[p.propertyType].str << ")";
                if (p.isList) std::cout << " (list_type=" << tinyply::PropertyTable[p.listType].str << ")";
                std::cout << std::endl;
            }
        }
#endif

        // Because most people have their own mesh types, tinyply treats parsed data as structured/typed byte buffers. 
        // See examples below on how to marry your own application-specific data structures with this one. 
        std::shared_ptr<PlyData> vertices, normals, colors, texcoords, faces, tripstrip;

        // The header information can be used to programmatically extract properties on elements
        // known to exist in the header prior to reading the data. For brevity of this sample, properties 
        // like vertex position are hard-coded: 
        try { vertices = file.request_properties_from_element("vertex", {"x", "y", "z"}); }
        catch (const std::exception &e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

        try { normals = file.request_properties_from_element("vertex", {"nx", "ny", "nz"}); }
        catch (const std::exception &e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

        try { colors = file.request_properties_from_element("vertex", {"red", "green", "blue", "alpha"}); }
        catch (const std::exception &e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

        try { texcoords = file.request_properties_from_element("vertex", {"s", "t"}); }
        catch (const std::exception &e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

        // Providing a list size hint (the last argument) is a 2x performance improvement. If you have 
        // arbitrary ply files, it is best to leave this 0. 
        try { faces = file.request_properties_from_element("face", {"vertex_indices"}, 3); }
        catch (const std::exception &e) { std::cerr << "tinyply exception: " << e.what() << std::endl; }

        file.read(*file_stream);

        if (vertices)
            std::cout << "\tRead " << vertices->count << " total vertices " << std::endl;
        else
            throw std::runtime_error("[ModelLoader] Missing vertex attribute (vertex position)!");
        if (normals)
            std::cout << "\tRead " << normals->count << " total vertex normals " << std::endl;
        else
            throw std::runtime_error("[ModelLoader] Missing vertex attribute (normals vector)!");
        if (colors)
            std::cout << "\tRead " << colors->count << " total vertex colors " << std::endl;
        else
            throw std::runtime_error("[ModelLoader] Missing vertex attribute (color)!");
        if (texcoords)
            std::cout << "\tRead " << texcoords->count << " total vertex texcoords " << std::endl;
        else
            throw std::runtime_error("[ModelLoader] Missing vertex attribute (texture coordinate)!");
        if (faces)
            std::cout << "\tRead " << faces->count << " total faces (triangles) " << std::endl;
        else
            throw std::runtime_error("[ModelLoader] Missing vertex attribute (face)!");

        // Example One: converting to your own application types
        if (vertices->count != normals->count ||
            vertices->count != colors->count ||
            vertices->count != texcoords->count) {
            throw std::runtime_error("[ModelLoader] Unequal ammount of vertices and vertex data!");
        }

        for (const auto &e : file.get_elements()) {
            if (e.name == std::string("vertex")) {
                // TODO validate type
            } else if (e.name == std::string("face")) {
                // TODO validate type
            }
        }

        // TODO reuse vertices

        auto *vertexBuffer = (float *) vertices->buffer.get();
        auto *normalBuffer = (float *) normals->buffer.get();
        auto *colorBuffer = (uint8_t *) colors->buffer.get();
        auto *texcoordBuffer = (float *) texcoords->buffer.get();

        auto mesh = std::make_unique<Mesh>();

        for (size_t i = 0; i < vertices->count; i++) {
            Vertex vertex{};

            vertex.pos = glm::vec3(
                    vertexBuffer[i * 3 + 0],
                    vertexBuffer[i * 3 + 1],
                    vertexBuffer[i * 3 + 2]
            );
            vertex.color = glm::vec3(
                    colorBuffer[i * 4 + 0] / 255.0f,
                    colorBuffer[i * 4 + 1] / 255.0f,
                    colorBuffer[i * 4 + 2] / 255.0f
            );
            vertex.normal = glm::normalize(glm::vec3(
                    normalBuffer[i * 3 + 0],
                    normalBuffer[i * 3 + 1],
                    normalBuffer[i * 3 + 2]
            ));
            vertex.uv = glm::vec2(
                    texcoordBuffer[i * 2 + 0],
                    texcoordBuffer[i * 2 + 1]
            );
            mesh->vertices.push_back(vertex);
        }

        uint32_t *indexBuffer = (uint32_t *) faces->buffer.get();
        for (size_t i = 0; i < faces->count * 3; i++) {
            mesh->indices.push_back(indexBuffer[i]);
        }

        std::cout << "Loaded mesh(" << filename << ") with "
                  << mesh->vertices.size() << " vertices and " << mesh->indices.size() / 3 << " faces" << std::endl;

        return std::make_optional(std::move(mesh));
    } catch (const std::exception &e) {
        std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
        return std::nullopt;
    }
}

Mesh ModelLoader::getQuad() {
    const std::vector<Vertex> vertices = {
            {{-1.0f, -1.0f, +0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
            {{+1.0f, -1.0f, +0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{+1.0f, +1.0f, +0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{-1.0f, +1.0f, +0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
    };

    const std::vector<uint32_t> indices = {
            0, 1, 2, 2, 3, 0,
    };


    Mesh m_quadMesh;
    m_quadMesh.vertices = vertices;
    m_quadMesh.indices = indices;

    return m_quadMesh;
}