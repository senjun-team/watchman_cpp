#include <gtest/gtest.h>

#include "common/tar/detail/header.hpp"

using namespace tar;

TEST(TarHeader, BasicConstruction) {
    detail::Header header;
    EXPECT_EQ(header.name.size(), 100u);
    EXPECT_EQ(header.mode.size(), 8u);
    EXPECT_EQ(header.uid.size(), 8u);
    EXPECT_EQ(header.gid.size(), 8u);
    EXPECT_EQ(header.size.size(), 12u);
    EXPECT_EQ(header.mtime.size(), 12u);
    EXPECT_EQ(header.checksum.size(), 8u);
    EXPECT_EQ(header.typeflag, '0');
    EXPECT_EQ(header.linkname.size(), 100u);
    EXPECT_EQ(header.magic.size(), 6u);
    EXPECT_EQ(header.version.size(), 2u);
    EXPECT_EQ(header.uname.size(), 32u);
    EXPECT_EQ(header.gname.size(), 32u);
    EXPECT_EQ(header.devmajor.size(), 8u);
    EXPECT_EQ(header.devminor.size(), 8u);
    EXPECT_EQ(header.prefix.size(), 155u);
    EXPECT_EQ(header.padding.size(), 12u);  // Adjusted size to match the actual padding size
}
