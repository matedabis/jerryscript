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

var b = 5;

assert((b += 10) == 15);

assert(b != 14);
assert(b != 16);
assert(!(b >16 || b < 14))

assert((b -= 3) == 12);

assert(b != 11);
assert(b != 13);
assert(!(b > 13 || b < 11));

assert((b *= 10) == 120);

assert(b != 119);
assert(b != 121);
assert(!(b > 121 || b < 119));

assert((b /= 10) == 12);

assert(b != 11);
assert(b != 13);
assert(!(b > 13 || b < 11));

assert((b %= 10) == 2);

assert(b != 1);
assert(b != 3);
assert(!(b > 3 || b < 1));
