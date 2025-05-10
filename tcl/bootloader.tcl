setws ./workspace/bootloader
app create -name cottonos_bootloader -hw ./dist/design_1_wrapper.xsa -os standalone -proc ps7_cortexa9_0 -template {Empty Application(C)}

bsp setlib  xilffs 
platform generate