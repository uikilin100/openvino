// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <functional>
#include <map>
#include <ngraph/runtime/host_tensor.hpp>
#include <vector>

#include "ngraph/node.hpp"
#include "ngraph/op/util/nms_base.hpp"
#include "ngraph/op/util/op_types.hpp"
#include "ngraph/ops.hpp"
#include "ngraph/shape_util.hpp"

namespace ngraph {
namespace runtime {
namespace reference {
void multiclass_nms(const float* boxes_data,
                    const Shape& boxes_data_shape,
                    const float* scores_data,
                    const Shape& scores_data_shape,
                    const op::v8::MulticlassNms::Attributes& attrs,
                    float* selected_outputs,
                    const Shape& selected_outputs_shape,
                    int64_t* selected_indices,
                    const Shape& selected_indices_shape,
                    int64_t* valid_outputs);

}  // namespace reference
}  // namespace runtime
}  // namespace ngraph
