// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#pragma once

#include "ngraph/op/util/unary_elementwise_arithmetic.hpp"

namespace ngraph {
namespace op {
namespace v0 {
/// \brief Elementwise natural exponential (exp) operation.
class NGRAPH_API Exp : public util::UnaryElementwiseArithmetic {
public:
    NGRAPH_RTTI_DECLARATION;

    /// \brief Constructs an exponential operation.
    Exp() = default;
    /// \brief Constructs an exponential operation.
    ///
    /// \param arg Node that produces the input tensor.
    Exp(const Output<Node>& arg);

    bool visit_attributes(AttributeVisitor& visitor) override;
    virtual std::shared_ptr<Node> clone_with_new_inputs(const OutputVector& new_args) const override;

    bool evaluate(const HostTensorVector& outputs, const HostTensorVector& inputs) const override;
    bool has_evaluate() const override;
};
}  // namespace v0
using v0::Exp;
}  // namespace op
}  // namespace ngraph
