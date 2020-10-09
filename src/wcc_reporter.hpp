#ifndef WCC_REPORTER_HPP_
#define WCC_REPORTER_HPP_

struct Reporter
{
    String_View input;
    String_View filename;

    struct Location
    {
        size_t row;
        size_t column;
    };

    template <typename... Args>
    void inform(size_t offset, Args... args) const
    {
        auto location = offset_to_location(offset);
        println(stderr, filename, ":", location.row, ":", location.column, ": ", args...);
    }

    template <typename... Args>
    void warn(size_t offset, Args... args) const
    {
        inform(offset, "warning: ", args...);
    }

    template <typename... Args>
    [[noreturn]] void fail(size_t offset, Args... args) const
    {
        inform(offset, "error: ", args...);
        exit(1);
    }

    size_t offset_from_input(const char *current_position) const;
    Location offset_to_location(size_t offset) const;
};

#endif  // WCC_REPORTER_HPP_
