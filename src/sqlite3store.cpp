/*
 * sqlite3store.cpp
 *
 *  Created on: Jul 20, 2011
 *      Author: tuxdna
 */

#include "sqlite3store.h"
#include "entry.h"
#include "section.h"
#include <stdio.h>

Sqlite3Store * Sqlite3Store::_instance = 0;
const char * Sqlite3Store::default_filename = "entry_store_s3.db";

Sqlite3Store* Sqlite3Store::instance() {
  if (0 == _instance) {
    _instance = new Sqlite3Store;
  }
  return _instance;
}

Sqlite3Store::Sqlite3Store() {
  initialize();
}

Sqlite3Store::~Sqlite3Store() {
  destroy();
}

void Sqlite3Store::initialize() {
  // open database
  const char *file = Sqlite3Store::default_filename;

  sqlite3_initialize();

  printf("opening database: %s\n", file);
  rc = sqlite3_open_v2(file, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
		       NULL);

  if (rc != SQLITE_OK) {
    sqlite3_close(db);
    printf("database open failed: %s\n", file);
    exit(-1);
  }

  // Switch off synchronous operations!
  char * sErrMsg = 0;
  sqlite3_exec(db, "PRAGMA synchronous = OFF", NULL, NULL, &sErrMsg);
  printf("database opened successfully: %s\n", file);

  this->initializeSchema();
  this->prepareStatements();
}

int Sqlite3Store::doPrepare(const char *query, sqlite3_stmt **stmt ) {
  int rc = sqlite3_prepare_v2(db, query, -1, stmt, NULL);
  if (rc != SQLITE_OK) {
    printf("prepare failed for %s\n", query);
    exit(-1);
  }
  return rc;
}

int Sqlite3Store::prepareStatements() {
  // build all prepared statements
  this->prepareEntryQueries();
  this->prepareTagInfoQueries();
  this->prepareArgumentQueries();
  this->prepareArgumentListQueries();
  this->prepareBaseinfoQueries();
  this->prepareGroupingQueries();
  this->prepareSectioninfoQueries();
  this->prepareListiteminfoQueries();
  return 0;
}

int Sqlite3Store::prepareEntryQueries() {
  int rc;
  
  // prepare insert statement
  insert_entry_query = "INSERT INTO entry ("
    " id,"
    " parent_id,"
    " name,"
    " type,"
    " section,"
    " protection,"
    " mtype,"
    " spec,"
    " initLines,"
    " stat,"
    " explicitExternal,"
    " proto,"
    " subGrouping,"
    " callGraph,"
    " callerGraph,"
    " virt,"
    " args,"
    " bitfields,"
    " program,"
    " initializer,"
    " includeFile,"
    " includeName,"
    " doc,"
    " docLine,"
    " docFile,"
    " brief,"
    " briefLine,"
    " briefFile,"
    " inbodyDocs,"
    " inbodyLine,"
    " inbodyFile,"
    " relates,"
    " relatesType,"
    " read,"
    " write,"
    " inside,"
    " exception,"
    " bodyLine,"
    " endBodyLine,"
    " mGrpId,"
    " fileName,"
    " startLine,"
    " lang,"
    " hidden,"
    " artificial,"
    " groupDocType"
    " ) "
    " VALUES ("
    " :id,"
    " :parent_id,"
    " :name,"
    " :type,"
    " :section,"
    " :protection,"
    " :mtype,"
    " :spec,"
    " :initLines,"
    " :stat,"
    " :explicitExternal,"
    " :proto,"
    " :subGrouping,"
    " :callGraph,"
    " :callerGraph,"
    " :virt,"
    " :args,"
    " :bitfields,"
    " :program,"
    " :initializer,"
    " :includeFile,"
    " :includeName,"
    " :doc,"
    " :docLine,"
    " :docFile,"
    " :brief,"
    " :briefLine,"
    " :briefFile,"
    " :inbodyDocs,"
    " :inbodyLine,"
    " :inbodyFile,"
    " :relates,"
    " :relatesType,"
    " :read,"
    " :write,"
    " :inside,"
    " :exception,"
    " :bodyLine,"
    " :endBodyLine,"
    " :mGrpId,"
    " :fileName,"
    " :startLine,"
    " :lang,"
    " :hidden,"
    " :artificial,"
    " :groupDocType"
    ") ";
  insert_entry_stmt = NULL;
  rc = this->doPrepare(insert_entry_query,
		       &insert_entry_stmt);

  // prepare update entry query
  // and statement
  update_entry_query = "UPDATE  entry SET"
    "  parent_id = :parent_id,"
    "  name = :name,"
    "  type = :type,"
    "  section = :section,"
    "  protection = :protection,"
    "  mtype = :mtype,"
    "  spec = :spec,"
    "  initLines = :initLines,"
    "  stat = :stat,"
    "  explicitExternal = :explicitExternal,"
    "  proto = :proto,"
    "  subGrouping = :subGrouping,"
    "  callGraph = :callGraph,"
    "  callerGraph = :callerGraph,"
    "  virt = :virt,"
    "  args = :args,"
    "  bitfields = :bitfields,"
    "  program = :program,"
    "  initializer = :initializer,"
    "  includeFile = :includeFile,"
    "  includeName = :includeName,"
    "  doc = :doc,"
    "  docLine = :docLine,"
    "  docFile = :docFile,"
    "  brief = :brief,"
    "  briefLine = :briefLine,"
    "  briefFile = :briefFile,"
    "  inbodyDocs = :inbodyDocs,"
    "  inbodyLine = :inbodyLine,"
    "  inbodyFile = :inbodyFile,"
    "  relates = :relates,"
    "  relatesType = :relatesType,"
    "  read = :read,"
    "  write = :write,"
    "  inside = :inside,"
    "  exception = :exception,"
    "  bodyLine = :bodyLine,"
    "  endBodyLine = :endBodyLine,"
    "  mGrpId = :mGrpId,"
    "  fileName = :fileName,"
    "  startLine = :startLine,"
    "  lang = :lang,"
    "  hidden = :hidden,"
    "  artificial = :artificial,"
    "  groupDocType = :groupDocType"
    " WHERE"
    "  id = :id";
  update_entry_stmt = NULL;
  rc = this->doPrepare(update_entry_query,
		       &update_entry_stmt);

  // prepare count entry query and statement
  count_entry_by_id_query = "SELECT count(*) from entry where id = :id";
  count_entry_by_id_stmt = NULL;
  rc = this->doPrepare(count_entry_by_id_query,
		       &count_entry_by_id_stmt);

  // prepare load entry statement
  load_entry_query =
    "SELECT "
    " id,"
    " parent_id,"
    " name,"
    " type,"
    " section,"
    " protection,"
    " mtype,"
    " spec,"
    " initLines,"
    " stat,"
    " explicitExternal,"
    " proto,"
    " subGrouping,"
    " callGraph,"
    " callerGraph,"
    " virt,"
    " args,"
    " bitfields,"
    " program,"
    " initializer,"
    " includeFile,"
    " includeName,"
    " doc,"
    " docLine,"
    " docFile,"
    " brief,"
    " briefLine,"
    " briefFile,"
    " inbodyDocs,"
    " inbodyLine,"
    " inbodyFile,"
    " relates,"
    " relatesType,"
    " read,"
    " write,"
    " inside,"
    " exception,"
    " bodyLine,"
    " endBodyLine,"
    " mGrpId,"
    " fileName,"
    " startLine,"
    " lang,"
    " hidden,"
    " artificial,"
    " groupDocType"
    " FROM entry "
    "WHERE id = :id";
  load_entry_stmt = NULL;
  rc = this->doPrepare(load_entry_query,
		       &load_entry_stmt);
  
  return rc;
}

int Sqlite3Store::prepareTagInfoQueries() {
  int rc;

  insert_taginfo_stmt = NULL;
  insert_taginfo_query =
    "INSERT INTO taginfo ( "
    " entry_id, "
    " tagName, "
    " fileName, "
    " anchor "
    ") VALUES ( "
    " :entry_id, "
    " :tagName, "
    " :fileName, "
    " :anchor "
    ")"
    ;
  rc = this->doPrepare(insert_taginfo_query,
		       &insert_taginfo_stmt);
  
  update_taginfo_stmt = NULL;
  update_taginfo_query =
    "UPDATE taginfo SET "
    " entry_id = :entry_id, "
    " tagName = :tagName, "
    " fileName = :fileName, "
    " anchor = :anchor "
    " WHERE id = :id"
    ;
  rc = this->doPrepare(update_taginfo_query,
		       &update_taginfo_stmt);

  load_taginfo_stmt = NULL;
  load_taginfo_query =
    "SELECT id, "
    " entry_id, "
    " tagName, "
    " filename, "
    " anchor "
    " FROM taginfo"
    " WHERE entry_id = :entry_id"
    ;
  rc = this->doPrepare(load_taginfo_query,
		       &load_taginfo_stmt);

  delete_taginfo_stmt = NULL;
  delete_taginfo_query =
    "DELETE from taginfo WHERE entry_id = :entry_id";
  rc = this->doPrepare(delete_taginfo_query,
		       &delete_taginfo_stmt);

  load_taginfo_by_id_stmt = NULL;
  load_taginfo_by_id_query =
    "SELECT id, "
    " entry_id, "
    " tagName, "
    " filename, "
    " anchor "
    " FROM taginfo"
    " WHERE id = :id";
  rc = this->doPrepare(load_taginfo_by_id_query,
		       &load_taginfo_by_id_stmt);
  
  delete_taginfo_by_id_stmt = NULL;
  delete_taginfo_by_id_query =
    "DELETE from taginfo WHERE id = :id";
  rc = this->doPrepare(delete_taginfo_by_id_query,
		       &delete_taginfo_by_id_stmt);

  return rc;
}


