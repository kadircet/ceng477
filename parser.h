#ifndef __HW1__PARSER__
#define __HW1__PARSER__

#include <string>
#include <vector>
#include <iostream>
#include <cmath>

namespace parser
{
    //Notice that all the structures are as simple as possible
    //so that you are not enforced to adopt any style or design.
  namespace {
    int max(int a, int b) {
      return a>b?a:b;
    }

    int min(int a, int b) {
      return a<b?a:b;
    }

    int round(float a) {
      return (int)(a>0?a+.5:a-.5);
    }
  }

    struct Vec3i
    {
        int x, y, z;
    };

    struct Vec3f
    {
        float x, y, z;
        
        Vec3f() {

        }

        Vec3f(float x, float y, float z) {
            this->x = x;
            this->y = y;
            this->z = z;
        }

	Vec3f(const Vec3i& rhs) {
	  this->x = rhs.x;
	  this->y = rhs.y;
	  this->z = rhs.z;
	}

        Vec3f operator+(const Vec3f& rhs) const {
            return Vec3f(x + rhs.x,
                        y + rhs.y,
                        z + rhs.z);
        }

        Vec3f operator-(const Vec3f& rhs) const {
            return Vec3f(x - rhs.x,
                        y - rhs.y,
                        z - rhs.z);
        }

	// Dot product.
        float operator*(const Vec3f& rhs) const {
            return x * rhs.x + y * rhs.y + z * rhs.z;
        }

        Vec3f operator*(const float& rhs) const {
            return Vec3f(x*rhs, y*rhs, z*rhs);
        }

        Vec3f operator/(const float& rhs) {
	  Vec3f result(x/rhs,y/rhs,z/rhs);
	  return result;
        }

        Vec3f& operator/=(const float& rhs) {
	  x/=rhs;
	  y/=rhs;
	  z/=rhs;
	  return *this;
        }

        Vec3f& operator*=(const float& rhs) {
	  x*=rhs;
	  y*=rhs;
	  z*=rhs;
	  return *this;
        }

        Vec3f& operator+=(const Vec3f& rhs) {
	  x+=rhs.x;
	  y+=rhs.y;
	  z+=rhs.z;
	  return *this;
        }

        Vec3f CrossProduct(const Vec3f& rhs) const {
            return Vec3f(y * rhs.z - z * rhs.y,
                        z * rhs.x - x * rhs.z,
                        x * rhs.y - y * rhs.x);
        }

	Vec3f PointWise(const Vec3f& rhs) const {
	  return Vec3f(x*rhs.x, y*rhs.y, z*rhs.z);
	}

	float Length() const {
	  return sqrt(*this**this);
	}

	Vec3f& Normalize() {
	  *this /= this->Length();
	}

	Vec3i ToVec3i() const {
	  Vec3i res;
	  res.x = min(255, max(0, round(x)));
	  res.y = min(255, max(0, round(y)));
	  res.z = min(255, max(0, round(z)));
	  return res;
	}

	void Print() const {
	  std::cout << x << ' ' << y << ' ' << z << std::endl;
	}
    };

    struct Vec4f
    {
        float x, y, z, w;
    };

    struct Camera
    {
        Vec3f position;
        Vec3f gaze;
        Vec3f up;
        Vec4f near_plane;
        float near_distance;
        int image_width, image_height;
        std::string image_name;
    };

    struct PointLight
    {
        Vec3f position;
        Vec3f intensity;
    };

    struct Material
    {
        Vec3f ambient;
        Vec3f diffuse;
        Vec3f specular;
        Vec3f mirror;
        float phong_exponent;
    };

    struct Face
    {
        int v0_id;
        int v1_id;
        int v2_id;
	Vec3f normal;
	void CalculateNormal(const std::vector<Vec3f>& vertex_data) {
	  const Vec3f v0 = vertex_data[v0_id];
	  const Vec3f e1 = vertex_data[v1_id] - v0;
	  const Vec3f e2 = vertex_data[v2_id] - v0;
	  normal = e2.CrossProduct(e1).Normalize();
	}
    };

    struct Mesh
    {
        int material_id;
        std::vector<Face> faces;
    };

    struct Triangle
    {
        int material_id;
        Face indices;
    };

    struct Sphere
    {
        int material_id;
        int center_vertex_id;
        float radius;
    };

    struct Scene
    {
        //Data
        Vec3i background_color;
        float shadow_ray_epsilon;
        int max_recursion_depth;
        std::vector<Camera> cameras;
        Vec3f ambient_light;
        std::vector<PointLight> point_lights;
        std::vector<Material> materials;
        std::vector<Vec3f> vertex_data;
        std::vector<Mesh> meshes;
        std::vector<Triangle> triangles;
        std::vector<Sphere> spheres;

        //Functions
        void loadFromXml(const std::string& filepath);
    };
}

#endif
