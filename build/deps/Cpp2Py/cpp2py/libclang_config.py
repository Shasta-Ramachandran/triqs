# This file is configured by cmake, it contains the libclang location and flags.
# Overruled by the environment variable if it exists.

import os
LIBCLANG_LOCATION = os.getenv('LIBCLANG_LOCATION', '/usr/local/opt/llvm/lib/libclang.dylib')
LIBCLANG_CXX_FLAGS = os.getenv('LIBCLANG_CXX_FLAGS', ' -isystem/usr/local/include -isystem/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1 -isystem/Library/Developer/CommandLineTools/usr/lib/clang/15.0.0/include -isystem/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include -isystem/Library/Developer/CommandLineTools/usr/include -isystem/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks').strip().split()

#LIBCLANG_CXX_FLAGS = ''.strip().split()

#print("LIBCLANG_CXX_FLAGS ", LIBCLANG_CXX_FLAGS, " -isystem/usr/local/include -isystem/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/c++/v1 -isystem/Library/Developer/CommandLineTools/usr/lib/clang/15.0.0/include -isystem/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include -isystem/Library/Developer/CommandLineTools/usr/include -isystem/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/System/Library/Frameworks")

# FIXME Should go when including triqs will not require Python.h any more
LIBCLANG_CXX_FLAGS  += ["-isystem/Users/ssr38/anaconda3/include/python3.11", "-isystem/Users/ssr38/anaconda3/lib/python3.11/site-packages/numpy/core/include"]

CXX_COMPILER = "/Library/Developer/CommandLineTools/usr/bin/c++"
