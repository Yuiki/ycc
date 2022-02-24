#!/bin/sh
docker run --rm -v $(pwd):/cb -w /cb compilerbook make test
