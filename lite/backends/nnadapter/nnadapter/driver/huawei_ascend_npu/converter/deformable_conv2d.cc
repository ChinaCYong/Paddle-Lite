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
/**
 * fuse deformable_offset and conv2d as deformable_conv2d
 *
 * [input] [offsets]  [mask]
 *    \             \ /
 *     \             |
 *      \          concat
 *       \         /
 *        \       /
 * deformable_offset(input) [filter]  [bias]
 *                       \     |     /
 *                           \   /
 *                           conv2d
 *                             |
 *                      deformable_conv2d
 */
int Program::ConvertDeformableConv2d(hal::Operation* operation) {
  auto& input_operands = operation->input_operands;
  auto& output_operands = operation->output_operands;
  auto input_count = input_operands.size();
  auto output_count = output_operands.size();
  NNADAPTER_CHECK_EQ(input_count, 16);
  NNADAPTER_CHECK_EQ(output_count, 1);
  // Input
  auto input_operand = input_operands[0];
  NNADAPTER_VLOG(5) << "input: " << OperandToString(input_operand);
  // Offset
  auto offset_operand = input_operands[1];
  NNADAPTER_VLOG(5) << "offset: " << OperandToString(offset_operand);
  auto offset_channel = offset_operand->type.dimensions[1];
  // Mask
  auto mask_operand = input_operands[2];
  NNADAPTER_VLOG(5) << "mask: " << OperandToString(mask_operand);
  // Filter
  auto filter_operand = input_operands[3];
  NNADAPTER_VLOG(5) << "filter: " << OperandToString(filter_operand);
  auto filter_height = filter_operand->type.dimensions[2];
  auto filter_width = filter_operand->type.dimensions[3];
  // Bias
  auto bias_operand = input_operands[4];
  NNADAPTER_VLOG(5) << "bias: " << OperandToString(bias_operand);
  // Paddings
  auto padding_width_left =
      *reinterpret_cast<int32_t*>(input_operands[5]->buffer);
  auto padding_width_right =
      *reinterpret_cast<int32_t*>(input_operands[6]->buffer);
  auto padding_height_top =
      *reinterpret_cast<int32_t*>(input_operands[7]->buffer);
  auto padding_height_bottom =
      *reinterpret_cast<int32_t*>(input_operands[8]->buffer);
  NNADAPTER_VLOG(5) << "paddings=[" << padding_width_left << ","
                    << padding_width_right << "," << padding_height_top << ","
                    << padding_height_bottom << "]";
  // Strides
  auto stride_width = *reinterpret_cast<int32_t*>(input_operands[9]->buffer);
  auto stride_height = *reinterpret_cast<int32_t*>(input_operands[10]->buffer);
  NNADAPTER_VLOG(5) << "strides=[" << stride_width << "," << stride_height
                    << "]";
  // Group
  auto group = *reinterpret_cast<int32_t*>(input_operands[11]->buffer);
  NNADAPTER_VLOG(5) << "group=" << group;
  // Deformable groups
  auto deformable_groups =
      *reinterpret_cast<int32_t*>(input_operands[12]->buffer);
  NNADAPTER_VLOG(5) << "deformable_groups=" << deformable_groups;
  // Fuse code
  auto fuse_code = *reinterpret_cast<int32_t*>(input_operands[13]->buffer);
  NNADAPTER_VLOG(5) << "fuse_code=" << fuse_code;
  // Dilations
  auto dilation_width = *reinterpret_cast<int32_t*>(input_operands[14]->buffer);
  auto dilation_height =
      *reinterpret_cast<int32_t*>(input_operands[15]->buffer);
  NNADAPTER_VLOG(5) << "dilations=[" << dilation_width << "," << dilation_height
                    << "]";
  // Output
  auto output_operand = output_operands[0];
  NNADAPTER_VLOG(5) << "output: " << OperandToString(output_operand);

  // Convert to GE operators
  auto input_operator = GetMappedOperator(input_operand);
  if (!input_operator) {
    input_operator = ConvertOperand(input_operand);
  }
  auto offset_operator = GetMappedOperator(offset_operand);
  if (!offset_operator) {
    offset_operator = ConvertOperand(offset_operand);
  }
  auto mask_operator = GetMappedOperator(mask_operand);
  if (!mask_operator) {
    mask_operator = ConvertOperand(mask_operand);
  }

  /**
   * Convert the offsets data arrangement in deformable_offsets
   *
   * NNAdapter:     yyyyy          ->      Ascend:   xxxxx
   *                xxxxx                            xxxxx
   *                yyyyy                            xxxxx
   *                xxxxx                            yyyyy
   *                yyyyy                            yyyyy
   *                xxxxx                            yyyyy
   *
   */
  // Slice offset_x in the channel dimension
  auto x_begin_operator = AddInt32ConstantOperator(std::vector<int32_t>({1}));
  auto x_end_operator =
      AddInt32ConstantOperator(std::vector<int32_t>({offset_channel}));
  auto x_strides_operator = AddInt32ConstantOperator(2);
  auto x_axes_operator = AddInt32ConstantOperator(1);
  auto slice_x_name = GetOperatorName(output_operand) + "/split_x";
  auto slice_x_op = std::make_shared<ge::op::StridedSliceV2>(slice_x_name);
  SET_INPUT(slice_x_op, x, offset_operator);
  SET_INPUT(slice_x_op, begin, x_begin_operator);
  SET_INPUT(slice_x_op, end, x_end_operator);
  SET_INPUT(slice_x_op, strides, x_strides_operator);
  SET_INPUT(slice_x_op, axes, x_axes_operator);
  auto slice_x_operator = MAP_OUTPUT(slice_x_op, y, output_operand);

  // Slice offset_y in the channel dimension
  auto y_begin_operator = AddInt32ConstantOperator(std::vector<int32_t>({0}));
  auto y_end_operator =
      AddInt32ConstantOperator(std::vector<int32_t>({offset_channel - 1}));
  auto y_strides_operator = AddInt32ConstantOperator(2);
  auto y_axes_operator = AddInt32ConstantOperator(1);
  auto slice_y_name = GetOperatorName(output_operand) + "/split_y";
  auto slice_y_op = std::make_shared<ge::op::StridedSliceV2>(slice_y_name);
  SET_INPUT(slice_y_op, x, offset_operator);
  SET_INPUT(slice_y_op, begin, y_begin_operator);
  SET_INPUT(slice_y_op, end, y_end_operator);
  SET_INPUT(slice_y_op, strides, y_strides_operator);
  SET_INPUT(slice_y_op, axes, y_axes_operator);
  auto slice_y_operator = MAP_OUTPUT(slice_y_op, y, output_operand);

  // Concat the offset and mask in the channel dimension
  auto concat_name = GetOperatorName(output_operand) + "/concat";
  auto concat_op = std::make_shared<ge::op::ConcatD>(concat_name);
  concat_op->set_attr_concat_dim(1);
  concat_op->set_attr_N(3);
  concat_op->create_dynamic_input_x(3);
  SET_DYNAMIC_INPUT(concat_op, x, 0, slice_x_operator);
  SET_DYNAMIC_INPUT(concat_op, x, 1, slice_y_operator);
  SET_DYNAMIC_INPUT(concat_op, x, 2, mask_operator);
  auto concat_operator = MAP_OUTPUT(concat_op, y, output_operand);

  /**
   * Create deformable_offsets operator
   *
   * deformable_offset args:
   *   tensor:
   *      x:          use input_operand
   *      offsets:    use concat_operator output
   *   attrs:
   *      [ksize]:                {filter_height, filter_width}
   *      [strides]:              {1, 1, stride_height, stride_width}
   *      [paddings]:             {padding_height_top,
   *                               padding_height_bottom,
   *                               padding_width_left,
   *                               padding_width_right}
   *      [dilations]:            {1, 1, dilation_height, dilation_width}
   *      [data_format]:          NCHW
   *      [deformable_groups]:    deformable_groups
   *      [modulated]:            true
   */
  auto deformable_offsets_name = GetOperatorName(output_operand);
  auto deformable_offsets_op =
      std::make_shared<ge::op::DeformableOffsets>(deformable_offsets_name);
  deformable_offsets_op->set_attr_strides(
      ge::Operator::OpListInt({1, 1, stride_height, stride_width}));
  deformable_offsets_op->set_attr_pads(
      ge::Operator::OpListInt({padding_height_top,
                               padding_height_bottom,
                               padding_width_left,
                               padding_width_right}));
  deformable_offsets_op->set_attr_ksize(
      ge::Operator::OpListInt({filter_height, filter_width}));
  deformable_offsets_op->set_attr_dilations(
      ge::Operator::OpListInt({1, 1, dilation_height, dilation_width}));
  deformable_offsets_op->set_attr_deformable_groups(deformable_groups);
  deformable_offsets_op->set_attr_data_format("NCHW");
  deformable_offsets_op->set_attr_modulated(true);
  SET_INPUT(deformable_offsets_op, x, input_operator);
  SET_INPUT(deformable_offsets_op, offsets, concat_operator);
  auto deformable_offsets_operator =
      MAP_OUTPUT(deformable_offsets_op, y, output_operand);

  /** Create deformable_offsets operator
   *
   * conv2d args:
   *   tensor:
   *      x:              use deformable_offsets operator output
   *      filter:         use filter operator
   *      bias:           use bias operator
   *   attrs:
   *      [strides]:      {1, 1, filter_height, filter_width}
   *      [pads]:         defualt:{0, 0, 0, 0}
   *      [dilations]:    defualt:{1, 1, 1, 1}
   *      [data_format]:  NCHW
   */
  auto conv_name = GetOperatorName(output_operand);
  auto filter_operator = ConvertOperand(filter_operand);
  auto bias_operator = ConvertOperand(bias_operand);
  auto conv_op = std::make_shared<ge::op::Conv2D>(conv_name);
  conv_op->set_attr_pads(ge::Operator::OpListInt({0, 0, 0, 0}));
  conv_op->set_attr_dilations(ge::Operator::OpListInt({1, 1, 1, 1}));
  conv_op->set_attr_strides(
      ge::Operator::OpListInt({1, 1, filter_height, filter_width}));
  conv_op->set_attr_data_format("NCHW");
  SET_INPUT(conv_op, x, deformable_offsets_operator);
  SET_INPUT(conv_op, filter, filter_operator);
  SET_INPUT(conv_op, bias, bias_operator);
  auto conv_operator = MAP_OUTPUT(conv_op, y, output_operand);

  // Fuse activations
  auto act_name = GetOperatorName(output_operand);
  switch (fuse_code) {
#define CONVERT_UNARY_ACTIVATION(type, class_name)                \
  case NNADAPTER_FUSED_##type: {                                  \
    auto act_op = std::make_shared<ge::op::class_name>(act_name); \
    SET_INPUT(act_op, x, conv_operator);                          \
    MAP_OUTPUT(act_op, y, output_operand);                        \
  } break;
    CONVERT_UNARY_ACTIVATION(RELU, Relu);
    CONVERT_UNARY_ACTIVATION(RELU6, Relu6);
#undef CONVERT_UNARY_ACTIVATION
    case NNADAPTER_FUSED_NONE:
      break;
    default:
      NNADAPTER_LOG(FATAL) << "Unsupported fuse_code(" << fuse_code
                           << ") is found.";
      break;
  }
  return NNADAPTER_NO_ERROR;
}

}  // namespace huawei_ascend_npu
}  // namespace nnadapter
