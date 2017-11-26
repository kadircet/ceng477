#include "parser.h"
using parser::Sphere;
using parser::Vec3f;

void BoundingBox::Expand(const BoundingBox& bounding_box) {
  min_corner.x = fmin(min_corner.x, bounding_box.min_corner.x);
  min_corner.y = fmin(min_corner.y, bounding_box.min_corner.y);
  min_corner.z = fmin(min_corner.z, bounding_box.min_corner.z);

  max_corner.x = fmax(max_corner.x, bounding_box.max_corner.x);
  max_corner.y = fmax(max_corner.y, bounding_box.max_corner.y);
  max_corner.z = fmax(max_corner.z, bounding_box.max_corner.z);

  delta = max_corner - min_corner;
  center = (max_corner + min_corner) / 2.;
}

int BoundingBox::GetMaxDimension() const {
  if (delta.x > delta.y) {
    if (delta.x > delta.z) return 0;
    return 2;
  }
  if (delta.y > delta.z) return 1;
  return 2;
}

float BoundingBox::DoesIntersect(const Ray& ray) const {
  float tnmax = -kInf;
  float tfmin = kInf;
  const Vec3f origin = ray.origin;
  const Vec3f direction = ray.direction;
  for (int i = 0; i < 3; i++) {
    if (fabs(direction[i]) < kEpsilon) continue;
    float tn = (min_corner[i] - origin[i]) / direction[i];
    float tf = (max_corner[i] - origin[i]) / direction[i];
    if (direction[i] < 0) std::swap(tn, tf);
    if (tn > tnmax) tnmax = tn;
    if (tf < tfmin) tfmin = tf;
    if (tnmax > tfmin) return kInf;
  }
  return tnmax;
}

Vec3f BoundingBox::GetExtent() const { return delta; }
Vec3f BoundingBox::GetCenter() const { return center; }

void BoundingVolumeHierarchy::GetIntersection(const Ray& ray, Node* cur,
                                              HitRecord& hit_record,
                                              const Object* hit_obj) const {
  const int left = cur->start;
  const int right = cur->end;

  if (left + 1 == right) {
    const Object* leaf = (*objects_)[left];
    if (leaf == hit_obj) return;
    const HitRecord rec = leaf->GetIntersection(ray);
    if (rec.t < hit_record.t && rec.t > .0) {
      hit_record = rec;
    }
    return;
  }

  float t_left = kInf;
  float t_right = kInf;
  if (cur->left != nullptr) {
    const BoundingBox bounding_box = cur->left->bounding_box;
    t_left = bounding_box.DoesIntersect(ray);
  }
  if (cur->right != nullptr) {
    const BoundingBox bounding_box = cur->right->bounding_box;
    t_right = bounding_box.DoesIntersect(ray);
  }
  if (t_left < t_right) {
    if (t_left < hit_record.t) {
      GetIntersection(ray, cur->left, hit_record, hit_obj);
    }
    if (t_right < hit_record.t) {
      GetIntersection(ray, cur->right, hit_record, hit_obj);
    }
  } else {
    if (t_right < hit_record.t) {
      GetIntersection(ray, cur->right, hit_record, hit_obj);
    }
    if (t_left < hit_record.t) {
      GetIntersection(ray, cur->left, hit_record, hit_obj);
    }
  }
}

void BoundingVolumeHierarchy::GetIntersection(const Ray& ray, Node* cur,
                                              float tmax, float& tmin,
                                              const Object* hit_obj) const {
  const int left = cur->start;
  const int right = cur->end;
  if (tmin < tmax + kEpsilon) return;

  if (left + 1 == right) {
    const Object* leaf = (*objects_)[left];
    if (leaf == hit_obj) return;
    const HitRecord rec = leaf->GetIntersection(ray);
    if (rec.t < tmin && rec.t > .0) {
      tmin = rec.t;
    }
    return;
  }

  float t_left = kInf;
  float t_right = kInf;
  if (cur->left != nullptr) {
    const BoundingBox bounding_box = cur->left->bounding_box;
    t_left = bounding_box.DoesIntersect(ray);
  }
  if (cur->right != nullptr) {
    const BoundingBox bounding_box = cur->right->bounding_box;
    t_right = bounding_box.DoesIntersect(ray);
  }
  if (t_left < t_right) {
    if (t_left < tmin) {
      GetIntersection(ray, cur->left, tmax, tmin, hit_obj);
    }
    if (t_right < tmin) {
      GetIntersection(ray, cur->right, tmax, tmin, hit_obj);
    }
  } else {
    if (t_right < tmin) {
      GetIntersection(ray, cur->right, tmax, tmin, hit_obj);
    }
    if (t_left < tmin) {
      GetIntersection(ray, cur->left, tmax, tmin, hit_obj);
    }
  }
}

