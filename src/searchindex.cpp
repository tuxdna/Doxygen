/******************************************************************************
 *
 * 
 *
 * Copyright (C) 1997-2012 by Dimitri van Heesch.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation under the terms of the GNU General Public License is hereby 
 * granted. No representations are made about the suitability of this software 
 * for any purpose. It is provided "as is" without express or implied warranty.
 * See the GNU General Public License for more details.
 *
 * Documents produced by Doxygen are derivative works derived from the
 * input used in their production; they are not affected by this license.
 *
 */

#include "qtbc.h"
#include "searchindex.h"
#include "config.h"
#include "util.h"
#include <qfile.h>
#include <ctype.h>
#include <qregexp.h>
#include <fstream>


// file format: (all multi-byte values are stored in big endian format)
//   4 byte header
//   256*256*4 byte index (4 bytes)
//   for each index entry: a zero terminated list of words 
//   for each word: a \0 terminated string + 4 byte offset to the stats info
//   padding bytes to align at 4 byte boundary
//   for each word: the number of urls (4 bytes) 
//               + for each url containing the word 8 bytes statistics
//                 (4 bytes index to url string + 4 bytes frequency counter)
//   for each url: a \0 terminated string

const int numIndexEntries = 256*256;

//--------------------------------------------------------------------

IndexWord::IndexWord(const char *word) : m_word(word), m_urls(17)
{
  m_urls.setAutoDelete(TRUE);
  //printf("IndexWord::IndexWord(%s)\n",word);
}

void IndexWord::addUrlIndex(int idx,bool hiPriority)
{
  //printf("IndexWord::addUrlIndex(%d,%d)\n",idx,hiPriority);
  URLInfo *ui = m_urls.find(idx);
  if (ui==0)
  {
    //printf("URLInfo::URLInfo(%d)\n",idx);
    ui=new URLInfo(idx,0);
    m_urls.insert(idx,ui);
  }
  ui->freq+=2;
  if (hiPriority) ui->freq|=1; // mark as high priority document
}

//--------------------------------------------------------------------

SearchIndex::SearchIndex() : m_words(328829), m_index(numIndexEntries), m_urlIndex(-1)
{
  int i;
  m_words.setAutoDelete(TRUE);
  m_urls.setAutoDelete(TRUE);
  m_index.setAutoDelete(TRUE);
  for (i=0;i<numIndexEntries;i++) m_index.insert(i,new QList<IndexWord>);
}

void SearchIndex::setCurrentDoc(const char *name,const char *baseName,const char *anchor)
{
  if (name==0 || baseName==0) return;
  //printf("SearchIndex::setCurrentDoc(%s,%s,%s)\n",name,baseName,anchor);
  QCString url=baseName+Config_getString("HTML_FILE_EXTENSION");
  if (anchor) url+=(QCString)"#"+anchor;  
  m_urlIndex++;
  m_urls.insert(m_urlIndex,new URL(name,url));
}

static int charsToIndex(const char *word)
{
  if (word==0) return -1;

  // Fast string hashing algorithm
  //register ushort h=0;
  //const char *k = word;
  //ushort mask=0xfc00;
  //while ( *k ) 
  //{
  //  h = (h&mask)^(h<<6)^(*k++);
  //}
  //return h;

  // Simple hashing that allows for substring searching
  uint c1=word[0];
  if (c1==0) return -1;
  uint c2=word[1];
  if (c2==0) return -1;
  return c1*256+c2;
}

void SearchIndex::addWord(const char *word,bool hiPriority,bool recurse)
{
  static QRegExp nextPart("[_a-z:][A-Z]");
  //printf("SearchIndex::addWord(%s,%d)\n",word,hiPriority);
  QString wStr(word);
  if (wStr.isEmpty()) return;
  wStr=wStr.lower();
  IndexWord *w = m_words[wStr];
  if (w==0)
  {
    int idx=charsToIndex(wStr);
    if (idx<0) return;
    w = new IndexWord(wStr);
    //fprintf(stderr,"addWord(%s) at index %d\n",word,idx);
    m_index[idx]->append(w);
    m_words.insert(wStr,w);
  }
  w->addUrlIndex(m_urlIndex,hiPriority);
  int i;
  bool found=FALSE;
  if (!recurse) // the first time we check if we can strip the prefix
  {
    i=getPrefixIndex(word);
    if (i>0)
    {
      addWord(word+i,hiPriority,TRUE);
      found=TRUE;
    }
  }
  if (!found) // no prefix stripped
  {
    if ((i=nextPart.match(word))>=1)
    {
      addWord(word+i+1,hiPriority,TRUE);
    }
  }
}


