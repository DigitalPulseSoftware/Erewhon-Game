dofile("spacelib.lua")

local MouseButtonPressed = {}
local KeyPressed = {}
function OnLostFocus()
	KeyPressed = {}
	MouseButtonPressed = {}
end

function OnKeyPressed(event)
	if (event.key == "Space") then
		Shoot()
	else
		KeyPressed[event.key] = true
	end
end

function OnKeyReleased(event)
	KeyPressed[event.key] = false
end

local IsRotationEnabled = false
local RotationCursorPosition = Vec2.New(0, 0)
function OnMouseButtonPressed(event)
	MouseButtonPressed[event.button] = true
	if (event.button == "Right") then
		ShowRotationCursor(true)
		IsRotationEnabled = true
		RotationCursorPosition = Vec2.New(0, 0)
	end
end

function OnMouseButtonReleased(event)
	MouseButtonPressed[event.button] = false
	if (event.button == "Right") then
		ShowRotationCursor(false)
		IsRotationEnabled = false
		RotationCursorPosition = Vec2.New(0, 0)
	end
end

local DistMax = 200

function OnMouseMoved(event)
	if (not IsRotationEnabled) then
		return
	end

	RotationCursorPosition.x = RotationCursorPosition.x + event.deltaX
	RotationCursorPosition.y = RotationCursorPosition.y + event.deltaY
	
	if (RotationCursorPosition:SquaredLength() > DistMax * DistMax) then
		RotationCursorPosition:Normalize()
		RotationCursorPosition = RotationCursorPosition * DistMax
	end

	local cursorAngle = math.deg(math.atan(RotationCursorPosition.y, RotationCursorPosition.x))
	local cursorAlpha = RotationCursorPosition:SquaredLength() / (DistMax * DistMax)
	UpdateRotationCursor(RotationCursorPosition, cursorAngle, cursorAlpha)

	RecenterMouse()
end

local Acceleration = 1.0  -- 100%
local StrafeSpeed = 0.66 -- 66%
local JumpSpeed = 0.66 -- 66%
local RollSpeed = 0.66 -- 66%
local RotationSpeedPerPixel = 0.002  -- 0.2%

function UpdateInput(elapsedTime)
	if (KeyPressed["G"] or (MouseButtonPressed["Left"] and IsRotationEnabled)) then
		Shoot()
	end
	
	local SpaceshipMovement = Vec3.New()
	local SpaceshipRotation = Vec3.New()

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
	if (KeyPressed["LShift"]) then
		SpaceshipMovement.z = SpaceshipMovement.z + JumpSpeed
	end

	if (KeyPressed["LControl"]) then
		SpaceshipMovement.z = SpaceshipMovement.z - JumpSpeed
	end

	local rollSpeedModifier = 0.0
	if (KeyPressed["A"]) then
		SpaceshipRotation.z = SpaceshipRotation.z + RollSpeed
	end

	if (KeyPressed["E"]) then
		SpaceshipRotation.z = SpaceshipRotation.z - RollSpeed
	end

	local rotationDirection = RotationCursorPosition

	--SpaceshipMovement.x = Clamp(SpaceshipMovement.x + forwardSpeedModifier, -20.0, 20.0);
	--SpaceshipMovement.y = Clamp(SpaceshipMovement.y + leftSpeedModifier, -15.0, 15.0);
	--SpaceshipMovement.z = Clamp(SpaceshipMovement.z + jumpSpeedModifier, -15.0, 15.0);
	--SpaceshipRotation.z = Clamp(SpaceshipRotation.z + rollSpeedModifier, -100.0, 100.0);

	SpaceshipRotation.x = Clamp(-rotationDirection.y * RotationSpeedPerPixel, -1.0, 1.0)
	SpaceshipRotation.y = Clamp(-rotationDirection.x * RotationSpeedPerPixel, -1.0, 1.0)

	--SpaceshipRotation.x = Approach(SpaceshipRotation.x, 0.0, 200.0 * elapsedTime);
	--SpaceshipRotation.y = Approach(SpaceshipRotation.y, 0.0, 200.0 * elapsedTime);

	return SpaceshipMovement, SpaceshipRotation
end

function Init()
	--UpdateCamera(Vec3.Backward * 12.0 + Vec3.Up * 5, Vec3.New(-10.0, 0.0, 0.0))
end

function OnUpdate(pos, rot)
	local position = Vec3.New(pos.x, pos.y, pos.z)
	local rotation = Quaternion.New(rot.w, rot.x, rot.y, rot.z)
		
	UpdateCamera(position + rotation * (Vec3.Backward * 12.0 + Vec3.Up * 5), rotation * Quaternion.FromEulerAngles(-10, 0.0, 0.0))
end