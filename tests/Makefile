CXXFLAGS=-Wall -Wextra
CXX=c++

run-test: test.cpp testlib/*.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

test: run-test
	./run-test
