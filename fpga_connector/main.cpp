#include "common.h"
#include "ConditionTree.h"
#include "ConditionHandle.h"
#include "CreateTruthTable.h"
#include "ColumnHandle.h"
#include "ComplexCondHandle.h"

string descriptions = "logtime:string,svr_ip:string,user_ip:string,port:long,host:string,url:string,req_args:string,strrange:string,http_code:long,send_bytes:long,handle_time:long,refer:string,user_agent:string,stdform:string,uin:long,isnormalclosed:long,url302:string,cdn:long,sample:long,filesize:long,inner_errcode:long,inner_filename:string,bizid:long,flow:long,clientappid:string,reverse_proxy:string,oc_id:string,str_reserve:string,vkey:string,int_reserve:long,province:long,isp:long,log_type:long,get_store_time:long,deliver_time:long,store_type:long,bit_rate:long,media_time:long,media_type:string,req_type:long,inner_errmsg:string,content_type:string,store_ip:string,resolution:long,reserve1:long,reserve2:long,reserve3:long,reserve4:string,reserve5:string";

void set_condtions(vector<AtomCondition> atom_cond_vec, map<string, int> columns_index, unsigned char *truthtable,
                   unsigned char *dram_outptr);

void gen_conditions(string input, string descriptions, unsigned char *dram_outptr);

void set_condtions(vector<AtomCondition> atom_cond_vec, map<string, int> columns_index, unsigned char *truthtable,
                   unsigned char *dram_outptr) {
    unsigned char field[MAX_Cond];
    unsigned char op[MAX_Cond];
    long longtype[MAX_Cond];
    unsigned char stringtype[MAX_Cond][MAX_Cmp_Bytes];

    bzero(field, MAX_Cond);
    bzero(op, MAX_Cond);
    bzero(longtype, MAX_Cond * 8);
    bzero(stringtype, MAX_Cond * MAX_Cmp_Bytes);

    for (int i = 0; i < atom_cond_vec.size(); i++) {
        int column_num = columns_index[atom_cond_vec[i].getAtomColumn()];
        if (column_num < 0) {
            cout << "string column" << endl;
            field[i] = (unsigned char) (-column_num - 1);
            if (atom_cond_vec[i].getOperation() == "=") {
                op[i] = 1;
            } else if (atom_cond_vec[i].getOperation() == "<>") {
                op[i] = 3;
            } else {
                cout << "Uncorrect operator: " << atom_cond_vec[i].getOperation() << endl;
            }
            memcpy(stringtype[i], atom_cond_vec[i].getValue().c_str(), atom_cond_vec[i].getValue().length());
        } else if (column_num > 0) {
            cout << "long column" << endl;
            field[i] = (unsigned char) (column_num - 1);

            if (atom_cond_vec[i].getOperation() == "<>") {
                op[i] = 2;
            } else if (atom_cond_vec[i].getOperation() == ">=") {
                op[i] = 4;
            } else if (atom_cond_vec[i].getOperation() == "<=") {
                op[i] = 5;
            } else if (atom_cond_vec[i].getOperation() == ">") {
                op[i] = 6;
            } else if (atom_cond_vec[i].getOperation() == "<") {
                op[i] = 7;
            } else if (atom_cond_vec[i].getOperation() == "=") {
                op[i] = 0;
            } else {
                cout << "Uncorrect operator: " << atom_cond_vec[i].getOperation() << endl;
            }
            longtype[i] = atol(atom_cond_vec[i].getValue().c_str());
        } else {
            cout << "Uncorrect column name" << atom_cond_vec[i].getAtomColumn() << endl;
        }
    }

    for (int i = (int) atom_cond_vec.size(); i < MAX_Cond; i++) {
        field[i] = MAX_Columns;
    }

    memcpy(dram_outptr, field, MAX_Cond);   //0
    memcpy(&dram_outptr[8], op, MAX_Cond);  //0
    for (int i = 0; i < MAX_Cond; i++)
        memcpy(&dram_outptr[64 + 8 * i], &longtype[i], 8); //1
    for (int i = 0; i < MAX_Cond; i++)
        memcpy(&dram_outptr[128 + 64 * i], stringtype[i], MAX_Cmp_Bytes); //2-9

    memcpy(&dram_outptr[640], truthtable, sizeof(unsigned) * 8);   //10
}

