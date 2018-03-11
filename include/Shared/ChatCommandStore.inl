// Copyright (C) 2017 Jérôme Leclercq
// This file is part of the "Erewhon Server" project
// For conditions of distribution and use, see copyright notice in LICENSE

#include <Shared/ChatCommandStore.hpp>
#include <Nazara/Core/Algorithm.hpp>
#include <cassert>
#include <tuple>
#include <type_traits>

namespace ewn
{
	template<typename Client, typename T>
	bool ChatCommandProcessArg(Client* client, std::string_view& cmdArgs, T* arg, const T& defArg, Nz::TypeTag<T>)
	{
		if (cmdArgs.empty())
		{
			*arg = defArg;
			return true;
		}

		return ChatCommandProcessArg(client, cmdArgs, arg, Nz::TypeTag<T>());
	}

	template<bool HasDefault>
	struct ChatCommandImplArgProcessor;

	template<>
	struct ChatCommandImplArgProcessor<true>
	{
		template<std::size_t N, std::size_t FirstDefArg, typename Client, typename ArgType, typename ArgContainer, typename DefArgContainer>
		static bool Process(Client* client, std::string_view& cmdArgs, ArgContainer& args, const DefArgContainer& defArgs)
		{
			return ChatCommandProcessArg(client, cmdArgs, &std::get<N>(args), std::get<FirstDefArg + std::tuple_size<DefArgContainer>() - N - 1>(defArgs), Nz::TypeTag<std::decay_t<ArgType>>());
		}
	};

	template<>
	struct ChatCommandImplArgProcessor<false>
	{
		template<std::size_t N, std::size_t FirstDefArg, typename Client, typename ArgType, typename ArgContainer, typename DefArgContainer>
		static bool Process(Client* client, std::string_view& cmdArgs, ArgContainer& args, const DefArgContainer&)
		{
			return ChatCommandProcessArg(client, cmdArgs, &std::get<N>(args), Nz::TypeTag<ArgType>());
		}
	};

	template<typename Client, typename... Args>
	class ChatCommandImplCommandProxy
	{
		public:
			template<typename... DefArgs>
			class Impl
			{
				static constexpr std::size_t ArgCount = sizeof...(Args);
				static constexpr std::size_t DefArgCount = sizeof...(DefArgs);

				static_assert(ArgCount >= DefArgCount, "There cannot be more default arguments than argument");

				static constexpr std::size_t FirstDefArg = ArgCount - DefArgCount;

			public:
				Impl(DefArgs&&... defArgs) :
				m_defaultArgs(std::forward<DefArgs>(defArgs)...)
				{
				}

				bool ProcessArguments(Client* client, std::string_view cmd) const
				{
					return ProcessArgs<0, Args...>(client, cmd);
				}

				template<typename F>
				bool Invoke(F&& func) const
				{
					return std::apply(func, m_args);
				}

			private:
				using ArgContainer = std::tuple<std::decay_t<Args>...>;
				using DefArgContainer = std::tuple<std::decay_t<DefArgs>...>;

				template<std::size_t N>
				bool ProcessArgs(Client* /*client*/, std::string_view& /*cmd*/) const
				{
					// No argument to process
					return true;
				}

				template<std::size_t N, typename ArgType>
				bool ProcessArgs(Client* client, std::string_view& cmd) const
				{
					return ChatCommandImplArgProcessor<(N >= FirstDefArg)>::template Process<N, FirstDefArg, Client, ArgType>(client, cmd, m_args, m_defaultArgs);
				}

				template<std::size_t N, typename ArgType1, typename ArgType2, typename... Rest>
				bool ProcessArgs(Client* client, std::string_view& cmd) const
				{
					if (!ProcessArgs<N, ArgType1>(client, cmd))
						return false;

					return ProcessArgs<N + 1, ArgType2, Rest...>(client, cmd);
				}

				mutable ArgContainer m_args;
				DefArgContainer m_defaultArgs;
			};
	};

	template<typename Application, typename Client>
	ChatCommandStore<Application, Client>::ChatCommandStore(Application* app) :
	m_app(app)
	{
	}

	template<typename Application, typename Client>
	std::optional<bool> ChatCommandStore<Application, Client>::ExecuteCommand(Client* client, std::string_view cmd)
	{
		std::string_view commandName;
		std::string_view commandArguments;
		if (std::size_t firstSpace = cmd.find_first_of(' '); firstSpace != cmd.npos)
		{
			commandName = cmd.substr(0, firstSpace);
			commandArguments = cmd.substr(firstSpace + 1);
		}
		else
			commandName = cmd;

		auto it = m_commands.find(commandName);
		if (it != m_commands.end())
			return it->second(m_app, client, commandArguments);
		else
			return {};
	}

	template<typename Application, typename Client>
	template<typename... Args, typename... DefArgs>
	void ChatCommandStore<Application, Client>::RegisterCommand(std::string name, bool(*command)(Application* app, Client* client, Args...), DefArgs&&... defArgs)
	{
		typename ChatCommandImplCommandProxy<Client, Args...>::template Impl<DefArgs...> handler(std::forward<DefArgs>(defArgs)...);

		RegisterCommand(std::move(name), [handler, command](Application* app, Client* client, const std::string_view& cmd) -> bool
		{
			if (!handler.ProcessArguments(client, cmd))
				return false;

			return handler.Invoke([&](Args... args) -> bool
			{
				return command(app, client, std::forward<Args>(args)...);
			});
		});
	}

