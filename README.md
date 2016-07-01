LinIMAmt
========
This is a quick-and-dirty GUI for GNU Mtools made in Qt. It is dedicated to opening, browsing, extracting and composing 
floppy disk image files. It is a project under slow development, and **most functions are not done yet**. When
it will be done, it will be shared with other programs.

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

Todo
=========
Most important things to do (except these in "MEMENTO" and "TODO" sections in source code):
 * Drag-drop
 * New image option
 * ARTWORK! (toolbar should look more like a GUI)
 * Refactoring code. This window class with everything thrown into it doesn't look good.
 * Attribute editing, maybe messing with metadata
 * Do something with directory structure visualization code as it's quite bad now.
 * Change the name to something which can be pronounced (after I've made HGCFEU and FTDU it should be more English in name). The current name is directly 
  taken from "Linux", "Image" and "Mtools".


Links
=======

 * https://www.gnu.org/software/mtools/ - GNU MTools home page
 * http://mcbx.netne.net - page with my software. Server may be periodically down.
