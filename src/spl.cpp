/* spl.cpp:  Spellbook control program

    Copyright (C) 1993 John-Marc Chandonia

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define VERSION "1.0, 8/10/93"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define INCL_WIN
#define INCL_GPI
#define INCL_WINSTDFILE
#define INCL_DOSPROCESS
#include <os2.h>
#include "general.hpp"
#include "splbook.hpp"
#include "spl.h"
#include "spldlg.h"

MRESULT EXPENTRY window_func(HWND, ULONG, MPARAM, MPARAM);

ULONG flFlags;  // window frame declaration 
unsigned char spbclass[] = "Spellbookwindow";  // class name 
unsigned char splclass[] = "Spellwindow";  // class name 
HAB hand_ab;  // anchor block 
BYTE xfer_buffer[10240];
CNRINFO cnrInfo;

// table of icons for various schools and spheres.
int icons;
HPOINTER *icontable;
char **icontext;

// icon for dragging
HPOINTER dragicon;

int windows=1000;
spellbook *masterlist;

// spell record 
typedef struct spellrecord {
    RECORDCORE core;
    spell *s;  // the spell
    spelllist *spl;  // where the spell is in the spellbook
} spellrecord;

// set up the icon table and the drag icon.
void setupicons() {
    FILE *infile;
    char buffer[256];
    int i,j,k;

    dragicon=WinLoadFileIcon((PSZ)"spl.ico",FALSE);

    // get spell icons out of file.
    icons=0;
    if ((infile=fopen(ICONFILE,"r"))==NULL) return;

    // count lines
    while (!feof(infile)) {
	fgets(buffer,256,infile);
	if ((buffer[0]!=';') && (strlen(buffer)>2)) {
	    icons++;
	}
    }
    rewind(infile);

    icontable=new HPOINTER[icons];
    icontext=new PCHAR[icons];

    for (i=0; i<icons; i++) {
	do {
	    fgets(buffer,256,infile);
	} while ((buffer[0]==';') || (strlen(buffer)<=2));

	// get rid of \n
	buffer[strlen(buffer)-1]=(char)0;

	// find first space in line
	if (strchr(buffer,' ')==NULL) {
	    icons=0;
	    return;
	}
	// with no error handling whatsoever!
	j=strlen(buffer)-strlen(strchr(buffer,' '));
	icontext[i]=new char[j+1];
	icontext[i][j]=(char)0;
	strncpy(icontext[i],buffer,j);
	icontable[i]=WinLoadFileIcon((PSZ)(buffer+j+1),FALSE);
    }
}

// return the appropriate icon for a spell, from the icon table
HPOINTER lookup_icon(spell *s) {
    magespell *ms;
    priestspell *ps;
    int i;

    // if a priest spell, try to look it up by sphere
    if (s->type=='P') {
	ps=(priestspell *)s;
	if (ps->sphere) {
	    for (i=0; i<icons; i++) 
		if (strstr(ps->sphere,icontext[i])!=0)
		    return(icontable[i]);
	}
    }

    // try looking it up by school.
    if ((s->type=='M') || (s->type=='P')) {
	ms=(magespell *)s;
	if (ms->school) {
	    for (i=0; i<icons; i++) 
		if (strstr(ms->school,icontext[i])!=0)
		    return(icontable[i]);
	}
    }

    // return generic icon if nothing else works.
    return(dragicon);
}

// print copyleft notice
void copyleft() {
    int i;

    sprintf((char *)xfer_buffer,"spl version %s\n",VERSION);
    i=strlen((char *)xfer_buffer);
    sprintf((char *)xfer_buffer+i,"Copyright (C) 1993 John-Marc Chandonia\n");
    i=strlen((char *)xfer_buffer);
    sprintf((char *)xfer_buffer+i,"spl comes with ABSOLUTELY NO WARRANTY; ");
    i=strlen((char *)xfer_buffer);
    sprintf((char *)xfer_buffer+i,"for details see license.txt.\n");
    i=strlen((char *)xfer_buffer);
    sprintf((char *)xfer_buffer+i,"This is free software, and you are welcome to redistribute ");
    i=strlen((char *)xfer_buffer);
    sprintf((char *)xfer_buffer+i,"it under certain conditions; see license.txt for details.\n");

    i=strlen((char *)xfer_buffer);
    WinMessageBox(HWND_DESKTOP,
		  HWND_DESKTOP,
		  (PSZ)xfer_buffer,
		  (PSZ)"Copyright Notice",
		  102,
		  MB_OK);
}

// a spellbook window class 
class spellwindow {
public:
    // handles for all relevant windows 
    HWND hwndbook;  // main window
    HWND hwndframe;  // frame window
    HWND hwndcnr;  // container window
    
    // one spellbook per window.
    spellbook *sb;

    // has it been saved?
    boolean saved;

    // is is changeable?
    boolean readonly;

    // maintain double linked list 
    spellwindow *next;
    spellwindow *prev;

    spellwindow(char *bookname="New Spellbook");
    ~spellwindow();

    boolean save_titles();
    boolean save_book();
    boolean load_book();
    boolean load_titles();
    boolean set_spellbook(spellbook *);
    boolean sort_book();

    boolean add_spell(spell *s, spellrecord *where=NULL);
    boolean add_new_spell();
    boolean delete_spell(spellrecord *s);
    boolean delete_selected_spells();
    boolean change_spell(spell *oldsp, spell *newsp);
    boolean refresh();  // redraw the window after adding or deleting spells.
};

spellwindow *first=NULL;  /* pointer to first spell window;
			     must be most recently created one */

spellwindow::spellwindow(char *bookname) {    
    // add new window as the first one
    next=first;
    if (first!=NULL) first->prev=this;
    first=this;
    prev=NULL;
    saved=true;
    readonly=false;
    hwndcnr=NULL;
    hwndbook=NULL;
    sb = new spellbook;
    sb->name = strdup(bookname);

    // create the window -- requires first to point to this window!
    hwndframe = WinCreateStdWindow(
				   HWND_DESKTOP,
				   WS_VISIBLE,
				   &flFlags,
				   (PSZ)spbclass,
				   (PSZ)bookname,
				   WS_VISIBLE,
				   0,
				   BOOKMENU,
				   NULL);
}

spellwindow::~spellwindow() {
    // remove from linked list
    if (this==first) first=next;
    if (next!=NULL) next->prev=prev;
    if (prev!=NULL) prev->next=next;

    delete sb; // but not the spells inside!

    // remove from display
    WinDestroyWindow(hwndframe);
}


// look up the spell window associated with a given book handle 
spellwindow *windowof(HWND b) {
    spellwindow *x;
    for (x=first; x!=NULL; x=x->next) 
	if (x->hwndbook==b) return(x);

    return(NULL);
}

// save spellbook as list of spells.
boolean spellwindow::save_titles() {
    FILEDLG fd;
    char filename[CCHMAXPATH]="*.lst";

    // clear all fields to zero before proceeding
    memset(&fd,0,sizeof(FILEDLG));

    // set up file dialog
    fd.cbSize=sizeof(FILEDLG);
    fd.fl=FDS_CENTER|FDS_SAVEAS_DIALOG;
    fd.pszTitle=(PSZ)"Save spellbook to file:";
    strcpy(fd.szFullFile,filename);

    if (!WinFileDlg(HWND_DESKTOP,
		    hwndbook,
		    &fd)) return(false);

    if (fd.lReturn==DID_OK) {
      sb->print_titles(fd.szFullFile);
      saved=true;
      return(true);
    }
    else return(false);
}

