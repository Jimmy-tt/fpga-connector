#include <iostream>
#include "common.h"
#include "LongProducer.h"
#include "FileReader.h"
#include "ConditionHandle.h"
#include "DataMerge.h"
#include "ConditionTree.h"
#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <errno.h>
#include "FPGA.h"

//#include "xcompare_top.h"

#define TARGET_NUM 2 // number of target value

#define OFFSET_IN_FPGA_DRAM 0x10000000

using namespace std;

int main() {
//    FileReader *sf = new FileReader();
//    char *inptr = (char *) malloc(10000);
//    ifstream fin("/home/wjt/Desktop/short", ios::in);
//    fin.getline(inptr, 10000);
//    vector<string> columns = sf->split(inptr, "\t");
//    for (int sfs = 0; sfs < columns.size(); sfs++) {
//        cout << "num " << sfs << ": " << columns[sfs] << endl;
//    }

    struct timeval start;
    struct timeval end;
    struct timeval cpu_compare_start;
    struct timeval cpu_compare_end;
    unsigned long timer;
    unsigned long timer_cpu = 0;
    unsigned long timer_back = 0;

    struct timeval conditionHandle_time;
    struct timeval fileReader_time;
    struct timeval pre_time;
    struct timeval back_time;


    in_port_t server_port = 8887;
    //建立
    int server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_sock < 0) {
        cout << "socket() failed" << endl;
    }
    //配置服务器socket
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(server_port);

    int reuse = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof(int)) == -1) {
        exit(1);
    }

    //绑定端口
    if (bind(server_sock, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        cout << "bind() failed!" << endl;
    }
    //监听端口
    if (listen(server_sock, 1) < 0) {
        cout << "listen() failed!" << endl;
    }
    printf("server start success on %d \n", server_port);
    for (;;) {
        //启动成功，等待客户端链接，
        //client_addr保存客户端地址信息
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        //获取客户端socket
        int client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_sock < 0) {
            cout << "accept() failed!" << endl;
        }
        //获取客户端IP和端口
        char clientName[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, clientName, sizeof(clientName)) != NULL) {
            printf("Handing Client %s /%d \n", clientName, ntohs(client_addr.sin_port));
        } else {
            puts("Unable to get client address!");
        }

        char buff[100];
        ssize_t n = recv(client_sock, buff, 100, 0);
        buff[n] = '\0';

        cout << "buff: " << buff << endl;

        gettimeofday(&start, NULL);

        ConditionHandle *cHandle = new ConditionHandle();
        vector<string> conditions = cHandle->split(buff, "@");


//---------------------------Condition Handle----------------------------------------------

        long fileId = 0;
        //   string input = "((((\"id\" > 1) OR (\"id\" < 5)) AND ((\"id\" = 5) OR (\"id\" = 2))) AND (((\"age\" = 1) OR (\"id\" < 5)) AND ((\"age\" != a) OR (\"id\" = 2))))";
//    string input = "(\"age\" != a) AND (\"id\" = 5)";
//    string input = "(\"age\" != a) AND (\"id\" > 5) AND (\"lastname\" = 9)";
//        string input = "(\"0\" = CAST('2017-06-06 12:21:27' AS varchar))";
//    string input = "(\"age\" != 2) AND (\"id\" > 4000)";
        string input = conditions[0];
//    string input = "(\"id\" > 4000)";
//    string input = "(\"age\" != 2) AND (\"id\" > 4000) AND (\"lastname\" != 9)";
//    string input = "(\"age\" = a) OR (\"id\" > 5) AND (\"id\" < 9)";
//    string input = "((\"id\" = 5) AND (\"lastname\" != CAST('1234' AS varchar))) AND (BIGINT '31600' = \"port_3\")";
//    string input = "(\"id\" = 5) AND (BIGINT '31600' = \"port_3\") AND (\"age\" != a)";
//    string input = "\"id\" = 5";
        string and_description = " AND ";
        string and_replace_symbol = "&";
        string or_description = " OR ";
        string or_replace_symbol = "|";

        input = cHandle->m_replace(input, and_description, and_replace_symbol);
        input = cHandle->m_replace(input, or_description, or_replace_symbol);
        cout << input << endl;
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

        cHandle->unique_array(result_string_array);
        vector<string> atom_vec;

        for (int n = 0; n < result_string_array.size(); n++) {
//        atom_vec = cHandle->split(atom, " ");
            atom_vec = cHandle->solveAtomCondition(result_string_array[n]);
            cHandle->setAtomCondition(atom_vec);
        }

