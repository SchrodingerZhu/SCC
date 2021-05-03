FROM archlinux:latest
RUN pacman -Syu --noconfirm qemu-headless qemu-headless-arch-extra gcc cmake ninja wget zsh && pacman -Sc --noconfirm
RUN wget http://musl.cc/mipsel-linux-musl-cross.tgz && \
    tar xvzf mipsel-linux-musl-cross.tgz && \
    rm -f mipsel-linux-musl-cross.tgz && \
    ln -sf /mipsel-linux-musl-cross/bin/* /usr/bin/
COPY . /project
WORKDIR /project
RUN cmake /project -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-flto -fdata-sections -ffunction-sections -march=native -pipe -O3" -GNinja && ninja scc
CMD /usr/bin/zsh
