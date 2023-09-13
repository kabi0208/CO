#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstring>
using namespace std;

struct cachee{
    int vaild = 0;
    int tag;
    int hit;
    int dirty = 0;
    int change;
    int data;
};

int main(int argc, char *argv[]) {

    // initial
    int offset = 2, block_size = atoi(argv[2]), index = 0, cache_size = atoi(argv[1]), tag,i, j, tmp, associativity;
    string replace_policy = argv[4], file_name = argv[5];
    
    // count offset, index, tag
    block_size /= 4;
    while(block_size > 1){
        offset++;
        block_size /= 2;
    }
    block_size = atoi(argv[2]);
    cache_size *= 1024;
    cache_size /= block_size;
    while(cache_size > 1){
        index++;
        cache_size /= 2;
    }
    cache_size = atoi(argv[1]);
    tag  = 32 - offset - index;
    tmp = pow(2, index) + 2;
    int entries = pow(2, index);
    struct cachee table[tmp]; // index編號
    if(48<= int(argv[3][0]) && int(argv[3][0]) <= 57)
        associativity = int(argv[3][0]) - '0';
    else
        associativity = entries;

    // open file
    string buffer;
    ifstream input_file(file_name);
    if (!input_file.is_open()) {
        cerr << "Could not open the file - '"  << file_name << "'" << endl;
        return EXIT_FAILURE;
    }

    //initial
    int df = 0, ch = 0, cm = 0, bfm = 0, btm = 0, rd = 0, wd = 0, mtr = 0, k = 0;
    int tag_num = 0, index_num = 0, offset_w = 0, offset_b = 0;
    int max, min, set;
    vector <char> input(16);
    vector <int> address(32);
    
    // start to read
    while (getline(input_file, buffer)) {
        df++, tag_num = 0, index_num = 0, offset_w = 0, offset_b = 0;
        input.clear(), i = 0;
        while(buffer[i] != '\0'){
            input.push_back(buffer[i]);
            i++;
        }
        if(input.size() < 2){
            df--;
            continue;
        }

        j = 0;
        for(i = 2; i < input.size(); i++){
            if(input[i] != 'f')
                j = 1;
        }
        if(j == 0){
            df--;
            continue;
        }    

        while(input.size() < 10){ // push 0 (len：8)
            input.insert(input.begin() + 2, '0');
        }
        
        address.clear();
        for(i = 2; i < 10; i++){ // address 16進位 => 2進位
            if(48<= int(input[i]) && int(input[i]) <= 57)
                tmp = int(input[i]) - '0';
            else
                tmp = int(input[i]) - 'a' + 10;
            for(j = 0; j < 4; j++){
                address.insert(address.begin() + (i - 2) * 4, tmp % 2);
                tmp /= 2;
            }
        }
        if(tag + associativity > 32 - offset){
            for(i = 0; i < 32 - offset; i++)
                tag_num += address[i] * pow(2, 32 - i - 1 - offset);
        }
        if(tag + associativity <= 32){
            for(i = 0; i < tag + associativity; i++)
                tag_num += address[i] * pow(2, tag - i - 1 + associativity); // +2
        }
        for(i = tag; i < tag + index; i++)
            index_num += address[i] * pow(2, index - i + tag - 1);
        for(i = 31; i > 29; i--)
            offset_b += address[i] * pow(2, 31 - i);
        for(i = 29; i > 29 - offset + 2; i--)
            offset_w += address[i] * pow(2, 29 - i);

        // change
        if(entries == associativity || entries < associativity)
            tmp = 0;
        if(entries > associativity)
            tmp = index_num % (entries/associativity);
        j = 0;

        /*if(df > 0){
            for(i = tmp * associativity; i < tmp * associativity + associativity; i++)
                printf("table:%d %d %d\n", i, table[i].tag, table[i].vaild);
            printf("%d %d %d %d %d\n", df, tmp, index_num, tag_num, ch);
        }*/

        for(i = tmp * associativity; i < tmp * associativity + associativity; i++){
            /*if(df > 250){
                printf("table:%d %d %d\n", i, table[i].tag, table[i].vaild);
            }*/
            if(table[i].tag == tag_num && table[i].vaild == 1){
                ch++, j = 1;
                table[i].hit = df;
                if((buffer[0] - '0') == 1)
                    table[i].dirty = 1;
                break;
            }
            if(table[i].vaild == 0){
                cm++, j = 1;
                table[i].tag = tag_num;
                table[i].change = df;
                table[i].hit = df;
                table[i].vaild = 1;
                if(table[i].dirty == 1)
                    mtr++;
                if((buffer[0] - '0') == 1)
                    table[i].dirty = 1;
                if((buffer[0] - '0') != 1)
                    table[i].dirty = 0;
                break;
            }
        }
        if(j == 0){
            min = df + 1;
            for(i = tmp * associativity; i < tmp * associativity + associativity; i++){
                if(strcmp(argv[4], "FIFO")==0){
                    if(table[i].change < min){
                        min = table[i].change;
                        set = i;
                    }
                }
                else if(strcmp(argv[4], "LRU")==0){
                    if(table[i].hit < min){
                        min = table[i].hit;
                        set = i;
                    }
                }
                else{
                    cout << "Unknown Replace Policy";
                    
                }
                //cout << "hi";
            }
            cm++;
            table[set].tag = tag_num;
            table[set].change = df;
            table[set].hit = df;
            table[set].vaild = 1;
            if(table[set].dirty == 1)
                mtr++;
            if((buffer[0] - '0') == 1)
                table[set].dirty = 1;
            if((buffer[0] - '0') != 1)
                table[set].dirty = 0;
        }

        
        if((buffer[0] - '0') == 0){ // read data
            rd++;
        }
        if((buffer[0] - '0') == 1){ // write data
            wd++;
        }
        if((buffer[0] - '0') == 2){ // read instruction
            j = j;
        }
        /*
        if(df > 250){
            printf("%d %d %d %d %d\n", df, associativity, i, tag_num, ch);
        }
            
        if(df > 300)
            break;*/
        //break;
    }
    for(i = 0; i < entries;i++){
        if(table[i].dirty == 1)
            mtr++;
    }
    input_file.close();
    float mr = cm/float(df);
    cout << "input file = " << file_name << endl;
    cout << "Demand fetch = " << df << endl;
    cout << "Cache hit = " << ch << endl;
    cout << "Cache miss = " << cm << endl;
    printf("Miss rate = %.4f\n", mr);
    cout << "Read data = " << rd << endl;
    cout << "Write data = " << wd << endl;
    cout << "Bytes from memory = " << cm * block_size << endl;
    cout << "Bytes to memory = " << mtr * block_size << endl;
    return 0;
}