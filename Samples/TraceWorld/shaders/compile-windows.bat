set validator=C:\vulkan\glslangValidator

%validator% -V raygen.rgen -o ray_gen.spv
%validator% -V rayhit.rchit -o ray_chit.spv
%validator% -V raymiss.rmiss -o ray_miss.spv

%validator% -V shadow.rchit -o shadow_chit.spv
%validator% -V shadow.rmiss -o shadow_miss.spv

pause