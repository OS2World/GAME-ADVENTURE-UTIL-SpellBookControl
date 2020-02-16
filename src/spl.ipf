:userdoc.
:title.spl documentation
:h1.Introduction and Copyright
:p.
:lines align=center.
:font facename='Tms Rmn' size=36x24.
:hp4.
Spellbook Control Program
:font facename=default size=0x0.
:ehp4.

:hp1.spl.exe:ehp1.

:hp2.Version 1.0:ehp2.

:hp2.8/10/93:ehp2.

:hp2.Copyright (C) 1993 John-Marc Chandonia:ehp2.

:elines.
:p.
For suggested enhancements and bug reports, please mail
:font facename=Courier size=13x8.
chandoni@husc8.harvard.edu
:font facename=default size=0x0.
or write:

:lines align=center.
John-Marc Chandonia
7 Divinity Avenue, #121
Cambridge, Ma. 02138
:elines.
:p.
This is spl, a program for handling spells and spellbooks
from various fantasy role-playing (FRP) games.  It can handle
both mage and priest spells for TSR's AD&amp.D (a trademark of TSR, Inc)
game, and can easily be expanded to handle other systems as well.
In addition to possibly being useful to FRP players and
GM's, the source may be useful to OS/2 2.x programmers learning
about containers, MLE's, and PM programming in general.
:p.
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
:p.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
:p.
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
:p.
:h1.Release Notes
:p.
This is the first public release of the program, and therefore
probably has many more bugs than I know about!  Known bugs are
listed in the file 
:font facename=Courier size=13x8.
bugs.txt.
:font facename=default size=0x0.
If you find any more, please either fix them yourself and send
me the patch (sure!) or report the bug to the address given
above.
:p.
I will put all future versions of this program on the OS/2
anonymous ftp site, currently
:font facename=Courier size=13x8.
ftp-os2.cdrom.com.
:font facename=default size=0x0.
If you distribute any enhanced versions of this program, please
do the same.  Also, please comment any changes in the source
code, and describe them in the file 
:font facename=Courier size=13x8.
changes.txt.
:font facename=default size=0x0.

:h1.Using the program
This section describes how to install and use the program.

:h2.Installation
Unzip the zip file in some directory.  If you want, create a program
object for the file
:font facename=Courier size=13x8.
spl.exe,
:font facename=default size=0x0.
If you don't plan to look at the source code, you can delete the
src directory... this is not needed to run the program.
:p.
Check out the file
:font facename=Courier size=13x8.
splbook.all,
:font facename=default size=0x0.
which contains pointers to all spellbooks which will comprise the
master spell list.  
:link reftype=hd res=0.Click here:elink. for more information.
:p.
Also, look at the file
:font facename=Courier size=13x8.
splicon.res
:font facename=default size=0x0.
which contains a mapping of sphell spheres and schools to icons.
:link reftype=hd res=1.Click here:elink. for more information.
:p.
You already know how to view the docs; to run the program, just
run 
:font facename=Courier size=13x8.
spl.exe.
:font facename=default size=0x0.

:h2 res=0.Master spell list
:p.
The file
:font facename=Courier size=13x8.
splbook.all
:font facename=default size=0x0.
contains a list of spellbooks which make up the master spell list.
These spellbooks should contain full spell listings.   This is
important because other spellbooks are by default saved only
as lists of titles, which are then looked up in the master spell
list as the spells are loaded in.
:p.
The format of this file is just a list of other files.  To place a
comment in this list, start the line with a semicolon.
:p.

:h2 res=1.Icon mapping
:p.
The file
:font facename=Courier size=13x8.
splicon.def
:font facename=default size=0x0.
contains a mapping of spell schools and spheres to icons.
:p.
Spells in a book are shown with an icon which corresponds to their sphere
(if given) or school (if no sphere is available).  This file contains
a list of text strings to look for in the schools and spheres, and
a list of icons they correspond to.  If a spell matches two or more
text strings, the first one found is used.
:p.
The format of the file is a text string (containing no spaces), then
a space, then the path name of an icon file to use.  Comments are
allowed, as long as they begin with a semicolon.
:p.

:h2.File menu
:p.
From this menu, you can load and save spellbooks, or create new
spellbooks.  Loading a spellbook will create a new window.
:p.
Spellbooks can be loaded and saved in two ways.  The menu options
"Load Spellbook" and "Save Spellbook" only save lists of spells, which
are looked up in the master list.  The options referring to the
"full" spellbook save the entire text of spells.
:p.

:h2.Spell menu
:p.
These options are only available if the book is writeable
(i.e. not the master spell list).  These options allow you to
add new spells (use
:link reftype=hd res=2.Drag and Drop:elink.
instead) or delete spells from a book.

:h2.Book menu
These are more detailed descriptions of options under the book menu.

