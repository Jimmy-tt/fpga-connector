//
// Created by wjt on 18-5-10.
//

#ifndef FPGA_CONNECTOR_COLUMNHANDLE_H
#define FPGA_CONNECTOR_COLUMNHANDLE_H

#include "common.h"

class ColumnHandle {
public:

    ColumnHandle(string table_description);

    static vector<string> split(const std::string &s, const std::string &seperator);

    map<string, int> getColumnIndex();

private:
    map<string, int> column_index;
};

#endif //FPGA_CONNECTOR_COLUMNHANDLE_H
