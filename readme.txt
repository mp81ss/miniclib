CLibrary is just a dummy data-structure library containing vector and list only.
Both vector and list are 2 single and independent headers.

The list requires dynamic memory, mallocating the nodes, but has some tricks
to make it fast in traversal and memory efficient. Moreover it is safe: All
pointers are NULL-checked and all indexes are checked for out-of-bound errors.

vector is speed oriented, no checks are done and user must check that no NULL
pointers are passed if not explicitely allowed and indexes are in valid range.

See example.c or directly the headers (fully doxygenated), or the help file.