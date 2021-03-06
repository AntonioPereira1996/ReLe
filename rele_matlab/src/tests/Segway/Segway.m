%% Rocky data visualizer
addpath('../Statistics');
addpath('../Toolbox/');

%clear old data
clear

%clear old figures
figure(1)
clf(1)

figure(2)
clf(2)

figure(3)
clf(3)

figure(4)
clf(4)

figure(5)
clf(5)

cmd = '/home/mpirotta/Projects/github/ReLe/rele-build/segway_BBO';
status = system(cmd);

%% Read data

disp('Reading data trajectories...')
csv = csvread('/tmp/ReLe/segway/BBO/seqway_final.log');

disp('Organizing data in episodes...')
episodes = readDataset(csv);
clearvars csv

%% Display Data
close all
disp('Plotting trajectories...')

l = 1.2; %m
r = 0.2; %m

%%
for i=1:size(episodes, 1)
    x = episodes(i).x;
    u = episodes(i).u;
    
    for step = 1:length(x)
        
        theta = x(step,1);
        
        x_c = 0; y_c = 0.2;
        th = 0:pi/50:2*pi;
        xunit = r * cos(th) + x_c;
        yunit = r * sin(th) + y_c;
        
        x_h = l * sin(theta);
        y_h = l * cos(theta);
        
        figure(1);
        plot(xunit, yunit);
        hold on;
        plot([x_c;x_h],[y_c;y_h]);
        axis equal;
        ylim([0, l+0.1]);
        
        hold off;
        pause(0.05)
    end
end

%%

index = 1;
ep = 1;
csv = csvread('/tmp/ReLe/segway/BBO/lqr_nes_diag_agentData.log');
while(index < size(csv, 1))
    [data(ep), index] = ReadNESStatistics(csv, index);
    ep = ep + 1;
end
clear csv;

J_history = [];
J = zeros(length(data),1);
for o = 1:length(data)
    J(o) = mean([data(o).policies.J]);
    J_history = [J_history, [data(o).policies.J]'];
end

shadedErrorBar(1:size(J_history,2), ...
    smooth(mean(J_history)), ...
    smooth(2*sqrt(diag(cov(J_history)))), ...
    {'LineWidth', 2'}, 1);

%% ODE45
x0 = [0.08 0 0];
[t,y] = ode45(@segway_ode, [0, size(x,1)*0.03], x0);
figure(2);
plot (t,y(:,1)*180/pi, 0.03*(1:size(x,1)), x(:,1)*180/pi);
legend('matlab', 'odeint');
xlabel('t'); ylabel('x_1');
figure(3);
plot (t,y(:,2), 0.03*(1:size(x,1)), x(:,2));
xlabel('t'); ylabel('x_2');
figure(4);
plot (t,y(:,3), 0.03*(1:size(x,1)), x(:,3));
xlabel('t'); ylabel('x_3');
figure(5);
title('action u');
plot (0.03*(1:size(x,1)), u);
xlabel('t'); ylabel('u');


