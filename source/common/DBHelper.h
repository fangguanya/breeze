﻿
/*
* mini-breeze License
* Copyright (C) 2014 YaweiZhang <yawei_zhang@foxmail.com>.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

/*
*  file desc
*  asynchronous operate db via multi-thread.
*  利用线程异步化DB操作。
*  
*/


#ifndef _DB_HELPER_H_
#define _DB_HELPER_H_
#include <Common.h>
#include <mysqlclient/errmsg.h>
#include <mysqlclient/mysql.h>

namespace  zsummer
{
	namespace mysql
	{
		enum class QueryErrorCode : unsigned short
		{
			QEC_SUCCESS = 0,
			QEC_UNQUERY,
			QEC_MYSQLERROR,
		};
		

		inline std::string EscapeString(const char * orgBuff, size_t lenght);
		inline std::string EscapeString(const std::string &orgField){ return EscapeString(orgField.c_str(), orgField.length()); }
		
		class DBResult : public std::enable_shared_from_this<DBResult>
		{
		public:
			DBResult(){}
			~DBResult(){if (m_res){mysql_free_result(m_res);m_res = nullptr;}}
		public:
			inline QueryErrorCode GetErrorCode(){ return m_ec; }
			inline std::string GetLastError(){ return m_lastErrorMsg; }
			inline unsigned long long GetAffectedRows(){ return m_affectedRows; }
			inline bool HaveRow(){ return m_row != nullptr; }
			const std::string & SQLString(){ return m_sql; }
			template<class T>
			inline DBResult & operator >>(T & t){t = _FromeString<T>(ExtractOneField());return *this;}

		public:
			void _SetQueryResult(QueryErrorCode qec, const std::string & sql, MYSQL * db);
		private:
			const char * ExtractOneField();
			template<class RET>
			inline RET _FromeString(const char * str)
			{
				std::stringstream ss(str);
				RET ret;
				ss >> ret;
				return std::move(ret);
			}
			template<class T>
			inline std::string _ToString(const T & t)
			{
				std::stringstream ss;
				ss << t;
				return ss.str();
			}
		private:
			QueryErrorCode m_ec = QueryErrorCode::QEC_UNQUERY;
			//sql
			std::string m_sql;
			//error
			std::string m_lastErrorMsg;

			//res
			MYSQL_RES * m_res = nullptr;
			MYSQL_ROW  m_row = nullptr;
			unsigned long long m_affectedRows = 0;
			unsigned int m_fieldCount = 0;
			unsigned int m_fieldCursor = 0;
		};
		typedef std::shared_ptr<DBResult> DBResultPtr;



		
		class DBHelper :public std::enable_shared_from_this<DBHelper>
		{
		public:
			DBHelper(const bool & isRuning) : m_isRuning(isRuning){}
			~DBHelper(){if (m_mysql){ mysql_close(m_mysql); m_mysql = nullptr; } }
			inline void Init(const DBConfig & dbconfig){ m_config = dbconfig; }
			inline bool Connect();
			bool WaitEnable();
			DBResultPtr Query(const std::string & sql);
		private:
			MYSQL * m_mysql = nullptr;
			DBConfig m_config;
			const bool & m_isRuning;
		};
		typedef std::shared_ptr<DBHelper> DBHelperPtr;




		class CDBClientManager
		{
		public:
			CDBClientManager();
			~CDBClientManager();
			bool Start();
			bool Stop();
		public:
			inline DBHelperPtr & getAuthDB(){ return m_authDB; }
			inline DBHelperPtr & getInfoDB(){ return m_infoDB; }
			inline DBHelperPtr & getLogDB(){ return m_logDB; }

			inline const std::atomic_ullong & getPostCount(){ return m_uPostCount; }
			inline const std::atomic_ullong & getFinalCount(){ return m_uFinalCount; }
		public:
			void async_query(DBHelperPtr &dbhelper, const string &sql,
				const std::function<void(DBResultPtr)> & handler);
		protected:
			void _async_query(DBHelperPtr &dbhelper, const string &sql,
				const std::function<void(DBResultPtr)> & handler);

			inline void Run();

		private:
			DBHelperPtr m_infoDB;
			DBHelperPtr m_logDB;
			DBHelperPtr m_authDB;
			std::shared_ptr<std::thread> m_thread;
			zsummer::network::ZSummerPtr m_summer;

			bool m_bRuning = false;
			char __tmpAlignas1[128];
			std::atomic_ullong m_uPostCount;
			char __tmpAlignas2[128];
			std::atomic_ullong m_uFinalCount;
			char __tmpAlignas3[128];
		};













		inline std::string EscapeString(const char * orgBuff, size_t lenght)
		{
			std::string ret;
			if (lenght == 0)
			{
				return std::move(ret);
			}
			for (size_t i = 0; i < lenght; ++i)
			{
				char ch = orgBuff[i];
				switch (ch)
				{
				case 0:
					ret += "\\0";
					break;
				case '\n':
					ret += "\\n";
					break;
				case '\r':
					ret += "\\r";
					break;
				case '\\':
					ret += "\\\\";
					break;
				case '\'':
					ret += "\\\'";
					break;
				case '"':
					ret += "\\\"";
					break;
				case '\032':
					ret += "Z";
					break;
				default:
					ret += ch;
				}
			}
			return std::move(ret);
		}

	};
};
















#endif