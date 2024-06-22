#include "decode.h"

#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

const string DELIM = "--------------------------------------------------";


TCodeParams::TCodeParams(int n_, int k_, int l_, const TMatr& HCheck)
    : n(n_)
    , k(k_)
    , l(l_)
    , H(HCheck)
{
    assert(k < n);
    assert(H.Col() == n);
    assert(H.Lines() == n - k);
}

int TCodeParams::N() const {
    return n;
}

int TCodeParams::K() const {
    return k;
}

int TCodeParams::L() const {
    return l;
}


const TMatr& TCodeParams::GetH() const {
    return H;
}

TMatr TCodeParams::GetG() const {
    vector<vector<int>> res(k, vector<int>(n));
    for (int i = 0; i < k; ++i) {
        res[i][i] = (1 << l) - 1;
    }
    TMatr myH = H;
    vector<vector<TNumber>>& data = myH.MutableData();
    auto add = [](vector<TNumber>& l, const vector<TNumber>& r) {
        assert(l.size() == r.size());
        for (int i = 0; i < l.size(); ++i) {
            l[i] = l[i] + r[i];
        }
        return;
    };
    for (int line_of_gauss = 0; line_of_gauss < n - k; ++line_of_gauss) {
        int pos_with_one = -1;
        for (int cur = line_of_gauss; cur < n - k; ++cur) {
            if (data[cur][k + line_of_gauss].Val()) {
                pos_with_one = cur;
            }
        }
        assert(pos_with_one != 1); // H contains linear-depended lines
        swap(data[line_of_gauss], data[pos_with_one]);
        for (int relax = 0; relax < n - k; ++relax) {
            if (relax == line_of_gauss) {
                continue;
            }
            if (data[relax][k + line_of_gauss].Val()) {
                add(data[relax], data[line_of_gauss]);
            }
        }
    }
    for (int izbit_id = 0; izbit_id < n - k; ++izbit_id) {
        for (int real_id = 0; real_id < k; ++real_id) {
            res[real_id][k + izbit_id] = data[izbit_id][real_id].Val();
        }
    }
    return TMatr(res, l);
}

TCodeParams InitFromFile(const string& filename) {
    ifstream in(filename);
    if (!in)
    {
        cerr << "Failed to open file " << filename << endl;
        assert(false);
    }
    int n, k, l;
    in >> n >> k >> l;
    vector<vector<int>> dat(n - k, vector<int>(n));
    for (int i = 0; i < n - k; ++i) {
        for (int j = 0; j < n; ++j) {
            in >> dat[i][j];
            assert(0 <= dat[i][j] && dat[i][j] <= 1);
            dat[i][j] *= (1 << l) - 1;
        }
    }
    in.close();
    TMatr matr(dat, l);
    return TCodeParams(n, k, l, matr);
}

void Code(const string& code_params_filename, const string& data_to_code, const string& channel_input) {
    cout << DELIM << " CODING INFORMATION " << DELIM << endl;
    TCodeParams code_params = InitFromFile(code_params_filename);
    TMatr G = code_params.GetG();
    ifstream inFile(data_to_code);
    ofstream outFile(channel_input);
    if (!inFile) {
        cerr << "Error opening " << data_to_code << " file\n";
        return;
    }
    if  (!outFile) {
        cerr << "Error opening " << channel_input << " file\n";
        return;
    }

    string line;
    getline(inFile, line);
    if (line.size() && line.back() == '\n') {
        line.pop_back();
    }
    inFile.close();

    cout << "Line to code:\nContent: " << line << "; Total size: " << line.size() << "\n";
    int pos = 0;
    int line_count = 0;
    for (int i = 0; i < line.size(); i += code_params.K() * code_params.L()) {
        cout << "Block " << line_count++ << ": ";
        for (int j = 0; j < code_params.K() && pos < line.size(); ++j) {
            for (int k = 0; k < code_params.L() && pos < line.size(); ++k) {
                cout << line[pos++];
            }
            cout << "; ";
        }
        cout << "\n";
    }
    cout << '\n' << DELIM << endl;

    cout << "Matrix G:\n";
    for (const auto& y : G.Data()) {
        for (const auto x : y) {
            cout << x.Val() << " ";
        }
        cout << endl;
    }
    cout << DELIM << endl;


    pos = 0;
    line_count = 0;
    string finalResult = "";
    for (int i = 0; i < line.size(); i += code_params.K() * code_params.L()) {
        cout << "Block " << line_count++ << ": ";
        vector<vector<int>> data(1, vector<int>(code_params.K()));
        for (int j = 0; j < code_params.K() && pos < line.size(); ++j) {
            for (int k = 0; k < code_params.L() && pos < line.size(); ++k) {
                assert('0' <= line[pos] && line[pos] <= '1');
                data[0][j] = data[0][j] + data[0][j] + (line[pos++] - '0');
            }
        }
        TMatr inp_block(data, code_params.L());
        cout << "Input data: ( ";
        for (const auto& y : inp_block.Data()) {
            for (const auto x : y) {
                cout << x.Val() << " ";
            }
        }
        cout << "); Encoded data: ( ";

        TMatr code_res = inp_block * G;

        for (const auto& y : code_res.Data()) {
            for (const auto x : y) {
                cout << x.Val() << " ";
            }
        }
        cout << ")" << endl;
        for (int i = 0; i < code_params.N(); ++i) {
            for (int j = 0; j < code_params.L(); ++j) {
                outFile << ((code_res.Data()[0][i].Val() >> (code_params.L() - j - 1)) & 1);
                finalResult.push_back('0' + (((code_res.Data()[0][i].Val() >> (code_params.L() - j - 1)) & 1)));
            }
        }
    }
    outFile.close();
    cout << "Result of coding: " << finalResult << "; Total size: " << finalResult.size() << endl;
    cout << DELIM << " FINISHED CODING INFORMATION " << DELIM << endl;
}

