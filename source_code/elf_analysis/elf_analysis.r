Rebol []
    
draw_block: []
temp_block: []
maxfuncsize: 0
linecount: 0
infilename: %data_usage.txt
pngfilename: %data_usage.png

bold14: make face/font [style: 'bold size: 14]
infile: read/lines infilename
foreach line infile [
    temp: parse line none
    if (uppercase second temp) = "T" [
        append temp_block third temp
        append temp_block to-integer first temp
        append temp_block second temp
        maxfuncsize: max maxfuncsize to-integer first temp
    ]
]
append draw_block reduce [ 
        'backcolor white
        'across
        'space 0x0
]
foreach [ name size type ] temp_block [
    temp: size / maxfuncsize * 800
    append draw_block reduce [ 
        'label 800x20 'effect  
        reduce [
            'draw reduce compose [
                'fill-pen (either odd? linecount [ 38.58.108.190 ][ 38.58.108.220 ])
                'box 0x0 799x19            
                'pen 'none
                'fill-pen 'linear 0x0 0 14 90 1 1 0.255.0.130 0.255.0.30 0.180.0.0
                'box 0x1 (as-pair temp 18)
                'pen (black) 
                'font 'bold14 'anti-aliased
                'text 5x0 (to-string size)
                'text 60x0 (type)
                'text 80x0 (name)
            ]
        ]  
        'return
    ]
    
    linecount: linecount + 1
    
]
save/png pngfilename to-image layout draw_block