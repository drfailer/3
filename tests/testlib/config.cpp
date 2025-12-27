#include "config.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <cassert>

#define debug(...) fprintf(stderr, __VA_ARGS__)

struct ParserValue {
    std::string key;
    std::vector<std::string> values;
};

enum class ParserStatus {
    Ok,
    Error,
    ParsingValueList,
};

struct ParserState {
    char *buf;
    size_t size;
    size_t idx;
    ParserStatus status;
    ParserValue value;
};

bool is_number(char c) { return '0' <= c && c <= '9'; }

bool is_triple_quote(ParserState const &state) {
    return (state.idx + 3) < state.size && strncmp(state.buf, "\"\"\"", 3) == 0;
}

size_t buf_size(char const *buf, size_t max_size) {
    size_t i = 0;
    while (i < max_size && buf[i] != '\n' && buf[i] != 0) {
        ++i;
    }
    return i;
}

void value_append(ParserState *state, std::string const &value) {
    if (state->value.values.empty()) {
        state->value.values.push_back(value);
    } else {
        state->value.values.back().append(value);
    }
}

ParserState skip_spaces(ParserState const &state) {
    ParserState result = state;
    while (result.idx < result.size && result.buf[result.idx] == ' ') {
        ++result.idx;
    }
    return result;
}

ParserState parse_char(ParserState const &state, char c) {
    ParserState result = skip_spaces(state);
    if (result.buf[result.idx] != c) {
        result.status = ParserStatus::Error;
        return result;
    }
    ++result.idx;
    result.status = ParserStatus::Ok;
    return result;
}

ParserState parse_key(ParserState const &state) {
    ParserState result = skip_spaces(state);

    while (result.idx < result.size && result.buf[result.idx] != ' ' &&
           result.buf[result.idx] != '=') {
        result.value.key.push_back(result.buf[result.idx++]);
    }
    result.status = ParserStatus::Ok;
    return result;
}

ParserState parse_value(ParserState const &state);

ParserState parse_list(ParserState const &state) {
    ParserState result = skip_spaces(state);

    if (result.status != ParserStatus::ParsingValueList) {
        result = parse_char(state, '[');
        if (result.status != ParserStatus::Ok) {
            return result;
        }
        result.status = ParserStatus::ParsingValueList;
    }

    while (result.idx < result.size) {
        ParserState new_state = parse_char(result, ']');
        if (new_state.status == ParserStatus::Ok) {
            result.idx = new_state.idx;
            result.status = ParserStatus::Ok;
            return result;
        }
        new_state = parse_value(result);
        if (new_state.status != ParserStatus::Ok) {
            result.status = ParserStatus::Error;
            return  result;
        }
        result.value.values.push_back(new_state.value.values[0]);
        result.idx = new_state.idx;
        new_state = parse_char(result, ',');
        if (new_state.status == ParserStatus::Ok) {
            result.idx = new_state.idx;
        }
    }
    return result;
}

ParserState parse_bool(ParserState const &state) {
    ParserState result = skip_spaces(state);

    if (strncmp(result.buf + result.idx, "true", 4) == 0) {
        result.value.values.push_back("true");
    } else if (strncmp(result.buf + result.idx, "false", 5) == 0) {
        result.value.values.push_back("false");
    } else {
        result.status = ParserStatus::Error;
    }
    return result;
}

/*
 * For now we don't need float, so they are not supported yet.
 */
ParserState parse_number(ParserState const &state) {
    ParserState result = skip_spaces(state);

    if (!is_number(result.buf[result.idx])) {
        result.status = ParserStatus::Error;
        return result;
    }

    std::string value;
    while (is_number(result.buf[result.idx])) {
        value.push_back(result.buf[result.idx++]);
    }
    value_append(&result, value);
    result.status = ParserStatus::Ok;
    return result;
}

