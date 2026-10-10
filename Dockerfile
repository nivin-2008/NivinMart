FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update

RUN apt-get install -y --no-install-recommends build-essential cmake pkg-config libdrogon-dev libjsoncpp-dev libsodium-dev libpq-dev uuid-dev zlib1g-dev libsqlite3-dev default-libmysqlclient-dev ca-certificates

WORKDIR /app

COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

RUN cmake --build build --config Release -j2

EXPOSE 10000

CMD ["./build/NivinMart"]
