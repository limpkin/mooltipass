from lxml import objectify
from lxml import etree
import matplotlib.pyplot as plt
import numpy
import re

class Unicode_XML_Parser():
    def __init__(self, xml_file):
        self.xml_file = xml_file
        self.keyboard = objectify.fromstring(open(xml_file, "r").read())
        self.unicode_list = []
        self.language = ""
    
    def get_unicode_list(self):
        # Unicode List
        self.unicode_list = []
        try:
            for name in self.keyboard.names:
                #print ""
                #print name.name.get("value")
                self.language = name.name.get("value")
                pass
        except:
            pass
        
        # Retrieve Modifier + Key Combinations:
        for keyMap in self.keyboard.keyMap:
            #print "------- %s ------- " % (keyMap.get("modifiers"))
            #print "%-8s%-8s%-8s"%("ISO","symbol","unicode")
            for key in keyMap.map:
                key_pos = key.get("iso")
                key_sym = key.get("to")
                if "\\u{" in key_sym:
                    regexp = r"{(?P<digit>[^\\u{*}]+)}"
                    m = re.search(regexp, key_sym)
                    match = m.groups()
                    key_sym = [unichr(int(x,16)) for x in match] 
                try:
                    key_hex = [ord(x) for x in key_sym]
                    self.unicode_list.extend(key_hex)
                    #print "%-8s%-8s%-8s"%(key_pos, key_sym, [hex(x) for x in key_hex] )
                except:
                    print "error: ", key_sym, len(key_sym)

        # Retrieve Key + Key Sequences:
        try:
            for transforms in self.keyboard.transforms:
                #print "------- %s ------- " % (transforms.get("type"))
                #print "%-8s%-8s%-8s%-8s"%("FROM","symbol","SEQUENCE=","unicode")
                for key in transforms.transform:
                    key_pos = key.get("from")
                    key_sym = key.get("to")
                    if "\\u{" in key_sym:
                        regexp = r"{(?P<digit>[^\\u{*}]+)}"
                        m = re.search(regexp, key_sym)
                        match = m.groups()
                        key_sym = [unichr(int(x,16)) for x in match] 
                    try:
                        key_hex = [ord(x) for x in key_sym]
                        self.unicode_list.extend(key_hex)
                        #print "%-8s%-8s%-8s"%(key_pos, key_sym, [hex(x) for x in key_hex] )
                    except:
                        print "error: ", key_sym, len(key_sym)
        except:
            # print "[ERR] No transforms found"
            pass

        self.unicode_list.sort()
        self.unicode_list = set(self.unicode_list)
        return list(self.unicode_list)

    def get_modifier_list(self):
        modifier_list = []
        for keyMap in self.keyboard.keyMap:
            #print "------- %s ------- " % (keyMap.get("modifiers"))
            modifier_list.append(keyMap.get("modifiers"))
        return modifier_list

    def get_continuous_ranges(self):
        from operator import itemgetter
        from itertools import groupby
        data = self.unicode_list
        ranges = []
        for k, g in groupby(enumerate(data), lambda (i,x):i-x):
            group = map(itemgetter(1), g)
            ranges.append((group[0], group[-1]))
        return ranges


def plot_histogram(data=[], title="", xlabel="", ylabel="", filename=""):
    plt.figure()
    plt.hist(data, 50)
    plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    if(filename!=""):
        plt.savefig("out/"+filename, bbox_inches='tight')
    plt.close()

#path = "./cldr-keyboards-32/keyboards/windows/es-t-k0-windows.xml"
path = "./cldr-keyboards-32/keyboards/windows/*.xml"
import glob

# List all xml files under path directory
file_list = glob.glob(path)

unicode_list = []
for f in file_list:
    kbd = Unicode_XML_Parser(f)
    u_list = kbd.get_unicode_list()
    print "file: %s, unicode max: 0x%04x size: %d" % (f, max(u_list), len(u_list))
    #print u_list
    #print kbd.get_continuous_ranges()
    unicode_list.append(u_list)
    #print unicode_list
    title = "%s Histogram" % (kbd.language)
    fname = kbd.language.lower().replace(" ","")+".png"
    plot_histogram(u_list, title,"unicode","frequency",fname)

plot_histogram(unicode_list, "All Histogram","unicode","frequency","all.png")
"""
# Plot Histogram
plot_histogram = True
if(plot_histogram == True):
    # Generate Histogram from unicode_list
    #hist, bin_edges = numpy.histogram(unicode_list, bins=range(0, 0x1FF, 0xFF))
    import matplotlib.pyplot as plt
    plt.hist(unicode_list, 50)
    plt.title("%s Histogram" % (kbd.language))
    plt.xlabel("Unicode Value")
    plt.ylabel("Frequency")
    plt.savefig("out/%s.png"%(kbd.language.lower().replace(" ", "")), bbox_inches="tight")
    #plt.show()
"""
"""
# List of Modifiers
modifier_list = []
for f in file_list:
    kbd = Unicode_XML_Parser(f)
    for modifier in kbd.get_modifier_list():
        if( modifier not in modifier_list):
            modifier_list.append(modifier)
print modifier_list
"""
