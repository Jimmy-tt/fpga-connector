# Amazon FPGA Hardware Development Kit
#
# Copyright 2016 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Amazon Software License (the "License"). You may not use
# this file except in compliance with the License. A copy of the License is
# located at
#
#    http://aws.amazon.com/asl/
#
# or in the "license" file accompanying this file. This file is distributed on
# an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, express or
# implied. See the License for the specific language governing permissions and
# limitations under the License.


INCLUDES = -I$(SDK_DIR)/userspace/include

CC = g++
CFLAGS = -DCONFIG_LOGLEVEL=4 -std=gnu++11 -g -Wall -Werror $(INCLUDES)
LDLIBS = -lrt -lpthread

SRC = main.cpp ConditionHandle.cpp ConditionTree.cpp CreateTruthTable.cpp ColumnHandle.cpp ComplexCondHandle.cpp
OBJ = $(SRC:.cpp=.o)
BIN = fpga_connector-master

all: check_env $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f *.o $(BIN)

check_env:
#ifndef SDK_DIR
#    $(error SDK_DIR is undefined. Try "source sdk_setup.sh" to set the software environment)
#endif


