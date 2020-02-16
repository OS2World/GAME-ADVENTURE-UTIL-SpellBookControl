/* spellbook.cpp: routines for spell and spellbook manipulation

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "splbook.hpp"
#include "general.hpp"

// get a spell description from the appropriate file
void spell::get_desc() {
    char buffer[256];
    FILE *infile;
    int lenname=strlen(name);
    
    // figure out what file the spell will be in
    sprintf(buffer,"%c%d",type,level);
    
    if ((infile=fopen(buffer,"r"))==NULL)
	return;
    
    do {
	fgets(buffer,256,infile);
    } while (strncmp(buffer,name,lenname) && !feof(infile));
    
    if (feof(infile)) {
	fclose(infile);
	return;
    }
    
    do {
	fgets(buffer,256,infile);
    } while (!feof(infile) && strncmp(buffer,"Sav",3));
    
    if (feof(infile)) {
	fclose(infile);
	return;
    }
    
    fgets(buffer,256,infile);
    fgets(buffer,256,infile);
    
    while (strncmp(buffer,"-----",5)==0) {
	add_desc(buffer);
	fgets(buffer,256,infile);
    }
    
    fclose(infile);
}

// kill off a spell's description
void spell::kill_desc() {
    int i;
    
    for (i=0; i<lines; i++)
	delete(description[i]);
    lines=0;
}

// search for a string in the description
// if ignore_case, have string in upper case to save time!
boolean spell::desc_search(char *str, boolean ignore_case) {
    int i;
    char *tmpstr;

    if (ignore_case) {
	for (i=0; i<lines; i++) {
	    tmpstr=strdup(description[i]);
	    upstr(tmpstr);
	    if (strstr(tmpstr,str)!=0) return(true);
	}
    }
    else for (i=0; i<lines; i++)
	if (strstr(description[i],str)!=0) return(true);
   
    return(false);
}

spell::spell() {
    name=source=range=area=NULL;
    type=' ';
    level=lines=0;
}

spell::~spell() {
    delete(name);
    delete(source);
    delete(range);
    delete(area);
    for (int i=0; i<lines; i++)
	delete(description[i]);
}

void spell::add_desc(char *x) {
    if (lines>255) return;

    // kluge to remove " " as a description... take out space.
    if ((strlen(x)==2) && (x[0]==' ')) {
	description[lines]=new char[strlen(x)];
	strcpy(description[lines++],x+1);
    }
    else {
	description[lines]=new char[strlen(x)+1];
	strcpy(description[lines++],x);
    }
}

magespell::magespell() {
    school=components=duration=casttime=save=NULL;
    reversible=false;
    type='M';
};

magespell::~magespell() {
    delete(school);
    delete(components);
    delete(duration);
    delete(casttime);
    delete(save);
};

void magespell::print_stats() {
    printf("%s (%s)\r\n",name,school);
    if (reversible) printf("Reversible\r\n");
    printf("  from %s\r\n",source);
    printf("Range: %s\r\n",range);
    printf("Components: %s\r\n",components);
    printf("Duration: %s\r\n",duration);
    printf("Casting Time: %s\r\n",casttime);
    printf("Area of Effect: %s\r\n",area);
    printf("Saving Throw: %s\r\n",save);
}

void magespell::f_print(FILE *outfile) {
    fprintf(outfile,"%s (%s)\n",name,school);
    if (reversible) fprintf(outfile,"Reversible\n");
    if (source) fprintf(outfile,"  from %s\n",source);
    if (level) fprintf(outfile,"Level: %d\n",level);
    if (range) fprintf(outfile,"Range: %s\n",range);
    if (components) fprintf(outfile,"Components: %s\n",components);
    if (duration) fprintf(outfile,"Duration: %s\n",duration);
    if (casttime) fprintf(outfile,"Casting Time: %s\n",casttime);
    if (area) fprintf(outfile,"Area of Effect: %s\n",area);
    if (save) fprintf(outfile,"Saving Throw: %s\n",save);
    fprintf(outfile,"\n");
    for (int i=0; i<lines; i++)
	fprintf(outfile,"%s\n",description[i]);
}

void magespell::s_print(char *buffer) {
    int i;

    sprintf(buffer,"%s (%s)\n",name,school);
    i=strlen(buffer);
    if (reversible) sprintf(buffer+i,"Reversible\n");
    i=strlen(buffer);
    if (source) sprintf(buffer+i,"  from %s\n",source);
    i=strlen(buffer);
    if (level) sprintf(buffer+i,"Level: %d\n",level);
    i=strlen(buffer);
    if (range) sprintf(buffer+i,"Range: %s\n",range);
    i=strlen(buffer);
    if (components) sprintf(buffer+i,"Components: %s\n",components);
    i=strlen(buffer);
    if (duration) sprintf(buffer+i,"Duration: %s\n",duration);
    i=strlen(buffer);
    if (casttime) sprintf(buffer+i,"Casting Time: %s\n",casttime);
    i=strlen(buffer);
    if (area) sprintf(buffer+i,"Area of Effect: %s\n",area);
    i=strlen(buffer);
    if (save) sprintf(buffer+i,"Saving Throw: %s\n",save);
    i=strlen(buffer);
    sprintf(buffer+i," \n");
    i=strlen(buffer);
    for (int j=0; j<lines; j++) {
	if ((strlen(description[j])==0) || 
	    ((strlen(description[j])==1) && (description[j][0]==' '))) 
	    sprintf(buffer+i," \n");
	else {
	    sprintf(buffer+i,"%s ",description[j]);
	    i=strlen(buffer);
	    if ((j<lines-1) && 
		((description[j+1][0]==' ') || 
		 (strlen(description[j+1])<2) ||
		 (description[j+1][0]=='\t')))
		sprintf(buffer+i," \n");
	}
	i=strlen(buffer);
    }
}

void magespell::f_print_header(FILE *outfile) {
    fprintf(outfile,"%17.17s ",name);
    if (reversible) fprintf(outfile,"(R) ");
    else fprintf(outfile,"    ");
    fprintf(outfile,"%5.5s ",casttime);
    fprintf(outfile,"%7.7s ",components);
    fprintf(outfile,"%15.15s ",range);
    fprintf(outfile,"%15.15s ",duration);
    fprintf(outfile,"%15.15s ",area);
    fprintf(outfile,"%4.4s ",save);
    fprintf(outfile,"\n");
}

void magespell::f_read(FILE *infile) {
    char buffer[256];
    
    do {
	fgets(buffer,256,infile);
    } while ((strlen(buffer)<=1) && !feof(infile));
    if (feof(infile)) return;
    if (strlen(buffer)) buffer[strlen(buffer)-1]=(char)0;

    char *l=strrchr(buffer,'(');
    char *m=strrchr(buffer,')');
    if ((m==NULL) || (l==NULL))
	name = strdup(buffer);
    else {
	int namelen=strlen(buffer)-strlen(l);
	name=new char[namelen];
	name[namelen-1]=(char)0;
	strncpy(name,buffer,namelen-1);
	int schoollen=strlen(l)-strlen(m);
	school=new char[schoollen];
	school[schoollen-1]=(char)0;
	strncpy(school,l+1,schoollen-1);
    }
    fgets(buffer,256,infile);
    if (strncmp(buffer,"Reversible",10)==0) {
	reversible=true;
	fgets(buffer,256,infile);
    }
    while ((strlen(buffer)>2) && !feof(infile)) {
	if (strlen(buffer)) buffer[strlen(buffer)-1]=(char)0;
	
	if (strstr(buffer,"Range:"))
	    range=strdup(buffer+7);
	else if (strstr(buffer,"Components:"))
	    components=strdup(buffer+12);
	else if (strstr(buffer,"Duration:"))
	    duration=strdup(buffer+10);
	else if (strstr(buffer,"Casting Time:"))
	    casttime=strdup(buffer+14);
	else if (strstr(buffer,"Area of Effect:"))
	    area=strdup(buffer+16);
	else if (strstr(buffer,"Saving Throw:"))
	    save=strdup(buffer+14);
	else if (strstr(buffer,"School:"))
	    school=strdup(buffer+8);
	else if (strstr(buffer,"  from "))
	    source=strdup(buffer+7);
	else if (strstr(buffer,"Level:"))
	    sscanf(buffer+7,"%d",&level);
	
	fgets(buffer,256,infile);
    }
    fgets(buffer,256,infile);
    while ((strncmp(buffer,"-----",5)!=0) && !feof(infile)) {
	if (strlen(buffer)) buffer[strlen(buffer)-1]=(char)0;
	add_desc(buffer);
	fgets(buffer,256,infile);
    }
}

priestspell::priestspell() {
    sphere=NULL;
    type='P';
};

priestspell::~priestspell() {
    delete(sphere);
};

void priestspell::print_stats() {
    printf("%s (%s)\r\n",name,school);
    if (reversible) printf("Reversible\r\n");
    printf("  from %s\r\n",source);
    printf("Sphere: %s\r\n",sphere);
    printf("Range: %s\r\n",range);
    printf("Components: %s\r\n",components);
    printf("Duration: %s\r\n",duration);
    printf("Casting Time: %s\r\n",casttime);
    printf("Area of Effect: %s\r\n",area);
    printf("Saving Throw: %s\r\n",save);
}

void priestspell::f_print(FILE *outfile) {
    fprintf(outfile,"%s (%s)\n",name,school);
    if (reversible) fprintf(outfile,"Reversible\n");
    fprintf(outfile,"  from %s\n",source);
    fprintf(outfile,"Sphere:  %s\n",sphere);
    fprintf(outfile,"Range: %s\n",range);
    fprintf(outfile,"Components: %s\n",components);
    fprintf(outfile,"Duration: %s\n",duration);
    fprintf(outfile,"Casting Time: %s\n",casttime);
    fprintf(outfile,"Area of Effect: %s\n",area);
    fprintf(outfile,"Saving Throw: %s\n\n",save);
    for (int i=0; i<lines; i++)
	fprintf(outfile,"%s\n",description[i]);
}

void priestspell::s_print(char *buffer) {
    int i;

    sprintf(buffer,"%s (%s)\n",name,school);
    i=strlen(buffer);
    if (reversible) sprintf(buffer+i,"Reversible\n");
    i=strlen(buffer);
    if (source) sprintf(buffer+i,"  from %s\n",source);
    i=strlen(buffer);
    if (level) sprintf(buffer+i,"Level: %d\n",level);
    i=strlen(buffer);
    if (sphere) sprintf(buffer+i,"Sphere: %s\n",sphere);
    i=strlen(buffer);
    if (range) sprintf(buffer+i,"Range: %s\n",range);
    i=strlen(buffer);
    if (components) sprintf(buffer+i,"Components: %s\n",components);
    i=strlen(buffer);
    if (duration) sprintf(buffer+i,"Duration: %s\n",duration);
    i=strlen(buffer);
    if (casttime) sprintf(buffer+i,"Casting Time: %s\n",casttime);
    i=strlen(buffer);
    if (area) sprintf(buffer+i,"Area of Effect: %s\n",area);
    i=strlen(buffer);
    if (save) sprintf(buffer+i,"Saving Throw: %s\n",save);
    i=strlen(buffer);
    sprintf(buffer+i," \n");
    i=strlen(buffer);
    for (int j=0; j<lines; j++) {
	if ((strlen(description[j])==0) || 
	    ((strlen(description[j])==1) && (description[j][0]==' '))) 
	    sprintf(buffer+i," \n");
	else {
	    sprintf(buffer+i,"%s ",description[j]);
	    i=strlen(buffer);
	    if ((j<lines-1) && 
		((description[j+1][0]==' ') || 
		 (strlen(description[j+1])<2) ||
		 (description[j+1][0]=='\t')))
		sprintf(buffer+i," \n");
	}
	i=strlen(buffer);
    }
}

void priestspell::f_print_header(FILE *outfile) {
    fprintf(outfile,"%17.17s ",name);
    if (reversible) fprintf(outfile,"(R) ");
    else fprintf(outfile,"    ");
    fprintf(outfile,"15.15s ",school);
    fprintf(outfile,"%5.5s ",casttime);
    fprintf(outfile,"%7.7s ",components);
    fprintf(outfile,"%15.15s ",range);
    fprintf(outfile,"%15.15s ",duration);
    fprintf(outfile,"%15.15s ",area);
    fprintf(outfile,"%4.4s ",save);
    fprintf(outfile,"\n");
}

void priestspell::f_read(FILE *infile) {
    char buffer[256];
    
    do {
	fgets(buffer,256,infile);
    } while ((strlen(buffer)<=1) && !feof(infile));
    if (feof(infile)) return;
    if (strlen(buffer)) buffer[strlen(buffer)-1]=(char)0;

    char *l=strrchr(buffer,'(');
    char *m=strrchr(buffer,')');
    if ((m==NULL) || (l==NULL))
	name = strdup(buffer);
    else {
	int namelen=strlen(buffer)-strlen(l);
	name=new char[namelen];
	name[namelen-1]=(char)0;
	strncpy(name,buffer,namelen-1);
	int schoollen=strlen(l)-strlen(m);
	school=new char[schoollen];
	school[schoollen-1]=(char)0;
	strncpy(school,l+1,schoollen-1);
    }
    fgets(buffer,256,infile);
    if (strncmp(buffer,"Reversible",10)==0) {
	reversible=true;
	fgets(buffer,256,infile);
    }
    while ((strlen(buffer)>2) && !feof(infile)) {
	if (strlen(buffer)) buffer[strlen(buffer)-1]=(char)0;
	
	if (strstr(buffer,"Range:"))
	    range=strdup(buffer+7);
	else if (strstr(buffer,"Components:"))
	    components=strdup(buffer+12);
	else if (strstr(buffer,"Duration:"))
	    duration=strdup(buffer+10);
	else if (strstr(buffer,"Casting Time:"))
	    casttime=strdup(buffer+14);
	else if (strstr(buffer,"Area of Effect:"))
	    area=strdup(buffer+16);
	else if (strstr(buffer,"Saving Throw:"))
	    save=strdup(buffer+14);
	else if (strstr(buffer,"Sphere:"))
	    sphere=strdup(buffer+8);
	else if (strstr(buffer,"School:"))
	    school=strdup(buffer+8);
	else if (strstr(buffer,"  from "))
	    source=strdup(buffer+7);
	else if (strstr(buffer,"Level:"))
	    sscanf(buffer+7,"%d",&level);
	
	fgets(buffer,256,infile);
    }
    fgets(buffer,256,infile);
    while ((strncmp(buffer,"-----",5)!=0) && !feof(infile)) {
	if (strlen(buffer)) buffer[strlen(buffer)-1]=(char)0;
	add_desc(buffer);
	fgets(buffer,256,infile);
    }
}

void priest_to_mage(magespell &y, priestspell &x) {
  if (x.name) y.name=strdup(x.name);
  y.level=x.level;
  if (x.range) y.range=strdup(x.range);
  if (x.area) y.area=strdup(x.area);
  if (x.source) y.source=strdup(x.source);
  if (x.school) y.school=strdup(x.school);
  if (x.duration) y.duration=strdup(x.duration);
  if (x.components) y.components=strdup(x.components);
  if (x.casttime) y.casttime=strdup(x.casttime);
  if (x.save) y.save=strdup(x.save);
  y.reversible=x.reversible;
  for (int i=0; i<x.lines; i++)
    y.add_desc(x.description[i]);
}

spellbook::spellbook(spellbook &s) {
  first=last=NULL;
  if (s.name) name=strdup(s.name);
  else name=NULL;
  *this += s;
}

spellbook::~spellbook() {
  spelllist *i=first;
  spelllist *j;
    
  while (i!=NULL) {
    j=i->next;
    delete i;
	i=j;
  }
  delete name;
}

spelllist *spellbook::add_spell(spell &x, spelllist *where) {
    spelllist *sl;

    sl=new spelllist;

    // spell is given.
    sl->s=&x;

    if (where==NULL) {   // add at end.
	sl->next=NULL;
	sl->prev=last;
	if (last!=NULL) last->next=sl;
	if (first==NULL) first=sl;
	last=sl;
    } else {  // add after "where"
	sl->next=where->next;
	sl->prev=where;
	if (where->next!=NULL) where->next->prev=sl;
	where->next=sl;
	if (where==last) last=sl;
    }
    return(sl);
}


// delete a spell given its spelllist entry
void spellbook::del_spell(spelllist *sl) {
  if (sl->next!=NULL) sl->next->prev=sl->prev;
  if (sl->prev!=NULL) sl->prev->next=sl->next;
  if (first==sl) first=sl->next;
  if (last==sl) last=sl->prev;
  delete sl;
}

// find and delete one reference to spell x in book
void spellbook::del_spell(spell &x) {
  spelllist *i;
  
  for (i=first; i!=NULL; i=i->next) 
    if (i->s==&x) {
      del_spell(i);
      return;
    }

}

spellbook& spellbook::operator +=(spell &x) {
    add_spell(x);
    return(*this);
}

spellbook& spellbook::operator +=(spellbook &x) {
    for (spelllist *i=x.first; i!=NULL; i=i->next) 
	add_spell(*(i->s));
    return(*this);
}

spellbook& spellbook::operator -=(spell &x) {
  del_spell(x);
  return(*this);
}

spellbook& spellbook::operator -=(spellbook &x) {
    for (spelllist *i=x.first; i!=NULL; i=i->next) 
	del_spell(*(i->s));
    return(*this);
}

boolean spellbook::read_book(char *filename) {
    FILE *infile;
    fpos_t pos;
    char buffer[256];

    // clear out old book, if present.
    this->~spellbook();
    first=last=NULL;
    if ((infile=fopen(filename,"r"))==NULL) return(false);

    // get spellbook name from first line of file, if there
    fgetpos(infile,&pos);
    fgets(buffer,256,infile);
    if (strlen(buffer)) buffer[strlen(buffer)-1]=(char)0;
    if (strstr(buffer,"Title:  ")==NULL) {
	name=NULL;
	fsetpos(infile,&pos);
    }
    else name=strdup(buffer+8);

    while (!feof(infile)) {
      priestspell *x;
      x = new priestspell;
      x->f_read(infile);
      if (x->sphere) add_spell(*x);
      else {  // was actually a magespell.
	magespell *y;
	y=new magespell;
	priest_to_mage(*y,*x);
	delete x;
	add_spell(*y);
      }
    }
    fclose(infile);
    return(true);
}


// look up a spell by title.
spell *spellbook::lookup(char *sname) {
    for (spelllist *i=first; i!=NULL; i=i->next) 
	if (strcmp(i->s->name,sname)==0) return(i->s);
    return(NULL);
}

// read spellbook from list of spells
boolean spellbook::read_titles(char *filename, spellbook *masterlist) {
    FILE *infile;
    fpos_t pos;
    char buffer[256];

    // clear out old book, if present.
    this->~spellbook();
    first=last=NULL;
    if ((infile=fopen(filename,"r"))==NULL) return(false);

    // get spellbook name from first line of file, if there
    fgetpos(infile,&pos);
    fgets(buffer,256,infile);
    if (strlen(buffer)) buffer[strlen(buffer)-1]=(char)0;
    if (strstr(buffer,"Title:  ")==NULL) {
	name=NULL;
	fsetpos(infile,&pos);
    }
    else name=strdup(buffer+8);

    while (!feof(infile)) {
	fgets(buffer,256,infile);
	if (!feof(infile)) {
	    spell *ns;
	    buffer[strlen(buffer)-1]=(char)0;
	    ns=masterlist->lookup(buffer);
	    if (ns!=NULL) add_spell(*ns);
	}
    }
}

void spellbook::print_titles(char *filename) {
    FILE *outfile;
    
    if ((outfile=fopen(filename,"w"))==NULL) {
	error("Can't open output file %s",filename);
	return;
    }
    
    if (name) fprintf(outfile,"Title:  %s\n",name);
    else fprintf(outfile,"\n");
    for (spelllist *i=first; i!=NULL; i=i->next)
	fprintf(outfile,"%s\n",i->s->name);
    
    fclose(outfile);
}

// save an entire spellbook 
void spellbook::print_book(char *filename) {
    FILE *outfile;
    
    if ((outfile=fopen(filename,"w"))==NULL) {
	error("Can't open output file %s",filename);
	return;
    }
    
    if (name) fprintf(outfile,"Title:  %s\n",name);
    for (spelllist *i=first; i!=NULL; i=i->next) {
	i->s->f_print(outfile);
	if (i->next!=NULL) fprintf(outfile,"-----\n");
    }
    
    fclose(outfile);
}

// print out abbreviated spellbook
void spellbook::print_abbrev(char *filename) {
    FILE *outfile;
    
    if ((outfile=fopen(filename,"w"))==NULL) {
	error("Can't open output file %s",filename);
	return;
    }
    
    if (name) fprintf(outfile,"%s\n",name);
    else fprintf(outfile,"\n");
    for (spelllist *i=first; i!=NULL; i=i->next) {
	i->s->f_print_header(outfile);
    }
    
    fclose(outfile);
}


// get master list of spells from ALLSPELLS
spellbook *get_master_list() {
    spellbook *ml;
    spellbook *x;
    char buffer[256];
    FILE *infile;

    ml=new spellbook;    
    if (ml==NULL) return(NULL);
    ml->name=strdup("Master list of spells");
    x=new spellbook;
    if (x==NULL) return(NULL);

    if ((infile=fopen(ALLSPELLS,"r"))==NULL) return(NULL);

    while (!feof(infile)) {
	fgets(buffer,256,infile);
	if ((!feof(infile)) && (buffer[0]!=';')) {
	    x->read_book(buffer);
	    (*ml)+=(*x);
	}
    }
    
    return(ml);
}

