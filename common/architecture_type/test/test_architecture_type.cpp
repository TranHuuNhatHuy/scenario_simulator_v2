// Copyright 2026 The Autoware Contributors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <architecture_type/architecture_type.hpp>
#include <gtest/gtest.h>

using namespace common::architecture_type;

TEST(ArchitectureType, universe_is_recognised)
{
  EXPECT_TRUE(isUniverse("awf/universe"));
  EXPECT_TRUE(isUniverse("awf/universe/20240605"));
  EXPECT_TRUE(isSupported("awf/universe/20230906"));
  EXPECT_TRUE(isSupported("awf/universe/20250130"));
}

TEST(ArchitectureType, the_test_is_a_prefix_match_not_a_substring_search)
{
  // Behaviour change from the `find("awf/universe") != npos` this package replaces: a value that
  // merely contains the architecture name no longer passes.
  EXPECT_FALSE(isSupported(" awf/universe"));
  EXPECT_FALSE(isSupported("-awf/universe"));
  EXPECT_TRUE(isSupported("awf/universe-"));
}

TEST(ArchitectureType, unknown_values_are_not_supported)
{
  EXPECT_FALSE(isSupported("tier4/proposal"));
  EXPECT_FALSE(isSupported(""));
  EXPECT_FALSE(isSupported("awf"));
}