ParserState parse_string(ParserState const &state) {
    ParserState result = skip_spaces(state);

    if (result.buf[result.idx] != '"') {
        result.status = ParserStatus::Error;
        return result;
    }
    ++result.idx;

    std::string value;
    while (result.idx < result.size) {
        if (result.buf[result.idx] == '"' &&
            result.buf[result.idx - 1] != '\\') {
            value_append(&result, value);
            ++result.idx;
            result.status = ParserStatus::Ok;
            return result;
        }
        value.push_back(result.buf[result.idx]);
        ++result.idx;
    }
    result.status = ParserStatus::Error;
    return result;
}

ParserState parse_value(ParserState const &state) {
    ParserState result = state;

    result = parse_bool(state);
    if (result.status != ParserStatus::Error) {
        return result;
    }
    result = parse_number(state);
    if (result.status != ParserStatus::Error) {
        return result;
    }
    result = parse_string(state);
    if (result.status != ParserStatus::Error) {
        return result;
    }
    result = parse_list(state);
    if (result.status != ParserStatus::Error) {
        return result;
    }
    result.status = ParserStatus::Error;
    return result;
}

ParserState parse_key_value(ParserState const &state) {
    ParserState result;

    // continue parsing list if necessary
    if (state.status == ParserStatus::ParsingValueList) {
        return parse_list(state);
    }
    result = parse_key(state);
    if (result.status != ParserStatus::Ok) {
        return result;
    }
    result = parse_char(result, '=');
    if (result.status != ParserStatus::Ok) {
        return result;
    }
    return parse_value(result);
}

ParserState parse_line(ParserState const &state) {
    return parse_key_value(state);
}

void update_config(TestConfig *config, ParserValue const &value) {
    if (value.key.empty())
        return;
    if (value.key == "title") {
        config->title = value.values[0];
    } else if (value.key == "category") {
        config->category = value.values[0];
    } else if (value.key == "description") {
        config->description = value.values[0];
    } else if (value.key == "dir") {
        config->files.dir = value.values[0];
    } else if (value.key == "src") {
        config->files.src = value.values[0];
    } else if (value.key == "exit_code") {
        config->results.exit_code = atoi(value.values[0].c_str());
    } else if (value.key == "should_compile") {
        config->results.should_compile = true;
        if (value.values[0] == "false") {
            config->results.should_compile = false;
        }
    } else if (value.key == "should_run") {
        config->results.should_run = true;
        if (value.values[0] == "false") {
            config->results.should_run = false;
        }
    } else if (value.key == "flags") {
        config->compiler.flags = value.values;
    } else if (value.key == "ldflags") {
        config->compiler.ldflags = value.values;
    } else if (value.key == "platforms") {
        config->compiler.platforms = value.values;
    } else {
        fprintf(stderr, "error: unknown config key '%s'\n", value.key.c_str());
    }
}

TestConfig parse_config_file(std::string const &filename) {
    size_t line = 0;
    TestConfig config;
    std::ifstream file(filename);
    char buf[1024];
    ParserState state;
    state.buf = buf;

    assert(file.is_open());

    state.status = ParserStatus::Ok;
    while (!file.eof()) {
        ++line;
        file.getline(state.buf, 1024);
        if (state.buf[0] == 0 || state.buf[0] == '#') {
            continue;
        }
        state.idx = 0;
        state.size = buf_size(state.buf, 1024);
        state = parse_line(state);

        if (state.status == ParserStatus::Ok) {
            update_config(&config, state.value);
            state.value = {};
        } else if (state.status == ParserStatus::Error) {
            fprintf(
                stderr,
                "%s(%ld:%ld) error: test config parser returned an error.\n",
                filename.c_str(), line, state.idx);
            exit(1);
        }
    }

    return config;
}

std::vector<std::filesystem::path> get_config_files(std::string const &config_dir) {
    std::vector<std::filesystem::path> result;

    for (auto entry : std::filesystem::recursive_directory_iterator(config_dir)) {
        if (entry.is_directory()) {
            continue;
        }
        std::filesystem::path path(entry);
        if (path.extension() == CONFIG_EXTENSION) {
            result.push_back(entry);
        }
    }
    return result;
}
