WorkspaceName = "Erewhon"
Projects = {
	{
		Name = "argon2",
		Kind = "StaticLib",
		Defines = {},
		Files = {"../thirdparty/include/argon2/**", "../thirdparty/src/argon2/**"},
		Includes = {"../thirdparty/include/argon2"},
		Libs = {},
		LibsDebug = {},
		LibsRelease = {},
		AdditionalDependencies = {}
	},
	{
		Name = "ErewhonClient",
		Kind = "ConsoleApp",
		Defines = {},
		Files = {"../include/Shared/**", "../src/Shared/**", "../src/Client/**"},
		Includes = {"../thirdparty/include"},
		Libs = os.istarget("windows") and {} or {"pthread"},
		LibsDebug = {"enet", "ws2_32", "Winmm", "argon2-d", "NazaraAudio-d", "NazaraCore-d", "NazaraLua-d", "NazaraGraphics-d", "NazaraNetwork-d", "NazaraNoise-d", "NazaraRenderer-d", "NazaraPhysics2D-d", "NazaraPhysics3D-d", "NazaraPlatform-d", "NazaraSDK-d", "NazaraUtility-d"},
		LibsRelease = {"enet", "ws2_32", "Winmm", "argon2", "NazaraAudio", "NazaraCore", "NazaraLua", "NazaraGraphics", "NazaraNetwork", "NazaraNoise", "NazaraRenderer", "NazaraPhysics2D", "NazaraPhysics3D", "NazaraPlatform", "NazaraSDK", "NazaraUtility"},
		AdditionalDependencies = {"Newton", "libsndfile-1", "soft_oal"}
	},
	{
		Name = "ErewhonServer",
		Kind = "ConsoleApp",
		Defines = {"NDK_SERVER"},
		Files = {"../include/Shared/**", "../src/Shared/**", "../src/Server/**"},
		Includes = {"../thirdparty/include"},
		Libs = os.istarget("windows") and {"libpq"} or {"pq", "pthread"},
		LibsDebug = {"enet", "ws2_32", "Winmm", "argon2-d", "NazaraCore-d", "NazaraLua-d", "NazaraNetwork-d", "NazaraNoise-d", "NazaraPhysics2D-d", "NazaraPhysics3D-d", "NazaraSDKServer-d", "NazaraUtility-d"},
		LibsRelease = {"enet", "ws2_32", "Winmm", "argon2", "NazaraCore", "NazaraLua", "NazaraNetwork", "NazaraNoise", "NazaraPhysics2D", "NazaraPhysics3D", "NazaraSDKServer", "NazaraUtility"},
		AdditionalDependencies = {"libeay32", "libintl-8", "libiconv-2", "Newton", "ssleay32"}
	}
}

-- Do not edit past this line if you don't know what you're doing

-- Load configs
if (not os.isfile("config.lua")) then
	error("config.lua is missing")
end

Config = {}

local configLoader, err = load(io.readfile("config.lua"), "config.lua", "t", Config)
if (not configLoader) then
	error("config.lua failed to load: " .. err)
end

local configLoaded, err = pcall(configLoader)
if (not configLoaded) then
	error("config.lua failed to load: " .. err)
end

local libs = {"NazaraPath", "PostgresClientPath", "ENetTest"}
local libsDirs = {"", "/bin", "/include"}
if (os.istarget("windows")) then
	table.insert(libsDirs, "/lib")
end

