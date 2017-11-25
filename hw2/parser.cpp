#include "parser.h"
#include <sstream>
#include <stdexcept>
#include "matrixInverse.h"
#include "tinyxml2.h"

void parser::Scene::loadFromXml(const std::string& filepath) {
  tinyxml2::XMLDocument file;
  std::stringstream stream;

  auto res = file.LoadFile(filepath.c_str());
  if (res) {
    throw std::runtime_error("Error: The xml file cannot be loaded.");
  }

  auto root = file.FirstChild();
  if (!root) {
    throw std::runtime_error("Error: Root is not found.");
  }

  // Get BackgroundColor
  auto element = root->FirstChildElement("BackgroundColor");
  if (element) {
    stream << element->GetText() << std::endl;
  } else {
    stream << "0 0 0" << std::endl;
  }
  stream >> background_color.x >> background_color.y >> background_color.z;

  // Get ShadowRayEpsilon
  element = root->FirstChildElement("ShadowRayEpsilon");
  if (element) {
    stream << element->GetText() << std::endl;
  } else {
    stream << "0.001" << std::endl;
  }
  stream >> shadow_ray_epsilon;

  // Get MaxRecursionDepth
  element = root->FirstChildElement("MaxRecursionDepth");
  if (element) {
    stream << element->GetText() << std::endl;
  } else {
    stream << "0" << std::endl;
  }
  stream >> max_recursion_depth;

  // Get Cameras
  element = root->FirstChildElement("Cameras");
  element = element->FirstChildElement("Camera");
  Camera camera;
  while (element) {
    auto child = element->FirstChildElement("Position");
    stream << child->GetText() << std::endl;
    child = element->FirstChildElement("Gaze");
    stream << child->GetText() << std::endl;
    child = element->FirstChildElement("Up");
    stream << child->GetText() << std::endl;
    child = element->FirstChildElement("NearPlane");
    stream << child->GetText() << std::endl;
    child = element->FirstChildElement("NearDistance");
    stream << child->GetText() << std::endl;
    child = element->FirstChildElement("ImageResolution");
    stream << child->GetText() << std::endl;
    child = element->FirstChildElement("ImageName");
    stream << child->GetText() << std::endl;

    stream >> camera.position.x >> camera.position.y >> camera.position.z;
    stream >> camera.gaze.x >> camera.gaze.y >> camera.gaze.z;
    stream >> camera.up.x >> camera.up.y >> camera.up.z;
    stream >> camera.near_plane.x >> camera.near_plane.y >>
        camera.near_plane.z >> camera.near_plane.w;
    stream >> camera.near_distance;
    stream >> camera.image_width >> camera.image_height;
    stream >> camera.image_name;

    cameras.push_back(camera);
    element = element->NextSiblingElement("Camera");
  }

  // Get Lights
  element = root->FirstChildElement("Lights");
  auto child = element->FirstChildElement("AmbientLight");
  stream << child->GetText() << std::endl;
  stream >> ambient_light.x >> ambient_light.y >> ambient_light.z;
  element = element->FirstChildElement("PointLight");
  PointLight point_light;
  while (element) {
    child = element->FirstChildElement("Position");
    stream << child->GetText() << std::endl;
    child = element->FirstChildElement("Intensity");
    stream << child->GetText() << std::endl;

    stream >> point_light.position.x >> point_light.position.y >>
        point_light.position.z;
    stream >> point_light.intensity.x >> point_light.intensity.y >>
        point_light.intensity.z;

    point_lights.push_back(point_light);
    element = element->NextSiblingElement("PointLight");
  }

  // Get Materials
  element = root->FirstChildElement("Materials");
  element = element->FirstChildElement("Material");
  Material material;
  while (element) {
    child = element->FirstChildElement("AmbientReflectance");
    stream << child->GetText() << std::endl;
    child = element->FirstChildElement("DiffuseReflectance");
    stream << child->GetText() << std::endl;
    child = element->FirstChildElement("SpecularReflectance");
    stream << child->GetText() << std::endl;
    child = element->FirstChildElement("PhongExponent");
    stream << child->GetText() << std::endl;

    stream >> material.ambient.x >> material.ambient.y >> material.ambient.z;
    stream >> material.diffuse.x >> material.diffuse.y >> material.diffuse.z;
    stream >> material.specular.x >> material.specular.y >> material.specular.z;
    stream >> material.phong_exponent;

    materials.push_back(material);
    element = element->NextSiblingElement("Material");
  }

  // Get VertexData
  element = root->FirstChildElement("VertexData");
  stream << element->GetText() << std::endl;
  Vec3f vertex;
  while (!(stream >> vertex.x).eof()) {
    stream >> vertex.y >> vertex.z;
    vertex_data.push_back(vertex);
  }
  stream.clear();

  // Get Transformations
  element = root->FirstChildElement("Transformations");
  if (element) {
    // Get Scalings
    child = element->FirstChildElement("Scaling");
    if (child) {
      Scaling scaling;
      while (child) {
        stream << child->GetText() << std::endl;
        stream >> scaling.x >> scaling.y >> scaling.z;

        scalings.push_back(scaling);
        child = child->NextSiblingElement("Scaling");
      }
    }

    // Get Translations
    child = element->FirstChildElement("Translation");
    if (child) {
      Translation translation;
      while (child) {
        stream << child->GetText() << std::endl;
        stream >> translation.x >> translation.y >> translation.z;

        translations.push_back(translation);
        child = child->NextSiblingElement("Translation");
      }
    }

    // Get Rotations
    child = element->FirstChildElement("Rotation");
    if (child) {
      Rotation rotation;
      while (child) {
        stream << child->GetText() << std::endl;
        stream >> rotation.angle >> rotation.x >> rotation.y >> rotation.z;
        rotation.angle *= M_PI / 180.;

        rotations.push_back(rotation);
        child = child->NextSiblingElement("Rotation");
      }
    }
    stream.clear();
  }

  // Get Meshes
  element = root->FirstChildElement("Objects");
  element = element->FirstChildElement("Mesh");
  Mesh mesh;
  while (element) {
    child = element->FirstChildElement("Material");
    stream << child->GetText() << std::endl;
    stream >> mesh.material_id;
    mesh.material_id--;

    mesh.texture_id = -1;
    child = element->FirstChildElement("Texture");
    if (child != nullptr) {
      stream << child->GetText() << std::endl;
      stream >> mesh.texture_id;
      mesh.texture_id--;
    }

    child = element->FirstChildElement("Transformations");
    Matrix transformation;
    transformation.MakeIdentity();
    if (child != nullptr) {
      char type;
      int index;
      stream.clear();
      stream << child->GetText() << std::endl;
      while (!(stream >> type).eof()) {
        stream >> index;
        index--;

        switch (type) {
          case 's':
            transformation = scalings[index].ToMatrix() * transformation;
            break;
          case 't':
            transformation = translations[index].ToMatrix() * transformation;
            break;
          case 'r':
            transformation = rotations[index].ToMatrix() * transformation;
            break;
        }
      }
      stream.clear();
    }

    child = element->FirstChildElement("Faces");
    stream << child->GetText() << std::endl;
    Face face;
    int v0_id, v1_id, v2_id;
    while (!(stream >> v0_id).eof()) {
      stream >> v1_id >> v2_id;
      face.v0 = transformation * vertex_data[v0_id - 1];
      face.v1 = transformation * vertex_data[v1_id - 1];
      face.v2 = transformation * vertex_data[v2_id - 1];
      face.material_id = mesh.material_id;
      face.texture_id = mesh.texture_id;
      face.CalculateNormal();
      mesh.faces.push_back(std::move(face));
    }
    stream.clear();

    meshes.push_back(std::move(mesh));
    mesh.faces.clear();
    element = element->NextSiblingElement("Mesh");
  }
  stream.clear();

  // Get MeshInstances
  element = root->FirstChildElement("Objects");
  element = element->FirstChildElement("MeshInstance");
  MeshInstance mesh_instance;
  while (element) {
    mesh_instance.base_mesh_id = element->IntAttribute("baseMeshId") - 1;
    mesh_instance.texture_id = meshes[mesh_instance.base_mesh_id].texture_id;
    mesh_instance.material_id = meshes[mesh_instance.base_mesh_id].material_id;

    child = element->FirstChildElement("Material");
    stream << child->GetText() << std::endl;
    stream >> mesh_instance.material_id;
    mesh_instance.material_id--;

    child = element->FirstChildElement("Texture");
    if (child != nullptr) {
      stream << child->GetText() << std::endl;
      stream >> mesh_instance.texture_id;
      mesh_instance.texture_id--;
    }

    child = element->FirstChildElement("Transformations");
    Matrix transformation;
    transformation.MakeIdentity();
    if (child != nullptr) {
      char type;
      int index;
      stream.clear();
      stream << child->GetText() << std::endl;
      while (!(stream >> type).eof()) {
        stream >> index;
        index--;

        switch (type) {
          case 's':
            transformation = scalings[index].ToMatrix() * transformation;
            break;
          case 't':
            transformation = translations[index].ToMatrix() * transformation;
            break;
          case 'r':
            transformation = rotations[index].ToMatrix() * transformation;
            break;
        }
      }
      stream.clear();
    }
    for (Face face : meshes[mesh_instance.base_mesh_id].faces) {
      face.v0 = transformation * face.v0;
      face.v1 = transformation * face.v1;
      face.v2 = transformation * face.v2;
      face.texture_id = mesh_instance.texture_id;
      face.material_id = mesh_instance.material_id;
      face.CalculateNormal();
      mesh_instance.faces.push_back(face);
    }

    mesh_instances.push_back(std::move(mesh_instance));
    element = element->NextSiblingElement("MeshInstance");
  }
  stream.clear();

  // Get Triangles
  element = root->FirstChildElement("Objects");
  element = element->FirstChildElement("Triangle");
  Triangle triangle;
  while (element) {
    child = element->FirstChildElement("Material");
    stream << child->GetText() << std::endl;
    stream >> triangle.material_id;
    triangle.material_id--;

    triangle.texture_id = -1;
    child = element->FirstChildElement("Texture");
    if (child != nullptr) {
      stream << child->GetText() << std::endl;
      stream >> triangle.texture_id;
      triangle.texture_id--;
    }

    child = element->FirstChildElement("Transformations");
    Matrix transformation;
    transformation.MakeIdentity();
    if (child != nullptr) {
      char type;
      int index;
      stream.clear();
      stream << child->GetText() << std::endl;
      while (!(stream >> type).eof()) {
        stream >> index;
        index--;

        switch (type) {
          case 's':
            transformation = scalings[index].ToMatrix() * transformation;
            break;
          case 't':
            transformation = translations[index].ToMatrix() * transformation;
            break;
          case 'r':
            transformation = rotations[index].ToMatrix() * transformation;
            break;
        }
      }
      stream.clear();
    }

    child = element->FirstChildElement("Indices");
    stream << child->GetText() << std::endl;
    int v0_id, v1_id, v2_id;
    stream >> v0_id >> v1_id >> v2_id;

    triangle.indices.v0 = transformation * vertex_data[v0_id - 1];
    triangle.indices.v1 = transformation * vertex_data[v1_id - 1];
    triangle.indices.v2 = transformation * vertex_data[v2_id - 1];
    triangle.indices.material_id = triangle.material_id;
    triangle.indices.texture_id = triangle.texture_id;
    triangle.indices.CalculateNormal();

    triangles.push_back(std::move(triangle));
    element = element->NextSiblingElement("Triangle");
  }

  // Get Spheres
  element = root->FirstChildElement("Objects");
  element = element->FirstChildElement("Sphere");
  Sphere sphere;
  while (element) {
    child = element->FirstChildElement("Material");
    stream << child->GetText() << std::endl;
    stream >> sphere.material_id;
    sphere.material_id--;

    sphere.texture_id = -1;
    child = element->FirstChildElement("Texture");
    if (child != nullptr) {
      stream << child->GetText() << std::endl;
      stream >> sphere.texture_id;
      sphere.texture_id--;
    }

    child = element->FirstChildElement("Transformations");
    sphere.transformation.MakeIdentity();
    sphere.inverse_transformation.MakeIdentity();
    if (child != nullptr) {
      char type;
      int index;
      stream.clear();
      stream << child->GetText() << std::endl;
      while (!(stream >> type).eof()) {
        stream >> index;
        index--;
        std::cout << type << ' ' << index << std::endl;

        switch (type) {
          case 's':
            sphere.transformation =
                scalings[index].ToMatrix() * sphere.transformation;
            break;
          case 't':
            sphere.transformation =
                translations[index].ToMatrix() * sphere.transformation;
            break;
          case 'r':
            sphere.transformation =
                rotations[index].ToMatrix() * sphere.transformation;
            break;
        }
      }
      stream.clear();
      double trans[16];
      double inv_trans[16];
      for (int i = 0; i < 16; i++) {
        trans[i] = sphere.transformation[i / 4][i % 4];
      }
      invert(trans, inv_trans);
      for (int i = 0; i < 16; i++) {
        sphere.inverse_transformation[i / 4][i % 4] = inv_trans[i];
        sphere.inverse_transformation_transpose[i % 4][i / 4] = inv_trans[i];
      }
    }

    child = element->FirstChildElement("Center");
    stream << child->GetText() << std::endl;
    int center;
    stream >> center;
    sphere.center_of_sphere = vertex_data[center - 1];

    child = element->FirstChildElement("Radius");
    stream << child->GetText() << std::endl;
    stream >> sphere.radius;
    sphere.Initialize();

    spheres.push_back(std::move(sphere));
    element = element->NextSiblingElement("Sphere");
  }

  // Get Textures
  element = root->FirstChildElement("Textures");
  if (element) {
    element = element->FirstChildElement("Texture");
    while (element) {
      Texture* texture = new Texture;
      child = element->FirstChildElement("ImageName");
      texture->image_name = child->GetText();
      child = element->FirstChildElement("Interpolation");
      texture->interpolation_type =
          Texture::ToInterpolationType(child->GetText());
      child = element->FirstChildElement("DecalMode");
      texture->decal_mode = Texture::ToDecalMode(child->GetText());
      child = element->FirstChildElement("Appearance");
      texture->appearance = Texture::ToApperance(child->GetText());
      texture->LoadImage();

      textures.push_back(texture);
      element = element->NextSiblingElement("Texture");
    }
  }

  // Get TexCoordData
  element = root->FirstChildElement("TexCoordData");
  if (element) {
    stream << element->GetText() << std::endl;
    while (!(stream >> vertex.x).eof()) {
      stream >> vertex.y >> vertex.z;
      tex_coord_data.push_back(vertex);
    }
  }
}
