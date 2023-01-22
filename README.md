# Tic-Tac-Toe V1

This is a project to make a Tic-Tac-Toe game based on Arduino Nano.

## PCB

 * The PCB has been designed with [KiCAD 6.0.10](https://www.kicad.org/).
 * The KiCAD project is available in [KiCAD/](KiCAD/) directory.

<img src="images/PCB_front.png" width="20%" height="20%" />
<img src="images/PCB_back.png" width="20%" height="20%" />
<img src="images/PCB_front_with_parts.png" width="20%" height="20%" />
<img src="images/PCB_isometric_with_parts.png" width="20%" height="20%" />
 
## Arduino

(TODO: explain how to install the software on the Arduino Nano.)

## Parts

 * Omron B3W-9000-RG2C: Button with two LEDs (red and green)
   * [Product webpage at Omron](https://components.omron.com/eu-en/products/switches/B3W-9)
   * [Datasheet](https://components.omron.com/eu-en/datasheet_pdf/A167-E1.pdf)
   * [Product page at Mouser](https://www.mouser.com/ProductDetail/Omron-Electronics/B3W-9000-RG2C?qs=QTkYtwcxD6NNQk1oSF8osQ%3D%3D): includes the link to footprint.
   * [3D Model at SnapEDA](https://www.snapeda.com/parts/B3W-9000-RG2C/Omron%20Electronics/view-part/?ref=digikey)
 * Analog Devices MAX7219
   * [Product webpage at Analog Devices](https://www.analog.com/en/products/max7219.html)
   * [Datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/MAX7219-MAX7221.pdf)
   * [LedControl - An Arduino library for the MAX7219 and MAX7221](http://wayoda.github.io/LedControl/index.html)
     * This program uses this library to control MAX7219.
     * The page above also explains how to wire up LEDs to MAX7219.
