FROM exherbo/exherbo-x86_64-pc-linux-gnu-base:latest
MAINTAINER Marvin Schmidt <marv@exherbo.org>

# Switch to option.conf.d layout
RUN rm /etc/paludis/options.conf \
    && mkdir /etc/paludis/options.conf.d

COPY ./config/options/* /etc/paludis/options.conf.d/
COPY ./config/sets/paludis-deps.conf /etc/paludis/sets/

RUN chgrp tty /dev/tty \
    && eclectic env update \
    && source /etc/profile \
    && cave resolve -1z repository/{media,pyro,python,scientific,x11} -x \
    && cave resolve -1z -Ks -ks -x \
            -o dev-python/sphinxcontrib-websupport \
            --recommendations ignore \
            --suggestions ignore \
            paludis-deps

# Clean up
RUN rm -fr \
        /var/log/paludis* \
        /var/cache/paludis/distfiles/* \
        /var/tmp/paludis/build/*

# Add non-privileged user
RUN useradd -M builder
USER builder
