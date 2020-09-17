#!/usr/bin/env bash

set -e

SIMULATOR_VERSION=13.7

xcodebuild test -scheme "MapboxDirections iOS" -destination "platform=iOS Simulator,name=iPhone 8,OS=${SIMULATOR_VERSION}" -testLanguage fr -testRegion fr_FR | xcpretty -r junit --output test-reports/TEST-MBDirections.xml && exit ${PIPESTATUS[0]}
