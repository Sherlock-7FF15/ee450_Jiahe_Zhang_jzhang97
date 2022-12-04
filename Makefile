OBJ= client.o serverM.o serverC.o serverCS.o serverEE.o
OUT= client serverM serverC serverCS serverEE 
CXX= g++
FLAGS= -Wall -std=c++11 -g

all : $(OUT)

serverM : serverM.o
	$(CXX) $(FLAGS) -o $@ $^

serverC : serverC.o
	$(CXX) $(FLAGS) -o $@ $^

serverCS : serverCS.o
	$(CXX) $(FLAGS) -o $@ $^

serverEE : serverEE.o
	$(CXX) $(FLAGS) -o $@ $^

client : client.o
	$(CXX) $(FLAGS) -o $@ $^

%.o:%.cpp
	$(CXX) $(FLAGS) -c -o $@ $^


.PHONY:clean
clean:
	rm $(OUT)
	rm $(OBJ)
