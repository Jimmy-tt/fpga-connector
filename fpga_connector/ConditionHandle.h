//
// Created by wjt on 18-5-8.
//

#ifndef FPGA_CONNECTOR_CONDITIONHANDLE_H
#define FPGA_CONNECTOR_CONDITIONHANDLE_H

#include "common.h"

class AtomCondition {
public:
    AtomCondition() {};

    AtomCondition(vector<string> atom_vec);

    string getAtomColumn();

    string getOperation();

    string getValue();

private:
    string column;
    string operation;
    string value;
};


class ConditionHandle {
public:
    static string m_replace(string strSrc, const string &oldStr, const string &newStr, int count = -1);

    vector<string> split(const string &str, const string &pattern);

    string solve_quote(string str);

    vector<string> unique_array(vector<string> input_array);

    void setAtomCondition(vector<string> atom_vec);

    string simplifySelect(string sourceString, vector<string> resultString);

    AtomCondition getAtomCondition(int num);

    vector<AtomCondition> getTotalCondition();

    vector<string> solveAtomCondition(string atom);

private:
    vector<AtomCondition> atom_vec;

    int index[MAX_Cond];
};

#endif //FPGA_CONNECTOR_CONDITIONHANDLE_H
