//
// Created by W504473 on 24/03/23.
//

#ifndef RESTPP_RESTPP_HPP
#define RESTPP_RESTPP_HPP

#include <string>
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>
#include <deque>

#include "http.hpp"

namespace restpp {

class matcher {
public:
    bool parse(const std::string& exp) {
        auto start = exp.find_first_of('{');
        auto end = exp.find_first_of('}');
        if (start == std::string::npos || end == std::string::npos || end <= start) {
            return false;
        }
        auto sep = exp.find(':', start);
        if (sep == std::string::npos || exp.find(':', sep + 1) != std::string::npos) {
            return false;
        }
        m_type = exp.substr(start + 1, sep - start - 1);
        m_name = exp.substr(sep + 1, end - sep - 1);
        return true;
    }
    bool match(const std::string& value) {
        if (m_type == "int") {
            try {
                m_value = std::stoi(value);
                m_raw_value = value;
                return true;
            } catch(std::exception& err) {
                return false;
            }
        } else if (m_type == "str") {
            m_value = value;
            m_raw_value = value;
            return true;
        }
        return false;
    }
    std::string name() const { return m_name; }
    std::string raw_value() const { return m_raw_value; }
private:
    std::string m_type;
    std::string m_name;
    std::string m_raw_value;
    std::variant<int, std::string> m_value;
};

class endpoint {
    using response_ptr = std::shared_ptr<response>;
    using request_ptr = std::shared_ptr<request>;
    using handler_type = std::function<response_ptr(request_ptr)>;
public:
    endpoint() = default;

    void register_method_handler(const std::string& http_method, handler_type handler) {
        m_handlers[http_method] = std::move(handler);
    }

    void add_resource_handler(
            const std::string& http_method,
            const std::string& location,
            handler_type handler) {
        matcher m;
        if (m.parse(location)) {
            auto& e = m_compiled_matches.emplace_back(m, endpoint{});
            e.second.register_method_handler(http_method, std::move(handler));
        } else {
            auto& e = m_endpoints[location];
            e.register_method_handler(http_method, std::move(handler));
        }
    }

    void add_resource(const std::string& name, const std::function<void(endpoint& e)>& callback) {
        auto& e = m_endpoints[name];
        callback(e);
    }
    std::shared_ptr<response> handle_request(std::shared_ptr<request> r) {
        if (r->empty_path()) {
            auto it = m_handlers.find(std::string{r->method()});
            if (it != m_handlers.end()) {
                return it->second(r);
            }
            auto res = std::make_shared<response>();
            res->code(http_code::http_405_not_allowed);
            return res;
        }

        auto path = std::string {r->pop_path() };
        if (auto it = m_endpoints.find(path); it != m_endpoints.end()) {
            return it->second.handle_request(r);
        }
        for (auto& [m, handler]: m_compiled_matches) {
            if (m.match(path)) {
                r->add_variable(m.name(), m.raw_value());
                return handler.handle_request(r);
            }
        }

        auto res = std::make_shared<response>();
        res->code(http_code::http_404_not_found);
        return res;
    }

private:
    std::unordered_map<std::string, endpoint> m_endpoints;
    std::vector<std::pair<matcher, endpoint>> m_compiled_matches;
    std::unordered_map<std::string, handler_type> m_handlers;
};

}

#endif //RESTPP_RESTPP_HPP
