
// case for mooltipass prototype
// v1
//
// Author: Darran Hunt


m3_diameter	= 3.2;
m3_nut_diameter = 5.5/cos(30)+0.4;
m3_nut_height   = 3.0;
m3_cap_diameter	= 5.7;
m3_cap_height	= 3.2;

radius = 10.2/2;
bevel = 12.2/2;

pcb_len = 88.6;
pcb_width = 41.2;
pcb_thickness = 2.1;
pcb_component_height = 3.0;
card_slot_height = 6.5;
card_slot_length = 60.0;

oled_width = 28.5;
oled_length = 88.0;
oled_offset = 1.5;
oled_thickness = 2.2;
oled_flex_clearance = 3.0;

usb_width = 13.0;
usb_height = 9.0; // 6.5mm above PCB

isp_len = 12.6;
isp_width = 7.5;
isp_xoff = 13.0;
isp_yoff = 5.0;

support_width = 5.0;
support_len = 5.0;

wall = 2.2;
cover = 2;
length = pcb_len + wall*2;
width = pcb_width + oled_flex_clearance + 2*wall+10;
height = card_slot_height+pcb_thickness+pcb_component_height+oled_thickness+2*cover;

oled_height =  cover+card_slot_height+pcb_thickness+pcb_component_height;

top_height = cover+card_slot_height+pcb_thickness+pcb_component_height+oled_thickness;
top_thickness = 6.7; // height-top_height;

case(top=true, bottom=false);

module case(top, bottom)
{

    rotate([top && !bottom ? 180:0,0,0])
    difference() {
	union() {
	    top();
	    bottom();

	    // screw blocks
	    translate([2, width-11, cover]) cube([11,9,height-cover*2]);
	    translate([length-11-2, width-11, cover]) cube([11,9,height-cover*2]);

	}

	// screw holes
	translate([8, width-7, -0.01]) {
	    cylinder(r=m3_nut_diameter/2, h=m3_nut_height, $fn=6);
	    translate([0, 0, 0.01+m3_nut_height+0.3])
		cylinder(r=m3_diameter/2, h=height-m3_cap_height-m3_nut_height-0.5, $fn=30);
	    translate([0, 0, 0.01+height-m3_cap_height])
		cylinder(r=m3_cap_diameter/2, h=m3_cap_height+0.01, $fn=30);
	}
	translate([length-8, width-7, -0.01]) {
	    cylinder(r=m3_nut_diameter/2, h=m3_nut_height, $fn=6);
	    #translate([0, 0, 0.01+m3_nut_height+0.3])
		cylinder(r=m3_diameter/2, h=height-m3_cap_height-m3_nut_height-0.5, $fn=30);
	    translate([0, 0, 0.01+height-m3_cap_height])
		cylinder(r=m3_cap_diameter/2, h=m3_cap_height+0.01, $fn=30);
	}


	// remove bottom
	if (!bottom) {
	    translate([-0.01,-0.01,-0.01]) cube([length+0.02,width+0.02,11]);
	}

	// remove top
	if (!top) {
	    translate([-0.01,-0.01,11-0.01]) cube([length+0.02,width+0.02,11]);
	}
    }
}

// PCB
*#translate([wall,wall+oled_flex_clearance,cover+pcb_component_height])
    cube([pcb_len, pcb_width, pcb_thickness]);

// PCB centre line
*%translate([wall,wall+oled_flex_clearance+pcb_width/2-1,cover+pcb_component_height])
    cube([pcb_len, 2, pcb_thickness*2]);


*difference() {
    translate([2, 2, height-top_thickness]) 
	top();

    // oled panel cutout
    translate([wall-0.5, wall+oled_flex_clearance, cover+card_slot_height+pcb_thickness+pcb_component_height-4])
	cube([oled_length+1, oled_width, oled_thickness+4]);
}


// oled
*%translate([wall, wall+oled_flex_clearance, cover+card_slot_height+pcb_thickness+pcb_component_height])
    cube([oled_length, oled_width, oled_thickness]);

module top() 
{
    difference() {
	translate([2, 2, height-top_thickness]) 
	    difference() {
		cube([length-4, width-4, top_thickness]);
		//rounded_box(length-4, width-4, height-top_height, 2);
		// oled cutout
		translate([3, oled_flex_clearance+4, -0.02])
		    cube([oled_length-6, oled_width-5, top_thickness+0.04]);
		// support cutouts
		translate([10, -1, -4]) 
		    cube([length - 4 - 2*10, 9, 7]);
		translate([9, width-28, -4]) 
		    cube([length - 4 - 2*9, 25, 7]);

		// oled screen
		*#translate([0,oled_flex_clearance, height - oled_height - top_thickness])
		    cube([oled_length, oled_width, oled_thickness]);

	    }
	translate([wall-0.5, wall+oled_flex_clearance, cover+card_slot_height+pcb_thickness+pcb_component_height-4])
	    cube([oled_length+1, oled_width, oled_thickness+4]);
    }

    // connector inserts
}

module bottom() {
    difference() {
	rounded_box(length, width, height, 4);
	translate([2,2,2])
	cube([length-4, width-4, height]);
	//rounded_box(length-4, width-4, height, 2);

	translate([0,oled_flex_clearance,0]) {
	    translate([length, wall+pcb_width/2, cover])
		usb();

	    // ISP access
	    translate([length-wall-isp_xoff, wall+isp_yoff,-1])
		cube([isp_len, isp_width+1, 4]);

	    // pcb cavity
	    translate([wall,wall,cover])
		cube([pcb_len, pcb_width, height-cover*2]);
	}

	//card_slot_height = 6.5;
	//card_slot_length = 63.0;
	// card slot
	translate([length/2,width-wall+1,cover+pcb_component_height+pcb_thickness+card_slot_height/2])
	    cube([card_slot_length, 4, 3], center=true);
    }

    // card support
    #translate([length/2-card_slot_length/2, 0.2+wall+oled_flex_clearance+pcb_width, cover])
        cube([card_slot_length, 0.1+width-pcb_width-oled_flex_clearance-wall*2, pcb_component_height+pcb_thickness+card_slot_height/2-1.5]);

    // pcb supports
    translate([0,oled_flex_clearance,0]) 
	for (i = [[0,0,0], [0,1,0], [1,1,0], [1,0,0]]) {
	    translate([2,2,cover] + [i[0]*(pcb_len-support_len+(wall-2)*2), i[1]*(pcb_width-support_width+(wall-2)*2), 0])
		cube([support_len, support_width, pcb_component_height]);
	}
    translate([2,2,cover])
        cube([10,oled_flex_clearance,pcb_component_height+pcb_thickness+4]);
    translate([length-2-10,2,cover])
        cube([10,oled_flex_clearance,pcb_component_height+pcb_thickness+4]);
    
}

module usb() 
{
    translate([-5,-usb_width/2,0])
	cube([6, usb_width, usb_height]);
}


module rounded_box(width, length, height, radius)
{
    union() {
        translate([radius,0,0])
	    cube([width-radius*2,length,height]);
        translate([0,radius,0])
	    cube([width,length-radius*2,height]);

	// rounded corners
	for (i = [[radius, radius, 0],
		  [width-radius, radius, 0],
		  [width-radius, length-radius, 0],
		  [radius, length-radius, 0]]) {
            translate(i) cylinder(h=height, r=radius, $fn=30);
	 }
    }
}
