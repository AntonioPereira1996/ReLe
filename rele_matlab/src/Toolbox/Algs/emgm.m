function [label, model, llh] = emgm(X, W, init)
% Performs weighted EM algorithm for fitting a Gaussian Mixture Model.
%
%  - X    : D-by-N data matrix
%  - W    : N-by-1 weights vector
%  - init : K (1-by-1) or label (1-by-N, 1 <= label(i) <= K) or center (D-by-K)
%
% Based on the function written by Michael Chen (sth4nth@gmail.com).

R = initialization(X, init);
[~, label(1,:)] = max(R, [], 2);
R = R(:, unique(label));

tol = 1e-10;
maxiter = 500;
llh = -inf(1, maxiter);
converged = false;
t = 1;

while ~converged && t < maxiter

    t = t + 1;
    R = bsxfun(@times, R, W);
    model = maximization(X, R);
    [R, llh(t)] = expectation(X, model);
    
    [~, label(:)] = max(R, [], 2);
    u = unique(label);   % non-empty components
    if size(R, 2) ~= size(u, 2)
        R = R(:, u);     % remove empty components
    else
        converged = llh(t) - llh(t-1) < tol * abs(llh(t));
    end
    
end

llh = llh(2:t);
% if converged
%     fprintf('Converged in %d steps.\n',t-1);
% else
%     fprintf('Not converged in %d steps.\n',maxiter);
% end

end

%% Init
function R = initialization(X, init)

[d, n] = size(X);
if isstruct(init)         % initialize with a model
    R = expectation(X, init);
% elseif length(init) == 1  % random initialization with k components
%     idx = randsample(n,init);
%     model.mu = X(:,idx);  % the means are k random points of the dataset
%     model.ComponentProportion = init;
%     model.Sigma = repmat(cov(X'),1,1,init);  % the covariances are the covariance of the dataset
%     R = expectation(X, model);
elseif length(init) == 1  % random initialization
    k = init;
    idx = randsample(n,k);
    m = X(:,idx);
    [~,label] = max(bsxfun(@minus,m'*X,dot(m,m,1)'/2),[],1);
    [u,~,label] = unique(label);
    while k ~= length(u)
        idx = randsample(n,k);
        m = X(:,idx);
        [~,label] = max(bsxfun(@minus,m'*X,dot(m,m,1)'/2),[],1);
        [u,~,label] = unique(label);
    end
    R = full(sparse(1:n,label,1,n,k,n));
elseif size(init,1) == 1 && size(init,2) == n  % initialize with labels
    label = init;
    k = max(label);
    R = full(sparse(1:n,label,1,n,k,n));
elseif size(init,1) == d  % initialize with only centers
    k = size(init,2);
    m = init;
    [~, label] = max(bsxfun(@minus, m' * X, dot(m,m,1)' / 2), [], 1);
    R = full(sparse(1:n,label,1,n,k,n));
else
    error('Initialization is not valid.');
end

end

%% Expectation
function [R, llh] = expectation(X, model)

mu = model.mu;
Sigma = model.Sigma;
w = model.ComponentProportion;

n = size(X,2);
k = size(mu,2);
logRho = zeros(n,k);

for i = 1 : k
    logRho(:,i) = loggausspdf(X, mu(:,i), Sigma(:,:,i));
end

logRho = bsxfun(@plus, logRho, log(w));
T = logsumexp(logRho,2);
llh = sum(T) / n; % loglikelihood
logR = bsxfun(@minus, logRho, T);
R = exp(logR);

end


%% Maximization
function model = maximization(X, R)

[d,n] = size(X);
k = size(R,2);

nk = sum(R,1);
w = nk / n;
mu = bsxfun(@times, X * R, 1 ./ nk);

Sigma = zeros(d,d,k);
sqrtR = sqrt(R);
for i = 1 : k
    Xo = bsxfun(@minus, X, mu(:,i));
    Xo = bsxfun(@times, Xo, sqrtR(:,i)');
    Sigma(:,:,i) = Xo * Xo' / nk(i);
    Sigma(:,:,i) = Sigma(:,:,i) + 1e-6 * eye(d); % add a prior for numerical stability
end

model.mu = mu;
model.Sigma = Sigma;
model.ComponentProportion = w;

end
