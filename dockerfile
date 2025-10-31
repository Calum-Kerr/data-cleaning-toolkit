From ubuntu:22.04
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    python3 \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . .
RUN git clone https://github.com/emscripten-core/emsdk.git /emsdk && \
    cd /emsdk && \
    ./emsdk install latest && \
    ./emsdk activate latest
RUN /emsdk/upstream/emscripten/emcc src/algorithms-wasm.cpp \
    -o frontend/algorithms.js \
    -s WASM=1 \
    -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
    -s MODULARIZE=1 \
    -s EXPORT_NAME='AlgorithmsModule' \
    -s ALLOW_MEMORY_GROWTH=1 \
    --std=c++17
RUN if [ ! -d "vcpkg/.git" ]; then \
        rm -rf vcpkg && \
        git clone https://github.com/microsoft/vcpkg.git vcpkg; \
    fi
RUN cd vcpkg && \
    ./bootstrap-vcpkg.sh && \
    cd .. && \
    ./vcpkg/vcpkg install asio:x64-linux boost-algorithm:x64-linux boost-functional:x64-linux boost-optional:x64-linux boost-lexical-cast:x64-linux boost-array:x64-linux
RUN mkdir -p build && \
    cd build && \
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake && \
    cmake --build .
EXPOSE 8080
CMD ["./build/Toolkit"]