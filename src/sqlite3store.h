/*
 * sqlite3store.h
 *
 *  Created on: Jul 20, 2011
 *      Author: tuxdna
 */

#ifndef SQLITE3STORE_H_
#define SQLITE3STORE_H_

#include "sqlite3.h"
#include "entry.h"

class Sqlite3Store {
 public:
  static Sqlite3Store* instance();
  int saveEntry(Entry *e);
  int addEntry(Entry *e);
  int updateEntry(Entry *e);
  int countEntries(int id);
  Entry *loadEntry(int id);
  void printEntry(Entry *e);
  Sqlite3Store();
  ~Sqlite3Store();

  enum ListType {
    MEMBER_ARGS_LIST,     // member argument list - ArgumentList
    TEMPLATE_ARGS_LIST,     // template argument lists - QList<ArgumentList> 
    TYPE_CONSTRAINT_ARGS_LIST, // type constraing list - ArgumentList
  };

 private:

  static Sqlite3Store* _instance;

  static const char *default_filename;

  sqlite3 *db;

  //  TABLE entry
  sqlite3_stmt *insert_entry_stmt;
  const char *insert_entry_query;

  sqlite3_stmt *update_entry_stmt;
  const char *update_entry_query;

  sqlite3_stmt *count_entry_by_id_stmt;
  const char *count_entry_by_id_query;

  sqlite3_stmt *load_entry_stmt;
  const char *load_entry_query;

  //  TABLE taginfo
  sqlite3_stmt *insert_taginfo_stmt;
  const char *insert_taginfo_query;

  sqlite3_stmt *update_taginfo_stmt;
  const char *update_taginfo_query;

  sqlite3_stmt *load_taginfo_stmt;
  const char *load_taginfo_query;

  sqlite3_stmt *delete_taginfo_stmt;
  const char *delete_taginfo_query;

  sqlite3_stmt *load_taginfo_by_id_stmt;
  const char *load_taginfo_by_id_query;

  sqlite3_stmt *delete_taginfo_by_id_stmt;
  const char *delete_taginfo_by_id_query;

  //  TABLE argument_list
  sqlite3_stmt *insert_argument_list_stmt;
  const char *insert_argument_list_query;

  sqlite3_stmt *update_argument_list_stmt;
  const char *update_argument_list_query;

  sqlite3_stmt *load_argument_list_stmt;
  const char *load_argument_list_query;

  sqlite3_stmt *delete_argument_list_stmt;
  const char *delete_argument_list_query;

  sqlite3_stmt *load_argument_list_by_id_stmt;
  const char *load_argument_list_by_id_query;

  sqlite3_stmt *delete_argument_list_by_id_stmt;
  const char *delete_argument_list_by_id_query;

  //  TABLE argument
  sqlite3_stmt *insert_argument_stmt;
  const char *insert_argument_query;

  sqlite3_stmt *update_argument_stmt;
  const char *update_argument_query;

  sqlite3_stmt *load_argument_stmt;
  const char *load_argument_query;

  sqlite3_stmt *delete_argument_stmt;
  const char *delete_argument_query;

  sqlite3_stmt *load_argument_by_id_stmt;
  const char *load_argument_by_id_query;

  sqlite3_stmt *delete_argument_by_id_stmt;
  const char *delete_argument_by_id_query;

  //  TABLE baseinfo
  sqlite3_stmt *insert_baseinfo_stmt;
  const char *insert_baseinfo_query;
	
  sqlite3_stmt *update_baseinfo_stmt;
  const char *update_baseinfo_query;
	
  sqlite3_stmt *load_baseinfo_stmt;
  const char *load_baseinfo_query;
	
  sqlite3_stmt *delete_baseinfo_stmt;
  const char *delete_baseinfo_query;

  sqlite3_stmt *load_baseinfo_by_id_stmt;
  const char *load_baseinfo_by_id_query;
	
  sqlite3_stmt *delete_baseinfo_by_id_stmt;
  const char *delete_baseinfo_by_id_query;

  //  TABLE grouping
  sqlite3_stmt *insert_grouping_stmt;
  const char *insert_grouping_query;

  sqlite3_stmt *update_grouping_stmt;
  const char *update_grouping_query;

  sqlite3_stmt *load_grouping_stmt;
  const char *load_grouping_query;

  sqlite3_stmt *delete_grouping_stmt;
  const char *delete_grouping_query;

  sqlite3_stmt *load_grouping_by_id_stmt;
  const char *load_grouping_by_id_query;

  sqlite3_stmt *delete_grouping_by_id_stmt;
  const char *delete_grouping_by_id_query;

  //  TABLE sectioninfo
  sqlite3_stmt *insert_sectioninfo_stmt;
  const char *insert_sectioninfo_query;

  sqlite3_stmt *update_sectioninfo_stmt;
  const char *update_sectioninfo_query;

  sqlite3_stmt *load_sectioninfo_stmt;
  const char *load_sectioninfo_query;

  sqlite3_stmt *delete_sectioninfo_stmt;
  const char *delete_sectioninfo_query;

  sqlite3_stmt *load_sectioninfo_by_id_stmt;
  const char *load_sectioninfo_by_id_query;

  sqlite3_stmt *delete_sectioninfo_by_id_stmt;
  const char *delete_sectioninfo_by_id_query;

  //  TABLE listiteminfo
  sqlite3_stmt *insert_listiteminfo_stmt;
  const char *insert_listiteminfo_query;

  sqlite3_stmt *update_listiteminfo_stmt;
  const char *update_listiteminfo_query;

  sqlite3_stmt *load_listiteminfo_stmt;
  const char *load_listiteminfo_query;

  sqlite3_stmt *delete_listiteminfo_stmt;
  const char *delete_listiteminfo_query;

  sqlite3_stmt *load_listiteminfo_by_id_stmt;
  const char *load_listiteminfo_by_id_query;

  sqlite3_stmt *delete_listiteminfo_by_id_stmt;
  const char *delete_listiteminfo_by_id_query;


  int rc;

  void initialize();
  void destroy();
  int initializeSchema();
  int prepareStatements();
  int doPrepare(const char *query, sqlite3_stmt **stmt );
  int prepareEntryQueries();
  int prepareTagInfoQueries();
  int prepareArgumentQueries();
  int prepareArgumentListQueries();
  int prepareBaseinfoQueries();
  int prepareGroupingQueries();
  int prepareSectioninfoQueries();
  int prepareListiteminfoQueries();


  int addTagInfo(Entry *e);
  int addArgsList(Entry *e, ArgumentList *argList, ListType list_type);
  int addTArgsList(Entry *e);
  int addArgument(int, Argument*);
  int addTypeConstr(Entry *e);
  int addExtends(Entry *e);
  int addGroups(Entry *e);
  int addAnchors(Entry *e);
  int addSli(Entry *e);

  Entry *readEntry(int id);
  TagInfo *readTagInfo(Entry *e);
  ArgumentList* readArgumentList(int list_id);
  ArgumentList* readArgsList(Entry *e, ListType list_type);
  QList<ArgumentList>* readTArgsList(Entry *e);
  ArgumentList*  readTypeConstr(Entry *e);
  QList<BaseInfo>* readExtends(Entry *e);
  QList<Grouping>* readGroups(Entry *e);
  QList<SectionInfo>* readAnchors(Entry *e);
  QList<ListItemInfo>* readSli(Entry *e);

};

#endif /* SQLITE3STORE_H_ */
