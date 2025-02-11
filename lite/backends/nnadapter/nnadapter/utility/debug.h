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

#pragma once

#include <string>
#include "core/hal/types.h"

namespace nnadapter {

// Visualize to dot file
std::string Visualize(hal::Model* model);

// NNAdapterType2string
std::string ResultCodeToString(NNAdapterResultCode type);
std::string OperandPrecisionCodeToString(NNAdapterOperandPrecisionCode type);
std::string OperandPrecisionCodeToSymbol(NNAdapterOperandPrecisionCode type);
std::string OperandLayoutCodeToString(NNAdapterOperandLayoutCode type);
std::string OperandLifetimeCodeToString(NNAdapterOperandLifetimeCode type);
std::string OperationTypeToString(NNAdapterOperationType type);
std::string FuseCodeToString(NNAdapterFuseCode type);
std::string DeviceCodeToString(NNAdapterDeviceCode type);
std::string DimensionsToString(const int32_t* dimensions,
                               uint32_t dimension_count);
std::string OperandToString(hal::Operand* operand);
std::string OperandIdToString(hal::Operand* operand);
std::string OperandTypeToString(NNAdapterOperandType* type);
std::string OperandValueToString(hal::Operand* operand);
std::string OperationIdToString(hal::Operation* operation);

}  // namespace nnadapter
