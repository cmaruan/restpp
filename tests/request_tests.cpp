
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "restpp/http.hpp"

#include <string_view>

using namespace restpp;
using namespace std::string_view_literals;

TEST_CASE("Parsing HTTP Request") {
    SUBCASE("GET without query parameters") {
        auto req_text =
                "GET /foo HTTP/1.1\r\n"
                "\r\n"sv;
        auto req = request{req_text};
        REQUIRE_EQ(req.method(), "GET");
        REQUIRE_EQ(req.path(), "/foo");
    }
    SUBCASE("GET with query parameters") {
        auto req_text =
                "GET /foo?hello=world&areYouGood=Yes HTTP/1.1\r\n"
                "\r\n"sv;
        auto req = request{req_text};
        REQUIRE_EQ(req.method(), "GET");
        REQUIRE_EQ(req.path(), "/foo");
        REQUIRE_EQ(req.full_path(), "/foo?hello=world&areYouGood=Yes");
        REQUIRE_EQ(req.GET("hello"), "world");
        REQUIRE_EQ(req.GET("areYouGood"), "Yes");
        REQUIRE_FALSE(req.GET("notAVariable"));
    }
    SUBCASE("POST without body") {
        auto req_text =
                "POST /foo HTTP/1.1\r\n"
                "\r\n"sv;
        auto req = request{req_text};
        REQUIRE_EQ(req.method(), "POST");
        REQUIRE_EQ(req.path(), "/foo");
    }

}

TEST_CASE("Parsing HTTP Request with complex path") {
    SUBCASE("GET without query parameters") {
        auto req_text =
                "GET /one/of/many/levels HTTP/1.1\r\n"
                "\r\n"sv;
        auto req = request{req_text};
        REQUIRE_EQ(req.method(), "GET");
        REQUIRE_EQ(req.pop_path(), "one");
        REQUIRE_EQ(req.pop_path(), "of");
        REQUIRE_EQ(req.pop_path(), "many");
        REQUIRE_EQ(req.pop_path(), "levels");
        REQUIRE(req.empty_path());
    }
}

TEST_CASE("Parsing HTTP Request with headers") {
    SUBCASE("GET without query parameters") {
        auto req_text =
                "GET /one/of/many/levels HTTP/1.1\r\n"
                "Host: fakesite.net\r\n"
                "Content-Length: 0\r\n"
                "\r\n"sv;
        auto req = request{req_text};
        REQUIRE_EQ(req.get_header("Host"), "fakesite.net");
        REQUIRE_EQ(req.get_header("Content-Length"), "0");
        REQUIRE_FALSE(req.get_header("Missing-Header"));
    }
}





TEST_CASE("Parsing HTTP Request with body") {
    SUBCASE("GET without headers") {
        auto req_text =
                "GET /one/of/many/levels HTTP/1.1\r\n"
                "\r\n"
                "This is the body content"sv;
        auto req = request{req_text};
        REQUIRE_EQ(req.get_body(), "This is the body content");
    }
    SUBCASE("GET with headers") {
        auto req_text =
                "GET /one/of/many/levels HTTP/1.1\r\n"
                "Host: fakesite.net\r\n"
                "Content-Length: 0\r\n"
                "\r\n"
                "This is the body content"sv;
        auto req = request{req_text};
        REQUIRE_EQ(req.get_body(), "This is the body content");
    }
}
