FROM ubuntu:20.04
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC
ENV PORT=8080
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    zip \
    unzip \
    tar \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . .
RUN if [ ! -d "vcpkg" ]; then \
        git clone https://github.com/Microsoft/vcpkg.git && \
        cd vcpkg && \
        chmod +x ./bootstrap-vcpkg.sh && \
        ./bootstrap-vcpkg.sh && \
        ./vcpkg install asio:x64-linux boost-algorithm:x64-linux boost-functional:x64-linux boost-optional:x64-linux boost-lexical-cast:x64-linux boost-array:x64-linux; \
    else \
        cd vcpkg && \
        chmod +x ./bootstrap-vcpkg.sh && \
        ./bootstrap-vcpkg.sh && \
        ./vcpkg install asio:x64-linux boost-algorithm:x64-linux boost-functional:x64-linux boost-optional:x64-linux boost-lexical-cast:x64-linux boost-array:x64-linux; \
    fi
RUN mkdir build && \
    cd build && \
    cmake .. -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release && \
    make
RUN groupadd -r appgroup && useradd -r -g appgroup appuser
RUN chown -R appuser:appgroup /app
USER appuser
EXPOSE $PORT
CMD ["./build/Toolkit"]