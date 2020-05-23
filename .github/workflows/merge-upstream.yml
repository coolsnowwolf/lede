#
# This is free software, lisence use MIT.
# 
# Copyright (C) 2019 KFERMercer <KFER.Mercer@gmail.com>
# 
# <https://github.com/KFERMercer/OpenWrt-CI>
#

name: Merge-upstream

on:
  push:
    branches: 
      - master
  schedule:
    - cron: 30 19 * * *

jobs:
  merge:

    runs-on: ubuntu-latest

    steps:

    - name: Checkout
      uses: actions/checkout@master
      with:
        ref: master
        fetch-depth: 0
        lfs: true

    - name: Set git identity
      run : |
        git config --global user.email "41898282+github-actions[bot]@users.noreply.github.com"
        git config --global user.name "github-actions[bot]"

    - name: Load upstream commits
      run: git pull https://github.com/coolsnowwolf/lede.git --log --no-commit

    - name: Apply commit changes
      run: |
        if [ -f ./.git/MERGE_MSG ]; then
        mkdir ./tmp && cp ./.git/MERGE_MSG ./tmp/message
        sed -i "1c [bot] AutoMerging: merge all upstream's changes:" ./tmp/message
        sed -i '/^\#.*/d' ./tmp/message
        git commit --file="./tmp/message"
        else
        echo "There is no merge commits."
        fi

    - name: Push Commits
      env:
        DOWNSTREAM_BRANCH: master
      run: git push origin $DOWNSTREAM_BRANCH
