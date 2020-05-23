set validator=..\..\..\Lib\glslangValidator.exe

%validator% -V grass.frag -o grass_frag.spv
%validator% -V grass.vert -o grass_vert.spv

%validator% -V final_pass.frag -o final_pass_frag.spv
%validator% -V final_pass.vert -o final_pass_vert.spv