vector<int> select_from_range(int pos_count, int range) {
    assert(pos_count <= range);
    // Making random shuffle vector of {0, 1, ..., range - 1}
    std::vector<int> tmp(range);
    for (int i = 0; i < range; ++i) {
        tmp[i] = i;
        swap(tmp[i], tmp[rand() % (i + 1)]);
    }
    // First pos_count elements would be randomly selected from range
    while (tmp.size() > pos_count) {
        tmp.pop_back();
    }
    // Getting select_from_range.back() + select_from_range.pop_back() give us increasing order of positions.
    sort(tmp.rbegin(), tmp.rend());
    return tmp;
}

void SendData(const std::string& code_params_filename, const std::string& channel_input_filename, const std::string& channel_output_filename, int error_count, int seed) {
    cout << DELIM << " \"SENDING\" INFORMATION " << DELIM << endl;
    TCodeParams code_params = InitFromFile(code_params_filename);
    ifstream inFile(channel_input_filename);
    ofstream outFile(channel_output_filename);
    if (!inFile) {
        cerr << "Error opening " << channel_input_filename << " file\n";
        return;
    }
    if  (!outFile) {
        cerr << "Error opening " << channel_output_filename << " file\n";
        return;
    }
    string line;
    getline(inFile, line);
    if (line.size() && line.back() == '\n') {
        line.pop_back();
    }
    inFile.close();
    int pos = 0;
    cout << "Input line: " << line << "; Total size: " << line.size() << endl;
    string title = "Q-symbols: ";
    cout << title;
    for (int i = 0; i < line.size(); i += code_params.N() * code_params.L()) {
        cout << "( ";
        for (int j = 0; j < code_params.N() && pos < line.size(); ++j) {
            for (int k = 0; k < code_params.L() && pos < line.size(); ++k) {
                cout << line[pos++];
            }
            cout << " ";
        }
        cout << ") ";
    }
    cout << endl;

    // Making errors
    string eror_title = "Errors: ";
    cout << eror_title << string(title.size() - eror_title.size(), ' ');
    srand(seed);
    pos = 0;
    string prev_line = line;
    assert(line.size() % (code_params.N() * code_params.L()) == 0);
    int iters_count = line.size() / (code_params.N() * code_params.L());
    int now_errors = error_count;
    for (int i = 0; i < line.size(); i += code_params.N() * code_params.L()) {
        cout << "( ";
        vector<int> error_positions = select_from_range(now_errors, code_params.N());
        for (int j = 0; j < code_params.N() && pos < line.size(); ++j) {
            if (!error_positions.size() || error_positions.back() != j) {
                cout << string(code_params.L(), ' ');
                pos += code_params.L();
            } else {
                error_positions.pop_back();
                for (int k = 0; k < code_params.L() && pos < line.size(); ++k) {
                    line[pos] = '0' + (rand() % 2);
                    cout << line[pos++];
                }
            }
            cout << " ";
        }
        cout << ") ";
    }
    cout << endl;
    pos = 0;
    cout << title;
    for (int i = 0; i < line.size(); i += code_params.N() * code_params.L()) {
        cout << "( ";
        for (int j = 0; j < code_params.N() && pos < line.size(); ++j) {
            for (int k = 0; k < code_params.L() && pos < line.size(); ++k) {
                cout << line[pos++];
            }
            cout << " ";
        }
        cout << ") ";
    }
    cout << endl;

    string error_title = "Diff:";
    cout << error_title << string(title.size() - error_title.size(), ' ');
    pos = 0;
    for (int i = 0; i < line.size(); i += code_params.N() * code_params.L()) {
        cout << "( ";
        for (int j = 0; j < code_params.N() && pos < line.size(); ++j) {
            int cor_block = 1;
            for (int k = 0; k < code_params.L() && pos < line.size(); ++k) {
                if (line[pos] != prev_line[pos]) {
                    cor_block = 0;
                }
                ++pos;
            }
            pos -= code_params.L();
            for (int k = 0; k < code_params.L() && pos < line.size(); ++k) {
                if (cor_block) {
                    cout << " ";
                } else {
                    cout << char('0' + (prev_line[pos] ^ line[pos]));
                }
                ++pos;
            }
            cout << " ";
        }
        cout << ") ";
    }
    cout << endl;
    cout << "Output line: " << line << "; Total size: " << line.size() << endl;
    outFile << line << endl;
    outFile.close();
    cout << DELIM << " FINISHED \"SENDING\" INFORMATION " << DELIM << endl;
}

