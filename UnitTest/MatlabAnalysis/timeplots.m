%% Time plots (approximative)
close all

figure
plotyy(1:length(pressure0256), pressure0256, 1:length(pressure0256), temp0256)

figure
plotyy(1:length(pressure0512), pressure0512, 1:length(pressure0512), temp0512)

figure
plotyy(1:length(pressure1024), pressure1024, 1:length(pressure1024), temp1024)

figure
plotyy(1:length(pressure2048), pressure2048, 1:length(pressure2048), temp2048)

figure
plotyy(1:length(pressure4096), pressure4096, 1:length(pressure4096), temp4096)

figure
plotyy(1:length(pressure8192), pressure8192, 1:length(pressure8192), temp8192)