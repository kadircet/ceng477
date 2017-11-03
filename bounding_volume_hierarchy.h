#ifndef _BOUNDING_VOLUME_HIERARCHY_
#define _BOUNDING_VOLUME_HIERARCHY_

#include "vector.h"

struct Ray {
  parser::Vec3f origin;
  parser::Vec3f direction;
  bool is_shadow;
};

class Object;

struct HitRecord {
  int material_id;
  float t;
  parser::Vec3f normal;
  const Object* obj;
};

class BoundingBox {
 public:
  BoundingBox()
      : min_corner(parser::Vec3f(kInf, kInf, kInf)),
        max_corner(parser::Vec3f(-kInf, -kInf, -kInf)) {}
  BoundingBox(const parser::Vec3f& min_c, const parser::Vec3f& max_c)
      : min_corner(min_c), max_corner(max_c) {}

  float DoesIntersect(const Ray& ray) const;
  void Expand(const BoundingBox& boudning_box);
  int GetMaxDimension() const;
  parser::Vec3f GetExtent() const;
  parser::Vec3f GetCenter() const;
  void Print() const {
    min_corner.Print();
    max_corner.Print();
    std::cout << "==========" << std::endl;
  }

 private:
  parser::Vec3f min_corner;
  parser::Vec3f max_corner;
};

class Object {
 public:
  virtual HitRecord GetIntersection(const Ray& ray,
                                    const parser::Scene& scene) const = 0;
  virtual BoundingBox GetBoundingBox(const parser::Scene& scene) const = 0;
  virtual void SayMyName() const = 0;
};

struct Node {
  BoundingBox bounding_box;
  Node* left;
  Node* right;
  int start;
  int end;
};

class BoundingVolumeHierarchy {
 public:
  BoundingVolumeHierarchy(std::vector<Object*>* objects,
                          const parser::Scene* scene);
  HitRecord GetIntersection(const Ray& ray, const Object* hit_obj) const;
  bool GetIntersection(const Ray& ray, float tmax, const Object* hit_obj) const;

 private:
  void build(Node* cur, int left, int right);
  void GetIntersection(const Ray& ray, Node* cur, HitRecord& hit_record,
                       const Object* hit_obj) const;
  void GetIntersection(const Ray& ray, Node* cur, float tmax, float& tmin,
                       const Object* hit_obj) const;

  std::vector<Object*>* objects_;
  Node* tree_;
  const parser::Scene* scene_;
};

#endif