//    string simplifyInput = cHandle->simplifySelect(input);
        string simplifyInput = cHandle->simplifySelect(input, result_string_array);
        cout << simplifyInput << endl;

        char *cSimpInput = (char *) malloc(sizeof(char) * simplifyInput.length());
        strcpy(cSimpInput, simplifyInput.c_str());

        BtreeNode *conditionTree = new BtreeNode;
        ConditionTree *cTree = new ConditionTree();
        conditionTree = cTree->afaToBtree(cSimpInput, 0, (int) simplifyInput.length());

        gettimeofday(&conditionHandle_time, NULL);

        timer = 1000000 * (conditionHandle_time.tv_sec - start.tv_sec) + conditionHandle_time.tv_usec - start.tv_usec;
        printf("condition handle timer = %ld us\n", timer);
//-------------------------------File Handle-------------------------------------------

        AtomCondition *atom = new AtomCondition();
        DataMerge *dm = new DataMerge();


//        vector<int> v_a;
//        vector<int> v_b;
//        vector<int> v_c;
//        vector<int> v_d;
//        vector<int> v_e;
//        vector<int> v_f;
//
//        for (int i = 0; i < 5; i++) {
//            v_a.push_back(i);
//            v_b.push_back(i + 1);
//            v_c.push_back(i + 2);
//            v_d.push_back(i + 3);
//            v_e.push_back(i + 4);
//            v_f.push_back(i + 5);
//        }
//
//        dm->setFilterLine(0, 0, v_a);
//        dm->setFilterLine(0, 1, v_b);
//        dm->setFilterLine(0, 2, v_c);
//        dm->setFilterLine(0, 3, v_d);
//        dm->setFilterLine(0, 4, v_e);
//        dm->setFilterLine(0, 5, v_f);

        char *config_in = (char *) malloc(1000);
        string table_config = "config/table_config";
        string location;
        string description;
        ifstream in(table_config, ios::in);
        while (in.getline(config_in, 1000)) {
            vector<string> table_name = cHandle->split(config_in, "\t");
            if (table_name[0] == conditions[1]) {
                location = table_name[1];
                description = table_name[2];
                break;
            }
        }

//        char *config_in = (char *) malloc(1000);
//        string table_config = "/home/wjt/Downloads/fpga_connector-master_debug_complete/config/table_config";
//        string location = conditions[2];
//        string description;
//        ifstream in(table_config, ios::in);
//        while (in.getline(config_in, 1000)) {
//            vector<string> table_name = cHandle->split(config_in, "\t");
//            if (table_name[0] == conditions[1]) {
//                description = table_name[1];
//                break;
//            }
//        }


        unsigned long output_size = 0;
        int columnByte;
        // read the file
