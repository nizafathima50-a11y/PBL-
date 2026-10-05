#!/bin/bash
echo "Starting Logger..."
./logger &
sleep 1
echo "Starting Core..."
./core &
sleep 1
echo "Starting UI..."
./ui
