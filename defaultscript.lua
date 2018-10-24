-- Ici Lynix
Spaceship.TimeSinceStart = 0
Spaceship.CurrentTarget = nil
Spaceship.FriendList = {}
Spaceship.NextBroadcast = 0

local password = "MieuxQueStarCitizen"

function Spaceship:OnStart()
	for k,moduleInfo in pairs(self.Core:GetModules()) do
		local moduleType = moduleInfo:GetType()
		for name,index in pairs(ModuleType) do
			if (moduleType == index) then
				self[name] = moduleInfo
				break
			end
		end
	end

	assert(self.Communications)
	assert(self.Engine)
	assert(self.Navigation)
	assert(self.Radar)
	assert(self.Weapon)
end

function Spaceship:OnCommunicationReceivedMessages(messages)
	for k,message in pairs(messages) do
		local friendSignature = message.data:match("^" .. password .. " (-?%d+)$")
		if (friendSignature) then
			friendSignature = tonumber(friendSignature)
			self.FriendList[friendSignature] = true

			if (self.CurrentTarget == friendSignature) then
				self:SetTarget(nil)
				self:FindNewTarget()
			end
		end
	end
end

function Spaceship:OnTick(elapsedTime)
	--print("OnTick(" .. elapsedTime .. ")")
	self.TimeSinceStart = self.TimeSinceStart + elapsedTime

	if (self.TimeSinceStart > self.NextBroadcast) then
		self.Communications:BroadcastSphere(500.0, password .. " " .. tostring(self.Core:GetSignature()))
		self.NextBroadcast = self.TimeSinceStart + 3
	end

	if (not self.isFleeing) then
		if (self.CurrentTarget) then
			local targetInfo = self.Radar:GetTargetInfo(self.CurrentTarget)
			if (targetInfo) then
				local forwardVec = self.Core:GetRotation() * Vec3.Forward
				if (forwardVec:DotProduct(targetInfo.direction) > 0.99) then
					self.Weapon:Shoot()
				end
			else
				self:SetTarget(nil)
				self:FindNewTarget()
			end
		else
			self:FindNewTarget()
		end
	end
end

function Spaceship:OnNavigationDestinationReached()
	--print("OnNavigationDestinationReached")
	if (self.CurrentTarget) then
		if (self.isFleeing) then
			self.isFleeing = false

			if (self.CurrentTarget) then
				self.Navigation:FollowTarget(self.CurrentTarget, 60)
			end
		else
			local currentTargetInfo = self.Radar:GetTargetInfo(self.CurrentTarget)
			if (currentTargetInfo) then
				local rotation = self.Core:GetRotation()
				local fleeingPos = self.Core:GetPosition() + rotation * Vec3.Forward * 200 + rotation * Vec3.Up * 50

				self.Engine:Impulse(Vec3.Up, 2)
				self.isFleeing = true

				self.Navigation:MoveToPosition(fleeingPos, 20)
			end
		end
	end
end

function Spaceship:OnRadarNewObjectInRange(objectSignature, objectEmSignature, objectSize, direction, distance)
	--print("OnRadarNewObjectInRange(" .. objectSignature .. ", " .. objectSize .. ", " .. tostring(direction) .. ", " .. distance .. ")")
	if (objectSignature ~= 0 and not self.FriendList[objectSignature] and objectEmSignature < 100.0) then
		if (self.CurrentTarget) then
			local targetInfo = self.Radar:GetTargetInfo(self.CurrentTarget)
			if (not targetInfo or targetInfo.distance > distance) then
				self:SetTarget(objectSignature)
			end
		else
			self:SetTarget(objectSignature)
		end
	end
end

function Spaceship:SetTarget(signature)
	--print("SetTarget(" .. tostring(signature) .. ")")
	self.CurrentTarget = signature
	self.isFleeing = false

	if (signature) then
		self.Navigation:FollowTarget(signature, 60)
	end
end

function Spaceship:FindNewTarget()
	--print("FindNewTarget")
	local closestTarget = nil
	local closestTargetDist = math.huge
	local ourPos = self.Core:GetPosition()
	for _,info in pairs(self.Radar:Scan()) do
		if (info.emSignature < 100 and info.distance < closestTargetDist and not self.FriendList[info.signature]) then
			closestTarget = info.signature
			closestTargetDist = info.distance
		end
	end

	if (closestTarget) then
		self:SetTarget(closestTarget)
		return true
	else
		return false -- No target in range
	end
end
