#!/usr/bin/env python


# Copyright JS Foundation and other contributors, http://js.foundation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import random

file = open("tests/jerry/arithmetics-4.js", "w")

random.seed(87673)

for i in range(2000):

    N1 = random.randint(-99999999999, 99999999999)
    N2 = random.randint(-99999999999, 99999999999)
    while N2 == 0:
            N2 = random.randint(-99, 99)

    if (((N1 < 0 and N2 > 0) or (N1 > 0 and N2 < 0)) and (N1 % N2 != 0) ):
        N3 = ((N1 % N2) - N2)
    else:
        N3 = (N1 % N2)

    file.write("assert ((" + str(N1) + ") % (" + str(N2) + ") == (" + str(int(N3)) + "));\n")
    file.write("assert ((" + str(N1) + ") % (" + str(N2) + ") != (" + str(int((N3) - 1)) + "));\n")
    file.write("assert ((" + str(N1) + ") % (" + str(N2) + ") != (" + str(int((N3) + 1)) + "));\n\n")

file.close()
