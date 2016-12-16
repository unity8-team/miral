#!/bin/sh
c++filt | sed 's/+ \([miral|typeinfo|vtable][^@]*[^[:space:]]*\)\(.*\)/+ (c++)"\1"\2/'
