-- spawn 1024 static entities with crate models
-- 1024 entities
-- 16*16*4

function G_InitGame()
	local i = 0
	local j = 0
	local k = 0

	for i=0,16,1 do
	  for j=0,16,1 do
		for k=0,4,1 do
			local ent = entity.Spawn("ModelEntity");
			ent:SetRenderModel("models/props/crates/crate1.obj");
			ent:SetOrigin(vector.Construct(i*64, j*64, k*64+32));
		end
	  end
	end
end






















