#!/usr/bin/env bash

set -e

SIMULATOR_VERSION=13.7

xcodebuild clean build -scheme "Example (Swift)" | xcpretty && exit ${PIPESTATUS[0]}
