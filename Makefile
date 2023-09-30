GTK_CFLAGS=$(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS=$(shell pkg-config --libs gtk+-3.0)
BIN=x11-systray-volume

.PHONY: default
default: build

.PHONY: help
help:
	@echo "Targets:"
	@echo "  build (default)"
	@echo "  clean"
	@echo "  help"

.PHONY: build
build:
	clang -Wall -Wno-deprecated-declarations $(GTK_CFLAGS) $(GTK_LIBS) -o $(BIN) main.c

.PHONY: clean
clean:
	rm -f $(BIN)
