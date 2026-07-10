/*
 * GuardSarm shared modules utils
 * Copyright (C) 2026 GuardSarm, Inc.
 * Nov 1, 2023.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#include "guardsarmDBQueryBuilder_test.hpp"
#include "guardsarmDBQueryBuilder.hpp"
#include <string>

TEST_F(GuardSarmDBQueryBuilderTest, GlobalTest)
{
    std::string message = GuardSarmDBQueryBuilder::builder().global().selectAll().fromTable("agent").build();
    EXPECT_EQ(message, "global sql SELECT * FROM agent ");
}

TEST_F(GuardSarmDBQueryBuilderTest, AgentTest)
{
    std::string message = GuardSarmDBQueryBuilder::builder().agent("0").selectAll().fromTable("sys_programs").build();
    EXPECT_EQ(message, "agent 0 sql SELECT * FROM sys_programs ");
}

TEST_F(GuardSarmDBQueryBuilderTest, WhereTest)
{
    std::string message = GuardSarmDBQueryBuilder::builder()
                              .agent("0")
                              .selectAll()
                              .fromTable("sys_programs")
                              .whereColumn("name")
                              .equalsTo("bash")
                              .build();
    EXPECT_EQ(message, "agent 0 sql SELECT * FROM sys_programs WHERE name = 'bash' ");
}

TEST_F(GuardSarmDBQueryBuilderTest, WhereAndTest)
{
    std::string message = GuardSarmDBQueryBuilder::builder()
                              .agent("0")
                              .selectAll()
                              .fromTable("sys_programs")
                              .whereColumn("name")
                              .equalsTo("bash")
                              .andColumn("version")
                              .equalsTo("1")
                              .build();
    EXPECT_EQ(message, "agent 0 sql SELECT * FROM sys_programs WHERE name = 'bash' AND version = '1' ");
}

TEST_F(GuardSarmDBQueryBuilderTest, WhereOrTest)
{
    std::string message = GuardSarmDBQueryBuilder::builder()
                              .agent("0")
                              .selectAll()
                              .fromTable("sys_programs")
                              .whereColumn("name")
                              .equalsTo("bash")
                              .orColumn("version")
                              .equalsTo("1")
                              .build();
    EXPECT_EQ(message, "agent 0 sql SELECT * FROM sys_programs WHERE name = 'bash' OR version = '1' ");
}

TEST_F(GuardSarmDBQueryBuilderTest, WhereIsNullTest)
{
    std::string message = GuardSarmDBQueryBuilder::builder()
                              .agent("0")
                              .selectAll()
                              .fromTable("sys_programs")
                              .whereColumn("name")
                              .isNull()
                              .build();
    EXPECT_EQ(message, "agent 0 sql SELECT * FROM sys_programs WHERE name IS NULL ");
}

TEST_F(GuardSarmDBQueryBuilderTest, WhereIsNotNullTest)
{
    std::string message = GuardSarmDBQueryBuilder::builder()
                              .agent("0")
                              .selectAll()
                              .fromTable("sys_programs")
                              .whereColumn("name")
                              .isNotNull()
                              .build();
    EXPECT_EQ(message, "agent 0 sql SELECT * FROM sys_programs WHERE name IS NOT NULL ");
}

TEST_F(GuardSarmDBQueryBuilderTest, InvalidValue)
{
    EXPECT_THROW(GuardSarmDBQueryBuilder::builder()
                     .agent("0")
                     .selectAll()
                     .fromTable("sys_programs")
                     .whereColumn("name")
                     .equalsTo("bash'")
                     .build(),
                 std::runtime_error);
}

TEST_F(GuardSarmDBQueryBuilderTest, InvalidColumn)
{
    EXPECT_THROW(GuardSarmDBQueryBuilder::builder()
                     .agent("0")
                     .selectAll()
                     .fromTable("sys_programs")
                     .whereColumn("name'")
                     .equalsTo("bash")
                     .build(),
                 std::runtime_error);
}

TEST_F(GuardSarmDBQueryBuilderTest, InvalidTable)
{
    EXPECT_THROW(GuardSarmDBQueryBuilder::builder()
                     .agent("0")
                     .selectAll()
                     .fromTable("sys_programs'")
                     .whereColumn("name")
                     .equalsTo("bash")
                     .build(),
                 std::runtime_error);
}

TEST_F(GuardSarmDBQueryBuilderTest, GlobalGetCommand)
{
    std::string message = GuardSarmDBQueryBuilder::builder().globalGetCommand("agent-info 1").build();
    EXPECT_EQ(message, "global get-agent-info 1 ");
}

TEST_F(GuardSarmDBQueryBuilderTest, GlobalFindCommand)
{
    std::string message = GuardSarmDBQueryBuilder::builder().globalFindCommand("agent 1").build();
    EXPECT_EQ(message, "global find-agent 1 ");
}

TEST_F(GuardSarmDBQueryBuilderTest, GlobalSelectCommand)
{
    std::string message = GuardSarmDBQueryBuilder::builder().globalSelectCommand("agent-name 1").build();
    EXPECT_EQ(message, "global select-agent-name 1 ");
}

TEST_F(GuardSarmDBQueryBuilderTest, AgentGetOsInfoCommand)
{
    std::string message = GuardSarmDBQueryBuilder::builder().agentGetOsInfoCommand("1").build();
    EXPECT_EQ(message, "agent 1 osinfo get ");
}

TEST_F(GuardSarmDBQueryBuilderTest, AgentGetHotfixesCommand)
{
    std::string message = GuardSarmDBQueryBuilder::builder().agentGetHotfixesCommand("1").build();
    EXPECT_EQ(message, "agent 1 hotfix get ");
}

TEST_F(GuardSarmDBQueryBuilderTest, AgentGetPackagesCommand)
{
    std::string message = GuardSarmDBQueryBuilder::builder().agentGetPackagesCommand("1").build();
    EXPECT_EQ(message, "agent 1 package get ");
}
