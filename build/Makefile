all:
	g++ --std=c++20 ../main.cpp ../math_lib/structs.cpp ../algorithms/decode.cpp -o code
	cat ../content/input_data.txt
	./code --code ../content/matrix.txt ../content/input_data.txt ../content/channel_input.txt
	./code --senddata ../content/matrix.txt ../content/channel_input.txt ../content/channel_output.txt 2 12
	./code --decode ../content/matrix.txt ../content/channel_output.txt ../content/output_data.txt