static void writeInt(QFile &f,int index)
{
  f.putch(((uint)index)>>24);
  f.putch((((uint)index)>>16)&0xff);
  f.putch((((uint)index)>>8)&0xff);
  f.putch(((uint)index)&0xff);
}

static void writeString(QFile &f,const char *s)
{
  const char *p = s;
  while (*p) f.putch(*p++);
  f.putch(0);
}

void SearchIndex::write(const char *fileName)
{
  int i;
  int size=4; // for the header
  size+=4*numIndexEntries; // for the index
  int wordsOffset = size;
  // first pass: compute the size of the wordlist
  for (i=0;i<numIndexEntries;i++)
  {
    QList<IndexWord> *wlist = m_index[i];
    if (!wlist->isEmpty())
    {
      QListIterator<IndexWord> iwi(*wlist);
      IndexWord *iw;
      for (iwi.toFirst();(iw=iwi.current());++iwi)
      {
        int ws = iw->word().length()+1; 
        size+=ws+4; // word + url info list offset
      }
      size+=1; // zero list terminator
    }
  }

  // second pass: compute the offsets in the index
  int indexOffsets[numIndexEntries];
  int offset=wordsOffset;
  for (i=0;i<numIndexEntries;i++)
  {
    QList<IndexWord> *wlist = m_index[i];
    if (!wlist->isEmpty())
    {
      indexOffsets[i]=offset;
      QListIterator<IndexWord> iwi(*wlist);
      IndexWord *iw;
      for (iwi.toFirst();(iw=iwi.current());++iwi)
      {
        offset+= iw->word().length()+1; 
        offset+=4; // word + offset to url info array 
      }
      offset+=1; // zero list terminator
    }
    else
    {
      indexOffsets[i]=0;
    }
  }
  int padding = size;
  size = (size+3)&~3; // round up to 4 byte boundary
  padding = size - padding;

  //int statsOffset = size;
  QDictIterator<IndexWord> wdi(m_words);
  //IndexWord *iw;
  int *wordStatOffsets = new int[m_words.count()];
  
  int count=0;

  // third pass: compute offset to stats info for each word
  for (i=0;i<numIndexEntries;i++)
  {
    QList<IndexWord> *wlist = m_index[i];
    if (!wlist->isEmpty())
    {
      QListIterator<IndexWord> iwi(*wlist);
      IndexWord *iw;
      for (iwi.toFirst();(iw=iwi.current());++iwi)
      {
        //printf("wordStatOffsets[%d]=%d\n",count,size);
        wordStatOffsets[count++] = size;
        size+=4+iw->urls().count()*8; // count + (url_index,freq) per url
      }
    }
  }
  int *urlOffsets = new int[m_urls.count()];
  //int urlsOffset = size;
  QIntDictIterator<URL> udi(m_urls);
  URL *url;
  for (udi.toFirst();(url=udi.current());++udi)
  {
    urlOffsets[udi.currentKey()]=size;
    size+=url->name.length()+1+
          url->url.length()+1;
  }
  //printf("Total size %x bytes (word=%x stats=%x urls=%x)\n",size,wordsOffset,statsOffset,urlsOffset);
  QFile f(fileName);
  if (f.open(IO_WriteOnly))
  {
    // write header
    f.putch('D'); f.putch('O'); f.putch('X'); f.putch('S');
    // write index
    for (i=0;i<numIndexEntries;i++)
    {
      writeInt(f,indexOffsets[i]);
    }
    // write word lists
    count=0;
    for (i=0;i<numIndexEntries;i++)
    {
      QList<IndexWord> *wlist = m_index[i];
      if (!wlist->isEmpty())
      {
        QListIterator<IndexWord> iwi(*wlist);
        IndexWord *iw;
        for (iwi.toFirst();(iw=iwi.current());++iwi)
        {
          writeString(f,iw->word());
          writeInt(f,wordStatOffsets[count++]);
        }
        f.putch(0);
      }
    }
    // write extra padding bytes
    for (i=0;i<padding;i++) f.putch(0);
    // write word statistics
    for (i=0;i<numIndexEntries;i++)
    {
      QList<IndexWord> *wlist = m_index[i];
      if (!wlist->isEmpty())
      {
        QListIterator<IndexWord> iwi(*wlist);
        IndexWord *iw;
        for (iwi.toFirst();(iw=iwi.current());++iwi)
        {
          int numUrls = iw->urls().count();
          writeInt(f,numUrls);
          QIntDictIterator<URLInfo> uli(iw->urls());
          URLInfo *ui;
          for (uli.toFirst();(ui=uli.current());++uli)
          {
            writeInt(f,urlOffsets[ui->urlIdx]);
            writeInt(f,ui->freq);
          }
        }
      }
    }
    // write urls
    QIntDictIterator<URL> udi(m_urls);
    URL *url;
    for (udi.toFirst();(url=udi.current());++udi)
    {
      writeString(f,url->name);
      writeString(f,url->url);
    }
  }

  delete[] urlOffsets;
  delete[] wordStatOffsets;
}

