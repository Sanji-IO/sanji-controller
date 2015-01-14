#!/bin/bash
set -e

ARCH=${1:-x86}
TAG=latest

if [ $ARCH=="armhf" ]; then
	TAG=armhf
	cp Dockerfile Dockerfile.tmp
	sed -e "s|debian:stable|mazzolino/armhf-debian|" Dockerfile.tmp > Dockerfile
fi

(docker build --tag=sanji/docker-controller:$TAG .) || true

if [ $ARCH=="armhf" ]; then
	mv Dockerfile.tmp Dockerfile
fi