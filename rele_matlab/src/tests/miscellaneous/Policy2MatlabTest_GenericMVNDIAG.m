%load policies
% addpath(genpath('../Toolbox/Library'));
%locate program
clear all;
reset(symengine);
excmd = '../../../../build/test/pol2mat';

%% Multivariate Normal policy with diagonal covariance (sigma parameters)
polname = 'genericmvndiag';
stateDim = 2;
actionDim = 2;
s = sym('s', [stateDim, 1]);
a = sym('a', [actionDim, 1]);
phi = [1;s(end:-1:1)];
if (actionDim == 2)
    c{1} = phi;
    c{2} = phi;
    phi = blkdiag(c{:});
end
w = sym('w', [size(phi,1), 1]);
mu = transpose(phi)*w;
diff = (a - mu);

sigma = sym('sg', [actionDim,1]);
S = diag(sigma.*sigma);
pols = (2*pi)^(-actionDim/2) * det(S)^(-1/2) * exp( ...1
    -0.5 * transpose(diff) * inv(S) * diff ...
    );

% polf = matlabFunction(pols);

g = transpose(jacobian(log(pols), [w;sigma]));
h = jacobian(g, [w;sigma]);


polDeg = 1;
state = [1.21321;0.956];
action = [0.865;1.123];
wVal = rand(6,1).*[0.5; 0.245; 0.99;0.3; 0.3245; 0.599];
sigmaVal = [1.3; 0.9] .* rand(2,1);

% write parameters
mkdir('/tmp/ReLe/pol2mat/test/')
dlmwrite('/tmp/ReLe/pol2mat/test/state.dat', state, 'delimiter', '\t', 'precision', 10);
dlmwrite('/tmp/ReLe/pol2mat/test/action.dat', action, 'delimiter', '\t', 'precision', 10);
dlmwrite('/tmp/ReLe/pol2mat/test/params.dat', [wVal;sigmaVal], 'delimiter', '\t', 'precision', 10);
dlmwrite('/tmp/ReLe/pol2mat/test/deg.dat', polDeg, 'delimiter', '\t');

tcmd = [excmd ' ' polname ' /tmp/ReLe/pol2mat/test/params.dat /tmp/ReLe/pol2mat/test/state.dat ' ...
    '/tmp/ReLe/pol2mat/test/action.dat /tmp/ReLe/pol2mat/test/deg.dat'];

disp('------------------------');
status = system(tcmd);
disp('------------------------');

%% read values
redD = dlmread('/tmp/ReLe/pol2mat/test/density.dat');
redG = dlmread('/tmp/ReLe/pol2mat/test/grad.dat');
% redH = dlmread('/tmp/ReLe/pol2mat/test/hessian.dat');

% compute using sym engine
evalD = double(subs(pols, [s;a;w;sigma], [state;action;wVal;sigmaVal]));
evalG = double(subs(g, [s;a;w;sigma], [state;action;wVal;sigmaVal]));
evalH = double(subs(h, [s;a;w;sigma], [state;action;wVal;sigmaVal]));
mval = double(subs(mu,[s;w], [state;wVal]));
Sval = double(subs(S, sigma, sigmaVal));
pval = mvnpdf(action,mval,Sval);
assert(abs(evalD-pval) <= 1e-9);

% check
[redD, evalD]
assert(abs(redD-evalD) <= 1e-6);

[redG, evalG]
assert(max(abs(redG-evalG)) <= 1e-6);

% redH, evalH
% assert(max(max(abs(redH-evalH))) <= 1e-6);


samples = dlmread('/tmp/ReLe/pol2mat/test/samples.dat');
assert(norm(mean(samples)' - mval) <= 0.1);
