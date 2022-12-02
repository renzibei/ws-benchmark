# WebSocket Benchmark

This is a benchmark to test the performance of WebSocket libraries.

## Build

First clone this repo.
```
cd ws-benchmark
```

Pull all the submodules.
```
git submodule update --init --recursive
```

Then you can build the repo. There a few cmake options.
Regards flashws, if you want to build it without the f-stack and use the system
tcp stack instead, add `-DFWS_ENABLE_FSTACK=OFF` in cmake commands. If you want
to use f-stack and DPDK, please compile and install f-stack first.

```
mkdir build
cd build
cmake .. (options e.g. -DFWS_ENABLE_FSTACK=OFF)
make -j6
```
You should be able to compile it if no more problems happen.

## Usage

You can choose which server and which client to use. And we provide a python script
for you to test it.

```
Usage: python3 ../tools/bench_ws.py c|s (client or server) lib_name host_ip host_port output_data_file [other arguments for dpdk]
```

The data will be exported as a csv file.