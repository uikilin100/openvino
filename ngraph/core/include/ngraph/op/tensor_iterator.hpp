// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include <vector>

#include "ngraph/function.hpp"
#include "ngraph/op/parameter.hpp"
#include "ngraph/op/util/sub_graph_base.hpp"

namespace ngraph {
namespace op {
namespace v0 {
/// \brief  Iterate a body over tensors, accumulating into tensors.
class NGRAPH_API TensorIterator : public op::util::SubGraphOp {
public:
    NGRAPH_RTTI_DECLARATION;

    bool visit_attributes(AttributeVisitor& visitor) override;

    TensorIterator() = default;
    explicit TensorIterator(const OutputVector& values);

    std::shared_ptr<Node> clone_with_new_inputs(const OutputVector& new_args) const override;
    /// \return the body of the iteration
    std::shared_ptr<Function> get_body() const {
        return m_bodies[0];
    }
    /// \param body set the body of the iteration
    void set_body(const std::shared_ptr<Function>& body) {
        set_function(body);
    }
    void validate_and_infer_types() override;
    void revalidate_and_infer_types_for_body_ops();

private:
    void try_to_set_num_iterations_if_no_slice_inputs();
};
}  // namespace v0
using v0::TensorIterator;
}  // namespace op
}  // namespace ngraph