void Decode(const std::string& code_params_filename, const std::string& channel_output_filename, const std::string& output_data_filename) {
    cout << DELIM << " DECODING INFORMATION " << DELIM << endl;
    TCodeParams code_params = InitFromFile(code_params_filename);
    ifstream inFile(channel_output_filename);
    ofstream outFile(output_data_filename);
    if (!inFile) {
        cerr << "Error opening " << channel_output_filename << " file\n";
        return;
    }
    if  (!outFile) {
        cerr << "Error opening " << output_data_filename << " file\n";
        return;
    }
    string line;
    getline(inFile, line);
    if (line.size() && line.back() == '\n') {
        line.pop_back();
    }
    inFile.close();
    cout << "Decoding params:\nN: " << code_params.N() << "; K: " << code_params.K() << endl;
    cout << "Matrix H:\n";
    for (const auto& y : code_params.GetH().Data()) {
        for (const auto x : y) {
            cout << x.Val() << " ";
        }
        cout << endl;
    }
    cout << DELIM << endl;
    cout <<"Decoding line: " << line << endl;
    // Build all checking relations.
    TMatr check(code_params.GetH());
    std::vector<std::vector<TNumber>>& mat = check.MutableData();

    auto add = [](vector<TNumber>& l, const vector<TNumber>& r) {
        assert(l.size() == r.size());
        for (int i = 0; i < l.size(); ++i) {
            l[i] = l[i] + r[i];
        }
        return;
    };
    for (int mask = 1; mask < (1 << (code_params.N() - code_params.K())); ++mask) {
        std::vector<TNumber> newLine(code_params.N(), TNumber(0, check.L()));
        int line_add = 0;
        for (int byte = 0; byte < code_params.N() - code_params.K(); ++byte) {
            if ((mask >> byte) & 1) {
                add(newLine, mat[byte]);
                ++line_add;
            }
        }
        if (line_add > 1) {
            mat.push_back(newLine);
        }
    }


    int pos = 0;
    int line_count = 0;
    string finalResult = "";
    for (int i = 0; i < line.size(); i += code_params.N() * code_params.L()) {

        vector<vector<int>> data(1, vector<int>(code_params.N()));
        for (int j = 0; j < code_params.N() && pos < line.size(); ++j) {
            for (int k = 0; k < code_params.L() && pos < line.size(); ++k) {
                assert('0' <= line[pos] && line[pos] <= '1');
                data[0][j] = data[0][j] + data[0][j] + (line[pos++] - '0');
            }
        }
        TMatr inp_block(data, code_params.L());
        string decodeResult = DecodeBlock(code_params, check, inp_block);
        cout << "Block " << line_count++ << ": " << "Encoded data: ( ";
        for (const auto& y : inp_block.Data()) {
            for (const auto x : y) {
                cout << x.Val() << " ";
            }
        }
        auto f = [](const std::string& s, const TCodeParams& code_params) {
            string res;
            if (s[0] == 'X') {
                for (int i = 0; i < code_params.K(); ++i) {
                    res += "X" + string(TNumber(0, code_params.GetH().L()).to_print().size(), ' ');
                }
                return res;
            }
            int pos = 0;
            for (int i = 0; i < code_params.K(); ++i) {
                int val = 0;
                for (int j = 0; j < code_params.L(); ++j) {
                    val += val + (s[pos++] - '0');
                }
                res += TNumber(val, code_params.GetH().L()).to_print() + " ";
            }
            return res;

        };
        cout << "); Decoded data: ( " << f(decodeResult, code_params) << ") " << endl << DELIM << endl;
        outFile << decodeResult;
    }

    cout << DELIM << " FINISHED DECODING INFORMATION " << DELIM << endl;
}