//---------------------------------------------------------------------------
// the following part is for the javascript based search engine

#include "memberdef.h"
#include "namespacedef.h"
#include "classdef.h"
#include "filedef.h"
#include "language.h"
#include "doxygen.h"
#include "message.h"

static const char search_script[]=
#include "search_js.h"
;

#define MEMBER_INDEX_ENTRIES   256
#define NUM_SEARCH_INDICES      13
#define SEARCH_INDEX_ALL         0
#define SEARCH_INDEX_CLASSES     1
#define SEARCH_INDEX_NAMESPACES  2
#define SEARCH_INDEX_FILES       3
#define SEARCH_INDEX_FUNCTIONS   4
#define SEARCH_INDEX_VARIABLES   5
#define SEARCH_INDEX_TYPEDEFS    6
#define SEARCH_INDEX_ENUMS       7
#define SEARCH_INDEX_ENUMVALUES  8
#define SEARCH_INDEX_PROPERTIES  9
#define SEARCH_INDEX_EVENTS     10
#define SEARCH_INDEX_RELATED    11
#define SEARCH_INDEX_DEFINES    12

class SearchIndexList : public SDict< QList<Definition> >
{
  public:
    SearchIndexList(int size=17) : SDict< QList<Definition> >(size,FALSE) 
    {
      setAutoDelete(TRUE);
    }
   ~SearchIndexList() {}
    void append(Definition *d)
    {
      QList<Definition> *l = find(d->name());
      if (l==0)
      {
        l=new QList<Definition>;
        SDict< QList<Definition> >::append(d->name(),l);
      }
      l->append(d);
    }
    int compareItems(GCI item1, GCI item2)
    {
      QList<Definition> *md1=(QList<Definition> *)item1;
      QList<Definition> *md2=(QList<Definition> *)item2;
      QCString n1 = md1->first()->localName();
      QCString n2 = md2->first()->localName();
      return stricmp(n1.data(),n2.data());
    }
};

static void addMemberToSearchIndex(
         SearchIndexList symbols[NUM_SEARCH_INDICES][MEMBER_INDEX_ENTRIES],
         int symbolCount[NUM_SEARCH_INDICES],
         MemberDef *md)
{
  static bool hideFriendCompounds = Config_getBool("HIDE_FRIEND_COMPOUNDS");
  bool isLinkable = md->isLinkable();
  ClassDef *cd=0;
  NamespaceDef *nd=0;
  FileDef *fd=0;
  GroupDef *gd=0;
  if (isLinkable             && 
      (
       ((cd=md->getClassDef()) && cd->isLinkable() && cd->templateMaster()==0) ||
       ((gd=md->getGroupDef()) && gd->isLinkable())
      )
     )
  {
    QCString n = md->name();
    uchar charCode = (uchar)n.at(0);
    uint letter = charCode<128 ? tolower(charCode) : charCode;
    if (!n.isEmpty()) 
    {
      bool isFriendToHide = hideFriendCompounds &&
        (QCString(md->typeString())=="friend class" || 
         QCString(md->typeString())=="friend struct" ||
         QCString(md->typeString())=="friend union");
      if (!(md->isFriend() && isFriendToHide))
      {
        symbols[SEARCH_INDEX_ALL][letter].append(md);
        symbolCount[SEARCH_INDEX_ALL]++;
      }
      if (md->isFunction() || md->isSlot() || md->isSignal())
      {
        symbols[SEARCH_INDEX_FUNCTIONS][letter].append(md);
        symbolCount[SEARCH_INDEX_FUNCTIONS]++;
      } 
      else if (md->isVariable())
      {
        symbols[SEARCH_INDEX_VARIABLES][letter].append(md);
        symbolCount[SEARCH_INDEX_VARIABLES]++;
      }
      else if (md->isTypedef())
      {
        symbols[SEARCH_INDEX_TYPEDEFS][letter].append(md);
        symbolCount[SEARCH_INDEX_TYPEDEFS]++;
      }
      else if (md->isEnumerate())
      {
        symbols[SEARCH_INDEX_ENUMS][letter].append(md);
        symbolCount[SEARCH_INDEX_ENUMS]++;
      }
      else if (md->isEnumValue())
      {
        symbols[SEARCH_INDEX_ENUMVALUES][letter].append(md);
        symbolCount[SEARCH_INDEX_ENUMVALUES]++;
      }
      else if (md->isProperty())
      {
        symbols[SEARCH_INDEX_PROPERTIES][letter].append(md);
        symbolCount[SEARCH_INDEX_PROPERTIES]++;
      }
      else if (md->isEvent())
      {
        symbols[SEARCH_INDEX_EVENTS][letter].append(md);
        symbolCount[SEARCH_INDEX_EVENTS]++;
      }
      else if (md->isRelated() || md->isForeign() ||
               (md->isFriend() && !isFriendToHide))
      {
        symbols[SEARCH_INDEX_RELATED][letter].append(md);
        symbolCount[SEARCH_INDEX_RELATED]++;
      }
    }
  }
  else if (isLinkable && 
      (((nd=md->getNamespaceDef()) && nd->isLinkable()) || 
       ((fd=md->getFileDef())      && fd->isLinkable())
      )
     )
  {
    QCString n = md->name();
    uchar charCode = (uchar)n.at(0);
    uint letter = charCode<128 ? tolower(charCode) : charCode;
    if (!n.isEmpty()) 
    {
      symbols[SEARCH_INDEX_ALL][letter].append(md);
      symbolCount[SEARCH_INDEX_ALL]++;

      if (md->isFunction()) 
      {
        symbols[SEARCH_INDEX_FUNCTIONS][letter].append(md);
        symbolCount[SEARCH_INDEX_FUNCTIONS]++;
      }
      else if (md->isVariable()) 
      {
        symbols[SEARCH_INDEX_VARIABLES][letter].append(md);
        symbolCount[SEARCH_INDEX_VARIABLES]++;
      }
      else if (md->isTypedef())
      {
        symbols[SEARCH_INDEX_TYPEDEFS][letter].append(md);
        symbolCount[SEARCH_INDEX_TYPEDEFS]++;
      }
      else if (md->isEnumerate())
      {
        symbols[SEARCH_INDEX_ENUMS][letter].append(md);
        symbolCount[SEARCH_INDEX_ENUMS]++;
      }
      else if (md->isEnumValue())
      {
        symbols[SEARCH_INDEX_ENUMVALUES][letter].append(md);
        symbolCount[SEARCH_INDEX_ENUMVALUES]++;
      }
      else if (md->isDefine())
      {
        symbols[SEARCH_INDEX_DEFINES][letter].append(md);
        symbolCount[SEARCH_INDEX_DEFINES]++;
      }
    }
  }
}

