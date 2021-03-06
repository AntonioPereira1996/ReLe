%% Unicycle data visualizer
clc; clear all;
addpath('../Statistics');

%% Read data

disp('Reading data trajectories...')
csv = csvread('/tmp/ReLe/ucp/BBO/ucp_final.log');
% csv = csvread('/tmp/ReLe/mc/GIRL/mletraining.log');

disp('Organizing data in episodes...')
episodes = readDataset(csv);
clearvars csv

%% Draw data
subplot(5,1,1);
plot(episodes.x(:,1));
ylabel('rho');
subplot(5,1,2);
plot(episodes.x(:,2));
ylabel('gamma');
subplot(5,1,3);
plot(episodes.x(:,3));
ylabel('delta');
subplot(5,1,4);
plot(episodes.u(1:end-1,1));
ylabel('linear vel');
subplot(5,1,5);
plot(episodes.u(1:end-1,2));
ylabel('angular vel');

%%
x = 4;
y = 4;
th = pi;

Tr = [cos(0), sin(0), 0;
    -sin(0), cos(0), 0;
    0, 0, 1];
e = Tr *[x;y;th];

s0 = ones(3,1);
s0(1) = sqrt(e(1)^2+e(2)^2);
s0(2) = atan2(e(2),e(1)) - e(3) + pi;
s0(3) = s0(2) + e(3);

s0(2) = wrapToPi(s0(2));
s0(3) = wrapToPi(s0(3));

k = [0.2465   0.4047   0.0169]';

s = s0;
y = s';
t = 0;
A = [];
for i = 1:300
    % optimal policy
    rho = s(1);
    gamma = wrapToPi(s(2));
    delta = wrapToPi(s(3));
    v = k(1) * rho * cos(gamma);
    w = k(2) * gamma + k(1) * sin(gamma) * cos(gamma) * (gamma + k(3) * delta) / gamma;
    A = [A;v,w];
    [r,x] = ode45(@(t,y) unicycle_ode(t,y,[v;w]), [0,0.03], s);
    s = x(end,:)';
    y = [y;x(end,:)];
    t = [t;i*r(end)];
end
y(:,2) = wrapToPi(y(:,2));
y(:,3) = wrapToPi(y(:,3));

V = k(1) * rho * cos(y(:,2));
W = k(2) * y(:,2) + k(1) .* sin(y(:,2)) .* cos(y(:,2)) .* (y(:,2) + k(3) * y(:,3)) ./ y(:,2);

nb = size(episodes.x(:,1),1);

subplot(3,1,1);
plot(t(1:nb),y(1:nb,1), '-ob');
hold on;
plot(t(1:nb),episodes.x(:,1), '-+r');
hold off;
subplot(3,1,2);
plot(t(1:nb),y(1:nb,2), '-ob');
hold on;
plot(t(1:nb),episodes.x(:,2), '-+r');
hold off;
subplot(3,1,3);
plot(t(1:nb),y(1:nb,3), '-ob');
hold on;
plot(t(1:nb),episodes.x(:,3), '-+r');
hold off;

%% plot in space [x,y]
% theta = delta - gamma
% x = rho * cos(theta)
% y = rho * sin(theta)
clc;

rho   = episodes.x(:,1);
gamma = episodes.x(:,2);
delta = episodes.x(:,3);

Tr = [cos(0), sin(0), 0;
      -sin(0), cos(0), 0;
      0, 0, 1];

theta = delta - gamma;
x = rho .* cos(theta);
y = rho .* sin(theta);


plot(x,y);
hold on;
plot(x(1),y(1), 'sg');
plot(x(end),y(end), 'xr');
fprintf('Angle_0: %10.3f gradi\n', theta(1)*180/pi);
fprintf('Angle_f: %10.3f gradi\n', theta(end)*180/pi);


