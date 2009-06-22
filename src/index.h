/******************************************************************************
 *
 * 
 *
 * Copyright (C) 1997-2008 by Dimitri van Heesch.
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

#ifndef INDEX_H
#define INDEX_H

#include "qtbc.h"
#include <qfile.h>
#include <qlist.h>

class MemberDef;
class OutputList;
class QTextStream;


class IndexIntf
{
  public:
    virtual ~IndexIntf() {}
    virtual void initialize() = 0;
    virtual void finalize() = 0;
    virtual void incContentsDepth() = 0;
    virtual void decContentsDepth() = 0;
    virtual void addContentsItem(bool isDir, const char *name, const char *ref = 0, 
                                 const char *file = 0, const char *anchor = 0) = 0;
    virtual void addIndexItem(const char *level1, const char *level2, const char *contRef, 
                              const char *memRef, const char *anchor,const MemberDef *md) = 0;
    virtual void addIndexFile(const char *name) = 0;
    virtual void addImageFile(const char *name) = 0;
    virtual void addStyleSheetFile(const char *name) = 0;
};

class IndexList : public IndexIntf
{
  private:
    QList<IndexIntf> m_intfs;

    void foreach(void (IndexIntf::*methodPtr)())
    {
      QListIterator<IndexIntf> li(m_intfs);
      for (li.toFirst();li.current();++li) (li.current()->*methodPtr)();
    }
    template<typename A1>
    void foreach(void (IndexIntf::*methodPtr)(A1),A1 a1)
    {
      QListIterator<IndexIntf> li(m_intfs);
      for (li.toFirst();li.current();++li) (li.current()->*methodPtr)(a1);
    }
    template<typename A1,typename A2,typename A3,typename A4,typename A5>
    void foreach(void (IndexIntf::*methodPtr)(A1,A2,A3,A4,A5),A1 a1,A2 a2,A3 a3,A4 a4,A5 a5)
    {
      QListIterator<IndexIntf> li(m_intfs);
      for (li.toFirst();li.current();++li) (li.current()->*methodPtr)(a1,a2,a3,a4,a5);
    }
    template<typename A1,typename A2,typename A3,typename A4,typename A5,typename A6>
    void foreach(void (IndexIntf::*methodPtr)(A1,A2,A3,A4,A5,A6),A1 a1,A2 a2,A3 a3,A4 a4,A5 a5,A6 a6)
    {
      QListIterator<IndexIntf> li(m_intfs);
      for (li.toFirst();li.current();++li) (li.current()->*methodPtr)(a1,a2,a3,a4,a5,a6);
    }

  public:
    IndexList() { m_intfs.setAutoDelete(TRUE); }
    void addIndex(IndexIntf *intf) 
    { m_intfs.append(intf); }

    // IndexIntf implementation
    void initialize() 
    { foreach(&IndexIntf::initialize); }
    void finalize() 
    { foreach(&IndexIntf::finalize); }
    void incContentsDepth()
    { foreach(&IndexIntf::incContentsDepth); }
    void decContentsDepth()
    { foreach(&IndexIntf::decContentsDepth); }
    void addContentsItem(bool isDir, const char *name, const char *ref = 0, 
                         const char *file = 0, const char *anchor = 0)
    { foreach<bool,const char *,const char *,const char *,const char*>
             (&IndexIntf::addContentsItem,isDir,name,ref,file,anchor); }
    void addIndexItem(const char *level1, const char *level2, const char *contRef, 
                      const char *memRef, const char *anchor,const MemberDef *md)
    { foreach<const char *,const char *,const char *,const char *,const char *,const MemberDef *>
             (&IndexIntf::addIndexItem,level1,level2,contRef,memRef,anchor,md); }
    void addIndexFile(const char *name) 
    { foreach<const char *>(&IndexIntf::addIndexFile,name); }
    void addImageFile(const char *name) 
    { foreach<const char *>(&IndexIntf::addImageFile,name); }
    void addStyleSheetFile(const char *name) 
    { foreach<const char *>(&IndexIntf::addStyleSheetFile,name); }

};


enum IndexSections
{
  isTitlePageStart,
  isTitlePageAuthor,
  isMainPage,
  isModuleIndex,
  isDirIndex,
  isNamespaceIndex,
  isClassHierarchyIndex,
  isCompoundIndex,
  isFileIndex,
  isPageIndex,
  isModuleDocumentation,
  isDirDocumentation,
  isNamespaceDocumentation,
  isClassDocumentation,
  isFileDocumentation,
  isExampleDocumentation,
  isPageDocumentation,
  isPageDocumentation2,
  isEndIndex
};

enum HighlightedItem
{
  HLI_None=0,
  HLI_Main,
  HLI_Modules,
  HLI_Directories,
  HLI_Namespaces,
  HLI_Hierarchy,
  HLI_Classes,
  HLI_Annotated,
  HLI_Files,
  HLI_NamespaceMembers,
  HLI_Functions,
  HLI_Globals,
  HLI_Pages,
  HLI_Examples,
  HLI_Search,

  HLI_ClassVisible,
  HLI_NamespaceVisible,
  HLI_FileVisible
};

enum ClassMemberHighlight
{
  CMHL_All = 0,
  CMHL_Functions,
  CMHL_Variables,
  CMHL_Typedefs,
  CMHL_Enums,
  CMHL_EnumValues,
  CMHL_Properties,
  CMHL_Events,
  CMHL_Related,
  CMHL_Total = CMHL_Related+1
};

enum FileMemberHighlight
{
  FMHL_All = 0,
  FMHL_Functions,
  FMHL_Variables,
  FMHL_Typedefs,
  FMHL_Enums,
  FMHL_EnumValues,
  FMHL_Defines,
  FMHL_Total = FMHL_Defines+1
};

enum NamespaceMemberHighlight
{
  NMHL_All = 0,
  NMHL_Functions,
  NMHL_Variables,
  NMHL_Typedefs,
  NMHL_Enums,
  NMHL_EnumValues,
  NMHL_Total = FMHL_EnumValues+1
};

enum ClassHighlight
{
  CHL_All = 0,
  CHL_Classes,
  CHL_Structs,
  CHL_Unions,
  CHL_Interfaces,
  CHL_Protocols,
  CHL_Categories,
  CHL_Exceptions,
  CHL_Total = CHL_Exceptions+1
};

void writeIndex(OutputList &ol);
void writeHierarchicalIndex(OutputList &ol);
void writeAlphabeticalIndex(OutputList &ol);
void writeClassHierarchy(OutputList &ol);
void writeAnnotatedIndex(OutputList &ol);
void writeAnnotatedClassList(OutputList &ol);
void writeMemberList(OutputList &ol,bool useSections);
void writeSourceIndex(OutputList &ol);
void writeHeaderIndex(OutputList &ol);
void writeHeaderFileList(OutputList &ol);
void writeExampleIndex(OutputList &ol);
void writePageIndex(OutputList &ol);
void writeFileIndex(OutputList &ol);
void writeNamespaceIndex(OutputList &ol);
void writeGroupIndex(OutputList &ol);
void writeDirIndex(OutputList &ol);
void writePackageIndex(OutputList &ol);
void writeClassMemberIndex(OutputList &ol);
void writeFileMemberIndex(OutputList &ol);
void writeNamespaceMemberIndex(OutputList &ol);
void writeGraphicalClassHierarchy(OutputList &ol);
void writeGraphInfo(OutputList &ol);

void countDataStructures();

extern int annotatedClasses;
extern int hierarchyClasses;
extern int documentedFiles;
extern int documentedGroups;
extern int documentedNamespaces;
extern int indexedPages;
extern int documentedClassMembers[CMHL_Total];
extern int documentedFileMembers[FMHL_Total];
extern int documentedNamespaceMembers[NMHL_Total];
extern int documentedHtmlFiles;
extern int documentedPages;
extern int documentedDirs;

void startTitle(OutputList &ol,const char *fileName);
void endTitle(OutputList &ol,const char *fileName,const char *name);
void startFile(OutputList &ol,const char *name,const char *manName,
               const char *title,HighlightedItem hli=HLI_None,
               bool additionalIndices=FALSE);
void endFile(OutputList &ol,bool external=FALSE);

void initClassMemberIndices();
void initFileMemberIndices();
void initNamespaceMemberIndices();
void addClassMemberNameToIndex(MemberDef *md);
void addFileMemberNameToIndex(MemberDef *md);
void addNamespaceMemberNameToIndex(MemberDef *md);

// search engine
void writeSearchIndex();
void writeSearchCategories(QTextStream &t);

#endif
