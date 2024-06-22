#ifndef STRUCTS
#define STRUCTS

#include <vector>
#include <string>

using namespace std;

class TNumber {
    int val_;
    int l_;
public:
    TNumber(int val, int l);
    TNumber(const TNumber& TNumber);
    friend TNumber operator + (const TNumber& l, const TNumber& r);
    friend TNumber operator * (const TNumber& l, const TNumber& r);
    int Val() const;
    string to_print() const {
        string res(to_string((1 << l_) - 1).size(), '0');
        string tmp = to_string(val_);
        for (int i = 0; i < tmp.size(); ++i) {
            res[i + res.size() - tmp.size()] = tmp[i];
        }
        return res;
    }
    int L() const;
};

TNumber operator + (const TNumber& l, const TNumber& r);
TNumber operator * (const TNumber& l, const TNumber& r);

class TMatr {
    vector<vector<TNumber>> data;
public:
    TMatr(const vector<vector<int>>& TNumbers, int l);
    TMatr(const TMatr& TMatr);
    TMatr();
    int Lines() const;
    int Col() const;
    int L() const;
    const vector<vector<TNumber>>& Data() const;
    vector<vector<TNumber>>& MutableData();

    friend TMatr operator + (const TMatr& l, const TMatr& r);
    friend TMatr operator * (const TMatr& l, const TMatr& r);
};

TMatr operator + (const TMatr& l, const TMatr& r);
TMatr operator * (const TMatr& l, const TMatr& r);

#endif // STRUCTS
