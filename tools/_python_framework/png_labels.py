#!/usr/bin/env python

# pip install viivakoodi
import barcode
from barcode.writer import ImageWriter
try:
	import Image,ImageDraw, ImageFont		
except ImportError:		
	from PIL import Image, ImageDraw, ImageFont

CODE128 = barcode.get_barcode_class('code128')
FONT = "FreeSans.ttf"

options = {
  "module_width":   0.254, # 3 dots / 300 dpi * 25.4mm/in
  "module_height": 10.0,   # The height of the barcode modules in mm
  "quiet_zone":     0.0,   # Distance on the left and on the right from the border to the first (last) barcode module in mm as float. Defaults to 6.5.
  "font_size":     18  ,   # Font size of the text under the barcode in pt as integer. Defaults to 10.
  "text_distance":  0.7,   # Distance between the barcode and the text under it in mm as float. Defaults to 5.0.
}

label_sizes = {
  "17x54": (566, 165), # printable area: 47.92mm x 13.97mm (566dots x 165dots in 300dpi = 135.84dots x 39.60dots in 72dpi)
  "17x87": (956, 165), # printable area: 80.94mm x 13.97mm (956dots x 165dots in 300dpi = 229.44dots x 39.60dots in 72dpi)
  "29x90": (991, 306), # printable area: 83.90mm x 25.91mm (991dots x 306dots in 300dpi = 237.84dots x 73.44dots in 72dpi)
}

pt300 = 0.24 # (pt300 = 0.24 pt72)

def create_label_type1(label_size, barcode_value, line1=None, line2=None, line3=None):
    im = Image.new("L", label_sizes[label_size], 255)
    draw = ImageDraw.Draw(im)
    barcode_im = CODE128(barcode_value, writer=ImageWriter()).render(options)
    im.paste(barcode_im, (2, -5))
    x_off = barcode_im.size[0] + 30
    font = ImageFont.truetype(FONT, 52)
    width, height = font.getsize(line1)
    y_pos = 5
    draw.text((x_off, y_pos), line1, font=font, fill=0)
    draw.line((x_off, y_pos+height) + (x_off + width, y_pos+height), fill=0)
    font = ImageFont.truetype(FONT, 37)
    draw.text((x_off,  70), line2, font=font, fill=0)
    draw.text((x_off, 115), line3, font=font, fill=0)
    return im

def create_label_type2(label_size, text=None, font_size=11):
    im = Image.new("L", label_sizes[label_size], 255)
    draw = ImageDraw.Draw(im)
    font = ImageFont.truetype(FONT, int(font_size * 4.2))
    width, height = font.getsize(text)
    x_pos = int((im.size[0]-width)/2)
    y_pos = int((im.size[1]-height)/2-3)
    draw.text((x_pos, y_pos), text, font=font, fill=0)
    return im

if __name__ == "__main__":
    create_label_type1("29x90", "MPM-RED-54321", "Mooltipass Mini", "Color: Red", "Serial Number: 54321").save("label1_29x90_MPM-RED-54321.png")
    create_label_type1("17x87", "MPM-RED-54321", "Mooltipass Mini", "Color: Red", "Serial Number: 54321").save("label1_17x87_MPM-RED-54321.png")
    create_label_type2("17x87", "MPM-RED-12345").save("label2_17x87_MPM-V1-8MB-RED-12345.png")
    create_label_type2("17x54", "MPM-RED-12345").save("label2_17x54_MPM-V1-8MB-RED-12345.png")

