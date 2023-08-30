# GDash #

Forked [GDash](https://bitbucket.org/czirkoszoltan/gdash/src/master/README.md) to add new features:

* New command line options for bulk export (*the reason for the fork's name...*)
* Full cave view [#21](https://github.com/revvv/gdash-export-CrLi/issues/21)
* Fixed: Screen wrapped boulder does not kill player instantly [#42](https://github.com/revvv/gdash-export-CrLi/issues/42)
* Fixed replay feature (fire was not recorded) [#18](https://github.com/revvv/gdash-export-CrLi/issues/18)
* New feature *"Milling time 0 is infinite"* [#12](https://github.com/revvv/gdash-export-CrLi/issues/12)
* New player animations (gfx by [cwscws](https://github.com/cwscws)) [#4](https://github.com/revvv/gdash-export-CrLi/issues/4)
* Higher scaling factors
* Enhanced game controller support
    * Now all connected gamepads are supported at the same time
    * Left stick or DPAD control the player
    * You can configure your button layout with [`gamecontrollerdb.txt`](https://github.com/revvv/gdash-export-CrLi/blob/master/gamecontrollerdb.txt)
* Updated caves, fixed caves, added caves by [renyxadarox](https://github.com/renyxadarox), [Dustin974](https://github.com/Dustin974), [cwscws](https://github.com/cwscws)
* New [BD3 theme](https://github.com/revvv/gdash-export-CrLi/blob/master/include/c64_gfx_bd3.png) (gfx by [cwscws](https://github.com/cwscws))
* New shaders [#10](https://github.com/revvv/gdash-export-CrLi/issues/10)
* GTK fixes (*esp. for Mac: Drag-and-drop [#15](https://github.com/revvv/gdash-export-CrLi/issues/15) [#17](https://github.com/revvv/gdash-export-CrLi/issues/17), stuck key [#6](https://github.com/revvv/gdash-export-CrLi/issues/6)*)
* Test game uses GTK/SDL/OpenGL as configured [#8](https://github.com/revvv/gdash-export-CrLi/issues/6)
* 64 bit ZIP distribution for **Windows, Linux and Mac**
* CrLi now also exports teleporters
* CrLi export bug [fixed](https://github.com/revvv/gdash-export-CrLi/commit/f2c9913cfdc84fc8a0e519cf547e35d6d3d70fca): Butterflies had wrong directions
* Default game is BD1

### Bulk export

Previously you had to do that with the GUI for each cave, which is not very comfortable for very many caves.

    $ gdash BoulderDash02.bd --save-crli -q

will generate a `.CrLi` file for each cave. See [Crazy Light engine format specification](http://www.gratissaugen.de/erbsen/BD-Inside-FAQ.html#CrLi-Engine)

    $ gdash BoulderDash02.bd --save-flat BoulderDash02-flat.bd -q

will flatten all caves.

My motivation: I wanted to import new caves to various Boulder Dash engines and the `CrLi` file format is pretty powerful.<br>
However if you don't want to dig deep into the `CrLi` specification you can flatten the BDCFF file, which has an almost self-explaining ASCII
representation of the caves.

![Screenshot](https://raw.githubusercontent.com/revvv/gdash-export-CrLi/master/Arno_Dash-21-A.png)

