#include "builders/baseBuilders_test.hpp"

#include "builders/opfilter/opBuilderHelperFilter.hpp"
#include "syntax.hpp"

namespace
{

auto contextExpected(bool indexUnclassified)
{
    return [=](const BuildersMocks& mocks)
    {
        static Context ctx;
        ctx.assetName = "test/asset";
        ctx.policyName = "test/policy";
        ctx.indexUnclassifiedEvents = indexUnclassified;

        ON_CALL(*mocks.ctx, context()).WillByDefault(testing::ReturnRef(ctx));

        return None {};
    };
}

} // namespace

namespace filterbuildtest
{
class IndexUnclassifiedEventsBuildTest : public BaseBuilderTest
{
};

TEST_F(IndexUnclassifiedEventsBuildTest, BuildsWithCategoryField)
{
    Reference targetField {"guardsarm.integration.category"};

    contextExpected(true)(*mocks);
    expectBuildSuccess();

    ASSERT_NO_THROW(opfilter::opBuilderHelperIndexUnclassifiedEvents(targetField, {}, mocks->ctx));
}

TEST_F(IndexUnclassifiedEventsBuildTest, RejectsUnexpectedTargetField)
{
    // Any field other than guardsarm.integration.category must be rejected at build-time.
    for (const auto& path : {"targetField", "guardsarm.integration.decoders", "guardsarm.integration.name"})
    {
        Reference targetField {path};
        ASSERT_THROW(opfilter::opBuilderHelperIndexUnclassifiedEvents(targetField, {}, mocks->ctx), std::exception)
            << "Expected throw for target field: " << path;
    }
}

TEST_F(IndexUnclassifiedEventsBuildTest, RejectsValueArgument)
{
    Reference targetField {"guardsarm.integration.category"};

    ASSERT_THROW(opfilter::opBuilderHelperIndexUnclassifiedEvents(targetField, {makeValue(R"(1)")}, mocks->ctx),
                 std::exception);
}

TEST_F(IndexUnclassifiedEventsBuildTest, RejectsReferenceArgument)
{
    Reference targetField {"guardsarm.integration.category"};

    ASSERT_THROW(opfilter::opBuilderHelperIndexUnclassifiedEvents(targetField, {makeRef("ref")}, mocks->ctx),
                 std::exception);
}
} // namespace filterbuildtest

namespace filteroperatestest
{
INSTANTIATE_TEST_SUITE_P(Builders,
                         FilterOperationTest,
                         testing::Values(
                             /*** Index Unclassified Events - Tests with policy disabled (default) ***/
                             // Policy disabled, category unclassified - should fail
                             FilterT(R"({"guardsarm": {"integration": {"category": "unclassified"}}})",
                                     opfilter::opBuilderHelperIndexUnclassifiedEvents,
                                     "guardsarm.integration.category",
                                     {},
                                     FAILURE(contextExpected(false))),
                             // Policy disabled, category security - should fail
                             FilterT(R"({"guardsarm": {"integration": {"category": "security"}}})",
                                     opfilter::opBuilderHelperIndexUnclassifiedEvents,
                                     "guardsarm.integration.category",
                                     {},
                                     FAILURE(contextExpected(false))),
                             // Policy disabled, category missing - should fail
                             FilterT(R"({"guardsarm": {"integration": {}}})",
                                     opfilter::opBuilderHelperIndexUnclassifiedEvents,
                                     "guardsarm.integration.category",
                                     {},
                                     FAILURE(contextExpected(false))),
                             /*** Index Unclassified Events - Tests with policy enabled ***/
                             // Policy enabled, category unclassified - should succeed
                             FilterT(R"({"guardsarm": {"integration": {"category": "unclassified"}}})",
                                     opfilter::opBuilderHelperIndexUnclassifiedEvents,
                                     "guardsarm.integration.category",
                                     {},
                                     SUCCESS(contextExpected(true))),
                             // Policy enabled, category security - should fail
                             FilterT(R"({"guardsarm": {"integration": {"category": "security"}}})",
                                     opfilter::opBuilderHelperIndexUnclassifiedEvents,
                                     "guardsarm.integration.category",
                                     {},
                                     FAILURE(contextExpected(true))),
                             // Policy enabled, category system-activity - should fail
                             FilterT(R"({"guardsarm": {"integration": {"category": "system-activity"}}})",
                                     opfilter::opBuilderHelperIndexUnclassifiedEvents,
                                     "guardsarm.integration.category",
                                     {},
                                     FAILURE(contextExpected(true))),
                             /*** Index Unclassified Events - Error cases ***/
                             // Policy enabled, category field missing - should fail (not found)
                             FilterT(R"({"guardsarm": {"integration": {}}})",
                                     opfilter::opBuilderHelperIndexUnclassifiedEvents,
                                     "guardsarm.integration.category",
                                     {},
                                     FAILURE(contextExpected(true))),
                             // Policy enabled, completely unrelated event - should fail (not found)
                             FilterT(R"({"other": "x"})",
                                     opfilter::opBuilderHelperIndexUnclassifiedEvents,
                                     "guardsarm.integration.category",
                                     {},
                                     FAILURE(contextExpected(true))),
                             // Policy enabled, category is not a string (number) - should fail (wrong type)
                             FilterT(R"({"guardsarm": {"integration": {"category": 123}}})",
                                     opfilter::opBuilderHelperIndexUnclassifiedEvents,
                                     "guardsarm.integration.category",
                                     {},
                                     FAILURE(contextExpected(true))),
                             // Policy enabled, category is not a string (array) - should fail (wrong type)
                             FilterT(R"({"guardsarm": {"integration": {"category": ["unclassified"]}}})",
                                     opfilter::opBuilderHelperIndexUnclassifiedEvents,
                                     "guardsarm.integration.category",
                                     {},
                                     FAILURE(contextExpected(true)))),
                         testNameFormatter<FilterOperationTest>("IndexUnclassifiedEvents"));
} // namespace filteroperatestest
