//
// Created by wjt on 18-5-14.
//

#include "ComplexCondHandle.h"
#include "ColumnHandle.h"
#include "ConditionHandle.h"

string solve_quote_complete(string str);

string solve_quote_complete(string str) {
    if (str[0] == '(') {
        str = str.substr(1, str.length() - 1);
        str = solve_quote_complete(str);
    }
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

string ComplexCondHandle::handleIN(string condition) {
    while (condition.find(" IN ") != string::npos) {
        unsigned long index = condition.find(" IN ");
        unsigned long left_and_index = 0;
        unsigned long left_or_index = 0;
        unsigned long right_and_index = condition.length();
        unsigned long right_or_index = condition.length();
        unsigned long left_index;
        unsigned long right_index;
        string not_in_str;
        //find the start '(' of condition with 'IN'
        if (condition.rfind("&", index) != string::npos) {
            left_and_index = condition.rfind("&", index) + 1;
        }
        if (condition.rfind("|", index) != string::npos) {
            left_or_index = condition.rfind("|", index) + 1;
        }
        left_index = (left_and_index > left_or_index) ? left_and_index : left_or_index;

        //find the end ')' of condition with 'IN'
        if (condition.find("&", index) != string::npos) {
            right_and_index = condition.find("&", index) - 1;
        }
        if (condition.find("|", index) != string::npos) {
            right_or_index = condition.find("|", index) - 1;
        }
        right_index = (right_and_index < right_or_index) ? right_and_index : right_or_index;

        string inCondition = condition.substr(left_index, right_index - left_index + 1);
        inCondition = solve_quote_complete(inCondition);

        vector<string> column_vec = ColumnHandle::split(inCondition, "\"");
        string column = column_vec[1];

        vector<string> values_vec = ColumnHandle::split(inCondition, "()");
        string values = values_vec[1];

        vector<string> value_vec = ColumnHandle::split(values, ",");

        not_in_str += "(";
        for (int i = 0; i < value_vec.size() - 1; i++) {
            not_in_str += ("\"" + column + "\" = " + value_vec[i] + "|");
        }
        not_in_str += ("\"" + column + "\" = " + value_vec[value_vec.size() - 1] + ")");

        condition = ConditionHandle::m_replace(condition, inCondition, not_in_str);
    }

    return condition;
}