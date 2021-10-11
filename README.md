# GDash export #

Forked [GDash](https://bitbucket.org/czirkoszoltan/gdash/src/master/README.md) to add a command line option to export caves as `.CrLi` file:
[Crazy Light engine format specification](http://www.gratissaugen.de/erbsen/BD-Inside-FAQ.html#CrLi-Engine)<br>
Previously you had to do that with the GUI for each cave, which is not very comfortable for a bulk export.

### Usage

    $ ./gdash BoulderDash02.bd --export

will generate a `.CrLi` file for each cave in the current directory.

### Notes

This project was only compiled on Windows and requires cygwin.


It does not require an X server. Except if you want to play or edit caves.<br>
If you also want sound and music, please get the original [distribution](https://bitbucket.org/czirkoszoltan/gdash/downloads/)
and copy the cygwin exe into this directory.
