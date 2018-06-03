local players = {}
local timeSinceStart = 0
local spawnDelay = 5

function OnPlayerJoined(player)
	Arena:DispatchChatMessage(string.format("Player %s has joined", player:GetName()));

	players[player:GetSessionId()] = {
		RespawnTime = 0,
		Player = player
	}
end

function OnPlayerLeave(player)
	Arena:DispatchChatMessage(string.format("Player %s left", player:GetName()));

	players[player:GetSessionId()] = nil
end

function OnPlayerDeath(player)
	print("Player " .. player:GetName() .. " died")
	players[player:GetSessionId()].RespawnTime = timeSinceStart + spawnDelay
end

function OnReset()
	print("On reset, " .. Arena:GetName())

	elapsedTime = 0

	Ball = Arena:CreateEntity("ball", "The (big) ball created from script", nil, Vector3(0, 60, 0), Quaternion.Identity)
	Earth = Arena:CreateEntity("earth", "The (small) Earth created from script", nil, Vector3(0, 0, -60), Quaternion.Identity)
	Light = Arena:CreateEntity("light", "", nil, Vector3(0, 0, 0), Quaternion.Identity)
end

function OnUpdate(elapsedTime)
	timeSinceStart = timeSinceStart + elapsedTime

	for sessionId, playerData in pairs(players) do
		if (playerData.RespawnTime < timeSinceStart and not playerData.Player:GetControlledEntity():IsValidHandle()) then
			playerData.Player:UpdateControlledEntity(Arena:CreatePlayerSpaceship(playerData.Player))
		end
	end
end
