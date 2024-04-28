# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.

idf_component_register(
    SRCS 
    main.c 
    HW/hardware.c
    HW/buttons.c
    HW/pot.c
    HW/display.c
    HW/backlight.c
    HW/buzzer.c
    HW/systimer.c
    HW/random.c 
    HW/spi_handler.c
    MISC/misc.c
    APPLICATION/SdCardHandler.c
    APPLICATION/Configuration.c
    APPLICATION/ColorScheme.c
    APPLICATION/Scheduler.c  
    APPLICATION/LcdWriter.c
    APPLICATION/BitmapHandler.c
    APPLICATION/MessageBox.c
    APPLICATION/PowerHourMain.c
    APPLICATION/SpecialTasks.c
    APPLICATION/Bitmaps/Bitmaps.c
    APPLICATION/Bitmaps/Beershot.c
    APPLICATION/Bitmaps/laudur_lapik.c
    APPLICATION/Menus/menu.c
    APPLICATION/Menus/Bargraph.c
    APPLICATION/Fonts/Arial12.c
    APPLICATION/Fonts/Arial14Bold.c
    APPLICATION/Fonts/Arial16Bold.c
    APPLICATION/Fonts/ClockFont.c
    APPLICATION/Fonts/Fonts.c
    APPLICATION/Fonts/Arial24Bold.c
    APPLICATION/Bitmaps/beer1.c
    APPLICATION/Bitmaps/beer2.c
    APPLICATION/Bitmaps/beer3.c
    APPLICATION/Bitmaps/beer4.c
    APPLICATION/Bitmaps/beer5.c
    APPLICATION/Bitmaps/beer6.c
    APPLICATION/Bitmaps/beer7.c
    APPLICATION/Bitmaps/beer8.c
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
