// Copyright (C) 2018-2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <algorithm>
#include <cinttypes>
#include <cmath>
#include <cstdlib>
#include <random>
#include <string>

// clang-format off
#ifdef ${BACKEND_NAME}_FLOAT_TOLERANCE_BITS
#define DEFAULT_FLOAT_TOLERANCE_BITS ${BACKEND_NAME}_FLOAT_TOLERANCE_BITS
#endif

#ifdef ${BACKEND_NAME}_DOUBLE_TOLERANCE_BITS
#define DEFAULT_DOUBLE_TOLERANCE_BITS ${BACKEND_NAME}_DOUBLE_TOLERANCE_BITS
#endif
// clang-format on

#include "gtest/gtest.h"
#include "ngraph/ngraph.hpp"
#include "util/engine/test_engines.hpp"
#include "util/test_case.hpp"
#include "util/test_control.hpp"

using namespace std;
using namespace ngraph;

static string s_manifest = "${MANIFEST}";
using TestEngine = test::ENGINE_CLASS_NAME(${BACKEND_NAME});

NGRAPH_TEST(${BACKEND_NAME}, floor) {
    Shape shape{2, 2};
    auto A = make_shared<op::Parameter>(element::f32, shape);
    auto f = make_shared<Function>(make_shared<op::Floor>(A), ParameterVector{A});

    auto test_case = test::TestCase<TestEngine>(f);
    test_case.add_input<float>({-2.5f, -2.0f, 0.3f, 4.8f});
    test_case.add_expected_output<float>(shape, {-3.0f, -2.0f, 0.0f, 4.0f});
    test_case.run();
}

NGRAPH_TEST(${BACKEND_NAME}, floor_int32) {
    Shape shape{2, 2};
    auto A = make_shared<op::Parameter>(element::i32, shape);
    auto f = make_shared<Function>(make_shared<op::Floor>(A), ParameterVector{A});

    auto test_case = test::TestCase<TestEngine>(f);
    test_case.add_input<int32_t>({-2, -136314888, 0x40000010, 0x40000001});
    test_case.add_expected_output<int32_t>(shape, {-2, -136314888, 0x40000010, 0x40000001});
    test_case.run();
}

NGRAPH_TEST(${BACKEND_NAME}, floor_int64) {
    // This tests large numbers that will not fit in a double
    Shape shape{3};
    auto A = make_shared<op::Parameter>(element::i64, shape);
    auto f = make_shared<Function>(make_shared<op::Floor>(A), ParameterVector{A});

    auto test_case = test::TestCase<TestEngine>(f);
    test_case.add_input<int64_t>({0, 1, 0x4000000000000001});
    test_case.add_expected_output<int64_t>(shape, {0, 1, 0x4000000000000001});
    test_case.run();
}
