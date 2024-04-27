# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS 
    main.c 
    HW/hardware.c
    HW/buttons.c
    HW/pot.c
    HW/display.c 
    MISC/misc.c
    APPLICATION/SdCardHandler.c
    APPLICATION/Configuration.c
    APPLICATION/ColorScheme.c  
    # list the source files of this component
    
    INCLUDE_DIRS 
    HW 
    MISC
    APPLICATION
    APPLICATION/Bitmaps
    APPLICATION/Fonts
    APPLICATION/Menus     
    # optional, add here public include directories
    
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)
