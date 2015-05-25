#!/bin/bash
# cppclean attempts to find problems in C++ source that slow development

srcdir=$1
builddir=$2

cppclean --verbose $srcdir \
    --exclude *.moc \
    $(for i in $(find $srcdir -type d | egrep -v "(\.git|generated)") ; do echo -e "-I $i " ; done) \
    $(for i in $(find $builddir -type d | egrep -v "(CMakeFiles|Testing)") ; do echo -e "-I $i " ; done)

