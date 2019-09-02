# Introduction

Native node library of computationally heavy functions that will be run in libuv worker threads.

This was designed as part of Vortex but there is no technical limitation that would limit it to
that scenario.

# Usage

Call the functions, there is nothing special to take care of. However you may want to change the
environment variable "UV_THREADPOOL_SIZE" (before running node!) which limits the number of
parallel threads and thus how well the cpu will get utilized.

# Implemented Functions

- fileMD5 (calculates md5 hash for a file)

# Supported OS

Should be platform independent but was tested only on Windows atm.

