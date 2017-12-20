function Color(r, g, b, a)
	return {
		["r"] = r,
		["g"] = g,
		["b"] = b,
		["a"] = a or 255
	}
end

function PrintTable( t, indent, done )
	done = done or {}
	indent = indent or 0

	for k,v in pairs(t) do
		if (type(v) == "table") then
			if (not done[v]) then
				done[v] = true

				print(string.rep("  ", indent) .. tostring(k) .. ": ")
				PrintTable(t, indent + 1, done)
			end
		else
			print(string.rep("  ", indent) .. tostring(k) .. ": " .. tostring(v))
		end
	end
end

Vec2 = {}
Vec2.__index = Vec2

function Vec2:__newindex(fieldName, value)
	error("Vec2 has no field " .. fieldName)
end

function Vec2:__add(rhs)
	if (getmetatable(rhs) == Vec2) then
		return Vec2.New(self.x + rhs.x, self.y + rhs.y)
	else
		error("Unknown type")
	end
end

function Vec2:__sub(rhs)
	if (getmetatable(rhs) == Vec2) then
		return Vec2.New(self.x - rhs.x, self.y - rhs.y)
	else
		error("Unknown type")
	end
end

function Vec2.__mul(lhs, rhs)
	if (getmetatable(lhs) == getmetatable(rhs) and getmetatable(lhs) == Vec2) then
		return Vec2.New(lhs.x * rhs.x, lhs.y * rhs.y)
	elseif (type(lhs) == "number" and getmetatable(rhs) == Vec2) then
		return Vec2.New(lhs * rhs.x, lhs * rhs.y)
	elseif (getmetatable(lhs) == Vec2 and type(rhs) == "number") then
		return Vec2.New(lhs.x * rhs, lhs.y * rhs)
	else
		error("Unknown type")
	end
end

function Vec2:__div(rhs)
	if (type(rhs) == "number") then
		return Vec2.New(self.x / rhs, self.y / rhs)
	elseif (getmetatable(rhs) == Vec2) then
		return Vec2.New(self.x / rhs.x, self.y / rhs.y)
	else
		error("Unknown type")
	end
end

function Vec2:__tostring()
	return "Vec2(" .. self.x .. ", " .. self.y .. ")"
end

function Vec2:Normalize()
	local length = self:Length()
	self.x = self.x / length
	self.y = self.y / length
end

function Vec2:Distance(rhs)
	return math.sqrt(self:SquaredDistance(rhs))
end

function Vec2:Length()
	return math.sqrt(self:SquaredLength())
end

function Vec2:SquaredLength()
	return self.x * self.x + self.y * self.y
end

function Vec2:SquaredDistance(rhs)
	assert(getmetatable(rhs) == Vec2)

	local relativeVec = rhs - self
	return relativeVec:SquaredLength()
end

function Vec2.New(x, y)
	local o = {}
	o.x = x or 0
	o.y = y or 0

	setmetatable(o, Vec2)
	return o
end

Vec2.Down = Vec2.New(0, 1, 0)
Vec2.Left = Vec2.New(-1, 0, 0)
Vec2.Right = Vec2.New(1, 0, 0)
Vec2.Up = Vec2.New(0, -1, 0)


Vec3 = {}
Vec3.__index = Vec3

function Vec3:__newindex(fieldName, value)
	error("Vec3 has no field " .. fieldName)
end

function Vec3:__add(rhs)
	if (getmetatable(rhs) == Vec3) then
		return Vec3.New(self.x + rhs.x, self.y + rhs.y, self.z + rhs.z)
	else
		error("Unknown type")
	end
end

function Vec3:__sub(rhs)
	if (getmetatable(rhs) == Vec3) then
		return Vec3.New(self.x - rhs.x, self.y - rhs.y, self.z - rhs.z)
	else
		error("Unknown type")
	end
end

function Vec3.__mul(lhs, rhs)
	if (getmetatable(lhs) == getmetatable(rhs) and getmetatable(lhs) == Vec3) then
		return Vec3.New(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z)
	elseif (type(lhs) == "number" and getmetatable(rhs) == Vec3) then
		return Vec3.New(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z)
	elseif (getmetatable(lhs) == Vec3 and type(rhs) == "number") then
		return Vec3.New(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs)
	else
		error("Unknown type")
	end
end

