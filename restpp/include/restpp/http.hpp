//
// Created by W504473 on 25/03/23.
//

#ifndef RESTPP_HTTP_HPP
#define RESTPP_HTTP_HPP

#include <string_view>
#include <map>
#include <unordered_map>
#include <utility>
#include <vector>
#include <deque>


namespace restpp {

enum class http_code {
    http_404_not_found = 404,
    http_405_not_allowed = 405,
};

class response {
public:
    http_code code() const {
        return m_code;
    }
    void code(http_code code) { m_code = code; }

private:
    http_code m_code;
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

    [[nodiscard]] std::string_view method() const { return m_method; }
    [[nodiscard]] std::string_view path() const { return m_path; }
    [[nodiscard]] std::string_view full_path() const { return m_full_path; }
    [[nodiscard]] std::string_view get_variable(std::string_view variable) const {
        return m_vars.at(variable);
    }

    std::optional<std::string_view> GET(std::string_view variable) {
        if (auto it = m_get_variables.find(variable); it != m_get_variables.end()) {
            return it->second;
        }
        return {};
    }

    [[nodiscard]] std::optional<std::string_view> get_header(std::string_view header) const {
        if (auto it = m_headers.find(header); it != m_headers.end()) {
            return it->second;
        }
        return {};
    }

    [[nodiscard]] std::optional<std::string_view> get_body() const {
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

    [[nodiscard]] bool empty_path() const {
        return m_path_parts.empty();
    }
private:
    void parse_text(std::string_view text) {
        auto lines = split(text, "\r\n");
        auto iter = lines.cbegin();

        // Parsing first line of request: the status line
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

        // Splitting the url into its composing parts
        auto url_parts = split(m_path, "/", true);
        m_path_parts.insert(m_path_parts.begin(), url_parts.begin(), url_parts.end());

        // Parse all headers
        while (iter != lines.end() && !(++iter)->empty()) {
            size_t sep_pos = iter->find(':');
            size_t value_pos = iter->find_first_not_of(' ', sep_pos + 1);
            m_headers[iter->substr(0, sep_pos)] = iter->substr(value_pos);
        }

        // Finally, get body
        if (iter != lines.end()) {
            m_body = *++iter;
        }
    }


    static std::vector<std::string_view> split(
            std::string_view text,
            std::string_view delim,
            bool skip_empty = false)
    {
        std::vector<std::string_view> parts;
        size_t pos = 0;
        while (pos != std::string::npos) {
            size_t next_pos = text.find(delim, pos);
            std::string_view part;
            if (next_pos != std::string::npos) {
                part = text.substr(pos, next_pos - pos);
                pos = next_pos + delim.size();
            } else {
                part = text.substr(pos);
                pos = next_pos;
            }
            if (!skip_empty || !part.empty()) {
                parts.push_back(part);
            }
        }
        return parts;
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
                // TODO: url decode key and value
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

}


#endif //RESTPP_HTTP_HPP
