# GDash export #

Forked [GDash](https://bitbucket.org/czirkoszoltan/gdash/src/master/README.md) to add two new command line options for bulk export:

* Export caves as `.CrLi` files:
[Crazy Light engine format specification](http://www.gratissaugen.de/erbsen/BD-Inside-FAQ.html#CrLi-Engine)
* Flatten all caves

Previously you had to do that with the GUI for each cave, which is not very comfortable for very many caves.

### HOWTO

    $ gdash BoulderDash02.bd --save-crli -q

will generate a `.CrLi` file for each cave.

    $ gdash BoulderDash02.bd --save-flat BoulderDash02-flat.bd -q

will flatten all caves.

### WHY

I wanted to import new caves to various Boulder Dash engines and the `CrLi` file format is pretty powerful.<br>
However if you don't want to dig deep into the `CrLi` specification you can flatten the BDCFF file, which has an almost self-explaining ASCII
representation of the caves.

### What's the difference to the last official distribution? ###

* New command line option `--save-crli`
* New command line option `--save-flat`
* Improved gamepad support
    * Now all connected gamepads are supported at the same time
    * Left stick or DPAD control the player
    * You can configure your button layout with gamecontrollerdb.txt
* [Fixed](https://github.com/revvv/gdash-export-CrLi/commit/fba5a7feb71335903b70c80627ea24d4911956b8) missing images in HTML help generated with: `gdash -q --save-docs 1`
* [Fixed:](https://github.com/revvv/gdash-export-CrLi/commit/3bd063698313bd4fb86e61732ebfca262c92827e) Start editor from a running game crashed
* 64 bit
* ZIP distribution
* CrLi now also exports teleporter
* CrLi export bug [fixed](https://github.com/revvv/gdash-export-CrLi/commit/f2c9913cfdc84fc8a0e519cf547e35d6d3d70fca): Butterflies had wrong directions
* Added Arno Dash 21 (and fixed some caves in Arno Dash 6, 15, 17-21 and Future Dash)
* Added fixed version of BD4
* Default game is BD1

![Screenshot](https://raw.githubusercontent.com/revvv/gdash-export-CrLi/master/Arno_Dash-21-A.png)