int Sqlite3Store::prepareArgumentListQueries() {
  int rc;

  insert_argument_list_stmt = NULL;
  insert_argument_list_query =
    "INSERT INTO argument_list ( "
    // " id, "
    " entry_id, "
    " list_type, "
    " constSpecifier, "
    " volatileSpecifier, "
    " pureSpecifier "
    ") VALUES ( "
    // " :id, "
    " :entry_id, "
    " :list_type, "
    " :constSpecifier, "
    " :volatileSpecifier, "
    " :pureSpecifier "
    ") ";
  rc = this->doPrepare (insert_argument_list_query,
			&insert_argument_list_stmt);
    
  update_argument_list_stmt = NULL;
  update_argument_list_query =
    "UPDATE argument_list SET "
    " entry_id = entry_id, "
    " list_type = :list_type, "
    " constSpecifier = :constSpecifier, "
    " volatileSpecifier = :volatileSpecifier, "
    " pureSpecifier = :pureSpecifier "
    " WHERE id = :id ";
  rc = this->doPrepare(update_argument_list_query,
		       &update_argument_list_stmt);
  
  load_argument_list_stmt = NULL;
  load_argument_list_query =
    "SELECT "
    " id, "
    " entry_id, "
    " list_type, "
    " constSpecifier, "
    " volatileSpecifier, "
    " pureSpecifier "
    " FROM argument_list"
    " WHERE entry_id = :entry_id and list_type = :list_type";
  rc = this->doPrepare(load_argument_list_query,
		       &load_argument_list_stmt);
  
  delete_argument_list_stmt = NULL;
  delete_argument_list_query =
    "DELETE FROM argument_list where entry_id = :entry_id";
  rc = this->doPrepare(delete_argument_list_query,
		       &delete_argument_list_stmt);

  load_argument_list_by_id_stmt = NULL;
  load_argument_list_by_id_query =
    "SELECT "
    " id, "
    " entry_id, "
    " list_type, "
    " constSpecifier, "
    " volatileSpecifier, "
    " pureSpecifier "
    " FROM argument_list"
    " WHERE id = :id";
  rc = this->doPrepare(load_argument_list_by_id_query,
		       &load_argument_list_by_id_stmt);
  
  delete_argument_list_by_id_stmt = NULL;
  delete_argument_list_by_id_query =
    "DELETE FROM argument_list where id = :id";
  rc = this->doPrepare(delete_argument_list_by_id_query,
		       &delete_argument_list_by_id_stmt);
  
  return rc;
}

int Sqlite3Store::prepareArgumentQueries() {
  int rc;

  insert_argument_stmt = NULL;
  insert_argument_query
    = 
    "INSERT INTO argument( "
    " argument_list_id ,"
    " attrib, "
    " type, "
    " canType, "
    " name, "
    " array, "
    " defval, "
    " docs "
    " ) VALUES ( "
    " :argument_list_id, "
    " :attrib, "
    " :type, "
    " :canType, "
    " :name, "
    " :array, "
    " :defval, "
    " :docs)";

  rc = this->doPrepare(insert_argument_query,
		       &insert_argument_stmt);
  
  update_argument_stmt = NULL;
  update_argument_query
    =
    "UPDATE argument " 
    " SET argument_list_id = :argument_list_id, "
    " attrib = :attrib, type = :type, canType = :canType, name = :name, array = :array, defval = :defavl, docs = :docs WHERE id = :id";
  rc = this->doPrepare(update_argument_query,
		       &update_argument_stmt);
  
  load_argument_stmt = NULL;
  load_argument_query =
    "SELECT id, argument_list_id, attrib, type, canType, name, array, defval, docs FROM argument WHERE argument_list_id = :argument_list_id";
  rc = this->doPrepare(load_argument_query,
		       &load_argument_stmt);
  
  delete_argument_stmt = NULL;
  delete_argument_query = 
    "DELETE FROM argument WHERE argument_list_id = :argument_list_id";
  rc = this->doPrepare(delete_argument_query,
		       &delete_argument_stmt);
  
  load_argument_by_id_stmt = NULL;
  load_argument_by_id_query =
    "SELECT id, argument_list_id, attrib, type, canType, name, array, defval, docs FROM argument WHERE id = :id";
  rc = this->doPrepare(load_argument_by_id_query,
		       &load_argument_by_id_stmt);

  delete_argument_by_id_stmt = NULL;
  delete_argument_by_id_query = 
    "DELETE FROM argument WHERE id = :id";
  rc = this->doPrepare(delete_argument_by_id_query,
		       &delete_argument_by_id_stmt);
  
  return rc;
}

int Sqlite3Store::prepareBaseinfoQueries() {
  int rc;

  insert_baseinfo_stmt = NULL;
  insert_baseinfo_query = 
    "INSERT INTO baseinfo (id, entry_id, name, prot, virt) "
    "VALUES(:id, :entry_id, :name, :prot, :virt)";
  rc = this->doPrepare(insert_baseinfo_query,
		       &insert_baseinfo_stmt);
  
  update_baseinfo_stmt = NULL;
  update_baseinfo_query = "UPDATE baseinfo "
    " SET entry_id = :entry_id, name = :name, prot = :prot, virt = :virt "
    " WHERE id = :id";
  rc = this->doPrepare(update_baseinfo_query,
		       &update_baseinfo_stmt);

  load_baseinfo_stmt = NULL;
  load_baseinfo_query = 
    "SELECT id, entry_id, name, prot, virt FROM baseinfo WHERE entry_id = :entry_id";
  rc = this->doPrepare(load_baseinfo_query,
		       &load_baseinfo_stmt);

  delete_baseinfo_stmt = NULL;
  delete_baseinfo_query = "DELETE FROM baseinfo WHERE entry_id = :entry_id";
  rc = this->doPrepare(delete_baseinfo_query,
		       &delete_baseinfo_stmt);

  load_baseinfo_by_id_stmt = NULL;
  load_baseinfo_by_id_query = 
    "SELECT id, entry_id, name, prot, virt from baseinfo WHERE id = :id";
  rc = this->doPrepare(load_baseinfo_by_id_query,
		       &load_baseinfo_by_id_stmt);

  delete_baseinfo_by_id_stmt = NULL;
  delete_baseinfo_by_id_query = 
    "DELETE FROM baseinfo WHERE id = :id";
  rc = this->doPrepare(delete_baseinfo_by_id_query,
		       &delete_baseinfo_by_id_stmt);

  return rc;
}

int Sqlite3Store::prepareGroupingQueries() {
  int rc;
  insert_grouping_stmt = NULL;
  insert_grouping_query = 
    "INSERT INTO grouping (id, entry_id, groupname, pri) "
    "VALUES(:id, :entry_id, :groupname, :pri)";
  rc = this->doPrepare(insert_grouping_query, &insert_grouping_stmt);
  
  update_grouping_stmt = NULL;
  update_grouping_query = "UPDATE grouping "
    " SET entry_id = :entry_id, groupname = :groupname, pri = :pri "
    " WHERE id = :id";
  rc = this->doPrepare(update_grouping_query,
		       &update_grouping_stmt);

  load_grouping_stmt = NULL;
  load_grouping_query = 
    "SELECT id, entry_id, groupname, pri "
    " FROM grouping WHERE entry_id = :entry_id";
  rc = this->doPrepare(load_grouping_query,
		       &load_grouping_stmt);

  delete_grouping_stmt = NULL;
  delete_grouping_query = "DELETE FROM grouping WHERE entry_id = :entry_id";
  rc = this->doPrepare(delete_grouping_query,
		       &delete_grouping_stmt);

  load_grouping_by_id_stmt = NULL;
  load_grouping_by_id_query = 
    "SELECT id, entry_id, groupname, pri "
    " FROM grouping WHERE id = :id";
  rc = this->doPrepare(load_grouping_by_id_query,
		       &load_grouping_by_id_stmt);

  delete_grouping_by_id_stmt = NULL;
  delete_grouping_by_id_query =
    "DELETE FROM grouping WHERE id = :id";
  rc = this->doPrepare(delete_grouping_by_id_query,
		       &delete_grouping_by_id_stmt);

  return rc;
}

