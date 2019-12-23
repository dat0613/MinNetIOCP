#pragma once

#include <functional>
#include <vector>
#include <queue>
#include "MinNetOptimizer.h"
#include <mysql.h>

using DataBaseIOcallback = std::function<void(int, std::vector<MYSQL_ROW> *, int, int)>;

class NonBlockingDataBaseIO
{
public:

	std::string query;// Äõ¸®¹®
	DataBaseIOcallback callback;// ÄÝ¹é
	std::vector<MYSQL_ROW> rowVector;
};

static class MinNetMySQL
{
public:
	
	MinNetMySQL();
	~MinNetMySQL();

	static void ConnectToMySQL(const char * host, int port, const char * user, const char * password, const char * databaseName);
	static void IOprocessing();
	static void AddQuery(std::string query, DataBaseIOcallback callBack);

private:

	static void IOprocessingReset();

	static MinNetSpinLock spinLock;
	
	static bool isConnected;

	static MYSQL * connection;
	static MYSQL conn;
	static MYSQL_RES * sql_result;

	static std::queue<NonBlockingDataBaseIO *> dataBaseIO;
	static NonBlockingDataBaseIO * nowIO;

	static net_async_status result;

	static bool query_end;
	static bool store_end;
	static bool fetch_end;
	static bool free_end;
	static int max_row;
	static int now_row;
};