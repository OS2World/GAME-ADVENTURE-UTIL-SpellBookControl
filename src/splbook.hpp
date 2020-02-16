/* spellbk.hpp: spell and spellbook classes

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

#include "general.hpp"

// file containing names of spellbooks to create master list:
#define ALLSPELLS "splbook.all"

// any spell or psionic effect
class spell {
    
public:
    char *name;
    
    char *source;     // what book it came from
    
    int level;
    char type;    // the type of spell; i.e. M, C, P
    char *range;
    char *area;
    char *description[256];
    int lines; // # of lines in the description
    
    spell();
    ~spell();
    
    void add_desc(char *x);
    virtual void print_stats()=0;
    
    // print to a file
    virtual void f_print(FILE *outfile)=0;

    // print to a buffer
    virtual void s_print(char *buffer)=0;
    
    // print a one line header to a file.
    virtual void f_print_header(FILE *outfile)=0;
    
    // read from file
    virtual void f_read(FILE *infile)=0;
    
    // find and read in description
    void get_desc();
    void kill_desc();

    // search for a string in the description
    // if ignore_case,, have string in upper case to save time!
    boolean desc_search(char *str,boolean ignore_case);
};

// a mage spell
class magespell: public spell {
public:
	char *school;
	char *components;
	char *duration;
	char *casttime;
	char *save;
	boolean reversible;

	magespell();
	~magespell();

	void print_stats();
	void f_print(FILE *outfile);
	void s_print(char *buffer);
	void f_print_header(FILE *outfile);
	void f_read(FILE *infile);
};

// a priest spell
class priestspell: public magespell {
public:
	char *sphere;

	priestspell();
	~priestspell();

	void print_stats();
	void f_print(FILE *outfile);
	void s_print(char *buffer);
	void f_print_header(FILE *outfile);
	void f_read(FILE *infile);
};

class spelllist {
public:
	spell *s;
	spelllist *next;
	spelllist *prev;

	spelllist() {s=NULL; next=NULL; prev=NULL;};
	spelllist(spell *x) {s=x; next=NULL; prev=NULL;};
};

// a list of spells
class spellbook {
public:
    spelllist *first;
    spelllist *last;

    char *name;

    spellbook() {first=last=NULL; name = NULL;};
    spellbook(spellbook &s);
    ~spellbook();

    // for adding spells to books
    spelllist* add_spell(spell &x, spelllist *where=NULL);  // null= at beginning.
    spellbook& operator +=(spell &x);  // at end.
    spellbook& operator +=(spellbook &s);  // at end.

    // for deleting spells from books
    void del_spell(spelllist *sl);  // dangerous!  may affect other books!
    void del_spell(spell &x);  // find and delete 1st copy of spell from book
    spellbook& operator -=(spell &x);  // find and delete 1st copy of spell.
    spellbook& operator -=(spellbook &s); // find and delete 1 copy of each
                                          // spell in s.

    // look up a spell by title
    spell *lookup(char *);

    // for saving books
    void print_book(char *);
    void print_abbrev(char *);
    void print_titles(char *);

    // for loading books
    boolean read_book(char *);
    // given filename and master spell list:
    boolean read_titles(char *, spellbook *);
};

// return master spellbok using ALLSPELLS
spellbook *get_master_list();
