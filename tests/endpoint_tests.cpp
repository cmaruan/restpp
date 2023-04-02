//
// Created by W504473 on 25/03/23.
//
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <string_view>
#include <doctest/doctest.h>
#include <magic_enum.hpp>
#include <nlohmann/json.hpp>

#include "restpp/http.hpp"
#include "restpp/restpp.hpp"


using namespace restpp;
using namespace std::string_view_literals;



class Resource {
public:

    static std::shared_ptr<response> ok() {
        return std::make_shared<response>("Okay");
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
//
//
//#include <typeinfo>
//#include <typeindex>
//
//template <typename A, typename B>
//bool same_decay_type() {
//    return std::is_same_v<std::decay_t<A>, B>;
//}
//
//class base_field {
//public:
//    base_field() = delete;
//    base_field(std::type_index type): m_type { type } {}
//    virtual ~base_field() = default;
//
//    void set_name(const std::string& name) {
//        m_name = name;
//    }
//
//    std::string get_type() const { return m_type.name(); }
//
//    const std::string& get_name() const {
//        return m_name;
//    }
//
//    template <class Type>
//    void set_value(Type value) {
//        if (std::type_index(typeid(std::decay_t<Type>)) == m_type) {
//            m_value = value;
//        }
//        else {
//            throw std::invalid_argument {"Types do not match"};
//        }
//    }
//
//    std::any get_value() const {
//        return m_value;
//    }
//
//    template <class Type, class = std::enable_if<std::is_enum_v<Type>, void>>
//    Type as_enum() {
//        return magic_enum::enum_cast<Type>(get_value());
//    }
//
//private:
//    std::string m_name;
//    std::any m_value;
//    std::type_index m_type;
//};
//
//template <typename Type>
//class field : public base_field {
//public:
//    using D = std::decay_t<Type>;
//    using base_type = std::conditional_t<std::is_enum_v<D>, std::string, D>;
//    field() : base_field(std::type_index(typeid(base_type))) {}
//};
//
//enum class CardStatus {
//    Active = 1,
//    Inactive = 2,
//    Deleted = 3,
//    OnHold = 4,
//    OnFraudHold = 5,
//};
//
//
//template <typename DTO>
//class model {
//    using field_ref = std::reference_wrapper<base_field>;
//public:
//
//    void register_field(base_field& field, std::string field_name) {
//        field.set_name(field_name);
//        m_fields.emplace_back(field);
//    }
//
//    std::vector<field_ref> get_fields() const {
//        return m_fields;
//    }
//
//    void load_json(const std::string &json_string) {
//        auto json = nlohmann::json::parse(json_string);
//        for (auto& field_reference: m_fields) {
//            auto& field = field_reference.get();
//            if (!json.contains(field.get_name())) {
//                continue;
//            }
//            auto& json_field = json[field.get_name()];
//            switch (json_field.type()) {
//                case nlohmann::json::value_t::number_float:
//                    field.template set_value<double>(json_field);
//                    break;
//                case nlohmann::json::value_t::number_integer:
//                case nlohmann::json::value_t::number_unsigned:
//                    field.template set_value<long>(json_field);
//                    break;
//                case nlohmann::json::value_t::string:
//                    field.template set_value<std::string>(json_field);
//                    break;
//                case nlohmann::json::value_t::null:
//                    // field.template set_value<double>(json_field);
//                    // field.clear_value();
//                    break;
//                case nlohmann::json::value_t::boolean:
//                    field.template set_value<bool>(json_field);
//                    break;
//                default:
//                    break;
//            }
//        }
//    }
//
//private:
//    std::vector<field_ref> m_fields;
//};
//
//class CardDTO : public model<CardDTO> {
//    using model_dto = model<CardDTO>;
//public:
//    field<std::string> card_num;
//    field<long> card_id;
//    field<CardStatus> card_status;
//
//    CardDTO()
//    {
//        register_fields();
//    }
//
//    void register_fields() {
//        model_dto::register_field(card_num, "card_num");
//        model_dto::register_field(card_id, "card_id");
//        model_dto::register_field(card_status, "card_status");
//    }
//};
//
//#include <cxxabi.h>
//const std::string demangle(const char* name) {
//    int status = -4;
//    char* res = abi::__cxa_demangle(name, NULL, NULL, &status);
//    const char* const demangled_name = (status==0)?res:name;
//    std::string ret_val(demangled_name);
//    free(res);
//    return ret_val;
//}
//
//TEST_CASE("CardDTO") {
//    CardDTO card;
//    card.load_json(R"({"card_num": "123456", "card_id": 10000001, "card_status": "Active"})");
//    for (const auto& field: card.get_fields()) {
//        std::cout << field.get().get_name() << ": " << field.get().get_value<>() << std::endl;
//    }
//}




