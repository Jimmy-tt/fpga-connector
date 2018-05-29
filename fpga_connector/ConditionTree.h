//
// Created by wjt on 18-5-8.
//

#ifndef FPGA_CONNECTOR_CONDITIONTREE_H
#define FPGA_CONNECTOR_CONDITIONTREE_H

#include "common.h"

class BtreeNode {
public:
    char data;
    bool isMerge = false;
    unsigned sum;
    BtreeNode *lchild;
    BtreeNode *rchild;
};

class ConditionTree {
public:
    //条件表达式转化二叉树
/*
   afa为指向表达式字符串的指针
   s为要转化的表达式字符串的起始位置
   e为要转化的表达式字符串的结束位置的后一个
*/
    BtreeNode *afaToBtree(char *afa, int s, int e) {
        //如果只有一个数那就是叶子结点了
        if (e - s == 1) {
            BtreeNode *bn = new BtreeNode();
            bn->data = afa[s];
            bn->lchild = NULL;
            bn->rchild = NULL;
            return bn;
        }
        /*
           local_r记录当前要转化的表达式生成二叉树的根节点操作符的位置
           flag记录是否当前搜索在括号里面
           m_m_p记录当前表达式中括号外面最右边的|位置
           a_s_p记录当前表达式中括号外面最右边的&位置
        */
        int local_r = 0, flag = 0;
        int m_m_p = 0, a_s_p = 0;
        for (int i = s; i < e; i++) {
            if (afa[i] == '(')flag++;
            else if (afa[i] == ')')flag--;
            if (flag == 0) {
                if (afa[i] == '&')
                    m_m_p = i;
                else if (afa[i] == '|')
                    a_s_p = i;
            }
        }
        if ((m_m_p == 0) && (a_s_p == 0))
            //如果式子整个有括号如(a&b|c&d)，即括号外面没有操作符，则去掉括号找二叉树
            afaToBtree(afa, s + 1, e - 1);
        else {
            //如果有|，则根节点为最右边的|，否则是最右边的&
            if (a_s_p > 0)local_r = a_s_p;
            else if (m_m_p > 0)local_r = m_m_p;
            //确定根节点和根节点的左孩子和右孩子
//            BtreeNode *b = (struct BtreeNode *) malloc(sizeof(struct BtreeNode));
            BtreeNode *b = new BtreeNode();
            b->data = afa[local_r];
            b->lchild = afaToBtree(afa, s, local_r);
            b->rchild = afaToBtree(afa, local_r + 1, e);
            return b;
        }
    }

    void post_order_traverser(unsigned *cond_array, BtreeNode *pTree) {
        if (pTree != NULL) {
            post_order_traverser(cond_array, pTree->lchild);
            post_order_traverser(cond_array, pTree->rchild);
            char c_s[2] = {pTree->data, '\0'};
            string str = c_s;
            if (!pTree->isMerge && ((str.compare("|")) == 0 || str.compare("&") == 0)) {
                unsigned ldata = this->treeMerge(cond_array, pTree->lchild);
                unsigned rdata = this->treeMerge(cond_array, pTree->rchild);
                if (str.compare("|") == 0) {
                    pTree->sum = ldata | rdata;
                } else {
                    pTree->sum = ldata & rdata;
                }
                pTree->isMerge = true;
//                free(pTree->lchild);
//                pTree->lchild = NULL;
//                free(pTree->rchild);
//                pTree->rchild = NULL;
            }
        }
    }

    unsigned treeMerge(unsigned *cond_array, BtreeNode *pTree) {
        vector<int> lineNum;
        unsigned sum;
        if (!pTree->isMerge) {
            char c_s[2] = {pTree->data, '\0'};
            string str = c_s;
            sum = cond_array[atoi(c_s)];
        } else {
            sum = pTree->sum;
        }
        pTree->isMerge = false;
        return sum;
    }

    unsigned compute(unsigned *cond_array, BtreeNode *pTree) {
        if (pTree->data != '|' && pTree->data != '&') {
            pTree->sum = this->treeMerge(cond_array, pTree);
        } else {
            this->post_order_traverser(cond_array, pTree);
            pTree->isMerge = false;
        }
        return pTree->sum;
    }

    void freeTree(BtreeNode *pTree) {
        if (pTree != NULL) {
            freeTree(pTree->lchild);
            freeTree(pTree->rchild);
            free(pTree);
            pTree = NULL;
        }
    }

};

#endif //FPGA_CONNECTOR_CONDITIONTREE_H
