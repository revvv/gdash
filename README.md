# GDash export #

Forked [GDash](https://bitbucket.org/czirkoszoltan/gdash/src/master/README.md) to add a command line option to export caves as `.CrLi` file:
[Crazy Light engine format specification](http://www.gratissaugen.de/erbsen/BD-Inside-FAQ.html#CrLi-Engine)<br>
Previously you had to do that with the GUI for each cave, which is not very comfortable for a bulk export.

### HOWTO

    $ gdash BoulderDash02.bd --save-crli -q

will generate a `.CrLi` file for each cave.

### What's the difference to the last official distribution? ###

* New command line option `--save-crli`
* Fixed joystick support (recompile did it, cygwin required [patch](https://github.com/revvv/gdash-export-CrLi/commit/991c77465c4c0a08ffc8b56dc0cc4a0c4c6dcf19#diff-e1abb84f560b62e25bbf61530f2bf2a0e4047f0ea7ac730175da93b3916a1572))
* [Fixed](https://github.com/revvv/gdash-export-CrLi/commit/fba5a7feb71335903b70c80627ea24d4911956b8) missing images in HTML help generated with: `gdash -q --save-docs 1`
* [Fixed:](https://github.com/revvv/gdash-export-CrLi/commit/aecf55649b96386d2b5d13a46ba4568e5a3d99e0) Start editor from a running game crashed
* 64 bit
* ZIP distribution
* **New 1.2:** CrLi now also exports teleporter
* **New 1.2:** CrLi export bug [fixed](https://github.com/revvv/gdash-export-CrLi/commit/f2c9913cfdc84fc8a0e519cf547e35d6d3d70fca): Butterflies had wrong directions
* **New 1.2:** Added Arno Dash 21 (and fixed some caves in Arno Dash 6, 15, 17-21 and Future Dash)

![Screenshot](https://bitbucket.org/repo/ejzABR/images/4034423466-gdashgame.png)

### Compile with MinGW

Same as it ever was...

    $ ./configure && make
    
