%% Time plots (approximative)
close all

figure
[hAx] = plotyy(1:length(pressure0256), pressure0256/10, 1:length(pressure0256), temp0256/100);
title('Resolution 256')
xlabel('Data #')
ylabel(hAx(1),'Pressure (mbar)')
ylabel(hAx(2),'Temperature (C°)')


figure
[hAx] = plotyy(1:length(pressure0512), pressure0512/10, 1:length(pressure0512), temp0512/100);
title('Resolution 512')
xlabel('Data #')
ylabel(hAx(1),'Pressure (mbar)')
ylabel(hAx(2),'Temperature (C°)')

figure
[hAx] = plotyy(1:length(pressure1024), pressure1024/10, 1:length(pressure1024), temp1024/100);
title('Resolution 1024')
xlabel('Data #')
ylabel(hAx(1),'Pressure (mbar)')
ylabel(hAx(2),'Temperature (C°)')

figure
[hAx] = plotyy(1:length(pressure2048), pressure2048/10, 1:length(pressure2048), temp2048/100);
title('Resolution 2048')
xlabel('Data #')
ylabel(hAx(1),'Pressure (mbar)')
ylabel(hAx(2),'Temperature (C°)')

figure
[hAx] = plotyy(1:length(pressure4096), pressure4096/10, 1:length(pressure4096), temp4096/100);
title('Resolution 4096')
xlabel('Data #')
ylabel(hAx(1),'Pressure (mbar)')
ylabel(hAx(2),'Temperature (C°)')

figure
[hAx] = plotyy(1:length(pressure8192), pressure8192/10, 1:length(pressure8192), temp8192/100);
title('Resolution 8192')
xlabel('Data #')
ylabel(hAx(1),'Pressure (mbar)')
ylabel(hAx(2),'Temperature (C°)')