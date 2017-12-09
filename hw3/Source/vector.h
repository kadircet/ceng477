#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

constexpr const float kInf = std::numeric_limits<float>::infinity();
constexpr const float kEpsilon = 1e-6;

namespace parser {
// Notice that all the structures are as simple as possible
// so that you are not enforced to adopt any style or design.
namespace {
int max(int a, int b) { return a > b ? a : b; }

int min(int a, int b) { return a < b ? a : b; }

int round(float a) { return (int)a; }  //(int)(a > 0 ? a + .5 : a - .5); }

}  // namespace

struct Vec3i {
  int x, y, z;
};

struct Vec3f {
  float x, y, z;

  Vec3f() {
    x = 0;
    y = 0;
    z = 0;
  }

  Vec3f(float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
  }

  Vec3f(const Vec3i rhs) {
    this->x = rhs.x;
    this->y = rhs.y;
    this->z = rhs.z;
  }

  Vec3f operator+(const Vec3f rhs) const {
    return Vec3f(x + rhs.x, y + rhs.y, z + rhs.z);
  }

  Vec3f operator-(const Vec3f rhs) const {
    return Vec3f(x - rhs.x, y - rhs.y, z - rhs.z);
  }

  // Dot product.
  float operator*(const Vec3f rhs) const {
    return x * rhs.x + y * rhs.y + z * rhs.z;
  }

  Vec3f operator*(float rhs) const { return Vec3f(x * rhs, y * rhs, z * rhs); }

  Vec3f operator/(float rhs) const {
    Vec3f result(x / rhs, y / rhs, z / rhs);
    return result;
  }

  Vec3f& operator/=(float rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    return *this;
  }

  Vec3f& operator*=(float rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
  }

  Vec3f& operator+=(const Vec3f rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }

  Vec3f CrossProduct(const Vec3f rhs) const {
    return Vec3f(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z,
                 x * rhs.y - y * rhs.x);
  }

  Vec3f PointWise(const Vec3f rhs) const {
    return Vec3f(x * rhs.x, y * rhs.y, z * rhs.z);
  }

  float Length() const { return sqrt(*this * *this); }

  const Vec3f Normalized() const { return *this / this->Length(); }
  const Vec3f& Normalize() {
    *this /= this->Length();
    return *this;
  }

  Vec3i ToVec3i() const {
    Vec3i res;
    res.x = min(255, max(0, round(x)));
    res.y = min(255, max(0, round(y)));
    res.z = min(255, max(0, round(z)));
    return res;
  }

  float operator[](int idx) const {
    switch (idx) {
      case 0:
        return x;
      case 1:
        return y;
    }
    return z;
  }

  float& operator[](int idx) {
    switch (idx) {
      case 0:
        return x;
      case 1:
        return y;
    }
    return z;
  }

  void Print() const { std::cout << x << ' ' << y << ' ' << z << std::endl; }
};

struct Vec4f {
  float x, y, z, w;
};

struct Matrix {
  float elems[4][4];

  float* operator[](int idx) { return elems[idx]; }
  const float* operator[](int idx) const { return elems[idx]; }

  Matrix() {
    for (int i = 0; i < 4; i++)
      for (int j = 0; j < 4; j++) elems[i][j] = 0.;
  }

  void Print() {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        std::cout << elems[i][j] << ' ';
      }
      std::cout << std::endl;
    }
  }

  Matrix Transpose() {
    Matrix trans;
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        trans[i][j] = elems[j][i];
      }
    }
    return trans;
  }

  Vec3f MultiplyVector(const Vec3f& rhs) {
    Vec3f res;
    for (int i = 0; i < 3; i++) {
      res[i] = 0;
      for (int j = 0; j < 3; j++) {
        res[i] += elems[i][j] * rhs[j];
      }
    }
    return res;
  }

  Vec3f MultiplyVector(const Vec3f& rhs) const {
    Vec3f res;
    for (int i = 0; i < 3; i++) {
      res[i] = 0;
      for (int j = 0; j < 3; j++) {
        res[i] += elems[i][j] * rhs[j];
      }
    }
    return res;
  }

  Vec3f operator*(const Vec3f& rhs) const {
    Vec3f res;
    for (int i = 0; i < 3; i++) {
      res[i] = elems[i][3];
      for (int j = 0; j < 3; j++) {
        res[i] += elems[i][j] * rhs[j];
      }
    }
    return res;
  }

  Matrix operator*(const Matrix& rhs) {
    Matrix res;
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        for (int k = 0; k < 4; k++) {
          res[i][j] += elems[i][k] * rhs[k][j];
        }
      }
    }
    return res;
  }

  Matrix& operator*=(const Matrix& rhs) {
    float res[4][4];
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        res[i][j] = 0;
        for (int k = 0; k < 4; k++) {
          res[i][j] += elems[i][k] * rhs[k][j];
        }
      }
    }
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        elems[i][j] = res[i][j];
      }
    }
    return *this;
  }

  void MakeIdentity() {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 4; j++) {
        elems[i][j] = i == j ? 1. : .0;
      }
    }
  }
};

struct Rotation {
  float angle, x, y, z;

  Matrix ToMatrix() const {
    const Vec3f u = Vec3f(x, y, z).Normalized();
    const Vec3f v = ((x != 0 || y != 0) ? Vec3f(-u.y, u.x, .0) : Vec3f(0, 1, 0))
                        .Normalized();
    const Vec3f w = u.CrossProduct(v);

    Matrix M;
    M[0][0] = u.x;
    M[0][1] = u.y;
    M[0][2] = u.z;
    M[1][0] = v.x;
    M[1][1] = v.y;
    M[1][2] = v.z;
    M[2][0] = w.x;
    M[2][1] = w.y;
    M[2][2] = w.z;
    M[3][3] = 1;

    Matrix rot;
    rot[0][0] = 1;
    rot[1][1] = cos(angle);
    rot[1][2] = -sin(angle);
    rot[2][1] = -rot[1][2];
    rot[2][2] = rot[1][1];
    rot[3][3] = 1;

    return M.Transpose() * (rot * M);
  }
};
}  // namespace parser

#endif
