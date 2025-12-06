#!/bin/bash

for ((i=1; i<=30; i++)); do
    python3 project3.py insert test.idx "$i" "$((i+1))"
done
