#ifndef DECODE
#define DECODE

#include "../math_lib/structs.h"

using namespace std;

class TCodeParams {
    int n;
    int k;
    int l;
    TMatr H;
public:
    TCodeParams(int n_, int k_, int l_, const TMatr& HCheck);
    TMatr GetG() const;
    int N() const;
    int K() const;
    int L() const;
    const TMatr& GetH() const;
};

TCodeParams InitFromFile(const string& filename);

void Code(const string& code_params, const string& data_to_code, const string& channel_input);

void SendData(const string& code_params_filename, const string& channel_input_filename, const string& channel_output_filename, int error_count, int seed);

void Decode(const string& code_params_filename, const string& channel_output_filename, const string& output_data_filename);

string DecodeBlock(const TCodeParams& code_params, const TMatr& relations, const TMatr& inp_block);

#endif // DECODEs
