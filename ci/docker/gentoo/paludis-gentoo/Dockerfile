FROM gentoo/portage:latest as portage
FROM gentoo/stage3-amd64:latest

# copy the entire portage volume in
COPY --from=portage /usr/portage /usr/portage

COPY ./config/package.use /etc/portage/
COPY ./config/package.mask /etc/portage/
COPY ./config/package.accept_keywords /etc/portage/
COPY ./config/sets/paludis-deps /etc/portage/sets/

RUN emerge --update --newuse --deep -v @world \
    && emerge --update --newuse --deep -v @paludis-deps \
    && rm -rf /usr/portage/*

# Unprivileged user
RUN useradd -M builder
USER builder
