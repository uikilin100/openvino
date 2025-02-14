// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "gtest/gtest.h"
#include "ngraph/ngraph.hpp"
#include "ngraph/runtime/tensor.hpp"
#include "runtime/backend.hpp"
#include "util/all_close_f.hpp"
#include "util/test_control.hpp"
#include "util/test_tools.hpp"

using namespace std;
using namespace ngraph;

static string s_manifest = "${MANIFEST}";

NGRAPH_TEST(${BACKEND_NAME}, create_dynamic_backend) {
    auto backend = runtime::Backend::create("${BACKEND_NAME}", true);
    ASSERT_NE(backend, nullptr);
    ASSERT_TRUE(backend->supports_dynamic_tensors());
}

NGRAPH_TEST(${BACKEND_NAME}, create_dynamic_tensor) {
    auto backend = runtime::Backend::create("${BACKEND_NAME}", true);
    auto t = backend->create_dynamic_tensor(element::f32, PartialShape{2, Dimension::dynamic(), 3});
    ASSERT_TRUE(t->get_partial_shape().same_scheme(PartialShape{2, Dimension::dynamic(), 3}));
}

NGRAPH_TEST(${BACKEND_NAME}, dynamic_abc) {
    //
    // Create a graph for f(a,b,c) = (a+b)*c, where a, b, c all have shape {2,?,3}.
    //
    auto a = make_shared<op::Parameter>(element::f32, PartialShape{2, Dimension::dynamic(), 3});
    auto b = make_shared<op::Parameter>(element::f32, PartialShape{2, Dimension::dynamic(), 3});
    auto c = make_shared<op::Parameter>(element::f32, PartialShape{2, Dimension::dynamic(), 3});

    auto a_plus_b = make_shared<op::v1::Add>(a, b);
    auto a_plus_b_times_c = make_shared<op::v1::Multiply>(a_plus_b, c);

    auto f = make_shared<Function>(NodeVector{a_plus_b_times_c}, ParameterVector{a, b, c});

    //
    // Get a backend with dynamic support, and compile f.
    //
    auto backend = runtime::Backend::create("${BACKEND_NAME}", true);

    auto ex = backend->compile(f);

    //
    // Create a dynamic output tensor with shape {2,?,3}.
    //
    auto t_r = backend->create_dynamic_tensor(element::f32, PartialShape{2, Dimension::dynamic(), 3});

    //
    // For each of n=[0,...,5), run the compiled executable against a test vector of shape
    // {2,n,3}, and check the results.
    //
    for (size_t middle_dim = 0; middle_dim < 5; middle_dim++) {
        // Fill in some test input values, which we'll use for a, b, and c.
        vector<float> inputs(2 * middle_dim * 3);
        for (size_t i = 0; i < 2 * middle_dim * 3; i++) {
            inputs[i] = i;
        }

        // Create static tensors for the inputs and copy data.
        auto t_a = backend->create_tensor(element::f32, Shape{2, middle_dim, 3});
        auto t_b = backend->create_tensor(element::f32, Shape{2, middle_dim, 3});
        auto t_c = backend->create_tensor(element::f32, Shape{2, middle_dim, 3});

        copy_data(t_a, inputs);
        copy_data(t_b, inputs);
        copy_data(t_c, inputs);

        // Call ex, writing result into t_r (note we're using the same t_r from outside the loop.)
        ex->call_with_validate({t_r}, {t_a, t_b, t_c});

        // After call, t_r should have a shape of {2,n,3}.
        ASSERT_EQ(t_r->get_shape(), (Shape{2, middle_dim, 3}));

        // Read out the results, and compare them against expected values.
        auto results = read_vector<float>(t_r);

        vector<float> expected_values(2 * middle_dim * 3);
        for (size_t i = 0; i < 2 * middle_dim * 3; i++) {
            expected_values[i] = (i + i) * i;
        }

        EXPECT_TRUE(test::all_close_f(results, expected_values));
    }
}

