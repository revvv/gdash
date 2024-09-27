# GDash #

[GDash](https://bitbucket.org/czirkoszoltan/gdash/src/master/README.md) is a feature-rich Boulder Dash clone.
The main goal of the project is to implement a clone which is as close as possible to the original.
GDash has a cave editor, supports sound, joystick and keyboard controls.
It can use GTK+, SDL2 and OpenGL for drawing.
The OpenGL engine can use shaders, which provide fullscreen graphical effects like TV screen emulation.
It supports replays, snapshots and has highscore tables.

This fork adds some new features:

* New command line options for bulk export (*the reason for the fork's name...*)
* After completing a cave you can skip the time countdown with F (fast) or ESC [#50](https://github.com/revvv/gdash-export-CrLi/issues/50)<br>
  Very useful if your test cave has time 999.
* Show complete cave without scrolling [#21](https://github.com/revvv/gdash-export-CrLi/issues/21) [#59](https://github.com/revvv/gdash-export-CrLi/issues/59)
* You can now activate the OpenGL renderer for super smooth scrolling [#25](https://github.com/revvv/gdash-export-CrLi/issues/25)
* Improved snapshot feature for Twitch [#23](https://github.com/revvv/gdash-export-CrLi/issues/23)
* Show all elements in element statistics [#31](https://github.com/revvv/gdash-export-CrLi/issues/31)
* New command line argument `--help-localized`
* Fixed replay feature (fire was not recorded) [#18](https://github.com/revvv/gdash-export-CrLi/issues/18)
* New feature *"Milling time 0 is infinite"* [#12](https://github.com/revvv/gdash-export-CrLi/issues/12)
* New player animations (gfx by [cwscws](https://github.com/cwscws)) [#4](https://github.com/revvv/gdash-export-CrLi/issues/4)
* Higher scaling factors and autoscale
* Enhanced game controller support
    * Now all connected gamepads are supported at the same time
    * Left stick or DPAD control the player
    * You can configure your button layout with [`gamecontrollerdb.txt`](https://github.com/revvv/gdash-export-CrLi/blob/master/gamecontrollerdb.txt)
* Updated caves, fixed caves, added caves by [renyxadarox](https://github.com/renyxadarox), [Dustin974](https://github.com/Dustin974), [cwscws](https://github.com/cwscws)
* New [BD3 theme](https://github.com/revvv/gdash-export-CrLi/blob/master/include/c64_gfx_bd3.png) (gfx by [cwscws](https://github.com/cwscws))
* New shaders [#10](https://github.com/revvv/gdash-export-CrLi/issues/10)
* GTK+ fixes (*esp. for Mac: Drag-and-drop [#15](https://github.com/revvv/gdash-export-CrLi/issues/15) [#17](https://github.com/revvv/gdash-export-CrLi/issues/17) [cave list](https://github.com/revvv/gdash-export-CrLi/commit/1c528dc19f3d7377c5c9f201e04a4d2790be35cb), stuck key [#6](https://github.com/revvv/gdash-export-CrLi/issues/6), frozen Window [#57](https://github.com/revvv/gdash-export-CrLi/issues/57)*)
* Full screen enhancements [#29](https://github.com/revvv/gdash-export-CrLi/issues/29) [#61](https://github.com/revvv/gdash-export-CrLi/issues/61)
* Test game uses GTK+/SDL/OpenGL as configured [#8](https://github.com/revvv/gdash-export-CrLi/issues/8)
* 64 bit ZIP distribution for **Windows, Linux and Mac**
* CrLi now also exports teleporters
* CrLi export bug [fixed](https://github.com/revvv/gdash-export-CrLi/commit/f2c9913cfdc84fc8a0e519cf547e35d6d3d70fca): Butterflies had wrong directions
* Default game is BD1

### FAQ
- Q: Why is there no console output for `gdash --help` on Windows?<br>
  A: You can redirect the output to a file:<br>
    `$ gdash --help > gdash.log 2>&1`

- Q: On Mac/Linux executing `gdash` seems not to work?<br>
  A: Always use the shell script instead:<br>
    `$ ./start-gdash-mac.command`<br>
    `$ ./start-gdash-linux.sh`
- Q: What changes to the project are not obvious?<br>
  A: `make install` is not maintained. It may work, but GDash expects all caves in the installation folder and not in `/share/locale`.
- Q: On Mac some keys seem not to work?<br>
  A: Mac default shortcuts collide with some keys.
  
    | Key       | GDash      | Mac                                      | Recommendation                                                                  |
    |-----------|------------|------------------------------------------|---------------------------------------------------------------------------------|
    | CTRL      | Snap       | Change desktop: _CTRL+Left/Right-Cursor_ | Configure another _snap key_ in GDash (press K to configure)                    |
    | F11       | Fullscreen | Show desktop                             | Disable F11 in _System Preferences -> Keyboard -> Shortcuts -> Mission Control_ |
- Q: Why are caves sometimes in `.bd` or `.gds` or both formats?<br>
  A: `.gds` is a binary import from the C64/Atari. `.bd` is the new BDCFF format with many new features.
     However not all elements the 8-Bit community used are yet identified. So it could make sense to keep both until these elements are supported.
     Unknown elements are simply imported as _steel wall_. If you want to play the caves, always prefer the .bd version.
- Q: I have the feeling that a butterfly moves in the wrong direction?<br>
  A: There was a fix added in GDash-export 1.2. Beginning with this version you should be able to import/export caves
     from Crazy Light Construction Kit preserving the correct direction.
     Unfortunately some 8-Bit caves were manually created by binary editing with wrong bufferfly directions.
     You can try to import them with version GDash-export 1.1. Usually these caves start with binary header _GDashCRL_.
     See [#40](https://github.com/revvv/gdash-export-CrLi/issues/40)
- Q: Does GDash for cygwin support gamepads?<br>
  A: Yes, but make sure you have the latest version: SDL2-2.28.4-1a (2023-10-06)<br>
     Please check if dinput and xinput gamepads work in GTK+ and SDL mode. Right now all combinations work fine!

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