static QCString searchId(const QCString &s)
{
  int c;
  uint i;
  QCString result;
  for (i=0;i<s.length();i++)
  {
    c=s.at(i);
    if ((c>='0' && c<='9') || (c>='A' && c<='Z') || (c>='a' && c<='z'))
    {
      result+=(char)tolower(c);
    }
    else
    {
      char val[4];
      sprintf(val,"_%02x",(uchar)c);
      result+=val;
    }
  }
  return result;
}

static int g_searchIndexCount[NUM_SEARCH_INDICES];
static SearchIndexList g_searchIndexSymbols[NUM_SEARCH_INDICES][MEMBER_INDEX_ENTRIES];
static const char *g_searchIndexName[NUM_SEARCH_INDICES] = 
{ 
    "all",
    "classes",
    "namespaces",
    "files",
    "functions",
    "variables",
    "typedefs", 
    "enums", 
    "enumvalues",
    "properties", 
    "events", 
    "related",
    "defines"
};


class SearchIndexCategoryMapping
{
  public:
    SearchIndexCategoryMapping()
    {
      categoryLabel[SEARCH_INDEX_ALL]        = theTranslator->trAll();
      categoryLabel[SEARCH_INDEX_CLASSES]    = theTranslator->trClasses();
      categoryLabel[SEARCH_INDEX_NAMESPACES] = theTranslator->trNamespace(TRUE,FALSE);
      categoryLabel[SEARCH_INDEX_FILES]      = theTranslator->trFile(TRUE,FALSE);
      categoryLabel[SEARCH_INDEX_FUNCTIONS]  = theTranslator->trFunctions();
      categoryLabel[SEARCH_INDEX_VARIABLES]  = theTranslator->trVariables();
      categoryLabel[SEARCH_INDEX_TYPEDEFS]   = theTranslator->trTypedefs();
      categoryLabel[SEARCH_INDEX_ENUMS]      = theTranslator->trEnumerations();
      categoryLabel[SEARCH_INDEX_ENUMVALUES] = theTranslator->trEnumerationValues();
      categoryLabel[SEARCH_INDEX_PROPERTIES] = theTranslator->trProperties();
      categoryLabel[SEARCH_INDEX_EVENTS]     = theTranslator->trEvents();
      categoryLabel[SEARCH_INDEX_RELATED]    = theTranslator->trFriends();
      categoryLabel[SEARCH_INDEX_DEFINES]    = theTranslator->trDefines();
    }
    QCString categoryLabel[NUM_SEARCH_INDICES];
};

