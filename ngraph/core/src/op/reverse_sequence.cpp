// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "ngraph/op/reverse_sequence.hpp"

#include <algorithm>
#include <memory>

#include "itt.hpp"
#include "ngraph/attribute_visitor.hpp"
#include "ngraph/node.hpp"
#include "ngraph/validation_util.hpp"

using namespace std;
using namespace ngraph;

NGRAPH_RTTI_DEFINITION(op::ReverseSequence, "ReverseSequence", 0);

op::ReverseSequence::ReverseSequence(const Output<Node>& arg,
                                     const Output<Node>& seq_indices,
                                     int64_t batch_axis,
                                     int64_t seq_axis)
    : Op({arg, seq_indices}),
      m_batch_axis(batch_axis),
      m_seq_axis(seq_axis),
      m_normalized_batch_axis{0},
      m_normalized_seq_axis{0} {
    constructor_validate_and_infer_types();
}

bool ngraph::op::v0::ReverseSequence::visit_attributes(AttributeVisitor& visitor) {
    NGRAPH_OP_SCOPE(v0_ReverseSequence_visit_attributes);
    visitor.on_attribute("batch_axis", m_batch_axis);
    visitor.on_attribute("seq_axis", m_seq_axis);
    return true;
}

void op::ReverseSequence::validate_and_infer_types() {
    NGRAPH_OP_SCOPE(v0_ReverseSequence_validate_and_infer_types);
    auto input_shape = get_input_partial_shape(0);
    auto input_rank = input_shape.rank();

    m_normalized_batch_axis = ngraph::normalize_axis(this, m_batch_axis, input_rank);
    m_normalized_seq_axis = ngraph::normalize_axis(this, m_seq_axis, input_rank);

    auto indices_shape = get_input_partial_shape(1);
    auto indices_rank = indices_shape.rank();

    NODE_VALIDATION_CHECK(this,
                          indices_rank.is_dynamic() || indices_rank.get_length() == 1,
                          "Sequence indices must be a 1-dimensional tensor (sequence indices shape: ",
                          indices_shape,
                          ").");

    PartialShape output_shape{input_shape};

    if (input_rank.is_static() && indices_rank.is_static()) {
        Dimension merged_sequence_length;

        NODE_VALIDATION_CHECK(
            this,
            Dimension::merge(merged_sequence_length, input_shape[m_normalized_batch_axis], indices_shape[0]),
            "Sequence length (",
            indices_shape[0],
            ") is not equal to batch axis ",
            "dimension (",
            input_shape[m_normalized_batch_axis],
            ") (argument shape: ",
            input_shape,
            ", sequence indices shape: ",
            indices_shape,
            ").");
        output_shape[m_normalized_batch_axis] = merged_sequence_length;
    }

    set_output_type(0, get_input_element_type(0), output_shape);
}

shared_ptr<Node> op::ReverseSequence::clone_with_new_inputs(const OutputVector& new_args) const {
    NGRAPH_OP_SCOPE(v0_ReverseSequence_clone_with_new_inputs);
    check_new_args_count(this, new_args);
    return make_shared<ReverseSequence>(new_args.at(0), new_args.at(1), m_batch_axis, m_seq_axis);
}
