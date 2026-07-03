#include <gtest/gtest.h>

#include "hlp_test.hpp"

auto constexpr NAME = "ignoreParser";

INSTANTIATE_TEST_SUITE_P(IgnoreBuild,
                         HlpBuildTest,
                         ::testing::Values(BuildT(FAILURE, getIgnoreParser, {NAME, "", {}, {}}),
                                           BuildT(SUCCESS, getIgnoreParser, {NAME, "", {}, {"ignore"}}),
                                           BuildT(FAILURE, getIgnoreParser, {NAME, "", {}, {"ignore", "unexpected"}}),
                                           BuildT(FAILURE, getIgnoreParser, {NAME, "not allow", {}, {"ignore"}})));

INSTANTIATE_TEST_SUITE_P(
    IgnoreParse,
    HlpParseTest,
    ::testing::Values(ParseT(SUCCESS, "guardsarm", j("{}"), 5, getIgnoreParser, {NAME, "", {}, {"guardsarm"}}),
                      ParseT(SUCCESS, "guardsarm 123", j("{}"), 5, getIgnoreParser, {NAME, "", {}, {"guardsarm"}}),
                      ParseT(SUCCESS, "guardsarmguardsarm", j("{}"), 10, getIgnoreParser, {NAME, "", {}, {"guardsarm"}}),
                      ParseT(SUCCESS, "guardsarmguardsarmguardsarmguardsarm", j("{}"), 20, getIgnoreParser, {NAME, "", {}, {"guardsarm"}}),
                      ParseT(SUCCESS, "guardsarmwa", j("{}"), 5, getIgnoreParser, {NAME, "", {}, {"guardsarm"}}),
                      ParseT(FAILURE, "GUARDSARM", j("{}"), 0, getIgnoreParser, {NAME, "", {}, {"guardsarm"}})));
