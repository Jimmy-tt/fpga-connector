//
// Created by wjt on 18-5-8.
//

#ifndef FPGA_CONNECTOR_CREATETRUTHTABLE_H
#define FPGA_CONNECTOR_CREATETRUTHTABLE_H

#include "common.h"
#include "ConditionTree.h"

//#include<ap_uint.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<assert.h>
#include<math.h>
//#include<string.h>
#define MAX_COND 8


class CreateTruthTable {
public:
    void create_true_table(int cond_num, BtreeNode *tree, unsigned char *true_table);

    void bit_set(unsigned char *p_data, int position, int flag);

};

#endif //FPGA_CONNECTOR_CREATETRUTHTABLE_H