// print entire spellbook to file.
boolean spellwindow::save_book() {
    FILEDLG fd;
    char filename[CCHMAXPATH]="*.sbk";

    // clear all fields to zero before proceeding
    memset(&fd,0,sizeof(FILEDLG));

    // set up file dialog
    fd.cbSize=sizeof(FILEDLG);
    fd.fl=FDS_CENTER|FDS_SAVEAS_DIALOG;
    fd.pszTitle=(PSZ)"Print spellbook to file:";
    strcpy(fd.szFullFile,filename);

    if (!WinFileDlg(HWND_DESKTOP,
		    hwndbook,
		    &fd)) return(false);

    if (fd.lReturn==DID_OK) {
      sb->print_book(fd.szFullFile);
      return(true);
    }
    else return(false);
}

// load a spellbook from a file, add to spellbook in window.
boolean spellwindow::load_book() {
    FILEDLG fd;
    char filename[CCHMAXPATH]="*.sbk";
    spellbook *newsb;
    spelllist *i;

    // clear all fields to zero before proceeding
    memset(&fd,0,sizeof(FILEDLG));

    // set up file dialog
    fd.cbSize=sizeof(FILEDLG);
    fd.fl=FDS_CENTER|FDS_OPEN_DIALOG;
    fd.pszTitle=(PSZ)"Load spellbook from file:";
    strcpy(fd.szFullFile,filename);

    if (!WinFileDlg(HWND_DESKTOP,
		    hwndbook,
		    &fd)) return(false);

    if (fd.lReturn==DID_OK) {
	newsb=new spellbook;
	newsb->read_book(fd.szFullFile);

	// add new spells to my window, at end.
	for (i=newsb->first; i!=NULL; i=i->next)
	    add_spell(i->s);
	delete newsb;
	
	refresh();

	// change name of book to book just read.
	if (newsb->name) {
	    delete (sb->name);
	    sb->name=strdup(newsb->name);
	    WinSetWindowText(hwndframe,(PSZ)sb->name);
	}
	saved=true;

	return(true);
    }
    else return(false);
}

// load a spellbook from a file of titles, add to spellbook in window.
boolean spellwindow::load_titles() {
    FILEDLG fd;
    char filename[CCHMAXPATH]="*.lst";
    spellbook *newsb;
    spelllist *i;

    // clear all fields to zero before proceeding
    memset(&fd,0,sizeof(FILEDLG));

    // set up file dialog
    fd.cbSize=sizeof(FILEDLG);
    fd.fl=FDS_CENTER|FDS_OPEN_DIALOG;
    fd.pszTitle=(PSZ)"Load spellbook from file:";
    strcpy(fd.szFullFile,filename);

    if (!WinFileDlg(HWND_DESKTOP,
		    hwndbook,
		    &fd)) return(false);

    if (fd.lReturn==DID_OK) {
	newsb=new spellbook;
	newsb->read_titles(fd.szFullFile,masterlist);

	// add new spells to my window, at end.
	for (i=newsb->first; i!=NULL; i=i->next)
	    add_spell(i->s);
	delete newsb;

	refresh();
	
	// change name of book to book just read.
	if (newsb->name) {
	    delete (sb->name);
	    sb->name=strdup(newsb->name);
	    WinSetWindowText(hwndframe,(PSZ)sb->name);
	}

	saved=true;
	return(true);
    }
    else return(false);
}

// set window to a given spellbook
boolean spellwindow::set_spellbook(spellbook *x) {
    spellrecord *deleteme;
    spelllist *i;

    // clear old spellbook, and window
    while (deleteme=(spellrecord *)
	   WinSendMsg(hwndcnr,
		      CM_QUERYRECORD,
		      0,
		      MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER)))
	   delete_spell(deleteme);
	   
    // add new spells to my window, at end.
    for (i=x->first; i!=NULL; i=i->next)
	   add_spell(i->s);
	
    // change name of book to book just read.
    if (x->name) {
	delete (sb->name);
	sb->name=strdup(x->name);
	WinSetWindowText(hwndframe,(PSZ)sb->name);
    }
    
    refresh();

    saved=true;
    return(true);
}

// sort function:  mage, then priest in alphabetical order.
SHORT APIENTRY spell_order(PRECORDCORE prc1, PRECORDCORE prc2, PVOID foo) {
    spellrecord *psr1, *psr2;
    FILE *outfile;

    psr1=(spellrecord *)prc1;
    psr2=(spellrecord *)prc2;

    if ((psr1->s->type=='P') && (psr2->s->type=='M'))
	return(-1);
    if ((psr1->s->type=='M') && (psr2->s->type=='P'))
	return(1);

    return(strcmpi(psr1->s->name,psr2->s->name));
}

