A simple tool for measuring latency between CPU cores.

Execute `make test` command to run the benchmark.

Requires Linux with a C++11 compiler (tested with gcc) and boost libraries.

Note that measured latencies are for full roundtrip between two cores.
Divide them by two to get actual time needed to fetch data from another core.

Latencies are commutative so only one measument is displayed for each core pair.
