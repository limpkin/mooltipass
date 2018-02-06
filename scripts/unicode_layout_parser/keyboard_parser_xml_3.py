from lxml import objectify
from lxml import etree
import numpy


class Unicode_XML_Parser():
    def __init__(self, xml_file):
        self.xml_file = xml_file
        file_str = open(xml_file, 'r', encoding="utf8")
        #file_str = file_str.read()
##        tree = etree.parse(xml_file)
##        root = tree.getroot()
        self.keyboard = objectify.parse(file_str)
        file_str = etree.tostring(self.keyboard, pretty_print=True)
        self.keyboard = objectify.fromstring(file_str)

    def get_unicode_list(self):
        # Unicode List
        unicode_list = []
        
        # Retrieve Modifier + Key Combinations:
        for keyMap in self.keyboard.keyMap:
            print("------- %s ------- " % (keyMap.get("modifiers")))
            print("%-8s%-8s%-8s"%("ISO","symbol","unicode"))
            for key in keyMap.map:
                key_pos = key.get("iso")
                key_sym = key.get("to")
                if "\\u{" in key_sym:
                    key_sym = key_sym.replace("\\u{", "")
                    key_sym = key_sym.replace("}", "")
                    key_sym = chr(int(key_sym,16)) 
                try:
                    key_hex = ord(key_sym)
                    unicode_list.append(key_hex)
                    print("%-8s%-8s%-8s"%(key_pos, key_sym, hex(key_hex)))
                except:
                    print("error: ", key_sym)

        # Retrieve Key + Key Sequences:
        try:
            for transforms in self.keyboard.transforms:
                print("------- %s ------- " % (transforms.get("type")))
                print("%-8s%-8s%-8s%-8s"%("FROM","symbol","SEQUENCE=","unicode"))
                for key in transforms.transform:
                    key_pos = key.get("from")
                    key_sym = key.get("to")
                    if "\\u{" in key_sym:
                        key_sym = key_sym.replace("\\u{", "")
                        key_sym = key_sym.replace("}", "")
                        key_sym = chr(int(key_sym,16))
                    try:
                        key_hex = ord(key_sym)
                        unicode_list.append(key_hex)
                        print("%-8s%-8s%-8s"%(key_pos, key_sym, hex(key_hex)))
                    except:
                        print("error: ", key_sym)
        except:
            print("[ERR] No transforms found")

        return unicode_list

path = ".\\cldr-keyboards-32\\keyboards\\windows\\"
kbd = Unicode_XML_Parser(path+"ar-t-k0-windows.xml")

# Generate Histogram from unicode_list
hist, bin_edges = numpy.histogram(kbd.get_unicode_list(), bins=range(255))

# Plot Histogram
import matplotlib.pyplot as plt
plt.bar(bin_edges[:-1], hist, width = 1)
plt.xlim(min(bin_edges), max(bin_edges))
plt.show()


