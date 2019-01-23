set validator=C:\vulkan\glslangValidator

%validator% -V rayhit.rchit -o ray_chit.spv
%validator% -V raygen.rgen -o ray_gen.spv
%validator% -V raymiss.rmiss -o ray_miss.spv

pause