static void axpy_test(const PartialShape& input_pshape, const std::vector<Shape>& input_shapes) {
    auto a = make_shared<op::Parameter>(element::f32, input_pshape);
    auto x = make_shared<op::Parameter>(element::f32, input_pshape);
    auto y = make_shared<op::Parameter>(element::f32, input_pshape);

    auto axpy = make_shared<op::v1::Add>(make_shared<op::v1::Multiply>(a, x), y);

    auto f = make_shared<Function>(NodeVector{axpy}, ParameterVector{a, x, y});
    auto backend = runtime::Backend::create("${BACKEND_NAME}", true);
    auto ex = backend->compile(f);

    auto t_r = backend->create_dynamic_tensor(element::f32, input_pshape);

    for (auto& shape : input_shapes) {
        vector<float> inputs(shape_size(shape));
        for (size_t i = 0; i < shape_size(shape); i++) {
            inputs[i] = i;
        }

        auto t_a = backend->create_tensor(element::f32, shape);
        auto t_x = backend->create_tensor(element::f32, shape);
        auto t_y = backend->create_tensor(element::f32, shape);

        copy_data(t_a, inputs);
        copy_data(t_x, inputs);
        copy_data(t_y, inputs);

        ex->call_with_validate({t_r}, {t_a, t_x, t_y});

        ASSERT_EQ(t_r->get_shape(), shape);

        auto results = read_vector<float>(t_r);

        vector<float> expected_values(shape_size(shape));
        for (size_t i = 0; i < shape_size(shape); i++) {
            expected_values[i] = (i * i) + i;
        }

        EXPECT_TRUE(test::all_close_f(results, expected_values));
    }
}

NGRAPH_TEST(${BACKEND_NAME}, dynamic_axpy) {
    // Test with shape {?, 3, 3}.
    axpy_test(PartialShape{Dimension::dynamic(), 3, 3}, {Shape{2, 3, 3}, Shape{5, 3, 3}});

    // Test with shape {?, ?, ?}.
    axpy_test(PartialShape::dynamic(3), {Shape{2, 3, 3}, Shape{5, 3, 3}, Shape{2, 5, 2}, Shape{8, 1, 8}});

    // Test with shape ?. (Rank unknown.)
    axpy_test(PartialShape::dynamic(),
              {Shape{2, 3, 3},
               Shape{5, 3, 3},
               Shape{2, 5, 2},
               Shape{8, 1, 8},
               Shape{5},
               Shape{8, 2},
               Shape{8, 2, 8, 2},
               Shape{2, 3, 4, 5, 2}});
}

static void to_vector_test(const PartialShape& input_pshape, const std::vector<Shape>& input_shapes) {
    auto x = make_shared<op::Parameter>(element::f32, input_pshape);

    shared_ptr<Node> x_new_shape = make_shared<op::v0::ShapeOf>(x);
    auto axes = op::Constant::create(element::i64, {}, {0});
    x_new_shape = make_shared<op::v1::ReduceProd>(x_new_shape, axes);
    x_new_shape = make_shared<op::v1::Reshape>(x_new_shape, op::Constant::create(element::u64, {1}, Shape{1}), false);

    auto x_reshaped = make_shared<op::v1::Reshape>(x, x_new_shape, true);

    auto f = make_shared<Function>(NodeVector{x_reshaped}, ParameterVector{x});
    auto backend = runtime::Backend::create("${BACKEND_NAME}", true);
    auto ex = backend->compile(f);

    auto t_r = backend->create_dynamic_tensor(element::f32, PartialShape::dynamic(1));

    for (auto& shape : input_shapes) {
        vector<float> inputs(shape_size(shape));
        for (size_t i = 0; i < shape_size(shape); i++) {
            inputs[i] = i;
        }

        auto t_x = backend->create_tensor(element::f32, shape);

        copy_data(t_x, inputs);

        ex->call_with_validate({t_r}, {t_x});

        ASSERT_EQ(t_r->get_shape(), (Shape{shape_size(shape)}));

        auto results = read_vector<float>(t_r);

        EXPECT_TRUE(test::all_close_f(results, inputs));
    }
}

NGRAPH_TEST(${BACKEND_NAME}, dynamic_to_vector) {
    // Test with shape {?, 3, 3}.
    to_vector_test(PartialShape{Dimension::dynamic(), 3, 3}, {Shape{2, 3, 3}, Shape{5, 3, 3}});

    // Test with shape {?, ?, ?}.
    to_vector_test(PartialShape::dynamic(3), {Shape{2, 3, 3}, Shape{5, 3, 3}, Shape{2, 5, 2}, Shape{8, 1, 8}});

    // Test with shape ?. (Rank unknown.)
    to_vector_test(PartialShape::dynamic(),
                   {Shape{2, 3, 3},
                    Shape{5, 3, 3},
                    Shape{2, 5, 2},
                    Shape{8, 1, 8},
                    Shape{5},
                    Shape{8, 2},
                    Shape{8, 2, 8, 2},
                    Shape{2, 3, 4, 5, 2}});
}

