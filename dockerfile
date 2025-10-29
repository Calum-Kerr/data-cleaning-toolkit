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
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . .
RUN git submodule update --init --recursive && \
    cd vcpkg && \
    ./bootstrap-vcpkg.sh && \
    cd .. && \
    ./vcpkg/vcpkg install asio:x64-linux boost-algorithm:x64-linux boost-functional:x64-linux boost-optional:x64-linux boost-lexical-cast:x64-linux boost-array:x64-linux
RUN mkdir -p build && \
    cd build && \
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake && \
    cmake --build .
EXPOSE 8080
CMD ["./build/Toolkit"]