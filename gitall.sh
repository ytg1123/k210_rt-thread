#!/bin/bash
set -e
rm -rf .git_credentials
git add .
git commit -m "Update Commit For rt-thread source  `date '+%F(%H:%M)'` "
git push -u origin --all
rm -rf .git_credentials