HitRecord BoundingVolumeHierarchy::GetIntersection(
    const Ray& ray, const Object* hit_obj) const {
  HitRecord hit_record;
  hit_record.t = kInf;
  hit_record.material_id = -1;
  for (const Sphere sphere : *spheres_) {
    const Ray ray_transformed = ray.Transform(sphere.inverse_transformation);
    if (sphere.GetBoundingBox().DoesIntersect(ray_transformed) < hit_record.t) {
      const HitRecord hit_record_sp = sphere.GetIntersection(ray_transformed);
      if (hit_record_sp.t < hit_record.t && hit_record_sp.t > .0) {
        hit_record = hit_record_sp;
        hit_record.normal = sphere.inverse_transformation_transpose
                                .MultiplyVector(hit_record_sp.normal)
                                .Normalized();
      }
    }
  }
  const BoundingBox bounding_box = tree_->bounding_box;
  const float t = bounding_box.DoesIntersect(ray);
  if (t < kInf) GetIntersection(ray, tree_, hit_record, hit_obj);
  return hit_record;
}

bool BoundingVolumeHierarchy::GetIntersection(const Ray& ray, float tmax,
                                              const Object* hit_obj) const {
  float tmin = kInf;
  const BoundingBox bounding_box = tree_->bounding_box;
  for (const Sphere sphere : *spheres_) {
    const Ray ray_transformed = ray.Transform(sphere.inverse_transformation);
    if (sphere.GetBoundingBox().DoesIntersect(ray_transformed) < tmax) {
      const HitRecord hit_record = sphere.GetIntersection(ray_transformed);
      if (hit_record.t < tmax + kEpsilon && hit_record.t > .0) {
        return true;
      }
    }
  }
  const float t = bounding_box.DoesIntersect(ray);
  if (t < kInf) GetIntersection(ray, tree_, tmax, tmin, hit_obj);
  return tmin < tmax + kEpsilon && tmin > .0;
}

void BoundingVolumeHierarchy::build(Node* cur, int left, int right) {
  cur->left = nullptr;
  cur->right = nullptr;
  cur->start = left;
  cur->end = right;
  for (int i = left; i < right; i++) {
    cur->bounding_box.Expand((*objects_)[i]->GetBoundingBox());
  }

  if (left + 1 >= right) return;

  const int max_dimension = cur->bounding_box.GetMaxDimension();
  const float mean = cur->bounding_box.GetExtent()[max_dimension] / 2.;

  int mid_idx = left;
  for (int i = left; i < right; i++) {
    const Object* obj = (*objects_)[i];
    if (obj->GetBoundingBox().GetCenter()[max_dimension] < mean) {
      std::swap((*objects_)[i], (*objects_)[mid_idx++]);
    }
  }
  if (mid_idx == left || mid_idx == right) {
    mid_idx = (left + right) / 2;
  }

  if (left < mid_idx) {
    cur->left = new Node;
    build(cur->left, left, mid_idx);
  }
  if (mid_idx < right) {
    cur->right = new Node;
    build(cur->right, mid_idx, right);
  }
}

BoundingVolumeHierarchy::BoundingVolumeHierarchy(std::vector<Object*>* objects,
                                                 std::vector<Sphere>* spheres)
    : objects_(objects), spheres_(spheres) {
  tree_ = new Node;
  build(tree_, 0, objects_->size());
}
