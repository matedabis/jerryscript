// Copyright JS Foundation and other contributors, http://js.foundation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

function f()
{
  return 1;
}

assert(typeof(a) === "undefined");
assert(typeof(null) === "object");
assert(typeof(false) === "boolean");
assert(typeof(true) === "boolean");
assert(typeof(1) === "number");
assert(typeof(1.1) === "number");
assert(typeof('abcd') === "string");
assert(typeof("1.1") === "string");
assert(typeof(this) === "object");
assert(typeof(f) === "function");

assert (typeof (null) !== "undefined");
assert (typeof (false) !== "undefined");
assert (typeof (true) !== "undefined");
assert (typeof (1) !== "undefined");
assert (typeof (1.1) !== "undefined");
assert (typeof ('abcd') !== "undefined");
assert (typeof ("1.1") !== "undefined");
assert (typeof (this) !== "undefined");
assert (typeof (f) !== "undefined");

assert (typeof (a) !== "object");
assert (typeof (false) !== "object");
assert (typeof (true) !== "object");
assert (typeof (1) !== "object");
assert (typeof (1.1) !== "object");
assert (typeof ('abcd') !== "object");
assert (typeof ("1.1") !== "object");
assert (typeof (f) !== "object");

assert (typeof (a) !== "boolean");
assert (typeof (null) !== "boolean");
assert (typeof (1) !== "boolean");
assert (typeof (1.1) !== "boolean");
assert (typeof ('abcd') !== "boolean");
assert (typeof ("1.1") !== "boolean");
assert (typeof (this) !== "boolean");
assert (typeof (f) !== "boolean");

assert (typeof (a) !== "number");
assert (typeof (null) !== "number");
assert (typeof (false) !== "number");
assert (typeof (true) !== "number");
assert (typeof ('abcd') !== "number");
assert (typeof ("1.1") !== "number");
assert (typeof (this) !== "number");
assert (typeof (f) !== "number");

assert (typeof (a) !== "string");
assert (typeof (null) !== "string");
assert (typeof (false) !== "string");
assert (typeof (true) !== "string");
assert (typeof (1) !== "string");
assert (typeof (1.1) !== "string");
assert (typeof (this) !== "string");
assert (typeof (f) !== "string");

assert (typeof (a) !== "function");
assert (typeof (null) !== "function");
assert (typeof (false) !== "function");
assert (typeof (true) !== "function");
assert (typeof (1) !== "function");
assert (typeof (1.1) !== "function");
assert (typeof ('abcd') !== "function");
assert (typeof ("1.1") !== "function");
assert (typeof (this) !== "function");
