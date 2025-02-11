// Copyright (c) 2021 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "driver/huawei_ascend_npu/converter.h"
#include "utility/debug.h"
#include "utility/logging.h"

namespace nnadapter {
namespace huawei_ascend_npu {

int Program::ConvertResizeNearest(hal::Operation* operation) {
  auto& input_operands = operation->input_operands;
  auto& output_operands = operation->output_operands;
  auto input_count = input_operands.size();
  auto output_count = output_operands.size();
  NNADAPTER_CHECK_EQ(input_count, 4);
  NNADAPTER_CHECK_EQ(output_count, 1);

  // Input
  auto input_operand = input_operands[0];
  NNADAPTER_VLOG(5) << "input: " << OperandToString(input_operand);
  // Shape
  auto shape_operand = input_operands[1];
  if (shape_operand != nullptr) {
    NNADAPTER_VLOG(5) << "shape: " << OperandToString(shape_operand);
  }
  // Scales
  auto scales_operand = input_operands[2];
  if (scales_operand != nullptr) {
    NNADAPTER_VLOG(5) << "scales: " << OperandToString(scales_operand);
  }
  // Align_corners
  auto align_corners_operand = input_operands[3];
  NNADAPTER_VLOG(5) << "align_corners: "
                    << OperandToString(align_corners_operand);
  bool align_corners =
      reinterpret_cast<bool*>(align_corners_operand->buffer)[0];

  // Output
  auto output_operand = output_operands[0];
  NNADAPTER_VLOG(5) << "output: " << OperandToString(output_operand);

  // Convert to GE operators
  auto resize_nearest_name = GetOperatorName(output_operand);
  auto resize_nearest_op =
      std::make_shared<ge::op::ResizeNearestNeighborV2>(resize_nearest_name);
  auto input_operator = GetMappedOperator(input_operand);
  if (input_operator == nullptr) {
    input_operator = ConvertOperand(input_operand);
  }
  SET_INPUT(resize_nearest_op, x, input_operator);
  if (shape_operand != nullptr) {
    auto shape_operator = GetMappedOperator(shape_operand);
    if (shape_operator == nullptr) {
      shape_operator = ConvertOperand(shape_operand);
    }
    SET_INPUT(resize_nearest_op, size, shape_operator);
  } else if (scales_operand != nullptr) {
    auto scale_operator = GetMappedOperator(scales_operand);
    if (scale_operator == nullptr) {
      scale_operator = ConvertOperand(scales_operand);
    }
    // Shape op
    auto shape_op =
        std::make_shared<ge::op::Shape>(resize_nearest_name + "/shape");
    SET_INPUT(shape_op, x, input_operator);
    auto shape_tensor_desc = std::make_shared<ge::TensorDesc>();
    shape_op->update_output_desc_y(*shape_tensor_desc);
    auto shape_operator = UpdateOperatorMap(
        shape_operand,
        std::make_shared<Operator>(shape_op, shape_tensor_desc, "y", -1));
    // Slice op
    auto slice_op =
        std::make_shared<ge::op::Slice>(resize_nearest_name + "/slice");
    std::vector<int> offsets{2};
    std::vector<int> size{2};
    auto offsets_operator = AddInt32ConstantOperator(offsets);
    auto size_operator = AddInt32ConstantOperator(size);
    SET_INPUT(slice_op, x, shape_operator);
    SET_INPUT(slice_op, offsets, offsets_operator);
    SET_INPUT(slice_op, size, size_operator);
    auto slice_tensor_desc = std::make_shared<ge::TensorDesc>();
    slice_op->update_output_desc_y(*slice_tensor_desc);
    auto slice_operator = UpdateOperatorMap(
        nullptr,
        std::make_shared<Operator>(slice_op, slice_tensor_desc, "y", -1));
    // Cast op
    auto slice_cast_op =
        std::make_shared<ge::op::Cast>(resize_nearest_name + "/slice_cast");
    slice_cast_op->set_attr_dst_type(ge::DT_FLOAT);
    SET_INPUT(slice_cast_op, x, slice_operator);
    auto slice_cast_tensor_desc = std::make_shared<ge::TensorDesc>();
    slice_cast_op->update_output_desc_y(*slice_cast_tensor_desc);
    auto slice_cast_operator =
        UpdateOperatorMap(nullptr,
                          std::make_shared<Operator>(
                              slice_cast_op, slice_cast_tensor_desc, "y", -1));
    // Mul op
    auto mul_op = std::make_shared<ge::op::Mul>(resize_nearest_name + "/mul");
    SET_INPUT(mul_op, x1, slice_cast_operator);
    SET_INPUT(mul_op, x2, scale_operator);
    auto mul_tensor_desc = std::make_shared<ge::TensorDesc>();
    mul_op->update_output_desc_y(*mul_tensor_desc);
    auto mul_operator = UpdateOperatorMap(
        nullptr, std::make_shared<Operator>(mul_op, mul_tensor_desc, "y", -1));
    // Cast op
    auto mul_cast_op =
        std::make_shared<ge::op::Cast>(resize_nearest_name + "/mul_cast");
    mul_cast_op->set_attr_dst_type(ge::DT_INT32);
    SET_INPUT(mul_cast_op, x, mul_operator);
    auto mul_cast_tensor_desc = std::make_shared<ge::TensorDesc>();
    mul_cast_op->update_output_desc_y(*mul_cast_tensor_desc);
    auto mul_cast_operator = UpdateOperatorMap(
        nullptr,
        std::make_shared<Operator>(mul_cast_op, mul_cast_tensor_desc, "y", -1));
    SET_INPUT(resize_nearest_op, size, mul_cast_operator);
  } else {
    NNADAPTER_LOG(WARNING) << "Either shape_operand or scales_operand should "
                              "be set.";
    return NNADAPTER_INVALID_PARAMETER;
  }
  resize_nearest_op->set_attr_align_corners(align_corners);
  MAP_OUTPUT(resize_nearest_op, y, output_operand);
  return NNADAPTER_NO_ERROR;
}

}  // namespace huawei_ascend_npu
}  // namespace nnadapter