// sort spellbook
boolean spellwindow::sort_book() {
    spelllist *sl1, *sl2;

    WinSendMsg(hwndcnr,
	       CM_SORTRECORD,
	       (MPARAM)(spell_order),
	       (MPARAM)NULL);

    // sort sb also, according to result of container sort
    spellrecord *i;
    i=(spellrecord *)
	WinSendMsg(hwndcnr,
		   CM_QUERYRECORD,
		   0,
		   MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
    sl1=i->spl;
    sb->first=sl1;
    sl1->prev=NULL;  
    while (i!=NULL) {
	i=(spellrecord *)
	    WinSendMsg(hwndcnr,
		       CM_QUERYRECORD,
		       (MPARAM)i,
		       MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
	if (i!=NULL) {
	    sl2=i->spl;
	    sl2->prev=sl1;
	    sl1->next=sl2;
	    sl1=sl2;
	}
    }
    sl1->next=NULL;
    sb->last=sl1;

    return(true);    
}

// add spell to a window, after spell "where" (and to book also)
boolean spellwindow::add_spell(spell *s, spellrecord *where) {
    char buffer[256];
    spellrecord *newrecord;
    LONG extrabytes;
    HPOINTER myicon;

    extrabytes=(LONG)(sizeof(spellrecord)-sizeof(RECORDCORE));
    // tell the container to allocate memory for new record 
    newrecord=(spellrecord *)WinSendMsg(hwndcnr,
					CM_ALLOCRECORD,
					MPFROMLONG(extrabytes),
					(MPARAM)1);
	    
    // set up record 
    newrecord->core.flRecordAttr=0;
    newrecord->s=s;
    newrecord->core.pszIcon=(PSZ)s->name;
    newrecord->core.pszText=(PSZ)s->name;
    newrecord->core.pszName=(PSZ)s->name;
    myicon=lookup_icon(s);
    newrecord->core.hptrIcon=myicon;
    newrecord->core.hptrMiniIcon=myicon;
    newrecord->core.preccNextRecord=0;
	    
    // tell the container to add the record 
    RECORDINSERT ri;
    ri.cb=sizeof(RECORDINSERT);
    if (where==NULL) 
      ri.pRecordOrder=(PRECORDCORE)CMA_END;
    else
      ri.pRecordOrder=&(where->core);
    ri.pRecordParent=NULL;
//    ri.fInvalidateRecord=TRUE;
    ri.fInvalidateRecord=FALSE;
    ri.zOrder=CMA_TOP;
    ri.cRecordsInsert=1;
	
    WinSendMsg(hwndcnr,
	       CM_INSERTRECORD,
	       MPFROMP(newrecord),
	       MPFROMP(&ri));

    // add to spellbook, also
    if (where==NULL)  // add at end.
	newrecord->spl = sb->add_spell(*s, sb->last);
    else  // add after where
	newrecord->spl = sb->add_spell(*s, where->spl);

    if (!readonly) saved=false;
    return(true);
}

// add a new spell to a window
boolean spellwindow::add_new_spell() {
    static int i=0;
    char *buffer;
    
    spell *newspell;

    // edit new spell in editor

    // load new spell into window
    newspell=new magespell;
    buffer=new char[100];
    sprintf(buffer,"New spell # %d\n",i++);
    newspell->name=buffer;

    // insert spell after first emphasized record, if any
    spellrecord *insafter;
    insafter=(spellrecord *)
	PVOIDFROMMR(WinSendMsg(hwndcnr,
			       CM_QUERYRECORDEMPHASIS,
			       MPFROMP(CMA_FIRST),
			       MPFROMSHORT(CRA_SELECTED)));

    if (insafter==NULL)
      add_spell(newspell);
    else
      add_spell(newspell,insafter);

    refresh();
    return(true);
}

// delete a spell from window and from the book.
boolean spellwindow::delete_spell(spellrecord *deleteme) {
  PVOID removeme=&(deleteme);

  // remove from spellbook
  sb->del_spell(deleteme->spl);

  // and from window
  WinSendMsg(hwndcnr,
	     CM_REMOVERECORD,
	     MPFROMP(removeme),
	     MPFROM2SHORT(1,CMA_FREE)); // |CMA_INVALIDATE));

  if (!readonly) saved=false;
  return(true);
}

// delete all selected spells from window, then update the window.
boolean spellwindow::delete_selected_spells() {
    spellrecord *deleteme;

    while (deleteme=(spellrecord *)
	   WinSendMsg(hwndcnr,
		      CM_QUERYRECORDEMPHASIS,
		      MPFROMP(CMA_FIRST),
		      MPFROMSHORT(CRA_SELECTED)))
	delete_spell(deleteme);

    refresh();
    return(true);
}

// refresh spell window 
boolean spellwindow::refresh() {
    WinSendMsg(hwndcnr,
	       CM_INVALIDATERECORD,
	       (MPARAM)0,
	       MPFROM2SHORT(0,CMA_REPOSITION));
    return(true);
}


// change spell, update screen and spellbook.
boolean spellwindow::change_spell(spell *oldsp, spell *newsp) {
    spellrecord *found;
    SEARCHSTRING oldname;

    oldname.pszSearch=(PSZ)oldsp->name;
    oldname.fsPrefix=TRUE;
    oldname.fsCaseSensitive=TRUE;
    oldname.usView=CV_NAME;
    oldname.cb=sizeof(oldname);

    found=(spellrecord *)WinSendMsg(hwndcnr,
				    CM_SEARCHSTRING,
				    (MPARAM)(&oldname),
				    (MPARAM)CMA_FIRST);

    if (!found) return(false);

    // name match does not imply a real match.
    if (found->s == oldsp) {
	add_spell(newsp,found);
	delete_spell(found);
	refresh();
	return (true);
    }
    return(false);
}

// a class for displaying individual spells
class slwindow {
public:
    // handles for all relevant windows 
    HWND hwndmain;  // main window
    HWND hwndmle;   // entry field
    HWND hwndframe;  // frame window

    // double linked list
    slwindow *next;
    slwindow *prev;

    // default is readonly; can be edited.
    boolean readonly;

    // one spelllist associated with the window:
    spelllist *sl;

    // the window that the spelllist is in, if any.
    spellwindow *parent;
    
    slwindow(spelllist *spll=NULL);
    ~slwindow();
};

slwindow *firstspell=NULL;

slwindow::slwindow(spelllist *spll) {    
    char *windowname;

    // add new window as the first one
    next=firstspell;
    if (firstspell!=NULL) firstspell->prev=this;
    firstspell=this;
    prev=NULL;

    readonly=true;
    sl=spll;

    if (sl) windowname=sl->s->name;
    else windowname="Spell";

    // create the window 
    hwndframe = WinCreateStdWindow(
				  HWND_DESKTOP,
				  WS_VISIBLE,
				  &flFlags,
				  (PSZ)splclass,
				  (PSZ)windowname,
				  WS_VISIBLE,
				  0,
				  SPELLMENU,
				  NULL);
}

slwindow::~slwindow() {
    // remove from linked list
    if (this==firstspell) firstspell=next;
    if (next!=NULL) next->prev=prev;
    if (prev!=NULL) prev->next=next;

    // remove from display
    WinDestroyWindow(hwndframe);
}

slwindow *spellwindowof(HWND b) {
    slwindow *x;
    for (x=firstspell; x!=NULL; x=x->next) 
	if (x->hwndmain==b) return(x);

    return(NULL);
}

void show_spell(spellwindow *mywin, MPARAM whichspell) {
    char buffer[256];
    PNOTIFYRECORDENTER pn;
    spellrecord *selected;
    slwindow *child;

    pn = (PNOTIFYRECORDENTER)whichspell;
    selected = (spellrecord *)(pn->pRecord);
    if (selected) {
	child = new slwindow(selected->spl);
	child->parent=mywin;
    }
}

void find_and_show_spell(spellwindow *mywin, spell *s) {
    spellrecord *found;
    SEARCHSTRING oldname;
    slwindow *child;

    oldname.pszSearch=(PSZ)s->name;
    oldname.fsPrefix=TRUE;
    oldname.fsCaseSensitive=TRUE;
    oldname.usView=CV_NAME;
    oldname.cb=sizeof(oldname);

    found=(spellrecord *)WinSendMsg(mywin->hwndcnr,
				    CM_SEARCHSTRING,
				    (MPARAM)(&oldname),
				    (MPARAM)CMA_FIRST);

    if (!found) return;

    // name match does not imply a real match.
    if (found->s == s) {
	child = new slwindow(found->spl);
	child->parent=mywin;
    }
}

MRESULT drop_spell(spellwindow *mywin, PCNRDRAGINFO pcdi) {
    char buffer[256];
    int items;
    PDRAGINFO pdraginfo;

    pdraginfo=pcdi->pDragInfo;

    // make sure we can access the draginfo structure
    if (!DrgAccessDraginfo(pdraginfo))
	return(MRFROM2SHORT(DOR_NODROPOP,0));
    
    // how many items?
    items=DrgQueryDragitemCount(pdraginfo);

    spellrecord *dropped_on;
    PDRAGITEM pditem;
    spellrecord *dropping;
    // add each item
    if (pcdi->pRecord) {            // if dropped on another spell
	for (int i=items-1; i>=0; i--) {
	    pditem=DrgQueryDragitemPtr(pdraginfo,i);
	    
	    // add after record it was dropped on
	    dropped_on=(spellrecord *)(pcdi->pRecord);
	    dropping=(spellrecord *)pditem->ulItemID;
	    mywin->add_spell(dropping->s,dropped_on);

	    // dropped on same window -> delete old item
	    if (mywin->hwndbook==pditem->hwndItem)
		mywin->delete_spell(dropping);
	}
    }
    else {                             // dropped on empty space
	for (int i=0; i<items; i++) {
	    pditem=DrgQueryDragitemPtr(pdraginfo,i);
	    
	    // add at end
	    dropping=(spellrecord *)pditem->ulItemID;
	    mywin->add_spell(dropping->s);

	    // dropped on same window -> delete old item
	    if (mywin->hwndbook==pditem->hwndItem) 
	      mywin->delete_spell(dropping);
	}
    }

    mywin->refresh();

    DrgFreeDraginfo(pdraginfo);
	
}


// something is being dragged over the container
MRESULT drag_over(spellwindow *mywin, PDRAGINFO pdraginfo) {
    USHORT usIndicator=DOR_DROP,usOperation;
    int items;

    if (mywin->readonly) return(MRFROM2SHORT(DOR_NEVERDROP,DO_COPY));

    // make sure we can access the draginfo structure
    if (!DrgAccessDraginfo(pdraginfo)) 
	return(MRFROM2SHORT(DOR_NODROPOP,0));
    
    // how many items?
    items=DrgQueryDragitemCount(pdraginfo);

    // check each item
    for (int i=0; i<items; i++) {
	PDRAGITEM pditem;
	pditem=DrgQueryDragitemPtr(pdraginfo,i);
	if (!DrgVerifyRMF(pditem,(PSZ)"Spell",(PSZ)"Spell"))
	    usIndicator=DOR_NEVERDROP;
    }

    // only operation for spells
    usOperation=DO_COPY;

    DrgFreeDraginfo(pdraginfo);
    return(MRFROM2SHORT(usIndicator,usOperation));
}

BOOL init_drag(spellwindow *mywin,PCNRDRAGINIT pcdi) {
    char buffer[256];
    ULONG items;
    spellrecord *selected;

    // no dragging from empty space
    if (pcdi->pRecord==NULL) return(FALSE);

    // if record is selected, it may be part of a group 
    if (pcdi->pRecord->flRecordAttr & CRA_SELECTED) {
	// find # of items being dragged
	items=0;
	selected=(spellrecord *)WinSendMsg(mywin->hwndcnr,
					   CM_QUERYRECORDEMPHASIS,
					   MPFROMP(CMA_FIRST),
					   MPFROMSHORT(CRA_SELECTED));
	while (selected!=NULL) {
	    selected=(spellrecord *)WinSendMsg(mywin->hwndcnr,
					       CM_QUERYRECORDEMPHASIS,
					       MPFROMP(selected),
					       MPFROMSHORT(CRA_SELECTED));
	    items++;
	}
    }
    else items=1;

    // no items dragged?  shouldn't get here.
    if (items==0) return (FALSE);

    // get DRAGINFO structure
    PDRAGINFO pdraginfo;
    pdraginfo=DrgAllocDraginfo(items);

    DRAGITEM dragitem;

    // common parts of DRAGITEM structure
    dragitem.hwndItem=mywin->hwndbook;
    dragitem.hstrType=DrgAddStrHandle((PSZ)"Spell");
    dragitem.hstrRMF=DrgAddStrHandle((PSZ)"<Spell,Spell>");
    dragitem.hstrContainerName=NULL;
    dragitem.fsControl=0;
    dragitem.fsSupportedOps=DO_COPYABLE;

    // add each dragged item
    selected=(spellrecord *)CMA_FIRST;
    for (int i=0; i<items; i++) {
	if (items==1) 
	    // if there's only 1 item, it must be the one direct
	    // manipulation started on.
	    selected=(spellrecord *)pcdi->pRecord;
	else
	    selected=(spellrecord *)WinSendMsg(mywin->hwndcnr,
					       CM_QUERYRECORDEMPHASIS,
					       MPFROMP(selected),
					       MPFROMSHORT(CRA_SELECTED));

	dragitem.hstrSourceName=DrgAddStrHandle(selected->core.pszText);
	dragitem.hstrTargetName=dragitem.hstrSourceName;
	dragitem.ulItemID=(ULONG)selected;

	// add dragitem to draginfo structure
	DrgSetDragitem(pdraginfo,&dragitem,sizeof(DRAGITEM),i);
    }

    DRAGIMAGE dimg;
    if (items==1) 
	dimg.hImage=lookup_icon(selected->s);
    else dimg.hImage=dragicon;
    dimg.fl=DRG_ICON;
    dimg.cb=sizeof(DRAGIMAGE);
    dimg.cxOffset=0;
    dimg.cyOffset=0;

    pdraginfo->hwndSource=mywin->hwndbook;
    HWND destination;
    destination = DrgDrag(mywin->hwndcnr,pdraginfo,&dimg,1,VK_ENDDRAG,NULL);

/*
    sprintf(buffer,"Item Dropped on handle %d\n",(int)destination);
    WinMessageBox(HWND_DESKTOP,
		  HWND_DESKTOP,
		  (PSZ)buffer,
		  (PSZ)"WM_INITDRAG",
		  102,
		  MB_OK);
*/	    
	    
    return(TRUE);
}

MRESULT savequit(spellwindow *mywin) {

    // has window been saved? 
    if (!mywin->saved) {
	ULONG response;
	response= WinMessageBox(HWND_DESKTOP,
				HWND_DESKTOP,
				(PSZ)"Save spellbook before closing?",
				(PSZ)"Spellbook changed since last save",
				102,
				MB_YESNOCANCEL|MB_ICONQUESTION);
	if (response==MBID_YES) {
	    if (!mywin->save_book()) return((MRESULT)FALSE);
	}
	else if (response==MBID_CANCEL) return((MRESULT)FALSE);
    }
    
    if (first->next==NULL) { //this is the last window 
	ULONG response = WinMessageBox(HWND_DESKTOP,
				       HWND_DESKTOP,
				       (PSZ)"Closing the last spellbook will end the program.  Are you sure?",
				       (PSZ)"Warning!",
				       102,
				       MB_YESNO|MB_ICONEXCLAMATION);
	if (response==MBID_YES)
	    WinPostMsg(mywin->hwndbook,WM_QUIT,0,0);
    }
    else {
	delete mywin;
	return ((MRESULT)TRUE);
    }
    
    return ((MRESULT)TRUE);
}

// quick enabling of menu items
VOID EnableMenuItem( HWND hwndMenu, SHORT sIditem, BOOL bEnable)
{
  SHORT sFlag;

  if(bEnable)
    sFlag = 0;
  else
    sFlag = MIA_DISABLED;

  WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(sIditem, TRUE),
               MPFROM2SHORT(MIA_DISABLED, sFlag));

}

// create a subset, given a window to choose from and a selection dialog.
// returns true if a subset was found, else false.
boolean create_subset(spellwindow *mywin,HWND dlg) {
    spellwindow *nsw=NULL;
    spellrecord *sr;
    spell *s;
    char *tmpstr;
    magespell *ms;
    priestspell *ps;
    boolean matches,itemmatch;
    int i,j;

    // see if we're doing "and" or "or"
    boolean s_and;
    if (WinQueryButtonCheckstate(dlg,DLG_AND)==1) s_and=true;
    else s_and=false;

    // see if things are case sensitive
    boolean s_nocase=false;
    if (WinQueryButtonCheckstate(dlg,DLG_CASE)==0) s_nocase=true;

    // see if there's a restriction on the name.
    boolean s_name;
    char *str_name;
    if ((i=WinQueryDlgItemTextLength(dlg,DLG_NAME))==0) 
	s_name=false;
    else {
	s_name=true;
	str_name=new char[i+1];
	WinQueryDlgItemText(dlg,DLG_NAME,i+1,(PSZ)str_name);
	if (s_nocase) upstr(str_name);
    }

    // check school
    boolean s_school;
    char *str_school;
    if ((i=WinQueryDlgItemTextLength(dlg,DLG_SCHOOL))==0) 
	s_school=false;
    else {
	s_school=true;
	str_school=new char[i+1];
	WinQueryDlgItemText(dlg,DLG_SCHOOL,i+1,(PSZ)str_school);
	if (s_nocase) upstr(str_school);
    }

    // check sphere
    boolean s_sphere;
    char *str_sphere;
    if ((i=WinQueryDlgItemTextLength(dlg,DLG_SPHERE))==0) 
	s_sphere=false;
    else {
	s_sphere=true;
	str_sphere=new char[i+1];
	WinQueryDlgItemText(dlg,DLG_SPHERE,i+1,(PSZ)str_sphere);
	if (s_nocase) upstr(str_sphere);
    }

    // check mage,priest,reversible
    boolean s_mage;
    boolean s_priest;
    boolean s_reversible;
    if (WinQueryButtonCheckstate(dlg,DLG_MAGE)==1) s_mage=true;
    else s_mage=false;
    if (WinQueryButtonCheckstate(dlg,DLG_PRIEST)==1) s_priest=true;
    else s_priest=false;
    if (WinQueryButtonCheckstate(dlg,DLG_REVERSIBLE)==1) s_reversible=true;
    else s_reversible=false;

    
    // check save
    boolean s_save;
    char *str_save;
    if ((i=WinQueryDlgItemTextLength(dlg,DLG_SAVE))==0) 
	s_save=false;
    else {
	s_save=true;
	str_save=new char[i+1];
	WinQueryDlgItemText(dlg,DLG_SAVE,i+1,(PSZ)str_save);
	if (s_nocase) upstr(str_save);
    }

    // check components
    boolean s_components;
    boolean s_c_v=false;
    boolean s_c_s=false;
    boolean s_c_m=false;
    if ((i=WinQueryDlgItemTextLength(dlg,DLG_COMPONENTS))==0) 
	s_components=false;
    else {
	s_components=true;
	tmpstr=new char[i+1];
	WinQueryDlgItemText(dlg,DLG_COMPONENTS,i+1,(PSZ)tmpstr);
	upstr(tmpstr);
	if (strchr(tmpstr,'V')!=0) s_c_v=true;
	if (strchr(tmpstr,'S')!=0) s_c_s=true;
	if (strchr(tmpstr,'M')!=0) s_c_m=true;
	delete tmpstr;
    }

    // see if there's a restriction on the level.
    boolean s_level;
    boolean s_level_gt=false;
    boolean s_level_lt=false;
    int int_level=0;
    if ((i=WinQueryDlgItemTextLength(dlg,DLG_LEVEL))==0) 
	s_level=false;
    else {
	s_level=true;
	tmpstr=new char[i+1];
	WinQueryDlgItemText(dlg,DLG_LEVEL,i+1,(PSZ)tmpstr);
	if (tmpstr[0]=='>') {
	    s_level_gt=true;
	    sscanf(tmpstr,">%d",&int_level);
	}
	else if (tmpstr[0]=='<') {
	    s_level_lt=true;
	    sscanf(tmpstr,"<%d",&int_level);
	}
	else sscanf(tmpstr,"%d",&int_level);
	delete tmpstr;
    }

    // see if there's a description search going
    boolean s_desc;
    int int_desc;
    int str_desc_len;
    boolean s_desc_and=false;
    PCHAR *str_desc;
    HWND dh=WinWindowFromID(dlg,DLG_DESCRIPTION);
    if (WinSendMsg(dh,
		   MLM_QUERYTEXTLENGTH,
		   0,0)==0)
	s_desc=false;
    else {
	int_desc=(int)WinSendMsg(dh,
				 MLM_QUERYLINECOUNT,
				 0,0);
	str_desc=new PCHAR[int_desc];
	IPT pos= 0;

	// see if we're and'ing or or'ing the description search.
	if (WinQueryButtonCheckstate(dlg,DLG_DES_AND)==1) s_desc_and=true;

	// set up xfer buffer
	WinSendMsg(dh,
		   MLM_SETIMPORTEXPORT,
		   (MPARAM)(&xfer_buffer),
		   (MPARAM)10240);
	WinSendMsg(dh,
		   MLM_FORMAT,
		   (MPARAM)MLFIE_NOTRANS,
		   0);
	for (i=0; i<int_desc; i++) {
	    WinSendMsg(dh,MLM_SETSEL,(MPARAM)pos,(MPARAM)pos);
	    j=(int)WinSendMsg(dh,
			      MLM_QUERYLINELENGTH,
			      (MPARAM)pos,
			      0);
	    str_desc[i]=new char[j+1];
	    str_desc_len=j;
	    WinSendMsg(dh,
		       MLM_EXPORT,
		       (MPARAM)&pos,
		       (MPARAM)&j);
	    strncpy(str_desc[i],(char *)xfer_buffer,str_desc_len);
	    str_desc[i][str_desc_len]=(char)0;

	    // get rid of \n at end of string.
	    if ((str_desc_len>1) && (i<int_desc-1))
		str_desc[i][str_desc_len-1]=(char)0;

	    if (s_nocase) upstr(str_desc[i]);
	}
    }
    
    // get first spell
    sr=(spellrecord *)
	WinSendMsg(mywin->hwndcnr,
		   CM_QUERYRECORD,
		   0,
		   MPFROM2SHORT(CMA_FIRST,CMA_ITEMORDER));
    while (sr!=NULL) {
	s=sr->s;
	ms=(magespell *)s;
	ps=(priestspell *)s;
	if (s_and) matches=true;
	else matches=false;

	// see if this spell matches selection.
	if (s_name) {
	    if (s_nocase) {
		tmpstr=strdup(s->name);
		upstr(tmpstr);
		itemmatch=(boolean)(strstr(tmpstr,str_name)!=0);
		delete tmpstr;
	    }
	    else itemmatch = (boolean)(strstr(s->name,str_name)!=0);

	    if (s_and) matches = (boolean)(matches && itemmatch);
	    else matches = (boolean)(matches || itemmatch);
	}
	if (s_school) {
	    if ((s->type=='M') || (s->type=='P')) {
		if (s_nocase) {
		    tmpstr=strdup(ms->school);
		    upstr(tmpstr);
		    itemmatch=(boolean)(strstr(tmpstr,str_school)!=0);
		    delete tmpstr;
		}
		else itemmatch = (boolean)(strstr(ms->school,str_school)!=0);
	    }
	    else itemmatch=false;

	    if (s_and) matches = (boolean)(matches && itemmatch);
	    else matches = (boolean)(matches || itemmatch);
	}
	if (s_sphere) {
	    if (s->type=='P') {
		if (s_nocase) {
		    tmpstr=strdup(ps->sphere);
		    upstr(tmpstr);
		    itemmatch=(boolean)(strstr(tmpstr,str_sphere)!=0);
		    delete tmpstr;
		}
		else itemmatch = (boolean)(strstr(ps->sphere,str_sphere)!=0);
	    }
	    else itemmatch=false;

	    if (s_and) matches = (boolean)(matches && itemmatch);
	    else matches = (boolean)(matches || itemmatch);
	}
	if (s_level) {
	    if (s_level_gt) itemmatch = (boolean)(s->level > int_level);
	    else if (s_level_lt) itemmatch = (boolean)(s->level < int_level);
	    else itemmatch = (boolean)(s->level == int_level);

	    if (s_and) matches = (boolean)(matches && itemmatch);
	    else matches = (boolean)(matches || itemmatch);
	}
	if (s_save) {
	    if ((s->type=='M') || (s->type=='P')) {
		if (s_nocase) {
		    tmpstr=strdup(ms->save);
		    upstr(tmpstr);
		    itemmatch=(boolean)(strstr(tmpstr,str_save)!=0);
		    delete tmpstr;
		}
		else itemmatch = (boolean)(strstr(ms->save,str_save)!=0);
	    }
	    else itemmatch=false;

	    if (s_and) matches = (boolean)(matches && itemmatch);
	    else matches = (boolean)(matches || itemmatch);
	}
	if (s_mage) {
	    itemmatch=(boolean)(s->type=='M');

	    if (s_and) matches = (boolean)(matches && itemmatch);
	    else matches = (boolean)(matches || itemmatch);
	}
	if (s_priest) {
	    itemmatch=(boolean)(s->type=='P');

	    if (s_and) matches = (boolean)(matches && itemmatch);
	    else matches = (boolean)(matches || itemmatch);
	}
	if (s_reversible) {
	    if ((s->type=='M') || (s->type=='P')) 
		itemmatch=ms->reversible;
	    else itemmatch=false;

	    if (s_and) matches = (boolean)(matches && itemmatch);
	    else matches = (boolean)(matches || itemmatch);
	}
	if (s_components) {
	    if ((s->type=='M') || (s->type=='P')) {
		itemmatch=(boolean)(s_c_v || s_c_m || s_c_s);
		if (s_c_v) itemmatch = 
		    (boolean)(itemmatch && strchr(ms->components,'V'));
		if (s_c_s) itemmatch = 
		    (boolean)(itemmatch && strchr(ms->components,'S'));
		if (s_c_m) itemmatch = 
		    (boolean)(itemmatch && strchr(ms->components,'M'));
	    }
	    else itemmatch=false;

	    if (s_and) matches = (boolean)(matches && itemmatch);
	    else matches = (boolean)(matches || itemmatch);
	}
	// only search thru the descs if it matters... slow!
	if ((s_desc) &&
	    (((s_and) && (matches==true)) ||
	     ((s_and==false) && (matches==false)))) {
	    if (s_desc_and) itemmatch=true;
	    else itemmatch=false;

	    for (i=0; i<int_desc; i++) {
		if (strlen(str_desc[i])!=0) {
		    if (s_desc_and) itemmatch = 
			(boolean)(itemmatch && 
				  s->desc_search(str_desc[i],s_nocase));
		    else itemmatch = 
			(boolean)(itemmatch || 
				  s->desc_search(str_desc[i],s_nocase));
		}
	    }

	    if (s_and) matches = (boolean)(matches && itemmatch);
	    else matches = (boolean)(matches || itemmatch);
	}

	// if it still matches, add it in.
	if (matches) {
	    if (!nsw) nsw=new spellwindow("Selection Matches:");
	    nsw->add_spell(s);
	}

	// get next spell
	sr=(spellrecord *)
	    WinSendMsg(mywin->hwndcnr,
		       CM_QUERYRECORD,
		       (MPARAM)sr,
		       MPFROM2SHORT(CMA_NEXT,CMA_ITEMORDER));
    }

    // see if any spells were actually selected
    if (nsw==NULL) {
	WinMessageBox(HWND_DESKTOP,
		      HWND_DESKTOP,
		      (PSZ)"No spells in this book matched these selection criteria",
		      (PSZ)"Sorry!",
		      102,
		      MB_OK|MB_ICONEXCLAMATION);
	return(false);
    }
    
    // don't bug the user about closing the window
    nsw->saved=true;
    nsw->refresh();
    return(true);
}

// dialog procedure for select boxes:
MRESULT EXPENTRY select_dlg_func(HWND handle, ULONG msg,
				 MPARAM param1, MPARAM param2) {
    switch (msg) {
    case WM_INITDLG:
	// turn on default buttons.
	WinSendMsg(WinWindowFromID(handle,DLG_AND),
		   BM_SETCHECK,
		   (MPARAM)1,
		   0);
	WinSendMsg(WinWindowFromID(handle,DLG_DES_OR),
		   BM_SETCHECK,
		   (MPARAM)1,
		   0);
	WinSendMsg(WinWindowFromID(handle,DLG_CASE),
		   BM_SETCHECK,
		   (MPARAM)1,
		   0);
	break;

    case WM_COMMAND: 
	switch (SHORT1FROMMP(param1)) {
	case DLG_OK:
	    HWND hwndparent;
	    hwndparent=WinQueryWindow(handle,QW_OWNER);    
	    // search list of spellwindows for frame who owns this dlg.
	    // if found, create subset window.
	    boolean subset_found;
	    spellwindow *x;	
	    subset_found=false;
	    for (x=first; x!=NULL; x=x->next) 
		if (x->hwndframe==hwndparent) 
		    subset_found=create_subset(x,handle);
	    if (subset_found) WinDismissDlg(handle,TRUE);
	    return (MRESULT)TRUE;
	    break;

	case DLG_CANCEL:
	    WinDismissDlg(handle,TRUE);
	    return (MRESULT)FALSE;
	    break;

	case DLG_HELP:
	    WinMessageBox(HWND_DESKTOP,
			  HWND_DESKTOP,
			  (PSZ)"I haven't figured out how to do this yet... see the INF file instead.",
			  (PSZ)"Sorry!",
			  102,
			  MB_OK|MB_ICONQUESTION);
	    return (MRESULT)TRUE;
	}
	break;
    }
    return (WinDefDlgProc(handle,msg,param1,param2));
}

// dialog procedure for renaming spellbook
MRESULT EXPENTRY rename_dlg_func(HWND handle, ULONG msg,
				 MPARAM param1, MPARAM param2) {
    int i;

    switch (msg) {
    case WM_COMMAND: 
	switch (SHORT1FROMMP(param1)) {
	case DLG_OK:
	    HWND hwndparent;
	    hwndparent=WinQueryWindow(handle,QW_OWNER);    
	    // search list of spellwindows for frame who owns this dlg.
	    // if found, create subset window.
	    spellwindow *x;	
	    for (x=first; x!=NULL; x=x->next) 
		if (x->hwndframe==hwndparent) {
		    i=WinQueryDlgItemTextLength(handle,DLG_NEWNAME);
		    if (i>0) {
			delete (x->sb->name);
			x->sb->name=new char[i+1];
			WinQueryDlgItemText(handle,DLG_NEWNAME,i+1,(PSZ)(x->sb->name));
			WinSetWindowText(x->hwndframe,(PSZ)(x->sb->name));
		    }
		}
	    WinDismissDlg(handle,TRUE);
	    return (MRESULT)TRUE;
	    break;

	case DLG_CANCEL:
	    WinDismissDlg(handle,TRUE);
	    return (MRESULT)FALSE;
	    break;
	}
	break;
    }
    return (WinDefDlgProc(handle,msg,param1,param2));
}

// custom window procedure
MRESULT EXPENTRY book_window_func(HWND handle, ULONG msg, 
				  MPARAM param1, MPARAM param2) {
    SWP windowpos;
    short cx,cy;
    char *txt,buffer[256];
    spellwindow *mywin;
    spellwindow *nsw;

    switch (msg) {

    case WM_ERASEBACKGROUND:
	return (MRESULT)TRUE;

    case WM_CREATE:
	// save book handle in first window
	first->hwndbook=handle;

	// find out my size 
	WinQueryWindowPos(handle,&windowpos);
	cx=windowpos.cx;
	cy=windowpos.cy;
	
	// create container filling whole window 
	first->hwndcnr=
	    WinCreateWindow(
			    handle,
			    WC_CONTAINER,
			    0,
			    CCS_AUTOPOSITION |            // Auto position   
			    CCS_EXTENDSEL,           // Extended selection     
			    (LONG)0,
			    (LONG)0,
			    (LONG)cx,
			    (LONG)cy,
			    handle,
			    HWND_TOP,
			    windows++,
			    NULL,
			    NULL);

	// put container in name/icon mode 
	cnrInfo.flWindowAttr = CV_NAME|CV_MINI;
	WinSendMsg(first->hwndcnr,
		   CM_SETCNRINFO,
		   MPFROMP(&cnrInfo),
		   MPFROMLONG(CMA_FLWINDOWATTR));
	
	// and activate it 
	WinShowWindow(first->hwndcnr,TRUE);
	break;

    case WM_SIZE:
	mywin=windowof(handle);

	cx=SHORT1FROMMP(param2);
	cy=SHORT2FROMMP(param2);
	// tell the container to change its size 
	WinSetWindowPos(mywin->hwndcnr,0,0,0,cx,cy,SWP_SIZE);

	break;

    // is menu valid?
    case WM_INITMENU:
	mywin=windowof(handle);
	
	switch(SHORT1FROMMP(param1)) {
	case SPELL_SUBMENU:
	    BOOL enable;

	    if (mywin->readonly) enable=FALSE;
	    else enable=TRUE;

	    EnableMenuItem(HWNDFROMMP(param2), ADDSPELL, enable);
	    EnableMenuItem(HWNDFROMMP(param2), DELSPELL, enable);
	}
	break;

    // command from menu or dialog
    case WM_COMMAND:
	mywin=windowof(handle);
	
	switch(SHORT1FROMMP(param1)) {
	case ADDSPELL:
	    mywin->add_new_spell();
	    break;

	case DELSPELL:
	  mywin->delete_selected_spells();
	  break;

	case SUBSET:
	    WinLoadDlg(HWND_DESKTOP,
		       handle,
		       select_dlg_func,
		       0L,
		       SELECT_DLG,
		       NULL);
	    break;

	case SORTBOOK:
	    mywin->sort_book();
	    break;

	case RENAME:
	    WinDlgBox(HWND_DESKTOP,
		      handle,
		      rename_dlg_func,
		      0L,
		      RENAME_DLG,
		      NULL);
	    break;

	case ABOUT:
	    copyleft();
	    break;

	case NEWBOOK:
	  new spellwindow;
	  break;

	case SAVETITLES:
	  mywin->save_titles();
	  break;

	case SAVEBOOK:
	  mywin->save_book();
	  break;

	case LOADBOOK:
	  nsw=new spellwindow;
	  nsw->load_book();
	  break;

	case LOADTITLES:
	  nsw=new spellwindow;
	  nsw->load_titles();
	  break;


	case CLOSEBOOK:
	  WinSendMsg(mywin->hwndbook,
		     WM_CLOSE,
		     0,
		     0);
	  break;

	case QUITPROGRAM:
	  boolean allsaved=true;
	  spellwindow *ptr;
	  for (ptr=first; ptr!=NULL; ptr=ptr->next)
	      if (ptr->saved==false) allsaved=false;
	  
	  if (!allsaved) {
	      ULONG response = WinMessageBox(HWND_DESKTOP,
					     HWND_DESKTOP,
					     (PSZ)"End the program without saving all spellbooks?",
					     (PSZ)"Warning!",
					     102,
					     MB_YESNO|MB_ICONEXCLAMATION);
	      if (response==MBID_NO) break;
	  }
	  WinPostMsg(mywin->hwndbook,WM_QUIT,0,0);
      }
	break;

    // message from container 
    case WM_CONTROL:
	PCNRDRAGINFO pcdi;
	mywin=windowof(handle);
	
	switch (SHORT2FROMMP(param1)) {
	case CN_ENTER:
	    show_spell(mywin, param2);
	    return ((MRESULT)TRUE);
	    break;

	case CN_INITDRAG:
	    if (init_drag(mywin,(PCNRDRAGINIT)param2))
		return ((MRESULT)FALSE);
	    else
		return ((MRESULT)TRUE);
	    break;
	    
	case CN_DRAGOVER:
	    pcdi=(PCNRDRAGINFO)param2;
	    return drag_over(mywin,pcdi->pDragInfo);
	    break;

	case CN_DROP:
	    pcdi=(PCNRDRAGINFO)param2;
	    return drop_spell(mywin,pcdi);
	    break;

	}
	break;

    // user wants to close the window 
    case WM_CLOSE:
	mywin=windowof(handle);
	return savequit(mywin);
	break;
    }
    return WinDefWindowProc(handle, msg, param1, param2);
}

// empty out mle and show a spell in it
void mle_show_spell(HWND hwndmle, spell *s) {
    IPT insertion_point=0;

    // remove old spell
    IPT text_length;
    text_length=(IPT)WinSendMsg(hwndmle,
				MLM_QUERYTEXTLENGTH,
				0,0);

    // hide window for speed:
    WinShowWindow(hwndmle,FALSE);

    WinSendMsg(hwndmle,
	       MLM_DELETE,
	       0,
	       (MPARAM)text_length);

    // set up transfer buffer
    WinSendMsg(hwndmle,
	       MLM_SETIMPORTEXPORT,
	       (MPARAM)(&xfer_buffer),
	       (MPARAM)10240);
    WinSendMsg(hwndmle,
	       MLM_FORMAT,
	       (MPARAM)MLFIE_CFTEXT,
	       0);

    s->s_print((char *)xfer_buffer);

    WinSendMsg(hwndmle,
	       MLM_IMPORT,
	       (MPARAM)(&insertion_point),
	       (MPARAM)strlen((char *)xfer_buffer));

    // show window again
    WinShowWindow(hwndmle,TRUE);
}

void edit_save_changes(slwindow *mywin) {
    IPT insertion_point;
    ULONG text_length;
    char *tmpfilename;
    FILE *tmpfile;
    spell *mod_spell, *old_spell;
    spellwindow *ptr;
    char *bptr;
    int i;

    WinSendMsg(mywin->hwndmle,MLM_SETREADONLY,(MPARAM)TRUE,0);
    mywin->readonly=true;

	    
    // set up transfer buffer
    text_length=(ULONG)WinSendMsg(mywin->hwndmle,
				  MLM_QUERYTEXTLENGTH,
				  0,0);
    WinSendMsg(mywin->hwndmle,
	       MLM_SETIMPORTEXPORT,
	       (MPARAM)(&xfer_buffer),
	       (MPARAM)10240);
    WinSendMsg(mywin->hwndmle,
	       MLM_FORMAT,
	       (MPARAM)MLFIE_CFTEXT,
	       0);
    
    // get spell into transfer buffer
    insertion_point=0;

    WinSendMsg(mywin->hwndmle,
	       MLM_EXPORT,
	       (MPARAM)(&insertion_point),
	       (MPARAM)(&text_length));
    
    // write spell to file
    tmpfilename=tmpnam(NULL);
    tmpfile=fopen(tmpfilename,"w");
    bptr=(char *)xfer_buffer;
    i=0;
    while (*bptr!=(char)0) {
	if (*bptr==(char)13) {
	    fprintf(tmpfile,"\n");
	    bptr++;
	    i=0;
	}
	else if ((i>65) && (*bptr==' ')) {
	    fprintf(tmpfile,"\n");
	    i=0;
	}
	else fprintf(tmpfile,"%c",*bptr);
	bptr++;
	i++;
    }
    fprintf(tmpfile,"\n");
    fclose(tmpfile);
    
    // read it back in, deleting tmp file
    tmpfile=fopen(tmpfilename,"r");
    if (mywin->sl->s->type=='M') mod_spell=new magespell;
    else mod_spell=new priestspell;
    mod_spell->f_read(tmpfile);
    fclose(tmpfile);
    //unlink(tmpfilename);

    old_spell=mywin->sl->s;
    // get all spellbooks updated with the new spell, including parent of this window!
    for (ptr=first; ptr!=NULL; ptr=ptr->next) {
	ptr->change_spell(old_spell,mod_spell);
    }

    // write the spell to the end of "changed"
    tmpfile=fopen(CHANGEFILE,"a");
    fprintf(tmpfile,"-----\n");
    mod_spell->f_print(tmpfile);
    fclose(tmpfile);

    // show the new spell window
    find_and_show_spell(mywin->parent, mod_spell);

    // remove old spell, and my window
    delete old_spell;
    WinSendMsg(mywin->hwndmain,
	       WM_CLOSE,
	       0,
	       0); 
}

// change mle to given spellist, if everything's OK.
void mle_set_spell(slwindow *mywin, spelllist *sl) {
    if (sl!=NULL) {
	if (!mywin->readonly) {
	    ULONG response = WinMessageBox(HWND_DESKTOP,
					   HWND_DESKTOP,
					   (PSZ)"You are in the middle of editing.  Any changes will be lost.  Are you sure?",
					   (PSZ)"Warning!",
					   102,
					   MB_YESNO|MB_ICONEXCLAMATION);
	    if (response==MBID_NO) return;
	    mywin->readonly=true;
	}
	mywin->sl=sl;
	WinSetWindowText(mywin->hwndframe,(PSZ)mywin->sl->s->name);
	mle_show_spell(mywin->hwndmle,mywin->sl->s);
    }
}

// window function for spell window:
MRESULT EXPENTRY spell_window_func(HWND handle, ULONG msg, 
				   MPARAM param1, MPARAM param2) {
    slwindow *mywin;
    RECTL rcl;
    int cx,cy;

    switch (msg) {
    case WM_ERASEBACKGROUND:
	return (MRESULT)TRUE;

    case WM_CREATE:
	SWP windowpos;

	// save handle 
	firstspell->hwndmain=handle;

	// find out my size 
	WinQueryWindowRect(handle, (PRECTL)&rcl);
	
	// create multi line entry filling whole window 
	firstspell->hwndmle=
	    WinCreateWindow(
			    handle,
			    WC_MLE,
			    (PSZ)0,
			    MLS_READONLY | MLS_WORDWRAP | MLS_VSCROLL,
			    rcl.xLeft,
			    rcl.yBottom,
			    rcl.xRight,
			    rcl.yTop,
			    handle,
			    HWND_TOP,
			    windows++,
			    NULL,
			    NULL);

	// fill mle with spell 
	mle_show_spell(firstspell->hwndmle, firstspell->sl->s);

	// and activate it 
	WinShowWindow(firstspell->hwndmle,TRUE);

	break;

    case WM_SIZE:
	mywin=spellwindowof(handle);

	cx=SHORT1FROMMP(param2);
	cy=SHORT2FROMMP(param2);
	// tell the MLE to change its size 
	WinSetWindowPos(mywin->hwndmle,0,0,0,cx,cy,SWP_SIZE);

	break;


    // menu is activated
    case WM_INITMENU:
	mywin=spellwindowof(handle);
	
	switch(SHORT1FROMMP(param1)) {
	case EDIT_SUBMENU:
	    MRESULT mr1, mr2;
	    BOOL enable;

	    // see if text is selected in MLE
	    mr1 = WinSendMsg(mywin->hwndmle, MLM_QUERYSEL,MPFROMSHORT(MLFQS_MINSEL), NULL);
	    mr2 = WinSendMsg(mywin->hwndmle, MLM_QUERYSEL,MPFROMSHORT(MLFQS_MAXSEL), NULL);
	    if (mr1 != mr2) enable = TRUE;
	    else enable = FALSE;

	    EnableMenuItem(HWNDFROMMP(param2), EDIT_COPY, enable);

	    if (mywin->readonly) enable=FALSE;
	    EnableMenuItem(HWNDFROMMP(param2), EDIT_CUT, enable);
	    EnableMenuItem(HWNDFROMMP(param2), EDIT_CLEAR, enable);

	    // if undoable, and !readonly, enable undo.
	    if (!mywin->readonly) {
		mr1 =  WinSendMsg(mywin->hwndmle, MLM_QUERYUNDO, NULL, NULL);
		if (mr1 != 0) enable = TRUE;
		else enable = FALSE;
	    }
	    EnableMenuItem(HWNDFROMMP(param2), EDIT_UNDO, enable);

	    // if clipboard has text, and !readonly, enable paste.
	    if (!mywin->readonly) {
		if(WinOpenClipbrd(hand_ab))
		{
		    ULONG ulFmtInfo;
		    if (WinQueryClipbrdFmtInfo(hand_ab, CF_TEXT, &ulFmtInfo))
			enable = TRUE;
		    else
			enable = FALSE;
		    WinCloseClipbrd(hand_ab);
		}
		else
		    enable = TRUE;
	    }
	    EnableMenuItem(HWNDFROMMP(param2), EDIT_PASTE, enable);

	    // enable other menus depending on readonly status
	    if (mywin->readonly) enable=TRUE;
	    else enable=FALSE;
	    EnableMenuItem(HWNDFROMMP(param2),EDIT_START,enable);
	    EnableMenuItem(HWNDFROMMP(param2),EDIT_SAVECHANGES,!enable);
	    EnableMenuItem(HWNDFROMMP(param2),EDIT_DISCARDCHANGES,!enable);

	    break;
	}
	break;
	
    // command from menu 
    case WM_COMMAND:
	mywin=spellwindowof(handle);
	
	switch(SHORT1FROMMP(param1)) {
	case NEXTSPELL:
	    mle_set_spell(mywin,mywin->sl->next);
	    break;

	case PREVSPELL:
	    mle_set_spell(mywin,mywin->sl->prev);
	    break;

	case EDIT_UNDO:
	    WinSendMsg(mywin->hwndmle,MLM_UNDO,0,0);
	    break;

	case EDIT_CUT:
	    WinSendMsg(mywin->hwndmle,MLM_CUT,0,0);
	    break;

	case EDIT_CLEAR:
	    WinSendMsg(mywin->hwndmle,MLM_CLEAR,0,0);
	    break;

	case EDIT_COPY:
	    WinSendMsg(mywin->hwndmle,MLM_COPY,0,0);
	    break;

	case EDIT_PASTE:
	    WinSendMsg(mywin->hwndmle,MLM_PASTE,0,0);
	    break;

	case EDIT_START:
	    WinSendMsg(mywin->hwndmle,MLM_SETREADONLY,(MPARAM)FALSE,0);
	    mywin->readonly=false;
	    break;

	case EDIT_SAVECHANGES:
	    edit_save_changes(mywin);
	    break;

	case EDIT_DISCARDCHANGES:
	    WinSendMsg(mywin->hwndmle,MLM_SETREADONLY,(MPARAM)TRUE,0);
	    mywin->readonly=true;
	    mle_show_spell(mywin->hwndmle,mywin->sl->s);
	    break;

	}
	break;

    case WM_CLOSE:
	mywin=spellwindowof(handle);
	delete mywin;
	return (MRESULT)TRUE;
    }
    return WinDefWindowProc(handle, msg, param1, param2);
}

main() {
    int i;
    HMQ hand_mq;  // message queue 
    QMSG q_mess;  // message queue 
    spellwindow *masterwindow;

    // define the frame contents 
    flFlags = FCF_TITLEBAR |
      FCF_SIZEBORDER|
      FCF_MINMAX|
      FCF_SYSMENU|
      FCF_MENU|
      FCF_ACCELTABLE|
      FCF_SHELLPOSITION;

    hand_ab=WinInitialize(0);  // get anchor block 
    hand_mq = WinCreateMsgQueue(hand_ab, 0); // start queue 
 
    // register the classes 
    if (!WinRegisterClass(
			  hand_ab,
			  (PSZ) spbclass,
			  (PFNWP) book_window_func,
			  CS_SIZEREDRAW,
			  0))
	exit(1);
    if (!WinRegisterClass(
			  hand_ab,
			  (PSZ) splclass,
			  (PFNWP) spell_window_func,
			  CS_SIZEREDRAW,
			  0))
	exit(1);


    // load in icons
    setupicons();

    // create first spell book with master list
    masterlist=get_master_list();
    masterwindow=new spellwindow;
    masterwindow->set_spellbook(masterlist);
    masterwindow->readonly=true;
    masterwindow->sort_book();

    // show copyright notice
    copyleft();

    // message loop 
    while(WinGetMsg(hand_ab, &q_mess, 0L,0,0))
	WinDispatchMsg(hand_ab, &q_mess);

    // shut down all remaining windows and quit 
    while (first!=NULL) delete first;
    while (firstspell!=NULL) delete firstspell;
    WinDestroyMsgQueue(hand_mq);
    WinTerminate(hand_ab);
}



