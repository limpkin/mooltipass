BUNDLE ?= ./bundle.py
BUNDLE_IMAGES = $(shell ls [0-9]*.img | sort -n)

all: bundle.img

bundle.img: $(BUNDLE_IMAGES)
	$(BUNDLE) -o $@ $^

