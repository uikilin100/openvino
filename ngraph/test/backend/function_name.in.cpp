// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include "gtest/gtest.h"
#include "ngraph/ngraph.hpp"
#include "ngraph/runtime/tensor.hpp"
#include "runtime/backend.hpp"
#include "util/all_close_f.hpp"
#include "util/ndarray.hpp"
#include "util/test_control.hpp"
#include "util/test_tools.hpp"

using namespace std;
using namespace ngraph;

static string s_manifest = "${MANIFEST}";

NGRAPH_TEST(${BACKEND_NAME}, function_name) {
    Shape shape{2, 2};
    auto A = make_shared<op::Parameter>(element::f32, shape);
    auto B = make_shared<op::Parameter>(element::f32, shape);
    auto add = make_shared<op::v1::Add>(A, B);
    auto f = make_shared<Function>(add, ParameterVector{A, B}, "funky func name");

    auto backend = runtime::Backend::create("${BACKEND_NAME}");

    // Create some tensors for input/output
    shared_ptr<runtime::Tensor> a = backend->create_tensor<float>(shape);
    shared_ptr<runtime::Tensor> b = backend->create_tensor<float>(shape);
    shared_ptr<runtime::Tensor> result = backend->create_tensor<float>(shape);

    copy_data(a, test::NDArray<float, 2>({{1, 2}, {3, 4}}).get_vector());
    copy_data(b, test::NDArray<float, 2>({{5, 6}, {7, 8}}).get_vector());

    auto handle = backend->compile(f);
    handle->call_with_validate({result}, {a, b});
    EXPECT_TRUE(
        test::all_close_f(read_vector<float>(result), (test::NDArray<float, 2>({{6, 8}, {10, 12}})).get_vector()));
}
