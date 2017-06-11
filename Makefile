CXXFLAGS=-std=c++11 -Wall -pthread -O3 -Iinclude

SOURCES=main.cpp

core-latency: $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

.PHONY: distclean test

distclean:
	rm core-latency

test: core-latency
	@./core-latency
