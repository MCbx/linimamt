LinIMAmt
========

![Screenshot](https://raw.githubusercontent.com/MCbx/linimamt/master/sshot.png)

This is a quick-and-dirty GUI for GNU Mtools made in Qt. It is dedicated for opening, browsing, extracting and composing
floppy disk image files. It is a project under slow development, and the simple functionality has been achieved allowing
to work with disk images. When it will be done, it will be shared with other programs.

The program is made the way that people using Windows disk imaging software (e.g. WinImage) should be familiar with
the UI and the main goal is to make Windows users who switched to Linux manage disk images for their old PCs.
The biggest problem with Linux tools is: We have a command-line Mtools or we can mount the image and let file
manager destroy metadata. This program should protect the image by having a temporary file-based "Open-Save" strategy, 
like e.g. a typical text editor.
However, there will be significant differences. There will be no "Read image" or "Write image" command
as making floppy disk images in Linux is terrible. The idea is to add few "Execute user routine" statements
to make user execute the favourite disk imaging tool.


License
=====================
GPL. Needs GPL Mtools.

Working
===========
Which means: Now working, and probably has to be fixed on some edge cases:
* Creating, opening and saving modified image file,
* Changing disk label and serial number
* Browsing image file structure
* Extracting files/directories, with partial drag and drop
* Adding files/directories, creating directories
* Renaming
* Deleting
* Setting attributes
* Editing and transplanting boot sectors
* Hard disk image support for e.g. BOCHS ones with partition selection.
* "Run Testdisk on image" 

Parameters
===============
* -new - open new file window
* [filename] - open file
* [filename] -d - open file in direct mode
* [filename] -r - open file in read-only mode

Todo
=========
Most important things to do:
 * Drag-drop should work better
 * Image formats conversion, defragmenting
 * Maybe messing with metadata
 * Refactoring code. This window class with everything thrown into it doesn't look good.
 * Do something with directory structure visualization code as it's quite bad now.
 * Change the name to something which can be pronounced (after I've made HGCFEU and FTDU it should be more English in name). The current name is directly taken from "Linux", "Image" and "Mtools".
 * Windows port. It is developed under Linux. There is a mtools for Windows and TestDisk for Windows.

Links
=======

 * https://www.gnu.org/software/mtools/ - GNU MTools home page
 * https://www.gnu.org/software/mtools/manual/mtools.html - GNU Mtools manual
 * http://oldcomputer.info/software/ - page with my software. Server may be periodically down.
 * http://reboot.pro/files/file/267-mtools/ - Mtools Windows port
