# Chum
Example using Scintilla with gtkmm 3

This illustrates several basic Scintilla techniques from an
example application that uses gtkmm 3.

A short snippet of C++ code is loaded, the lexer is set to
C++ and the major styles are given reasonable
attributes.

Menu commands allow opening a file, performing searches,
displaying an autocompletion or calltip, and changing the
encoding between UTF-8 (default) Latin-1 and Shift-JIS.

To build, type "make" at a command prompt in the directory.
There must be a compiled copy of Scintilla in a sibling
directory named "scintilla".