function Vec3:__div(rhs)
	if (type(rhs) == "number") then
		return Vec3.New(self.x / rhs, self.y / rhs, self.z / rhs)
	elseif (getmetatable(rhs) == Vec3) then
		return Vec3.New(self.x / rhs.x, self.y / rhs.y, self.z / rhs.z)
	else
		error("Unknown type")
	end
end

function Vec3:__tostring()
	return "Vec3(" .. self.x .. ", " .. self.y .. ", " .. self.z .. ")"
end

function Vec3:CrossProduct(vec)
	assert(getmetatable(vec) == Vec3)
	return Vec3.New(self.y * vec.z - self.z * vec.y, self.z * vec.x - self.x * vec.z, self.x * vec.y - self.y * vec.x)
end

function Vec3:Normalize()
	local length = self:Length()
	self.x = self.x / length
	self.y = self.y / length
	self.z = self.z / length
end

function Vec3:Distance(rhs)
	return math.sqrt(self:SquaredDistance(rhs))
end

function Vec3:Length()
	return math.sqrt(self:SquaredLength())
end

function Vec3:SquaredLength()
	return self.x * self.x + self.y * self.y + self.z * self.z
end

function Vec3:SquaredDistance(rhs)
	assert(getmetatable(rhs) == Vec3)

	local relativeVec = rhs - self
	return relativeVec:SquaredLength()
end

function Vec3.New(x, y, z)
	local o = {}
	o.x = x or 0
	o.y = y or 0
	o.z = z or 0

	setmetatable(o, Vec3)
	return o
end

Vec3.Backward = Vec3.New(0, 0, 1)
Vec3.Down = Vec3.New(0, -1, 0)
Vec3.Forward = Vec3.New(0, 0, -1)
Vec3.Left = Vec3.New(-1, 0, 0)
Vec3.Right = Vec3.New(1, 0, 0)
Vec3.Up = Vec3.New(0, 1, 0)


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

Quaternion = {}
Quaternion.__index = Quaternion

function Quaternion:__tostring()
	return "Quaternion(" .. self.w .. " | " .. self.x .. ", " .. self.y .. ", " .. self.z .. ")"
end

function Quaternion.__mul(lhs, rhs)
	if (getmetatable(lhs) == getmetatable(rhs) and getmetatable(lhs) == Quaternion) then
		local quat = Quaternion.New()
		quat.w = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z
		quat.x = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y
		quat.y = lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z
		quat.z = lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x
		
		return quat
	elseif (getmetatable(lhs) == Quaternion and getmetatable(rhs) == Vec3) then
		local quatVec = Vec3.New(lhs.x, lhs.y, lhs.z)
		local uv = quatVec:CrossProduct(rhs)
		local uuv = quatVec:CrossProduct(uv)
		uv = uv * 2.0 * lhs.w;
		uuv = uuv * 2.0

		return rhs + uv + uuv
	else
		error("Unknown type")
	end
end

function Quaternion:Magnitude()
	return math.sqrt(self:SquaredMagnitude())
end

function Quaternion:Normalize()
	local magnitude = self:Magnitude()
	self.x = self.x / magnitude
	self.y = self.y / magnitude
	self.z = self.z / magnitude
	self.w = self.w / magnitude
end

function Quaternion:SquaredMagnitude()
	return self.w * self.w + self.x * self.x + self.y * self.y + self.z * self.z
end

function Quaternion.FromEulerAngles(pitch, yaw, roll)
	pitch = math.rad(pitch) * 0.5
	yaw = math.rad(yaw) * 0.5
	roll = math.rad(roll) * 0.5

	local c1 = math.cos(yaw)
	local c2 = math.cos(roll)
	local c3 = math.cos(pitch)

	local s1 = math.sin(yaw)
	local s2 = math.sin(roll)
	local s3 = math.sin(pitch)

	return Quaternion.New(c1 * c2 * c3 - s1 * s2 * s3,
	                      s1 * s2 * c3 + c1 * c2 * s3,
	                      s1 * c2 * c3 + c1 * s2 * s3,
	                      c1 * s2 * c3 - s1 * c2 * s3)
end

function Quaternion.New(w, x, y, z)
	local o = {}
	o.x = x or 0
	o.y = y or 0
	o.z = z or 0
	o.w = w or 1

	setmetatable(o, Quaternion)
	return o
end

Quaternion.Identity = Quaternion.New(1, 0, 0, 0)

