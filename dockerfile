From ubuntu:20.04
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . .
RUN mkdir build && \
    cd build && \
    cmake .. && \
    cmake --build .
EXPOSE 8080
CMD ["./build/Toolkit"]