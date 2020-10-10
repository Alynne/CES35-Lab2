#include "gtest/gtest.h"
#include "http/url.h"

TEST(HttpTests, basicUrl) {
    std::string source = "https://google.com";
    auto url = http::url::parse(source);

    ASSERT_TRUE(url.has_value());
    EXPECT_EQ(url->getScheme(), "https");
    EXPECT_EQ(url->getHost(), "google.com");
    EXPECT_EQ(url->getPath(), "");
    EXPECT_EQ(url->port, 0);
}

TEST(HttpTests, urlWithPort) {
    std::string source = "https://google.com:443";
    auto url = http::url::parse(source);

    ASSERT_TRUE(url.has_value());
    EXPECT_EQ(url->getScheme(), "https");
    EXPECT_EQ(url->getHost(), "google.com");
    EXPECT_EQ(url->getPath(), "");
    EXPECT_EQ(url->port, 443);
}

TEST(HttpTests, urlWithPath) {
    std::string source = "https://google.com/index.html";
    auto url = http::url::parse(source);

    ASSERT_TRUE(url.has_value());
    EXPECT_EQ(url->getScheme(), "https");
    EXPECT_EQ(url->getHost(), "google.com");
    EXPECT_EQ(url->getPath(), "index.html");
    EXPECT_EQ(url->port, 0);
}

TEST(HttpTests, urlWithPortAndPath) {
    std::string source = "https://google.com:443/index.html";
    auto url = http::url::parse(source);

    ASSERT_TRUE(url.has_value());
    EXPECT_EQ(url->getScheme(), "https");
    EXPECT_EQ(url->getHost(), "google.com");
    EXPECT_EQ(url->port, 443);
    EXPECT_EQ(url->getPath(), "index.html");
}
