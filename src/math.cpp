#include "../header/math.h"

namespace app {

// auto Vec::GetAttributeDescription()
//     -> std::vector<vk::VertexInputAttributeDescription> {
//   std::vector<vk::VertexInputAttributeDescription>
//   descriptions(2); descriptions[0]
//       .setBinding(0)
//       .setFormat(vk::Format::eR32G32Sfloat)
//       .setLocation(0)
//       .setOffset(0);
//   descriptions[1]
//       .setBinding(0)
//       .setFormat(vk::Format::eR32G32Sfloat)
//       .setLocation(1)
//       .setOffset(offsetof(Vertex, texcoord));
//   return descriptions;
// }

// std::vector<vk::VertexInputBindingDescription>
// Vec::GetBindingDescription() {
//   std::vector<vk::VertexInputBindingDescription>
//   descriptions(1); descriptions[0]
//       .setBinding(0)
//       .setStride(sizeof(Vertex))
//       .setInputRate(vk::VertexInputRate::eVertex);
//   return descriptions;
// }

// auto Mat4::Create(const std::initializer_list<float>
// &initList) -> Mat4 {
//   Mat4 mat;
//   int counter = 0;
//   for (auto &value : initList) {
//     int x = counter % 4;
//     int y = counter / 4;
//     mat.Set(y, x, value);
//     counter++;
//   }
//   return mat;
// }

// Mat4::Mat4() {
//   memset(data_, 0, sizeof(float) * 4 * 4);
//   for (int i = 0; i < 4; i++) {
//     Set(i, i, 1);
//   }
// }

// auto Mat4::CreateOnes() -> Mat4 {
//   Mat4 mat;
//   memset(mat.data_, 1, sizeof(float) * 4 * 4);
//   return mat;
// }

// auto Mat4::CreateIdentity() -> Mat4 { return Mat4{}; }

// auto Mat4::CreateOrtho(int left, int right, int top, int
// bottom, int near,
//                        int far) -> Mat4 {
//   Mat4 mat = CreateIdentity();

//   mat.Set(0, 0, 2.0 / (right - left));
//   mat.Set(1, 1, 2.0 / (top - bottom));
//   mat.Set(2, 2, 2.0 / (near - far));
//   mat.Set(3, 0, (left + right) / (left - right));
//   mat.Set(3, 1, (top + bottom) / (bottom - top));
//   mat.Set(3, 2, (near + far) / (far - near));

//   return mat;
// }

// auto Mat4::CreateTranslate(const Vec &pos) -> Mat4 {
//   Mat4 mat = CreateIdentity();

//   mat.Set(3, 0, pos.x);
//   mat.Set(3, 1, pos.y);

//   return mat;
// }

// auto Mat4::CreateScale(const Vec &scale) -> Mat4 {
//   Mat4 mat = CreateIdentity();

//   mat.Set(0, 0, scale.x);
//   mat.Set(1, 1, scale.y);

//   return mat;
// }

// Mat4 Mat4::Mul(const Mat4 &m) const {
//   Mat4 mat;
//   for (int i = 0; i < 4; i++) {
//     for (int j = 0; j < 4; j++) {
//       int sum = 0;
//       for (int k = 0; k < 4; k++) {
//         sum += Get(k, i) * m.Get(j, k);
//       }
//       mat.Set(j, i, sum);
//     }
//   }
//   return mat;
// }

} // namespace app