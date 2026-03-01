CXXFLAGS=-Wall -Wextra
CXX=c++

.PHONY: test
test: test.cpp testlib/*.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^
