#!/bin/sh

if [ -d .git ]; then
    cat >AUTHORS <<ENDAUTHORS
# Generated from git logs by autogen.sh
# Format: name <email>

ENDAUTHORS
    git log --pretty=format:'%aN <%aE>' | sort -u - >>AUTHORS
fi

touch NEWS ChangeLog

echo mkdir -p m4
mkdir -p m4
echo aclocal --warnings=all --install -I m4
aclocal --warnings=all --install -I m4
echo libtoolize --verbose --install --copy --automake
libtoolize --verbose --install --copy --automake
echo autoconf --warnings=all -I m4
autoconf --warnings=all -I m4
echo autoheader --warnings=all
autoheader --warnings=all
echo automake --warnings=all --copy --add-missing --foreign
automake --warnings=all --copy --add-missing --foreign

