local gameObject;
local component;

function RPC(methodName, target, ...)

	local paramCount = select("#", ...);
	local param = MinNetPool.PopPacket();
	param:create_packet()

	for i = 1, paramCount do

		param:push(select(i, ...));

	end

	gameObject:sendRPC(methodName, target, param);

end

function SyncPosition (packet)

	--print("SyncPosition");

	local position = packet:pop_vector3 ();
	local euler = packet:pop_vector3 ();
	local cameraEuler = packet:pop_vector3 ();
	
	gameObject.position = position;

end

--[[function SetGameObject (object)
	--print("오브젝트 설정");
	gameObject = object
end]]--

function SetComponent (object)

	component = object;
	gameObject = object.gameObject;

end