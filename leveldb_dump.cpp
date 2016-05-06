#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <leveldb/db.h>
#include <leveldb/options.h>
#include <leveldb/comparator.h>
#include <leveldb/env.h>

bool g_stop = false;

void print_help(const char* name);
void sign_handler(int sig);
int do_key_dump(const char* db_path, const char* dump_file);
leveldb::Status open_db_readonly(const char* db_path, leveldb::Options& options, leveldb::DB*& db);

using namespace std;

int main(int argc, char** argv)
{
  char* db_path = NULL;
  char* dump_file = NULL;
	int i = 0;

  while ((i = getopt(argc, argv, "p:f:d:")) != EOF)
  {
    switch (i)
    {
    case 'p':
      db_path = optarg;
      break;
    case 'd':
      dump_file = optarg;
      break;
    default:
      print_help(argv[0]);
      return 1;
    }
  }

  if (db_path == NULL || dump_file == NULL)
  {
    print_help(argv[0]);
    return 1;
  }

  signal(SIGINT, sign_handler);
  signal(SIGTERM, sign_handler);

	return do_key_dump(db_path, dump_file);
}

void print_help(const char* name)
{
  fprintf(stderr, "dump ldb data to file\n"
          "%s -p db_path -d dump_file",
          name);
}

void sign_handler(int sig)
{
  switch (sig)
  {
  case SIGTERM:
  case SIGINT:
    fprintf(stderr, "catch sig %d\n", sig);
    g_stop = true;
    break;
  default:
    break;
  }
}

int do_key_dump(const char* db_path, const char* dump_file)
{
  int dump_fd = -1;

  // open db
  leveldb::Options open_options;
  leveldb::DB* db = NULL;
  leveldb::Status s = open_db_readonly(db_path, open_options, db);
  if (!s.ok() || NULL == db)
  {
    fprintf(stderr, "open db fail: %s\n", s.ToString().c_str());
    return 1;
  }

  // get db iterator
  leveldb::ReadOptions scan_options;
  scan_options.verify_checksums = false;
  scan_options.fill_cache = false;
  leveldb::Iterator* db_it = db->NewIterator(scan_options);

  for (db_it->SeekToFirst(); !g_stop && db_it->Valid(); db_it->Next())
  {
    string key(const_cast<char*>(db_it->key().data()) + 2, db_it->key().size() - 2);
    cout << key << endl;
  }

  if (dump_fd > 0)
  {
    close(dump_fd);
  }

  if (db_it != NULL)
  {
    delete db_it;
  }
  if (db != NULL)
  {
    delete db;
    delete open_options.comparator;
    delete open_options.env;
    delete open_options.info_log;
  }

	return 0;

}

leveldb::Status open_db_readonly(const char* db_path,
                                 leveldb::Options& options, leveldb::DB*& db)
{
  options.error_if_exists = false; // exist is ok
  options.create_if_missing = false;

  char buf[32];
  snprintf(buf, sizeof(buf), "/dev/null");
  leveldb::Status s = options.env->NewLogger(buf, &options.info_log);
  if (s.ok())
  {
    s = leveldb::DB::Open(options, db_path, &db);
  }

  if (!s.ok())
  {
    fprintf(stderr, "open db fail: %s", s.ToString().c_str());
    delete options.comparator;
    delete options.env;
    if (options.info_log != NULL)
    {
      delete options.info_log;
    }
  }

  return s;
}