string DecodeBlock(const TCodeParams& code_params, const TMatr& relations, const TMatr& inp_block) {
    auto check_relation = [](const std::vector<TNumber>& relation, const std::vector<TNumber>& data) {
        assert(relation.size() == data.size());
        TNumber res(0, relation[0].L());
        for (int i = 0; i < relation.size(); ++i) {
            res = res + relation[i] * data[i];
        }
        return res.Val() == 0;
    };
    int correctRelations = 0;
    vector<int> localized_symbols(code_params.N());
    for (int line = 0; line < relations.Lines(); ++line) {
        if (check_relation(relations.Data()[line], inp_block.Data()[0])) {
            correctRelations += 1;
            for (int i = 0; i < relations.Data()[line].size(); ++i) {
                if (relations.Data()[line][i].Val() != 0) {
                    localized_symbols[i] = 1;
                }
            }
        }
    }
    cout << "Correct checking relations count: " << correctRelations << endl;
    int non_localized = code_params.N();
    for (const auto x : localized_symbols) {
        non_localized -= x;
    }
    cout << endl << "Localized: " << code_params.N() - non_localized << ", not localized: " << non_localized << endl;
    assert (code_params.N() <= 26);
    auto letters = [&]() {
        for (int i = 0; i < code_params.N(); ++i) {
            cout << char('A' + i);
            for (int j = 0; j < code_params.GetH().Data()[0][0].to_print().size(); ++j) {
                cout << " ";
            }
        }
        cout << endl;
    };
    letters();
    for (const auto x : localized_symbols) {
        cout << x << " ";
    }
    cout << " - LocalizedInfo" << endl;
    if (non_localized > (code_params.N() - code_params.K()) / 2) {
        cout << "Failed to decode block: too much unlocalized symbols. Max: ";
        cout << (code_params.N() - code_params.K()) / 2 << "; Got: " << non_localized << endl;
        return string(code_params.K() * code_params.L(), 'X');
    }
    int valid_vals = code_params.N() - code_params.K() - non_localized;
    int expect_correct_relations = (1 << valid_vals) - 1;
    if (expect_correct_relations > correctRelations) {
        cout << "Failed to decode block: Bad count of correct relations. ";
        cout << "Expect " << expect_correct_relations << " correct relations with " << non_localized;
        cout << " non-localized symbols, got " << correctRelations << " correct relations" << endl;
        return string(code_params.K() * code_params.L(), 'X');
    }
    // Build relations to restore unlocalized_values;
    vector<vector<TNumber>> data = relations.Data();
    auto add = [](vector<TNumber>& l, const vector<TNumber>& r) {
        assert(l.size() == r.size());
        for (int i = 0; i < l.size(); ++i) {
            l[i] = l[i] + r[i];
        }
        return;
    };
    vector<int> non_localized_pos;
    for (int i = 0; i < localized_symbols.size(); ++i) {
        if (localized_symbols[i] == 0) {
            non_localized_pos.push_back(i);
        }
    }
    for (int line_of_gauss = 0; line_of_gauss < non_localized_pos.size(); ++line_of_gauss) {
        int pos_with_one = -1;
        for (int cur = line_of_gauss; cur < data.size(); ++cur) {
            if (data[cur][non_localized_pos[line_of_gauss]].Val()) {
                pos_with_one = cur;
            }
        }
        assert(pos_with_one != 1); // H contains linear-depended lines
        swap(data[line_of_gauss], data[pos_with_one]);
        for (int relax = 0; relax < data.size(); ++relax) {
            if (relax == line_of_gauss) {
                continue;
            }
            if (data[relax][non_localized_pos[line_of_gauss]].Val()) {
                add(data[relax], data[line_of_gauss]);
            }
        }
    }
    std::vector<TNumber> res = inp_block.Data()[0];
    if (non_localized_pos.size() > 0) {
        cout << endl;
        letters();
        for (int i = 0; i < code_params.N(); ++i) {
            cout << res[i].to_print() << " ";
        }
        cout << endl;
        for (int restore_pos = 0; restore_pos < non_localized_pos.size(); ++restore_pos) {
            cout << char('A' + non_localized_pos[restore_pos]) << ' ';
            TNumber fixed_val(0, code_params.GetH().L());
            char sym = '=';
            for (int val = 0; val < code_params.N(); ++val) {
                if (val == non_localized_pos[restore_pos]) {
                    continue;
                }
                if (data[restore_pos][val].Val() != 0) {
                    cout << sym << ' ';
                    sym = '+';
                    cout << char('A' + val) << ' ';
                    fixed_val = fixed_val + res[val];
                }
            }
            res[non_localized_pos[restore_pos]] = fixed_val;
            cout << endl;
        }
    }
    for (const auto x : res) {
        cout << x.to_print() << " ";
    }
    cout << endl;
    string result;
    for (int i = 0; i < code_params.N(); ++i) {
        for (int j = 0; j < code_params.L(); ++j) {
            result.push_back('0' + (((res[i].Val() >> (code_params.L() - j - 1)) & 1)));
        }
    }
    return result;
}