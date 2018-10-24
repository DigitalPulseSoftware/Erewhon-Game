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

local timeSinceStart = 0
local spawnDelay = 5
local fightingPlayers = {}
local fightInPreparation = false
local fightInProgress = false
local fighterCount = 2
local nextStepTime = 0
local countdownStep = 10
local fightInfo = {}

function OnPlayerJoined(player)
	Arena:PrintChatMessage(string.format("Player %s has joined", player:GetName()))
end

function OnPlayerLeave(player)
	Arena:PrintChatMessage(string.format("Player %s left", player:GetName()));

	fightingPlayers[player:GetSessionId()] = nil

	CheckFightConditions()
end

function OnReset()
	print("On reset, " .. Arena:GetName())

	Ball = Arena:CreateEntity("ball", "The (big) ball created from script", nil, Vector3(0, 60, 0), Quaternion.Identity)
	--Earth = Arena:CreateEntity("earth", "The (small) Earth created from script", nil, Vector3(0, 0, -60), Quaternion.Identity)
	Light = Arena:CreateEntity("light", "", nil, Vector3(0, 0, 0), Quaternion.Identity)
end

function OnUpdate(elapsedTime)
	timeSinceStart = timeSinceStart + elapsedTime

	if (fightInPreparation) then
		if (timeSinceStart > nextStepTime) then
			StartFight()
		else
			local remainingTime = math.ceil(nextStepTime - timeSinceStart)
			if (remainingTime < countdownStep) then
				countdownStep = remainingTime
				if (countdownStep > 0) then
					Arena:PrintChatMessage(string.format("%s second%s...", countdownStep, countdownStep > 1 and "s" or ""))
				end
			end
		end
	elseif (fightInProgress) then
		if (timeSinceStart > nextStepTime) then
			EndFight(true)
		end
	end
	CheckFightConditions()
end

function OnPlayerChat(player, message)
	if (message:sub(1,6) == "/fight") then
		local fleetName = message:sub(8)
		player:GetFleetData(fleetName, function (fleet)
			if (not fleet) then
				player:PrintMessage("Fleet \"" .. fleetName .. "\" not found")
				return
			end

			if (fightInProgress) then
				player:PrintMessage("You cannot register to fight during a fight")
				return
			end

			local sessionId = player:GetSessionId()
			local doesExists = fightingPlayers[sessionId] and true or false
			if (doesExists) then
				player:PrintMessage("Your fighting fleet has been updated to \"" .. fleetName .. "\"")
			else
				player:PrintMessage("Your participation has been registered with fleet \"" .. fleetName .. "\"")
				Arena:PrintChatMessage(player:GetName() .. " is ready to fight!")
			end

			fightingPlayers[player:GetSessionId()] = {FleetData = fleet, Player = player}
			CheckFightConditions()
		end)
		return false
	end

	return true
end

function CheckFightConditions()
	if (fightInPreparation) then
		if (table.Count(fightingPlayers) < fighterCount) then
			fightInPreparation = false

			Arena:PrintChatMessage("Fight cancelled due to fighter disconnecting")
		end
	elseif (fightInProgress) then
		for playerKey,playerData in pairs(fightInfo.Players) do
			local aliveSpaceshipCount = 0
			for spaceshipKey,spaceship in pairs(playerData.Spaceships) do
				if (spaceship:IsValidHandle()) then
					aliveSpaceshipCount = aliveSpaceshipCount + 1
				else
					playerData.Spaceships[spaceshipKey] = nil
				end
			end

			if (aliveSpaceshipCount == 0) then
				Arena:PrintChatMessage("All spaceships belonging to " .. playerData.Name .. " have been destroyed")
				fightInfo.Players[playerKey] = nil

				if (table.Count(fightInfo.Players) == 1) then
					for k,playerData in pairs(fightInfo.Players) do
						Arena:PrintChatMessage(playerData.Name .. " won!")
					end

					EndFight(false)
				end
			end
		end
	else
		if (table.Count(fightingPlayers) >= fighterCount) then
			fightInPreparation = true
			nextStepTime = timeSinceStart + 10
			countdownStep = 10

			Arena:PrintChatMessage("Next fight start in 10 seconds")
		end
	end
end

function StartFight()
	assert(fightInPreparation and not fightInProgress)

	fightInPreparation = false
	fightInProgress = true
	nextStepTime = timeSinceStart + 5 * 60

	local fightingPlayerCount = table.Count(fightingPlayers)
	if (fightingPlayerCount > fighterCount) then
		local allPlayersSessions = {}
		for session,_ in pairs(fightingPlayers) do
			table.insert(allPlayersSessions, session)
		end

		repeat
			local removedPlayerIndex = math.random(1, #allPlayersSessions)
			local removedPlayer = allPlayersSessions[removedPlayerIndex]
			fightingPlayers[removedPlayer] = nil
			table.remove(allPlayersSessions, removedPlayerIndex)

			fightingPlayerCount = fightingPlayerCount - 1

		until fightingPlayerCount == fighterCount
	end

	local fighterNames = {}
	for k,fightData in pairs(fightingPlayers) do
		table.insert(fighterNames, fightData.Player:GetName())
	end
	Arena:PrintChatMessage(table.concat(fighterNames, ", ") .. " are fighting")

	local sepAngle = 2 * math.pi / fighterCount
	local circleRadius = fighterCount * 100

	fightInfo = {}
	fightInfo.Players = {}

	local index = 0
	for k,fightData in pairs(fightingPlayers) do
		local angle = index * sepAngle
		local startPos = Vector3(circleRadius * math.cos(angle), circleRadius * math.sin(angle), 0)

		local playerData = {}
		playerData.Name = fightData.Player:GetName()
		playerData.Spaceships = {}

		local fleetData = fightData.FleetData
		for k,spaceshipData in pairs(fleetData.spaceships) do
			local spaceshipType = fleetData.spaceshipTypes[spaceshipData.spaceshipType]

			local position = Vector3(startPos.x + spaceshipData.position.x, startPos.y + spaceshipData.position.y, startPos.z + spaceshipData.position.z)
			local spaceship = Arena:SpawnSpaceship(fightData.Player, spaceshipType.script, spaceshipType.hullId, spaceshipType.modules, position, Quaternion.Identity)
			table.insert(playerData.Spaceships, spaceship)
		end

		table.insert(fightInfo.Players, playerData)

		index = index + 1
	end
end

function EndFight(dueToTimer)
	Arena:PrintChatMessage(dueToTimer and "Time's up, fight is over" or "Fight is over")
	fightInProgress = false
	fightingPlayers = {}
	Arena:Reset()
end