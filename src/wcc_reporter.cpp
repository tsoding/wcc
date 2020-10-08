#include "./wcc_reporter.hpp"

size_t Reporter::offset_from_input(const char *current_position) const
{
    assert(input.data <= current_position);
    return static_cast<size_t>(current_position - input.data);
}

Reporter::Location Reporter::offset_to_location(size_t offset) const
{
    String_View input0 = input;
    for (size_t line_number = 1; input0.count > 0; ++line_number) {
        String_View line = input0.chop_by_delim('\n');

        if (offset <= line.count) {
            return {line_number, offset + 1};
        }

        offset -= line.count + 1;
    }

    assert(0 && "I'm not sure when exactly this could happen. Please investigate when triggered.");
    return {};
}
