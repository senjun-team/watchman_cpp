#!/bin/bash
set -e
rpmdev-setuptree

cp -Rp /mnt/watchman_cpp /root/rpmbuild/SOURCES

mv /root/rpmbuild/SOURCES/watchman_cpp /root/rpmbuild/SOURCES/watchman_cpp-$VERSION

cd /root/rpmbuild/SOURCES/
tar -czf watchman_cpp-$VERSION.tar.gz watchman_cpp-$VERSION
rm -rf watchman_cpp-$VERSION
cd

cp /mnt/watchman_cpp/accomodation/watchman_cpp-el9.spec /root/rpmbuild/SPECS
rpmbuild -bb /root/rpmbuild/SPECS/watchman_cpp-el9.spec

cp -Rp /root/rpmbuild/RPMS /mnt/watchman_cpp/accomodation/build_rpm