:h3.Find subset
:p.
This brings up a dialog which allows you to search through the
spells in the book (window) and bring up a new book (window)
which contains the results of the search.
:p.
Spells can be sorted on several fields, with the results
OR'ed or AND'ed together depending on the buttons in the
lower left corner of the dialog.  Case sensitivity is also
optional.  Most of the fields just want you to enter text.
:p.
The "level" field allows you to enter a number for the spell
level, such as "5" to select all 5th level spells.  It also
allows you to enter things like ">5" or "<5" to select all
spells higher or lower than a certain level.
:p.
The "description" field allows you to enter several lines of
text instead of only one.  All lines entered are searched
for in the spell descriptions, with the results OR'ed or 
AND'ed together according to the button pushed on the right.
:p.
Searches go faster with case sensitivity enabled.  Also,
searches go faster if the program doesn't have to search through
entire spell descriptions... limit using the other fields if possible.
:p.
:h3.Sort spellbook
:p.
This sorts the spellbook, priest spells before mage spells, then
in alphabetical order.

:h3.Rename spellbook
:p.
This renames the spellbook... it does not change the actual name of
the file the spellbook is saved in, only the name which appears
at the top of the window.

:h2 res=2.Drag and Drop
:p.
To copy spells between windows (spellbooks), drag them with the right mouse
button.  You can select several spells with the left button.  Dragging
can also be used to reposition spells within a window.

:h2.Looking at a spell
:p.
To look at one spell in detail, double click on it.  This brings
up a window which is pretty self-explanatory.  Use ALT-N or ALT-P
as a shortcut to go to the next or previous spell in the book.
:p.
You can copy text from spells onto the clipboard using the
usual methods.  You can even edit and change spells, although
this doesn't work very well and will probably require you to
go in and fix the saved text file with an editor.
:p.
Any file you change and save is saved in the file
:font facename=Courier size=13x8.
splbook.chg.
:font facename=default size=0x0.

:h1.Source information
This is information on the source code.
:h2.Miscellaneous
:p.
The source is written in C++ for the IBM C++ compiler; it will
probably work with other compilers, although I haven't tested
this.  You need a C++ compiler and a resource compiler to compile
the dialogs and menus.
:p.
The makefile is compatible with GNU's make.
:p.
You need an IPF compiler (IPFC) to compile the documentation.

:h2.general.cpp
This file contains some generally useful functions which are
called from the rest of the code.
:p.
There are 3 error functions, "fatal", "error", and "warning",
indicating various levels of severity.  These are pretty useless
in a PM program.
:p.
There is a memory allocation function, which is pretty useless
for a C++ program.  (Just use "new" instead)
:p.
There are 2 string functions to capitalize and uncapitalize a
string... these are rarely found in C libraries, so they're
included here.  These actually get used in the code.
:p.

:h2.splbook.cpp, .hpp
These files contain the C++ classes for handling spells and
spellbooks, and contain no OS/2-specific or PM-specific
information.  Routines for reading and writing spells out
of files are given.
:p.
Note that the classes are set up to allow later addition of
psionic disciplines as a subclass of the general spell.  This
is why the "magespell" class contains things like "school" and
so forth that you would normally associate with all spells.
:p.
If you want to change this program to change the format of
how spells are read in and out (for example to use this
program with another FRP system) this is the file to change.
It is also the most portable to other systems besides OS/2.
:p.

:h2.spl.cpp, .hpp
This is the main file that puts up menus and windows, allows
direct manipulation and so forth... all functions are commented
so experienced programmers should be able to find their way
around.
:p.
There is a class called "spellwindow" which is the window that
shows a list of spells and their icons.  This window uses a
container to display the icons.  A double-linked list of all
active spellwindows is pointed to by the variable "first".
:p.
There is also a class called "slwindow" (clever, huh?) which
shows one individual spell when you double-click on it.  This
window uses a multi-line entry field (MLE) to display the
spell.  A linked list of these is pointed to by "firstspell".
:p.
Naturally, this is really confusing and I'll try to clean
things up in a later version of the program!
:p.
:h1.Planned enhancements
:p.
These are things I plan to add to the program in the future, both
in order to give it more functionality, and in order to learn more
about PM programming myself.  If you would like to help me out
with any of these, please let me know what you're working on.
:p.
Better error handling.
:p.
On-line help from within the program.
:p.
More views of the main container window... text view, details view,
and large icon view should all be options.
:p.
More ways to sort spellbooks.
:p.
Allowing users to edit spells in a way that actually works.
:p.
More drag and drop stuff... should allow you to drop a text file
into a spell window and read it in.  Also, should allow you to
drag a spell onto the printer and print it.  Drag and drop
icon assignment would also be cool.
:p.
:euserdoc.
