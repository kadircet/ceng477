#include "bounding_volume_hierarchy.h"

using parser::Vec3f;

namespace {

bool NotZero(const Vec3f vec) {
  return vec.x != -0.02 || vec.y != -0.05 || vec.z != 1.5;
}

}  // namespace

void BoundingBox::Expand(const BoundingBox& bounding_box) {
  min_corner.x = fmin(min_corner.x, bounding_box.min_corner.x);
  min_corner.y = fmin(min_corner.y, bounding_box.min_corner.y);
  min_corner.z = fmin(min_corner.z, bounding_box.min_corner.z);

  max_corner.x = fmax(max_corner.x, bounding_box.max_corner.x);
  max_corner.y = fmax(max_corner.y, bounding_box.max_corner.y);
  max_corner.z = fmax(max_corner.z, bounding_box.max_corner.z);
}

int BoundingBox::GetMaxDimension() const {
  const Vec3f delta = max_corner - min_corner;
  if (delta.x > delta.y) {
    if (delta.x > delta.z) return 0;
    return 2;
  }
  if (delta.y > delta.z) return 1;
  return 2;
}

bool should_print = false;

float BoundingBox::DoesIntersect(const Ray& ray) const {
  float tnmax = -kInf;
  float tfmin = kInf;
  for (int i = 0; i < 3; i++) {
    if (fabs(ray.direction[i]) < kEpsilon) continue;
    float tn = (min_corner[i] - ray.origin[i]) / ray.direction[i];
    float tf = (max_corner[i] - ray.origin[i]) / ray.direction[i];
    if (ray.direction[i] < 0) std::swap(tn, tf);
    if (tn > tnmax) tnmax = tn;
    if (tf < tfmin) tfmin = tf;
    if (should_print) {
      std::cout << tn << ' ' << tf << std::endl;
    }
    if (tnmax > tfmin) return kInf;
  }
  return tnmax;
}

Vec3f BoundingBox::GetExtent() const { return max_corner - min_corner; }
Vec3f BoundingBox::GetCenter() const { return (max_corner + min_corner) * .5; }

void BoundingVolumeHierarchy::GetIntersection(const Ray& ray, Node* cur,
                                              HitRecord& hit_record,
                                              const Object* hit_obj) const {
  const int left = cur->start;
  const int right = cur->end;
  const BoundingBox bounding_box = cur->bounding_box;
  const float t = bounding_box.DoesIntersect(ray);
  if (t > hit_record.t || t == kInf) return;

  if (left + 1 == right) {
    const Object* leaf = (*objects_)[left];
    if (leaf == hit_obj) return;
    const HitRecord rec = leaf->GetIntersection(ray, *scene_);
    if (rec.t < hit_record.t && rec.t > .0) {
      hit_record = rec;
    }
    return;
  }

  if (cur->left != nullptr) {
    GetIntersection(ray, cur->left, hit_record, hit_obj);
  }
  if (cur->right != nullptr) {
    GetIntersection(ray, cur->right, hit_record, hit_obj);
  }
}

void BoundingVolumeHierarchy::GetIntersection(const Ray& ray, Node* cur,
                                              float tmax, float& tmin,
                                              const Object* hit_obj) const {
  const int left = cur->start;
  const int right = cur->end;
  const BoundingBox bounding_box = cur->bounding_box;
  const float t = bounding_box.DoesIntersect(ray);
  if (t > tmin || t == kInf || tmin < tmax + kEpsilon) return;

  if (left + 1 == right) {
    const Object* leaf = (*objects_)[left];
    if (leaf == hit_obj) return;
    const HitRecord rec = leaf->GetIntersection(ray, *scene_);
    // std::cout << left << ' ' << right << ' ' << rec.t << std::endl;
    if (rec.t < tmin && rec.t > .0) {
      tmin = rec.t;
    }
    return;
  }

  if (cur->left != nullptr) {
    GetIntersection(ray, cur->left, tmax, tmin, hit_obj);
  }
  if (cur->right != nullptr) {
    GetIntersection(ray, cur->right, tmax, tmin, hit_obj);
  }
}

HitRecord BoundingVolumeHierarchy::GetIntersection(
    const Ray& ray, const Object* hit_obj) const {
  HitRecord hit_record;
  hit_record.t = kInf;
  hit_record.material_id = -1;
  GetIntersection(ray, tree_, hit_record, hit_obj);
  return hit_record;
}

bool BoundingVolumeHierarchy::GetIntersection(const Ray& ray, float tmax,
                                              const Object* hit_obj) const {
  float tmin = kInf;
  /*std::cout << "Ray dir:";
  ray.direction.Print();
  ray.origin.Print();
  for (unsigned long left = 0; left < objects_->size(); left++) {
    const Object* leaf = (*objects_)[left];
    if (leaf == hit_obj) continue;
    const HitRecord rec = leaf->GetIntersection(ray, *scene_);
    if (rec.material_id != -1) {
      std::cout << left << ' ' << rec.t << std::endl;
    }
    if (rec.t < tmin) {
      tmin = rec.t;
    }
  }
  return tmin < tmax + kEpsilon;*/
  GetIntersection(ray, tree_, tmax, tmin, hit_obj);
  // std::cout << tmin << std::endl;
  return tmin < tmax + kEpsilon && tmin > .0;
}

void BoundingVolumeHierarchy::build(Node* cur, int left, int right) {
  cur->left = nullptr;
  cur->right = nullptr;
  cur->start = left;
  cur->end = right;
  for (int i = left; i < right; i++) {
    cur->bounding_box.Expand((*objects_)[i]->GetBoundingBox(*scene_));
  }

  if (left + 1 >= right) return;

  const int max_dimension = cur->bounding_box.GetMaxDimension();
  const float mean = cur->bounding_box.GetExtent()[max_dimension] / 2.;

  int mid_idx = left;
  for (int i = left; i < right; i++) {
    const Object* obj = (*objects_)[i];
    if (obj->GetBoundingBox(*scene_).GetCenter()[max_dimension] < mean) {
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
                                                 const parser::Scene* scene)
    : objects_(objects), scene_(scene) {
  tree_ = new Node;
  build(tree_, 0, objects_->size());
}
