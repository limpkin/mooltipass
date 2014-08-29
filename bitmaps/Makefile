BUNDLE ?= ./bundle.py
BITMAP ?= ./bitmap.py

BITMAP_HEADERS = $(wildcard [0-9]*.h)
BITMAP_IMAGES = $(BITMAP_HEADERS:.h=.img)

FONT_IMAGES = $(wildcard [0-9]*font*.img)

BUNDLE_IMAGES = $(BITMAP_IMAGES) $(FONT_IMAGES)

all: bundle.img

%.img: %.h
	$(BITMAP) -i $< -o $@ -c -n $(@:.img=)

bundle.img: $(BUNDLE_IMAGES)
	$(BUNDLE) -s fw_strings.txt -o $@ $^

clean:
	rm -f bundle.img