void gen_conditions(string input, string descriptions, unsigned char *dram_outptr) {

    string and_description = " AND ";
    string and_replace_symbol = "&";
    string or_description = " OR ";
    string or_replace_symbol = "|";

    ConditionHandle *cHandle = new ConditionHandle();
    ColumnHandle *columnHandle = new ColumnHandle(descriptions);


    input = cHandle->m_replace(input, and_description, and_replace_symbol);
    input = cHandle->m_replace(input, or_description, or_replace_symbol);

    cout << "input before handle IN:" + input << endl;

    input = ComplexCondHandle::handleIN(input);

    cout << "input after handle IN:" + input << endl;

    vector<string> divided_string;
    divided_string = cHandle->split(input, and_replace_symbol);

    vector<string> result_string_array;

    for (int n = 0; n < divided_string.size(); n++) {
        vector<string> temp_string;
        temp_string = cHandle->split(divided_string[n], or_replace_symbol);
        for (int nn = 0; nn < temp_string.size(); nn++) {
            result_string_array.push_back(cHandle->solve_quote(temp_string[nn]));
        }
    }

    vector<string> uniq_array = cHandle->unique_array(result_string_array);
    vector<string> atom_vec;

    for (int n = 0; n < uniq_array.size(); n++) {
        atom_vec = cHandle->solveAtomCondition(uniq_array[n]);
        cHandle->setAtomCondition(atom_vec);
    }

    string simplifyInput = cHandle->simplifySelect(input, uniq_array);

    char *cSimpInput = (char *) malloc(sizeof(char) * simplifyInput.length());
    strcpy(cSimpInput, simplifyInput.c_str());

    BtreeNode *conditionTree = new BtreeNode;
    ConditionTree *cTree = new ConditionTree();
    conditionTree = cTree->afaToBtree(cSimpInput, 0, (int) simplifyInput.length());

    CreateTruthTable *truthTable = new CreateTruthTable();
    unsigned char true_table[TABLE_LEN] = {0};
    truthTable->create_true_table((int) uniq_array.size(), conditionTree, true_table);

    bzero(dram_outptr, strlen((char *) dram_outptr));
    set_condtions(cHandle->getTotalCondition(), columnHandle->getColumnIndex(), true_table, dram_outptr);

    cTree->freeTree(conditionTree);

}


int main() {
    //    string input = "((((\"id\" > 1) OR (\"id\" < 5)) AND ((\"id\" = 5) OR (\"id\" = 2))) AND (((\"age\" = 1) OR (\"id\" < 5)) AND ((\"age\" != a) OR (\"id\" = 2))))";
//    string input = "(\"age\" != a) AND (\"id\" = 5)";
//    string input = "((\"age\" != a) AND (\"id\" > 5)) AND (\"lastname\" = 9)";
//    string input = "(\"age\" = a) OR (\"id\" > 5) OR (\"id\" < 9) OR (\"id\" < 8) OR (\"id\" < 7) OR (\"id\" < 4) OR (\"id\" < 3)";
//    string input = "(\"0\" = CAST('2017-06-06 12:21:27' AS varchar))";
//    string input = "(\"age\" != 2) AND (\"id\" > 4000)";
//    string input = conditions[0];
    string input = "(\"port\" IN (31600,31700)) AND (\"host\" IN ('192.168.1.1','192.168.1.2'))";
//    string input = "(\"age\" != 2) AND (\"id\" > 4000) AND (\"lastname\" != 9)";
//    string input = "(\"svr_ip\" = '192.168.1.1') OR (\"http_code\" > 500) AND (BIGINT '31600' = \"port\")";
//    string input = "(\"id\" = 5) AND (BIGINT '31600' = \"port_3\") AND (\"age\" != a)";
//    string input = "(\"port\" IN (31600,31700))";
//    string input = "(\"svr_ip\" IN ('192.168.1.1','192.168.1.2')) OR (\"port\" IN (31600,31700)) OR (\"http_code\" > 500) OR (\"user_ip\" = '192.168.6.4')";
//    string descriptions = "logtime:string,svr_ip:string,user_ip:string,port:long,host:string,url:string,req_args:string,strrange:string,http_code:long,send_bytes:long,handle_time:long,refer:string,user_agent:string,stdform:string,uin:long,isnormalclosed:long,url302:string,cdn:long,sample:long,filesize:long,inner_errcode:long,inner_filename:string,bizid:long,flow:long,clientappid:string,reverse_proxy:string,oc_id:string,str_reserve:string,vkey:string,int_reserve:long,province:long,isp:long,log_type:long,get_store_time:long,deliver_time:long,store_type:long,bit_rate:long,media_time:long,media_type:string,req_type:long,inner_errmsg:string,content_type:string,store_ip:string,resolution:long,reserve1:long,reserve2:long,reserve3:long,reserve4:string,reserve5:string";
    unsigned char *dram_outptr = NULL;
    unsigned max_dram_malloc = 800;
    dram_outptr = (unsigned char *) malloc(max_dram_malloc);

    gen_conditions(input, descriptions, dram_outptr);

    cout << "fieId:" << endl;
    for (int i = 0; i < 8; i++) {
        printf("%x ", dram_outptr[i]);
    }

    cout << endl;

    cout << "op:" << endl;
    for (int i = 8; i < 16; i++) {
        printf("%x ", dram_outptr[i]);
    }

    cout << endl;

    cout << "longType:" << endl;
    for (int i = 64; i < 128; i++) {
        printf("%x ", dram_outptr[i]);
    }

    cout << endl;

    cout << "stringType:" << endl;
    for (int i = 128; i < 640; i++) {
        cout << dram_outptr[i] << " ";
    }

    cout << endl;

    cout << "truthTable:" << endl;
    for (int i = 640; i < 672; i++) {
        printf("%x ", dram_outptr[i]);
    }

    cout << endl;

    string name = "outdata";

    FILE *fp = fopen(name.c_str(), "wb");
    fwrite(dram_outptr, sizeof(char), 800, fp);
    fclose(fp);

    return 0;
}