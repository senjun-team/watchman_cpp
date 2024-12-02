FROM quay.io/centos/centos:stream9

RUN yum upgrade -y \
    && yum install -y -q gcc g++ bash diffutils \
    && yum install -y -q rpmdevtools rpmlint rpm-build redhat-rpm-config \
    && yum install -y -q epel-release \
    && yum install -y -q git wget vim vim-enhanced vim-common sudo python3 make cmake \
    && yum install -y -q boost-devel zlib-devel curl-devel

# env variable for spec file. watchman_cpp release version
ENV VERSION="0.3.2"

ENTRYPOINT sh /mnt/watchman_cpp/accomodation/build_rpm/build_rpm.sh
