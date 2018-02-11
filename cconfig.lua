Server = {
	Address = "localhost",
	Port    = 2050
}

-- Warning: changing these parameters will break login to already registered accounts
Security = {
	Argon2 = {
		IterationCost = 10,
		MemoryCost    = 64 * 1024,
		ThreadCost    = 1
	},
	HashLength          = 64,
	PasswordSalt        = "4349d12c0dddb5bcd4346cb014d5f98a"
}

ClientScript = {
	Filename = "spaceshipcontroller.lua"
}

ServerScript = {
	Filename = "botscript.lua"
}
