FLAGS=-O3 -Wall -finput-charset=UTF-8 -D_UNICODE -std=c++11
all: 
	g++ $(FLAGS) -fPIC -c ../../Tokenizer.cpp ../../Tokenizer.AlphaNum.cpp ../../Tokenizer.Win1251.cpp ../../russian/*.cpp ../../ukrainian/UkrDict.cpp
	ar rcs libmorpher.a *.o
	g++ -static $(FLAGS) ../test.cpp -L. -lmorpher -o test
	./test
clean:
	rm test
	rm libmorpher.a
	rm *.o
