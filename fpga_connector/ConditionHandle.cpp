//
// Created by wjt on 18-5-8.
//

#include "ConditionHandle.h"

void stringReplace(string &strBase, string strSrc, string strDes);

string ConditionHandle::m_replace(string strSrc, const string &oldStr, const string &newStr, int count) {
    string strRet = strSrc;
    size_t pos = 0;
    int l_count = 0;
    if (-1 == count) // replace all
        count = strRet.size();
    while ((pos = strRet.find(oldStr, pos)) != string::npos) {
        strRet.replace(pos, oldStr.size(), newStr);
        if (++l_count >= count) break;
        pos += newStr.size();
    }
    return strRet;
}

vector<string> ConditionHandle::split(const string &str, const string &pattern) {
    vector<string> resVec;

    if ("" == str) {
        return resVec;
    }
    //方便截取最后一段数据
    string strs = str + pattern;

    size_t pos = strs.find(pattern);
    size_t size = strs.size();

    while (pos != string::npos) {
        string x = strs.substr(0, pos);
        resVec.push_back(x);
        strs = strs.substr(pos + 1, size);
        pos = strs.find(pattern);
    }

    return resVec;
}

string ConditionHandle::solve_quote(string str) {
    int length = str.size();
    int left_quote_counter = 0;
    int right_quote_counter = 0;
    for (int i = 0; i < length; i++) {
        if (str[i] == '(') left_quote_counter++;
        if (str[i] == ')') right_quote_counter++;
    }
    if (left_quote_counter > right_quote_counter) {
        str = str.substr(left_quote_counter - right_quote_counter, str.size());
    } else {
        str = str.substr(0, str.size() + left_quote_counter - right_quote_counter);
    }
    return str;
}

vector<string> ConditionHandle::unique_array(vector<string> input_array) {
    int index = 0;
    unsigned long size = input_array.size();
    vector<string> uniq_array;
    while (index < size) {
        if (input_array[index] != "") {
            string temp_string = input_array[index];
            uniq_array.push_back(temp_string);
            for (int i = index; i < size; i++) {
                if (temp_string == input_array[i]) {
                    input_array[i] = "";
                }
            }
        }
        index++;
    }
    return uniq_array;


//    sort(input_array.begin(), input_array.end());
//    input_array.erase(unique(input_array.begin(), input_array.end()), input_array.end());
}

void ConditionHandle::setAtomCondition(vector<string> atom_vec) {
    AtomCondition *atom = new AtomCondition(atom_vec);
    this->atom_vec.push_back(*atom);
}

string ConditionHandle::simplifySelect(string sourceString, vector<string> resultString) {
//    string outString = sourceString;
//    for (int num = 0; num < this->atom_vec.size(); num++) {
//        string tempString;
//        string str_num = to_string(num);
//        AtomCondition temp_vec = this->getAtomCondition(num);
//        tempString =
//                "(\"" + temp_vec.getAtomColumn() + "\"" + " " + temp_vec.getOperation() + " " + temp_vec.getValue() +
//                ")";
//        cout << tempString << endl;
//        stringReplace(outString, tempString, str_num);
//    }
//    return outString;
    string outString = sourceString;
    for (int num = 0; num < resultString.size(); num++) {
        stringstream ss;
        string str_num;
        ss << num;
        ss >> str_num;
        stringReplace(outString, resultString[num], str_num);
    }
    return outString;
}

vector<string> ConditionHandle::solveAtomCondition(string atom) {
    vector<string> atom_vec;
    vector<string> temp_vec;

    temp_vec = this->split(atom, "\"");
    atom_vec.push_back(temp_vec[1]);

    if (atom.find("<>") < string::npos) {
        atom_vec.push_back("<>");
    } else if (atom.find(">=") < string::npos) {
        atom_vec.push_back(">=");
    } else if (atom.find("<=") < string::npos) {
        atom_vec.push_back("<=");
    } else if (atom.find("=") < string::npos) {
        atom_vec.push_back("=");
    } else if (atom.find(">") < string::npos) {
        atom_vec.push_back(">");
    } else if (atom.find("<") < string::npos) {
        atom_vec.push_back("<");
    }

    temp_vec = this->split(atom, "'");
    if (temp_vec.size() > 1) {
        atom_vec.push_back(temp_vec[1]);
    } else {
        temp_vec = this->split(atom, " ");
        atom_vec.push_back(temp_vec[2]);
    }
    return atom_vec;
}

AtomCondition ConditionHandle::getAtomCondition(int num) {
    AtomCondition atom = this->atom_vec[num];
    return atom;
}

vector<AtomCondition> ConditionHandle::getTotalCondition() {
    return this->atom_vec;
}

AtomCondition::AtomCondition(vector<string> atom_vec) {
    ConditionHandle *cHandle = new ConditionHandle();
    this->column = cHandle->solve_quote(atom_vec[0]);
    this->operation = cHandle->solve_quote(atom_vec[1]);
    this->value = cHandle->solve_quote(atom_vec[2]);
}

string AtomCondition::getAtomColumn() {
    return this->column;
}

string AtomCondition::getOperation() {
    return this->operation;
}

string AtomCondition::getValue() {
    return this->value;
}

void stringReplace(string &strBase, string strSrc, string strDes) {
    string::size_type pos = 0;
    string::size_type srcLen = strSrc.size();
    string::size_type desLen = strDes.size();
    pos = strBase.find(strSrc, pos);
    while ((pos != string::npos)) {
        strBase.replace(pos, srcLen, strDes);
        pos = strBase.find(strSrc, (pos + desLen));
    }
}