int Sqlite3Store::prepareSectioninfoQueries() {
  int rc;
  insert_sectioninfo_stmt = NULL;
  insert_sectioninfo_query = 
    "INSERT INTO sectioninfo (id, entry_id, label, title, type, ref, filename) "
    "VALUES(:id, :entry_id, :label, :title, :type, :ref, :filename)";
  rc = this->doPrepare(insert_sectioninfo_query, &insert_sectioninfo_stmt);
  
  update_sectioninfo_stmt = NULL;
  update_sectioninfo_query = "UPDATE sectioninfo "
    " SET entry_id = :entry_id, label = :label, title = :title, type = :type, ref = :ref, filename = :filename "
    " WHERE id = :id";
  rc = this->doPrepare(update_sectioninfo_query, &update_sectioninfo_stmt);

  load_sectioninfo_stmt = NULL;
  load_sectioninfo_query =
    "SELECT id, entry_id, label, title, type, ref, filename "
    " FROM sectioninfo where entry_id = :entry_id";
  rc = this->doPrepare(load_sectioninfo_query,
		       &load_sectioninfo_stmt);

  delete_sectioninfo_stmt = NULL;
  delete_sectioninfo_query = 
    "DELETE FROM sectioninfo WHERE entry_id = :entry_id";
  rc = this->doPrepare(delete_sectioninfo_query,
		       &delete_sectioninfo_stmt);

  load_sectioninfo_by_id_stmt = NULL;
  load_sectioninfo_by_id_query =
    "SELECT id, entry_id, label, title, type, ref, filename "
    " FROM sectioninfo where id = :id";
  rc = this->doPrepare(load_sectioninfo_by_id_query,
		       &load_sectioninfo_by_id_stmt);

  delete_sectioninfo_by_id_stmt = NULL;
  delete_sectioninfo_by_id_query = 
    "DELETE FROM sectioninfo WHERE id = :id";
  rc = this->doPrepare(delete_sectioninfo_by_id_query,
		       &delete_sectioninfo_by_id_stmt);

  return rc;
}

int Sqlite3Store::prepareListiteminfoQueries() {
  int rc;
  insert_listiteminfo_stmt = NULL;
  insert_listiteminfo_query = 
    "INSERT INTO listiteminfo (id, entry_id, type, itemId) "
    "VALUES(:id, :entry_id, :type, :itemId)";
  rc = this->doPrepare(insert_listiteminfo_query, &insert_listiteminfo_stmt);
  
  update_listiteminfo_stmt = NULL;
  update_listiteminfo_query = "UPDATE listiteminfo "
    " SET entry_id = :entry_id, type = :type, itemId = :itemId "
    " where id = :id";
  rc = this->doPrepare(update_listiteminfo_query, &update_listiteminfo_stmt);

  load_listiteminfo_stmt = NULL;
  load_listiteminfo_query = "SELECT id, entry_id, type, itemId from listiteminfo where entry_id = :entry_id";
  rc = this->doPrepare(load_listiteminfo_query, &load_listiteminfo_stmt);

  delete_listiteminfo_stmt = NULL;
  delete_listiteminfo_query = "DELETE FROM listiteminfo where entry_id = :entry_id";
  rc = this->doPrepare(delete_listiteminfo_query, &delete_listiteminfo_stmt);

  load_listiteminfo_by_id_stmt = NULL;
  load_listiteminfo_by_id_query = 
    "SELECT id, entry_id, type, itemId from listiteminfo where id = :id";
  rc = this->doPrepare(load_listiteminfo_by_id_query,
		       &load_listiteminfo_by_id_stmt);

  delete_listiteminfo_by_id_stmt = NULL;
  delete_listiteminfo_by_id_query =
    "DELETE FROM listiteminfo where id = :id";
  rc = this->doPrepare(delete_listiteminfo_by_id_query,
		       &delete_listiteminfo_by_id_stmt);

  return rc;
}

void Sqlite3Store::destroy() {
  sqlite3_finalize(insert_entry_stmt);
  sqlite3_close(db);
  sqlite3_shutdown();
}

int Sqlite3Store::initializeSchema() {
  sqlite3_stmt *stmt = NULL;
  int rc;
  const char * schema_queries[][2] = {
    {
      "entry",
      "CREATE TABLE IF NOT EXISTS entry ("
      " id INTEGER PRIMARY KEY NOT NULL,"
      " parent_id INTEGER,"
      " name TEXT,"
      " type TEXT,"
      " section INTEGER,"
      " protection INTEGER,"
      " mtype INTEGER,"
      " spec INTEGER,"
      " initLines  INTEGER,"
      " stat INTEGER,"
      " explicitExternal INTEGER,"
      " proto INTEGER,"
      " subGrouping INTEGER,"
      " callGraph INTEGER,"
      " callerGraph INTEGER,"
      " virt INTEGER,"
      " args TEXT,"
      " bitfields TEXT,"
      " program TEXT,"
      " initializer TEXT,"
      " includeFile TEXT,"
      " includeName TEXT,"
      " doc TEXT,"
      " docLine  INTEGER,"
      " docFile TEXT,"
      " brief TEXT,"
      " briefLine  INTEGER,"
      " briefFile TEXT,"
      " inbodyDocs TEXT,"
      " inbodyLine  INTEGER,"
      " inbodyFile TEXT,"
      " relates TEXT,"
      " relatesType INTEGER,"
      " read TEXT,"
      " write TEXT,"
      " inside TEXT,"
      " exception TEXT,"
      " bodyLine  INTEGER,"
      " endBodyLine  INTEGER,"
      " mGrpId  INTEGER,"
      " fileName TEXT,"
      " startLine INTEGER,"
      " lang INTEGER,"
      " hidden INTEGER,"
      " artificial INTEGER,"
      " groupDocType INTEGER) "
    },
    {
      "taginfo",
      "CREATE TABLE IF NOT EXISTS taginfo ( id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
      "entry_id INTEGER NOT NULL, tagName TEXT, fileName TEXT, anchor TEXT, "
      "FOREIGN KEY(entry_id) REFERENCES entry(id))"
    },
    {
      "argument_list",
      "CREATE TABLE IF NOT EXISTS argument_list ( id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
      "entry_id INTEGER NOT NULL, list_type INTEGER NOT NULL, constSpecifier INTEGER NOT NULL, "
      "volatileSpecifier INTEGER NOT NULL, pureSpecifier INTEGER NOT NULL, "
      "FOREIGN KEY(entry_id) REFERENCES entry(id))"
    },
    {
      "argument",
      "CREATE TABLE IF NOT EXISTS argument ( id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
      "argument_list_id INTEGER NOT NULL, attrib TEXT, type TEXT, canType TEXT, "
      "name TEXT, array TEXT, defval TEXT, docs TEXT, "
      "FOREIGN KEY(argument_list_id) REFERENCES argument_list(id) )"
    },
    {
      "baseinfo",
      "CREATE TABLE IF NOT EXISTS baseinfo ( id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
      "entry_id INTEGER NOT NULL, name TEXT, prot INTEGER, virt INTEGER, "
      "FOREIGN KEY(entry_id) REFERENCES entry(id))"
    },
    {
      "grouping",
      "CREATE TABLE IF NOT EXISTS grouping ( id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
      "entry_id INTEGER NOT NULL, groupname TEXT, pri INTEGER, "
      "FOREIGN KEY(entry_id) REFERENCES entry(id))"
    },
    {
      "sectioninfo",
      "CREATE TABLE IF NOT EXISTS sectioninfo ( id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
      "entry_id INTEGER NOT NULL, label TEXT, title, TEXT, type INTEGER, ref TEXT, filename TEXT, "
      "FOREIGN KEY(entry_id) REFERENCES entry(id))"
    },
    {
      "listiteminfo",
      "CREATE TABLE IF NOT EXISTS listiteminfo ( id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
      "entry_id INTEGER NOT NULL, type TEXT, itemId INTEGER, "
      "FOREIGN KEY(entry_id) REFERENCES entry(id))"
    }
  };

  for (unsigned int k = 0; k < sizeof(schema_queries) / sizeof(schema_queries[0]); k++) {
    const char *tname = schema_queries[k][0];
    const char *q = schema_queries[k][1];
    // create table
    rc = sqlite3_prepare_v2(db, q, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
      printf("failed to prepare query: %s\n", q);
      exit(-1);
    }
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
      printf("failed to execute query: %s\n", q);
      exit(-1);
    }
    sqlite3_finalize(stmt);

    printf("table created: %s\n%s\n", tname, q);
  }

  return 0;

}

