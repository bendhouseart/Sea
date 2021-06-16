#!/usr/bin/bash

echo "Clearing cache" && sync && echo 3 | sudo tee /proc/sys/vm/drop_caches
