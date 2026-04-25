#!/usr/bin/env bash
set -e
cd /home/user/pwd
python .cibuildwheel-hooks/inject_stubs.py