static void reverse_shape_test(const PartialShape& input_pshape, const std::vector<Shape>& input_shapes) {
    auto x = make_shared<op::Parameter>(element::f32, input_pshape);

    shared_ptr<Node> x_new_shape = make_shared<op::v0::ShapeOf>(x);
    x_new_shape = make_shared<op::v1::Reverse>(x_new_shape,
                                               op::Constant::create(element::i64, {1}, {0}),
                                               op::v1::Reverse::Mode::INDEX);

    auto x_reshaped = make_shared<op::v1::Reshape>(x, x_new_shape, true);

    auto f = make_shared<Function>(NodeVector{x_reshaped}, ParameterVector{x});
    auto backend = runtime::Backend::create("${BACKEND_NAME}", true);
    auto ex = backend->compile(f);

    auto t_r = backend->create_dynamic_tensor(element::f32, PartialShape::dynamic());

    for (auto& shape : input_shapes) {
        vector<float> inputs(shape_size(shape));
        for (size_t i = 0; i < shape_size(shape); i++) {
            inputs[i] = i;
        }

        auto t_x = backend->create_tensor(element::f32, shape);

        copy_data(t_x, inputs);

        ex->call_with_validate({t_r}, {t_x});

        Shape expected_shape = shape;
        std::reverse(expected_shape.begin(), expected_shape.end());
        ASSERT_EQ(t_r->get_shape(), expected_shape);

        auto results = read_vector<float>(t_r);

        EXPECT_TRUE(test::all_close_f(results, inputs));
    }
}

NGRAPH_TEST(${BACKEND_NAME}, dynamic_reverse_shape) {
    // Test with shape {?, 3, 3}.
    reverse_shape_test(PartialShape{Dimension::dynamic(), 3, 3}, {Shape{2, 3, 3}, Shape{5, 3, 3}});

    // Test with shape {?, ?, ?}.
    reverse_shape_test(PartialShape::dynamic(3), {Shape{2, 3, 3}, Shape{5, 3, 3}, Shape{2, 5, 2}, Shape{8, 1, 8}});

    // Test with shape ?. (Rank unknown.)
    reverse_shape_test(PartialShape::dynamic(),
                       {Shape{2, 3, 3},
                        Shape{5, 3, 3},
                        Shape{2, 5, 2},
                        Shape{8, 1, 8},
                        Shape{5},
                        Shape{8, 2},
                        Shape{8, 2, 8, 2},
                        Shape{2, 3, 4, 5, 2}});
}

NGRAPH_TEST(${BACKEND_NAME}, dynamic_transpose) {
    auto arg = std::make_shared<op::Parameter>(element::i32, PartialShape::dynamic());
    auto input_order = make_shared<op::Parameter>(element::i32, PartialShape::dynamic());
    auto transpose = std::make_shared<op::v1::Transpose>(arg, input_order);

    auto f = std::make_shared<Function>(NodeVector{transpose}, ParameterVector{arg, input_order});

    auto backend = runtime::Backend::create("${BACKEND_NAME}", true);
    auto ex = backend->compile(f);

    auto arg_data = vector<int32_t>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    auto input_order_data = vector<int32_t>{2, 0, 1};

    auto arg_tensor = backend->create_tensor(element::i32, Shape{2, 2, 3});
    auto input_order_tensor = backend->create_tensor(element::i32, Shape{input_order_data.size()});
    copy_data(arg_tensor, arg_data);
    copy_data(input_order_tensor, input_order_data);

    auto output = backend->create_dynamic_tensor(element::i32, PartialShape::dynamic());
    ex->call_with_validate({output}, {arg_tensor, input_order_tensor});

    ASSERT_EQ(output->get_element_type(), element::i32);
    EXPECT_EQ(read_vector<int32_t>(output), vector<int32_t>({1, 4, 7, 10, 2, 5, 8, 11, 3, 6, 9, 12}));
}
