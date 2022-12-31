# -*- Makefile -*-

DEBUG ?= false

ifndef DEBUG
DEBUG = true
endif

ifeq ($(DEBUG), true)
BUILD_OPT = --debug
endif

CXXARCH = -arch x86_64

PYTHON ?= python3

SETUP = $(PYTHON) -m setup

ENVPARAM = STDCXX="$(STDCXX)" ARCHFLAGS="$(CXXARCH)" DEBUG=$(DEBUG)


.PHONY: all build clean test

all:
	@echo "Usage make (build|clean)"

build:
	env $(ENVPARAM) $(SETUP) build $(BUILD_OPT)

install:
	env $(ENVPARAM) $(SETUP) install --user --old-and-unmanageable

clean:
	$(SETUP) clean -a
	rm -rf build bit_count.egg-info __pycache__
	rm -f *.pyc *~
