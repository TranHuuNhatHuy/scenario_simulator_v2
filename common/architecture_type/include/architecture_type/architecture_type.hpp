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

#ifndef ARCHITECTURE_TYPE__ARCHITECTURE_TYPE_HPP_
#define ARCHITECTURE_TYPE__ARCHITECTURE_TYPE_HPP_

#include <get_parameter/get_parameter.hpp>
#include <iomanip>
#include <scenario_simulator_exception/exception.hpp>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/*
   `architecture_type` names the Autoware interface generation a scenario run targets. It decides
   which topic names the simulator publishes on, which sensor bridges exist, and which launch file
   the Ego entity forks.

   Upstream scenario_simulator_v2 recognises only "awf/universe*" and spells the test for it as a
   bare `find("awf/universe") != npos` at eight separate sites. That is why running against
   autoware_core previously required lying about the architecture -- passing
   awf/universe/20240605 and overriding the launch package underneath, which every gate happily
   accepted because each is a substring or lexicographic test that string satisfies.

   This package is that one definition. It changes no accepted value and no default: it exists
   so that the next architecture can be added by editing one file rather than by finding all
   eight sites, and so that a rejected value produces one error message that lists what is
   accepted.

   One deliberate behaviour change: the test is a prefix match rather than a substring search,
   so " awf/universe" (leading space) is now rejected instead of silently accepted. A value that
   only happens to contain the architecture name was never meant to pass.
*/
namespace common
{
namespace architecture_type
{
inline constexpr std::string_view universe_prefix = "awf/universe";

/// Unchanged from the value the eight call sites hard-coded before this package existed.
inline constexpr auto default_architecture_type = "awf/universe/20240605";

inline auto startsWith(const std::string & value, const std::string_view prefix) -> bool
{
  return value.compare(0, prefix.size(), prefix) == 0;
}

inline auto isUniverse(const std::string & architecture_type) -> bool
{
  return startsWith(architecture_type, universe_prefix);
}

inline auto isSupported(const std::string & architecture_type) -> bool
{
  return isUniverse(architecture_type);
}

inline auto supported() -> std::vector<std::string>
{
  return {"awf/universe/20230906", "awf/universe/20240605", "awf/universe/20250130"};
}

/// The value in force for this process, read once from the ROS parameter of the same name.
inline auto current() -> const std::string &
{
  static const std::string architecture_type =
    getParameter<std::string>("architecture_type", std::string(default_architecture_type));
  return architecture_type;
}

/// Throw a scenario-level error naming the accepted values. Called by every site that used to
/// fall through to an unhelpful "Unexpected architecture_type".
[[noreturn]] inline auto reject(const std::string & architecture_type, const std::string & context)
  -> void
{
  auto what = std::stringstream();
  what << "Unexpected architecture_type " << std::quoted(architecture_type) << " in " << context
       << ". Supported values are: ";
  auto first = true;
  for (const auto & value : supported()) {
    what << (std::exchange(first, false) ? "" : ", ") << std::quoted(value);
  }
  what << ".";
  throw common::SemanticError(what.str());
}
}  // namespace architecture_type
}  // namespace common

#endif  // ARCHITECTURE_TYPE__ARCHITECTURE_TYPE_HPP_
