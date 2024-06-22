#include "math_lib/structs.h"
#include "algorithms/decode.h"

#include <iostream>
#include <cstring>

using namespace std;

void printHelp() {
    std::cout << "Usage: \n"
              << " --help                  for this menu\n"
              << " --code [code_params] [input_data] [channel_input]\n"
              << " --senddata [code_params] [channel_input] [channel_output] [error_count] [seed]\n"
              << " --decode [code_params] [channel_output] [output_data] \n";
}

void code(const char* code_params, const char* input_data, const char* channel_input) {
    std::cout << "Coding with params: " << code_params << ", input data: " << input_data << ", channel_input: " << channel_input << "\n";
    Code(code_params, input_data, channel_input);
}

void senddata(const char* code_params, const char* channel_input, const char* channel_output, const char* error_count, const char* seed) {
    std::cout << "Sending data. Coding params: " << code_params << ", channel input: " << channel_input << ", channel output: " << channel_output << ", error count: " << error_count << ", seed: " << seed << "\n";
    int errors = 0;
    try {
        errors = std::stoi(error_count);
        if(errors < 0) {
            std::cerr << "Error count is negative: " << errors << "\n";
            exit(1);
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error count is not a valid number: " << error_count << "\n";
        exit(1);
    }
    int seed_value = 0;
    try {
        seed_value = std::stoi(seed);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Seed value is not a valid number: " << seed << "\n";
        exit(1);
    }
    SendData(code_params, channel_input, channel_output, errors, seed_value);
}

void decode(const char* code_params, const char* channel_output, const char* output_data) {
    std::cout << "Decoding with params: " << code_params << ", channel output: " << channel_output << ", output data:" << output_data << "\n";
    Decode(code_params, channel_output, output_data);
}

int main(int argc, char* argv[]) {
    if(argc < 2) {
        printHelp();
        return 1;
    }

    if(std::strcmp(argv[1], "--help") == 0) {
        printHelp();
    } else if(std::strcmp(argv[1], "--decode") == 0) {
        if(argc != 5) {
            printHelp();
            return 1;
        }
        decode(argv[2], argv[3], argv[4]);
    } else if(std::strcmp(argv[1], "--code") == 0) {
        if(argc != 5) {
            printHelp();
            return 1;
        }
        code(argv[2], argv[3], argv[4]);
    } else if(std::strcmp(argv[1], "--senddata") == 0) {
        if(argc != 7) {
            printHelp();
            return 1;
        }
        senddata(argv[2], argv[3], argv[4], argv[5], argv[6]);
    } else {
        printHelp();
        return 1;
    }
    return 0;
}
