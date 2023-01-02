#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>

using namespace std;

bool ram[64000];
bool memory[512888];

int rootPTR = 504104;
int bootPTR = 0;
int  fatPTR = 4392;

int rootSize = 7808;
int bootSize = 4392;
int  fatSize = 499712;

void CPU(bool &run);
void Run(int ptr);

int pow(int n, int p) {
    if (p == 0)
        return 1;
    int ret = n;
    for (; p > 1; p--)
        ret *= n;
    return ret;
}

int bin2num(bool num[], int size) {
    int ret = 0;
    for (int i = size - 1; i >= 0; i--) {
        if (num[i])
            ret += pow(2, size - i - 1);
    }
    return ret;
}

void num2bin(int num, bool *ret, int size) {
    for (int i = 0; i < size; i++)
        ret[i] = 0;
    for (int i = size - 1; i >= 0; i--) {
        ret[i] = num % 2;
        num /= 2;
    }
}

int main() {
    for (int i = 0; i < 512888; i++)
        memory[i] = 0;

    for (int i = bootPTR; i < bootPTR + bootSize; i++) {
        if (i % 9 == 8)
            memory[i] = 0;
        else
            memory[i] = 1;
    }

    memory[fatPTR] = 1;

    bool run = true;
    while (run)
        CPU(run);
    return 0;
}

