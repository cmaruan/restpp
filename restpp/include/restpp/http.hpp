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
    http_100_continue = 100,
    http_101_switching_protocols = 101,
    http_102_processing = 102,
    http_103_early_hints = 103,
    http_200_ok = 200,
    http_201_created = 201,
    http_202_accepted = 202,
    http_203_non_authoritative_information = 203,
    http_204_no_content = 204,
    http_205_reset_content = 205,
    http_206_partial_content = 206,
    http_207_multi_status = 207,
    http_208_already_reported = 208,
    http_226_im_used = 226,
    http_300_multiple_choices = 300,
    http_301_moved_permanently = 301,
    http_302_found = 302,
    http_303_see_other = 303,
    http_304_not_modified = 304,
    http_307_temporary_redirect = 307,
    http_308_permanent_redirect = 308,
    http_400_bad_request = 400,
    http_401_unauthorized = 401,
    http_402_payment_required = 402,
    http_403_forbidden = 403,
    http_404_not_found = 404,
    http_405_method_not_allowed = 405,
    http_406_not_acceptable = 406,
    http_407_proxy_authentication_required = 407,
    http_408_request_timeout = 408,
    http_409_conflict = 409,
    http_410_gone = 410,
    http_411_length_required = 411,
    http_412_precondition_failed = 412,
    http_413_content_too_large = 413,
    http_414_uri_too_long = 414,
    http_415_unsupported_media_type = 415,
    http_416_range_not_satisfiable = 416,
    http_417_expectation_failed = 417,
    http_418_I_am_a_teapot = 418,
    http_421_misdirected_request = 421,
    http_422_unprocessable_content = 422,
    http_423_locked = 423,
    http_424_failed_dependency = 424,
    http_425_too_early = 425,
    http_426_upgrade_required = 426,
    http_428_precondition_required = 428,
    http_429_too_many_requests = 429,
    http_431_request_header_fields_too_large = 431,
    http_451_unavailable_for_legal_reasons = 451,
    http_500_internal_server_error = 500,
    http_501_not_implemented = 501,
    http_502_bad_gateway = 502,
    http_503_service_unavailable = 503,
    http_504_gateway_timeout = 504,
    http_505_http_version_not_supported = 505,
    http_506_variant_also_negotiates = 506,
    http_507_insufficient_storage = 507,
    http_508_loop_detected = 508,
    http_510_not_extended = 510,
    http_511_network_authentication_required = 511,

};

class response {
public:
    response() = default;
    response(http_code code, std::string text)
            : m_code{code}, m_text{std::move(text)} { }

    explicit response(std::string text) : response(http_code::http_200_ok, std::move(text)) {}

    [[nodiscard]] http_code code() const {
        return m_code;
    }
    void code(http_code code) { m_code = code; }

    [[nodiscard]] const std::string& text() const { return m_text; }
    void text(std::string text) { m_text = std::move(text); }

private:
    http_code m_code;
    std::string m_text;
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
    [[nodiscard]] std::string_view get_argument(std::string_view variable) const {
        return m_arguments.at(variable);
    }

    void set_argument(const std::string& name, const std::string& value) {
        m_arguments[name] = value;
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
    std::unordered_map<std::string_view, std::string_view> m_arguments;
    std::unordered_map<std::string_view, std::string_view> m_get_variables;
    std::unordered_map<std::string_view, std::string_view> m_headers;
};

}


#endif //RESTPP_HTTP_HPP
