game.Print("Hello lua world, from ",_VERSION,"!\n")



local i = 0
local j = 0

for i=0,10,1 do
  for j=0,10,1 do
    game.Print("i is ",i, " and j is ",j);
    local ent = entity.Spawn("ModelEntity");
    ent:SetRenderModel("models/props/crates/crate1.obj");
    ent:SetOrigin(vector.Construct(i*32, j*32, 16));
    game.Print(ent)
  end
end

