#!/bin/bash
# launch_backend.sh — 在干净环境中启动 C++ 后端
# 清除 conda 对动态链接器的影响
unset CONDA_PREFIX CONDA_DEFAULT_ENV CONDA_SHLVL CONDA_EXE
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu:/usr/local/lib
exec "$@"
