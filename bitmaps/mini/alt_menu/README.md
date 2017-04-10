# Alternative menu graphics

Alternative menu graphics are provided to accomodate the on-device credential 
management mode (see REPLACE\_FAVORITES\_WITH\_CREDENTIAL\_MANAGEMENT in 
source_code/src/defines.h).

This mode replaces the "favorites" menu with a management menu enabling 
on-device credential creation, editing, renewal and deletion.

Files provided in this folder allow you to properly "rename" the "favorites" 
menu.

    images/*             PNG and .img files
    bundle/bundle.img    bundle image precompiled with this modified menu and additional strings

PNGs were converted with :

    convert input.png -background white -type palette -depth 1 -colors 2 output.png