//    FileReader *source_file = new FileReader("/home/wjt/Desktop/data",
//                                             "id:long,lastname:string,age:string,3:string,4:string,5:string,port_3:long,7:string,8:long,9:string,10:long,11:long,12:string,13:string,14:long,15:string,16:string,17:long,18:string,19:long");
//    FileReader *source_file = new FileReader("/home/wjt/Desktop/data500000",
//                                             "id:long,lastname:string,age:string,3:string,4:string,5:string,port_3:long,7:string,8:long,9:string");
//    FileReader *source_file = new FileReader("/home/wjt/Desktop/data_aws",
//                                             "id:long,1:string,2:string");
        FileReader *source_file = new FileReader(location,
                                                 description);

        gettimeofday(&fileReader_time, NULL);

        timer = 1000000 * (fileReader_time.tv_sec - conditionHandle_time.tv_sec) + fileReader_time.tv_usec -
                conditionHandle_time.tv_usec;
        printf("fileReader timer = %ld us\n", timer);

        for (int conditionNum = 0; conditionNum < result_string_array.size(); conditionNum++) {

            struct timeval start_d;
            struct timeval end_d;

            unsigned long tt = 0;
            gettimeofday(&start_d, NULL);
            *atom = cHandle->getAtomCondition(conditionNum);
            int type = source_file->getColumnType(atom->getAtomColumn()); //long:= >0;string:= <0
            cout << "atom :" << atom->getAtomColumn() << endl;
            byte operation;
            if (type < 0) {
                columnByte = STRING_BYTE;

                if (atom->getOperation() == "=") {
                    operation = (byte) 1;
                } else if (atom->getOperation() == "<>") {
                    operation = (byte) 2;
                } else {
                    cout << "error operation:" << atom->getOperation() << endl;
                    exit(0);
                }

            } else {
                columnByte = LONG_BYTE;

                if (atom->getOperation() == ">") {
                    operation = (byte) 4;
                } else if (atom->getOperation() == "=") {
                    operation = (byte) 8;
                } else if (atom->getOperation() == "<") {
                    operation = (byte) 16;
                } else if (atom->getOperation() == ">=") {
                    operation = (byte) 32;
                } else if (atom->getOperation() == "<=") {
                    operation = (byte) 64;
                } else if (atom->getOperation() == "<>") {
                    operation = (byte) 128;
                } else {
                    cout << "error operation:" << atom->getOperation() << endl;
                    exit(0);
                }
            }

            source_file->setFlag(true);

//----------------------------------ceate a block , write , read-----------------------------------------------------
            while (source_file->getFlag()) {
                byte *output_array;

                output_size = source_file->getColumnSize(MAX_INPTR);
//            output_array = (byte *) malloc(
//                    sizeof(byte) *
//                    (FILEID_BYTE + CONDITIONID_BYTE + BEGININDEX_BYTE + OPERATION_BYTE + LINE_BYTE + columnByte +
//                     columnByte * source_file->getColumnSize(
//                             MAX_INPTR)));  //fileId + coditionId + offset + operation + line all + target + data
                output_array = (byte *) malloc(
                        sizeof(byte) *
                        (FILEID_BYTE + CONDITIONID_BYTE + BEGININDEX_BYTE + OPERATION_BYTE + LINE_BYTE + columnByte +
                         (columnByte + LINE_BYTE) * source_file->getColumnSize(
                                 MAX_INPTR)));

                unsigned long lineOffset = FILEID_BYTE + CONDITIONID_BYTE + BEGININDEX_BYTE + OPERATION_BYTE;

                /*-----------------------------------------------block head-----------------------------------------------------*/
                output_array[0] = (byte) fileId;

                output_array[FILEID_BYTE] = (byte) conditionNum;

                source_file->getBeginIndex(output_array);

                output_array[FILEID_BYTE + CONDITIONID_BYTE + BEGININDEX_BYTE] = operation;

                source_file->addLongAsBytes(output_array, output_size - 1, lineOffset, LINE_BYTE);

                if (type > 0) {
                    unsigned long offset =
                            FILEID_BYTE + CONDITIONID_BYTE + BEGININDEX_BYTE + OPERATION_BYTE + LINE_BYTE;
                    source_file->addLongAsBytes(output_array, atol((atom->getValue()).c_str()), offset, LONG_BYTE);
                } else {
                    unsigned long offset =
                            FILEID_BYTE + CONDITIONID_BYTE + BEGININDEX_BYTE + OPERATION_BYTE + LINE_BYTE;
                    byte byte_string[atom->getValue().length()];
                    atom->getValue().copy((char *) byte_string, atom->getValue().length());
                    source_file->dataFormate(byte_string, output_array,
                                             atom->getValue().length(), STRING_BYTE, offset);
                }
                /*-------------------------------------------block head end---------------------------------------------------------*/

                source_file->getColumnData(atom->getAtomColumn(), output_array, output_size);
//            string aaa = "0";
//            for (int i = 0; i < ttttt; i++) {
//                aaa += "0";
//            }
                string name = "/home/wjt/Desktop/outdata";

//                FILE *fp = fopen(name.c_str(), "wb");
//                fwrite(output_array, sizeof(byte),
//                       output_size * (columnByte + LINE_BYTE) + FILEID_BYTE + CONDITIONID_BYTE + BEGININDEX_BYTE +
//                       OPERATION_BYTE +
//                       LINE_BYTE + columnByte, fp);
//                fclose(fp);


                int rc, rcm;
                int slot_id;
                byte *back_data;
                back_data = (byte *) malloc(sizeof(byte) *
                                            (output_size * (columnByte + LINE_BYTE) + FILEID_BYTE + CONDITIONID_BYTE +
                                             BEGININDEX_BYTE +
                                             OPERATION_BYTE +
                                             LINE_BYTE + columnByte));
                unsigned long sourcebuflen =
                        output_size * (columnByte + LINE_BYTE) + FILEID_BYTE + CONDITIONID_BYTE + BEGININDEX_BYTE +
                        OPERATION_BYTE + LINE_BYTE + columnByte;
                //           int returnLen = 0;
                //           rc = fpga_pci_init();
                slot_id = 0;
                //           pci_bar_handle_t pci_bar_handle = PCI_BAR_HANDLE_INIT;

                //           rcm = fpga_pci_attach(slot_id, FPGA_APP_PF, APP_PF_BAR1, 0, &pci_bar_handle);

                printf("\n");
                printf("===== AXI CDMA compare =====\n");
                printf("\n");


                FPGA *fpga = new FPGA();

                gettimeofday(&cpu_compare_start, NULL);
//memcpy(back_data,output_array,strlen((char*)output_array));
//                memcpy(output_array,back_data,strlen((char*)back_data));
                int num = fpga->compare_top((char *) output_array, (char *) back_data);

                gettimeofday(&cpu_compare_end, NULL);

                timer_cpu += 1000000 * (cpu_compare_end.tv_sec - cpu_compare_start.tv_sec) + cpu_compare_end.tv_usec -
                             cpu_compare_start.tv_usec;
                //           returnLen = XCompare_top_Get_return(pci_bar_handle, &rcm);
                //          printf("size=%d\n", returnLen);
                //          back_data = (byte *) malloc(returnLen);
                //           if (back_data == NULL) {
                //               printf("malloc Outbufsource failed\n");
                //               return 1;
                //          }
                //           bzero(back_data, returnLen);
                //          rc = peek_poke_compare(slot_id, FPGA_APP_PF, APP_PF_BAR1, output_array, back_data, sourcebuflen,
                //                                  returnLen);
                //           if (returnLen == 0) {
//                printf("no return length, compare failed\n");
//                return 1;
                //           }


                cout << "CDMA done successfully!!" << endl;


//            if ((fd = open("/dev/edma0_queue_0", O_RDWR)) == -1) {
//                perror("open failed with errno");
//            }
//            ret = pwrite(fd, output_array, output_size, OFFSET_IN_FPGA_DRAM);
//            if (ret < 0) {
//                perror("write failed with errno");
//            }
//
//            printf("Tried to write %u bytes, succeeded in writing %u bytes\n", output_size, ret);
//
//            /* Ensure the fsync was successful */
//            ret = fsync(fd);
//            if (ret < 0) {
//                perror("fsync failed with errno");
//            }
//
//            begin_index = (byte *) malloc(sizeof(byte) * BEGININDEX_BYTE);
//            ret = pread(fd, begin_index, BEGININDEX_BYTE * sizeof(byte), OFFSET_IN_FPGA_DRAM);
//
//            line_all = (byte *) malloc(sizeof(byte) * LINE_BYTE);
//            ret = pread(fd, line_all, LINE_BYTE * sizeof(byte), OFFSET_IN_FPGA_DRAM);
////            long filterline = dm->addBitsAsLong(line_all, LINE_BYTE);

//            cout << "line:" << filterline << endl;
//
//            back_data = (byte *) malloc(filterline * sizeof(byte));
//            ret = pread(fd, back_data, filterline * sizeof(byte), OFFSET_IN_FPGA_DRAM);//need to be adjust----------
//
//
//
//            if (ret < 0) {
//                perror("read failed with errno");
//            }
//
//            printf("Tried reading %u byte, succeeded in read %u bytes\n", output_size, ret);
//
//            if (close(fd) < 0) {
//                perror("close failed with errno");
//            }
//
//            printf("Data read is %s\n", back_data);

//        for (int j = 0; j < output_size * 64; j++) {
//            if (j % 64 == 0) {
//                cout << endl;
//            }
////        printf("%x", output_array[j]);
//            cout << output_array[j];
                //     }
//                long filterline;

//                back_data[0] = (byte) 0;
//                back_data[1] = (byte) 0;
//                back_data[2] = (byte) 0;
//                back_data[3] = (byte) 2;
//                back_data[4] = (byte) 0;
//                back_data[5] = (byte) 4;
//                back_data[6] = (byte) 0;
//                back_data[7] = (byte) 1;
//                back_data[8] = (byte) 0;
//                back_data[9] = (byte) 2;
//                back_data[10] = (byte) 0;
//                back_data[11] = (byte) 3;
//                back_data[12] = (byte) 0;
//                back_data[13] = (byte) 4;
//                back_data[14] = (byte) 0;
//                back_data[15] = (byte) 5;
//                unsigned long ooff = 4;
//                source_file->addLongAsBytes(back_data, 5, ooff, LINE_BYTE);
//                unsigned long offset = 0;
//                unsigned long tt = 23;
//                unsigned long boff = 0;
//                filterline = 2;
//                back_data = (byte *) malloc(filterline * sizeof(byte));
//                back_data[0] = 0b00000001;
//                back_data[1] = 0b00000011;
                gettimeofday(&conditionHandle_time, NULL);

                if (num <= 6) {
                    cout << "no line return" << endl;
                    vector<int> line;
                    line.clear();
                    dm->setFilterLine(fileId, conditionNum, line);
                } else {
                    dm->transReturnData(back_data);
                }
                gettimeofday(&back_time, NULL);

                timer_back += 1000000 * (back_time.tv_sec - conditionHandle_time.tv_sec) + back_time.tv_usec -
                              conditionHandle_time.tv_usec;

//            vector<int> temp_vec = dm->getFilterLine(0);
//
//            for (int k = 0; k < temp_vec.size(); k++) {
//                cout << "the line is " << temp_vec[k] << endl;
//            }


                if (output_array)
                    free(output_array);
                output_array = NULL;
            }

            gettimeofday(&end_d, NULL);
            tt = 1000000 * (end_d.tv_sec - start_d.tv_sec) + end_d.tv_usec - start_d.tv_usec;

            printf("Num%d: timer cpu duel compare = %ld us\n", conditionNum + 1, tt);
        }

        cTree->lineMerge(conditionTree, dm);
        vector<int> p_vec = conditionTree->lineNum;

