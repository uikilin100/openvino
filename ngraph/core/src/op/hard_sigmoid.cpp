// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "ngraph/op/hard_sigmoid.hpp"

#include <memory>

#include "itt.hpp"
#include "ngraph/op/add.hpp"
#include "ngraph/op/constant.hpp"
#include "ngraph/op/maximum.hpp"
#include "ngraph/op/minimum.hpp"
#include "ngraph/op/multiply.hpp"
#include "ngraph/shape.hpp"

using namespace std;
using namespace ngraph;

NGRAPH_SUPPRESS_DEPRECATED_START

NGRAPH_RTTI_DEFINITION(op::HardSigmoid, "HardSigmoid", 0, op::util::FusedOp);

op::HardSigmoid::HardSigmoid() : FusedOp() {}

op::HardSigmoid::HardSigmoid(const Output<Node>& data, const Output<Node>& alpha, const Output<Node>& beta)
    : FusedOp({data, alpha, beta}) {
    constructor_validate_and_infer_types();
}

bool ngraph::op::v0::HardSigmoid::visit_attributes(AttributeVisitor& visitor) {
    NGRAPH_OP_SCOPE(v0_HardSigmoid_visit_attributes);
    return true;
}

void op::HardSigmoid::pre_validate_and_infer_types() {
    const auto& alpha_pshape = get_input_partial_shape(1);
    const auto& beta_pshape = get_input_partial_shape(2);

    if (alpha_pshape.is_static()) {
        const auto alpha_shape = alpha_pshape.to_shape();
        NODE_VALIDATION_CHECK(this,
                              is_scalar(alpha_shape),
                              "A scalar is expected for the 'alpha' input. Got: ",
                              alpha_shape);
    }

    if (beta_pshape.is_static()) {
        const auto beta_shape = beta_pshape.to_shape();
        NODE_VALIDATION_CHECK(this,
                              is_scalar(beta_shape),
                              "A scalar is expected for the 'beta' input. Got: ",
                              beta_shape);
    }

    const auto& data_et = get_input_element_type(0);
    const auto& alpha_et = get_input_element_type(1);
    const auto& beta_et = get_input_element_type(2);

    NODE_VALIDATION_CHECK(this,
                          data_et == alpha_et && data_et == beta_et,
                          "The element types of both alpha and beta inputs must match the data input type.");

    set_output_type(0, get_input_element_type(0), get_input_partial_shape(0));
}

OutputVector op::HardSigmoid::decompose_op() const {
    const auto data = input_value(0);

    const auto one_node = ngraph::op::Constant::create<float>(data.get_element_type(), data.get_shape(), {1.0f});

    const auto zero_node = ngraph::op::Constant::create<float>(data.get_element_type(), data.get_shape(), {0.0f});

    const auto alpha_node = input_value(1).get_node_shared_ptr();
    const auto beta_node = input_value(2).get_node_shared_ptr();

    std::shared_ptr<Node> alpha_x_plus_beta =
        std::make_shared<op::v1::Multiply>(alpha_node, data, AutoBroadcastType::NUMPY);

    alpha_x_plus_beta = std::make_shared<op::v1::Add>(alpha_x_plus_beta, beta_node, AutoBroadcastType::NUMPY);

    return {
        std::make_shared<op::v1::Minimum>(std::make_shared<op::v1::Maximum>(alpha_x_plus_beta, zero_node), one_node)};
}

shared_ptr<Node> op::HardSigmoid::clone_with_new_inputs(const OutputVector& new_args) const {
    NGRAPH_OP_SCOPE(v0_HardSigmoid_clone_with_new_inputs);
    check_new_args_count(this, new_args);

    return make_shared<HardSigmoid>(new_args.at(0), new_args.at(1), new_args.at(2));
}
