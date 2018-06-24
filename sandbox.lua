function PrintTable( t, indent, done )
	done = done or {}
	indent = indent or 0

	for k,v in pairs(t) do
		if (type(v) == "table" and not done[v]) then
			done[v] = true

			print(string.rep("  ", indent) .. tostring(k) .. ": {")
			PrintTable(v, indent + 1, done)
			print(string.rep("  ", indent) .. "}")
		else
			print(string.rep("  ", indent) .. tostring(k) .. ": " .. tostring(v))
		end
	end
end

function table.Count(t)
	local count = 0
	for k,v in pairs(t) do
		count = count + 1
	end

	return count
end

local players = {}
local timeSinceStart = 0
local spawnDelay = 5

function OnPlayerJoined(player)
	Arena:PrintChatMessage(string.format("Player %s has joined", player:GetName()));

	players[player:GetSessionId()] = {
		RespawnTime = 0,
		Player = player
	}
end

function OnPlayerLeave(player)
	Arena:PrintChatMessage(string.format("Player %s left", player:GetName()));

	local playerData = players[player:GetSessionId()]
	if (playerData.Spaceship and playerData.Spaceship:IsValidHandle()) then
		playerData.Spaceship:Kill()
	end

	players[player:GetSessionId()] = nil
end

function OnPlayerDeath(player)
	print("Player " .. player:GetName() .. " died")
	players[player:GetSessionId()].RespawnTime = timeSinceStart + spawnDelay
	player:PrintMessage("Respawning in 5 seconds...")
end

function OnReset()
	print("On reset (Sandbox), " .. Arena:GetName())

	Ball = Arena:CreateEntity("ball", "The (big) ball created from script", nil, Vector3(0, 60, 0), Quaternion.Identity)
	Earth = Arena:CreateEntity("earth", "The (small) Earth created from script", nil, Vector3(0, 0, -60), Quaternion.Identity)
	Light = Arena:CreateEntity("light", "", nil, Vector3(0, 0, 0), Quaternion.Identity)
end

function OnUpdate(elapsedTime)
	timeSinceStart = timeSinceStart + elapsedTime

	for sessionId, playerData in pairs(players) do
		if (playerData.RespawnTime < timeSinceStart and not playerData.Player:GetControlledEntity():IsValidHandle()) then
			playerData.Spaceship = Arena:CreateSpaceship(playerData.Player:GetName(), playerData.Player, 1, Vector3(0, 0, 0), Quaternion.Identity)
			playerData.Player:UpdateControlledEntity(playerData.Spaceship)
		end
	end
end