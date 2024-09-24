#include <gtest/gtest.h>
#include "common/tar_to_stream.hpp"

using namespace tar;

TEST(TarHeader, BasicConstruction) {
    TarHeader header;
    EXPECT_EQ(header.name.size(), 100);
    EXPECT_EQ(header.mode.size(), 8);
    EXPECT_EQ(header.uid.size(), 8);
    EXPECT_EQ(header.gid.size(), 8);
    EXPECT_EQ(header.size.size(), 12);
    EXPECT_EQ(header.mtime.size(), 12);
    EXPECT_EQ(header.checksum.size(), 8);
    EXPECT_EQ(header.typeflag, '0');
    EXPECT_EQ(header.linkname.size(), 100);
    EXPECT_EQ(header.magic.size(), 6);
    EXPECT_EQ(header.version.size(), 2);
    EXPECT_EQ(header.uname.size(), 32);
    EXPECT_EQ(header.gname.size(), 32);
    EXPECT_EQ(header.devmajor.size(), 8);
    EXPECT_EQ(header.devminor.size(), 8);
    EXPECT_EQ(header.prefix.size(), 155);
    EXPECT_EQ(header.padding.size(), 12); // Adjusted size to match the actual padding size
}
