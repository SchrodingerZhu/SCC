FROM            archlinux
RUN             patched_glibc=glibc-linux4-2.33-4-x86_64.pkg.tar.zst && \
                   curl -LO "https://repo.archlinuxcn.org/x86_64/$patched_glibc" && \
                   bsdtar -C / -xvf "$patched_glibc"
RUN             pacman -Syu --noconfirm graphviz cmake ninja gcc --overwrite '*' &&\
                pacman -Sc --noconfirm
COPY            ./ /project
WORKDIR         /project
RUN             cmake . -DCMAKE_BUILD_TYPE=Release -GNinja && ninja

