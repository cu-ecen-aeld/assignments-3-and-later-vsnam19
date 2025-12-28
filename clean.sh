#!/bin/bash
# Script to clean buildroot directory
# Author: AESD

cd `dirname $0`
make -C buildroot distclean
