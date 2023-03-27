//
// Created by W504473 on 25/03/23.
//
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "restpp/http.hpp"
#include "restpp/restpp.hpp"

#include <string_view>

using namespace restpp;
using namespace std::string_view_literals;



class Resource {
public:

    static std::shared_ptr<response> ok() {
        return std::make_shared<response>(http_code::http_200_ok);
    }
    static std::shared_ptr<response> handler1(std::shared_ptr<request> r) {
        method_called = "handler1";
        return ok();
    }

    static std::shared_ptr<response> handler2(std::shared_ptr<request> r) {
        method_called = "handler2";
        return ok();
    }

    static std::shared_ptr<response> handler_with_typed_args(std::shared_ptr<request> r) {
        method_called = "handler_with_typed_args";
        argument = r->get_argument("cool_id");
        return ok();

    }
    static std::shared_ptr<response> handler_with_non_typed_args(std::shared_ptr<request> r) {
        method_called = "handler_with_non_typed_args";
        argument = r->get_argument("cool_id");
        return ok();
    }

    static void reset() {
        method_called.clear();
        argument.clear();
    }

    static void config(endpoint& e) {
        e.add_resource_handler("GET", "{cool_id:int}", Resource::handler_with_typed_args);
        e.add_resource_handler("GET", "{cool_id}", Resource::handler_with_non_typed_args);
        e.add_resource_handler("GET", "handler2", Resource::handler2);
        e.add_resource_handler("GET", "handler1", Resource::handler1);
    }

    static inline std::string method_called{};
    static inline std::string argument{};

};

TEST_CASE("Correct resource is executed") {
    endpoint e;
    e.add_resource<Resource>("resource");
    Resource::reset();
    auto req = std::make_shared<request>(
            "GET /resource/handler1 HTTP/1.1\r\n"
            "\r\n"sv
    );
    auto res = e.handle_request(req);
    REQUIRE_EQ(res->code(), http_code::http_200_ok);
    REQUIRE_EQ(Resource::method_called, "handler1");
}

TEST_CASE("Method not allowed return HTTP 405") {
    endpoint e;
    e.add_resource<Resource>("resource");

    Resource::reset();
    auto req = std::make_shared<request>(
            "POST /resource/handler1 HTTP/1.1\r\n"
            "\r\n"
            "Body"sv
    );
    auto res = e.handle_request(req);
    REQUIRE_EQ(res->code(), http_code::http_405_method_not_allowed);
}

TEST_CASE("Handler with typed arguments") {
    endpoint e;
    e.add_resource<Resource>("resource");

    Resource::reset();
    auto req = std::make_shared<request>(
            "GET /resource/343434 HTTP/1.1\r\n"
            "\r\n"sv
    );
    auto res = e.handle_request(req);
    REQUIRE_EQ(res->code(), http_code::http_200_ok);
    REQUIRE_EQ(Resource::method_called, "handler_with_typed_args");
    REQUIRE_EQ(Resource::argument, "343434");
}

TEST_CASE("Handler with non-typed arguments") {
    endpoint e;
    e.add_resource<Resource>("resource");

    Resource::reset();
    auto req = std::make_shared<request>(
            "GET /resource/notATypedValue HTTP/1.1\r\n"
            "\r\n"sv
    );
    auto res = e.handle_request(req);
    REQUIRE_EQ(res->code(), http_code::http_200_ok);
    REQUIRE_EQ(Resource::method_called, "handler_with_non_typed_args");
    REQUIRE_EQ(Resource::argument, "notATypedValue");
}