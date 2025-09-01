# fsv
A C++ program that creates a lightweight class similar to std::string_view but with the addition of a predicate that allows filtering of the base string.

A `string_view` can be thought of as a “view” into an existing character buffer.  
Specifically, a `string_view` consists of only a pointer and a length, identifying a section of character data that is not owned by the `string_view` and cannot be modified by the view.