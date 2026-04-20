#!/bin/bash

case "$1" in
"" | "release")
	docker build \
		--output=out \
		-t z-gauges-esp .
	;;
"debug")
	cargo build
	;;
*)
	echo "Wrong argument. Only \"debug\"/\"release\" arguments are supported"
	exit 1
	;;
esac
