//
// Created by wjt on 18-5-4.
//

#ifndef FPGA_CONNECTOR_COMMON_H
#define FPGA_CONNECTOR_COMMON_H

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

#define MAX_Cond 8
#define MAX_Columns      49
#define MAX_Cmp_Bytes    64
#define LINE_BIT 8
#define TABLE_LEN (int)(pow(2, MAX_Cond)/LINE_BIT)

using namespace std;

#endif //FPGA_CONNECTOR_COMMON_H