void Sqlite3Store::printEntry(Entry *e) {
  printf("id: %d\n", e->id);
  printf("parent_id: %d\n", e->get_parent_id());
  printf("name: %s\n", e->name.data());
  printf("type: %s\n", e->type.data());
  printf("section: %d\n", e->section);
  printf("protection: %d\n", (int) e->protection);
  printf("mtype: %d\n", (int) e->mtype);
  printf("spec: %d\n", e->spec);
  printf("initLines: %d\n", e->initLines);
  printf("stat: %d\n", e->stat);
  printf("explicitExternal: %d\n", e->explicitExternal);
  printf("proto: %d\n", e->proto);
  printf("subGrouping: %d\n", e->subGrouping);
  printf("callGraph: %d\n", e->callGraph);
  printf("callerGraph: %d\n", e->callerGraph);
  printf("virt: %d\n", (int) e->virt);
  printf("args: %s\n", e->args.data());
  printf("bitfields: %s\n", e->bitfields.data());
  printf("program: %s\n", e->program.data());
  printf("initializer: %s\n", e->initializer.data());
  printf("includeFile: %s\n", e->includeFile.data());
  printf("includeName: %s\n", e->includeName.data());
  printf("doc: %s\n", e->doc.data());
  printf("docLine: %d\n", e->docLine);
  printf("docFile: %s\n", e->docFile.data());
  printf("brief: %s\n", e->brief.data());
  printf("briefLine: %d\n", e->briefLine);
  printf("briefFile: %s\n", e->briefFile.data());
  printf("inbodyDocs: %s\n", e->inbodyDocs.data());
  printf("inbodyLine: %d\n", e->inbodyLine);
  printf("inbodyFile: %s\n", e->inbodyFile.data());
  printf("relates: %s\n", e->relates.data());
  printf("relatesType: %d\n", e->relatesType);
  printf("read: %s\n", e->read.data());
  printf("write: %s\n", e->write.data());
  printf("inside: %s\n", e->inside.data());
  printf("exception: %s\n", e->exception.data());
  printf("bodyLine: %d\n", e->bodyLine);
  printf("endBodyLine: %d\n", e->endBodyLine);
  printf("mGrpId: %d\n", e->mGrpId);
  printf("fileName: %s\n", e->fileName.data());
  printf("startLine: %d\n", e->startLine);
  printf("lang: %d\n", (int) e->lang);
  printf("hidden: %d\n", e->hidden);
  printf("artificial: %d\n", e->artificial);
  printf("groupDocType: %d\n", e->groupDocType);

}


// TODO: fix this member function
int Sqlite3Store::saveEntry(Entry *e) {
  int rc;
  if(this->countEntries(e->id) > 0) {
    this->updateEntry(e);
    //TODO: delete entry
    //TODO: add entry
  } else  {
    this->addEntry(e);
    this->addTagInfo(e);
    
    ListType list_type = MEMBER_ARGS_LIST;
    this->addArgsList(e, e->argList, list_type);
    this->addTArgsList(e);
    this->addTypeConstr(e);
    this->addExtends(e);
    this->addGroups(e);
    this->addAnchors(e);
    this->addSli(e);
  }
  return rc;
}

// TODO: fix this member function
int Sqlite3Store::addEntry(Entry *e) {
  int rc;
  sqlite3_stmt *stmt = insert_entry_stmt;
  int idx;

  ASSERT(stmt != NULL);

  // this->printEntry(e);

  idx = sqlite3_bind_parameter_index(stmt, ":id");
  sqlite3_bind_int(stmt, idx, e->id);

  idx = sqlite3_bind_parameter_index(stmt, ":parent_id");
  sqlite3_bind_int(stmt, idx, e->get_parent_id());

  idx = sqlite3_bind_parameter_index(stmt, ":name");
  sqlite3_bind_text(stmt, idx, e->name.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":type");
  sqlite3_bind_text(stmt, idx, e->type.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":section");
  sqlite3_bind_int(stmt, idx, e->section);

  idx = sqlite3_bind_parameter_index(stmt, ":protection");
  sqlite3_bind_int(stmt, idx, (int) e->protection);

  idx = sqlite3_bind_parameter_index(stmt, ":mtype");
  sqlite3_bind_int(stmt, idx, (int) e->mtype);

  idx = sqlite3_bind_parameter_index(stmt, ":spec");
  sqlite3_bind_int(stmt, idx, e->spec);

  idx = sqlite3_bind_parameter_index(stmt, ":initLines");
  sqlite3_bind_int(stmt, idx, e->initLines);

  idx = sqlite3_bind_parameter_index(stmt, ":stat");
  sqlite3_bind_int(stmt, idx, e->stat);

  idx = sqlite3_bind_parameter_index(stmt, ":explicitExternal");
  sqlite3_bind_int(stmt, idx, e->explicitExternal);

  idx = sqlite3_bind_parameter_index(stmt, ":proto");
  sqlite3_bind_int(stmt, idx, e->proto);

  idx = sqlite3_bind_parameter_index(stmt, ":subGrouping");
  sqlite3_bind_int(stmt, idx, e->subGrouping);

  idx = sqlite3_bind_parameter_index(stmt, ":callGraph");
  sqlite3_bind_int(stmt, idx, e->callGraph);

  idx = sqlite3_bind_parameter_index(stmt, ":callerGraph");
  sqlite3_bind_int(stmt, idx, e->callerGraph);

  idx = sqlite3_bind_parameter_index(stmt, ":virt");
  sqlite3_bind_int(stmt, idx, (int) e->virt);

  idx = sqlite3_bind_parameter_index(stmt, ":args");
  sqlite3_bind_text(stmt, idx, e->args.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":bitfields");
  sqlite3_bind_text(stmt, idx, e->bitfields.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":program");
  sqlite3_bind_text(stmt, idx, e->program.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":initializer");
  sqlite3_bind_text(stmt, idx, e->initializer.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":includeFile");
  sqlite3_bind_text(stmt, idx, e->includeFile.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":includeName");
  sqlite3_bind_text(stmt, idx, e->includeName.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":doc");
  sqlite3_bind_text(stmt, idx, e->doc.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":docLine");
  sqlite3_bind_int(stmt, idx, e->docLine);

  idx = sqlite3_bind_parameter_index(stmt, ":docFile");
  sqlite3_bind_text(stmt, idx, e->docFile.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":brief");
  sqlite3_bind_text(stmt, idx, e->brief.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":briefLine");
  sqlite3_bind_int(stmt, idx, e->briefLine);

  idx = sqlite3_bind_parameter_index(stmt, ":briefFile");
  sqlite3_bind_text(stmt, idx, e->briefFile.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":inbodyDocs");
  sqlite3_bind_text(stmt, idx, e->inbodyDocs.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":inbodyLine");
  sqlite3_bind_int(stmt, idx, e->inbodyLine);

  idx = sqlite3_bind_parameter_index(stmt, ":inbodyFile");
  sqlite3_bind_text(stmt, idx, e->inbodyFile.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":relates");
  sqlite3_bind_text(stmt, idx, e->relates.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":relatesType");
  sqlite3_bind_int(stmt, idx, e->relatesType);

  idx = sqlite3_bind_parameter_index(stmt, ":read");
  sqlite3_bind_text(stmt, idx, e->read.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":write");
  sqlite3_bind_text(stmt, idx, e->write.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":inside");
  sqlite3_bind_text(stmt, idx, e->inside.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":exception");
  sqlite3_bind_text(stmt, idx, e->exception.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":bodyLine");
  sqlite3_bind_int(stmt, idx, e->bodyLine);

  idx = sqlite3_bind_parameter_index(stmt, ":endBodyLine");
  sqlite3_bind_int(stmt, idx, e->endBodyLine);

  idx = sqlite3_bind_parameter_index(stmt, ":mGrpId");
  sqlite3_bind_int(stmt, idx, e->mGrpId);

  idx = sqlite3_bind_parameter_index(stmt, ":fileName");
  sqlite3_bind_text(stmt, idx, e->fileName.data(), e->fileName.length(),
		    SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":startLine");
  sqlite3_bind_int(stmt, idx, e->startLine);

  idx = sqlite3_bind_parameter_index(stmt, ":lang");
  sqlite3_bind_int(stmt, idx, (int) e->lang);

  idx = sqlite3_bind_parameter_index(stmt, ":hidden");
  sqlite3_bind_int(stmt, idx, e->hidden);

  idx = sqlite3_bind_parameter_index(stmt, ":artificial");
  sqlite3_bind_int(stmt, idx, e->artificial);

  idx = sqlite3_bind_parameter_index(stmt, ":groupDocType");
  sqlite3_bind_int(stmt, idx, e->groupDocType);

  rc = sqlite3_step(stmt);
  if ((rc != SQLITE_DONE) && (rc != SQLITE_ROW)) {
    printf("failed to add row: code %d\n", rc);
    printf("query is: %s\n", insert_entry_query);
    exit(-1);
  }
  sqlite3_reset(stmt);

  printf("successfully added a row: id %d\n", e->id);

  return 0;
}

