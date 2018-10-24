AssetsFolder = "Assets/"

Database = {
	Host = "localhost",
	Port = 5432,
	Name = "erewhon",
	Username = "erewhon",
	Password = "erewhon",
	WorkerCount = 2
}

Game = {
	MaxClients  = 100,
	Port        = 2050,
	WorkerCount = 2
}

DefaultSpaceship = {
	Name = "default",
	Hull = "Default hull",
	Modules = "Basic radar module|Basic navigation module|Plasma beam weapon module|Basic engine module|Basic communication array",
	ScriptFile = "defaultscript.lua"
}

-- Warning: changing these parameters will break login to already registered accounts
Security = {
	Argon2 = {
		IterationCost = 10,
		MemoryCost    = 4 * 1024,
		ThreadCost    = 1
	},
	HashLength   = 32,
	PasswordSalt = "<random and unique salt>",
}
