include ./cmake/container.mk
include ./cmake/build.mk

.DEFAULT_GOAL := help

.PHONY: help

help: container-help build-help
	@:
