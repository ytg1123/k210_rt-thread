#!/bin/bash
set -e
git add .
git commit -m "Update Commit For rt-thread source  `date '+%F(%H:%M)'` "
git push -u origin --all
