#include "scene_renderer.h"
using namespace parser;
bool SceneRenderer::DoesIntersect(const Vec3f& e, const Vec3f& s, const Mesh& mesh) {
    return false;
}

bool SceneRenderer::DoesIntersect(const Vec3f& e, const Vec3f& s, cont Triangle& triangle) {
    return false;
}

bool SceneRenderer::DoesIntersect(const Vec3f& e, const Vec3f& s, const Sphere& sphere) {
    return false;
}
