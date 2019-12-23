#include "Debug.h"
#include "MinNetPool.h"
#include "MinNetMySQL.h"

MYSQL * MinNetMySQL::connection = nullptr;
MYSQL MinNetMySQL::conn;
MYSQL_RES * MinNetMySQL::sql_result = nullptr;

std::queue<NonBlockingDataBaseIO *> MinNetMySQL::dataBaseIO = std::queue<NonBlockingDataBaseIO *>();
NonBlockingDataBaseIO * MinNetMySQL::nowIO = nullptr;

net_async_status MinNetMySQL::result = net_async_status::NET_ASYNC_COMPLETE;

MinNetSpinLock MinNetMySQL::spinLock;

bool MinNetMySQL::isConnected = false;
bool MinNetMySQL::query_end = false;
bool MinNetMySQL::store_end = false;
bool MinNetMySQL::fetch_end = false;
bool MinNetMySQL::free_end = false;
int MinNetMySQL::max_row = 0;
int MinNetMySQL::now_row = 0;

MinNetMySQL::MinNetMySQL()
{
}

MinNetMySQL::~MinNetMySQL()
{
}

void MinNetMySQL::ConnectToMySQL(const char * host, int port, const char * user, const char * password, const char * databaseName)
{
	if (mysql_init(&conn) == NULL)
	{
		Debug::Log("mysql_init 오류");
	}

	connection = mysql_real_connect(&conn, host, user, password, databaseName, port, (const char *)NULL, 0);
	if (connection == NULL)
	{
		Debug::Log("MySQL서버와 연결 실패 ", mysql_errno(&conn));
		return;
	}
	else
	{
		Debug::Log("MySQL 서버와 연결 성공 host " , host);
	}

	if (mysql_select_db(&conn, databaseName))
	{
		Debug::Log("데이터 베이스 접속 실패 ", mysql_errno(&conn));
		return;
	}

	isConnected = true;
}

void MinNetMySQL::IOprocessing()
{
	if (nowIO != nullptr)
	{
		if (!query_end)
		{// 쿼리문을 보냄
			result = mysql_real_query_nonblocking(connection, nowIO->query.c_str(), nowIO->query.length());
			query_end = result == net_async_status::NET_ASYNC_COMPLETE;
		}

		if (query_end && !store_end)
		{// 쿼리문을 보낸 결과를 받음
			result = mysql_store_result_nonblocking(connection, &sql_result);
			store_end = result == net_async_status::NET_ASYNC_COMPLETE;

			if (store_end)
			{
				max_row = mysql_num_rows(sql_result);
				nowIO->rowVector.resize(max_row);
			}
		}

		if (query_end && sql_result == nullptr)
		{// MySQL로 부터 받는 값이 없는 쿼리문을 완료함
			if (result == net_async_status::NET_ASYNC_ERROR)
			{
				auto error_number = mysql_errno(&conn);

				if (nowIO->callback != nullptr)
				{
					nowIO->callback(error_number, nullptr, -1, -1);
				}
			}
			else
			{
				nowIO->callback(0, nullptr, 0, 0);
			}

			IOprocessingReset();

			return;
		}

		if (query_end && store_end && !fetch_end)
		{// 받은 결과에서 행을 분석함
			result = mysql_fetch_row_nonblocking(sql_result, &nowIO->rowVector[now_row]);

			fetch_end = result == net_async_status::NET_ASYNC_COMPLETE;

			if (fetch_end)
			{// 하나의 fetch가 끝남
				now_row++;// 완료한 행 추가
				if (now_row < max_row)
				{// 아직 처리할 행이 남아있음
					fetch_end = false;
				}
			}
		}

		if (query_end && store_end && fetch_end && !free_end)
		{
			result = mysql_free_result_nonblocking(sql_result);
			free_end = result == net_async_status::NET_ASYNC_COMPLETE;
		}

		if (query_end && store_end && fetch_end && free_end)
		{// 모든 IO작업을 성공적으로 완료함
			auto num_fields = mysql_num_fields(sql_result);

			if (nowIO->callback != nullptr)
			{
				nowIO->callback(0, &nowIO->rowVector, max_row, num_fields);
			}

			IOprocessingReset();
		}

		if (result == net_async_status::NET_ASYNC_ERROR)
		{
			auto error_number = mysql_errno(&conn);

			if (nowIO->callback != nullptr)
			{
				nowIO->callback(error_number, nullptr, -1, -1);
			}

			IOprocessingReset();
		}
	}
	else
	{
		if (dataBaseIO.size() > 0)
		{
			nowIO = dataBaseIO.front();
			dataBaseIO.pop();
		}
	}

}

void MinNetMySQL::AddQuery(std::string query, DataBaseIOcallback callBack)
{
	auto io = MinNetPool::ioPool->pop();

	io->query = query;
	io->callback = callBack;
	
	spinLock.lock();
	dataBaseIO.push(io);
	spinLock.unlock();
}

void MinNetMySQL::IOprocessingReset()
{
	query_end = store_end = fetch_end = free_end = false;
	max_row = now_row = 0;

	if (nowIO != nullptr)
		delete nowIO;

	nowIO = nullptr;

	result = net_async_status::NET_ASYNC_COMPLETE;
}