This repository contains a library and example programs for the
publicly-available API of CASTUS 4. It is intended to be useful
for third parties that wish to interoperate with a CASTUS system.

# Building

This repository uses a standard autotools buildsystem. If you
have a release archive, simply follow the standard sequence of

```
./configure
make
make install
```
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
