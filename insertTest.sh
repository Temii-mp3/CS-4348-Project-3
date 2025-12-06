#!/bin/bash

for ((i=1; i<=30; i++)); do
    ./project3 insert test3.idx "$i" "$((i+1))"
done
