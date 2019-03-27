This repository contains a library and example programs for the
publicly-available API of CASTUS 4. It is intended to be useful
for third parties that wish to interoperate with a CASTUS system.

It compiles to 3 progams.
- filter-example-check-duration
- filter-example-check-duration-and-rippl
- filter-example-check-duration-and-ripple-bidi

The logic for all of these is in `utils.cpp`.  They depend on
schedule helpers in `utils_schedule.cpp`.

To create a new filter use the main from `filter-example-check-duration-and-ripple-bidi.cpp` and the logic
from `utils.cpp`.

# Building

This repository uses a standard autotools buildsystem. If you
have a release archive, simply follow the standard sequence of

```
./configure
make
make install
```

or

    ./configure
    make
    sudo make install PREFIX=/usr/local

If you are working from the git repository, you may need to run
```
./autogen.sh
```
first, in order to generate the 'configure' script (and other
files) from their templates. This requires having autoconf
and automake installed.

# Library

The sources of the library are in `src/lib/`, while the headers
are in `include/`.

# Example programs

Several example programs can be found in `src/bin`, including
tools to parse and format dates, a metadata reader, and an
example schedule filter.


