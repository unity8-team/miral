#!/bin/sh
c++filt | sed 's/+ \(miral[^@]*[^[:space:]]*\)\(.*\)/+ (c++)"\1"\2/'