// TODO: fix this member function
int Sqlite3Store::updateEntry(Entry *e) {
  int rc;
  sqlite3_stmt *stmt = update_entry_stmt;
  int idx;

  ASSERT(stmt != NULL);

  // printEntry(e);

  idx = sqlite3_bind_parameter_index(stmt, ":id");
  sqlite3_bind_int(stmt, idx, e->id);

  idx = sqlite3_bind_parameter_index(stmt, ":parent_id");
  sqlite3_bind_int(stmt, idx, e->get_parent_id());

  idx = sqlite3_bind_parameter_index(stmt, ":name");
  sqlite3_bind_text(stmt, idx, e->name.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":type");
  sqlite3_bind_text(stmt, idx, e->type.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":section");
  sqlite3_bind_int(stmt, idx, e->section);

  idx = sqlite3_bind_parameter_index(stmt, ":protection");
  sqlite3_bind_int(stmt, idx, (int) e->protection);

  idx = sqlite3_bind_parameter_index(stmt, ":mtype");
  sqlite3_bind_int(stmt, idx, (int) e->mtype);

  idx = sqlite3_bind_parameter_index(stmt, ":spec");
  sqlite3_bind_int(stmt, idx, e->spec);

  idx = sqlite3_bind_parameter_index(stmt, ":initLines");
  sqlite3_bind_int(stmt, idx, e->initLines);

  idx = sqlite3_bind_parameter_index(stmt, ":stat");
  sqlite3_bind_int(stmt, idx, e->stat);

  idx = sqlite3_bind_parameter_index(stmt, ":explicitExternal");
  sqlite3_bind_int(stmt, idx, e->explicitExternal);

  idx = sqlite3_bind_parameter_index(stmt, ":proto");
  sqlite3_bind_int(stmt, idx, e->proto);

  idx = sqlite3_bind_parameter_index(stmt, ":subGrouping");
  sqlite3_bind_int(stmt, idx, e->subGrouping);

  idx = sqlite3_bind_parameter_index(stmt, ":callGraph");
  sqlite3_bind_int(stmt, idx, e->callGraph);

  idx = sqlite3_bind_parameter_index(stmt, ":callerGraph");
  sqlite3_bind_int(stmt, idx, e->callerGraph);

  idx = sqlite3_bind_parameter_index(stmt, ":virt");
  sqlite3_bind_int(stmt, idx, (int) e->virt);

  idx = sqlite3_bind_parameter_index(stmt, ":args");
  sqlite3_bind_text(stmt, idx, e->args.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":bitfields");
  sqlite3_bind_text(stmt, idx, e->bitfields.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":program");
  sqlite3_bind_text(stmt, idx, e->program.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":initializer");
  sqlite3_bind_text(stmt, idx, e->initializer.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":includeFile");
  sqlite3_bind_text(stmt, idx, e->includeFile.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":includeName");
  sqlite3_bind_text(stmt, idx, e->includeName.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":doc");
  sqlite3_bind_text(stmt, idx, e->doc.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":docLine");
  sqlite3_bind_int(stmt, idx, e->docLine);

  idx = sqlite3_bind_parameter_index(stmt, ":docFile");
  sqlite3_bind_text(stmt, idx, e->docFile.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":brief");
  sqlite3_bind_text(stmt, idx, e->brief.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":briefLine");
  sqlite3_bind_int(stmt, idx, e->briefLine);

  idx = sqlite3_bind_parameter_index(stmt, ":briefFile");
  sqlite3_bind_text(stmt, idx, e->briefFile.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":inbodyDocs");
  sqlite3_bind_text(stmt, idx, e->inbodyDocs.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":inbodyLine");
  sqlite3_bind_int(stmt, idx, e->inbodyLine);

  idx = sqlite3_bind_parameter_index(stmt, ":inbodyFile");
  sqlite3_bind_text(stmt, idx, e->inbodyFile.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":relates");
  sqlite3_bind_text(stmt, idx, e->relates.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":relatesType");
  sqlite3_bind_int(stmt, idx, e->relatesType);

  idx = sqlite3_bind_parameter_index(stmt, ":read");
  sqlite3_bind_text(stmt, idx, e->read.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":write");
  sqlite3_bind_text(stmt, idx, e->write.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":inside");
  sqlite3_bind_text(stmt, idx, e->inside.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":exception");
  sqlite3_bind_text(stmt, idx, e->exception.data(), -1, SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":bodyLine");
  sqlite3_bind_int(stmt, idx, e->bodyLine);

  idx = sqlite3_bind_parameter_index(stmt, ":endBodyLine");
  sqlite3_bind_int(stmt, idx, e->endBodyLine);

  idx = sqlite3_bind_parameter_index(stmt, ":mGrpId");
  sqlite3_bind_int(stmt, idx, e->mGrpId);

  idx = sqlite3_bind_parameter_index(stmt, ":fileName");
  sqlite3_bind_text(stmt, idx, e->fileName.data(), e->fileName.length(),
		    SQLITE_STATIC);

  idx = sqlite3_bind_parameter_index(stmt, ":startLine");
  sqlite3_bind_int(stmt, idx, e->startLine);

  idx = sqlite3_bind_parameter_index(stmt, ":lang");
  sqlite3_bind_int(stmt, idx, (int) e->lang);

  idx = sqlite3_bind_parameter_index(stmt, ":hidden");
  sqlite3_bind_int(stmt, idx, e->hidden);

  idx = sqlite3_bind_parameter_index(stmt, ":artificial");
  sqlite3_bind_int(stmt, idx, e->artificial);

  idx = sqlite3_bind_parameter_index(stmt, ":groupDocType");
  sqlite3_bind_int(stmt, idx, e->groupDocType);

  rc = sqlite3_step(stmt);
  if ((rc != SQLITE_DONE) && (rc != SQLITE_ROW)) {
    printf("failed to update row: code %d\n", rc);
    printf("query is: %s\n", update_entry_query);
    exit(-1);
  }
  sqlite3_reset(stmt);

  printf("successfully updated a row: id %d\n", e->id);

  return 0;
}

int Sqlite3Store::countEntries(int id) {
  int count = 0;
  sqlite3_stmt *stmt = count_entry_by_id_stmt;

  ASSERT(stmt != NULL);

  int idx = sqlite3_bind_parameter_index(stmt, ":id");
  sqlite3_bind_int(stmt, idx, id);

  int rc = sqlite3_step(stmt);

  if ((rc != SQLITE_DONE) && (rc != SQLITE_ROW)) {
    printf("failed execute query: code %d\n", rc);
    printf("query is: %s\n", count_entry_by_id_query);
    exit(-1);
  }

  count = sqlite3_column_int(stmt, 0);
  printf("id: %d: count: %d\n", id, count);
  sqlite3_reset(stmt);

  return count;
}

Entry *Sqlite3Store::loadEntry(int id) {
  Entry* e = this->readEntry(id);

  delete e->tagInfo;
  e->tagInfo = this->readTagInfo(e);

    ListType list_type = MEMBER_ARGS_LIST;
  ArgumentList *al = this->readArgsList(e, list_type);
  if(al) {
    delete e->argList;
    e->argList = al;
  }

  e->tArgLists = this->readTArgsList(e);

  e->typeConstr = this->readTypeConstr(e);

  delete e->extends;
  e->extends = this->readExtends(e);

  delete e->groups;
  e->groups = this->readGroups(e);

  delete e->anchors;
  e->anchors = this->readAnchors(e);

  e->sli = this->readSli(e);
  
  return e;
}