void CPU(bool &run) {
    string input;
    getline(cin, input);
    if (input == "mem -r") {
        for (int i = rootPTR; i < rootSize + rootPTR; i += 18) {
            bool num[9];
            bool name[9];
            for (int j = 0; j < 9; j++)
                name[j] = memory[i + j];
            for (int j = 8; j < 18; j++)
                num[j - 9] = memory[i + j];
            cout << bin2num(name, 9) << ": " << bin2num(num, 9) << "\n";
        }
        cout << endl;
    }
    else if (input == "mem -f") {
        for (int i = fatPTR; i < fatPTR + fatSize; i += 8) {
            if ((i - fatPTR) % 1024 == 0)
                cout << "\n\nCluster:\n";
            bool num[8];
            for (int j = 0; j < 8; j++)
                num[j] = memory[i + j];
            cout << bin2num(num, 8) << " ";
        }
        cout << endl << endl;
    }
    else if (input == "mem -b") {
        for (int i = bootPTR; i < bootPTR + bootSize; i += 9) {
            cout << (i - bootPTR) / 9 << ": ";
            bool num[9];
            for (int j = 0; j < 9; j++) 
                num[j] = memory[i + j];
            cout << bin2num(num, 9) << "\n";
        }
        cout << endl;
    }
    else if (input[0] == 'r' && input[1] == ' ') {
        string name = "";
        for (int i = 2; i < input.size(); i++)
            name += input[i];
        int n = stoi(name);
        int begin;
        for (int i = rootPTR; i < rootSize + rootPTR; i += 18) {
            bool nm[9];
            for (int j = 0; j < 9; j++) 
                nm[j] = memory[i + j];
            if (bin2num(nm, 9) == n) {
                bool start[9];
                for (int j = 0; j < 9; j++)
                    start[j] = memory[9 + j + i];
                begin = bin2num(start, 9);
                break;
            }
        }
        bool state[9];
        for (int i = 0; i < 9; i++) {
            state[i] = memory[bootPTR + begin * 9 + i];
        }
        for (int i = fatPTR + begin * 1024; i < fatPTR + (begin + 1) * 1024; i += 8) {
            bool num[8];
            for (int j = 0; j < 8; j++) 
                num[j] = memory[i + j];
            cout << bin2num(num, 8) << " ";
        }
        cout << endl << endl;
        while (bin2num(state, 9) != 511) {
            for (int i = fatPTR + bin2num(state, 9) * 1024; i < fatPTR + (bin2num(state, 9) + 1) * 1024; i += 8) {
                bool num[8];
                for (int j = 0; j < 8; j++) 
                    num[j] = memory[i + j];
                cout << bin2num(num, 8) << " ";
            }
            cout << endl << endl;
            int last_state = bin2num(state, 9);
            for (int i = 0; i < 9; i++) {
                state[i] = memory[bootPTR + last_state * 9 + i];
            }
        }
    }
    else if (input[0] == 'c' && input[1] == ' ') {
        string n = "";
        string s = "";
        int g = 2;
        for (; input[g] != ' '; g++)
            n += input[g];
        g++;
        for (; g < input.size(); g++)
            s += input[g];
        int name = stoi(n);
        int size = stoi(s);

        for (int j = rootPTR; j < rootPTR + rootSize; j += 18) {
            bool num[9];
            for (int i = 0; i < 9; i++)
                num[i] = memory[i + j];

            if (bin2num(num, 9) == 0) {
                bool bin_name[9]; num2bin(name, bin_name, 9);
                for (int i = 0; i < 9; i++) 
                    memory[j + i] = bin_name[i];
                int free_clusters[size];
                int free_clusters_count = 0;
                for (int h = bootPTR; h < bootPTR + bootSize && free_clusters_count < size; h += 9) {
                    bool num1[9];
                    for (int i = 0; i < 9; i++)
                        num1[i] = memory[h + i];
                    if (bin2num(num1, 9) == 510) {
                        free_clusters[free_clusters_count] = h / 9;
                        free_clusters_count++;
                    }
                }
                bool num1[9]; num2bin(free_clusters[0], num1, 9);
                for (int i = 0; i < 9; i++)
                    memory[i + j + 9] = num1[i];
                for (int h = 0; h < size - 1; h++) {
                    bool num2[9]; num2bin(free_clusters[h + 1], num2, 9);
                    for (int i = 0; i < 9; i++)
                        memory[free_clusters[h] * 9 + i + bootPTR] = num2[i];
                }
                for (int i = 0; i < 9; i++)
                    memory[free_clusters[size - 1] * 9 + i + bootPTR] = 1;
                break;
            }
        }
    }
    else if (input[0] == 'e' && input[1] == ' ') {
        string n, load_file_name;
        int ii = 2;
        for (; input[ii] != ' '; ii++)
            n += input[ii];
        ii++;
        for (; ii < input.size(); ii++)
            load_file_name += input[ii];
        int name = stoi(n);
        ifstream load_file(load_file_name);
        string str;
        string num;
        vector<int> load_file_nums;
        while (getline(load_file, str)) {
            for (int i = 0; i < str.size(); i++) {
                if (str[i] != ' ')
                    num += str[i];
                else {
                    load_file_nums.push_back(stoi(num));
                    num = "";
                }
            }
            load_file_nums.push_back(stoi(num));
            num = "";
        }
        int first_cluster = 0;
        for (int j = rootPTR; j < rootPTR + rootSize; j += 18) {
            bool num1[9];
            for (int i = 0; i < 9; i++)
                num1[i] = memory[j + i];
            if (name == bin2num(num1, 9)) {
                for (int i = 0; i < 9; i++)
                    num1[i] = memory[i + j + 9];
                first_cluster = bin2num(num1, 9);
            }
        }
        int state = first_cluster;
        int h = 0;
        do {
            bool num1[9];
            for (int i = 0; i < 9; i++)
                num1[i] = memory[bootPTR + state * 9 + i];
            for (int j = 0; j < fmin(load_file_nums.size() - 128 * h, 128); j++) {
                bool num2[8];num2bin(load_file_nums[j + h * 128], num2, 8);
                for (int i = 0; i < 8; i++)
                    memory[fatPTR + state * 1024 + j * 8 + i] = num2[i];
            }
            state = bin2num(num1, 9);
            h++;
        } while (state != 511);
    }
    else if (input == "exit")
        run = false;
    else if (input == "clear")
        cout << "\033[2J" << endl;
}

void Run(int ptr) {

}