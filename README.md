# GDash #

Forked [GDash](https://bitbucket.org/czirkoszoltan/gdash/src/master/README.md) to add a command line option to export caves in `.CrLi` (Crazy Light engine) format. Previously you had to do that with the GUI for each cave, which is not very comfortable for a bulk export.

### Usage

    $ ./gdash BoulderDash02.bd --export

will generate a `.CrLi file` for each cave in the current directory.

### Notes

This project was only compiled on Windows and requires cygwin.


It does not require an X server, indeed this is untested.
