#include "structs.h"

#include <cassert>
#include <iostream>

TNumber::TNumber(int val, int l)
{
    assert(l <= 30);
    assert(0 <= val && val < (1 << l));
    val_ = val;
    l_ = l;
}

TNumber::TNumber(const TNumber& number)
    : val_(number.val_)
    , l_(number.l_)
{ }

int TNumber::Val() const {
    return val_;
}

int TNumber::L() const {
    return l_;
}

TNumber operator + (const TNumber& l, const TNumber& r) {
    assert (l.l_ == r.l_);
    return TNumber(l.val_ ^ r.val_, l.l_);
}

TNumber operator * (const TNumber& l, const TNumber& r) {
    assert (l.l_ == r.l_);
    return TNumber(l.val_ & r.val_, l.l_);
}

TMatr::TMatr(const std::vector<std::vector<int>>& numbers, int l) {
    for (const auto& line : numbers) {
        data.push_back({});
        assert(line.size() == numbers[0].size());
        for (const auto el : line) {
            data.back().push_back(TNumber{el, l});
        }
    }
}

TMatr::TMatr(const TMatr& TMatr)
    : data(TMatr.data)
{ }

TMatr::TMatr()
{}

int TMatr::Lines() const {
    return data.size();
}

int TMatr::Col() const {
    if (data.size() == 0) {
        return 0;
    }
    return data[0].size();
}

int TMatr::L() const {
    assert(data.size() != 0);
    assert(data[0].size() != 0);
    return data[0][0].L();
}

const std::vector<std::vector<TNumber>>& TMatr::Data() const {
    return data;
}

std::vector<std::vector<TNumber>>& TMatr::MutableData() {
    return data;
}

TMatr operator + (const TMatr& l, const TMatr& r) {
    assert(l.Lines() == r.Lines() && l.Col() == r.Col());
    TMatr res(l);
    for (int i = 0; i < res.data.size(); ++i) {
        for (int j = 0; j < res.data[0].size(); ++j) {
            res.data[i][j] = res.data[i][j] + r.data[i][j];
        }
    }
    return res;
}

TMatr operator * (const TMatr& l, const TMatr& r) {
    assert(l.Col() == r.Lines());
    TMatr res;
    for (int i = 0; i < l.Lines(); ++i) {
        res.data.push_back({});
        for (int j = 0; j < r.Col(); ++j) {
            res.data.back().push_back(TNumber{0, l.L()});
            for (int k = 0; k < l.Col(); ++k) {
                res.data.back().back() = res.data.back().back() + l.data[i][k] * r.data[k][j];
            }
        }
    }
    return res;
}
