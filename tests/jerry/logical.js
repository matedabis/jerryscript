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

assert((!false) === true);

assert (false === false);
assert (true === true);

assert (true !== false);
assert (true !== undefined);
assert (false !== undefined);

assert((true || true) === true);
assert((false || true) === true);
assert((false || false) === false);
assert((true || false) === true);

assert ((true || true) !== false);
assert ((false || true) !== false);
assert ((false || false) !== true);
assert ((true || false) !== false);

assert ((true || true) !== undefined);
assert ((false || true) !== undefined);
assert ((false || false) !== undefined);
assert ((true || false) !== undefined);

assert((true && true) === true);
assert((true && false) === false);
assert((false && true) === false);
assert((false && false) === false);

assert ((true && true) !== false);
assert ((true && false) !== true);
assert ((false && true) !== true);
assert ((false && false) !== true);

assert ((true && true) !== undefined);
assert ((true && false) !== undefined);
assert ((false && true) !== undefined);
assert ((false && false) !== undefined);
