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

// The package is header-only; this translation unit exists so that ament_auto_add_library has
// something to build, matching how common/get_parameter and common/scenario_simulator_exception
// are packaged in this repository. Including the header here also means a syntax error in it
// fails this package's own build rather than a consumer's.
#include <architecture_type/architecture_type.hpp>