//        dm->outFinalData(p_vec, source_file, "111", client_sock);
//        cout << p_vec.size() << endl;
//        for (int i = 0; i < p_vec.size(); i++)
//            cout << p_vec[i] << endl;



        gettimeofday(&end, NULL);
        timer = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;

        printf("timer cpu compare = %ld us\n", timer_cpu);
        printf("back timer = %ld us\n", timer_back);
        printf("timer = %ld us\n", timer);
//        source_file->getFinalData(p_vec);

        timer_cpu = 0;
        timer_back = 0;
        const char *data = "/usr/local/tmp/tmp";
        send(client_sock, data, strlen(data), 0);

        close(client_sock);
//    int test_column = source_file->getColumnNum("first");

//    int buffersize = source_file->getColumnSize(test_column);

//    byte buffer[buffersize];

//    int fd = 0;// pretend it to be a file description

//    write(fd, buffer, buffersize); // transform data to FPGA or target file

//    long return_size = 0;
//    byte return_header[8];
//    //read 8 bytes at first , in order to get the size of returning buffer
//    read(fd, return_header, 9);
//
//    return_size = LongProducer::transFromByte(return_header);
//
//    byte result_buffer[return_size];
//
//    read(fd, result_buffer, return_size);

    }

    close(server_sock);
    return 0;

}