// TODO: fix this member function
Entry *Sqlite3Store::readEntry(int id) {
  sqlite3_stmt *stmt = load_entry_stmt;
  int rc;
  int idx = sqlite3_bind_parameter_index(stmt, ":id");
  sqlite3_bind_int(stmt, idx, id);

  //Load Entry with data from database
  Entry *e = 0;
  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW) {
    e = new Entry;
    idx = 0;
    e->id = sqlite3_column_int(stmt, idx++);
    e->set_parent_id(sqlite3_column_int(stmt, idx++));
    e->name = ((const char*) sqlite3_column_text(stmt, idx++));
    e->type = ((const char*) sqlite3_column_text(stmt, idx++));
    e->section = sqlite3_column_int(stmt, idx++);
    e->protection = (Protection) sqlite3_column_int(stmt, idx++);
    e->mtype = (MethodTypes) sqlite3_column_int(stmt, idx++);
    e->spec = sqlite3_column_int(stmt, idx++);
    e->initLines = sqlite3_column_int(stmt, idx++);
    e->stat = sqlite3_column_int(stmt, idx++);
    e->explicitExternal = sqlite3_column_int(stmt, idx++);
    e->proto = sqlite3_column_int(stmt, idx++);
    e->subGrouping = sqlite3_column_int(stmt, idx++);
    e->callGraph = sqlite3_column_int(stmt, idx++);
    e->callerGraph = sqlite3_column_int(stmt, idx++);
    e->virt = (Specifier) sqlite3_column_int(stmt, idx++);
    e->args = ((const char*) sqlite3_column_text(stmt, idx++));
    e->bitfields = ((const char*) sqlite3_column_text(stmt, idx++));
    e->program = (const char *) sqlite3_column_text(stmt, idx++);
    e->initializer = ((const char *) sqlite3_column_text(stmt, idx++));
    e->includeFile = ((const char *) sqlite3_column_text(stmt, idx++));
    e->includeName = ((const char *) sqlite3_column_text(stmt, idx++));
    e->doc = ((const char *) sqlite3_column_text(stmt, idx++));
    e->docLine = sqlite3_column_int(stmt, idx++);
    e->docFile = ((const char *) sqlite3_column_text(stmt, idx++));
    e->brief = ((const char *) sqlite3_column_text(stmt, idx++));
    e->briefLine = sqlite3_column_int(stmt, idx++);
    e->briefFile = ((const char *) sqlite3_column_text(stmt, idx++));
    e->inbodyDocs = ((const char *) sqlite3_column_text(stmt, idx++));
    e->inbodyLine = sqlite3_column_int(stmt, idx++);
    e->inbodyFile = ((const char *) sqlite3_column_text(stmt, idx++));
    e->relates = ((const char *) sqlite3_column_text(stmt, idx++));
    e->relatesType = (RelatesType) sqlite3_column_int(stmt, idx++);
    e->read = ((const char *) sqlite3_column_text(stmt, idx++));
    e->write = ((const char *) sqlite3_column_text(stmt, idx++));
    e->inside = ((const char *) sqlite3_column_text(stmt, idx++));
    e->exception = ((const char *) sqlite3_column_text(stmt, idx++));
    e->bodyLine = sqlite3_column_int(stmt, idx++);
    e->endBodyLine = sqlite3_column_int(stmt, idx++);
    e->mGrpId = sqlite3_column_int(stmt, idx++);
    e->fileName = ((const char *) sqlite3_column_text(stmt, idx++));
    e->startLine = sqlite3_column_int(stmt, idx++);
    e->lang = (SrcLangExt) sqlite3_column_int(stmt, idx++);
    e->hidden = sqlite3_column_int(stmt, idx++);
    e->artificial = sqlite3_column_int(stmt, idx++);
    e->groupDocType = (Entry::GroupDocType) sqlite3_column_int(stmt, idx++);

    printf("successfully retrieved Entry\n");
  } else {
    printf("Entry not found for id: %d\n", id);
  }
  sqlite3_reset(stmt);

  return e;
}

// TODO: add implementation
TagInfo* Sqlite3Store::readTagInfo(Entry *e) {
  int rc;
  int idx;
  sqlite3_stmt *stmt = load_taginfo_stmt;
  ASSERT(stmt != NULL);
  idx = sqlite3_bind_parameter_index(stmt, ":entry_id");
  sqlite3_bind_int(stmt, idx, e->id);

  TagInfo *ti = 0;
  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW) {
    ti  = new TagInfo;
    int idx = 0;
    int id  = sqlite3_column_int(stmt, idx++);
    int entry_id  = sqlite3_column_int(stmt, idx++);
    ti->tagName  = ((const char *) sqlite3_column_text(stmt, idx++));
    ti->fileName = ((const char *) sqlite3_column_text(stmt, idx++));
    ti->anchor = ((const char *) sqlite3_column_text(stmt, idx++));
    printf("taginfo read successfully\n");
  } else {
    printf("taginfo not found for entry: %d\n", e->id);
  }
  sqlite3_reset(stmt);
  return ti;
}

ArgumentList* Sqlite3Store::readArgsList(Entry *e, ListType list_type) {
  int rc;
  int idx;
  sqlite3_stmt *stmt = load_argument_list_stmt;
  ASSERT(stmt != NULL);
  idx = sqlite3_bind_parameter_index(stmt, ":entry_id");
  sqlite3_bind_int(stmt, idx, e->id);

  idx = sqlite3_bind_parameter_index(stmt, ":list_type");
  sqlite3_bind_int(stmt, idx, (int)list_type);

  ArgumentList *argsList = 0;

  //Load Entry with data from database
  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW) {
    int idx = 0;
    int list_id  = sqlite3_column_int(stmt, idx++);
    int entry_id = sqlite3_column_int(stmt, idx++);
    int list_type = sqlite3_column_int(stmt, idx++);
    int constSpecifier = sqlite3_column_int(stmt, idx++);
    int volatileSpecifier = sqlite3_column_int(stmt, idx++);
    int pureSpecifier = sqlite3_column_int(stmt, idx++);
    argsList = this->readArgumentList(list_id);
    argsList->constSpecifier = (bool) constSpecifier;
    argsList->volatileSpecifier = (bool) volatileSpecifier;
    argsList->pureSpecifier = (bool) pureSpecifier;
    printf("successfully retrieved a row\n");
  } else {
    printf("no rows found for id: %d\n", e->id);
  }
  sqlite3_reset(stmt);

  return argsList;
}


ArgumentList* Sqlite3Store::readArgumentList(int list_id) {
  int rc;
  int idx;
  sqlite3_stmt *stmt = load_argument_stmt;
  ASSERT(stmt != NULL);

  idx = sqlite3_bind_parameter_index(stmt, ":argument_list_id");
  sqlite3_bind_int(stmt, idx, list_id);

  ArgumentList *result = new ArgumentList;
  
  rc = sqlite3_step(stmt);
  while (SQLITE_ROW == rc) {
    Argument *a = new Argument;
    int idx = 0;
    int id  = sqlite3_column_int(stmt, idx++);
    int argument_list_id  = sqlite3_column_int(stmt, idx++);
    a->attrib  = ((const char *) sqlite3_column_text(stmt, idx++));
    a->type    = ((const char *) sqlite3_column_text(stmt, idx++));
    a->canType = ((const char *) sqlite3_column_text(stmt, idx++));
    a->name    = ((const char *) sqlite3_column_text(stmt, idx++));
    a->array   = ((const char *) sqlite3_column_text(stmt, idx++));
    a->defval  = ((const char *) sqlite3_column_text(stmt, idx++));
    a->docs    = ((const char *) sqlite3_column_text(stmt, idx++));
    result->append(a);
    printf("successfully retrieved a row\n");
    rc = sqlite3_step(stmt);
  }

  sqlite3_reset(stmt);

  return result;
}

QList<ArgumentList> *Sqlite3Store::readTArgsList(Entry *e) {
  int rc;
  int idx;

  sqlite3_stmt *stmt = load_argument_list_stmt;
  ASSERT(stmt != NULL);

  idx = sqlite3_bind_parameter_index(stmt, ":id");
  sqlite3_bind_int(stmt, idx, e->id);

  ListType list_type =TEMPLATE_ARGS_LIST;
  idx = sqlite3_bind_parameter_index(stmt, ":list_type");
  sqlite3_bind_int(stmt, idx, (int)list_type);

  QList<ArgumentList> *result = new QList<ArgumentList>;
  result->setAutoDelete(TRUE);

  //Load Entry with data from database
  rc = sqlite3_step(stmt);
  while (rc == SQLITE_ROW) {

    int idx = 0;
    int list_id  = sqlite3_column_int(stmt, idx++);
    int entry_id = sqlite3_column_int(stmt, idx++);
    int list_type = sqlite3_column_int(stmt, idx++);
    int constSpecifier = sqlite3_column_int(stmt, idx++);
    int volatileSpecifier = sqlite3_column_int(stmt, idx++);
    int pureSpecifier = sqlite3_column_int(stmt, idx++);

    ArgumentList* argsList = this->readArgumentList(list_id);
    argsList->constSpecifier = (bool) constSpecifier;
    argsList->volatileSpecifier = (bool) volatileSpecifier;
    argsList->pureSpecifier = (bool) pureSpecifier;

    result->append(argsList);
    printf("successfully retrieved a row\n");

    rc = sqlite3_step(stmt);
  }
  sqlite3_reset(stmt);

  if(result->count()) {
      return result;
  } else {
      delete result;
      return 0;
  }
}

