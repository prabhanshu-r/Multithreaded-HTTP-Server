# ---- build stage ----
FROM debian:bookworm-slim AS build
RUN apt-get update \
    && apt-get install -y --no-install-recommends build-essential cmake \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY CMakeLists.txt .
COPY include include
COPY src src
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j

# ---- runtime stage (small image, no compiler) ----
FROM debian:bookworm-slim
RUN useradd --system --no-create-home appuser
WORKDIR /app
COPY --from=build /src/build/MultithreadedHTTPServer .
COPY config config
COPY public public
USER appuser
EXPOSE 8080
CMD ["./MultithreadedHTTPServer", "config/server.conf"]
