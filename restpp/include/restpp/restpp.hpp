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

namespace restpp {

    class matcher {
    public:
        bool parse(const std::string& exp) {
            auto start = exp.find_first_of('{');
            auto end = exp.find_first_of('}');
            if (start == std::string::npos || end == std::string::npos || end <= start) {
                std::cout << "Either start, end is npos or end is less that start" << std::endl;
                return false;
            }
            auto sep = exp.find(':', start);
            if (sep == std::string::npos || exp.find(':', sep + 1) != std::string::npos) {
                std::cout << "Cannot find separator ':' or too many of them" << std::endl;
                return false;
            }
            m_type = exp.substr(start + 1, sep - start - 1);
            m_name = exp.substr(sep + 1, end - sep - 1);
            std::cout << "Type: " << m_type << "; Name: " << m_name << std::endl;
            return true;
        }
        bool match(const std::string& value) {
            std::cout << "Trying to match " << value << " as " << m_type << std::endl;
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


    class response {
    public:
    };

    class request {
    public:
        explicit request(const std::string &text) {
            m_raw_text = text;
            parse_text(m_raw_text);
        }

        explicit request(std::string_view text) {
            m_raw_text = std::string{text};
            parse_text(m_raw_text);
        }

        std::string_view method() const { return m_method; }
        std::string_view path() const { return m_path; }
        std::string_view full_path() const { return m_full_path; }
        std::string_view get_variable(std::string_view variable) const {
            return m_vars.at(variable);
        }

        std::optional<std::string_view> GET(std::string_view variable) {
            if (auto it = m_get_variables.find(variable); it != m_get_variables.end()) {
                return it->second;
            }
            return {};
        }

        std::optional<std::string_view> get_header(std::string_view header) const {
            if (auto it = m_headers.find(header); it != m_headers.end()) {
                return it->second;
            }
            return {};
        }

        std::optional<std::string_view> get_body() const {
            if (!m_body.empty())
                return m_body;
            return {};
        }

        std::string_view pop_path() {
            auto part = m_path_parts.front();
            m_path_parts.pop_front();
            return part;
        }

        void add_variable(const std::string& name, const std::string& value) {
            m_vars[name] = value;
        }

        bool empty_path() const {
            return m_path_parts.empty();
        }
    private:
        void parse_text(std::string_view text) {
            auto lines = split(text, "\r\n");
            auto iter = lines.cbegin();
            const auto& status = *iter;
            size_t pos = status.find(' ');
            if (pos != std::string::npos) {
                m_method = status.substr(0, pos);
                size_t start = pos + 1;
                pos = status.find(' ', start);
                if (pos != std::string::npos) {
                    m_full_path = status.substr(start, pos - start);
                }
                size_t query_params_start = m_full_path.find('?');
                if (query_params_start != std::string::npos) {
                    parse_encoded_vars(m_full_path.substr(query_params_start + 1));
                    m_path = m_full_path.substr(0, query_params_start);
                } else {
                    m_path = m_full_path;
                }
            }

            auto url_parts = split(m_path, "/", true);
            m_path_parts.insert(m_path_parts.begin(), url_parts.begin(), url_parts.end());

            while (iter != lines.end() && !(++iter)->empty()) {
                size_t sep_pos = iter->find(':');
                size_t value_pos = iter->find_first_not_of(' ', sep_pos + 1);
                m_headers[iter->substr(0, sep_pos)] = iter->substr(value_pos);
            }
            if (iter != lines.end()) {
                m_body = *++iter;
            }
        }


        std::vector<std::string_view> split(std::string_view text, std::string_view delim, bool skip_empty = false) {
            std::vector<std::string_view> subpaths;
            size_t pos = 0;
            while (true) {
                size_t next_pos = text.find(delim, pos);
                std::string_view subpath;
                if (next_pos != std::string::npos) {
                    subpath = text.substr(pos, next_pos - pos);
                } else {
                    subpath = text.substr(pos);
                }
                if (!(skip_empty && subpath.empty())) {
                    subpaths.push_back(subpath);
                }
                if (next_pos == std::string::npos) {
                    break;
                }
                pos = next_pos + delim.size();
            }
            return subpaths;
        }

        void parse_encoded_vars(std::string_view vars) {
            size_t pos = 0;
            while (pos != std::string::npos) {
                size_t end_pos = vars.find('&', pos);
                std::string_view param;
                if (end_pos != std::string::npos) {
                    param = vars.substr(pos, end_pos - pos);
                    pos = end_pos + 1;
                } else {
                    param = vars.substr(pos);
                    pos = end_pos;
                }
                size_t equals_pos = param.find('=');
                if (equals_pos != std::string::npos) {
                    std::string_view key = param.substr(0, equals_pos);
                    std::string_view value = param.substr(equals_pos + 1);
                    // url decode key and value
                    m_get_variables[key] = value;
                }
            }
        }


        std::string_view m_body;
        std::string_view m_method;
        std::string_view m_path;
        std::string_view m_full_path;
        std::string m_raw_text;
        std::deque<std::string_view> m_path_parts;
        std::unordered_map<std::string_view, std::string_view> m_vars;
        std::unordered_map<std::string_view, std::string_view> m_get_variables;
        std::unordered_map<std::string_view, std::string_view> m_headers;
    };


//    class endpoint {
//    public:
//        endpoint() = default;
//        explicit endpoint(std::function<response(request)> handler) : m_handler {std::move(handler)} {}
//
//        void add_resource_handler(const std::string& http_method, const std::string& location, std::function<response(request)> func) {
//            matcher m;
//            if (m.parse(location)) {
//                std::cout << "Registering compiled handler: " << http_method << " at " << location << std::endl;
//                m_compiled_matches.emplace_back(m, endpoint{std::move(func)});
//            } else {
//                std::cout << "Registering normal handler: " << http_method << " at " << location << std::endl;
//            }
//        }
//
//        void add_resource(const std::string& name, const std::function<void(endpoint& e)>& callback) {
//            std::cout << "Registering resource: " << name << std::endl;
//            auto& e = m_endpoints[name];
//            callback(e);
//        }
//
//        response handle_request(request r) {
//            if (r.empty_path()) {
//                std::cout << "Empty path...\n";
//                return m_handler(r);
//            }
//
//            auto path = r.pop_path();
//            std::cout << "Got path: " << path << "\n";
//            if (auto it = m_endpoints.find(path); it != m_endpoints.end()) {
//                std::cout << "Found endpoint handler\n";
//                return it->second.handle_request(r);
//            }
//            for (auto& [m, handler]: m_compiled_matches) {
//                if (m.match(path)) {
//                    std::cout << "Found compiled match\n";
//                    r.add_variable(m.name(), m.raw_value());
//                    return handler.handle_request(r);
//                }
//            }
//
//            std::cout << "Found nothing that matches\n";
//            return {};
//        }
//
//    private:
//        std::unordered_map<std::string, endpoint> m_endpoints;
//        std::vector<std::pair<matcher, endpoint>> m_compiled_matches;
//        std::function<response(request)> m_handler;
//    };
//
//
//    class server {
//    public:
//
//    };
}

#endif //RESTPP_RESTPP_HPP
