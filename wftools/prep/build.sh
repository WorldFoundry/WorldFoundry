#!/bin/bash
# Build prep for Linux.
# This replaces the Windows Watcom toolchain and the complex GNUMakefile.tool
# system (which requires pre-built wfsource libraries).
#
# Bug fixed: unsigned delimiterIndex = token.find("=>") in macro.cc
# On 64-bit Linux, string::npos is 64-bit; truncating to unsigned caused
# the named-parameter branch to fire for every normal parameter token.
# Fixed by using std::string::size_type instead of unsigned.

set -e
cd "$(dirname "$0")"
WF_SRC="$(cd ../../wfsource/source && pwd)"

g++ -std=c++14 \
    -I"$WF_SRC" \
    -D__LINUX__ -DSW_DBSTREAM=0 \
    -O2 \
    -o prep \
    prep.cc macro.cc source.cc \
    "$WF_SRC/recolib/command.cc" \
    "$WF_SRC/recolib/infile.cc" \
    "$WF_SRC/recolib/ktstoken.cc" \
    "$WF_SRC/eval/expr_tab.cc" \
    "$WF_SRC/eval/lexyy.cc" \
    "$WF_SRC/regexp/regexp.cc" \
    "$WF_SRC/regexp/regsub.cc" \
    "$WF_SRC/regexp/regerror.cc" \
    2>&1 | grep -v "warning:"

echo "prep built successfully"
