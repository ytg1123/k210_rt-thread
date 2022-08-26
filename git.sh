#!/bin/bash
set -e
git init
git remote add origin git@github.com:ytg1123/k210_rt-thread.git
git push -u origin master
git add .
git commit -m "Initial Commit For rt-thread for k210 source  2021.09.09 "
git push -u origin --all