ArgumentList* Sqlite3Store::readTypeConstr(Entry *e) {
  int rc;
  int idx;
  sqlite3_stmt *stmt = load_argument_list_stmt;
  ASSERT(stmt != NULL);

  ListType list_type = TYPE_CONSTRAINT_ARGS_LIST;

  idx = sqlite3_bind_parameter_index(stmt, ":id");
  sqlite3_bind_int(stmt, idx, e->id);

  idx = sqlite3_bind_parameter_index(stmt, ":list_type");
  sqlite3_bind_int(stmt, idx, (int)list_type);

  //Load Entry with data from database
  ArgumentList *argsList = 0;

  rc = sqlite3_step(stmt);
  if (rc == SQLITE_ROW) {
    int idx = 0;
    int list_id  = sqlite3_column_int(stmt, idx++);
    int entry_id = sqlite3_column_int(stmt, idx++);
    int list_type = sqlite3_column_int(stmt, idx++);
    int constSpecifier = sqlite3_column_int(stmt, idx++);
    int volatileSpecifier = sqlite3_column_int(stmt, idx++);
    int pureSpecifier = sqlite3_column_int(stmt, idx++);

    argsList = this->readArgumentList(list_id);
    argsList->constSpecifier = (bool) constSpecifier;
    argsList->volatileSpecifier = (bool) volatileSpecifier;
    argsList->pureSpecifier = (bool) pureSpecifier;
    printf("successfully retrieved a row\n");
  } else {
    printf("no rows found for id: %d\n", e->id);
  }
  sqlite3_reset(stmt);

  return argsList;
}

QList<BaseInfo> * Sqlite3Store::readExtends(Entry *e) {
  int rc;
  int idx;
  sqlite3_stmt *stmt = load_baseinfo_stmt;
  ASSERT(stmt != NULL);

  idx = sqlite3_bind_parameter_index(stmt, ":entry_id");
  sqlite3_bind_int(stmt, idx, e->id);

  QList<BaseInfo> *result = new QList<BaseInfo>;
  result->setAutoDelete(TRUE);

  rc = sqlite3_step(stmt);
  while( SQLITE_ROW == rc ) {
    int idx = 0;
    int id = sqlite3_column_int(stmt, idx++);
    int entry_id = sqlite3_column_int(stmt, idx++);
    QCString name   = ((const char *) sqlite3_column_text(stmt, idx++));
    Protection prot = (Protection) sqlite3_column_int(stmt, idx++);
    Specifier virt  = (Specifier) sqlite3_column_int(stmt, idx++);
    
    result->append(new BaseInfo(name,prot,virt));
    rc = sqlite3_step(stmt);
  }

  sqlite3_reset(stmt);

  return result;
}

QList<Grouping>* Sqlite3Store::readGroups(Entry *e) {
  int rc;
  int idx;
  sqlite3_stmt *stmt = load_grouping_stmt;
  ASSERT(stmt != NULL);

  idx = sqlite3_bind_parameter_index(stmt, ":entry_id");
  sqlite3_bind_int(stmt, idx, e->id);

  QList<Grouping> *result = new QList<Grouping>;
  result->setAutoDelete(TRUE);
  rc = sqlite3_step(stmt);
  while( SQLITE_ROW == rc ) {
    int idx = 0;
    int id = sqlite3_column_int(stmt, idx++);
    int entry_id = sqlite3_column_int(stmt, idx++);
    QCString name =  ((const char *)sqlite3_column_text(stmt, idx++));
    Grouping::GroupPri_t prio = (Grouping::GroupPri_t) sqlite3_column_int(stmt, idx++);
    result->append(new Grouping(name,prio));
    rc = sqlite3_step(stmt);
  }

  sqlite3_reset(stmt);
  return result;
}

QList<SectionInfo> * Sqlite3Store::readAnchors(Entry *e) {
  int rc;
  int idx;
  sqlite3_stmt *stmt = load_sectioninfo_stmt;
  ASSERT(stmt != NULL);

  idx = sqlite3_bind_parameter_index(stmt, ":entry_id");
  sqlite3_bind_int(stmt, idx, e->id);

  QList<SectionInfo> *result = new QList<SectionInfo>;
  result->setAutoDelete(TRUE);

  rc = sqlite3_step(stmt);
  while( SQLITE_ROW == rc ) {
    int idx = 0;

    int id = sqlite3_column_int(stmt, idx++);
    int entry_id = sqlite3_column_int(stmt, idx++);

    QCString label = ((const char *)sqlite3_column_text(stmt, idx++));
    QCString title = ((const char *)sqlite3_column_text(stmt, idx++));
    SectionInfo::SectionType type = (SectionInfo::SectionType) sqlite3_column_int(stmt, idx++);
    QCString ref   = ((const char *)sqlite3_column_text(stmt, idx++));
    QCString fileName = ((const char *)sqlite3_column_text(stmt, idx++));

    result->append(new SectionInfo(fileName,label,title,type,ref));

    rc = sqlite3_step(stmt);
  }

  sqlite3_reset(stmt);
  return result;
}

QList<ListItemInfo> * Sqlite3Store::readSli(Entry *e) {
  int rc;
  int idx;
  sqlite3_stmt *stmt = load_listiteminfo_stmt;
  ASSERT(stmt != NULL);

  idx = sqlite3_bind_parameter_index(stmt, ":entry_id");
  sqlite3_bind_int(stmt, idx, e->id);

  QList<ListItemInfo> *result = new QList<ListItemInfo>;
  result->setAutoDelete(TRUE);

  rc = sqlite3_step(stmt);
  while( SQLITE_ROW == rc ) {
    ListItemInfo *lii = new ListItemInfo;

    int idx = 0;
    int id = sqlite3_column_int(stmt, idx++);
    int entry_id = sqlite3_column_int(stmt, idx++);
    lii->type   = ((const char *)sqlite3_column_text(stmt, idx++));
    lii->itemId = sqlite3_column_int(stmt, idx++);
    result->append(lii);

    rc = sqlite3_step(stmt);
  }

  sqlite3_reset(stmt);
  return result;
}


