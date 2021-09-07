FROM marvins/exherbo-x86_64-pc-linux-gnu-gcc-with-clang:12
MAINTAINER Marvin Schmidt <marv@exherbo.org>

# Switch to option.conf.d layout
RUN mkdir /etc/paludis/options.conf.d \
    && mv /etc/paludis/options.conf /etc/paludis/options.conf.d/10_clang.conf

COPY ./config/options/* /etc/paludis/options.conf.d/
COPY ./config/sets/paludis-deps.conf /etc/paludis/sets/

RUN chgrp tty /dev/tty \
    && eclectic env update \
    && source /etc/profile \
    && cave resolve -1z repository/{media,net,pyro,rust,scientific} -x \
    && cave resolve \
            --execute \
            --preserve-world \
            --lazy \
            --keep-targets if-same \
            --keep if-same \
            --permit-old-version "*/*" \
            --recommendations ignore \
            --suggestions ignore \
            paludis-deps
RUN eclectic c++ set clang \
    && eclectic cc set clang \
    && eclectic cpp set clang

# Clean up
RUN rm -fr \
        /var/log/paludis/* \
        /var/cache/paludis/distfiles/* \
        /var/tmp/paludis/build/*

# Add non-privileged user
RUN useradd -M builder
USER builder
