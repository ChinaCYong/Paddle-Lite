// Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.
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

#include "driver/rockchip_npu/converter/converter.h"
#include <unistd.h>
#include <algorithm>
#include <vector>
#include "driver/rockchip_npu/optimizer/fix_ops.h"
#include "driver/rockchip_npu/optimizer/unpack_op_fusion.h"
#include "optimizer/symm2asymm.h"
#include "utility/debug.h"
#include "utility/logging.h"
#include "utility/modeling.h"
#include "utility/string.h"
#include "utility/utility.h"

namespace nnadapter {
namespace rockchip_npu {

#define REGISTER_CONVERTER(__op_type__, __func_name__) \
  extern int __func_name__(Converter* converter, hal::Operation* operation);
#include "driver/rockchip_npu/converter/all.h"  // NOLINT
#undef __NNADAPTER_DRIVER_ROCKCHIP_CONVERTER_ALL_H__
#undef REGISTER_CONVERTER

int Converter::Apply(hal::Model* model) {
  // Convert the NNAdapter operations to the rknn operators
  std::vector<hal::Operation*> operations =
      SortOperationsInTopologicalOrder(model);
  for (auto operation : operations) {
    NNADAPTER_VLOG(5) << "Converting " << OperationTypeToString(operation->type)
                      << " ...";
    switch (operation->type) {
#define REGISTER_CONVERTER(__op_type__, __func_name__) \
  case NNADAPTER_##__op_type__:                        \
    __func_name__(this, operation);                    \
    break;
#include "driver/rockchip_npu/converter/all.h"  // NOLINT
#undef __NNADAPTER_DRIVER_ROCKCHIP_CONVERTER_ALL_H__
#undef REGISTER_CONVERTER
      default:
        NNADAPTER_LOG(FATAL) << "Unsupported operation("
                             << OperationTypeToString(operation->type)
                             << ") is found.";
        break;
    }
  }
  return NNADAPTER_NO_ERROR;
}

std::string Converter::GetTensorName(hal::Operand* operand) {
  auto operand_id = OperandIdToString(operand);
  auto index = 0;
  auto it = tensors_->find(operand);
  if (it != tensors_->end()) {
    index = it->second.size();
  }
  return operand_id + string_format("_%d", index);
}

std::shared_ptr<rk::nn::Tensor> Converter::GetMappedTensor(
    hal::Operand* operand) {
  auto it = tensors_->find(operand);
  if (it != tensors_->end()) {
    return it->second.back();
  }
  return nullptr;
}

std::shared_ptr<rk::nn::Tensor> Converter::UpdateTensorMap(
    hal::Operand* operand, std::shared_ptr<rk::nn::Tensor> tensor) {
  auto it = tensors_->find(operand);
  if (it == tensors_->end()) {
    auto result = tensors_->insert(std::make_pair(
        operand, std::vector<std::shared_ptr<rk::nn::Tensor>>()));
    NNADAPTER_CHECK(result.second);
    it = result.first;
  }
  it->second.push_back(tensor);
  return tensor;
}

std::shared_ptr<rk::nn::Tensor> Converter::AddConstantTensor(
    void* values,
    int32_t* dimensions,
    uint32_t dimension_count,
    rk::nn::PrecisionType precision,
    const float* quant_scale,
    const int32_t* zero_point) {
  auto name = GetTensorName(nullptr);
  auto tensor = CreateRknnTensor(graph_,
                                 name,
                                 dimensions,
                                 dimension_count,
                                 precision,
                                 quant_scale,
                                 zero_point,
                                 values,
                                 rk::nn::DataLayoutType::NCHW);
  NNADAPTER_CHECK(tensor);
  UpdateTensorMap(nullptr, tensor);
  return tensor;
}

std::shared_ptr<rk::nn::Tensor> Converter::AddVariableTensor(
    const std::string& name,
    int32_t* dimensions,
    uint32_t dimension_count,
    rk::nn::PrecisionType precision,
    const float* quant_scale,
    const int32_t* zero_point) {
  return CreateRknnTensor(graph_,
                          name,
                          dimensions,
                          dimension_count,
                          precision,
                          quant_scale,
                          zero_point,
                          nullptr,
                          rk::nn::DataLayoutType::NCHW);
}

std::shared_ptr<rk::nn::Tensor> Converter::AddQuant8ConstantTensor(
    uint8_t* values,
    int32_t* dimensions,
    uint32_t dimension_count,
    float quant_scale,
    int32_t zero_point) {
  return AddConstantTensor(values,
                           dimensions,
                           dimension_count,
                           rk::nn::PrecisionType::UINT8,
                           &quant_scale,
                           &zero_point);
}

std::shared_ptr<rk::nn::Tensor> Converter::AddQuant32ConstantTensor(
    int32_t* values,
    int32_t* dimensions,
    uint32_t dimension_count,
    float quant_scale) {
  return AddConstantTensor(values,
                           dimensions,
                           dimension_count,
                           rk::nn::PrecisionType::INT32,
                           &quant_scale,
                           nullptr);
}

std::shared_ptr<rk::nn::Tensor> Converter::AddQuant8VariableTensor(
    const std::string& name,
    int32_t* dimensions,
    uint32_t dimension_count,
    float quant_scale,
    int32_t zero_point) {
  return AddVariableTensor(name,
                           dimensions,
                           dimension_count,
                           rk::nn::PrecisionType::UINT8,
                           &quant_scale,
                           &zero_point);
}

std::shared_ptr<rk::nn::Tensor> Converter::ConvertOperand(
    hal::Operand* operand, std::vector<int32_t> dimensions) {
  auto tensor = CreateRknnTensor(graph_,
                                 GetTensorName(operand),
                                 &operand->type,
                                 operand->buffer,
                                 dimensions);
  NNADAPTER_CHECK(tensor);
  UpdateTensorMap(operand, tensor);
  return tensor;
}

std::shared_ptr<rk::nn::Operator> Converter::AddOperator(
    rk::nn::OperatorType type,
    std::vector<std::shared_ptr<rk::nn::Tensor>> input_tensors,
    std::vector<std::shared_ptr<rk::nn::Tensor>> output_tensors,
    void* attrs,
    std::string name) {
  return graph_->AddOperator(type, input_tensors, output_tensors, attrs, name);
}

}  // namespace rockchip_npu
}  // namespace nnadapter
