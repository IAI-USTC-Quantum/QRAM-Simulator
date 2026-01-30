#!/bin/bash
git pull && \
mkdir -p build && \
cd build && \

# ensure we are in build directory
[ "$(basename "$(pwd)")" = "build" ] && rm -rf * && \
cmake .. -DCMAKE_BUILD_TYPE=Release&& \
make -j12 && \
./bin/ExecutabilityTest && \
./bin/CorrectnessTest && \
./bin/CudaExecutabilityTest

# 打印最终状态
if [ $? -eq 0 ]; then
  echo "All tests passed. Congratulations!"
else
  echo "Test failed."
fi