int Sqlite3Store::addTagInfo(Entry *e) {
  int rc = 0;
  sqlite3_stmt *stmt = insert_taginfo_stmt;
  int idx;

  ASSERT(stmt != NULL);

  // this->printEntry(e);
  if(e->tagInfo) {
    TagInfo *ti = e->tagInfo;
    idx = sqlite3_bind_parameter_index(stmt, ":entry_id");
    sqlite3_bind_int(stmt, idx, e->id);

    idx = sqlite3_bind_parameter_index(stmt, ":tagName");
    sqlite3_bind_text(stmt, idx, ti->tagName.data(), -1, SQLITE_STATIC);

    idx = sqlite3_bind_parameter_index(stmt, ":fileName");
    sqlite3_bind_text(stmt, idx, ti->fileName.data(), -1, SQLITE_STATIC);

    idx = sqlite3_bind_parameter_index(stmt, ":anchor");
    sqlite3_bind_text(stmt, idx, ti->anchor.data(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if ((rc != SQLITE_DONE) && (rc != SQLITE_ROW)) {
      printf("failed to add row: code %d\n", rc);
      printf("query is: %s\n", insert_entry_query);
      exit(-1);
    }
    sqlite3_reset(stmt);
    printf("successfully added a tagInfo for entry id %d\n", e->id);
  }
  return rc;
}

  
int Sqlite3Store::addArgsList(Entry *e, ArgumentList *argList, ListType list_type) {
  int rc = 0;
  sqlite3_stmt *stmt = insert_argument_list_stmt;
  int idx;
  
  ASSERT(stmt != NULL);

  if (argList==0) {
    // do nothing
  } else {
    if (argList->count()>0) {
      idx = sqlite3_bind_parameter_index(stmt, ":entry_id");
      sqlite3_bind_int(stmt, idx, e->id);
      
      idx = sqlite3_bind_parameter_index(stmt, ":list_type");
      sqlite3_bind_int(stmt, idx, list_type);
      
      idx = sqlite3_bind_parameter_index(stmt, ":constSpecifier");
      sqlite3_bind_int(stmt, idx, argList->constSpecifier);
      
      idx = sqlite3_bind_parameter_index(stmt, ":volatileSpecifier");
      sqlite3_bind_int(stmt, idx, argList->volatileSpecifier);
      
      idx = sqlite3_bind_parameter_index(stmt, ":pureSpecifier");
      sqlite3_bind_int(stmt, idx, argList->pureSpecifier);
      
      rc = sqlite3_step(stmt);
      if ((rc != SQLITE_DONE) && (rc != SQLITE_ROW)) {
	printf("failed to add row: code %d\n", rc);
	printf("query is: %s\n", insert_argument_list_query);
	exit(-1);
      }
      sqlite3_reset(stmt);

      sqlite3_int64 list_id = sqlite3_last_insert_rowid(this->db);
      printf("successfully added a argslist: %d for entry id %d\n", (int) list_id, e->id);
      
      ArgumentListIterator ali(*argList);
      Argument *a;
      for (ali.toFirst();(a=ali.current());++ali) {
	this->addArgument((int)list_id, a);
      }
    }
  }

  return rc;
}

int Sqlite3Store::addArgument(int list_id, Argument *a) {
  int rc = 0;
  sqlite3_stmt *stmt = insert_argument_stmt;
  int idx;

  ASSERT(stmt != NULL);

  idx = sqlite3_bind_parameter_index(stmt, ":argument_list_id");
  sqlite3_bind_int(stmt, idx, list_id);
  
  idx = sqlite3_bind_parameter_index(stmt, ":attrib");
  sqlite3_bind_text(stmt, idx, a->attrib.data(), -1, SQLITE_STATIC);
  
  idx = sqlite3_bind_parameter_index(stmt, ":type");
  sqlite3_bind_text(stmt, idx, a->type.data(), -1, SQLITE_STATIC);
  
  idx = sqlite3_bind_parameter_index(stmt, ":canType");
  sqlite3_bind_text(stmt, idx, a->canType.data(), -1, SQLITE_STATIC);
  
  idx = sqlite3_bind_parameter_index(stmt, ":name");
  sqlite3_bind_text(stmt, idx, a->name.data(), -1, SQLITE_STATIC);
  
  idx = sqlite3_bind_parameter_index(stmt, ":array");
  sqlite3_bind_text(stmt, idx, a->array.data(), -1, SQLITE_STATIC);
  
  idx = sqlite3_bind_parameter_index(stmt, ":defval");
  sqlite3_bind_text(stmt, idx, a->defval.data(), -1, SQLITE_STATIC);
  
  idx = sqlite3_bind_parameter_index(stmt, ":docs");
  sqlite3_bind_text(stmt, idx, a->docs.data(), -1, SQLITE_STATIC);
  
  rc = sqlite3_step(stmt);
  if ((rc != SQLITE_DONE) && (rc != SQLITE_ROW)) {
    printf("failed to add row: code %d\n", rc);
    printf("query is: %s\n", insert_argument_query);
    exit(-1);
  }
  sqlite3_reset(stmt);
  return rc;
}

int Sqlite3Store::addTArgsList(Entry *e) {
  int rc = 0;
  sqlite3_stmt *stmt = insert_argument_list_stmt;
  int idx;
  
  ASSERT(stmt != NULL);

  QList<ArgumentList> *tArgLists = e->tArgLists;
  ListType list_type = TEMPLATE_ARGS_LIST;

  if (tArgLists==0) {
    // do nothing
  } else {
    QListIterator<ArgumentList> ali(*tArgLists);
    ArgumentList *al;
    for (ali.toFirst();(al=ali.current());++ali) {
      rc = this->addArgsList(e, al, list_type);
    }
  }
  return rc;
}

int Sqlite3Store::addTypeConstr(Entry *e) {
  int rc = 0;
  sqlite3_stmt *stmt = insert_argument_list_stmt;
  int idx;

  ASSERT(stmt != NULL);

  ArgumentList *typeConstr = e->typeConstr;
  ListType list_type = TYPE_CONSTRAINT_ARGS_LIST;

  if (typeConstr ==0) {
    // do nothing
  } else {
    rc = this->addArgsList(e, typeConstr, list_type);
  }
  return rc;
}


int Sqlite3Store::addExtends(Entry *e) {
  int rc = 0;

  sqlite3_stmt *stmt = insert_baseinfo_stmt;
  int idx;

  ASSERT(stmt != NULL);

  QList<BaseInfo> *baseList = e->extends;

  if (baseList ==0) {
    // do nothing
  } else {
    QListIterator<BaseInfo> bli(*baseList);
    BaseInfo *bi;
    for (bli.toFirst();(bi=bli.current());++bli) {
      
      idx = sqlite3_bind_parameter_index(stmt, ":entry_id");
      sqlite3_bind_int(stmt, idx, e->id);
      
      idx = sqlite3_bind_parameter_index(stmt, ":name");
      sqlite3_bind_text(stmt, idx, bi->name.data(), -1, SQLITE_STATIC);
      
      idx = sqlite3_bind_parameter_index(stmt, ":prot");
      sqlite3_bind_int(stmt, idx, (int)bi->prot);
      
      idx = sqlite3_bind_parameter_index(stmt, ":virt");
      sqlite3_bind_int(stmt, idx, (int)bi->virt);
      rc = sqlite3_step(stmt);

      if ((rc != SQLITE_DONE) && (rc != SQLITE_ROW)) {
	printf("failed to add row: code %d\n", rc);
	printf("query is: %s\n", insert_baseinfo_query);
	exit(-1);
      }
      sqlite3_reset(stmt);
    }
    
  }
  return rc;
}

int Sqlite3Store::addGroups(Entry *e) {
  int rc = 0;

  sqlite3_stmt *stmt = insert_grouping_stmt;
  int idx;

  ASSERT(stmt != NULL);

  QList<Grouping> *groups = e->groups;

  if (groups==0) {
    // do nothing
  } else {
    QListIterator<Grouping> gli(*groups);
    Grouping *g;
    for (gli.toFirst();(g=gli.current());++gli) {
      idx = sqlite3_bind_parameter_index(stmt, ":entry_id");
      sqlite3_bind_int(stmt, idx, e->id);
      
      idx = sqlite3_bind_parameter_index(stmt, ":groupname");
      sqlite3_bind_text(stmt, idx, g->groupname.data(), -1, SQLITE_STATIC);
      
      idx = sqlite3_bind_parameter_index(stmt, ":pri");
      sqlite3_bind_int(stmt, idx, (int)g->pri);
      
      rc = sqlite3_step(stmt);
      
      if ((rc != SQLITE_DONE) && (rc != SQLITE_ROW)) {
	printf("failed to add row: code %d\n", rc);
	printf("query is: %s\n", insert_grouping_query);
	exit(-1);
      }
      sqlite3_reset(stmt);
    }
  }
  
  return rc;
}

int Sqlite3Store::addAnchors(Entry *e) {
  int rc = 0;

  sqlite3_stmt *stmt = insert_sectioninfo_stmt;
  int idx;

  ASSERT(stmt != NULL);

  QList<SectionInfo> *anchors = e->anchors;

  if (anchors==0) {
    // do nothing
  } else {
    QListIterator<SectionInfo> sli(*anchors);
    SectionInfo *si;
    for (sli.toFirst();(si=sli.current());++sli) {
      idx = sqlite3_bind_parameter_index(stmt, ":entry_id");
      sqlite3_bind_int(stmt, idx, e->id);
      
      idx = sqlite3_bind_parameter_index(stmt, ":label");
      sqlite3_bind_text(stmt, idx, si->label.data(), -1, SQLITE_STATIC);
      
      idx = sqlite3_bind_parameter_index(stmt, ":title");
      sqlite3_bind_text(stmt, idx, si->title.data(), -1, SQLITE_STATIC);
      
      idx = sqlite3_bind_parameter_index(stmt, ":ref");
      sqlite3_bind_text(stmt, idx, si->ref.data(), -1, SQLITE_STATIC);
      
      idx = sqlite3_bind_parameter_index(stmt, ":type");
      sqlite3_bind_int(stmt, idx, (int)si->type);

      idx = sqlite3_bind_parameter_index(stmt, ":fileName");
      sqlite3_bind_text(stmt, idx, si->fileName.data(), -1, SQLITE_STATIC);
      
      rc = sqlite3_step(stmt);
      
      if ((rc != SQLITE_DONE) && (rc != SQLITE_ROW)) {
	printf("failed to add row: code %d\n", rc);
	printf("query is: %s\n", insert_sectioninfo_query);
	exit(-1);
      }
      sqlite3_reset(stmt);
    }
  }
  
  return rc;
}

int Sqlite3Store::addSli(Entry *e) {
  int rc = 0;
  sqlite3_stmt *stmt = insert_listiteminfo_stmt;
  int idx;

  ASSERT(stmt != NULL);

  QList<ListItemInfo> *sli = e->sli;

  if (sli==0) {
    // do nothing
  } else {
    QListIterator<ListItemInfo> liii(*sli);
    ListItemInfo *lii;
    for (liii.toFirst();(lii=liii.current());++liii) {
      
      idx = sqlite3_bind_parameter_index(stmt, ":entry_id");
      sqlite3_bind_int(stmt, idx, e->id);
      
      idx = sqlite3_bind_parameter_index(stmt, ":type");
      sqlite3_bind_text(stmt, idx, lii->type.data(), -1, SQLITE_STATIC);
      
      idx = sqlite3_bind_parameter_index(stmt, ":type");
      sqlite3_bind_int(stmt, idx, (int)lii->itemId);

      rc = sqlite3_step(stmt);
      
      if ((rc != SQLITE_DONE) && (rc != SQLITE_ROW)) {
	printf("failed to add row: code %d\n", rc);
	printf("query is: %s\n", insert_listiteminfo_query);
	exit(-1);
      }
      sqlite3_reset(stmt);
    }
  }
  
  return rc;
}

