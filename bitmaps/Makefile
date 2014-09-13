BUNDLE ?= ./bundle.py
BITMAP ?= ./bitmap.py

BITMAP_PNG = $(wildcard [0-9]*.png)
BITMAP_IMAGES = $(BITMAP_PNG:.png=.img)

FONT_IMAGES = $(wildcard [0-9]*font*.img)

BUNDLE_IMAGES = $(BITMAP_IMAGES) $(FONT_IMAGES)

all: bundle.img

%.img: %.png
	$(BITMAP) -i $< -o $@ -c -n $(@:.img=)

bundle.img: $(BUNDLE_IMAGES)
	$(BUNDLE) -s fw_strings.txt -o $@ $^

clean:
	rm -f bundle.img

