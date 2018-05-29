//
// Created by wjt on 18-5-10.
//

#include "ColumnHandle.h"

ColumnHandle::ColumnHandle(string table_description) {
    vector<string> descriptions = split(table_description, ",:");

    for (int i = 0; i < descriptions.size() / 2; i++) {
        if (descriptions[2 * i + 1] == "long") {
            this->column_index[descriptions[2 * i]] = i + 1;
        } else if (descriptions[2 * i + 1] == "string") {
            this->column_index[descriptions[2 * i]] = -(i + 1);
        } else {
            cout << "Description Has Unexpected Type:" << descriptions[2 * i + 1] << endl;
            cout << "Column [" << descriptions[2 * i] << "] couldn't be use for gen_condition() " << endl;
        }
    }
}

map<string, int> ColumnHandle::getColumnIndex() {
    return this->column_index;
}

vector<string> ColumnHandle::split(const string &s, const string &seperator) {
    vector<string> result;
    typedef string::size_type string_size;
    string_size i = 0;
    bool b;

    while (i != s.size()) {
        //找到字符串中首个不等于分隔符的字母；
        int flag = 0;
        b = false;
        while (i != s.size() && flag == 0) {
            flag = 1;
            for (string_size x = 0; x < seperator.size(); ++x) {
                if (s[i] == seperator[x]) {
                    //when the first charactor is seperator,it means that a "" is before the first seperator
                    if (i == 0) {
                        result.push_back("");
                    }
                    //when the last charactor is seperator,it means that a "" is after the last seperator
                    if (i == s.size() - 1) {
                        result.push_back("");
                    }
                    ++i;
                    flag = 0;
                    //when "" between two seperator
                    if (b) {
                        result.push_back("");
                    }
                    b = true;
                    break;

                }
            }
        }

        //找到又一个分隔符，将两个分隔符之间的字符串取出；
        flag = 0;
        string_size j = i;
        while (j != s.size() && flag == 0) {
            for (string_size x = 0; x < seperator.size(); ++x)
                if (s[j] == seperator[x]) {
                    flag = 1;
                    break;
                }
            if (flag == 0)
                ++j;
        }
        if (i != j) {
            result.push_back(s.substr(i, j - i));
            i = j;
        }
    }
    return result;
}