//
// Created by wjt on 18-5-8.
//

#include "CreateTruthTable.h"
#include "ConditionTree.h"

void CreateTruthTable::create_true_table(int cond_num, BtreeNode *tree, unsigned char *true_table) {
    ConditionTree *CTree = new ConditionTree();
    int i = 0;
    //ap_int<1> tmp; ap_int<1> cond_array[NUM_COND];
    unsigned tmp = 0;
    unsigned cond_array[MAX_Cond] = {0};
    int table_line = (int) pow(2, cond_num);
    for (; i < table_line; i++) {
        int k = 0;
        for (; k < cond_num; k++) {
            //cond_array[index[k] = i>>1 & 1;
            cond_array[cond_num - k - 1] = (unsigned) ((i >> k) & 1);
        }
//        for (int qq = 0; qq < cond_num; qq++)
//            cout << cond_array[qq] << "\t";
//        cout << "\n";
        if ((tmp = CTree->compute(cond_array, tree))) {
//            this->bit_set(&true_table[TABLE_LEN - i / LINE_BIT - 1], i % LINE_BIT, 1);
            this->bit_set(&true_table[i / LINE_BIT], i % LINE_BIT, 1);
        }
//        printf("tmp:%d\t", tmp);
    }

}


void CreateTruthTable::bit_set(unsigned char *p_data, int position, int flag) {
    if (flag == 1) {
        *p_data |= (1 << (position));
    } else if (flag == 0) {
        *p_data &= ~(1 << (position));
    }
}