	template<typename Application, typename Client>
	void ChatCommandStore<Application, Client>::UnregisterCommand(const std::string& name)
	{
		m_commands.erase(name);
	}

	template<typename Application, typename Client>
	void ChatCommandStore<Application, Client>::RegisterCommand(std::string name, Command cmd)
	{
		m_commands.emplace(std::move(name), std::move(cmd));
	}

	template<typename Client>
	bool ChatCommandProcessArg(Client* /*client*/, std::string_view& cmdArgs, bool* arg, Nz::TypeTag<bool>)
	{
		if (cmdArgs.empty())
			return false;

		if (cmdArgs == "1" || cmdArgs == "true")
		{
			*arg = true;
			return true;
		}
		else if (cmdArgs == "0" || cmdArgs == "false")
		{
			*arg = false;
			return true;
		}
		else
			return false;
	}

	template<typename Client, typename T>
	std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T>, bool> ChatCommandProcessArg(Client* /*client*/, std::string_view& cmdArgs, T* arg, Nz::TypeTag<T>)
	{
		if (cmdArgs.empty())
			return false;

		char* endPtr;
		long long value = std::strtoll(cmdArgs.data(), &endPtr, 0);
		if (endPtr == cmdArgs.data())
			return false;

		constexpr long long minBounds = std::numeric_limits<T>::min();
		constexpr long long maxBounds = std::numeric_limits<T>::max();
		if (value < minBounds || value > maxBounds)
			return false;

		assert(endPtr <= cmdArgs.data() + cmdArgs.size());
		cmdArgs.remove_prefix(endPtr - cmdArgs.data());

		*arg = static_cast<T>(value);
		return true;
	}

	template<typename Client, typename T>
	std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, bool> ChatCommandProcessArg(Client* /*client*/, std::string_view& cmdArgs, T* arg, Nz::TypeTag<T>)
	{
		if (cmdArgs.empty())
			return false;

		char* endPtr;
		unsigned long long value = std::strtoull(cmdArgs.data(), &endPtr, 0);
		if (endPtr == cmdArgs.data())
			return false;

		constexpr unsigned long long minBounds = std::numeric_limits<T>::min();
		constexpr unsigned long long maxBounds = std::numeric_limits<T>::max();
		if (value < minBounds || value > maxBounds)
			return false;

		assert(endPtr <= cmdArgs.data() + cmdArgs.size());
		cmdArgs.remove_prefix(endPtr - cmdArgs.data());

		*arg = static_cast<T>(value);
		return true;
	}

	template<typename Client, typename T>
	std::enable_if_t<std::is_floating_point_v<T>, bool> ChatCommandProcessArg(Client* /*client*/, std::string_view& cmdArgs, T* arg, Nz::TypeTag<T>)
	{
		if (cmdArgs.empty())
			return false;

		char* endPtr;
		T value;
		if constexpr (std::is_same_v<T, float>)
			value = std::strtof(cmdArgs.data(), &endPtr);
		else if constexpr (std::is_same_v<T, double>)
			value = std::strtod(cmdArgs.data(), &endPtr);
		else if constexpr (std::is_same_v<T, long double>)
			value = std::strtold(cmdArgs.data(), &endPtr);

		if (endPtr == cmdArgs.data())
			return false;

		assert(endPtr <= cmdArgs.data() + cmdArgs.size());
		cmdArgs.remove_prefix(endPtr - cmdArgs.data());

		*arg = static_cast<T>(value);
		return true;
	}

	template<typename Client>
	bool ChatCommandProcessArg(Client* /*client*/, std::string_view& cmdArgs, std::string* arg, Nz::TypeTag<std::string>)
	{
		if (cmdArgs.empty())
			return false;

		std::size_t startPos = 0;
		std::size_t endPos;
		std::size_t processedCharacters;

		bool found = false;
		if (cmdArgs[0] == '"')
		{
			startPos++;
			std::size_t closingQuotes = 0;
			while (closingQuotes = cmdArgs.find('"', closingQuotes + 1))
			{
				if (closingQuotes == cmdArgs.npos)
					break;

				if (cmdArgs[closingQuotes - 1] == '\\')
					continue;

				endPos = closingQuotes - 1;
				processedCharacters = closingQuotes + 1; //< Skip closing quote
				found = true;
				break;
			}
		}

		if (!found)
		{
			endPos = cmdArgs.find(' ');
			processedCharacters = endPos;
		}

		if (processedCharacters != std::numeric_limits<std::size_t>::max())
			++processedCharacters; //< Skip space

		*arg = cmdArgs.substr(startPos, endPos);
		cmdArgs.remove_prefix(std::min(processedCharacters, cmdArgs.size()));

		if (arg->empty())
			return false;

		std::size_t start_pos = 0;
		while ((start_pos = arg->find(R"(\")", start_pos)) != std::string::npos)
		{
			arg->replace(start_pos, 2, "\"");
			start_pos += 1;
		}

		return true;
	}
}
