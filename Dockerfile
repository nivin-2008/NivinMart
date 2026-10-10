FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update

RUN apt-get install -y --no-install-recommends build-essential cmake pkg-config libdrogon-dev libjsoncpp-dev libsodium-dev libpq-dev uuid-dev zlib1g-dev libsqlite3-dev libmariadb-dev libmariadb-dev-compat libbrotli-dev ca-certificates

WORKDIR /app

COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DMYSQL_INCLUDE_DIR=/usr/include/mariadb -DMYSQL_LIB_DIR=/usr/lib/x86_64-linux-gnu -DMYSQL_LIBRARY=/usr/lib/x86_64-linux-gnu/libmariadb.so -DMYSQL_LIB=/usr/lib/x86_64-linux-gnu/libmariadb.so

RUN cmake --build build --config Release -j2

EXPOSE 10000

CMD ["./build/NivinMart"]
