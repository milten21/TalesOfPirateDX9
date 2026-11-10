# syntax=docker/dockerfile:1
# check=skip=FromPlatformFlagConstDisallowed

FROM --platform=linux/386 i386/debian:bookworm-slim AS toolchain
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        g++ cmake ninja-build pkg-config libicu-dev icu-devtools unixodbc-dev \
    && rm -rf /var/lib/apt/lists/*

FROM toolchain AS build
WORKDIR /src
COPY . .
RUN cmake -S /src -B /build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    && cmake --build /build \
    && ctest --test-dir /build --output-on-failure \
    && cmake --install /build --prefix /opt/tales

FROM --platform=linux/386 i386/debian:bookworm-slim AS runtime
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        libicu72 unixodbc odbc-postgresql gettext-base tini \
    && rm -rf /var/lib/apt/lists/* \
    && odbcinst -q -d -n "PostgreSQL ANSI" \
    && useradd --system --home-dir /opt/tales --shell /usr/sbin/nologin tales
COPY --from=build /opt/tales /opt/tales
COPY docker/entrypoint.sh /opt/tales/entrypoint.sh
COPY docker/listening.sh /opt/tales/listening.sh
COPY docker/connected.sh /opt/tales/connected.sh
COPY docker/templates /opt/tales/templates
RUN chmod 0755 /opt/tales/entrypoint.sh /opt/tales/listening.sh /opt/tales/connected.sh \
    && for dir in /opt/tales/*Server; do \
        logs="/var/log/tales/$(basename "$dir")"; \
        mkdir -p "$logs/log" "$logs/logfile"; \
        ln -s "$logs/log" "$dir/log"; \
        ln -s "$logs/logfile" "$dir/logfile"; \
    done \
    && mkdir -p /var/log/tales/GameServer/LOG \
    && ln -s /var/log/tales/GameServer/LOG /opt/tales/GameServer/LOG \
    && chown -R tales:tales /opt/tales /var/log/tales
USER tales
STOPSIGNAL SIGINT
ENTRYPOINT ["/usr/bin/tini", "--", "/opt/tales/entrypoint.sh"]