for k,v in pairs(libs) do
	local dir = Config[v]
	if (not dir) then
		error("Missing " .. v .. " value in config.lua")
	end

	-- Only check directories if not empty
	if (#dir > 0) then
		for k,v in pairs(libsDirs) do
			local checkPath = dir .. v
			if (not os.isdir(checkPath)) then
				error("\"" .. checkPath .. "\" does not exists")
			end
		end
	end
end

location(_ACTION)

workspace(WorkspaceName)
	configurations { "Debug", "Release" }
	platforms("x64")

	filter "configurations:*32"
		architecture "x86"

	filter "configurations:*64"
		architecture "x86_64"

	for _,data in pairs(Projects) do
		project(data.Name)
			kind(data.Kind)
			language("C++")
			cppdialect("C++17")

			defines(data.Defines)
			
			includedirs(data.Includes)
			includedirs { "../include/", "../src/" }

			flags { "MultiProcessorCompile", "NoMinimalRebuild" }

			for _, path in pairs(data.Files) do
				for _, ext in pairs({".h", ".hpp", ".inl", ".c", ".cpp"}) do
					files(path .. ext)
				end
			end


			debugdir("../bin/%{cfg.buildcfg}")
			targetdir("../bin/%{cfg.buildcfg}")

			filter {"architecture:x86"}
				libdirs("../lib/" .. _ACTION .. "/x86")

			filter {"architecture:x86_64"}
				libdirs("../lib/" .. _ACTION .. "/x64")

			for k,v in pairs(libs) do
				local dir = Config[v]

				if (#dir > 0) then
					filter {}
						includedirs(dir .. "/include")

					filter {"architecture:x86", "system:not Windows", "configurations:Debug"}
						libdirs(dir .. "/bin/debug")
						libdirs(dir .. "/bin/x86/debug")

					filter {"architecture:x86", "system:not Windows"}
						libdirs(dir .. "/bin")
						libdirs(dir .. "/bin/x86")

					filter {"architecture:x86_64", "system:not Windows", "configurations:Debug"}
						libdirs(dir .. "/bin/debug")
						libdirs(dir .. "/bin/x64/debug")

					filter {"architecture:x86_64", "system:not Windows"}
						libdirs(dir .. "/bin")
						libdirs(dir .. "/bin/x64")

					filter {"architecture:x86", "system:Windows", "configurations:Debug"}
						libdirs(dir .. "/lib/debug")
						libdirs(dir .. "/lib/x86/debug")

					filter {"architecture:x86", "system:Windows"}
						libdirs(dir .. "/lib")
						libdirs(dir .. "/lib/x86")

					filter {"architecture:x86_64", "system:Windows", "configurations:Debug"}
						libdirs(dir .. "/lib/debug")
						libdirs(dir .. "/lib/x64/debug")
					
					filter {"architecture:x86_64", "system:Windows"}
						libdirs(dir .. "/lib")
						libdirs(dir .. "/lib/x64")
				end
			end
			
			filter "configurations:Debug"
				defines { "DEBUG" }
				links(data.LibsDebug)
				symbols "On"

			filter "configurations:Release"
				defines { "NDEBUG" }
				links(data.LibsRelease)
				optimize "On"

			filter {"configurations:Debug", "kind:*Lib"}
				targetsuffix("-d")

			filter {"architecture:x86", "kind:*Lib"}
				targetdir("../lib/" .. _ACTION .. "/x86")

			filter {"architecture:x86_64", "kind:*Lib"}
				targetdir("../lib/" .. _ACTION .. "/x64")

			filter "action:vs*"
				defines "_CRT_SECURE_NO_WARNINGS"

			filter {}
				links(data.Libs)

			if (os.ishost("windows")) then
				local commandLine = "premake5.exe " .. table.concat(_ARGV, ' ')

				prebuildcommands("cd .. && " .. commandLine)
				if (data.Kind == "WindowedApp" or data.Kind == "ConsoleApp") then
					postbuildcommands("cd .. && premake5.exe --buildarch=%{cfg.architecture} --buildmode=%{cfg.buildcfg} thirdparty_sync")
				end
			end
	end

	newoption({
		trigger     = "buildarch",
		description = "Set the directory for the thirdparty_update",
		value       = "VALUE",
		allowed = {
			{ "x86", "/x86" },
			{ "x86_64", "/x64" }
		}
	})

	newoption({
		trigger     = "buildmode",
		description = "Set the directory for the thirdparty_update",
		value       = "VALUE",
		allowed = {
			{ "Debug", "/Debug" },
			{ "Release", "/Release" }
		}
	})

	newaction {
		trigger = "thirdparty_sync",
		description = "Update .dll files from thirdparty directory",

		execute = function()
			assert(_OPTIONS["buildarch"])
			assert(_OPTIONS["buildmode"])

			local archToDir = {
				["x86"] = "x86",
				["x86_64"] = "x64"
			}

			local archDir = archToDir[_OPTIONS["buildarch"]]
			assert(archDir)

			local binPaths = {}
			for k,v in pairs(libs) do
				table.insert(binPaths, Config[v] .. "/bin")
				table.insert(binPaths, Config[v] .. "/bin/" .. archDir)
			end

			local updatedCount = 0
			local totalCount = 0

			local libs = {}
			for name,data in pairs(Projects) do
				libs = table.join(libs, data.Libs, data["Libs" .. _OPTIONS["buildmode"]], data.AdditionalDependencies)
			end

			for k,lib in pairs(libs) do
				lib = lib .. ".dll"
				local found = false
				local sourcePath
				for k,v in pairs(binPaths) do
					sourcePath = v .. "/" .. lib
					if (os.isfile(sourcePath)) then
						found = true
						break
					else
						sourcePath = v .. "/" .. path.getdirectory(lib) .. "/lib" .. path.getname(lib)
						if (os.isfile(sourcePath)) then
							lib = "lib" .. lib
							found = true
							break
						end
					end
				end

				if (found) then
					local fileName = path.getname(sourcePath)
					local targetPath = path.normalize(path.translate("../bin/" .. _OPTIONS["buildmode"] .. "/" .. lib), "/")

					local copy = true
					if (os.isfile(targetPath)) then
						local sourceUpdateTime = os.stat(sourcePath).mtime
						local targetUpdateTime = os.stat(targetPath).mtime

						if (targetUpdateTime > sourceUpdateTime) then
							copy = false
						end
					end

					if (copy) then
						print("Copying " .. lib .. "...")

						local ok, err = os.copyfile(sourcePath, targetPath)
						if (not ok) then
							error("Failed to copy " .. targetPath .. ": " .. tostring(err))
						end
						
						-- Copying using os.copyfile doesn't update modified time...
						local ok, err = os.touchfile(targetPath)
						if (not ok) then
							error("Failed to touch " .. targetPath .. ": " .. tostring(err))
						end

						updatedCount = updatedCount + 1
					end

					totalCount = totalCount + 1
				else
					print("Dependency not found: " .. lib)
				end
			end

			print("" .. updatedCount .. "/" .. totalCount .. " files required an update")
		end
	}
