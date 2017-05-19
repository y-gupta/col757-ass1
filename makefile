OPTS = -std=c++11 -fopenmp -g
q1: q1.cpp
	g++ $(OPTS) q1.cpp -o q1
q2: q2.cpp merge.h partition.h
	g++ $(OPTS) q2.cpp -o q2
test:
	g++  $(OPTS) test.cpp -o t