void writeJavascriptSearchIndex()
{
  if (!Config_getBool("GENERATE_HTML")) return;

  // index classes
  ClassSDict::Iterator cli(*Doxygen::classSDict);
  ClassDef *cd;
  for (;(cd=cli.current());++cli)
  {
    uchar charCode = (uchar)cd->localName().at(0);
    uint letter = charCode<128 ? tolower(charCode) : charCode;
    if (cd->isLinkable() && isId(letter))
    {
      g_searchIndexSymbols[SEARCH_INDEX_ALL][letter].append(cd);
      g_searchIndexSymbols[SEARCH_INDEX_CLASSES][letter].append(cd);
      g_searchIndexCount[SEARCH_INDEX_ALL]++;
      g_searchIndexCount[SEARCH_INDEX_CLASSES]++;
    }
  }

  // index namespaces
  NamespaceSDict::Iterator nli(*Doxygen::namespaceSDict);
  NamespaceDef *nd;
  for (;(nd=nli.current());++nli)
  {
    uchar charCode = (uchar)nd->name().at(0);
    uint letter = charCode<128 ? tolower(charCode) : charCode;
    if (nd->isLinkable() && isId(letter))
    {
      g_searchIndexSymbols[SEARCH_INDEX_ALL][letter].append(nd);
      g_searchIndexSymbols[SEARCH_INDEX_NAMESPACES][letter].append(nd);
      g_searchIndexCount[SEARCH_INDEX_ALL]++;
      g_searchIndexCount[SEARCH_INDEX_NAMESPACES]++;
    }
  }

  // index files
  FileNameListIterator fnli(*Doxygen::inputNameList);
  FileName *fn;
  for (;(fn=fnli.current());++fnli)
  {
    FileNameIterator fni(*fn);
    FileDef *fd;
    for (;(fd=fni.current());++fni)
    {
      uchar charCode = (uchar)fd->name().at(0);
      uint letter = charCode<128 ? tolower(charCode) : charCode;
      if (fd->isLinkable() && isId(letter))
      {
        g_searchIndexSymbols[SEARCH_INDEX_ALL][letter].append(fd);
        g_searchIndexSymbols[SEARCH_INDEX_FILES][letter].append(fd);
        g_searchIndexCount[SEARCH_INDEX_ALL]++;
        g_searchIndexCount[SEARCH_INDEX_FILES]++;
      }
    }
  }

  // index class members
  {
    MemberNameSDict::Iterator mnli(*Doxygen::memberNameSDict);
    MemberName *mn;
    // for each member name
    for (mnli.toFirst();(mn=mnli.current());++mnli)
    {
      MemberDef *md;
      MemberNameIterator mni(*mn);
      // for each member definition
      for (mni.toFirst();(md=mni.current());++mni)
      {
        addMemberToSearchIndex(g_searchIndexSymbols,g_searchIndexCount,md);
      }
    }
  }

  // index file/namespace members
  {
    MemberNameSDict::Iterator fnli(*Doxygen::functionNameSDict);
    MemberName *mn;
    // for each member name
    for (fnli.toFirst();(mn=fnli.current());++fnli)
    {
      MemberDef *md;
      MemberNameIterator mni(*mn);
      // for each member definition
      for (mni.toFirst();(md=mni.current());++mni)
      {
        addMemberToSearchIndex(g_searchIndexSymbols,g_searchIndexCount,md);
      }
    }
  }
  
  // sort all lists
  int i,p;
  for (i=0;i<NUM_SEARCH_INDICES;i++)
  {
    for (p=0;p<MEMBER_INDEX_ENTRIES;p++)
    {
      if (g_searchIndexSymbols[i][p].count()>0)
      {
        g_searchIndexSymbols[i][p].sort();
      }
    }
  }

  // write index files
  QCString searchDirName = Config_getString("HTML_OUTPUT")+"/search";

  for (i=0;i<NUM_SEARCH_INDICES;i++)
  {
    for (p=0;p<MEMBER_INDEX_ENTRIES;p++)
    {
      if (g_searchIndexSymbols[i][p].count()>0)
      {
        QCString baseName;
        baseName.sprintf("%s_%02x",g_searchIndexName[i],p);

        QCString fileName = searchDirName + "/"+baseName+".html";
        QCString dataFileName = searchDirName + "/"+baseName+".js";

        QFile outFile(fileName);
        QFile dataOutFile(dataFileName);
        if (outFile.open(IO_WriteOnly) && dataOutFile.open(IO_WriteOnly))
        {
          {
            FTextStream t(&outFile);

            t << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\""
              " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">" << endl;
            t << "<html><head><title></title>" << endl;
            t << "<meta http-equiv=\"Content-Type\" content=\"text/xhtml;charset=UTF-8\"/>" << endl;
            t << "<link rel=\"stylesheet\" type=\"text/css\" href=\"search.css\"/>" << endl;
            t << "<script type=\"text/javascript\" src=\"" << baseName << ".js\"></script>" << endl;
            t << "<script type=\"text/javascript\" src=\"search.js\"></script>" << endl;
            t << "</head>" << endl;
            t << "<body class=\"SRPage\">" << endl;
            t << "<div id=\"SRIndex\">" << endl;
            t << "<div class=\"SRStatus\" id=\"Loading\">" << theTranslator->trLoading() << "</div>" << endl;
            t << "<div id=\"SRResults\"></div>" << endl; // here the results will be inserted
            t << "<script type=\"text/javascript\"><!--" << endl;
            t << "createResults();" << endl; // this function will insert the results
            t << "--></script>" << endl;
            t << "<div class=\"SRStatus\" id=\"Searching\">" 
              << theTranslator->trSearching() << "</div>" << endl;
            t << "<div class=\"SRStatus\" id=\"NoMatches\">"
              << theTranslator->trNoMatches() << "</div>" << endl;

            t << "<script type=\"text/javascript\"><!--" << endl;
            t << "document.getElementById(\"Loading\").style.display=\"none\";" << endl;
            t << "document.getElementById(\"NoMatches\").style.display=\"none\";" << endl;
            t << "var searchResults = new SearchResults(\"searchResults\");" << endl;
            t << "searchResults.Search();" << endl;
            t << "--></script>" << endl;
            t << "</div>" << endl; // SRIndex
            t << "</body>" << endl;
            t << "</html>" << endl;
          }
          FTextStream ti(&dataOutFile);

          ti << "var searchData=" << endl;
          // format
          // searchData[] = array of items
          // searchData[x][0] = id
          // searchData[x][1] = [ name + child1 + child2 + .. ]
          // searchData[x][1][0] = name as shown
          // searchData[x][1][y+1] = info for child y
          // searchData[x][1][y+1][0] = url
          // searchData[x][1][y+1][1] = 1 => target="_parent"
          // searchData[x][1][y+1][2] = scope

          ti << "[" << endl;
          bool firstEntry=TRUE;

          SDict<QList<Definition> >::Iterator li(g_searchIndexSymbols[i][p]);
          QList<Definition> *dl;
          int itemCount=0;
          for (li.toFirst();(dl=li.current());++li)
          {
            Definition *d = dl->first();
            QCString id = d->localName();
//            t << "<div class=\"SRResult\" id=\"SR_"
//              << searchId(d->localName()) << "\">" << endl;
//            t << " <div class=\"SREntry\">\n";

            if (!firstEntry)
            {
              ti << "," << endl;
            }
            firstEntry=FALSE;

            ti << "  ['" << searchId(d->localName()) << "',['" 
               << d->localName() << "',[";

            if (dl->count()==1) // item with a unique name
            {
              MemberDef  *md   = 0;
              bool isMemberDef = d->definitionType()==Definition::TypeMember;
              if (isMemberDef) md = (MemberDef*)d;
//              t << "  <a id=\"Item" << itemCount << "\" "
//                << "onkeydown=\""
//                << "return searchResults.Nav(event," << itemCount << ")\" "
//                << "onkeypress=\""
//                << "return searchResults.Nav(event," << itemCount << ")\" "
//                << "onkeyup=\""
//                << "return searchResults.Nav(event," << itemCount << ")\" "
//                << "class=\"SRSymbol\" ";
//              t << externalLinkTarget() << "href=\"" << externalRef("../",d->getReference(),TRUE);
//              t << d->getOutputFileBase() << Doxygen::htmlFileExtension;
              QCString anchor = d->anchor();
//              if (!anchor.isEmpty())
//              {
//                t << "#" << anchor;
//              }
//              t << "\"";

              ti << "'" << externalRef("../",d->getReference(),TRUE)
                 << d->getOutputFileBase() << Doxygen::htmlFileExtension;
              if (!anchor.isEmpty())
              {
                ti << "#" << anchor;
              }
              ti << "',";

              static bool extLinksInWindow = Config_getBool("EXT_LINKS_IN_WINDOW");
              if (!extLinksInWindow || d->getReference().isEmpty())
              {
//                t << " target=\"_parent\"";
                ti << "1,";
              }
              else
              {
                ti << "0,";
              }
//              t << ">";
//              t << convertToXML(d->localName());
//              t << "</a>" << endl;


              if (d->getOuterScope()!=Doxygen::globalScope)
              {
//                t << "  <span class=\"SRScope\">" 
//                  << convertToXML(d->getOuterScope()->name()) 
//                  << "</span>" << endl;

                ti << "'" << convertToXML(d->getOuterScope()->name()) << "'";
              }
              else if (md)
              {
                FileDef *fd = md->getBodyDef();
                if (fd==0) fd = md->getFileDef();
                if (fd)
                {
//                  t << "  <span class=\"SRScope\">" 
//                    << convertToXML(fd->localName())
//                    << "</span>" << endl;
                 
                  ti << "'" << convertToXML(fd->localName()) << "'";
                }
              }
              else
              {
                ti << "''";
              }
              ti << "]]";
            }
            else // multiple items with the same name
            {
//              t << "  <a id=\"Item" << itemCount << "\" "
//                << "onkeydown=\""
//                << "return searchResults.Nav(event," << itemCount << ")\" "
//                << "onkeypress=\""
//                << "return searchResults.Nav(event," << itemCount << ")\" "
//                << "onkeyup=\""
//                << "return searchResults.Nav(event," << itemCount << ")\" "
//                << "class=\"SRSymbol\" "
//                << "href=\"javascript:searchResults.Toggle('SR_"
//                << searchId(d->localName()) << "')\">" 
//                << convertToXML(d->localName()) << "</a>" << endl;
//              t << "  <div class=\"SRChildren\">" << endl;

              QListIterator<Definition> di(*dl);
              bool overloadedFunction = FALSE;
              Definition *prevScope = 0;
              int childCount=0;
              for (di.toFirst();(d=di.current());)
              {
                ++di;
                Definition *scope     = d->getOuterScope();
                Definition *next      = di.current();
                Definition *nextScope = 0;
                MemberDef  *md        = 0;
                bool isMemberDef = d->definitionType()==Definition::TypeMember;
                if (isMemberDef) md = (MemberDef*)d;
                if (next) nextScope = next->getOuterScope();

//                t << "    <a id=\"Item" << itemCount << "_c" 
//                  << childCount << "\" "
//                  << "onkeydown=\""
//                  << "return searchResults.NavChild(event," 
//                  << itemCount << "," << childCount << ")\" "
//                  << "onkeypress=\""
//                  << "return searchResults.NavChild(event," 
//                  << itemCount << "," << childCount << ")\" "
//                  << "onkeyup=\""
//                  << "return searchResults.NavChild(event," 
//                  << itemCount << "," << childCount << ")\" "
//                  << "class=\"SRScope\" ";
//                if (!d->getReference().isEmpty())
//                {
//                  t << externalLinkTarget() << externalRef("../",d->getReference(),FALSE);
//                }
//                t << "href=\"" << externalRef("../",d->getReference(),TRUE);
//                t << d->getOutputFileBase() << Doxygen::htmlFileExtension;
                QCString anchor = d->anchor();
//                if (!anchor.isEmpty())
//                {
//                  t << "#" << anchor;
//                }
//                t << "\"";

                if (childCount>0)
                {
                  ti << "],[";
                }
                ti << "'" << externalRef("../",d->getReference(),TRUE)
                   << d->getOutputFileBase() << Doxygen::htmlFileExtension;
                if (!anchor.isEmpty())
                {
                  ti << "#" << anchor;
                }
                ti << "',";

                static bool extLinksInWindow = Config_getBool("EXT_LINKS_IN_WINDOW");
                if (!extLinksInWindow || d->getReference().isEmpty())
                {
//                  t << " target=\"_parent\"";
                  ti << "1,";
                }
                else
                {
                  ti << "0,";
                }
//                t << ">";
                bool found=FALSE;
                overloadedFunction = ((prevScope!=0 && scope==prevScope) ||
                                      (scope && scope==nextScope)
                                     ) && md && 
                                     (md->isFunction() || md->isSlot());
                QCString prefix;
                if (md) prefix=convertToXML(md->localName());
                if (overloadedFunction) // overloaded member function
                {
                  prefix+=convertToXML(md->argsString()); 
                          // show argument list to disambiguate overloaded functions
                }
                else if (md) // unique member function
                {
                  prefix+="()"; // only to show it is a function
                }
                QCString name;
                if (d->definitionType()==Definition::TypeClass)
                {
                  name = convertToXML(((ClassDef*)d)->displayName());
                  found = TRUE;
                }
                else if (d->definitionType()==Definition::TypeNamespace)
                {
                  name = convertToXML(((NamespaceDef*)d)->displayName());
                  found = TRUE;
                }
                else if (scope==0 || scope==Doxygen::globalScope) // in global scope
                {
                  if (md)
                  {
                    FileDef *fd = md->getBodyDef();
                    if (fd==0) fd = md->getFileDef();
                    if (fd)
                    {
                      if (!prefix.isEmpty()) prefix+=":&#160;";
                      name = prefix + convertToXML(fd->localName());
                      found = TRUE;
                    }
                  }
                }
                else if (md && (md->getClassDef() || md->getNamespaceDef())) 
                  // member in class or namespace scope
                {
                  SrcLangExt lang = md->getLanguage();
                  name = convertToXML(d->getOuterScope()->qualifiedName()) 
                       + getLanguageSpecificSeparator(lang) + prefix;
                  found = TRUE;
                }
                else if (scope) // some thing else? -> show scope
                {
                  name = prefix + convertToXML(scope->name());
                  found = TRUE;
                }
                if (!found) // fallback
                {
                  name = prefix + "("+theTranslator->trGlobalNamespace()+")";
                }
//                t << name;
//                t << "</a>" << endl;

                ti << "'" << name << "'";

                prevScope = scope;
                childCount++;
              }
//              t << "  </div>" << endl; // SRChildren

              ti << "]]";
            }
            ti << "]";
//            t << " </div>" << endl; // SREntry
//            t << "</div>" << endl; // SRResult
            itemCount++;
          }
          if (!firstEntry)
          {
            ti << endl;
          }

          ti << "];" << endl;

        }
        else
        {
          err("Failed to open file '%s' for writing...\n",fileName.data());
        }
      }
    }
  }
  //ol.popGeneratorState();

  {
    QFile f(searchDirName+"/search.js");
    if (f.open(IO_WriteOnly))
    {
      FTextStream t(&f);
      t << "// Search script generated by doxygen" << endl;
      t << "// Copyright (C) 2009 by Dimitri van Heesch." << endl << endl;
      t << "// The code in this file is loosly based on main.js, part of Natural Docs," << endl;
      t << "// which is Copyright (C) 2003-2008 Greg Valure" << endl;
      t << "// Natural Docs is licensed under the GPL." << endl << endl;
      t << "var indexSectionsWithContent =" << endl;
      t << "{" << endl;
      bool first=TRUE;
      int j=0;
      for (i=0;i<NUM_SEARCH_INDICES;i++)
      {
        if (g_searchIndexCount[i]>0)
        {
          if (!first) t << "," << endl;
          t << "  " << j << ": \"";
          for (p=0;p<MEMBER_INDEX_ENTRIES;p++)
          {
            t << (g_searchIndexSymbols[i][p].count()>0 ? "1" : "0");
          }
          t << "\"";
          first=FALSE;
          j++;
        }
      }
      if (!first) t << "\n";
      t << "};" << endl << endl;
      t << "var indexSectionNames =" << endl;
      t << "{" << endl;
      first=TRUE;
      j=0;
      for (i=0;i<NUM_SEARCH_INDICES;i++)
      {
        if (g_searchIndexCount[i]>0)
        {
          if (!first) t << "," << endl;
          t << "  " << j << ": \"" << g_searchIndexName[i] << "\"";
          first=FALSE;
          j++;
        }
      }
      if (!first) t << "\n";
      t << "};" << endl << endl;
      t << search_script;
    }
  }
  {
    QFile f(searchDirName+"/nomatches.html");
    if (f.open(IO_WriteOnly))
    {
      FTextStream t(&f);
      t << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" "
           "\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">" << endl;
      t << "<html><head><title></title>" << endl;
      t << "<meta http-equiv=\"Content-Type\" content=\"text/xhtml;charset=UTF-8\"/>" << endl;
      t << "<link rel=\"stylesheet\" type=\"text/css\" href=\"search.css\"/>" << endl;
      t << "<script type=\"text/javascript\" src=\"search.js\"></script>" << endl;
      t << "</head>" << endl;
      t << "<body class=\"SRPage\">" << endl;
      t << "<div id=\"SRIndex\">" << endl;
      t << "<div class=\"SRStatus\" id=\"NoMatches\">"
        << theTranslator->trNoMatches() << "</div>" << endl;
      t << "</div>" << endl;
      t << "</body>" << endl;
      t << "</html>" << endl;
    }
  }
  Doxygen::indexList.addStyleSheetFile("search/search.js");
}

void writeSearchCategories(FTextStream &t)
{
  static SearchIndexCategoryMapping map;
  int i,j=0;
  for (i=0;i<NUM_SEARCH_INDICES;i++)
  {
    if (g_searchIndexCount[i]>0)
    {
      t << "<a class=\"SelectItem\" href=\"javascript:void(0)\" "
        << "onclick=\"searchBox.OnSelectItem(" << j << ")\">"
        << "<span class=\"SelectionMark\">&#160;</span>"
        << convertToXML(map.categoryLabel[i])
        << "</a>";
      j++;
    }
  }
}



