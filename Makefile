GTK_CFLAGS=$(shell pkg-config --cflags gtk+-3.0)
GTK_LIBS=$(shell pkg-config --libs gtk+-3.0)
GTK_LIBS+=$(shell pkg-config --libs alsa)
BIN=x11-systray-volume
INSTALL_DIR=/usr/local/bin

.PHONY: default
default: build

.PHONY: help
help:
	@echo "Targets:"
	@echo "  build (default)"
	@echo "  install"
	@echo "  uninstall"
	@echo "  clean"
	@echo "  help"

.PHONY: build
build:
	clang -Wall -Wno-deprecated-declarations $(GTK_CFLAGS) $(GTK_LIBS) -o $(BIN) main.c

.PHONY: install
install:
	mv $(BIN) $(INSTALL_DIR)

.PHONY: uninstall
uninstall:
	rm -f $(INSTALL_DIR)/$(BIN)

.PHONY: clean
clean:
	rm -f $(BIN)
