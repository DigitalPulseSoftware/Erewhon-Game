// Copyright (C) 2018 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#ifndef EREWHON_SERVER_DATABASE_HPP
#define EREWHON_SERVER_DATABASE_HPP

#include <Nazara/Prerequisites.hpp>
#include <Server/Database/DatabaseConnection.hpp>
#include <Server/Database/DatabaseTransaction.hpp>
#include <Server/Database/DatabaseWorker.hpp>
#include <concurrentqueue/blockingconcurrentqueue.h>
#include <array>
#include <memory>
#include <string>
#include <vector>

namespace ewn
{
	template<typename T>
	struct PreparedStatement
	{
		static DatabaseResult Prepare(ewn::DatabaseConnection& conn)
		{
			return conn.PrepareStatement(T::StatementName, T::Query, T::Parameters.data(), T::Parameters.size());
		}
	};

	class Database
	{
		friend DatabaseWorker;

		public:
			using StatementCallback = std::function<void(DatabaseResult& result)>;
			using TransactionCallback = std::function<void(bool transactionSucceeded, std::vector<DatabaseResult>& queryResults)>;

			inline Database(std::string name, std::string dbHost, Nz::UInt16 port, std::string dbUser, std::string dbPassword, std::string dbName);
			~Database() = default;

			DatabaseConnection CreateConnection();

			template<typename T> void ExecuteStatement(T statement, StatementCallback callback);
			inline void ExecuteStatement(std::string statement, std::vector<DatabaseValue> parameters, StatementCallback callback);
			inline void ExecuteTransaction(DatabaseTransaction transaction, TransactionCallback callback);

			void Poll();

			void SpawnWorkers(std::size_t workerCount);

			void WaitForCompletion();

		protected:
			template<typename T> DatabaseResult PrepareStatement(DatabaseConnection& connection);
			void PrepareStatement(DatabaseConnection& connection, const std::string& statementName, const std::string& query, std::initializer_list<DatabaseType> parameterTypes);
			virtual void PrepareStatements(DatabaseConnection& connection) = 0;

		private:
			struct QueryRequest
			{
				std::string statement;
				std::vector<DatabaseValue> parameters;
				StatementCallback callback;
			};

			struct TransactionRequest
			{
				DatabaseTransaction transaction;
				TransactionCallback callback;
			};

			using Request = std::variant<QueryRequest, TransactionRequest>;

			struct QueryResult
			{
				StatementCallback callback;
				DatabaseResult result;
			};

			struct TransactionResult
			{
				TransactionCallback callback;
				std::vector<DatabaseResult> results;
				bool transactionSucceeded = false;
			};

			using Result = std::variant<QueryResult, TransactionResult>;

			using RequestQueue = moodycamel::BlockingConcurrentQueue<Request>;
			using ResultQueue = moodycamel::BlockingConcurrentQueue<Result>;

			inline RequestQueue& GetRequestQueue();
			inline void HandleResult(Result& result);
			inline void SubmitResult(Result&& result);

			RequestQueue m_requestQueue;
			ResultQueue m_resultQueue;
			std::string m_name;
			std::string m_dbHostname;
			std::string m_dbPassword;
			std::string m_dbName;
			std::string m_dbUsername;
			std::vector<std::unique_ptr<DatabaseWorker>> m_workers;
			Nz::UInt16 m_dbPort;
	};
}

#include <Server/Database/Database.inl>

#endif // EREWHON_SERVER_DATABASE_HPP
