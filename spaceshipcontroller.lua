print("Bonjour Twitch!")

function Vec2(x, y)
	return {
		["x"] = x or 0,
		["y"] = y or 0
	}
end

function Vec3(x, y, z)
	return {
		["x"] = x or 0,
		["y"] = y or 0,
		["z"] = z or 0
	}
end

function Approach(value, objective, increment)
	if (value < objective) then
		return math.min(value + increment, objective)
	elseif (value > objective) then
		return math.max(value - increment, objective)
	else
		return value
	end
end

function Clamp(value, minValue, maxValue)
	return math.max(math.min(value, maxValue), minValue)
end

KeyPressed = {}
function OnLostFocus()
	KeyPressed = {}
end

function OnKeyPressed(event)
	if (event.key == "ESPACE") then
		Shoot()
	else
		KeyPressed[event.key] = true
	end
end

function OnKeyReleased(event)
	KeyPressed[event.key] = false
end

function OnMouseButtonPressed(event)
	if (event.button == "Right") then
		EnableRotation(true)
	end
end

function OnMouseButtonReleased(event)
	if (event.button == "Right") then
		EnableRotation(false)
	end
end

Acceleration = 30.0
StrafeSpeed = 20.0
JumpSpeed = 20.0
RollSpeed = 300.0

function UpdateInput(elapsedTime)
	if (KeyPressed["G"]) then
		Shoot()
	end
	
	local SpaceshipMovement = Vec3()
	local SpaceshipRotation = Vec3()

	if (KeyPressed["Z"]) then
		SpaceshipMovement.x = SpaceshipMovement.x + Acceleration
	end

	if (KeyPressed["S"]) then
		SpaceshipMovement.x = SpaceshipMovement.x - Acceleration
	end

	local leftSpeedModifier = 0.0
	if (KeyPressed["Q"]) then
		SpaceshipMovement.y = SpaceshipMovement.y + StrafeSpeed
	end

	if (KeyPressed["D"]) then
		SpaceshipMovement.y = SpaceshipMovement.y - StrafeSpeed
	end

	local jumpSpeedModifier = 0.0
	if (KeyPressed["MAJ"]) then
		SpaceshipMovement.z = SpaceshipMovement.z + JumpSpeed
	end

	if (KeyPressed["CTRL"]) then
		SpaceshipMovement.z = SpaceshipMovement.z - JumpSpeed
	end

	local rollSpeedModifier = 0.0
	if (KeyPressed["A"]) then
		SpaceshipRotation.z = SpaceshipRotation.z + RollSpeed
	end

	if (KeyPressed["E"]) then
		SpaceshipRotation.z = SpaceshipRotation.z - RollSpeed
	end

	local rotationDirection = GetRotation();

	--SpaceshipMovement.x = Clamp(SpaceshipMovement.x + forwardSpeedModifier, -20.0, 20.0);
	--SpaceshipMovement.y = Clamp(SpaceshipMovement.y + leftSpeedModifier, -15.0, 15.0);
	--SpaceshipMovement.z = Clamp(SpaceshipMovement.z + jumpSpeedModifier, -15.0, 15.0);
	--SpaceshipRotation.z = Clamp(SpaceshipRotation.z + rollSpeedModifier, -100.0, 100.0);
	
	SpaceshipRotation.x = Clamp(-rotationDirection.y / 2.0, -200.0, 200.0);
	SpaceshipRotation.y = Clamp(-rotationDirection.x / 2.0, -200.0, 200.0);

	--SpaceshipRotation.x = Approach(SpaceshipRotation.x, 0.0, 200.0 * elapsedTime);
	--SpaceshipRotation.y = Approach(SpaceshipRotation.y, 0.0, 200.0 * elapsedTime);

	return SpaceshipMovement, SpaceshipRotation
end